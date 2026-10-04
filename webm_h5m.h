/* Hook5 native object textures bypass TK17's D3D8 texture loader. Associate
 * eligible file-loaded SRVs with the shared video player, then substitute only
 * those views at PS binding. The model/material and original image stay intact.
 * Included by NC-TK17-WebM.c after its shared playback helpers. */
#ifndef WEBM_H5M_H
#define WEBM_H5M_H

#define WEBM_H5M_VIEWS 32
typedef HRESULT (WINAPI *h5m_load_view_t)(ID3D11Device *, const WCHAR *,
    void *, void *, ID3D11ShaderResourceView **, HRESULT *);
typedef void (STDMETHODCALLTYPE *h5m_set_views_t)(ID3D11DeviceContext *, UINT,
    UINT, ID3D11ShaderResourceView *const *);
static h5m_load_view_t h5m_real_LoadView;
static h5m_set_views_t h5m_real_SetViews;
static void **h5m_context_vtable;
static SRWLOCK h5m_view_lock = SRWLOCK_INIT;
static volatile LONG h5m_view_count;
typedef HRESULT (STDMETHODCALLTYPE *h5m_map_t)(ID3D11DeviceContext *,
    ID3D11Resource *, UINT, D3D11_MAP, UINT, D3D11_MAPPED_SUBRESOURCE *);
typedef void (STDMETHODCALLTYPE *h5m_unmap_t)(ID3D11DeviceContext *, ID3D11Resource *, UINT);
static h5m_map_t h5m_real_Map;
static h5m_unmap_t h5m_real_Unmap;
static ID3D11Buffer **h5m_call_buffer_slot, **h5m_transform_buffer_slot;
static const float *h5m_main_view;
typedef struct {
    ID3D11Resource *resource;
    const float *mapped;
    float matrix[32];
    int valid;
    unsigned int serial;
} h5m_constants_t;
static h5m_constants_t h5m_call_constants, h5m_transform_constants;
typedef struct {
    ID3D11ShaderResourceView *original;
    char image[MAX_PATH * 4];
    DWORD last_bound;
    unsigned int bound_serial;
    video_d3d8_texture_t *player;
    float audio_position[3];
    float audio_distance2;
    int audio_position_valid;
    unsigned int audio_position_serial;
} h5m_view_t;
static h5m_view_t h5m_views[WEBM_H5M_VIEWS];

/* Verified Hook5 2021 layout, also used by Hook5 Extended's camera bridge.
 * Refuse unknown binaries: their constant buffers may have different layouts.
 * cbCallData begins with mworld; cbTransformData begins with mproj, mview.
 * Native renderer 0xD59B0 writes/unmaps cbCallData BEFORE binding its diffuse
 * SRV. Matrices are D3DX row-vector memory (HLSL column-major); translation is
 * at elements 12..14. No GPU readback and no engine object-pointer guessing. */
static int h5m_audio_layout(BYTE *module)
{
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER*)module;
    IMAGE_NT_HEADERS32 *nt;
    h5m_call_buffer_slot = h5m_transform_buffer_slot = NULL;
    h5m_main_view = NULL;
    if (!module || !ptr_readable(dos, sizeof(*dos)) ||
        dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0 ||
        dos->e_lfanew > 0x10000) return 0;
    nt = (IMAGE_NT_HEADERS32*)(module + dos->e_lfanew);
    if (!ptr_readable(nt, sizeof(*nt)) || nt->Signature != IMAGE_NT_SIGNATURE ||
        nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386 ||
        nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
        nt->OptionalHeader.SizeOfImage != 0x245000 ||
        (nt->FileHeader.TimeDateStamp != 0x603CF2B9 &&
         nt->FileHeader.TimeDateStamp != 0x603CF2C4) ||
        !ptr_readable(module + 0x1D2770, sizeof(void*)) ||
        !ptr_readable(module + 0x1D25C8, sizeof(void*)) ||
        !ptr_readable(module + 0x1D847C, 64)) return 0;
    h5m_call_buffer_slot = (ID3D11Buffer**)(module + 0x1D2770);
    h5m_transform_buffer_slot = (ID3D11Buffer**)(module + 0x1D25C8);
    h5m_main_view = (const float*)(module + 0x1D847C);
    return 1;
}

static int h5m_affine_matrix(const float *m)
{
    int i;
    for (i = 0; i < 16; ++i) if (!isfinite(m[i])) return 0;
    return fabsf(m[3]) < 0.0001f && fabsf(m[7]) < 0.0001f &&
           fabsf(m[11]) < 0.0001f && fabsf(m[15] - 1.0f) < 0.0001f;
}

static HRESULT STDMETHODCALLTYPE hook_h5m_Map(ID3D11DeviceContext *self,
    ID3D11Resource *resource, UINT sub, D3D11_MAP type, UINT flags,
    D3D11_MAPPED_SUBRESOURCE *mapped)
{
    HRESULT hr = h5m_real_Map(self, resource, sub, type, flags, mapped);
    h5m_constants_t *capture = NULL;
    D3D11_BUFFER_DESC desc;
    if (self != captured_d3d11_context || !h5m_view_count ||
        !h5m_call_buffer_slot || sub) return hr;
    if (resource == (ID3D11Resource*)*h5m_call_buffer_slot) capture = &h5m_call_constants;
    else if (resource == (ID3D11Resource*)*h5m_transform_buffer_slot) capture = &h5m_transform_constants;
    if (!capture) return hr;
    capture->valid = 0;
    capture->mapped = NULL;
    capture->resource = NULL;
    if (FAILED(hr) || !mapped || !mapped->pData ||
        (type != D3D11_MAP_WRITE_DISCARD && type != D3D11_MAP_WRITE &&
         type != D3D11_MAP_WRITE_NO_OVERWRITE)) return hr;
    ID3D11Buffer_GetDesc((ID3D11Buffer*)resource, &desc);
    if (desc.ByteWidth != 416 || !(desc.BindFlags & D3D11_BIND_CONSTANT_BUFFER)) return hr;
    capture->resource = resource;
    capture->mapped = (const float*)mapped->pData;
    return hr;
}

static void STDMETHODCALLTYPE hook_h5m_Unmap(ID3D11DeviceContext *self,
    ID3D11Resource *resource, UINT sub)
{
    h5m_constants_t *capture = NULL;
    if (self == captured_d3d11_context && !sub) {
        if (resource == h5m_call_constants.resource) capture = &h5m_call_constants;
        else if (resource == h5m_transform_constants.resource) capture = &h5m_transform_constants;
        if (capture && capture->mapped) {
            memcpy(capture->matrix, capture->mapped, sizeof(capture->matrix));
            capture->valid = 1;
            capture->serial = d3d8_present_serial;
            capture->mapped = NULL;
            capture->resource = NULL;
        }
    }
    h5m_real_Unmap(self, resource, sub);
}

/* Called while holding the view lock, only for an eligible diffuse bind.
 * Mirrors leave mworld unchanged; shadow/cubemap cameras are excluded by the
 * main-view comparison. Multiple instances of one texture share one decoder
 * and emitter; use the nearest rendered instance consistently within a frame. */
static void h5m_capture_audio_position(h5m_view_t *entry)
{
    const float *world = h5m_call_constants.matrix;
    const float *view = h5m_transform_constants.matrix + 16;
    float position[3], distance2;
    int i;
    if (!h5m_main_view || !h5m_call_constants.valid || !h5m_transform_constants.valid ||
        h5m_call_constants.serial != d3d8_present_serial ||
        !h5m_affine_matrix(world) || !h5m_affine_matrix(view)) return;
    for (i = 0; i < 16; ++i)
        if (!isfinite(h5m_main_view[i]) || fabsf(view[i] - h5m_main_view[i]) > 0.0001f) return;
    for (i = 0; i < 3; ++i) {
        position[i] = world[12] * view[i] + world[13] * view[4+i] +
                      world[14] * view[8+i] + view[12+i];
        if (!isfinite(position[i]) || fabsf(position[i]) > 100000.0f) return;
    }
    /* Perspective mproj[11] gives the forward-axis sign: OpenAL faces -Z. */
    if (h5m_transform_constants.matrix[11] > 0.0f) position[2] = -position[2];
    distance2 = position[0]*position[0] + position[1]*position[1] + position[2]*position[2];
    if (entry->audio_position_valid && entry->audio_position_serial == d3d8_present_serial &&
        distance2 >= entry->audio_distance2) return;
    memcpy(entry->audio_position, position, sizeof(position));
    entry->audio_distance2 = distance2;
    entry->audio_position_valid = 1;
    entry->audio_position_serial = d3d8_present_serial;
}

static void h5m_configure_audio(video_d3d8_texture_t *vt)
{
    /* H5M objects are outside TK17's scene graph; use the existing OpenAL path. */
    vt->audio_engine = 0;
    snprintf(vt->audio_node, sizeof(vt->audio_node), "h5m:%u",
             (unsigned int)(vt - video_d3d8_textures));
}

static int h5m_audio_position(const char *name, float source[3])
{
    unsigned int slot;
    int i, found = 0;
    if (!name || sscanf(name, "h5m:%u", &slot) != 1 ||
        slot >= sizeof(video_d3d8_textures)/sizeof(video_d3d8_textures[0])) return 0;
    AcquireSRWLockShared(&h5m_view_lock);
    for (i = 0; i < WEBM_H5M_VIEWS; ++i) {
        h5m_view_t *entry = &h5m_views[i];
        if (entry->player == &video_d3d8_textures[slot] && entry->player->active &&
            entry->audio_position_valid && GetTickCount() - entry->last_bound <= 1500) {
            memcpy(source, entry->audio_position, sizeof(entry->audio_position));
            found = 1;
            break;
        }
    }
    ReleaseSRWLockShared(&h5m_view_lock);
    return found;
}

static int h5m_sidecar(const char *image, char *webm, size_t size)
{
    char normalized[MAX_PATH * 4], ini[MAX_PATH * 4];
    char *dot;
    normalize_override_path_a(image, normalized, sizeof(normalized));
    if (!contains_i(normalized, "/mod/activemod/")) return 0;
    dot = strrchr(normalized, '.');
    if (!dot || (_stricmp(dot, ".png") && _stricmp(dot, ".dds") &&
                 _stricmp(dot, ".jpg") && _stricmp(dot, ".tga"))) return 0;
    lstrcpynA(webm, image, (int)size);
    dot = strrchr(webm, '.');
    if (!dot || size - (size_t)(dot - webm) < 6) return 0;
    strcpy(dot, ".webm");
    sidecar_ini_path_a(webm, ini, sizeof(ini));
    /* An empty Twitch section is a valid override-only target. */
    return regular_file_exists_a(webm) || webm_twitch_sidecar_is_compatible(ini);
}

static int h5m_player_matches(const h5m_view_t *entry)
{
    return entry->player && entry->player->active &&
           entry->player->h5m_original_view == entry->original;
}

static void h5m_clear_views(void)
{
    int i;
    AcquireSRWLockExclusive(&h5m_view_lock);
    for (i = 0; i < WEBM_H5M_VIEWS; ++i) {
        if (h5m_player_matches(&h5m_views[i]))
            clear_d3d8_texture_slot(h5m_views[i].player);
        if (h5m_views[i].original)
            ID3D11ShaderResourceView_Release(h5m_views[i].original);
    }
    memset(h5m_views, 0, sizeof(h5m_views));
    memset(&h5m_call_constants, 0, sizeof(h5m_call_constants));
    memset(&h5m_transform_constants, 0, sizeof(h5m_transform_constants));
    InterlockedExchange(&h5m_view_count, 0);
    ReleaseSRWLockExclusive(&h5m_view_lock);
}

static HRESULT WINAPI hook_h5m_LoadView(ID3D11Device *device, const WCHAR *path,
    void *load_info, void *pump, ID3D11ShaderResourceView **view, HRESULT *result)
{
    HRESULT hr = h5m_real_LoadView ?
        h5m_real_LoadView(device, path, load_info, pump, view, result) : E_FAIL;
    char image[MAX_PATH * 4], webm[MAX_PATH * 4];
    int i, free_index = -1;
    /* Hook5 uses synchronous file loads. An asynchronous result is not ready. */
    if (FAILED(hr) || pump || !path || !view || !*view) return hr;
    wide_to_mb(path, image, sizeof(image));
    if (!h5m_sidecar(image, webm, sizeof(webm))) return hr;
    if (captured_d3d11_device != device || !captured_d3d11_context) {
        ID3D11DeviceContext *context = NULL;
        ID3D11Device_GetImmediateContext(device, &context);
        if (context) {
            capture_d3d11_runtime(device, context);
            ID3D11DeviceContext_Release(context);
        }
    }
    AcquireSRWLockExclusive(&h5m_view_lock);
    for (i = 0; i < WEBM_H5M_VIEWS; ++i) {
        if (h5m_views[i].original == *view) break;
        if (!h5m_views[i].original && free_index < 0) free_index = i;
    }
    if (i == WEBM_H5M_VIEWS && free_index >= 0) {
        h5m_view_t *entry = &h5m_views[free_index];
        ID3D11ShaderResourceView_AddRef(*view);
        entry->original = *view;
        lstrcpynA(entry->image, image, sizeof(entry->image));
        InterlockedIncrement(&h5m_view_count);
        log_line("H5M video texture discovered image=\"%s\"", image);
    } else if (i == WEBM_H5M_VIEWS) {
        log_line("H5M video tracking full; keeping original image=\"%s\"", image);
    }
    ReleaseSRWLockExclusive(&h5m_view_lock);
    return hr;
}

static void STDMETHODCALLTYPE hook_h5m_PSSetShaderResources(
    ID3D11DeviceContext *self, UINT start, UINT count,
    ID3D11ShaderResourceView *const *views)
{
    ID3D11ShaderResourceView *replacement[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
    UINT j;
    int i, changed = 0;
    DWORD now;
    if (!h5m_real_SetViews) return;
    if (!views || !h5m_view_count || self != captured_d3d11_context ||
        count > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT) {
        h5m_real_SetViews(self, start, count, views);
        return;
    }
    now = GetTickCount();
    memcpy(replacement, views, count * sizeof(*views));
    AcquireSRWLockExclusive(&h5m_view_lock);
    for (j = 0; j < count; ++j) {
        if (!views[j]) continue;
        for (i = 0; i < WEBM_H5M_VIEWS; ++i) {
            h5m_view_t *entry = &h5m_views[i];
            video_d3d8_texture_t *vt;
            if (entry->original != views[j]) continue;
            if (start + j == 0) h5m_capture_audio_position(entry);
            entry->last_bound = now;
            entry->bound_serial = d3d8_present_serial;
            if (!h5m_player_matches(entry)) break;
            vt = entry->player;
            vt->last_bound_tick = now;
            vt->last_bound_present_serial = d3d8_present_serial;
            if (vt->bind_log_count < 1000000) vt->bind_log_count++;
            /* No frame / offline without WebM: preserve the original image. */
            if (vt->h5m_video_view && vt->uploaded_frame_serial && vt->decoder &&
                (!vt->twitch_session || vt->twitch_active || vt->twitch_fallback_active)) {
                replacement[j] = vt->h5m_video_view;
                changed = 1;
            }
            break;
        }
    }
    h5m_real_SetViews(self, start, count, changed ? replacement : views);
    ReleaseSRWLockExclusive(&h5m_view_lock);
}

static void h5m_patch_context(ID3D11DeviceContext *context)
{
    void **vt;
    if (!context) return;
    vt = *(void ***)context;
    /* One verified immediate-context vtable per game renderer. Refuse a
       different implementation rather than calling the wrong trampoline. */
    if (h5m_context_vtable && h5m_context_vtable != vt) {
        log_line("H5M context hook refused: different context implementation");
        return;
    }
    if (patch_vtable_slot(context, 8, (void*)hook_h5m_PSSetShaderResources,
                          (void**)&h5m_real_SetViews)) {
        h5m_context_vtable = vt;
        if (h5m_audio_layout((BYTE*)GetModuleHandleA("d3d8.dll"))) {
            int mapped = patch_vtable_slot(context, 14, (void*)hook_h5m_Map, (void**)&h5m_real_Map);
            int unmapped = patch_vtable_slot(context, 15, (void*)hook_h5m_Unmap, (void**)&h5m_real_Unmap);
            log_line("H5M spatial transform capture map=%d unmap=%d", mapped, unmapped);
        } else {
            log_line("H5M spatial transforms unavailable for this Hook5 build; using centered audio");
        }
    }
}

static video_d3d8_texture_t *h5m_create_player(h5m_view_t *entry)
{
    video_d3d8_texture_t *slot = NULL;
    ID3D11Resource *resource = NULL;
    ID3D11Texture2D *original_texture = NULL;
    ID3D11Device *owner = NULL;
    D3D11_TEXTURE2D_DESC desc;
    char webm[MAX_PATH * 4];
    int i, max_width, max_height, unused_levels;
    DWORD now = GetTickCount();
    HRESULT hr;
    if (!captured_d3d11_device || !captured_d3d11_context ||
        !h5m_sidecar(entry->image, webm, sizeof(webm))) return NULL;
    ID3D11ShaderResourceView_GetDevice(entry->original, &owner);
    if (owner != captured_d3d11_device) {
        if (owner) ID3D11Device_Release(owner);
        return NULL;
    }
    ID3D11Device_Release(owner);
    ID3D11ShaderResourceView_GetResource(entry->original, &resource);
    if (!resource) return NULL;
    hr = ID3D11Resource_QueryInterface(resource, &IID_ID3D11Texture2D, (void**)&original_texture);
    ID3D11Resource_Release(resource);
    if (FAILED(hr)) return NULL;
    ID3D11Texture2D_GetDesc(original_texture, &desc);
    ID3D11Texture2D_Release(original_texture);
    if (desc.ArraySize != 1 || desc.SampleDesc.Count != 1) return NULL;
    for (i = 0; i < sizeof(video_d3d8_textures)/sizeof(video_d3d8_textures[0]); ++i)
        if (!video_d3d8_textures[i].active) { slot = &video_d3d8_textures[i]; break; }
    if (!slot) return NULL;
    memset(slot, 0, sizeof(*slot));
    load_sidecar_settings_values_a(webm, &slot->update_interval_ms, &max_width,
        &max_height, &unused_levels, &slot->video_filtering, &slot->anisotropy,
        &slot->sidecar_ini_write_time, 0);
    slot->width = (int)desc.Width < max_width ? (int)desc.Width : max_width;
    slot->height = (int)desc.Height < max_height ? (int)desc.Height : max_height;
    memset(&desc, 0, sizeof(desc));
    desc.Width = slot->width; desc.Height = slot->height;
    desc.ArraySize = desc.MipLevels = desc.SampleDesc.Count = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    hr = ID3D11Device_CreateTexture2D(captured_d3d11_device, &desc, NULL, &slot->d3d11_texture);
    if (SUCCEEDED(hr)) hr = ID3D11Device_CreateShaderResourceView(captured_d3d11_device,
        (ID3D11Resource*)slot->d3d11_texture, NULL, &slot->h5m_video_view);
    if (FAILED(hr)) { clear_d3d8_texture_slot(slot); return NULL; }
    ID3D11ShaderResourceView_AddRef(entry->original);
    slot->h5m_original_view = entry->original;
    slot->active = 1;
    register_active_d3d8_slot(slot);
    slot->format = D3DFMT_A8R8G8B8;
    slot->level_count = slot->levels = slot->d3d8_mip_levels = 1;
    slot->d3d11_runtime_generation = captured_d3d11_generation;
    slot->first_seen_tick = now;
    slot->last_bound_tick = entry->last_bound;
    slot->last_bound_present_serial = entry->bound_serial;
    slot->uploaded_frame_index = -1;
    lstrcpynA(slot->image_path, entry->image, sizeof(slot->image_path));
    lstrcpynA(slot->sidecar_path, webm, sizeof(slot->sidecar_path));
    sidecar_ini_path_a(webm, slot->sidecar_ini_path, sizeof(slot->sidecar_ini_path));
    load_sidecar_audio_settings_a(webm, &slot->audio_enabled, &slot->audio_engine,
        &slot->audio_lead_ms, &slot->audio_volume, &slot->audio_3d,
        &slot->audio_3d_min_distance, &slot->audio_3d_max_distance,
        &slot->audio_3d_rolloff, slot->audio_node, sizeof(slot->audio_node),
        slot->audio_effect, sizeof(slot->audio_effect));
    h5m_configure_audio(slot);
    load_sidecar_game_audio_mutes_a(webm, &slot->game_audio_mutes);
    slot->audio_ready_tick = now + 3000;
    slot->twitch_session = create_twitch_session_a(slot->sidecar_ini_path,
        &slot->twitch_settings, &slot->twitch_logged_state);
    slot->last_config_check_tick = now;
    slot->config_generation = config_generation;
    log_line("H5M video player ready size=%dx%d sidecar=\"%s\"", slot->width, slot->height, webm);
    return slot;
}

static void h5m_process_views(void)
{
    int i;
    DWORD now = GetTickCount();
    if (!h5m_view_count) return;
    AcquireSRWLockExclusive(&h5m_view_lock);
    for (i = 0; i < WEBM_H5M_VIEWS; ++i) {
        h5m_view_t *entry = &h5m_views[i];
        if (!entry->original) continue;
        if (!h5m_player_matches(entry)) entry->player = NULL;
        if (!entry->last_bound || now - entry->last_bound > 1500) {
            if (entry->player) {
                clear_d3d8_texture_slot(entry->player);
                entry->player = NULL;
            }
            continue;
        }
        if (!entry->player) entry->player = h5m_create_player(entry);
    }
    ReleaseSRWLockExclusive(&h5m_view_lock);
}

static int h5m_upload_frame(video_d3d8_texture_t *vt)
{
    D3DLOCKED_RECT frame;
    size_t pitch = (size_t)vt->width * 4u, bytes = pitch * (size_t)vt->height;
    BYTE *pixels;
    int y, cached;
    if (!vt->d3d11_texture || !captured_d3d11_context ||
        vt->d3d11_runtime_generation != captured_d3d11_generation ||
        !bytes || bytes > 64u * 1024u * 1024u) return 0;
    if (vt->h5m_pixels_size < bytes * 2) {
        pixels = (BYTE*)realloc(vt->h5m_pixels, bytes * 2);
        if (!pixels) return 0;
        vt->h5m_pixels = pixels; vt->h5m_pixels_size = bytes * 2;
    }
    frame.Pitch = (int)pitch; frame.pBits = vt->h5m_pixels;
    cached = video_decoder_copy_d3d8_cached_mip(vt->decoder, 0, &frame,
        vt->width, vt->height, vt->format);
    if (!cached) {
        fill_d3d8_locked_frame(vt, &frame, 1, vt->width, vt->height);
        if (vt->twitch_active && vt->twitch_chat)
            webm_twitch_chat_compose(vt->twitch_chat, &vt->twitch_settings,
                (BYTE*)frame.pBits, vt->width, vt->height, frame.Pitch,
                WEBM_TWITCH_CHAT_BGR, 4, 1);
    }
    /* Shared TK17 conversion is bottom-up. Native D3DX/H5M images are top-down. */
    pixels = vt->h5m_pixels + bytes;
    for (y = 0; y < vt->height; ++y)
        memcpy(pixels + y * pitch, vt->h5m_pixels + (vt->height - 1 - y) * pitch, pitch);
    ID3D11DeviceContext_UpdateSubresource(captured_d3d11_context,
        (ID3D11Resource*)vt->d3d11_texture, 0, NULL, pixels, (UINT)pitch, (UINT)bytes);
    return 1;
}

static void h5m_catalog_scan(FILE *file, const char *dir, unsigned int depth)
{
    WIN32_FIND_DATAA data;
    char search[MAX_PATH * 4], child[MAX_PATH * 4], webm[MAX_PATH * 4], ini[MAX_PATH * 4];
    HANDLE find;
    if (depth > 24) return;
    path_join(search, sizeof(search), dir, "*");
    find = FindFirstFileA(search, &data);
    if (find == INVALID_HANDLE_VALUE) return;
    do {
        if (!strcmp(data.cFileName, ".") || !strcmp(data.cFileName, "..")) continue;
        path_join(child, sizeof(child), dir, data.cFileName);
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
                h5m_catalog_scan(file, child, depth + 1);
        } else if (h5m_sidecar(child, webm, sizeof(webm))) {
            sidecar_ini_path_a(webm, ini, sizeof(ini));
            fprintf(file, "%s\n", ini);
        }
    } while (FindNextFileA(find, &data));
    FindClose(find);
}

static void h5m_write_target_catalog(void)
{
    char game[MAX_PATH * 4], root[MAX_PATH * 4], catalog[MAX_PATH * 4];
    FILE *file;
    if (!config_path_global[0]) return;
    lstrcpynA(game, config_path_global, sizeof(game));
    dirname_inplace(game); dirname_inplace(game); dirname_inplace(game);
    /* The Lua settings sandbox can read indexed add-on files. It cannot
       enumerate arbitrary ActiveMod directories or read an unindexed catalog. */
    path_join(catalog, sizeof(catalog), game,
        "Addons\\Script.[NC].TK17 WebM Settings\\Scripts\\Shared\\NCWebMH5MTargets.txt");
    path_join(root, sizeof(root), game, "Mod\\ActiveMod");
    file = fopen(catalog, "wb");
    if (!file) return;
    h5m_catalog_scan(file, root, 0);
    fclose(file);
}
#endif
