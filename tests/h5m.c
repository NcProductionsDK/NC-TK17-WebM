#include "../NC-TK17-WebM.c"
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)

static float heard_position[3];
static int heard_relative, position_updates;
static void __cdecl capture_source_i(unsigned int source, int key, int value)
{
    CHECK(source == 7 && key == 0x202);
    heard_relative = value;
}
static void __cdecl capture_source_v(unsigned int source, int key, const float *value)
{
    CHECK(source == 7 && key == 0x1004);
    memcpy(heard_position, value, sizeof(heard_position));
    ++position_updates;
}
static void identity(float *m)
{
    memset(m, 0, 64); m[0] = m[5] = m[10] = m[15] = 1;
}
static void write_constants(ID3D11DeviceContext *context, ID3D11Buffer *buffer, const float *values)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    CHECK(SUCCEEDED(ID3D11DeviceContext_Map(context, (ID3D11Resource*)buffer,
        0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)));
    memcpy(mapped.pData, values, 416);
    ID3D11DeviceContext_Unmap(context, (ID3D11Resource*)buffer, 0);
}
static void check_spatial(ID3D11Device *device, ID3D11DeviceContext *context,
                          ID3D11ShaderResourceView *original, video_d3d8_texture_t *player)
{
    /* Model the verified module layout, using actual D3D11 buffers/Map/Unmap. */
    BYTE *module = VirtualAlloc(NULL, 0x245000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER*)module;
    IMAGE_NT_HEADERS32 *nt;
    ID3D11Buffer *call = NULL, *transform = NULL, *unrelated = NULL;
    D3D11_BUFFER_DESC desc = {0};
    float world[104] = {0}, camera[104] = {0}, source[3] = {0};
    audio_graph_t ag = {0};
    CHECK(module && !h5m_audio_layout(module));
    dos->e_magic = IMAGE_DOS_SIGNATURE; dos->e_lfanew = 0x80;
    nt = (IMAGE_NT_HEADERS32*)(module + dos->e_lfanew);
    nt->Signature = IMAGE_NT_SIGNATURE;
    nt->FileHeader.Machine = IMAGE_FILE_MACHINE_I386;
    nt->OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
    nt->OptionalHeader.SizeOfImage = 0x245000;
    nt->FileHeader.TimeDateStamp = 0x603CF2B9;
    CHECK(h5m_audio_layout(module));
    CHECK(patch_vtable_slot(context, 14, (void*)hook_h5m_Map, (void**)&h5m_real_Map));
    CHECK(patch_vtable_slot(context, 15, (void*)hook_h5m_Unmap, (void**)&h5m_real_Unmap));
    desc.ByteWidth = 416; desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    CHECK(SUCCEEDED(ID3D11Device_CreateBuffer(device, &desc, NULL, &call)));
    CHECK(SUCCEEDED(ID3D11Device_CreateBuffer(device, &desc, NULL, &transform)));
    CHECK(SUCCEEDED(ID3D11Device_CreateBuffer(device, &desc, NULL, &unrelated)));
    *h5m_call_buffer_slot = call; *h5m_transform_buffer_slot = transform;
    identity(world); identity(camera + 16);
    camera[11] = -1; /* RH projection, OpenAL -Z forward. */
    memcpy((void*)h5m_main_view, camera + 16, 64);
    world[12] = 3; world[13] = 2; world[14] = -4;
    ++d3d8_present_serial;
    write_constants(context, transform, camera); write_constants(context, call, world);
    ID3D11DeviceContext_PSSetShaderResources(context, 0, 1, &original);
    CHECK(h5m_audio_position(player->audio_node, source));
    CHECK(source[0] == 3 && source[1] == 2 && source[2] == -4);
    CHECK(h5m_views[0].audio_distance2 == 29);
    /* A farther duplicate of the same screen cannot steal its shared emitter. */
    world[12] = 10;
    write_constants(context, call, world);
    ID3D11DeviceContext_PSSetShaderResources(context, 0, 1, &original);
    CHECK(h5m_audio_position(player->audio_node, source) && source[0] == 3);
    /* Movement on the following frame, with a translated listener. */
    ++d3d8_present_serial; world[12] = -6; camera[28] = 2;
    memcpy((void*)h5m_main_view, camera + 16, 64);
    write_constants(context, transform, camera); write_constants(context, call, world);
    ID3D11DeviceContext_PSSetShaderResources(context, 0, 1, &original);
    CHECK(h5m_audio_position(player->audio_node, source) && source[0] == -4);
    /* Both local WebM and Twitch use this source update, without scene symbols. */
    vm_alSourcei = capture_source_i; vm_alSourcefv = capture_source_v;
    ag.openal = ag.audio_3d = 1; ag.al_source = 7;
    lstrcpynA(ag.source_name, player->audio_node, sizeof(ag.source_name));
    audio_graph_update_3d(&ag, 1000);
    CHECK(heard_relative == 1 && heard_position[0] == -4 && heard_position[2] == -4);
    ag.twitch_streaming = ag.twitch_override_audio = 1;
    audio_graph_update_3d(&ag, 1100); CHECK(position_updates == 2);
    /* Shadow/cubemap camera and unrelated mapped buffers cannot move it. */
    camera[28] = 900; world[12] = 500;
    write_constants(context, transform, camera); write_constants(context, unrelated, world);
    ++d3d8_present_serial;
    write_constants(context, call, world);
    ID3D11DeviceContext_PSSetShaderResources(context, 0, 1, &original);
    CHECK(h5m_audio_position(player->audio_node, source) && source[0] == -4);
    /* A 90 degree camera turn moves front sound to the side, preserving distance. */
    identity(camera + 16); camera[16] = camera[26] = 0;
    camera[18] = -1; camera[24] = 1;
    memcpy((void*)h5m_main_view, camera + 16, 64);
    world[12] = 0; world[13] = 0; world[14] = -5;
    ++d3d8_present_serial;
    write_constants(context, transform, camera); write_constants(context, call, world);
    ID3D11DeviceContext_PSSetShaderResources(context, 0, 1, &original);
    CHECK(h5m_audio_position(player->audio_node, source));
    CHECK(source[0] == -5 && source[1] == 0 && source[2] == 0);
    CHECK(h5m_views[0].audio_distance2 == 25);
    /* A normal-map bind, a stale world, and non-finite matrices are ignored. */
    ++d3d8_present_serial;
    ID3D11DeviceContext_PSSetShaderResources(context, 0, 1, &original);
    CHECK(h5m_views[0].audio_position_serial != d3d8_present_serial);
    world[14] = -1; write_constants(context, call, world);
    ID3D11DeviceContext_PSSetShaderResources(context, 1, 1, &original);
    CHECK(h5m_audio_position(player->audio_node, source) && source[0] == -5);
    world[14] = NAN; write_constants(context, call, world);
    ID3D11DeviceContext_PSSetShaderResources(context, 0, 1, &original);
    CHECK(h5m_audio_position(player->audio_node, source) && source[0] == -5);
    /* LH projection is converted to OpenAL's forward convention too. */
    identity(camera + 16); camera[11] = 1;
    memcpy((void*)h5m_main_view, camera + 16, 64); world[14] = 5;
    write_constants(context, transform, camera); write_constants(context, call, world);
    ID3D11DeviceContext_PSSetShaderResources(context, 0, 1, &original);
    CHECK(h5m_audio_position(player->audio_node, source) && source[2] == -5);
    /* Disabling spatial audio preserves the ordinary stereo path. */
    ag.audio_3d = 0; audio_graph_update_3d(&ag, 1200); CHECK(position_updates == 2);
    CHECK(!h5m_audio_layout(NULL));
    ID3D11Buffer_Release(call); ID3D11Buffer_Release(transform); ID3D11Buffer_Release(unrelated);
    VirtualFree(module, 0, MEM_RELEASE);
    vm_alSourcei = NULL; vm_alSourcefv = NULL;
    puts("PASS: H5M Map/Unmap spatial capture, object/camera motion, nearest instance, shadow isolation, OpenAL relative position, invalid/stale matrices");
}

static void check_pixels(video_d3d8_texture_t *player, ID3D11Device *device,
                         ID3D11DeviceContext *context)
{
    ID3D11Texture2D *readback = NULL;
    D3D11_TEXTURE2D_DESC desc;
    D3D11_MAPPED_SUBRESOURCE map;
    const BYTE rgb[] = {255,0,0, 0,255,0, 0,0,255, 255,255,255};
    video_decoder_t *decoder = calloc(1, sizeof(*decoder));
    DWORD *row;
    CHECK(decoder);
    decoder->frame = malloc(sizeof(rgb)); CHECK(decoder->frame);
    memcpy(decoder->frame, rgb, sizeof(rgb));
    decoder->width = decoder->height = 2; decoder->stride = 6;
    player->decoder = decoder;
    CHECK(h5m_upload_frame(player));
    ID3D11Texture2D_GetDesc(player->d3d11_texture, &desc);
    desc.Usage = D3D11_USAGE_STAGING; desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.BindFlags = desc.MiscFlags = 0;
    CHECK(SUCCEEDED(ID3D11Device_CreateTexture2D(device, &desc, NULL, &readback)));
    ID3D11DeviceContext_CopyResource(context, (ID3D11Resource*)readback,
                                    (ID3D11Resource*)player->d3d11_texture);
    CHECK(SUCCEEDED(ID3D11DeviceContext_Map(context, (ID3D11Resource*)readback,
        0, D3D11_MAP_READ, 0, &map)));
    row = (DWORD*)map.pData;
    CHECK(row[0] == 0xffff0000u); /* Top left red: no vertical flip. */
    CHECK(row[desc.Width - 1] == 0xff00ff00u);
    row = (DWORD*)((BYTE*)map.pData + (desc.Height - 1) * map.RowPitch);
    CHECK(row[0] == 0xff0000ffu);
    CHECK(row[desc.Width - 1] == 0xffffffffu);
    ID3D11DeviceContext_Unmap(context, (ID3D11Resource*)readback, 0);
    ID3D11Texture2D_Release(readback);
}

int main(int argc, char **argv)
{
    ID3D11Device *device = NULL;
    ID3D11DeviceContext *context = NULL;
    ID3D11ShaderResourceView *original = NULL, *other = NULL, *bound[2], *got[2];
    D3D_FEATURE_LEVEL feature;
    HMODULE d3dx;
    WCHAR image[MAX_PATH * 4];
    char webm[MAX_PATH * 4], temp_dir[MAX_PATH], temp_file[MAX_PATH];
    video_d3d8_texture_t *player;
    CHECK(argc == 2);
    CHECK(GetTempPathA(sizeof(temp_dir), temp_dir));
    CHECK(GetTempFileNameA(temp_dir, "h5v", 0, temp_file));
    config_loaded = 1;
    lstrcpynA(config_path_global, temp_file, sizeof(config_path_global));
    twitch_override_enabled = 0;
    CHECK(h5m_sidecar(argv[1], webm, sizeof(webm)));
    CHECK(!h5m_sidecar("C:/Mod/ActiveMod/ordinary.png", webm, sizeof(webm)));
    CHECK(!h5m_sidecar("C:/_hook5data/objects/CRT TV/model.h5m", webm, sizeof(webm)));
    CHECK(SUCCEEDED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_WARP, NULL, 0,
        NULL, 0, D3D11_SDK_VERSION, &device, &feature, &context)));
    capture_d3d11_runtime(device, context);
    CHECK(h5m_real_SetViews && h5m_context_vtable);
    d3dx = LoadLibraryA("d3dx11_43.dll"); CHECK(d3dx);
    h5m_real_LoadView = (h5m_load_view_t)GetProcAddress(d3dx, "D3DX11CreateShaderResourceViewFromFileW");
    CHECK(h5m_real_LoadView);
    real_GetProcAddress = (GetProcAddress_t)GetProcAddress;
    CHECK(hook_GetProcAddress(d3dx, "D3DX11CreateShaderResourceViewFromFileW") ==
          (FARPROC)hook_h5m_LoadView);
    CHECK(hook_GetProcAddress(d3dx, MAKEINTRESOURCEA(12)) ==
          GetProcAddress(d3dx, MAKEINTRESOURCEA(12)));
    {
        char directory[MAX_PATH * 4], line[MAX_PATH * 4];
        FILE *catalog = tmpfile(); CHECK(catalog);
        lstrcpynA(directory, argv[1], sizeof(directory)); dirname_inplace(directory);
        h5m_catalog_scan(catalog, directory, 0); rewind(catalog);
        CHECK(fgets(line, sizeof(line), catalog));
        CHECK(strstr(line, "NcToy7_TV_Screen.ini"));
        CHECK(!fgets(line, sizeof(line), catalog));
        fclose(catalog);
    }
    CHECK(MultiByteToWideChar(CP_ACP, 0, argv[1], -1, image, sizeof(image)/sizeof(image[0])));
    CHECK(SUCCEEDED(hook_h5m_LoadView(device, image, NULL, NULL, &original, NULL)));
    CHECK(h5m_view_count == 1 && original);
    CHECK(SUCCEEDED(h5m_real_LoadView(device, image, NULL, NULL, &other, NULL)));
    bound[0] = original; bound[1] = other;
    ID3D11DeviceContext_PSSetShaderResources(context, 0, 2, bound);
    ID3D11DeviceContext_PSGetShaderResources(context, 0, 2, got);
    CHECK(got[0] == original && got[1] == other);
    ID3D11ShaderResourceView_Release(got[0]); ID3D11ShaderResourceView_Release(got[1]);
    h5m_process_views();
    player = h5m_views[0].player;
    CHECK(player && player->active && !player->texture && player->h5m_video_view);
    CHECK(!player->twitch_session); /* Empty native channel uses local WebM. */
    CHECK(player->audio_3d && !player->audio_engine && !strncmp(player->audio_node, "h5m:", 4));
    check_spatial(device, context, original, player);
    check_pixels(player, device, context);
    player->uploaded_frame_serial = 1;
    ID3D11DeviceContext_PSSetShaderResources(context, 0, 2, bound);
    ID3D11DeviceContext_PSGetShaderResources(context, 0, 2, got);
    CHECK(got[0] == player->h5m_video_view && got[1] == other);
    ID3D11ShaderResourceView_Release(got[0]); ID3D11ShaderResourceView_Release(got[1]);
    /* Offline override without a WebM frame restores the original image. */
    player->twitch_session = (webm_twitch_session_t*)1;
    player->twitch_active = player->twitch_fallback_active = 0;
    ID3D11DeviceContext_PSSetShaderResources(context, 0, 2, bound);
    ID3D11DeviceContext_PSGetShaderResources(context, 0, 2, got);
    CHECK(got[0] == original && got[1] == other);
    ID3D11ShaderResourceView_Release(got[0]); ID3D11ShaderResourceView_Release(got[1]);
    player->twitch_active = 1;
    ID3D11DeviceContext_PSSetShaderResources(context, 0, 2, bound);
    ID3D11DeviceContext_PSGetShaderResources(context, 0, 2, got);
    CHECK(got[0] == player->h5m_video_view && got[1] == other);
    ID3D11ShaderResourceView_Release(got[0]); ID3D11ShaderResourceView_Release(got[1]);
    player->twitch_session = NULL;
    player->last_config_check_tick = 0;
    player->config_generation = 0;
    refresh_d3d8_texture_settings(player, GetTickCount());
    CHECK(!player->uploaded_frame_serial && player->uploaded_frame_index == -1);
    CHECK(!player->decoder && !player->twitch_session);
    /* Global overrides match the native screen's sidecar, including volume. */
    twitch_override_enabled = 1;
    strcpy(twitch_override_channel, "test");
    lstrcpynA(twitch_override_target, player->sidecar_ini_path, sizeof(twitch_override_target));
    CHECK(twitch_override_matches_sidecar_a(player->sidecar_ini_path));
    CHECK(!twitch_override_matches_sidecar_a("C:/unrelated.ini"));
    strcpy(twitch_override_target, "auto_room");
    twitch_override_auto_target[0] = 0;
    CHECK(!twitch_override_matches_sidecar_a(player->sidecar_ini_path));
    h5m_views[0].last_bound = GetTickCount() - 2000;
    h5m_process_views();
    CHECK(!h5m_views[0].player && !player->active); /* Invisible objects stop playback. */
    h5m_clear_views(); CHECK(!h5m_view_count);
    ID3D11DeviceContext_ClearState(context);
    CHECK(ID3D11ShaderResourceView_Release(original) == 0);
    CHECK(ID3D11ShaderResourceView_Release(other) == 0);
    ID3D11DeviceContext_Release(captured_d3d11_context); captured_d3d11_context = NULL;
    ID3D11Device_Release(captured_d3d11_device); captured_d3d11_device = NULL;
    ID3D11DeviceContext_Release(context); ID3D11Device_Release(device);
    FreeLibrary(d3dx); CHECK(DeleteFileA(temp_file));
    puts("PASS: real D3DX11 file loading, H5M registration, WARP upload/readback, orientation/colors, native binding, unrelated texture isolation, Twitch/fallback selection, override targeting, idle cleanup");
    return 0;
}
