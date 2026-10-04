/* Installed first SIMD candidate, retained for differential benchmarks. */
static int convert_rgb24_to_d3d8_first_simd(BYTE *dst_base, int dst_pitch, int format,
                                  int width, int height, const BYTE *source_frame,
                                  int source_width, int source_height, int source_stride)
{
    int offsets[4096];
    int x, y, sx = 0, error = 0, previous_sy = -1;
    int step, remainder;
    int bpp = d3d8_native_format_bpp(format);
    if (!dst_base || !source_frame || bpp <= 0 || width <= 0 || height <= 0 ||
        source_width <= 0 || source_height <= 0 || source_stride <= 0 ||
        dst_pitch < width * bpp) return 0;
    if (width > (int)(sizeof(offsets) / sizeof(offsets[0]))) {
        return convert_rgb24_to_d3d8_scalar(dst_base, dst_pitch, format, width, height,
                                            source_frame, source_width, source_height,
                                            source_stride);
    }
    /* Calculate horizontal sampling once, instead of once for every row. */
    step = source_width / width;
    remainder = source_width % width;
    for (x = 0; x < width; x++) {
        offsets[x] = sx * 3;
        sx += step;
        error += remainder;
        if (error >= width) {
            sx++;
            error -= width;
        }
    }
    if (bpp == 4 && width >= 64 && height >= 16 && source_width >= 6 &&
        source_width <= width && webm_rgb_has_ssse3() &&
        webm_rgb_shuffle_convert(dst_base, dst_pitch, format, width, height,
                                 source_frame, source_width, source_height,
                                 source_stride, offsets)) return 1;
    for (y = 0; y < height; y++) {
        BYTE *dst = dst_base + (size_t)y * (size_t)dst_pitch;
        int sy = ((height - 1 - y) * source_height) / height;
        const BYTE *row = source_frame + (size_t)sy * (size_t)source_stride;
        if (sy == previous_sy) {
            memcpy(dst, dst - dst_pitch, (size_t)width * (size_t)bpp);
            continue;
        }
        previous_sy = sy;
        /* Keep format decisions out of the pixel loop. memcpy stores permit
         * unaligned pitches without type-punning or alignment assumptions. */
        switch (format) {
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8:
            for (x = 0; x < width; x++) {
                const BYTE *src = row + offsets[x];
                DWORD pixel = 0xff000000u | ((DWORD)src[0] << 16) |
                              ((DWORD)src[1] << 8) | (DWORD)src[2];
                memcpy(dst + x * 4, &pixel, sizeof(pixel));
            }
            break;
        case WEBM_CACHE_FORMAT_GL_RGBA:
            for (x = 0; x < width; x++) {
                const BYTE *src = row + offsets[x];
                DWORD pixel = 0xff000000u | ((DWORD)src[2] << 16) |
                              ((DWORD)src[1] << 8) | (DWORD)src[0];
                memcpy(dst + x * 4, &pixel, sizeof(pixel));
            }
            break;
        case D3DFMT_R5G6B5:
            for (x = 0; x < width; x++) {
                const BYTE *src = row + offsets[x];
                WORD pixel = (WORD)(((src[0] >> 3) << 11) |
                                    ((src[1] >> 2) << 5) | (src[2] >> 3));
                memcpy(dst + x * 2, &pixel, sizeof(pixel));
            }
            break;
        case WEBM_CACHE_FORMAT_GL_RGB:
            if (width == source_width) {
                memcpy(dst, row, (size_t)width * 3u);
            } else {
                for (x = 0; x < width; x++) {
                    memcpy(dst + x * 3, row + offsets[x], 3);
                }
            }
            break;
        default: /* D3DFMT_R8G8B8 */
            for (x = 0; x < width; x++) {
                const BYTE *src = row + offsets[x];
                dst[x * 3] = src[2];
                dst[x * 3 + 1] = src[1];
                dst[x * 3 + 2] = src[0];
            }
            break;
        }
    }
    return 1;
}

