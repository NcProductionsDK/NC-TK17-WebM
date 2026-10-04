/* Four RGB24 samples per shuffle, using the converter's existing coordinates.
 * No load crosses the source row, including tightly packed final rows. */
#ifndef WEBM_RGB_SIMD_H
#define WEBM_RGB_SIMD_H
#include <tmmintrin.h>
#include <cpuid.h>

static volatile LONG webm_rgb_ssse3_state;

static int webm_rgb_has_ssse3(void)
{
    LONG state = InterlockedCompareExchange(&webm_rgb_ssse3_state, 0, 0);
    if (!state) {
        unsigned int a, b, c, d;
        state = (__get_cpuid(1, &a, &b, &c, &d) && (c & bit_SSSE3)) ? 1 : -1;
        InterlockedExchange(&webm_rgb_ssse3_state, state);
    }
    return state > 0;
}

__attribute__((target("ssse3")))
static int webm_rgb_shuffle_convert(BYTE *dst_base, int dst_pitch, int format,
                                    int width, int height, const BYTE *source,
                                    int source_width, int source_height,
                                    int source_stride, const int *offsets)
{
    struct shuffle_block { __m128i mask; int offset; } blocks[1024];
    const __m128i alpha = _mm_set1_epi32((int)0xff000000u);
    int count = width / 4;
    int rgb = format == WEBM_CACHE_FORMAT_GL_RGBA;
    int i, y, previous_sy = -1;
    int last_load = source_width * 3 - 16;
    if (last_load < 0 || count > 1024) return 0;

    /* Prepare once for all rows. Widely spaced downscaling retains the
     * original converter instead of adding gathers or changing filtering. */
    for (i = 0; i < count; i++) {
        BYTE mask[16];
        int j, base = offsets[i * 4];
        if (base > last_load) base = last_load;
        if (offsets[i * 4 + 3] - base > 13) return 0;
        blocks[i].offset = base;
        for (j = 0; j < 4; j++) {
            int relative = offsets[i * 4 + j] - base;
            mask[j * 4] = (BYTE)(relative + (rgb ? 0 : 2));
            mask[j * 4 + 1] = (BYTE)(relative + 1);
            mask[j * 4 + 2] = (BYTE)(relative + (rgb ? 2 : 0));
            mask[j * 4 + 3] = 0x80;
        }
        blocks[i].mask = _mm_loadu_si128((const __m128i*)mask);
    }
    for (y = 0; y < height; y++) {
        BYTE *dst = dst_base + (size_t)y * (size_t)dst_pitch;
        int sy = ((height - 1 - y) * source_height) / height;
        const BYTE *row = source + (size_t)sy * (size_t)source_stride;
        int x;
        if (sy == previous_sy) {
            memcpy(dst, dst - dst_pitch, (size_t)width * 4u);
            continue;
        }
        previous_sy = sy;
        for (i = 0; i < count; i++) {
            __m128i input = _mm_loadu_si128((const __m128i*)(row + blocks[i].offset));
            __m128i output = _mm_or_si128(_mm_shuffle_epi8(input, blocks[i].mask), alpha);
            _mm_storeu_si128((__m128i*)(dst + i * 16), output);
        }
        for (x = count * 4; x < width; x++) {
            const BYTE *pixel = row + offsets[x];
            dst[x * 4] = pixel[rgb ? 0 : 2];
            dst[x * 4 + 1] = pixel[1];
            dst[x * 4 + 2] = pixel[rgb ? 2 : 0];
            dst[x * 4 + 3] = 255;
        }
    }
    return 1;
}
/* Downscaling may put successive samples too far apart for one RGB load.
 * Gather four unaligned words, then reorder their channels in one shuffle.
 * The last RGB pixel is loaded starting one byte earlier, with a matching
 * shuffle offset, so even an unpadded source never requires an extra byte. */
__attribute__((target("ssse3")))
static int webm_rgb_gather_convert(BYTE *dst_base, int dst_pitch, int format,
                                   int width, int height, const BYTE *source,
                                   int source_width, int source_height,
                                   int source_stride, const int *offsets)
{
    struct gather_block { __m128i mask; int offsets[4]; } blocks[1024];
    const __m128i alpha = _mm_set1_epi32((int)0xff000000u);
    int count = width / 4;
    int rgb = format == WEBM_CACHE_FORMAT_GL_RGBA;
    int last_load = source_width * 3 - 4;
    int i, y, previous_sy = -1;
    if (last_load < 0 || count > 1024) return 0;
    for (i = 0; i < count; i++) {
        BYTE mask[16];
        int j;
        for (j = 0; j < 4; j++) {
            int offset = offsets[i * 4 + j];
            int base = offset > last_load ? last_load : offset;
            int relative = j * 4 + offset - base;
            blocks[i].offsets[j] = base;
            mask[j * 4] = (BYTE)(relative + (rgb ? 0 : 2));
            mask[j * 4 + 1] = (BYTE)(relative + 1);
            mask[j * 4 + 2] = (BYTE)(relative + (rgb ? 2 : 0));
            mask[j * 4 + 3] = 0x80;
        }
        blocks[i].mask = _mm_loadu_si128((const __m128i*)mask);
    }
    for (y = 0; y < height; y++) {
        BYTE *dst = dst_base + (size_t)y * (size_t)dst_pitch;
        int sy = ((height - 1 - y) * source_height) / height;
        const BYTE *row = source + (size_t)sy * (size_t)source_stride;
        int x;
        if (sy == previous_sy) {
            memcpy(dst, dst - dst_pitch, (size_t)width * 4u);
            continue;
        }
        previous_sy = sy;
        for (i = 0; i < count; i++) {
            int a, b, c, d;
            __m128i input, output;
            memcpy(&a, row + blocks[i].offsets[0], 4);
            memcpy(&b, row + blocks[i].offsets[1], 4);
            memcpy(&c, row + blocks[i].offsets[2], 4);
            memcpy(&d, row + blocks[i].offsets[3], 4);
            input = _mm_setr_epi32(a, b, c, d);
            output = _mm_or_si128(_mm_shuffle_epi8(input, blocks[i].mask), alpha);
            _mm_storeu_si128((__m128i*)(dst + i * 16), output);
        }
        for (x = count * 4; x < width; x++) {
            const BYTE *pixel = row + offsets[x];
            dst[x * 4] = pixel[rgb ? 0 : 2];
            dst[x * 4 + 1] = pixel[1];
            dst[x * 4 + 2] = pixel[rgb ? 2 : 0];
            dst[x * 4 + 3] = 255;
        }
    }
    return 1;
}
#endif
