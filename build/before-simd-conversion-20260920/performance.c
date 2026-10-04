/* Exercise the production static routines without loading the game/hooks. */
#include <stdlib.h>
static int reject_realloc;
static void *test_realloc(void *ptr, size_t bytes)
{
    return reject_realloc ? NULL : realloc(ptr, bytes);
}
#define realloc test_realloc
#include "../NC-TK17-WebM.c"
#undef realloc
#include "reference_playback.h"

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static unsigned random_state = 1234567;
static unsigned random_value(void)
{
    random_state = random_state * 1664525u + 1013904223u;
    return random_state;
}

static void check_conversion(int sw, int sh, int w, int h, int format, int padding)
{
    int bpp = d3d8_native_format_bpp(format);
    int stride = sw * 3 + padding;
    int pitch = w * bpp + padding;
    size_t source_size = (size_t)stride * sh;
    size_t size = (size_t)pitch * h + 32;
    BYTE *src = (BYTE*)malloc(source_size);
    BYTE *expected = (BYTE*)malloc(size);
    BYTE *actual = (BYTE*)malloc(size);
    size_t i;
    CHECK(src && expected && actual);
    for (i = 0; i < source_size; i++) src[i] = (BYTE)(random_value() >> 24);
    memset(expected, 0xCD, size);
    memset(actual, 0xCD, size);
    CHECK(convert_rgb24_to_d3d8_scalar(expected + 1, pitch, format, w, h, src, sw, sh, stride));
    CHECK(convert_rgb24_to_d3d8(actual + 1, pitch, format, w, h, src, sw, sh, stride));
    CHECK(memcmp(expected, actual, size) == 0); /* Includes padding and guards. */
    free(src); free(expected); free(actual);
}

static video_decoder_t *make_decoder(int live)
{
    video_decoder_t *dec = (video_decoder_t*)calloc(1, sizeof(*dec));
    CHECK(dec);
    InitializeCriticalSection(&dec->async_frame_lock);
    InitializeCriticalSection(&dec->live_queue_lock);
    dec->async_lock_initialized = dec->live_queue_lock_initialized = 1;
    dec->async_enabled = 1;
    dec->async_decoded_frame_index = -1;
    dec->network_source = live;
    dec->live_buffer_ms = live ? 300 : 0;
    dec->fps = 60.0;
    return dec;
}

static void set_frame(video_decoder_t *dec, int index, int w, int h)
{
    long size = w * h * 3;
    BYTE *frame = (BYTE*)realloc(dec->frame, size);
    CHECK(frame);
    dec->frame = frame;
    dec->frame_size = size;
    dec->width = w;
    dec->height = h;
    dec->stride = w * 3;
    dec->decoded_frame_index = index;
    dec->current_frame_ms = index * (1000.0 / 60.0);
    memset(frame, (BYTE)index, size);
}

static void dispose_decoder(video_decoder_t *dec)
{
    int i;
    free(dec->frame);
    free(dec->async_frame);
    free(dec->live_present_frame);
    free(dec->async_d3d8_cache[0]);
    free(dec->async_d3d8_cache[1]);
    for (i = 0; i < WEBM_LIVE_QUEUE_CAPACITY; i++) free(dec->live_queue[i].data);
    DeleteCriticalSection(&dec->async_frame_lock);
    DeleteCriticalSection(&dec->live_queue_lock);
    free(dec);
}

static void compare_presented(video_decoder_t *a, video_decoder_t *b)
{
    CHECK(a->live_queue_count == b->live_queue_count);
    CHECK(a->live_queue_head == b->live_queue_head);
    CHECK(a->async_decoded_frame_index == b->async_decoded_frame_index);
    CHECK(a->live_presented_frame_index == b->live_presented_frame_index);
    CHECK(a->async_width == b->async_width && a->async_height == b->async_height);
    CHECK(a->async_stride == b->async_stride);
    CHECK(a->async_looped == b->async_looped && a->looped == b->looped);
    if (a->async_frame) {
        CHECK(b->async_frame);
        CHECK(memcmp(a->async_frame, b->async_frame,
                     (size_t)a->async_stride * a->async_height) == 0);
    } else CHECK(!b->async_frame);
    CHECK(a->async_d3d8_cache_ready == b->async_d3d8_cache_ready);
    if (a->async_d3d8_cache_ready) {
        int level;
        CHECK(a->async_d3d8_cache_ready_levels == b->async_d3d8_cache_ready_levels);
        for (level = 0; level < a->async_d3d8_cache_ready_levels; level++) {
            CHECK(memcmp(a->async_d3d8_cache[a->async_d3d8_cache_front] + a->async_d3d8_cache_level_offset[level],
                         b->async_d3d8_cache[b->async_d3d8_cache_front] + b->async_d3d8_cache_level_offset[level],
                         (size_t)a->async_d3d8_cache_level_pitch[level] * a->async_d3d8_cache_level_height[level]) == 0);
        }
    }
}

static void test_publication(void)
{
    video_decoder_t *a = make_decoder(0), *b = make_decoder(0);
    int i;
    for (i = 0; i < 160; i++) {
        int index = (i / 2) % 20; /* Repeated frames and loop index reuse. */
        int w = i < 80 ? 65 : 99, h = i < 80 ? 33 : 51;
        set_frame(a, index, w, h); set_frame(b, index, w, h);
        if (i == 40 || i == 120) a->looped = b->looped = 1;
        /* Configuration changes on an unchanged frame must rebuild caches. */
        if (i == 1 || i == 31 || i == 81) {
            int format = i == 31 ? WEBM_CACHE_FORMAT_GL_RGBA : D3DFMT_X8R8G8B8;
            video_decoder_configure_d3d8_cache(a, w, h, format, 5, NULL, NULL);
            video_decoder_configure_d3d8_cache(b, w, h, format, 5, NULL, NULL);
        }
        video_decoder_async_publish_frame(a, index);
        reference_video_decoder_async_publish_frame(b, index);
        compare_presented(a, b);
    }
    /* A single-frame loop can change content without changing its index. */
    a->looped = b->looped = 1;
    memset(a->frame, 0xAA, a->frame_size); memset(b->frame, 0xAA, b->frame_size);
    video_decoder_async_publish_frame(a, a->decoded_frame_index);
    reference_video_decoder_async_publish_frame(b, b->decoded_frame_index);
    compare_presented(a, b);
    dispose_decoder(a); dispose_decoder(b);
}

static void test_live_queue(void)
{
    video_decoder_t *a = make_decoder(1), *b = make_decoder(1);
    int i;
    CHECK(!video_decoder_live_prepare_frame(a, 100));
    CHECK(!video_decoder_live_prepare_frame(a, 400));
    video_decoder_configure_d3d8_cache(a, 97, 49, D3DFMT_X8R8G8B8, 5, NULL, NULL);
    video_decoder_configure_d3d8_cache(b, 97, 49, D3DFMT_X8R8G8B8, 5, NULL, NULL);
    for (i = 0; i < 1000; i++) {
        int w = (i / 60) % 2 ? 97 : 63, h = w / 2;
        set_frame(a, i, w, h); set_frame(b, i, w, h);
        video_decoder_live_queue_push(a); video_decoder_live_queue_push(b);
        /* Bursts include full-queue drops, wraparound and skipped frames. */
        if (i % 45 == 0 || (i % 100 > 50 && i % 3 == 0)) {
            DWORD target = (DWORD)a->current_frame_ms + 300 - (i % 11) * 13;
            CHECK(video_decoder_live_prepare_frame(a, target) ==
                  reference_video_decoder_live_prepare_frame(b, target));
            compare_presented(a, b);
        }
    }
    CHECK(video_decoder_live_prepare_frame(a, 100000) ==
          reference_video_decoder_live_prepare_frame(b, 100000));
    compare_presented(a, b);
    dispose_decoder(a); dispose_decoder(b);
}

static void test_memory_fallback(void)
{
    video_decoder_t *dec = make_decoder(0);
    BYTE *front;
    set_frame(dec, 1, 65, 33);
    video_decoder_async_publish_frame(dec, 1);
    front = dec->async_frame;
    CHECK(front && !dec->live_present_frame);
    set_frame(dec, 2, 65, 33);
    reject_realloc = 1;
    video_decoder_async_publish_frame(dec, 2);
    reject_realloc = 0;
    CHECK(dec->async_frame == front && dec->async_decoded_frame_index == 2);
    CHECK(memcmp(dec->async_frame, dec->frame, dec->frame_size) == 0);
    dispose_decoder(dec);
    puts("Spare-buffer allocation failure retains original playback fallback");
}

typedef struct { video_decoder_t *dec; volatile LONG done; } stress_t;
static DWORD WINAPI publish_stress(void *arg)
{
    stress_t *s = (stress_t*)arg;
    int i;
    for (i = 1; i <= 1500; i++) {
        set_frame(s->dec, i, 127, 65);
        video_decoder_async_publish_frame(s->dec, i);
        if ((i & 15) == 0) SwitchToThread();
    }
    InterlockedExchange(&s->done, 1);
    return 0;
}

static void test_concurrent_publication(void)
{
    stress_t s = { make_decoder(0), 0 };
    HANDLE thread;
    int reads = 0;
    video_decoder_configure_d3d8_cache(s.dec, 127, 65, WEBM_CACHE_FORMAT_GL_RGBA, 1, NULL, NULL);
    thread = CreateThread(NULL, 0, publish_stress, &s, 0, NULL);
    CHECK(thread);
    do {
        d3d8_cached_mip_view_t view;
        int index, pinned;
        size_t j;
        EnterCriticalSection(&s.dec->async_frame_lock);
        index = s.dec->async_decoded_frame_index;
        if (s.dec->async_frame) {
            for (j = 0; j < (size_t)s.dec->async_stride * s.dec->async_height; j++)
                CHECK(s.dec->async_frame[j] == (BYTE)index);
        }
        pinned = video_decoder_pin_d3d8_cached_mip(s.dec, 0, 127, 65, WEBM_CACHE_FORMAT_GL_RGBA, &view);
        LeaveCriticalSection(&s.dec->async_frame_lock);
        if (pinned) {
            SwitchToThread(); /* Producer may publish while this buffer is pinned. */
            for (j = 0; j < 127u * 65u * 4u; j++)
                CHECK(view.pixels[j] == ((j & 3) == 3 ? 255 : (BYTE)index));
            video_decoder_unpin_d3d8_cached_mip(s.dec, &view);
            reads++;
        }
    } while (!InterlockedCompareExchange(&s.done, 0, 0));
    CHECK(WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0);
    CloseHandle(thread);
    CHECK(reads > 0);
    {
        BYTE pixels[127 * 65 * 4];
        CHECK(video_decoder_copy_gl_cached_frame(s.dec, pixels, sizeof(pixels), 127, 65, 0x1908, 0x1401));
        CHECK(pixels[0] == (BYTE)1500 && pixels[3] == 255);
        CHECK(!video_decoder_copy_gl_cached_frame(s.dec, pixels, sizeof(pixels)-1, 127, 65, 0x1908, 0x1401));
        CHECK(!s.dec->async_d3d8_cache_readers[0] && !s.dec->async_d3d8_cache_readers[1]);
    }
    printf("Concurrent publication: %d pinned reads passed\n", reads);
    dispose_decoder(s.dec);
}

static DWORD WINAPI live_stress(void *arg)
{
    stress_t *s = (stress_t*)arg;
    int i;
    for (i = 1; i <= 5000; i++) {
        set_frame(s->dec, i, (i / 100) % 2 ? 97 : 63, 49);
        video_decoder_live_queue_push(s->dec);
        if ((i & 15) == 0) SwitchToThread();
    }
    InterlockedExchange(&s->done, 1);
    return 0;
}

static void test_concurrent_live_queue(void)
{
    stress_t s = { make_decoder(1), 0 };
    HANDLE thread = CreateThread(NULL, 0, live_stress, &s, 0, NULL);
    int reads = 0;
    CHECK(thread);
    do {
        if (video_decoder_live_prepare_frame(s.dec, 1000000)) {
            size_t j;
            BYTE value = (BYTE)s.dec->async_decoded_frame_index;
            for (j = 0; j < (size_t)s.dec->async_stride * s.dec->async_height; j++)
                CHECK(s.dec->async_frame[j] == value);
            reads++;
        }
        SwitchToThread();
    } while (!InterlockedCompareExchange(&s.done, 0, 0));
    CHECK(WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0);
    CloseHandle(thread);
    video_decoder_live_prepare_frame(s.dec, 1000000);
    CHECK(s.dec->async_decoded_frame_index == 5000);
    {
        BYTE *owners[WEBM_LIVE_QUEUE_CAPACITY + 3];
        int i, j;
        owners[0] = s.dec->frame;
        owners[1] = s.dec->async_frame;
        owners[2] = s.dec->live_present_frame;
        for (i = 0; i < WEBM_LIVE_QUEUE_CAPACITY; i++) owners[i + 3] = s.dec->live_queue[i].data;
        for (i = 0; i < WEBM_LIVE_QUEUE_CAPACITY + 3; i++)
            for (j = 0; j < i; j++) CHECK(!owners[i] || owners[i] != owners[j]);
    }
    printf("Concurrent Twitch buffering: %d presentations, unique buffer ownership passed\n", reads);
    dispose_decoder(s.dec);
}

typedef int (*convert_fn)(BYTE*, int, int, int, int, const BYTE*, int, int, int);
static double benchmark(convert_fn fn, BYTE *dst, BYTE *src, int sw, int sh, int w, int h, int format)
{
    LARGE_INTEGER frequency, start, end;
    int i;
    QueryPerformanceFrequency(&frequency);
    for (i = 0; i < 3; i++) fn(dst, w * 4, format, w, h, src, sw, sh, sw * 3);
    QueryPerformanceCounter(&start);
    for (i = 0; i < 40; i++) fn(dst, w * 4, format, w, h, src, sw, sh, sw * 3);
    QueryPerformanceCounter(&end);
    return (double)(end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart / 40.0;
}

int main(void)
{
    int formats[] = {D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_R8G8B8,
                     D3DFMT_R5G6B5, WEBM_CACHE_FORMAT_GL_RGB, WEBM_CACHE_FORMAT_GL_RGBA};
    int cases[][4] = {{3840,2160,2048,1152}, {1920,1080,2048,1024},
                     {1280,720,2048,1152}, {1920,1080,1920,1080}};
    int i, f;
    for (f = 0; f < 6; f++) {
        check_conversion(1, 1, 1, 1, formats[f], 0);
        check_conversion(1, 1, 127, 65, formats[f], 7);
        check_conversion(31, 17, 4097, 3, formats[f], 1); /* scalar fallback */
        for (i = 0; i < 350; i++) {
            int sw = 1 + random_value() % 157, sh = 1 + random_value() % 97;
            int w = 1 + random_value() % 179, h = 1 + random_value() % 109;
            check_conversion(sw, sh, w, h, formats[f], i % 11);
        }
        for (i = 0; i < 5; i++)
            check_conversion(3840, 2160, 2048 >> i, 1152 >> i, formats[f], i);
    }
    puts("Conversion: 2148 byte-exact cases passed (6 formats, padding, scaling, mips)");
    test_publication();
    test_live_queue();
    puts("Publication and Twitch queue match the original routines");
    test_memory_fallback();
    test_concurrent_publication();
    test_concurrent_live_queue();
    for (i = 0; i < 4; i++) {
        int sw = cases[i][0], sh = cases[i][1], w = cases[i][2], h = cases[i][3];
        BYTE *src = (BYTE*)malloc((size_t)sw * sh * 3);
        BYTE *dst = (BYTE*)malloc((size_t)w * h * 4);
        double old_ms, new_ms;
        CHECK(src && dst);
        memset(src, 123, (size_t)sw * sh * 3);
        old_ms = benchmark(convert_rgb24_to_d3d8_scalar, dst, src, sw, sh, w, h, D3DFMT_X8R8G8B8);
        new_ms = benchmark(convert_rgb24_to_d3d8, dst, src, sw, sh, w, h, D3DFMT_X8R8G8B8);
        printf("BGRA %dx%d -> %dx%d: old %.3f ms, new %.3f ms, %.2fx\n",
               sw, sh, w, h, old_ms, new_ms, old_ms / new_ms);
        free(src); free(dst);
    }
    return 0;
}
