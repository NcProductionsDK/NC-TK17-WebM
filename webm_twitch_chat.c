#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <windows.h>
#include <objbase.h>
#include <objidl.h>
#include <wincodec.h>
#include <winhttp.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "webm_twitch_chat.h"
#include "webm_twitch_auth.h"
#include "webm_twitch_emotes.h"

#define CHAT_MESSAGE_COUNT 64
#define CHAT_TEXT_MAX 768
#define CHAT_NAME_MAX 96
#define CHAT_JSON_MAX (128 * 1024)
#define CHAT_SESSION_COUNT 4
#define CHAT_RENDER_CACHE_COUNT 5
#define CHAT_PADDING_Y 7
#define CHAT_RUN_COUNT 32
#define CHAT_RUN_TEXT_MAX 512
#define CHAT_EMOTE_ID_MAX 64
#define CHAT_EMOTE_CACHE_COUNT 192
#define CHAT_EMOTE_DOWNLOAD_MAX (4 * 1024 * 1024)
#define CHAT_EMOTE_FRAME_COUNT_MAX 48
#define CHAT_EMOTE_DECODE_BYTES_MAX (4 * 1024 * 1024)
#define CHAT_EMOTE_SESSION_BYTES_MAX (24 * 1024 * 1024)

typedef enum {
    CHAT_RUN_TEXT = 0,
    CHAT_RUN_EMOTE
} webm_twitch_chat_run_type_t;

typedef struct {
    webm_twitch_chat_run_type_t type;
    char text[CHAT_RUN_TEXT_MAX];
    char emote_id[CHAT_EMOTE_ID_MAX];
} webm_twitch_chat_run_t;

typedef struct {
    char name[CHAT_NAME_MAX];
    char text[CHAT_TEXT_MAX];
    COLORREF color;
    int run_count;
    int has_emotes;
    webm_twitch_chat_run_t runs[CHAT_RUN_COUNT];
} webm_twitch_chat_message_t;

typedef enum {
    CHAT_EMOTE_EMPTY = 0,
    CHAT_EMOTE_QUEUED,
    CHAT_EMOTE_LOADING,
    CHAT_EMOTE_READY,
    CHAT_EMOTE_FAILED
} webm_twitch_chat_emote_state_t;

typedef struct {
    char id[CHAT_EMOTE_ID_MAX];
    webm_twitch_chat_emote_state_t state;
    unsigned char *pixels;
    DWORD *frame_delays;
    int width;
    int height;
    int frame_count;
    size_t frame_stride;
    size_t pixel_bytes;
    DWORD animation_duration;
    DWORD animation_start_tick;
    DWORD retry_tick;
    unsigned int last_used;
} webm_twitch_chat_emote_t;

typedef struct {
    int width;
    int height;
    int chat_width;
    int text_size;
    int horizontal_padding;
    int emote_scale_key;
    unsigned int message_serial;
    unsigned int status_serial;
    unsigned int animation_key;
    unsigned int last_used;
    int content_left;
    int content_top;
    int content_right;
    int content_bottom;
    int visible_start;
    int visible_count;
    unsigned int *pixels;
    size_t pixel_count;
    HFONT font;
    int font_height;
    int *row_bounds;
    int row_bounds_height;
    int row_bounds_valid;
    int row_bounds_seen;
} webm_twitch_chat_render_cache_t;

struct webm_twitch_chat_session {
    CRITICAL_SECTION lock;
    LONG references;
    volatile LONG stop;
    HANDLE thread;
    HANDLE emote_thread;
    HANDLE catalog_thread;
    HANDLE emote_event;
    HINTERNET websocket;
    webm_twitch_settings_t settings;
    char channel[WEBM_TWITCH_CHANNEL_MAX];
    char status[256];
    int connected;
    webm_twitch_chat_message_t messages[CHAT_MESSAGE_COUNT];
    int message_start;
    int message_count;
    unsigned int message_serial;
    unsigned int status_serial;
    unsigned int render_serial;
    webm_twitch_chat_render_cache_t render_cache[CHAT_RENDER_CACHE_COUNT];
    webm_twitch_chat_emote_t emotes[CHAT_EMOTE_CACHE_COUNT];
    webm_twitch_emote_catalog_t provider_catalog;
    volatile LONG provider_catalog_state;
    char broadcaster_id[WEBM_TWITCH_USER_ID_MAX];
    unsigned int emote_serial;
    size_t emote_pixel_bytes;
    int emote_parsed_logged;
    int emote_ready_logged;
    int emote_failed_logged;
    unsigned char *frame_scratch;
    size_t frame_scratch_size;
    int *resize_x_offsets;
    int resize_x_count;
    int resize_source_width;
    int resize_target_width;
    int resize_bytes_per_pixel;
    int dim_table_valid;
    float dim_table_opacity;
    unsigned char dim_table[256];
    unsigned char dim_table_5[32];
    unsigned char dim_table_6[64];
};

enum {
    CHAT_CATALOG_IDLE = 0,
    CHAT_CATALOG_LOADING,
    CHAT_CATALOG_READY,
    CHAT_CATALOG_FAILED
};

static CRITICAL_SECTION chat_sessions_lock;
static volatile LONG chat_sessions_lock_state;
static webm_twitch_chat_session_t *chat_sessions[CHAT_SESSION_COUNT];
static webm_twitch_chat_log_fn chat_debug_logger;

void webm_twitch_chat_set_debug_logger(webm_twitch_chat_log_fn logger)
{
    chat_debug_logger = logger;
}

static void chat_debug(const char *format, ...)
{
    char message[512];
    va_list arguments;
    if (!chat_debug_logger || !format) return;
    va_start(arguments, format);
    _vsnprintf(message, sizeof(message) - 1, format, arguments);
    va_end(arguments);
    message[sizeof(message) - 1] = 0;
    chat_debug_logger("%s", message);
}

static void copy_string(char *dst, size_t dst_size, const char *src)
{
    if (!dst || !dst_size) return;
    lstrcpynA(dst, src ? src : "", (int)dst_size);
    dst[dst_size - 1] = 0;
}

static void ensure_sessions_lock(void)
{
    LONG state = InterlockedCompareExchange(&chat_sessions_lock_state, 1, 0);
    if (state == 0) {
        InitializeCriticalSection(&chat_sessions_lock);
        InterlockedExchange(&chat_sessions_lock_state, 2);
    } else {
        while (InterlockedCompareExchange(&chat_sessions_lock_state, 2, 2) != 2) Sleep(0);
    }
}

static int stopped(webm_twitch_chat_session_t *session)
{
    return !session || InterlockedCompareExchange(&session->stop, 0, 0) != 0;
}

static int wait_stopped(webm_twitch_chat_session_t *session, DWORD milliseconds)
{
    DWORD start = GetTickCount();
    while (!stopped(session) && (DWORD)(GetTickCount() - start) < milliseconds) Sleep(50);
    return stopped(session);
}

static void set_status(webm_twitch_chat_session_t *session, const char *status, int connected)
{
    if (!session) return;
    EnterCriticalSection(&session->lock);
    if (_stricmp(session->status, status ? status : "") != 0 ||
        session->connected != connected) {
        copy_string(session->status, sizeof(session->status), status);
        session->connected = connected;
        session->status_serial++;
    }
    LeaveCriticalSection(&session->lock);
}

static const char *find_json_value(const char *json, const char *key)
{
    char marker[128];
    const char *p;
    if (!json || !key) return NULL;
    _snprintf(marker, sizeof(marker) - 1, "\"%s\"", key);
    marker[sizeof(marker) - 1] = 0;
    p = strstr(json, marker);
    if (!p) return NULL;
    p += strlen(marker);
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p++ != ':') return NULL;
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}

static void append_utf8(char *out, size_t out_size, size_t *used, unsigned int value)
{
    if (value <= 0x7f && *used + 1 < out_size) {
        out[(*used)++] = (char)value;
    } else if (value <= 0x7ff && *used + 2 < out_size) {
        out[(*used)++] = (char)(0xc0 | (value >> 6));
        out[(*used)++] = (char)(0x80 | (value & 0x3f));
    } else if (*used + 3 < out_size) {
        out[(*used)++] = (char)(0xe0 | (value >> 12));
        out[(*used)++] = (char)(0x80 | ((value >> 6) & 0x3f));
        out[(*used)++] = (char)(0x80 | (value & 0x3f));
    }
}

static int hex_value(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int json_string_at(const char *json, const char *key, char *out, size_t out_size)
{
    const char *p = find_json_value(json, key);
    size_t used = 0;
    if (!out || !out_size) return 0;
    out[0] = 0;
    if (!p || *p++ != '"') return 0;
    while (*p && *p != '"' && used + 1 < out_size) {
        unsigned char c = (unsigned char)*p++;
        if (c != '\\') {
            out[used++] = (char)c;
            continue;
        }
        c = (unsigned char)*p++;
        if (c == 'n') out[used++] = '\n';
        else if (c == 'r') out[used++] = '\r';
        else if (c == 't') out[used++] = '\t';
        else if (c == 'b') out[used++] = '\b';
        else if (c == 'f') out[used++] = '\f';
        else if (c == 'u') {
            unsigned int value = 0;
            int i;
            for (i = 0; i < 4; i++) {
                int h = hex_value(*p++);
                if (h < 0) return 0;
                value = (value << 4) | (unsigned int)h;
            }
            append_utf8(out, out_size, &used, value);
        } else {
            out[used++] = (char)c;
        }
    }
    out[used] = 0;
    return *p == '"';
}

static const char *json_compound_end(const char *start)
{
    const char *p = start;
    int depth = 0;
    int in_string = 0;
    int escaped = 0;
    if (!p || (*p != '{' && *p != '[')) return NULL;
    for (; *p; p++) {
        char c = *p;
        if (in_string) {
            if (escaped) escaped = 0;
            else if (c == '\\') escaped = 1;
            else if (c == '"') in_string = 0;
            continue;
        }
        if (c == '"') in_string = 1;
        else if (c == '{' || c == '[') depth++;
        else if (c == '}' || c == ']') {
            depth--;
            if (depth == 0) return p + 1;
        }
    }
    return NULL;
}

static int parse_message_runs(const char *message, webm_twitch_chat_run_t *runs,
                              int run_capacity)
{
    const char *array;
    const char *array_end;
    const char *p;
    int count = 0;
    if (!message || !runs || run_capacity <= 0) return 0;
    array = find_json_value(message, "fragments");
    if (!array || *array != '[') return 0;
    array_end = json_compound_end(array);
    if (!array_end) return 0;
    p = array + 1;
    while (p < array_end && count < run_capacity) {
        const char *object;
        const char *object_end;
        size_t object_size;
        char fragment[2048];
        char type[32];
        char text[CHAT_RUN_TEXT_MAX];
        char emote_id[CHAT_EMOTE_ID_MAX];
        while (p < array_end && *p != '{') p++;
        if (p >= array_end) break;
        object = p;
        object_end = json_compound_end(object);
        if (!object_end || object_end > array_end) break;
        object_size = (size_t)(object_end - object);
        if (object_size >= sizeof(fragment)) {
            p = object_end;
            continue;
        }
        memcpy(fragment, object, object_size);
        fragment[object_size] = 0;
        type[0] = text[0] = emote_id[0] = 0;
        json_string_at(fragment, "type", type, sizeof(type));
        json_string_at(fragment, "text", text, sizeof(text));
        {
            const char *emote = find_json_value(fragment, "emote");
            if (emote && *emote == '{') {
                char raw_id[CHAT_EMOTE_ID_MAX];
                raw_id[0] = 0;
                json_string_at(emote, "id", raw_id, sizeof(raw_id));
                if (raw_id[0]) {
                    /* EventSub supplies the native emote ID but does not reliably
                       identify animated renditions. Try Twitch's animated CDN
                       rendition first; the downloader falls back to static and
                       the worker forces static when animation is disabled. */
                    _snprintf(emote_id, sizeof(emote_id) - 1, "tg:%s", raw_id);
                    emote_id[sizeof(emote_id) - 1] = 0;
                }
            }
            if (_stricmp(type, "emote") == 0 && !emote_id[0]) {
                /* Be tolerant of fragment variants while still requiring the
                   fragment itself to identify as a native Twitch emote. */
                char raw_id[CHAT_EMOTE_ID_MAX];
                raw_id[0] = 0;
                json_string_at(fragment, "id", raw_id, sizeof(raw_id));
                if (raw_id[0]) {
                    _snprintf(emote_id, sizeof(emote_id) - 1, "tg:%s", raw_id);
                    emote_id[sizeof(emote_id) - 1] = 0;
                }
            }
        }
        if (text[0]) {
            runs[count].type = emote_id[0] ? CHAT_RUN_EMOTE : CHAT_RUN_TEXT;
            copy_string(runs[count].text, sizeof(runs[count].text), text);
            copy_string(runs[count].emote_id, sizeof(runs[count].emote_id), emote_id);
            count++;
        }
        p = object_end;
    }
    return count;
}

static void append_text_run(webm_twitch_chat_run_t *runs, int *count, int capacity,
                            const char *text, size_t length)
{
    while (length && *count < capacity) {
        webm_twitch_chat_run_t *run = *count > 0 ? &runs[*count - 1] : NULL;
        size_t used = run && run->type == CHAT_RUN_TEXT ? strlen(run->text) : 0;
        size_t available;
        size_t copied;
        if (!run || run->type != CHAT_RUN_TEXT || used + 1 >= sizeof(run->text)) {
            run = &runs[(*count)++];
            memset(run, 0, sizeof(*run));
            run->type = CHAT_RUN_TEXT;
            used = 0;
        }
        available = sizeof(run->text) - used - 1;
        copied = length < available ? length : available;
        memcpy(run->text + used, text, copied);
        run->text[used + copied] = 0;
        text += copied;
        length -= copied;
        if (!copied) break;
    }
}

static int expand_provider_emotes(webm_twitch_chat_session_t *session,
                                  const webm_twitch_chat_run_t *source,
                                  int source_count,
                                  webm_twitch_chat_run_t *target,
                                  int target_capacity)
{
    int count = 0;
    int i;
    if (!session || !source || source_count <= 0 || !target || target_capacity <= 0 ||
        InterlockedCompareExchange(&session->provider_catalog_state,
                                   CHAT_CATALOG_READY, CHAT_CATALOG_READY) !=
            CHAT_CATALOG_READY) {
        if (source && target && target_capacity > 0) {
            count = source_count < target_capacity ? source_count : target_capacity;
            memcpy(target, source, (size_t)count * sizeof(*target));
        }
        return count;
    }
    for (i = 0; i < source_count && count < target_capacity; i++) {
        const webm_twitch_chat_run_t *run = &source[i];
        if (run->type == CHAT_RUN_EMOTE) {
            target[count++] = *run;
            continue;
        }
        {
            const char *p = run->text;
            while (*p && count < target_capacity) {
                const char *start = p;
                const char *provider_id;
                size_t length;
                if (isspace((unsigned char)*p)) {
                    while (*p && isspace((unsigned char)*p)) p++;
                    append_text_run(target, &count, target_capacity,
                                    start, (size_t)(p - start));
                    continue;
                }
                while (*p && !isspace((unsigned char)*p)) p++;
                length = (size_t)(p - start);
                provider_id = webm_twitch_emote_catalog_lookup(
                    &session->provider_catalog, start, length);
                if (provider_id && count < target_capacity) {
                    webm_twitch_chat_run_t *output = &target[count++];
                    size_t copied = length < sizeof(output->text) - 1
                                        ? length : sizeof(output->text) - 1;
                    memset(output, 0, sizeof(*output));
                    output->type = CHAT_RUN_EMOTE;
                    memcpy(output->text, start, copied);
                    output->text[copied] = 0;
                    copy_string(output->emote_id, sizeof(output->emote_id), provider_id);
                } else {
                    append_text_run(target, &count, target_capacity, start, length);
                }
            }
        }
    }
    return count;
}

static COLORREF parse_color(const char *value, const char *name)
{
    unsigned int rgb = 0;
    const unsigned char *p;
    if (value && value[0] == '#' && strlen(value) == 7 &&
        sscanf(value + 1, "%x", &rgb) == 1) {
        return RGB((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255);
    }
    rgb = 2166136261u;
    for (p = (const unsigned char*)(name ? name : ""); *p; p++) {
        rgb ^= *p;
        rgb *= 16777619u;
    }
    return RGB(96 + (rgb & 127), 96 + ((rgb >> 8) & 127), 96 + ((rgb >> 16) & 127));
}

static webm_twitch_chat_emote_t *queue_emote_locked(
    webm_twitch_chat_session_t *session, const char *id);

static void add_message(webm_twitch_chat_session_t *session,
                        const char *name, const char *text, const char *color,
                        const webm_twitch_chat_run_t *runs, int run_count)
{
    int index;
    webm_twitch_chat_message_t *message;
    if (!session || !text || !text[0]) return;
    EnterCriticalSection(&session->lock);
    if (session->message_count < CHAT_MESSAGE_COUNT) {
        index = (session->message_start + session->message_count++) % CHAT_MESSAGE_COUNT;
    } else {
        index = session->message_start;
        session->message_start = (session->message_start + 1) % CHAT_MESSAGE_COUNT;
    }
    message = &session->messages[index];
    copy_string(message->name, sizeof(message->name), name && name[0] ? name : "Twitch");
    copy_string(message->text, sizeof(message->text), text);
    message->color = parse_color(color, message->name);
    message->run_count = 0;
    message->has_emotes = 0;
    if (runs && run_count > 0) {
        int i;
        if (run_count > CHAT_RUN_COUNT) run_count = CHAT_RUN_COUNT;
        for (i = 0; i < run_count; i++) {
            message->runs[i] = runs[i];
            if (runs[i].type == CHAT_RUN_EMOTE) {
                message->has_emotes = 1;
                queue_emote_locked(session, runs[i].emote_id);
                if (!session->emote_parsed_logged) {
                    session->emote_parsed_logged = 1;
                    chat_debug("Twitch chat emote parsed channel=\"%s\" id=\"%s\" token=\"%s\"",
                               session->channel, runs[i].emote_id, runs[i].text);
                }
            }
        }
        message->run_count = run_count;
    }
    session->message_serial++;
    LeaveCriticalSection(&session->lock);
}

static void parse_event_message(webm_twitch_chat_session_t *session, const char *json)
{
    const char *event;
    const char *message;
    char type[64];
    char name[CHAT_NAME_MAX];
    char text[CHAT_TEXT_MAX];
    char color[32];
    webm_twitch_chat_run_t native_runs[CHAT_RUN_COUNT];
    webm_twitch_chat_run_t runs[CHAT_RUN_COUNT];
    int native_run_count;
    int run_count;
    type[0] = 0;
    if (!json_string_at(json, "message_type", type, sizeof(type)) ||
        _stricmp(type, "notification") != 0) return;
    event = strstr(json, "\"event\"");
    if (!event) return;
    name[0] = text[0] = color[0] = 0;
    json_string_at(event, "chatter_user_name", name, sizeof(name));
    json_string_at(event, "color", color, sizeof(color));
    message = strstr(event, "\"message\"");
    if (!message || !json_string_at(message, "text", text, sizeof(text))) return;
    memset(native_runs, 0, sizeof(native_runs));
    memset(runs, 0, sizeof(runs));
    native_run_count = parse_message_runs(message, native_runs, CHAT_RUN_COUNT);
    if (native_run_count <= 0 && text[0]) {
        native_runs[0].type = CHAT_RUN_TEXT;
        copy_string(native_runs[0].text, sizeof(native_runs[0].text), text);
        native_run_count = 1;
    }
    run_count = expand_provider_emotes(session, native_runs, native_run_count,
                                       runs, CHAT_RUN_COUNT);
    add_message(session, name, text, color, runs, run_count);
}

static webm_twitch_chat_emote_t *find_emote_locked(webm_twitch_chat_session_t *session,
                                                    const char *id)
{
    int i;
    if (!session || !id || !id[0]) return NULL;
    for (i = 0; i < CHAT_EMOTE_CACHE_COUNT; i++) {
        if (session->emotes[i].state != CHAT_EMOTE_EMPTY &&
            _stricmp(session->emotes[i].id, id) == 0) {
            session->emotes[i].last_used = ++session->emote_serial;
            return &session->emotes[i];
        }
    }
    return NULL;
}

static void free_emote_data_locked(webm_twitch_chat_session_t *session,
                                   webm_twitch_chat_emote_t *entry)
{
    if (!entry) return;
    if (session) {
        if (entry->pixel_bytes <= session->emote_pixel_bytes) {
            session->emote_pixel_bytes -= entry->pixel_bytes;
        } else {
            session->emote_pixel_bytes = 0;
        }
    }
    free(entry->pixels);
    free(entry->frame_delays);
    entry->pixels = NULL;
    entry->frame_delays = NULL;
    entry->pixel_bytes = 0;
    entry->frame_count = 0;
    entry->frame_stride = 0;
    entry->animation_duration = 0;
}

static void make_emote_cache_room_locked(webm_twitch_chat_session_t *session,
                                         webm_twitch_chat_emote_t *protected_entry,
                                         size_t incoming_bytes)
{
    while (session && session->emote_pixel_bytes + incoming_bytes >
                          CHAT_EMOTE_SESSION_BYTES_MAX) {
        webm_twitch_chat_emote_t *oldest = NULL;
        int i;
        for (i = 0; i < CHAT_EMOTE_CACHE_COUNT; i++) {
            webm_twitch_chat_emote_t *candidate = &session->emotes[i];
            if (candidate == protected_entry || !candidate->pixels ||
                candidate->state == CHAT_EMOTE_LOADING ||
                candidate->state == CHAT_EMOTE_QUEUED) {
                continue;
            }
            if (!oldest || candidate->last_used < oldest->last_used) oldest = candidate;
        }
        if (!oldest) break;
        free_emote_data_locked(session, oldest);
        memset(oldest, 0, sizeof(*oldest));
    }
}

static webm_twitch_chat_emote_t *queue_emote_locked(webm_twitch_chat_session_t *session,
                                                     const char *id)
{
    webm_twitch_chat_emote_t *entry;
    webm_twitch_chat_emote_t *slot = NULL;
    DWORD now = GetTickCount();
    int i;
    if (!session || !id || !id[0]) return NULL;
    entry = find_emote_locked(session, id);
    if (entry) {
        if (entry->state == CHAT_EMOTE_FAILED &&
            (LONG)(now - entry->retry_tick) >= 0) {
            entry->state = CHAT_EMOTE_QUEUED;
            if (session->emote_event) SetEvent(session->emote_event);
        }
        return entry;
    }
    for (i = 0; i < CHAT_EMOTE_CACHE_COUNT; i++) {
        webm_twitch_chat_emote_t *candidate = &session->emotes[i];
        if (candidate->state == CHAT_EMOTE_EMPTY) {
            slot = candidate;
            break;
        }
        if (candidate->state != CHAT_EMOTE_LOADING &&
            candidate->state != CHAT_EMOTE_QUEUED &&
            (!slot || candidate->last_used < slot->last_used)) {
            slot = candidate;
        }
    }
    if (!slot) return NULL;
    free_emote_data_locked(session, slot);
    memset(slot, 0, sizeof(*slot));
    copy_string(slot->id, sizeof(slot->id), id);
    slot->state = CHAT_EMOTE_QUEUED;
    slot->last_used = ++session->emote_serial;
    if (session->emote_event) SetEvent(session->emote_event);
    return slot;
}

static int download_emote_png(const char *id, unsigned char **data_out, size_t *size_out)
{
    HINTERNET internet = NULL;
    HINTERNET connection = NULL;
    HINTERNET request = NULL;
    wchar_t id_w[CHAT_EMOTE_ID_MAX];
    wchar_t path[256];
    unsigned char *data = NULL;
    size_t size = 0;
    DWORD status = 0;
    DWORD status_size = sizeof(status);
    int ok = 0;
    if (data_out) *data_out = NULL;
    if (size_out) *size_out = 0;
    if (!id || !id[0] || !data_out || !size_out) return 0;
    if (!MultiByteToWideChar(CP_UTF8, 0, id, -1, id_w,
                             (int)(sizeof(id_w) / sizeof(id_w[0])))) return 0;
    _snwprintf(path, (sizeof(path) / sizeof(path[0])) - 1,
               L"/emoticons/v2/%ls/static/dark/2.0", id_w);
    path[(sizeof(path) / sizeof(path[0])) - 1] = 0;
    internet = WinHttpOpen(L"NC-TK17-WebM Emotes/1.0",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!internet) goto done;
    WinHttpSetTimeouts(internet, 5000, 5000, 5000, 10000);
    connection = WinHttpConnect(internet, L"static-cdn.jtvnw.net",
                                INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connection) goto done;
    request = WinHttpOpenRequest(connection, L"GET", path, NULL,
                                 WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                 WINHTTP_FLAG_SECURE);
    if (!request ||
        !WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(request, NULL)) goto done;
    if (!WinHttpQueryHeaders(request,
                             WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_size,
                             WINHTTP_NO_HEADER_INDEX) || status != 200) goto done;
    for (;;) {
        DWORD available = 0;
        DWORD read = 0;
        unsigned char *grown;
        if (!WinHttpQueryDataAvailable(request, &available)) goto done;
        if (!available) break;
        if (size + available > CHAT_EMOTE_DOWNLOAD_MAX) goto done;
        grown = (unsigned char*)realloc(data, size + available);
        if (!grown) goto done;
        data = grown;
        if (!WinHttpReadData(request, data + size, available, &read) || !read) goto done;
        size += read;
    }
    ok = size > 0;
done:
    if (request) WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    if (internet) WinHttpCloseHandle(internet);
    if (!ok) {
        free(data);
        return 0;
    }
    *data_out = data;
    *size_out = size;
    return 1;
}

static DWORD metadata_uint(IWICMetadataQueryReader *reader,
                           const wchar_t *name, DWORD fallback)
{
    PROPVARIANT value;
    DWORD result = fallback;
    PropVariantInit(&value);
    if (reader && name &&
        SUCCEEDED(IWICMetadataQueryReader_GetMetadataByName(reader, name, &value))) {
        switch (value.vt) {
        case VT_UI1: result = value.bVal; break;
        case VT_UI2: result = value.uiVal; break;
        case VT_UI4: result = value.ulVal; break;
        case VT_I1: if (value.cVal >= 0) result = (DWORD)value.cVal; break;
        case VT_I2: if (value.iVal >= 0) result = (DWORD)value.iVal; break;
        case VT_I4: if (value.lVal >= 0) result = (DWORD)value.lVal; break;
        default: break;
        }
    }
    PropVariantClear(&value);
    return result;
}

static DWORD emote_frame_delay(IWICMetadataQueryReader *reader)
{
    DWORD centiseconds = metadata_uint(reader, L"/grctlext/Delay", 0);
    DWORD delay = centiseconds ? centiseconds * 10u : 100u;
    if (delay < 20) delay = 20;
    if (delay > 10000) delay = 10000;
    return delay;
}

static void clear_bgra_rect(unsigned char *canvas, UINT width, UINT height,
                            UINT left, UINT top, UINT rect_width, UINT rect_height)
{
    UINT y;
    if (!canvas || !width || !height || left >= width || top >= height) return;
    if (rect_width > width - left) rect_width = width - left;
    if (rect_height > height - top) rect_height = height - top;
    for (y = 0; y < rect_height; y++) {
        memset(canvas + (((size_t)top + y) * width + left) * 4u,
               0, (size_t)rect_width * 4u);
    }
}

static void blend_bgra_frame(unsigned char *canvas, UINT canvas_width,
                             UINT canvas_height, const unsigned char *frame,
                             UINT frame_width, UINT frame_height,
                             UINT left, UINT top)
{
    UINT x, y;
    UINT source_width = frame_width;
    if (!canvas || !frame || !canvas_width || !canvas_height ||
        left >= canvas_width || top >= canvas_height) return;
    if (frame_width > canvas_width - left) frame_width = canvas_width - left;
    if (frame_height > canvas_height - top) frame_height = canvas_height - top;
    for (y = 0; y < frame_height; y++) {
        unsigned char *dst = canvas +
            (((size_t)top + y) * canvas_width + left) * 4u;
        const unsigned char *src = frame + (size_t)y * source_width * 4u;
        for (x = 0; x < frame_width; x++, dst += 4, src += 4) {
            unsigned int alpha = src[3];
            if (!alpha) continue;
            if (alpha == 255u) {
                memcpy(dst, src, 4u);
            } else {
                unsigned int inverse = 255u - alpha;
                dst[0] = (unsigned char)((src[0] * alpha + dst[0] * inverse + 127u) / 255u);
                dst[1] = (unsigned char)((src[1] * alpha + dst[1] * inverse + 127u) / 255u);
                dst[2] = (unsigned char)((src[2] * alpha + dst[2] * inverse + 127u) / 255u);
                dst[3] = (unsigned char)(alpha + (dst[3] * inverse + 127u) / 255u);
            }
        }
    }
}

static int decode_emote_image(const unsigned char *data, size_t data_size,
                              int animated_enabled,
                              unsigned char **pixels_out, DWORD **frame_delays_out,
                              int *frame_count_out, size_t *frame_stride_out,
                              size_t *pixel_bytes_out, DWORD *duration_out,
                              int *width_out, int *height_out)
{
    HGLOBAL memory = NULL;
    void *memory_data;
    IStream *stream = NULL;
    IWICImagingFactory *factory = NULL;
    IWICBitmapDecoder *decoder = NULL;
    IWICMetadataQueryReader *decoder_reader = NULL;
    unsigned char *pixels = NULL;
    unsigned char *canvas = NULL;
    unsigned char *restore_canvas = NULL;
    unsigned char *raw_frame = NULL;
    size_t raw_frame_capacity = 0;
    DWORD *frame_delays = NULL;
    UINT width = 0;
    UINT height = 0;
    UINT available_frames = 0;
    UINT wanted_frames;
    UINT decoded_frames = 0;
    size_t frame_stride = 0;
    size_t pixel_bytes = 0;
    DWORD duration = 0;
    DWORD previous_disposal = 0;
    UINT previous_left = 0;
    UINT previous_top = 0;
    UINT previous_width = 0;
    UINT previous_height = 0;
    HRESULT hr;
    int ok = 0;
    int gif_container = 0;
    GUID container_format;
    if (pixels_out) *pixels_out = NULL;
    if (frame_delays_out) *frame_delays_out = NULL;
    if (frame_count_out) *frame_count_out = 0;
    if (frame_stride_out) *frame_stride_out = 0;
    if (pixel_bytes_out) *pixel_bytes_out = 0;
    if (duration_out) *duration_out = 0;
    if (width_out) *width_out = 0;
    if (height_out) *height_out = 0;
    if (!data || !data_size || !pixels_out || !frame_delays_out ||
        !frame_count_out || !frame_stride_out || !pixel_bytes_out ||
        !duration_out || !width_out || !height_out) return 0;
    memory = GlobalAlloc(GMEM_MOVEABLE, data_size);
    if (!memory) goto done;
    memory_data = GlobalLock(memory);
    if (!memory_data) goto done;
    memcpy(memory_data, data, data_size);
    GlobalUnlock(memory);
    if (FAILED(CreateStreamOnHGlobal(memory, TRUE, &stream))) goto done;
    memory = NULL;
    hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                          &IID_IWICImagingFactory, (void**)&factory);
    if (FAILED(hr)) goto done;
    if (FAILED(IWICImagingFactory_CreateDecoderFromStream(
            factory, stream, NULL, WICDecodeMetadataCacheOnLoad, &decoder)) ||
        FAILED(IWICBitmapDecoder_GetFrameCount(decoder, &available_frames)) ||
        !available_frames) goto done;
    if (SUCCEEDED(IWICBitmapDecoder_GetContainerFormat(decoder, &container_format))) {
        gif_container = IsEqualGUID(&container_format, &GUID_ContainerFormatGif);
    }
    if (SUCCEEDED(IWICBitmapDecoder_GetMetadataQueryReader(decoder, &decoder_reader)) &&
        decoder_reader) {
        width = metadata_uint(decoder_reader, L"/logscrdesc/Width", 0);
        height = metadata_uint(decoder_reader, L"/logscrdesc/Height", 0);
    }
    wanted_frames = animated_enabled ? available_frames : 1;
    if (wanted_frames > CHAT_EMOTE_FRAME_COUNT_MAX) {
        wanted_frames = CHAT_EMOTE_FRAME_COUNT_MAX;
    }
    while (decoded_frames < wanted_frames) {
        IWICBitmapFrameDecode *frame = NULL;
        IWICFormatConverter *converter = NULL;
        IWICMetadataQueryReader *frame_reader = NULL;
        UINT frame_width = 0;
        UINT frame_height = 0;
        UINT frame_left = 0;
        UINT frame_top = 0;
        UINT disposal_left = 0;
        UINT disposal_top = 0;
        UINT disposal_width = 0;
        UINT disposal_height = 0;
        size_t raw_size;
        DWORD disposal;
        DWORD delay;
        if (FAILED(IWICBitmapDecoder_GetFrame(decoder, decoded_frames, &frame)) ||
            FAILED(IWICBitmapFrameDecode_GetSize(frame, &frame_width, &frame_height)) ||
            !frame_width || !frame_height || frame_width > 256 || frame_height > 256) {
            if (frame) IWICBitmapFrameDecode_Release(frame);
            break;
        }
        IWICBitmapFrameDecode_GetMetadataQueryReader(frame, &frame_reader);
        if (frame_reader) {
            frame_left = metadata_uint(frame_reader, L"/imgdesc/Left", 0);
            frame_top = metadata_uint(frame_reader, L"/imgdesc/Top", 0);
            disposal_width = metadata_uint(frame_reader, L"/imgdesc/Width", frame_width);
            disposal_height = metadata_uint(frame_reader, L"/imgdesc/Height", frame_height);
        }
        disposal_left = frame_left;
        disposal_top = frame_top;
        if (!decoded_frames) {
            if (!width || !height) {
                width = frame_width;
                height = frame_height;
            }
            if (width > 256 || height > 256 ||
                frame_left >= width || frame_top >= height) {
                if (frame_reader) IWICMetadataQueryReader_Release(frame_reader);
                IWICBitmapFrameDecode_Release(frame);
                break;
            }
            frame_stride = (size_t)width * (size_t)height * 4u;
            if (!frame_stride) {
                if (frame_reader) IWICMetadataQueryReader_Release(frame_reader);
                IWICBitmapFrameDecode_Release(frame);
                break;
            }
            if (wanted_frames > CHAT_EMOTE_DECODE_BYTES_MAX / frame_stride) {
                wanted_frames = (UINT)(CHAT_EMOTE_DECODE_BYTES_MAX / frame_stride);
                if (!wanted_frames) wanted_frames = 1;
            }
            pixel_bytes = frame_stride * wanted_frames;
            pixels = (unsigned char*)malloc(pixel_bytes);
            canvas = (unsigned char*)calloc(1, frame_stride);
            restore_canvas = (unsigned char*)malloc(frame_stride);
            if (wanted_frames > 1) {
                frame_delays = (DWORD*)calloc(wanted_frames, sizeof(*frame_delays));
            }
            if (!pixels || !canvas || !restore_canvas ||
                (wanted_frames > 1 && !frame_delays)) {
                if (frame_reader) IWICMetadataQueryReader_Release(frame_reader);
                IWICBitmapFrameDecode_Release(frame);
                break;
            }
        }
        if (frame_width == width && frame_height == height) {
            frame_left = 0;
            frame_top = 0;
        }
        if (frame_left >= width || frame_top >= height) {
            if (frame_reader) IWICMetadataQueryReader_Release(frame_reader);
            IWICBitmapFrameDecode_Release(frame);
            break;
        }
        raw_size = (size_t)frame_width * (size_t)frame_height * 4u;
        if (raw_frame_capacity < raw_size) {
            unsigned char *grown = (unsigned char*)realloc(raw_frame, raw_size);
            if (!grown) {
                if (frame_reader) IWICMetadataQueryReader_Release(frame_reader);
                IWICBitmapFrameDecode_Release(frame);
                break;
            }
            raw_frame = grown;
            raw_frame_capacity = raw_size;
        }
        if (FAILED(IWICImagingFactory_CreateFormatConverter(factory, &converter)) ||
            FAILED(IWICFormatConverter_Initialize(
                converter, (IWICBitmapSource*)frame, &GUID_WICPixelFormat32bppBGRA,
                WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom)) ||
            FAILED(IWICFormatConverter_CopyPixels(
                converter, NULL, frame_width * 4u, (UINT)raw_size,
                raw_frame))) {
            if (converter) IWICFormatConverter_Release(converter);
            if (frame_reader) IWICMetadataQueryReader_Release(frame_reader);
            IWICBitmapFrameDecode_Release(frame);
            break;
        }
        if (!gif_container) {
            memset(canvas, 0, frame_stride);
        } else if (decoded_frames) {
            if (previous_disposal == 2u) {
                clear_bgra_rect(canvas, width, height,
                                previous_left, previous_top,
                                previous_width, previous_height);
            } else if (previous_disposal == 3u) {
                memcpy(canvas, restore_canvas, frame_stride);
            }
        }
        disposal = metadata_uint(frame_reader, L"/grctlext/Disposal", 0);
        if (disposal == 3u) memcpy(restore_canvas, canvas, frame_stride);
        blend_bgra_frame(canvas, width, height, raw_frame,
                         frame_width, frame_height, frame_left, frame_top);
        memcpy(pixels + (size_t)decoded_frames * frame_stride,
               canvas, frame_stride);
        delay = emote_frame_delay(frame_reader);
        if (frame_delays) frame_delays[decoded_frames] = delay;
        duration += delay;
        previous_disposal = disposal;
        previous_left = disposal_left;
        previous_top = disposal_top;
        previous_width = disposal_width;
        previous_height = disposal_height;
        decoded_frames++;
        IWICFormatConverter_Release(converter);
        if (frame_reader) IWICMetadataQueryReader_Release(frame_reader);
        IWICBitmapFrameDecode_Release(frame);
    }
    if (!decoded_frames) goto done;
    if (decoded_frames < wanted_frames) {
        unsigned char *smaller_pixels = (unsigned char*)realloc(
            pixels, frame_stride * decoded_frames);
        if (smaller_pixels) pixels = smaller_pixels;
        if (frame_delays && decoded_frames > 1) {
            DWORD *smaller_delays = (DWORD*)realloc(
                frame_delays, sizeof(*frame_delays) * decoded_frames);
            if (smaller_delays) frame_delays = smaller_delays;
        }
    }
    if (decoded_frames == 1) {
        free(frame_delays);
        frame_delays = NULL;
        duration = 0;
    }
    pixel_bytes = frame_stride * decoded_frames;
    *pixels_out = pixels;
    *frame_delays_out = frame_delays;
    *frame_count_out = (int)decoded_frames;
    *frame_stride_out = frame_stride;
    *pixel_bytes_out = pixel_bytes;
    *duration_out = duration;
    *width_out = (int)width;
    *height_out = (int)height;
    pixels = NULL;
    frame_delays = NULL;
    ok = 1;
done:
    free(pixels);
    free(canvas);
    free(restore_canvas);
    free(raw_frame);
    free(frame_delays);
    if (decoder_reader) IWICMetadataQueryReader_Release(decoder_reader);
    if (decoder) IWICBitmapDecoder_Release(decoder);
    if (factory) IWICImagingFactory_Release(factory);
    if (stream) IStream_Release(stream);
    if (memory) GlobalFree(memory);
    return ok;
}

static DWORD WINAPI emote_thread(void *parameter)
{
    webm_twitch_chat_session_t *session = (webm_twitch_chat_session_t*)parameter;
    HRESULT com_result = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    while (!stopped(session)) {
        char id[CHAT_EMOTE_ID_MAX];
        char download_id[CHAT_EMOTE_ID_MAX];
        unsigned char *download = NULL;
        unsigned char *pixels = NULL;
        DWORD *frame_delays = NULL;
        size_t download_size = 0;
        size_t frame_stride = 0;
        size_t pixel_bytes = 0;
        DWORD animation_duration = 0;
        int frame_count = 0;
        int width = 0;
        int height = 0;
        int found = 0;
        int loaded = 0;
        int i;
        WaitForSingleObject(session->emote_event, 500);
        if (stopped(session)) break;
        id[0] = 0;
        EnterCriticalSection(&session->lock);
        for (i = 0; i < CHAT_EMOTE_CACHE_COUNT; i++) {
            if (session->emotes[i].state == CHAT_EMOTE_QUEUED) {
                session->emotes[i].state = CHAT_EMOTE_LOADING;
                copy_string(id, sizeof(id), session->emotes[i].id);
                found = 1;
                break;
            }
        }
        LeaveCriticalSection(&session->lock);
        if (!found) continue;
        copy_string(download_id, sizeof(download_id), id);
        if (!session->settings.chat_animated_emotes &&
            (download_id[0] == 'b' || download_id[0] == '7' ||
             download_id[0] == 't') &&
            download_id[1] == 'g' && download_id[2] == ':') {
            download_id[1] = 'p';
        }
        loaded = webm_twitch_emote_download(download_id, &download, &download_size) &&
                 decode_emote_image(download, download_size,
                                    session->settings.chat_animated_emotes,
                                    &pixels, &frame_delays, &frame_count,
                                    &frame_stride, &pixel_bytes,
                                    &animation_duration, &width, &height);
        free(download);
        EnterCriticalSection(&session->lock);
        for (i = 0; i < CHAT_EMOTE_CACHE_COUNT; i++) {
            webm_twitch_chat_emote_t *entry = &session->emotes[i];
            if (entry->state == CHAT_EMOTE_LOADING && _stricmp(entry->id, id) == 0) {
                if (loaded) {
                    make_emote_cache_room_locked(session, entry, pixel_bytes);
                    if (session->emote_pixel_bytes + pixel_bytes >
                        CHAT_EMOTE_SESSION_BYTES_MAX) {
                        loaded = 0;
                    }
                }
                if (loaded) {
                    free_emote_data_locked(session, entry);
                    entry->pixels = pixels;
                    entry->frame_delays = frame_delays;
                    entry->width = width;
                    entry->height = height;
                    entry->frame_count = frame_count;
                    entry->frame_stride = frame_stride;
                    entry->pixel_bytes = pixel_bytes;
                    entry->animation_duration = animation_duration;
                    entry->animation_start_tick = GetTickCount();
                    session->emote_pixel_bytes += pixel_bytes;
                    entry->state = CHAT_EMOTE_READY;
                    if (!session->emote_ready_logged) {
                        session->emote_ready_logged = 1;
                        chat_debug("Twitch chat emote ready channel=\"%s\" id=\"%s\" size=%dx%d frames=%d",
                                   session->channel, id, width, height, frame_count);
                    }
                    pixels = NULL;
                    frame_delays = NULL;
                } else {
                    entry->state = CHAT_EMOTE_FAILED;
                    entry->retry_tick = GetTickCount() + 60000;
                    if (!session->emote_failed_logged) {
                        session->emote_failed_logged = 1;
                        chat_debug("Twitch chat emote load failed channel=\"%s\" id=\"%s\"",
                                   session->channel, id);
                    }
                }
                session->message_serial++;
                break;
            }
        }
        LeaveCriticalSection(&session->lock);
        free(pixels);
        free(frame_delays);
        SetEvent(session->emote_event);
    }
    if (SUCCEEDED(com_result)) CoUninitialize();
    return 0;
}

static int open_websocket(webm_twitch_chat_session_t *session,
                          HINTERNET *internet_out, HINTERNET *connection_out,
                          HINTERNET *websocket_out)
{
    HINTERNET internet = NULL;
    HINTERNET connection = NULL;
    HINTERNET request = NULL;
    HINTERNET websocket = NULL;
    DWORD status = 0;
    DWORD status_size = sizeof(status);
    internet = WinHttpOpen(L"NC-TK17-WebM Chat/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!internet) goto failed;
    WinHttpSetTimeouts(internet, 5000, 5000, 5000, 35000);
    connection = WinHttpConnect(internet, L"eventsub.wss.twitch.tv",
                                INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connection) goto failed;
    request = WinHttpOpenRequest(connection, L"GET", L"/ws?keepalive_timeout_seconds=30",
                                 NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                 WINHTTP_FLAG_SECURE);
    if (!request) goto failed;
    if (!WinHttpSetOption(request, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0) ||
        !WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(request, NULL)) goto failed;
    WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_size,
                        WINHTTP_NO_HEADER_INDEX);
    if (status != 101) goto failed;
    websocket = WinHttpWebSocketCompleteUpgrade(request, 0);
    if (!websocket) goto failed;
    WinHttpCloseHandle(request);
    EnterCriticalSection(&session->lock);
    session->websocket = websocket;
    LeaveCriticalSection(&session->lock);
    *internet_out = internet;
    *connection_out = connection;
    *websocket_out = websocket;
    return 1;
failed:
    if (websocket) WinHttpCloseHandle(websocket);
    if (request) WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    if (internet) WinHttpCloseHandle(internet);
    return 0;
}

static int receive_websocket_message(HINTERNET websocket, char *json, size_t json_size)
{
    size_t used = 0;
    for (;;) {
        DWORD read = 0;
        DWORD result;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE type;
        if (used + 1 >= json_size) return 0;
        result = WinHttpWebSocketReceive(websocket, json + used,
                                         (DWORD)(json_size - used - 1), &read, &type);
        if (result != NO_ERROR) return 0;
        used += read;
        if (type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE) return 0;
        if (type == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE) {
            json[used] = 0;
            return 1;
        }
        if (type != WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE) used = 0;
    }
}

static DWORD WINAPI catalog_thread(void *parameter)
{
    webm_twitch_chat_session_t *session = (webm_twitch_chat_session_t*)parameter;
    webm_twitch_emote_catalog_t catalog;
    int loaded;
    memset(&catalog, 0, sizeof(catalog));
    loaded = webm_twitch_emote_catalog_load(
        &catalog, session->broadcaster_id,
        session->settings.chat_bttv, session->settings.chat_7tv,
        &session->stop);
    if (stopped(session)) {
        webm_twitch_emote_catalog_destroy(&catalog);
        return 0;
    }
    if (loaded) {
        EnterCriticalSection(&session->lock);
        session->provider_catalog = catalog;
        session->message_serial++;
        LeaveCriticalSection(&session->lock);
        InterlockedExchange(&session->provider_catalog_state, CHAT_CATALOG_READY);
        chat_debug("Twitch chat provider catalogs ready channel=\"%s\" BTTV=%d 7TV=%d total=%u",
                   session->channel, catalog.bttv_count, catalog.seventv_count,
                   (unsigned int)catalog.count);
    } else {
        InterlockedExchange(&session->provider_catalog_state, CHAT_CATALOG_FAILED);
        chat_debug("Twitch chat provider catalogs unavailable channel=\"%s\" BTTV=%d 7TV=%d",
                   session->channel, session->settings.chat_bttv,
                   session->settings.chat_7tv);
    }
    return 0;
}

static DWORD WINAPI chat_thread(void *parameter)
{
    webm_twitch_chat_session_t *session = (webm_twitch_chat_session_t*)parameter;
    char access_token[WEBM_TWITCH_TOKEN_MAX];
    char user_id[WEBM_TWITCH_USER_ID_MAX];
    char user_login[WEBM_TWITCH_CHANNEL_MAX];
    char broadcaster_id[WEBM_TWITCH_USER_ID_MAX];
    char websocket_session_id[256];
    char error[256];
    char *json = (char*)malloc(CHAT_JSON_MAX);
    if (!json) {
        set_status(session, "Chat memory allocation failed", 0);
        return 0;
    }
    while (!stopped(session)) {
        HINTERNET internet = NULL;
        HINTERNET connection = NULL;
        HINTERNET websocket = NULL;
        set_status(session, "Authorizing Twitch chat...", 0);
        error[0] = 0;
        if (!webm_twitch_auth_get_chat_credentials(&session->settings,
                                                    webm_twitch_get_binary_dir(),
                                                    &session->stop,
                                                    access_token, sizeof(access_token),
                                                    user_id, sizeof(user_id),
                                                    user_login, sizeof(user_login),
                                                    error, sizeof(error))) {
            set_status(session, error[0] ? error : "Twitch chat authorization failed", 0);
            if (wait_stopped(session, 5000)) break;
            continue;
        }
        error[0] = 0;
        if (!webm_twitch_auth_get_user_id(session->settings.client_id, access_token,
                                          session->channel, broadcaster_id,
                                          sizeof(broadcaster_id), error, sizeof(error))) {
            set_status(session, error[0] ? error : "Twitch chat channel lookup failed", 0);
            if (wait_stopped(session, 5000)) break;
            continue;
        }
        if (session->settings.chat_emotes &&
            (session->settings.chat_bttv || session->settings.chat_7tv) &&
            InterlockedCompareExchange(&session->provider_catalog_state,
                                       CHAT_CATALOG_LOADING, CHAT_CATALOG_IDLE) ==
                CHAT_CATALOG_IDLE) {
            copy_string(session->broadcaster_id, sizeof(session->broadcaster_id),
                        broadcaster_id);
            session->catalog_thread = CreateThread(NULL, 0, catalog_thread,
                                                   session, 0, NULL);
            if (!session->catalog_thread) {
                InterlockedExchange(&session->provider_catalog_state,
                                    CHAT_CATALOG_FAILED);
            }
        }
        set_status(session, "Connecting to Twitch chat...", 0);
        if (!open_websocket(session, &internet, &connection, &websocket) ||
            !receive_websocket_message(websocket, json, CHAT_JSON_MAX) ||
            !json_string_at(json, "id", websocket_session_id, sizeof(websocket_session_id))) {
            set_status(session, "Twitch chat WebSocket connection failed", 0);
            goto disconnected;
        }
        error[0] = 0;
        if (!webm_twitch_auth_subscribe_chat(session->settings.client_id, access_token,
                                             broadcaster_id, user_id,
                                             websocket_session_id,
                                             error, sizeof(error))) {
            set_status(session, error[0] ? error : "Twitch chat subscription failed", 0);
            goto disconnected;
        }
        set_status(session, "", 1);
        while (!stopped(session) && receive_websocket_message(websocket, json, CHAT_JSON_MAX)) {
            parse_event_message(session, json);
        }
        if (!stopped(session)) set_status(session, "Reconnecting to Twitch chat...", 0);
disconnected:
        EnterCriticalSection(&session->lock);
        session->websocket = NULL;
        LeaveCriticalSection(&session->lock);
        if (websocket) {
            WinHttpWebSocketClose(websocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
            WinHttpCloseHandle(websocket);
        }
        if (connection) WinHttpCloseHandle(connection);
        if (internet) WinHttpCloseHandle(internet);
        if (!stopped(session) && wait_stopped(session, 3000)) break;
    }
    free(json);
    return 0;
}

webm_twitch_chat_session_t *webm_twitch_chat_acquire(
    const webm_twitch_settings_t *settings, const char *channel)
{
    webm_twitch_chat_session_t *session = NULL;
    int i;
    int free_index = -1;
    if (!settings || !settings->chat_enabled || !channel || !channel[0]) return NULL;
    ensure_sessions_lock();
    EnterCriticalSection(&chat_sessions_lock);
    for (i = 0; i < CHAT_SESSION_COUNT; i++) {
        if (!chat_sessions[i] && free_index < 0) free_index = i;
        if (chat_sessions[i] && _stricmp(chat_sessions[i]->channel, channel) == 0 &&
            _stricmp(chat_sessions[i]->settings.client_id, settings->client_id) == 0 &&
            chat_sessions[i]->settings.chat_emotes == settings->chat_emotes &&
            chat_sessions[i]->settings.chat_bttv == settings->chat_bttv &&
            chat_sessions[i]->settings.chat_7tv == settings->chat_7tv &&
            chat_sessions[i]->settings.chat_animated_emotes ==
                settings->chat_animated_emotes &&
            chat_sessions[i]->settings.chat_animated_emote_fps ==
                settings->chat_animated_emote_fps) {
            session = chat_sessions[i];
            InterlockedIncrement(&session->references);
            break;
        }
    }
    if (!session && free_index >= 0) {
        session = (webm_twitch_chat_session_t*)calloc(1, sizeof(*session));
        if (session) {
            InitializeCriticalSection(&session->lock);
            session->references = 1;
            session->settings = *settings;
            copy_string(session->channel, sizeof(session->channel), channel);
            copy_string(session->status, sizeof(session->status), "Starting Twitch chat...");
            session->emote_event = CreateEventA(NULL, FALSE, FALSE, NULL);
            if (session->emote_event) {
                session->emote_thread = CreateThread(NULL, 0, emote_thread,
                                                     session, 0, NULL);
            }
            session->thread = CreateThread(NULL, 0, chat_thread, session, 0, NULL);
            if (!session->thread) {
                InterlockedExchange(&session->stop, 1);
                if (session->emote_event) SetEvent(session->emote_event);
                if (session->emote_thread) {
                    WaitForSingleObject(session->emote_thread, INFINITE);
                    CloseHandle(session->emote_thread);
                }
                if (session->emote_event) CloseHandle(session->emote_event);
                DeleteCriticalSection(&session->lock);
                free(session);
                session = NULL;
            } else {
                chat_sessions[free_index] = session;
            }
        }
    }
    LeaveCriticalSection(&chat_sessions_lock);
    return session;
}

webm_twitch_chat_session_t *webm_twitch_chat_retain(
    webm_twitch_chat_session_t *session)
{
    if (session) InterlockedIncrement(&session->references);
    return session;
}

void webm_twitch_chat_release(webm_twitch_chat_session_t *session)
{
    HINTERNET websocket = NULL;
    int i;
    if (!session) return;
    ensure_sessions_lock();
    EnterCriticalSection(&chat_sessions_lock);
    if (InterlockedDecrement(&session->references) > 0) {
        LeaveCriticalSection(&chat_sessions_lock);
        return;
    }
    for (i = 0; i < CHAT_SESSION_COUNT; i++) {
        if (chat_sessions[i] == session) chat_sessions[i] = NULL;
    }
    InterlockedExchange(&session->stop, 1);
    if (session->emote_event) SetEvent(session->emote_event);
    EnterCriticalSection(&session->lock);
    websocket = session->websocket;
    LeaveCriticalSection(&session->lock);
    if (websocket) WinHttpWebSocketClose(websocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
    LeaveCriticalSection(&chat_sessions_lock);
    if (session->thread) {
        WaitForSingleObject(session->thread, INFINITE);
        CloseHandle(session->thread);
    }
    if (session->emote_thread) {
        WaitForSingleObject(session->emote_thread, INFINITE);
        CloseHandle(session->emote_thread);
    }
    if (session->catalog_thread) {
        WaitForSingleObject(session->catalog_thread, INFINITE);
        CloseHandle(session->catalog_thread);
    }
    if (session->emote_event) CloseHandle(session->emote_event);
    for (i = 0; i < CHAT_RENDER_CACHE_COUNT; i++) {
        free(session->render_cache[i].pixels);
        free(session->render_cache[i].row_bounds);
        if (session->render_cache[i].font) DeleteObject(session->render_cache[i].font);
    }
    for (i = 0; i < CHAT_EMOTE_CACHE_COUNT; i++) {
        free(session->emotes[i].pixels);
        free(session->emotes[i].frame_delays);
    }
    webm_twitch_emote_catalog_destroy(&session->provider_catalog);
    free(session->frame_scratch);
    free(session->resize_x_offsets);
    DeleteCriticalSection(&session->lock);
    free(session);
}

static void pixel_read(const unsigned char *pixel, webm_twitch_chat_pixel_format_t format,
                       int bpp, int *r, int *g, int *b)
{
    if (format == WEBM_TWITCH_CHAT_RGB565) {
        unsigned int value = (unsigned int)pixel[0] | ((unsigned int)pixel[1] << 8);
        *r = (int)(((value >> 11) & 31) * 255 / 31);
        *g = (int)(((value >> 5) & 63) * 255 / 63);
        *b = (int)((value & 31) * 255 / 31);
    } else if (format == WEBM_TWITCH_CHAT_BGR) {
        *b = pixel[0]; *g = pixel[1]; *r = pixel[2];
    } else {
        *r = pixel[0]; *g = pixel[1]; *b = pixel[2];
    }
    (void)bpp;
}

static void pixel_write(unsigned char *pixel, webm_twitch_chat_pixel_format_t format,
                        int bpp, int r, int g, int b)
{
    if (format == WEBM_TWITCH_CHAT_RGB565) {
        unsigned int value = ((unsigned int)(r >> 3) << 11) |
                             ((unsigned int)(g >> 2) << 5) |
                             (unsigned int)(b >> 3);
        pixel[0] = (unsigned char)(value & 255);
        pixel[1] = (unsigned char)(value >> 8);
    } else if (format == WEBM_TWITCH_CHAT_BGR) {
        pixel[0] = (unsigned char)b; pixel[1] = (unsigned char)g; pixel[2] = (unsigned char)r;
        if (bpp == 4) pixel[3] = 255;
    } else {
        pixel[0] = (unsigned char)r; pixel[1] = (unsigned char)g; pixel[2] = (unsigned char)b;
        if (bpp == 4) pixel[3] = 255;
    }
}

static void fill_black_pixels(unsigned char *pixels, int count, int bpp)
{
    int x;
    if (!pixels || count <= 0 || bpp <= 0) return;
    if (bpp == 4) {
        unsigned int *pixel = (unsigned int*)pixels;
        for (x = 0; x < count; x++) pixel[x] = 0xFF000000u;
    } else {
        memset(pixels, 0, (size_t)count * (size_t)bpp);
    }
}

static int prepare_resize_x_offsets(webm_twitch_chat_session_t *session,
                                    int source_width, int target_width, int bpp)
{
    int x;
    int source_x = 0;
    int source_x_accumulator = 0;
    int *offsets;
    if (!session || source_width < 1 || target_width < 1 || bpp < 1) return 0;
    if (session->resize_x_offsets && session->resize_x_count == target_width &&
        session->resize_source_width == source_width &&
        session->resize_target_width == target_width &&
        session->resize_bytes_per_pixel == bpp) {
        return 1;
    }
    offsets = (int*)realloc(session->resize_x_offsets,
                            (size_t)target_width * sizeof(*offsets));
    if (!offsets) return 0;
    session->resize_x_offsets = offsets;
    for (x = 0; x < target_width; x++) {
        offsets[x] = source_x * bpp;
        source_x_accumulator += source_width;
        while (source_x_accumulator >= target_width) {
            source_x++;
            source_x_accumulator -= target_width;
        }
    }
    session->resize_x_count = target_width;
    session->resize_source_width = source_width;
    session->resize_target_width = target_width;
    session->resize_bytes_per_pixel = bpp;
    return 1;
}

static void resize_video_for_chat(webm_twitch_chat_session_t *session,
                                  unsigned char *pixels, int width, int height, int pitch,
                                  int chat_width, int chat_left, int bpp)
{
    int y, x;
    int video_width = width - chat_width;
    int video_height;
    int video_left;
    int video_top;
    size_t scratch_pitch;
    size_t scratch_bytes;
    if (!session || video_width < 1) return;
    video_height = (height * video_width + width / 2) / width;
    if (video_height < 1) video_height = 1;
    if (video_height > height) video_height = height;
    scratch_pitch = (size_t)video_width * (size_t)bpp;
    scratch_bytes = scratch_pitch * (size_t)video_height;
    if (session->frame_scratch_size < scratch_bytes) {
        unsigned char *grown = (unsigned char*)realloc(session->frame_scratch,
                                                        scratch_bytes);
        if (!grown) return;
        session->frame_scratch = grown;
        session->frame_scratch_size = scratch_bytes;
    }
    if (!prepare_resize_x_offsets(session, width, video_width, bpp)) return;

    /* Scale into the fitted-size scratch buffer before touching the source.
       This avoids copying the full source frame solely to make the in-place
       resize safe. */
    for (y = 0; y < video_height; y++) {
        int source_y = (y * height) / video_height;
        const unsigned char *source_row = pixels +
                                          (size_t)source_y * (size_t)pitch;
        unsigned char *target_pixel = session->frame_scratch +
                                      (size_t)y * scratch_pitch;
        if (bpp == 4) {
            unsigned int *target = (unsigned int*)target_pixel;
            for (x = 0; x < video_width; x++) {
                unsigned int value;
                memcpy(&value, source_row + session->resize_x_offsets[x],
                       sizeof(value));
                target[x] = value;
            }
        } else if (bpp == 2) {
            unsigned short *target = (unsigned short*)target_pixel;
            for (x = 0; x < video_width; x++) {
                unsigned short value;
                memcpy(&value, source_row + session->resize_x_offsets[x],
                       sizeof(value));
                target[x] = value;
            }
        } else {
            for (x = 0; x < video_width; x++) {
                const unsigned char *source_pixel =
                    source_row + session->resize_x_offsets[x];
                target_pixel[0] = source_pixel[0];
                target_pixel[1] = source_pixel[1];
                target_pixel[2] = source_pixel[2];
                target_pixel += 3;
            }
        }
    }

    video_left = chat_left ? chat_width : 0;
    video_top = (height - video_height) / 2;
    /* Write the fitted video and its surrounding black regions in one pass. */
    for (y = 0; y < height; y++) {
        unsigned char *target_row = pixels + (size_t)y * (size_t)pitch;
        if (y < video_top || y >= video_top + video_height) {
            fill_black_pixels(target_row, width, bpp);
        } else {
            const unsigned char *source_row = session->frame_scratch +
                (size_t)(y - video_top) * scratch_pitch;
            if (video_left > 0) fill_black_pixels(target_row, video_left, bpp);
            memcpy(target_row + (size_t)video_left * (size_t)bpp,
                   source_row, scratch_pitch);
            if (video_left + video_width < width) {
                fill_black_pixels(target_row +
                                  (size_t)(video_left + video_width) * (size_t)bpp,
                                  width - video_left - video_width, bpp);
            }
        }
    }
}

static int utf8_to_wide(const char *text, wchar_t *wide, int wide_count)
{
    if (!text || !wide || wide_count <= 0) return 0;
    if (MultiByteToWideChar(CP_UTF8, 0, text, -1, wide, wide_count) > 0) return 1;
    return MultiByteToWideChar(CP_ACP, 0, text, -1, wide, wide_count) > 0;
}

typedef struct {
    HDC dc;
    unsigned int *overlay;
    int overlay_width;
    int overlay_height;
    int left;
    int right;
    int x;
    int y;
    int font_height;
    int line_height;
    int line_count;
    int draw;
} webm_twitch_chat_layout_t;

static int layout_new_line(webm_twitch_chat_layout_t *layout)
{
    int next_y;
    if (!layout) return 0;
    next_y = layout->y + layout->line_height + 2;
    if (next_y + layout->font_height > layout->overlay_height - CHAT_PADDING_Y) return 0;
    layout->y += layout->line_height + 2;
    layout->x = layout->left;
    layout->line_height = layout->font_height + 2;
    layout->line_count++;
    return 1;
}

static void layout_text(webm_twitch_chat_layout_t *layout,
                        const char *text, COLORREF color)
{
    wchar_t wide[CHAT_RUN_TEXT_MAX + CHAT_NAME_MAX + 8];
    int i;
    if (!layout || !text || !text[0] ||
        !utf8_to_wide(text, wide, (int)(sizeof(wide) / sizeof(wide[0])))) return;
    if (layout->draw) SetTextColor(layout->dc, color);
    for (i = 0; wide[i]; i++) {
        SIZE size;
        wchar_t c = wide[i];
        if (c == L'\r') continue;
        if (c == L'\n') {
            if (!layout_new_line(layout)) break;
            continue;
        }
        if (!GetTextExtentPoint32W(layout->dc, &c, 1, &size)) {
            size.cx = layout->font_height / 2;
        }
        if (layout->x + size.cx > layout->right && layout->x > layout->left) {
            if (!layout_new_line(layout)) break;
            if (c == L' ') continue;
        }
        if (layout->draw) TextOutW(layout->dc, layout->x, layout->y, &c, 1);
        layout->x += size.cx;
    }
}

static int emote_frame_index(const webm_twitch_chat_emote_t *emote, DWORD now,
                             int animated_enabled, int animation_fps)
{
    DWORD elapsed;
    DWORD offset;
    DWORD quantum;
    DWORD accumulated = 0;
    int i;
    if (!animated_enabled || !emote || emote->frame_count <= 1 ||
        !emote->frame_delays || !emote->animation_duration) return 0;
    if (animation_fps < 1) animation_fps = 1;
    if (animation_fps > 30) animation_fps = 30;
    quantum = 1000u / (DWORD)animation_fps;
    if (!quantum) quantum = 1;
    elapsed = (DWORD)(now - emote->animation_start_tick);
    elapsed = (elapsed / quantum) * quantum;
    offset = elapsed % emote->animation_duration;
    for (i = 0; i < emote->frame_count; i++) {
        accumulated += emote->frame_delays[i];
        if (offset < accumulated) return i;
    }
    return emote->frame_count - 1;
}

static const unsigned char *emote_frame_pixels(
    const webm_twitch_chat_emote_t *emote, DWORD now,
    int animated_enabled, int animation_fps)
{
    int frame_index = emote_frame_index(emote, now, animated_enabled, animation_fps);
    if (!emote || !emote->pixels || !emote->frame_stride) return NULL;
    return emote->pixels + (size_t)frame_index * emote->frame_stride;
}

static unsigned int visible_animation_key_locked(
    webm_twitch_chat_session_t *session,
    const webm_twitch_chat_message_t *messages,
    int message_start, int visible_start, int visible_count, DWORD now,
    int animated_enabled, int animation_fps)
{
    unsigned int key = 2166136261u;
    int found = 0;
    int i, j;
    if (!session || !messages || !animated_enabled) return 0;
    for (i = 0; i < visible_count; i++) {
        const webm_twitch_chat_message_t *message =
            &messages[(message_start + visible_start + i) % CHAT_MESSAGE_COUNT];
        for (j = 0; j < message->run_count; j++) {
            const webm_twitch_chat_run_t *run = &message->runs[j];
            webm_twitch_chat_emote_t *emote;
            int frame_index;
            const unsigned char *p;
            if (run->type != CHAT_RUN_EMOTE || !run->emote_id[0]) continue;
            emote = find_emote_locked(session, run->emote_id);
            if (!emote || emote->state != CHAT_EMOTE_READY ||
                emote->frame_count <= 1) continue;
            frame_index = emote_frame_index(emote, now, animated_enabled, animation_fps);
            for (p = (const unsigned char*)run->emote_id; *p; p++) {
                key = (key ^ *p) * 16777619u;
            }
            key = (key ^ (unsigned int)(frame_index + 1)) * 16777619u;
            found = 1;
        }
    }
    return found ? key : 0;
}

static void draw_emote_image(unsigned int *overlay, int overlay_width, int overlay_height,
                             int x0, int y0, int target_width, int target_height,
                             const webm_twitch_chat_emote_t *emote,
                             const unsigned char *frame_pixels)
{
    int x, y;
    if (!overlay || !emote || !frame_pixels || emote->width < 1 || emote->height < 1 ||
        target_width < 1 || target_height < 1) return;
    for (y = 0; y < target_height; y++) {
        int target_y = y0 + y;
        int source_y = (y * emote->height) / target_height;
        if (target_y < 0 || target_y >= overlay_height) continue;
        for (x = 0; x < target_width; x++) {
            int target_x = x0 + x;
            int source_x = (x * emote->width) / target_width;
            const unsigned char *source;
            if (target_x < 0 || target_x >= overlay_width) continue;
            source = frame_pixels +
                     ((size_t)source_y * (size_t)emote->width + (size_t)source_x) * 4u;
            overlay[(size_t)target_y * (size_t)overlay_width + (size_t)target_x] =
                source[3] ? ((unsigned int)source[0] |
                             ((unsigned int)source[1] << 8) |
                             ((unsigned int)source[2] << 16) |
                             ((unsigned int)source[3] << 24)) : 0;
        }
    }
}

static int layout_mixed_message(webm_twitch_chat_session_t *session,
                                HDC dc, unsigned int *overlay,
                                int overlay_width, int overlay_height,
                                int left, int right, int start_y, int font_height,
                                float emote_scale,
                                const webm_twitch_chat_message_t *message, int draw,
                                DWORD now, int animated_enabled, int animation_fps)
{
    webm_twitch_chat_layout_t layout;
    char name[CHAT_NAME_MAX + 4];
    int i;
    if (!session || !dc || !message) return font_height + 2;
    memset(&layout, 0, sizeof(layout));
    layout.dc = dc;
    layout.overlay = overlay;
    layout.overlay_width = overlay_width;
    layout.overlay_height = overlay_height;
    layout.left = left;
    layout.right = right;
    layout.x = left;
    layout.y = start_y;
    layout.font_height = font_height;
    layout.line_height = font_height + 2;
    layout.line_count = 1;
    layout.draw = draw;
    _snprintf(name, sizeof(name) - 1, "%s: ", message->name);
    name[sizeof(name) - 1] = 0;
    layout_text(&layout, name, message->color);
    for (i = 0; i < message->run_count; i++) {
        const webm_twitch_chat_run_t *run = &message->runs[i];
        if (run->type == CHAT_RUN_EMOTE && run->emote_id[0]) {
            webm_twitch_chat_emote_t *emote = queue_emote_locked(session, run->emote_id);
            if (emote && emote->state == CHAT_EMOTE_READY && emote->pixels) {
                int target_height = (int)(font_height * emote_scale + 0.5f);
                int target_width;
                if (target_height < 1) target_height = 1;
                target_width = (emote->width * target_height + emote->height / 2) /
                               emote->height;
                if (layout.x + target_width > layout.right && layout.x > layout.left) {
                    if (!layout_new_line(&layout)) break;
                }
                if (target_height > layout.line_height) layout.line_height = target_height;
                if (draw) {
                    const unsigned char *frame_pixels = emote_frame_pixels(
                        emote, now, animated_enabled, animation_fps);
                    draw_emote_image(overlay, overlay_width, overlay_height,
                                     layout.x, layout.y, target_width, target_height,
                                     emote, frame_pixels);
                }
                layout.x += target_width;
                continue;
            }
        }
        layout_text(&layout, run->text, RGB(238, 238, 238));
    }
    return (layout.y - start_y) + layout.line_height;
}

/* Called under the session lock. Keep the original float expression and
 * RGB565 expand/truncate rules, but evaluate each input value only once.
 * On 32-bit GCC, vectorizing the table calculation rounds intermediates to
 * SSE float precision instead of the original scalar x87 precision. */
#if defined(__GNUC__) && defined(__i386__)
__attribute__((optimize("no-tree-vectorize")))
#endif
static void dim_chat_background(webm_twitch_chat_session_t *session,
                                unsigned char *pixels, int height, int pitch,
                                int x0, int chat_width,
                                webm_twitch_chat_pixel_format_t format,
                                int bpp, float opacity)
{
    int x, y;
    if (!session->dim_table_valid || session->dim_table_opacity != opacity) {
        for (x = 0; x < 256; x++)
            session->dim_table[x] = (unsigned char)(int)(x * (1.0f - opacity));
        for (x = 0; x < 32; x++)
            session->dim_table_5[x] = session->dim_table[x * 255 / 31] >> 3;
        for (x = 0; x < 64; x++)
            session->dim_table_6[x] = session->dim_table[x * 255 / 63] >> 2;
        session->dim_table_opacity = opacity;
        session->dim_table_valid = 1;
    }
    for (y = 0; y < height; y++) {
        unsigned char *dst = pixels + (size_t)y * (size_t)pitch + (size_t)x0 * (size_t)bpp;
        if (format == WEBM_TWITCH_CHAT_RGB565) {
            for (x = 0; x < chat_width; x++, dst += bpp) {
                unsigned int value = dst[0] | ((unsigned int)dst[1] << 8);
                unsigned int result = ((unsigned int)session->dim_table_5[value >> 11] << 11) |
                    ((unsigned int)session->dim_table_6[(value >> 5) & 63] << 5) |
                    session->dim_table_5[value & 31];
                dst[0] = (unsigned char)result;
                dst[1] = (unsigned char)(result >> 8);
            }
        } else if (bpp == 4) {
            for (x = 0; x < chat_width; x++, dst += 4) {
                dst[0] = session->dim_table[dst[0]];
                dst[1] = session->dim_table[dst[1]];
                dst[2] = session->dim_table[dst[2]];
                dst[3] = 255;
            }
        } else {
            for (x = 0; x < chat_width; x++, dst += bpp) {
                dst[0] = session->dim_table[dst[0]];
                dst[1] = session->dim_table[dst[1]];
                dst[2] = session->dim_table[dst[2]];
            }
        }
    }
}

/* Bounds describe only pixels the original compositor would write. Cache
 * them alongside the rendered overlay; video pixels still blend every frame. */
static const int *chat_overlay_row_bounds(webm_twitch_chat_session_t *session,
                                          const unsigned int *overlay)
{
    int i, x, y;
    for (i = 0; i < CHAT_RENDER_CACHE_COUNT; i++) {
        webm_twitch_chat_render_cache_t *cache = &session->render_cache[i];
        if (cache->pixels != overlay) continue;
        if (cache->row_bounds_valid) return cache->row_bounds;
        /* A rapidly changing chat may redraw on every video frame. Only
         * build bounds once the overlay is actually reused, so those frames
         * pay just the original compositor scan, not a second full scan. */
        if (!cache->row_bounds_seen) {
            cache->row_bounds_seen = 1;
            return NULL;
        }
        if (cache->row_bounds_height < cache->height) {
            int *grown = (int*)realloc(cache->row_bounds, (size_t)cache->height * 2u * sizeof(int));
            if (!grown) return NULL; /* Retain the rectangular scan on failure. */
            cache->row_bounds = grown;
            cache->row_bounds_height = cache->height;
        }
        memset(cache->row_bounds, 0, (size_t)cache->height * 2u * sizeof(int));
        for (y = cache->content_top; y < cache->content_bottom; y++) {
            const unsigned int *row = overlay + (size_t)y * (size_t)cache->width;
            int left = cache->content_right, right = cache->content_left;
            for (x = cache->content_left; x < cache->content_right; x++) {
                unsigned int value = row[x];
                if ((value >> 24) || (value & 255) > 4 ||
                    ((value >> 8) & 255) > 4 || ((value >> 16) & 255) > 4) {
                    if (left > x) left = x;
                    right = x + 1;
                }
            }
            if (left < right) {
                cache->row_bounds[y * 2] = left;
                cache->row_bounds[y * 2 + 1] = right;
            }
        }
        cache->row_bounds_valid = 1;
        return cache->row_bounds;
    }
    return NULL;
}

static unsigned int *render_chat_overlay(webm_twitch_chat_session_t *session,
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
    if (!cache->font || cache->font_height != font_height) {
        if (cache->font) DeleteObject(cache->font);
        cache->font = CreateFontW(-font_height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
        cache->font_height = font_height;
    }
    font = cache->font;
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
    cache->row_bounds_valid = 0;
    cache->row_bounds_seen = 0;
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
    DeleteObject(bitmap);
    DeleteDC(dc);
    return cache->pixels;
}

/* GDI text and decoded emotes are already BGRA. Opaque pixels can be copied
 * directly into a 32-bit DirectX texture; only translucent emotes need a
 * destination read and channel blending. Keep the generic compositor below
 * for RGB/OpenGL and packed/24-bit destinations. */
static void compose_bgra_chat_overlay(unsigned char *pixels, int height, int pitch,
                                      const unsigned int *overlay, int chat_width, int x0,
                                      int flip, int top, int bottom, int left, int right,
                                      const int *row_bounds)
{
    int x, y;
    for (y = top; y < bottom; y++) {
        int first = row_bounds ? row_bounds[y * 2] : left;
        int last = row_bounds ? row_bounds[y * 2 + 1] : right;
        int target_y = flip ? height - 1 - y : y;
        unsigned char *row = pixels + (size_t)target_y * (size_t)pitch;
        const unsigned int *src = overlay + (size_t)y * (size_t)chat_width;
        for (x = first; x < last; x++) {
            unsigned int value = src[x];
            unsigned int a = value >> 24;
            unsigned char *dst;
            if (!a && (value & 255) <= 4 && ((value >> 8) & 255) <= 4 &&
                ((value >> 16) & 255) <= 4) continue;
            dst = row + (size_t)(x0 + x) * 4u;
            if (a && a < 255) {
                unsigned int inverse = 255 - a;
                unsigned int b = ((value & 255) * a + dst[0] * inverse + 127) / 255;
                unsigned int g = (((value >> 8) & 255) * a + dst[1] * inverse + 127) / 255;
                unsigned int r = (((value >> 16) & 255) * a + dst[2] * inverse + 127) / 255;
                value = b | (g << 8) | (r << 16);
            }
            value |= 0xFF000000u;
            memcpy(dst, &value, sizeof(value));
        }
    }
}

void webm_twitch_chat_compose(webm_twitch_chat_session_t *session,
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
        dim_chat_background(session, pixels, height, pitch, x0, chat_width,
                            format, bytes_per_pixel, opacity);
    }

    message_count = session->message_count;
    message_start = session->message_start;
    message_serial = session->message_serial;
    status_serial = session->status_serial;
    connected = session->connected;
    copy_string(status, sizeof(status), session->status);
    animation_now = GetTickCount();
    overlay = render_chat_overlay(session, session->messages,
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
        const int *row_bounds = chat_overlay_row_bounds(session, overlay);
        if (format == WEBM_TWITCH_CHAT_BGR && bytes_per_pixel == 4) {
            compose_bgra_chat_overlay(pixels, height, pitch, overlay, chat_width, x0,
                                      flip_text_vertical, content_top, content_bottom,
                                      content_left, content_right, row_bounds);
        } else {
            for (y = content_top; y < content_bottom; y++) {
                int left = row_bounds ? row_bounds[y * 2] : content_left;
                int right = row_bounds ? row_bounds[y * 2 + 1] : content_right;
                int target_y = flip_text_vertical ? height - 1 - y : y;
                unsigned char *row = pixels + (size_t)target_y * (size_t)pitch;
                unsigned int *source = overlay + (size_t)y * (size_t)chat_width;
                for (x = left; x < right; x++) {
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
    }
    LeaveCriticalSection(&session->lock);
}

void webm_twitch_chat_scale_frame(const unsigned char *source,
                                  int source_width, int source_height, int source_pitch,
                                  unsigned char *target,
                                  int target_width, int target_height, int target_pitch,
                                  int bytes_per_pixel)
{
    int x, y, channel;
    int source_y0 = 0;
    int source_y_accumulator = 0;
    if (!source || !target || source_width < 1 || source_height < 1 ||
        target_width < 1 || target_height < 1 || bytes_per_pixel < 2 ||
        source_pitch < source_width * bytes_per_pixel ||
        target_pitch < target_width * bytes_per_pixel) return;
    for (y = 0; y < target_height; y++) {
        int source_y1;
        int next_source_y = source_y0;
        int source_x0 = 0;
        int source_x_accumulator = 0;
        unsigned char *dst = target + (size_t)y * (size_t)target_pitch;
        source_y_accumulator += source_height;
        while (source_y_accumulator >= target_height) {
            next_source_y++;
            source_y_accumulator -= target_height;
        }
        source_y1 = next_source_y - 1;
        if (source_y1 < source_y0) source_y1 = source_y0;
        if (source_y1 >= source_height) source_y1 = source_height - 1;
        for (x = 0; x < target_width; x++) {
            int source_x1;
            int next_source_x = source_x0;
            const unsigned char *p00;
            source_x_accumulator += source_width;
            while (source_x_accumulator >= target_width) {
                next_source_x++;
                source_x_accumulator -= target_width;
            }
            source_x1 = next_source_x - 1;
            if (source_x1 < source_x0) source_x1 = source_x0;
            if (source_x1 >= source_width) source_x1 = source_width - 1;
            p00 = source + (size_t)source_y0 * (size_t)source_pitch +
                  (size_t)source_x0 * (size_t)bytes_per_pixel;
            if (bytes_per_pixel == 2) {
                dst[0] = p00[0];
                dst[1] = p00[1];
            } else {
                const unsigned char *p10 = source + (size_t)source_y0 * (size_t)source_pitch +
                                           (size_t)source_x1 * (size_t)bytes_per_pixel;
                const unsigned char *p01 = source + (size_t)source_y1 * (size_t)source_pitch +
                                           (size_t)source_x0 * (size_t)bytes_per_pixel;
                const unsigned char *p11 = source + (size_t)source_y1 * (size_t)source_pitch +
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
            source_x0 = next_source_x;
        }
        source_y0 = next_source_y;
    }
}

void webm_twitch_chat_downsample_half(const unsigned char *source,
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
    /* Ordinary 32-bit mip levels need no per-pixel format dispatch or edge
     * clamping. Keep the identical four-sample sum and +2 rounding, including
     * alpha; edge-clamped sizes and other formats retain the generic path. */
    if (bytes_per_pixel == 4 && target_width <= source_width / 2 &&
        target_height <= source_height / 2) {
        for (y = 0; y < target_height; y++) {
            const unsigned char *row0 = source + (size_t)(y * 2) * (size_t)source_pitch;
            const unsigned char *row1 = row0 + source_pitch;
            unsigned char *dst = target + (size_t)y * (size_t)target_pitch;
            for (x = 0; x < target_width; x++) {
                unsigned int a, b, c, d, rb, ga, result;
                memcpy(&a, row0 + (size_t)x * 8u, 4);
                memcpy(&b, row0 + (size_t)x * 8u + 4u, 4);
                memcpy(&c, row1 + (size_t)x * 8u, 4);
                memcpy(&d, row1 + (size_t)x * 8u + 4u, 4);
                rb = (a & 0x00FF00FFu) + (b & 0x00FF00FFu) +
                     (c & 0x00FF00FFu) + (d & 0x00FF00FFu) + 0x00020002u;
                ga = ((a >> 8) & 0x00FF00FFu) + ((b >> 8) & 0x00FF00FFu) +
                     ((c >> 8) & 0x00FF00FFu) + ((d >> 8) & 0x00FF00FFu) + 0x00020002u;
                result = ((rb >> 2) & 0x00FF00FFu) | (((ga >> 2) & 0x00FF00FFu) << 8);
                memcpy(dst + (size_t)x * 4u, &result, 4);
            }
        }
        return;
    }
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
