#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
static DWORD chat_test_now = 10000;
static DWORD chat_test_tick(void) { return chat_test_now; }
static int reject_chat_realloc;
static void *chat_test_realloc(void *p, size_t n)
{
    return reject_chat_realloc ? NULL : realloc(p, n);
}
#define GetTickCount chat_test_tick
#define realloc chat_test_realloc
#include "../webm_twitch_chat.c"
#undef realloc
#include "reference_chat.h"
#undef GetTickCount

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static unsigned rng = 12345;
static unsigned next_random(void) { rng = rng * 1664525u + 1013904223u; return rng; }

static webm_twitch_chat_session_t *make_session(void)
{
    webm_twitch_chat_session_t *s = (webm_twitch_chat_session_t*)calloc(1, sizeof(*s));
    int i;
    CHECK(s);
    InitializeCriticalSection(&s->lock);
    s->references = 1;
    strcpy(s->status, "Waiting for chat...");
    s->message_serial = s->status_serial = 1;
    for (i = 0; i < CHAT_MESSAGE_COUNT; i++) {
        webm_twitch_chat_message_t *m = &s->messages[i];
        sprintf(m->name, "Viewer%d", i);
        strcpy(m->text, "Some wrapping chat text with punctuation & emoji alternatives. UTF-8: \xc3\xa6\xc3\xb8\xc3\xa5");
        m->color = RGB((i * 23) % 256, (i * 71) % 256, (i * 91) % 256);
        if (i % 3 == 0) {
            m->has_emotes = 1;
            m->run_count = 3;
            m->runs[0].type = CHAT_RUN_TEXT;
            strcpy(m->runs[0].text, "Animated ");
            m->runs[1].type = CHAT_RUN_EMOTE;
            strcpy(m->runs[1].emote_id, "test-emote");
            m->runs[2].type = CHAT_RUN_TEXT;
            strcpy(m->runs[2].text, " after the emote");
        }
    }
    {
        webm_twitch_chat_emote_t *e = &s->emotes[0];
        strcpy(e->id, "test-emote");
        e->state = CHAT_EMOTE_READY;
        e->width = e->height = 8;
        e->frame_count = 3;
        e->frame_stride = 8 * 8 * 4;
        e->pixel_bytes = e->frame_stride * e->frame_count;
        e->pixels = (unsigned char*)malloc(e->pixel_bytes);
        e->frame_delays = (DWORD*)malloc(3 * sizeof(DWORD));
        CHECK(e->pixels && e->frame_delays);
        e->frame_delays[0] = 50; e->frame_delays[1] = 100; e->frame_delays[2] = 150;
        e->animation_duration = 300;
        e->animation_start_tick = 9000;
        for (i = 0; i < (int)e->pixel_bytes; i++)
            e->pixels[i] = (i % 4 == 3) ? (unsigned char)((i % 5) * 63) : (unsigned char)(i * 19);
    }
    return s;
}

static webm_twitch_settings_t settings(void)
{
    webm_twitch_settings_t s;
    memset(&s, 0, sizeof(s));
    s.chat_enabled = 1;
    s.chat_overlay = 1;
    s.chat_width = 0.25f;
    s.chat_background_opacity = 0.75f;
    s.chat_text_size = 18;
    s.chat_horizontal_padding = 20;
    s.chat_emotes = 1;
    s.chat_emote_scale = 1.6f;
    s.chat_animated_emotes = 1;
    s.chat_animated_emote_fps = 15;
    return s;
}

__attribute__((noinline))
static void original_dim(unsigned char *pixels, int count,
                         webm_twitch_chat_pixel_format_t format, int bpp, float opacity)
{
    int x;
    for (x = 0; x < count; x++) {
        int r, g, b;
        unsigned char *p = pixels + (size_t)x * bpp;
        pixel_read(p, format, bpp, &r, &g, &b);
        pixel_write(p, format, bpp, (int)(r * (1.0f - opacity)),
                    (int)(g * (1.0f - opacity)), (int)(b * (1.0f - opacity)));
    }
}

static void test_dimming(void)
{
    webm_twitch_chat_session_t *s = make_session();
    unsigned char *a = (unsigned char*)malloc(65536 * 2), *b = (unsigned char*)malloc(65536 * 2);
    int opacity, i;
    CHECK(a && b);
    for (opacity = 0; opacity <= 1000; opacity++) {
        float value = opacity / 1000.0f;
        for (i = 0; i < 65536; i++) { a[i * 2] = (unsigned char)i; a[i * 2 + 1] = (unsigned char)(i >> 8); }
        memcpy(b, a, 65536 * 2);
        original_dim(a, 65536, WEBM_TWITCH_CHAT_RGB565, 2, value);
        dim_chat_background(s, b, 1, 65536 * 2, 0, 65536, WEBM_TWITCH_CHAT_RGB565, 2, value);
        if (memcmp(a, b, 65536 * 2)) {
            for (i=0; i<65536*2 && a[i]==b[i]; i++) {}
            fprintf(stderr,"Dimming mismatch opacity=%d pixel=%d channel=%d old=%u new=%u\n",opacity,i/2,i%2,a[i],b[i]);
            exit(1);
        }
        for (i = 0; i < 1024; i++) a[i] = (unsigned char)i;
        memcpy(b, a, 1024);
        original_dim(a, 256, WEBM_TWITCH_CHAT_BGR, 4, value);
        dim_chat_background(s, b, 1, 1024, 0, 256, WEBM_TWITCH_CHAT_BGR, 4, value);
        CHECK(memcmp(a, b, 1024) == 0);
    }
    free(a); free(b);
    webm_twitch_chat_release(s);
    puts("Dimming: all 65,536 RGB565 values and 256 byte values match at 1,001 opacity levels");
}

static void compare_compose(webm_twitch_chat_session_t *a, webm_twitch_chat_session_t *b,
                             webm_twitch_settings_t *cfg, int w, int h, int bpp,
                             webm_twitch_chat_pixel_format_t format, int flip, int padding)
{
    int pitch = w * bpp + padding;
    size_t n = (size_t)pitch * h + 32, j;
    unsigned char *x = (unsigned char*)malloc(n), *y = (unsigned char*)malloc(n);
    CHECK(x && y);
    for (j = 0; j < n; j++) x[j] = (unsigned char)(next_random() >> 24);
    memcpy(y, x, n);
    webm_twitch_chat_compose(a, cfg, x + 1, w, h, pitch, format, bpp, flip);
    reference_webm_twitch_chat_compose(b, cfg, y + 1, w, h, pitch, format, bpp, flip);
    if (memcmp(x, y, n)) {
        for (j = 0; j < n && x[j] == y[j]; j++) {}
        fprintf(stderr, "Mismatch %dx%d bpp=%d format=%d overlay=%d flip=%d offset=%lu got=%u expected=%u\n",
                w,h,bpp,format,cfg->chat_overlay,flip,(unsigned long)j,x[j],y[j]);
        exit(1);
    }
    free(x); free(y);
}

static void test_composition(void)
{
    webm_twitch_chat_session_t *a = make_session(), *b = make_session();
    int i, j;
    int widths[] = {64, 127, 320, 641, 2048};
    int heights[] = {32, 65, 180, 361, 1152};
    int fonts[] = {0, 8, 18, 64};
    float opacities[] = {-0.2f, 0, 0.001f, 0.333f, 0.755f, 1.0f, 1.2f};
    for (i = 0; i < 360; i++) {
        webm_twitch_settings_t cfg = settings();
        int size = (i / 8) % 5;
        int bpp = i % 5 == 4 ? 2 : i % 2 ? 3 : 4;
        webm_twitch_chat_pixel_format_t format = bpp == 2 ? WEBM_TWITCH_CHAT_RGB565 :
            (i / 2) % 2 ? WEBM_TWITCH_CHAT_RGB : WEBM_TWITCH_CHAT_BGR;
        cfg.chat_overlay = (i / 5) % 2;
        cfg.chat_position = (i / 2) % 2;
        cfg.chat_width = i % 3 ? 0.227f : 0.60f;
        cfg.chat_background_opacity = opacities[i % 7];
        cfg.chat_text_size = fonts[(i / 3) % 4];
        cfg.chat_horizontal_padding = i % 5 ? 20 : 128;
        cfg.chat_emotes = i % 3 != 0;
        cfg.chat_animated_emotes = i % 4 != 0;
        cfg.chat_animated_emote_fps = 1 + i % 30;
        cfg.chat_emote_scale = i % 2 ? 0.5f : 3.0f;
        a->message_count = b->message_count = i % 4 == 0 ? 0 : i % 4 == 1 ? 1 : i % 4 == 2 ? 8 : 64;
        a->message_start = b->message_start = i % 64;
        a->message_serial++; b->message_serial++;
        a->status_serial++; b->status_serial++;
        a->connected = b->connected = i % 2;
        for (j = 0; j < 3; j++) {
            if (j == 2) chat_test_now += 137; /* Animated frame/cache invalidation. */
            compare_compose(a,b,&cfg,widths[size],heights[size],bpp,format,(i / 4) % 2,i % 9);
        }
    }
    /* Force the optional bounds allocation to fail after rendering succeeds. */
    {
        webm_twitch_settings_t cfg = settings();
        compare_compose(a,b,&cfg,320,180,4,WEBM_TWITCH_CHAT_BGR,0,3);
        for (i = 0; i < CHAT_RENDER_CACHE_COUNT; i++) {
            free(a->render_cache[i].row_bounds);
            a->render_cache[i].row_bounds = NULL;
            a->render_cache[i].row_bounds_height = a->render_cache[i].row_bounds_valid = 0;
            a->render_cache[i].row_bounds_seen = 1;
        }
        reject_chat_realloc = 1;
        compare_compose(a,b,&cfg,320,180,4,WEBM_TWITCH_CHAT_BGR,0,3);
        reject_chat_realloc = 0;
    }
    webm_twitch_chat_release(a); webm_twitch_chat_release(b);
    puts("Composition: 1,082 byte-exact cases passed, including text, animated emotes, cache hits, resizing, flips and allocation fallback");
}

typedef void (*compose_fn)(webm_twitch_chat_session_t*, const webm_twitch_settings_t*,
                          unsigned char*,int,int,int,webm_twitch_chat_pixel_format_t,int,int);
static double bench(compose_fn fn, webm_twitch_chat_session_t *s, webm_twitch_settings_t *cfg, int redraw)
{
    LARGE_INTEGER freq,start,end;
    int i;
    unsigned char *frame = (unsigned char*)malloc(2048u * 1152u * 4u);
    CHECK(frame);
    memset(frame,123,2048u * 1152u * 4u);
    fn(s,cfg,frame,2048,1152,2048*4,WEBM_TWITCH_CHAT_BGR,4,1);
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    for (i=0;i<40;i++) {
        if(redraw) s->message_serial++;
        fn(s,cfg,frame,2048,1152,2048*4,WEBM_TWITCH_CHAT_BGR,4,1);
    }
    QueryPerformanceCounter(&end);
    free(frame);
    return (double)(end.QuadPart-start.QuadPart)*1000.0/freq.QuadPart/40.0;
}

typedef struct { webm_twitch_chat_session_t *session; int id; } compose_thread_t;
static DWORD WINAPI compose_worker(void *arg)
{
    compose_thread_t *work=(compose_thread_t*)arg;
    webm_twitch_chat_session_t *reference=make_session();
    webm_twitch_settings_t cfg=settings();
    int i, bpp=work->id ? 2 : 4, pitch=320*bpp;
    webm_twitch_chat_pixel_format_t format=work->id ? WEBM_TWITCH_CHAT_RGB565 : WEBM_TWITCH_CHAT_BGR;
    unsigned char *a=(unsigned char*)malloc((size_t)pitch*180);
    unsigned char *b=(unsigned char*)malloc((size_t)pitch*180);
    CHECK(a&&b);
    reference->message_count=8;
    cfg.chat_overlay=work->id;
    for(i=0;i<80;i++) {
        cfg.chat_text_size=i%2 ? 18 : 0;
        cfg.chat_background_opacity=i%2 ? 0.333f : 0.755f;
        memset(a,(unsigned char)i,(size_t)pitch*180);
        memcpy(b,a,(size_t)pitch*180);
        webm_twitch_chat_compose(work->session,&cfg,a,320,180,pitch,format,bpp,work->id);
        reference_webm_twitch_chat_compose(reference,&cfg,b,320,180,pitch,format,bpp,work->id);
        CHECK(!memcmp(a,b,(size_t)pitch*180));
    }
    free(a);free(b);
    webm_twitch_chat_release(reference);
    return 0;
}

static void test_concurrent_composition(void)
{
    webm_twitch_chat_session_t *session=make_session();
    compose_thread_t work[2]={{session,0},{session,1}};
    HANDLE threads[2];
    session->message_count=8;
    threads[0]=CreateThread(NULL,0,compose_worker,&work[0],0,NULL);
    threads[1]=CreateThread(NULL,0,compose_worker,&work[1],0,NULL);
    CHECK(threads[0]&&threads[1]);
    CHECK(WaitForMultipleObjects(2,threads,TRUE,30000)==WAIT_OBJECT_0);
    CloseHandle(threads[0]); CloseHandle(threads[1]);
    webm_twitch_chat_release(session);
    puts("Concurrent chat composition: 160 byte-exact frames; shared font/table/bounds cleanup passed");
}

typedef void (*mip_fn)(const unsigned char*,int,int,int,unsigned char*,int,int,int,int);
static double bench_mip(mip_fn fn, unsigned char *src, unsigned char *dst)
{
    LARGE_INTEGER start,end,freq;
    int i;
    QueryPerformanceFrequency(&freq);
    fn(src,2048,1152,2048*4,dst,1024,576,1024*4,4);
    QueryPerformanceCounter(&start);
    for(i=0;i<80;i++) fn(src,2048,1152,2048*4,dst,1024,576,1024*4,4);
    QueryPerformanceCounter(&end);
    return (double)(end.QuadPart-start.QuadPart)*1000.0/freq.QuadPart/80.0;
}

static void test_mips(void)
{
    int bpp,i;
    for(bpp=2;bpp<=4;bpp++) for(i=0;i<600;i++) {
        int sw=1+next_random()%129, sh=1+next_random()%99;
        int tw=i%2 ? (sw>1?sw/2:1) : 1+next_random()%131;
        int th=i%2 ? (sh>1?sh/2:1) : 1+next_random()%101;
        int sp=sw*bpp+i%7, tp=tw*bpp+i%9;
        size_t sn=(size_t)sp*sh+32, tn=(size_t)tp*th+32,j;
        unsigned char *src=(unsigned char*)malloc(sn);
        unsigned char *a=(unsigned char*)malloc(tn), *b=(unsigned char*)malloc(tn);
        CHECK(src&&a&&b);
        for(j=0;j<sn;j++) src[j]=(unsigned char)(next_random()>>24);
        memset(a,0xCD,tn); memset(b,0xCD,tn);
        webm_twitch_chat_downsample_half(src+1,sw,sh,sp,a+1,tw,th,tp,bpp);
        reference_webm_twitch_chat_downsample_half(src+1,sw,sh,sp,b+1,tw,th,tp,bpp);
        CHECK(!memcmp(a,b,tn));
        free(src);free(a);free(b);
    }
    puts("Mip preparation: 1,800 byte-exact cases passed (2/3/4-byte formats, clamped edges, padding, unaligned buffers)");
    {
        unsigned char *src=(unsigned char*)malloc(2048u*1152u*4u);
        unsigned char *dst=(unsigned char*)malloc(1024u*576u*4u);
        double old_ms,new_ms;
        CHECK(src&&dst);
        memset(src,123,2048u*1152u*4u);
        old_ms=bench_mip(reference_webm_twitch_chat_downsample_half,src,dst);
        new_ms=bench_mip(webm_twitch_chat_downsample_half,src,dst);
        printf("Chat BGRA mip 2048x1152 -> 1024x576: original %.3f ms, optimized %.3f ms, %.2fx\n",old_ms,new_ms,old_ms/new_ms);
        free(src);free(dst);
    }
}

static double median3(double *v)
{
    double a=v[0], b=v[1], c=v[2], t;
    if(a>b) {t=a;a=b;b=t;}
    if(b>c) {t=b;b=c;c=t;}
    if(a>b) b=a;
    return b;
}

int main(void)
{
    DWORD gdi_before, gdi_after;
    int i;
    /* Warm the process font machinery before checking resource cleanup. */
    HFONT font = CreateFontW(-18,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Arial");
    DeleteObject(font);
    {
        webm_twitch_chat_session_t *a=make_session(), *b=make_session();
        webm_twitch_settings_t cfg=settings();
        compare_compose(a,b,&cfg,320,180,4,WEBM_TWITCH_CHAT_BGR,0,0);
        webm_twitch_chat_release(a); webm_twitch_chat_release(b);
    }
    gdi_before = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    test_dimming();
    test_composition();
    test_concurrent_composition();
    test_mips();
    for (i=0;i<4;i++) {
        webm_twitch_chat_session_t *a=make_session(), *b=make_session();
        webm_twitch_settings_t cfg=settings();
        double old_ms,new_ms, old_trials[3],new_trials[3];
        int trial;
        a->message_count=b->message_count=8;
        cfg.chat_overlay=i%2;
        for(trial=0;trial<3;trial++) {
            if(trial%2) {
                new_trials[trial]=bench(webm_twitch_chat_compose,a,&cfg,i/2);
                old_trials[trial]=bench(reference_webm_twitch_chat_compose,b,&cfg,i/2);
            } else {
                old_trials[trial]=bench(reference_webm_twitch_chat_compose,b,&cfg,i/2);
                new_trials[trial]=bench(webm_twitch_chat_compose,a,&cfg,i/2);
            }
        }
        old_ms=median3(old_trials);
        new_ms=median3(new_trials);
        printf("Chat %s %s: original %.3f ms, optimized %.3f ms, %.2fx\n",
               cfg.chat_overlay?"overlay":"side panel",i/2?"redraw":"cached",old_ms,new_ms,old_ms/new_ms);
        webm_twitch_chat_release(a); webm_twitch_chat_release(b);
    }
    gdi_after=GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    printf("GDI objects after session cleanup: %lu -> %lu\n",(unsigned long)gdi_before,(unsigned long)gdi_after);
    CHECK(gdi_before==gdi_after);
    return 0;
}
