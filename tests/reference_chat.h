/* Frozen chat rendering/composition before this optimization. Test-only. */


static unsigned int *reference_render_chat_overlay(webm_twitch_chat_session_t *session,
                                         const webm_twitch_chat_message_t *messages,
                                         int message_start, int message_count,
                                         const char *status, int connected,
                                         unsigned int message_serial,
                                         unsigned int status_serial, DWORD now,
                                         int chat_width, int height, int text_size,
                                         int horizontal_padding, int emotes_enabled,
                                         float emote_scale, int animated_enabled,
                                         int animation_fps,
                                         int *content_left, int *content_top,
                                         int *content_right, int *content_bottom)
{
    webm_twitch_chat_render_cache_t *cache = NULL;
    HDC dc;
    HBITMAP bitmap;
    HGDIOBJ old_bitmap;
    HFONT font;
    HGDIOBJ old_font;
    BITMAPINFO bmi;
    unsigned int *overlay = NULL;
    size_t pixel_count = (size_t)chat_width * (size_t)height;
    int font_height;
    int padding_x;
    int emote_scale_key = (int)(emote_scale * 1000.0f + 0.5f);
    RECT bounds;
    int cursor_y;
    int visible_start = message_count;
    int visible_count = 0;
    unsigned int animation_key;
    int i;
    for (i = 0; i < CHAT_RENDER_CACHE_COUNT; i++) {
        webm_twitch_chat_render_cache_t *candidate = &session->render_cache[i];
        if (candidate->width == chat_width && candidate->height == height &&
            candidate->text_size == text_size &&
            candidate->horizontal_padding == horizontal_padding &&
            candidate->emote_scale_key == (emotes_enabled ? emote_scale_key : 0)) {
            cache = candidate;
            break;
        }
        if (!cache || !candidate->pixels || candidate->last_used < cache->last_used) {
            cache = candidate;
        }
    }
    if (!cache) return NULL;
    padding_x = horizontal_padding;
    if (padding_x < 0) padding_x = 0;
    if (padding_x > (chat_width - 8) / 2) padding_x = (chat_width - 8) / 2;
    cache->last_used = ++session->render_serial;
    animation_key = visible_animation_key_locked(
        session, messages, message_start,
        cache->visible_start, cache->visible_count, now,
        animated_enabled, animation_fps);
    if (cache->pixels && cache->pixel_count == pixel_count &&
        cache->width == chat_width && cache->height == height &&
        cache->text_size == text_size &&
        cache->horizontal_padding == horizontal_padding &&
        cache->emote_scale_key == (emotes_enabled ? emote_scale_key : 0) &&
        cache->message_serial == message_serial &&
        cache->status_serial == status_serial &&
        cache->animation_key == animation_key) {
        *content_left = cache->content_left;
        *content_top = cache->content_top;
        *content_right = cache->content_right;
        *content_bottom = cache->content_bottom;
        return cache->pixels;
    }
    if (cache->pixel_count != pixel_count) {
        unsigned int *grown = (unsigned int*)realloc(cache->pixels,
                                                     pixel_count * sizeof(unsigned int));
        if (!grown) return NULL;
        cache->pixels = grown;
        cache->pixel_count = pixel_count;
    }
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = chat_width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    dc = CreateCompatibleDC(NULL);
    if (!dc) return NULL;
    bitmap = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, (void**)&overlay, NULL, 0);
    if (!bitmap || !overlay) {
        if (bitmap) DeleteObject(bitmap);
        DeleteDC(dc);
        return NULL;
    }
    memset(overlay, 0, pixel_count * sizeof(unsigned int));
    old_bitmap = SelectObject(dc, bitmap);
    font_height = text_size;
    if (font_height <= 0) {
        font_height = height / 34;
        if (font_height < 10) font_height = 10;
        if (font_height > 28) font_height = 28;
    }
    font = CreateFontW(-font_height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    old_font = SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    bounds.left = padding_x;
    bounds.right = chat_width - padding_x;
    cursor_y = height - CHAT_PADDING_Y;
    if (message_count <= 0) {
        wchar_t wide[512];
        RECT rect = { padding_x, CHAT_PADDING_Y,
                      chat_width - padding_x, height - CHAT_PADDING_Y };
        utf8_to_wide(status && status[0] ? status :
                     (connected ? "Waiting for chat..." : "Connecting to chat..."),
                     wide, (int)(sizeof(wide) / sizeof(wide[0])));
        SetTextColor(dc, RGB(210, 210, 210));
        DrawTextW(dc, wide, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);
    } else {
        for (i = message_count - 1; i >= 0 && cursor_y > CHAT_PADDING_Y; i--) {
            const webm_twitch_chat_message_t *message =
                &messages[(message_start + i) % CHAT_MESSAGE_COUNT];
            char combined[CHAT_NAME_MAX + CHAT_TEXT_MAX + 4];
            wchar_t wide[CHAT_NAME_MAX + CHAT_TEXT_MAX + 4];
            wchar_t wide_name[CHAT_NAME_MAX + 4];
            RECT measure;
            RECT draw;
            if (emotes_enabled && message->has_emotes && message->run_count > 0) {
                int mixed_height = layout_mixed_message(
                    session, dc, NULL, chat_width, height,
                    bounds.left, bounds.right, 0, font_height,
                    emote_scale, message, 0, now,
                    animated_enabled, animation_fps);
                cursor_y -= mixed_height + 4;
                if (cursor_y < CHAT_PADDING_Y) break;
                layout_mixed_message(session, dc, overlay, chat_width, height,
                                     bounds.left, bounds.right, cursor_y, font_height,
                                     emote_scale, message, 1, now,
                                     animated_enabled, animation_fps);
                visible_start = i;
                visible_count = message_count - i;
                continue;
            }
            _snprintf(combined, sizeof(combined) - 1, "%s: %s",
                      message->name, message->text);
            combined[sizeof(combined) - 1] = 0;
            if (!utf8_to_wide(combined, wide, (int)(sizeof(wide) / sizeof(wide[0])))) continue;
            _snprintf(combined, sizeof(combined) - 1, "%s:", message->name);
            combined[sizeof(combined) - 1] = 0;
            utf8_to_wide(combined, wide_name, (int)(sizeof(wide_name) / sizeof(wide_name[0])));
            measure = bounds;
            measure.top = 0;
            measure.bottom = height;
            DrawTextW(dc, wide, -1, &measure,
                      DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_CALCRECT);
            {
                int message_height = measure.bottom - measure.top;
                int available_height = cursor_y - CHAT_PADDING_Y - 4;
                if (available_height <= 0) break;
                if (message_height > available_height) message_height = available_height;
                measure.bottom = measure.top + message_height;
            }
            cursor_y -= (measure.bottom - measure.top) + 4;
            if (cursor_y < CHAT_PADDING_Y) cursor_y = CHAT_PADDING_Y;
            draw = bounds;
            draw.top = cursor_y;
            draw.bottom = cursor_y + (measure.bottom - measure.top);
            SetTextColor(dc, RGB(238, 238, 238));
            DrawTextW(dc, wide, -1, &draw, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);
            SetTextColor(dc, message->color);
            DrawTextW(dc, wide_name, -1, &draw, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
            visible_start = i;
            visible_count = message_count - i;
        }
    }
    /* Text is drawn through GDI while emotes write directly into the DIB bits.
       Flush GDI before caching those bits so mixed messages contain both. */
    GdiFlush();
    memcpy(cache->pixels, overlay, pixel_count * sizeof(unsigned int));
    cache->content_left = padding_x;
    cache->content_right = chat_width - padding_x;
    cache->content_top = message_count > 0 ? cursor_y : CHAT_PADDING_Y;
    if (cache->content_top < CHAT_PADDING_Y) cache->content_top = CHAT_PADDING_Y;
    if (cache->content_top > height) cache->content_top = height;
    cache->content_bottom = height;
    cache->visible_start = visible_start;
    cache->visible_count = visible_count;
    cache->width = chat_width;
    cache->height = height;
    cache->chat_width = chat_width;
    cache->text_size = text_size;
    cache->horizontal_padding = horizontal_padding;
    cache->emote_scale_key = emotes_enabled ? emote_scale_key : 0;
    cache->message_serial = message_serial;
    cache->status_serial = status_serial;
    cache->animation_key = visible_animation_key_locked(
        session, messages, message_start, visible_start, visible_count, now,
        animated_enabled, animation_fps);
    *content_left = cache->content_left;
    *content_top = cache->content_top;
    *content_right = cache->content_right;
    *content_bottom = cache->content_bottom;
    SelectObject(dc, old_font);
    SelectObject(dc, old_bitmap);
    DeleteObject(font);
    DeleteObject(bitmap);
    DeleteDC(dc);
    return cache->pixels;
}

void reference_webm_twitch_chat_compose(webm_twitch_chat_session_t *session,
                              const webm_twitch_settings_t *settings,
                              unsigned char *pixels, int width, int height, int pitch,
                              webm_twitch_chat_pixel_format_t format,
                              int bytes_per_pixel, int flip_text_vertical)
{
    unsigned int *overlay;
    char status[256];
    int message_count;
    int message_start;
    unsigned int message_serial;
    unsigned int status_serial;
    DWORD animation_now;
    int connected;
    int chat_width;
    int chat_left;
    int x0;
    int x, y;
    int content_left = 0;
    int content_top = 0;
    int content_right = 0;
    int content_bottom = 0;
    float opacity;
    if (!session || !settings || !settings->chat_enabled || !pixels ||
        width < 64 || height < 32 || pitch < width * bytes_per_pixel) return;
    chat_width = (int)((float)width * settings->chat_width + 0.5f);
    if (chat_width < 48) chat_width = 48;
    if (chat_width > width - 16) chat_width = width - 16;
    chat_left = settings->chat_position == 0;
    x0 = chat_left ? 0 : width - chat_width;
    opacity = settings->chat_background_opacity;
    if (opacity < 0.0f) opacity = 0.0f;
    if (opacity > 1.0f) opacity = 1.0f;

    EnterCriticalSection(&session->lock);
    if (!settings->chat_overlay) {
        resize_video_for_chat(session, pixels, width, height, pitch, chat_width,
                              chat_left, bytes_per_pixel);
    } else {
        for (y = 0; y < height; y++) {
            unsigned char *row = pixels + (size_t)y * (size_t)pitch;
            for (x = x0; x < x0 + chat_width; x++) {
                unsigned char *pixel = row + (size_t)x * (size_t)bytes_per_pixel;
                int r, g, b;
                pixel_read(pixel, format, bytes_per_pixel, &r, &g, &b);
                pixel_write(pixel, format, bytes_per_pixel,
                            (int)(r * (1.0f - opacity)),
                            (int)(g * (1.0f - opacity)),
                            (int)(b * (1.0f - opacity)));
            }
        }
    }

    message_count = session->message_count;
    message_start = session->message_start;
    message_serial = session->message_serial;
    status_serial = session->status_serial;
    connected = session->connected;
    copy_string(status, sizeof(status), session->status);
    animation_now = GetTickCount();
    overlay = reference_render_chat_overlay(session, session->messages,
                                  message_start, message_count, status, connected,
                                  message_serial, status_serial, animation_now,
                                  chat_width, height,
                                  settings->chat_text_size,
                                  settings->chat_horizontal_padding,
                                  settings->chat_emotes,
                                  settings->chat_emote_scale,
                                  settings->chat_animated_emotes,
                                  settings->chat_animated_emote_fps,
                                  &content_left, &content_top,
                                  &content_right, &content_bottom);
    if (overlay) {
        for (y = content_top; y < content_bottom; y++) {
            int target_y = flip_text_vertical ? height - 1 - y : y;
            unsigned char *row = pixels + (size_t)target_y * (size_t)pitch;
            unsigned int *source = overlay + (size_t)y * (size_t)chat_width;
            for (x = content_left; x < content_right; x++) {
                unsigned int value = source[x];
                int b = value & 255;
                int g = (value >> 8) & 255;
                int r = (value >> 16) & 255;
                int a = (value >> 24) & 255;
                if (a || r > 4 || g > 4 || b > 4) {
                    unsigned char *target = row +
                                            (size_t)(x0 + x) * (size_t)bytes_per_pixel;
                    if (!a) a = 255;
                    if (a < 255) {
                        int dr, dg, db;
                        pixel_read(target, format, bytes_per_pixel, &dr, &dg, &db);
                        r = (r * a + dr * (255 - a) + 127) / 255;
                        g = (g * a + dg * (255 - a) + 127) / 255;
                        b = (b * a + db * (255 - a) + 127) / 255;
                    }
                    pixel_write(target, format, bytes_per_pixel, r, g, b);
                }
            }
        }
    }
    LeaveCriticalSection(&session->lock);
}

void reference_webm_twitch_chat_downsample_half(const unsigned char *source,
                                      int source_width, int source_height, int source_pitch,
                                      unsigned char *target,
                                      int target_width, int target_height, int target_pitch,
                                      int bytes_per_pixel)
{
    int x, y, channel;
    if (!source || !target || source_width < 1 || source_height < 1 ||
        target_width < 1 || target_height < 1 || bytes_per_pixel < 2 ||
        source_pitch < source_width * bytes_per_pixel ||
        target_pitch < target_width * bytes_per_pixel) return;
    for (y = 0; y < target_height; y++) {
        int source_y0 = y * 2;
        int source_y1;
        const unsigned char *row0;
        const unsigned char *row1;
        unsigned char *dst = target + (size_t)y * (size_t)target_pitch;
        if (source_y0 >= source_height) source_y0 = source_height - 1;
        source_y1 = source_y0 + 1;
        if (source_y1 >= source_height) source_y1 = source_y0;
        row0 = source + (size_t)source_y0 * (size_t)source_pitch;
        row1 = source + (size_t)source_y1 * (size_t)source_pitch;
        for (x = 0; x < target_width; x++) {
            int source_x0 = x * 2;
            int source_x1;
            const unsigned char *p00;
            if (source_x0 >= source_width) source_x0 = source_width - 1;
            source_x1 = source_x0 + 1;
            if (source_x1 >= source_width) source_x1 = source_x0;
            p00 = row0 + (size_t)source_x0 * (size_t)bytes_per_pixel;
            if (bytes_per_pixel == 2) {
                dst[0] = p00[0];
                dst[1] = p00[1];
            } else {
                const unsigned char *p10 = row0 +
                    (size_t)source_x1 * (size_t)bytes_per_pixel;
                const unsigned char *p01 = row1 +
                    (size_t)source_x0 * (size_t)bytes_per_pixel;
                const unsigned char *p11 = row1 +
                    (size_t)source_x1 * (size_t)bytes_per_pixel;
                if (bytes_per_pixel == 4) {
                    unsigned int v00, v10, v01, v11;
                    unsigned int rb_sum, ga_sum, result;
                    memcpy(&v00, p00, sizeof(v00));
                    memcpy(&v10, p10, sizeof(v10));
                    memcpy(&v01, p01, sizeof(v01));
                    memcpy(&v11, p11, sizeof(v11));
                    rb_sum = (v00 & 0x00FF00FFu) + (v10 & 0x00FF00FFu) +
                             (v01 & 0x00FF00FFu) + (v11 & 0x00FF00FFu) +
                             0x00020002u;
                    ga_sum = ((v00 >> 8) & 0x00FF00FFu) +
                             ((v10 >> 8) & 0x00FF00FFu) +
                             ((v01 >> 8) & 0x00FF00FFu) +
                             ((v11 >> 8) & 0x00FF00FFu) + 0x00020002u;
                    result = ((rb_sum >> 2) & 0x00FF00FFu) |
                             (((ga_sum >> 2) & 0x00FF00FFu) << 8);
                    memcpy(dst, &result, sizeof(result));
                } else {
                    for (channel = 0; channel < bytes_per_pixel; channel++) {
                        dst[channel] = (unsigned char)(((unsigned int)p00[channel] +
                                                        (unsigned int)p10[channel] +
                                                        (unsigned int)p01[channel] +
                                                        (unsigned int)p11[channel] + 2u) / 4u);
                    }
                }
            }
            dst += bytes_per_pixel;
        }
    }
}
