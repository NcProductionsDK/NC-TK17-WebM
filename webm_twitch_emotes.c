#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>

#include "webm_twitch_emotes.h"

#define EMOTE_CATALOG_CAPACITY 4096
#define EMOTE_CATALOG_DOWNLOAD_MAX (4 * 1024 * 1024)
#define EMOTE_IMAGE_DOWNLOAD_MAX (4 * 1024 * 1024)

static unsigned int hash_code(const char *code, size_t length)
{
    unsigned int hash = 2166136261u;
    size_t i;
    for (i = 0; i < length; i++) {
        hash ^= (unsigned char)code[i];
        hash *= 16777619u;
    }
    return hash;
}

static void copy_string(char *dst, size_t dst_size, const char *src)
{
    if (!dst || !dst_size) return;
    lstrcpynA(dst, src ? src : "", (int)dst_size);
    dst[dst_size - 1] = 0;
}

static int utf8_to_wide(const char *text, wchar_t *wide, size_t wide_count)
{
    if (!text || !wide || !wide_count) return 0;
    wide[0] = 0;
    return MultiByteToWideChar(CP_UTF8, 0, text, -1, wide,
                               (int)wide_count) > 0;
}

static int stopped(volatile LONG *stop)
{
    return stop && InterlockedCompareExchange(stop, 0, 0) != 0;
}

static const char *compound_end(const char *start)
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
            if (!depth) return p + 1;
        }
    }
    return NULL;
}

static const char *find_value_range(const char *start, const char *end, const char *key)
{
    char marker[128];
    const char *p = start;
    size_t marker_length;
    if (!start || !end || !key || start >= end) return NULL;
    _snprintf(marker, sizeof(marker) - 1, "\"%s\"", key);
    marker[sizeof(marker) - 1] = 0;
    marker_length = strlen(marker);
    while (p + marker_length < end) {
        p = strstr(p, marker);
        if (!p || p + marker_length >= end) return NULL;
        p += marker_length;
        while (p < end && isspace((unsigned char)*p)) p++;
        if (p < end && *p == ':') {
            p++;
            while (p < end && isspace((unsigned char)*p)) p++;
            return p < end ? p : NULL;
        }
    }
    return NULL;
}

static int string_value_range(const char *start, const char *end, const char *key,
                              char *out, size_t out_size)
{
    const char *p = find_value_range(start, end, key);
    size_t used = 0;
    if (!out || !out_size) return 0;
    out[0] = 0;
    if (!p || p >= end || *p++ != '"') return 0;
    while (p < end && *p != '"' && used + 1 < out_size) {
        if (*p == '\\' && p + 1 < end) {
            p++;
            if (*p == 'n') out[used++] = '\n';
            else if (*p == 'r') out[used++] = '\r';
            else if (*p == 't') out[used++] = '\t';
            else out[used++] = *p;
            p++;
        } else {
            out[used++] = *p++;
        }
    }
    out[used] = 0;
    return p < end && *p == '"';
}

static int catalog_insert(webm_twitch_emote_catalog_t *catalog,
                          const char *code, const char *provider_id)
{
    size_t code_length;
    size_t index;
    size_t probe;
    if (!catalog || !catalog->entries || !code || !provider_id ||
        !code[0] || !provider_id[0]) return 0;
    code_length = strlen(code);
    if (code_length >= WEBM_TWITCH_EMOTE_CODE_MAX ||
        strlen(provider_id) >= WEBM_TWITCH_EMOTE_ID_MAX) return 0;
    index = hash_code(code, code_length) & (catalog->capacity - 1);
    for (probe = 0; probe < catalog->capacity; probe++) {
        webm_twitch_provider_emote_t *entry =
            &catalog->entries[(index + probe) & (catalog->capacity - 1)];
        if (!entry->code[0]) {
            copy_string(entry->code, sizeof(entry->code), code);
            copy_string(entry->id, sizeof(entry->id), provider_id);
            catalog->count++;
            return 1;
        }
        if (strcmp(entry->code, code) == 0) {
            copy_string(entry->id, sizeof(entry->id), provider_id);
            return 1;
        }
    }
    return 0;
}

static int parse_emote_array(webm_twitch_emote_catalog_t *catalog,
                             const char *array, const char *name_key,
                             char provider_prefix)
{
    const char *end;
    const char *p;
    int added = 0;
    if (!array || *array != '[') return 0;
    end = compound_end(array);
    if (!end) return 0;
    p = array + 1;
    while (p < end) {
        const char *object;
        const char *object_end;
        char id[48];
        char code[WEBM_TWITCH_EMOTE_CODE_MAX];
        char image_type[16];
        char provider_id[WEBM_TWITCH_EMOTE_ID_MAX];
        while (p < end && *p != '{') p++;
        if (p >= end) break;
        object = p;
        object_end = compound_end(object);
        if (!object_end || object_end > end) break;
        id[0] = code[0] = image_type[0] = 0;
        string_value_range(object, object_end, "id", id, sizeof(id));
        string_value_range(object, object_end, name_key, code, sizeof(code));
        string_value_range(object, object_end, "imageType", image_type,
                           sizeof(image_type));
        if (id[0] && code[0]) {
            if (provider_prefix == '7') {
                const char *animated = find_value_range(object, object_end, "animated");
                _snprintf(provider_id, sizeof(provider_id) - 1, "7%c:%s",
                          animated && strncmp(animated, "true", 4) == 0 ? 'g' : 'p', id);
            } else if (provider_prefix == 'b') {
                _snprintf(provider_id, sizeof(provider_id) - 1, "b%c:%s",
                          _stricmp(image_type, "gif") == 0 ? 'g' : 'p', id);
            } else {
                _snprintf(provider_id, sizeof(provider_id) - 1,
                          "%c:%s", provider_prefix, id);
            }
            provider_id[sizeof(provider_id) - 1] = 0;
            if (catalog_insert(catalog, code, provider_id)) added++;
        }
        p = object_end;
    }
    return added;
}

static int http_get(HINTERNET internet, const wchar_t *host, const wchar_t *path,
                    size_t maximum_size, volatile LONG *stop,
                    unsigned char **data_out, size_t *size_out)
{
    HINTERNET connection = NULL;
    HINTERNET request = NULL;
    unsigned char *data = NULL;
    size_t size = 0;
    DWORD status = 0;
    DWORD status_size = sizeof(status);
    int ok = 0;
    if (data_out) *data_out = NULL;
    if (size_out) *size_out = 0;
    if (!internet || !host || !path || !data_out || !size_out) return 0;
    connection = WinHttpConnect(internet, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connection) goto done;
    request = WinHttpOpenRequest(connection, L"GET", path, NULL,
                                 WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                 WINHTTP_FLAG_SECURE);
    if (!request ||
        !WinHttpSendRequest(request, L"Accept: */*\r\n", (DWORD)-1L,
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
        if (stopped(stop) || !WinHttpQueryDataAvailable(request, &available)) goto done;
        if (!available) break;
        if (size + available > maximum_size) goto done;
        grown = (unsigned char*)realloc(data, size + available + 1);
        if (!grown) goto done;
        data = grown;
        if (!WinHttpReadData(request, data + size, available, &read) || !read) goto done;
        size += read;
    }
    if (size) {
        data[size] = 0;
        ok = 1;
    }
done:
    if (request) WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    if (!ok) {
        free(data);
        return 0;
    }
    *data_out = data;
    *size_out = size;
    return 1;
}

static int load_bttv(webm_twitch_emote_catalog_t *catalog, HINTERNET internet,
                     const char *user_id, volatile LONG *stop)
{
    unsigned char *json = NULL;
    size_t size = 0;
    wchar_t path[160];
    wchar_t user_id_w[64];
    int added = 0;
    if (http_get(internet, L"api.betterttv.net", L"/3/cached/emotes/global",
                 EMOTE_CATALOG_DOWNLOAD_MAX, stop, &json, &size)) {
        added += parse_emote_array(catalog, (const char*)json, "code", 'b');
        free(json);
        json = NULL;
    }
    if (stopped(stop) || !user_id || !user_id[0]) return added;
    if (!utf8_to_wide(user_id, user_id_w,
                      sizeof(user_id_w) / sizeof(user_id_w[0]))) return added;
    _snwprintf(path, (sizeof(path) / sizeof(path[0])) - 1,
               L"/3/cached/users/twitch/%ls", user_id_w);
    path[(sizeof(path) / sizeof(path[0])) - 1] = 0;
    if (http_get(internet, L"api.betterttv.net", path,
                 EMOTE_CATALOG_DOWNLOAD_MAX, stop, &json, &size)) {
        const char *end = (const char*)json + size;
        const char *channel = find_value_range((const char*)json, end, "channelEmotes");
        const char *shared = find_value_range((const char*)json, end, "sharedEmotes");
        added += parse_emote_array(catalog, channel, "code", 'b');
        added += parse_emote_array(catalog, shared, "code", 'b');
        free(json);
    }
    return added;
}

static int load_seventv(webm_twitch_emote_catalog_t *catalog, HINTERNET internet,
                        const char *user_id, volatile LONG *stop)
{
    unsigned char *json = NULL;
    size_t size = 0;
    wchar_t path[160];
    wchar_t user_id_w[64];
    int added = 0;
    if (http_get(internet, L"7tv.io", L"/v3/emote-sets/global",
                 EMOTE_CATALOG_DOWNLOAD_MAX, stop, &json, &size)) {
        const char *end = (const char*)json + size;
        const char *emotes = find_value_range((const char*)json, end, "emotes");
        added += parse_emote_array(catalog, emotes, "name", '7');
        free(json);
        json = NULL;
    }
    if (stopped(stop) || !user_id || !user_id[0]) return added;
    if (!utf8_to_wide(user_id, user_id_w,
                      sizeof(user_id_w) / sizeof(user_id_w[0]))) return added;
    _snwprintf(path, (sizeof(path) / sizeof(path[0])) - 1,
               L"/v3/users/twitch/%ls", user_id_w);
    path[(sizeof(path) / sizeof(path[0])) - 1] = 0;
    if (http_get(internet, L"7tv.io", path,
                 EMOTE_CATALOG_DOWNLOAD_MAX, stop, &json, &size)) {
        const char *end = (const char*)json + size;
        const char *set = find_value_range((const char*)json, end, "emote_set");
        const char *set_end = set && *set == '{' ? compound_end(set) : NULL;
        const char *emotes = set_end ? find_value_range(set, set_end, "emotes") : NULL;
        added += parse_emote_array(catalog, emotes, "name", '7');
        free(json);
    }
    return added;
}

int webm_twitch_emote_catalog_load(webm_twitch_emote_catalog_t *catalog,
                                    const char *twitch_user_id,
                                    int load_bttv_enabled, int load_seventv_enabled,
                                    volatile LONG *stop)
{
    HINTERNET internet;
    if (!catalog || (!load_bttv_enabled && !load_seventv_enabled)) return 0;
    memset(catalog, 0, sizeof(*catalog));
    catalog->capacity = EMOTE_CATALOG_CAPACITY;
    catalog->entries = (webm_twitch_provider_emote_t*)calloc(
        catalog->capacity, sizeof(*catalog->entries));
    if (!catalog->entries) return 0;
    internet = WinHttpOpen(L"NC-TK17-WebM Third-party Emotes/1.0",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!internet) {
        webm_twitch_emote_catalog_destroy(catalog);
        return 0;
    }
    WinHttpSetTimeouts(internet, 5000, 5000, 5000, 10000);
    if (load_bttv_enabled && !stopped(stop)) {
        catalog->bttv_count = load_bttv(catalog, internet, twitch_user_id, stop);
    }
    if (load_seventv_enabled && !stopped(stop)) {
        catalog->seventv_count = load_seventv(catalog, internet, twitch_user_id, stop);
    }
    WinHttpCloseHandle(internet);
    if (!catalog->count) {
        webm_twitch_emote_catalog_destroy(catalog);
        return 0;
    }
    return 1;
}

void webm_twitch_emote_catalog_destroy(webm_twitch_emote_catalog_t *catalog)
{
    if (!catalog) return;
    free(catalog->entries);
    memset(catalog, 0, sizeof(*catalog));
}

const char *webm_twitch_emote_catalog_lookup(
    const webm_twitch_emote_catalog_t *catalog, const char *code, size_t code_length)
{
    size_t index;
    size_t probe;
    if (!catalog || !catalog->entries || !catalog->capacity || !code || !code_length ||
        code_length >= WEBM_TWITCH_EMOTE_CODE_MAX) return NULL;
    index = hash_code(code, code_length) & (catalog->capacity - 1);
    for (probe = 0; probe < catalog->capacity; probe++) {
        const webm_twitch_provider_emote_t *entry =
            &catalog->entries[(index + probe) & (catalog->capacity - 1)];
        if (!entry->code[0]) return NULL;
        if (strlen(entry->code) == code_length &&
            memcmp(entry->code, code, code_length) == 0) return entry->id;
    }
    return NULL;
}

int webm_twitch_emote_download(const char *id, unsigned char **data_out,
                               size_t *size_out)
{
    HINTERNET internet;
    wchar_t host[96];
    wchar_t path[256];
    wchar_t raw_id_w[WEBM_TWITCH_EMOTE_ID_MAX];
    const char *raw_id = id;
    int result;
    if (!id || !id[0] || !data_out || !size_out) return 0;
    if (id[0] == 'b' && id[2] == ':') {
        int animated = id[1] == 'g';
        raw_id = id + 3;
        wcscpy(host, L"cdn.betterttv.net");
        if (!utf8_to_wide(raw_id, raw_id_w,
                          sizeof(raw_id_w) / sizeof(raw_id_w[0]))) return 0;
        _snwprintf(path, (sizeof(path) / sizeof(path[0])) - 1,
                   animated ? L"/emote/%ls/2x.gif" : L"/emote/%ls/2x.png",
                   raw_id_w);
    } else if (id[0] == '7' && id[2] == ':') {
        int animated = id[1] == 'g';
        raw_id = id + 3;
        wcscpy(host, L"cdn.7tv.app");
        if (!utf8_to_wide(raw_id, raw_id_w,
                          sizeof(raw_id_w) / sizeof(raw_id_w[0]))) return 0;
        _snwprintf(path, (sizeof(path) / sizeof(path[0])) - 1,
                   animated ? L"/emote/%ls/2x.gif" : L"/emote/%ls/2x.png",
                   raw_id_w);
    } else if (id[0] == 't' && id[2] == ':') {
        int animated = id[1] == 'g';
        raw_id = id + 3;
        wcscpy(host, L"static-cdn.jtvnw.net");
        if (!utf8_to_wide(raw_id, raw_id_w,
                          sizeof(raw_id_w) / sizeof(raw_id_w[0]))) return 0;
        _snwprintf(path, (sizeof(path) / sizeof(path[0])) - 1,
                   animated ? L"/emoticons/v2/%ls/animated/dark/2.0" :
                              L"/emoticons/v2/%ls/static/dark/2.0",
                   raw_id_w);
    } else {
        wcscpy(host, L"static-cdn.jtvnw.net");
        if (!utf8_to_wide(raw_id, raw_id_w,
                          sizeof(raw_id_w) / sizeof(raw_id_w[0]))) return 0;
        _snwprintf(path, (sizeof(path) / sizeof(path[0])) - 1,
                   L"/emoticons/v2/%ls/static/dark/2.0", raw_id_w);
    }
    path[(sizeof(path) / sizeof(path[0])) - 1] = 0;
    internet = WinHttpOpen(L"NC-TK17-WebM Emotes/1.0",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!internet) return 0;
    WinHttpSetTimeouts(internet, 5000, 5000, 5000, 10000);
    result = http_get(internet, host, path, EMOTE_IMAGE_DOWNLOAD_MAX,
                      NULL, data_out, size_out);
    if (!result && id[0] == 't' && id[1] == 'g' && id[2] == ':') {
        _snwprintf(path, (sizeof(path) / sizeof(path[0])) - 1,
                   L"/emoticons/v2/%ls/static/dark/2.0", raw_id_w);
        path[(sizeof(path) / sizeof(path[0])) - 1] = 0;
        result = http_get(internet, host, path, EMOTE_IMAGE_DOWNLOAD_MAX,
                          NULL, data_out, size_out);
    }
    WinHttpCloseHandle(internet);
    return result;
}
