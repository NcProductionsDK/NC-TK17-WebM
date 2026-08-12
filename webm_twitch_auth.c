#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <wincrypt.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "webm_twitch_auth.h"

#define TWITCH_AUTH_MAGIC "NCTWA01"
#define TWITCH_TOKEN_MAX WEBM_TWITCH_TOKEN_MAX
#define TWITCH_HTTP_LIMIT (2u * 1024u * 1024u)

typedef struct {
    char magic[8];
    char client_id[WEBM_TWITCH_CLIENT_ID_MAX];
    char access_token[TWITCH_TOKEN_MAX];
    char refresh_token[TWITCH_TOKEN_MAX];
} twitch_auth_record_t;

static volatile LONG twitch_auth_busy;

static void copy_string(char *dst, size_t dst_size, const char *src)
{
    if (!dst || dst_size == 0) return;
    if (!src) src = "";
    lstrcpynA(dst, src, (int)dst_size);
    dst[dst_size - 1] = 0;
}

static void set_error(char *error, size_t error_size, const char *message)
{
    copy_string(error, error_size, message);
}

static int cancelled(volatile LONG *cancel)
{
    return cancel && InterlockedCompareExchange(cancel, 0, 0);
}

static int wait_cancelled(DWORD milliseconds, volatile LONG *cancel)
{
    DWORD start = GetTickCount();
    while ((DWORD)(GetTickCount() - start) < milliseconds) {
        if (cancelled(cancel)) return 1;
        Sleep(50);
    }
    return cancelled(cancel);
}

static int to_wide(const char *text, wchar_t *wide, int wide_count)
{
    if (!text || !wide || wide_count <= 0) return 0;
    return MultiByteToWideChar(CP_UTF8, 0, text, -1, wide, wide_count) > 0;
}

static int http_request(const char *host, const char *verb, const char *path,
                        const char *headers, const char *body,
                        char **response, DWORD *status,
                        char *error, size_t error_size)
{
    HINTERNET internet = NULL;
    HINTERNET connection = NULL;
    HINTERNET request = NULL;
    wchar_t host_w[256];
    wchar_t verb_w[16];
    wchar_t path_w[4096];
    wchar_t headers_w[2048];
    BYTE *data = NULL;
    size_t size = 0;
    size_t capacity = 0;
    DWORD code = 0;
    DWORD code_size = sizeof(code);
    int ok = 0;
    if (response) *response = NULL;
    if (status) *status = 0;
    if (!to_wide(host, host_w, 256) || !to_wide(verb, verb_w, 16) ||
        !to_wide(path, path_w, 4096)) {
        set_error(error, error_size, "Twitch HTTP request contains invalid UTF-8");
        return 0;
    }
    headers_w[0] = 0;
    if (headers && headers[0] && !to_wide(headers, headers_w, 2048)) {
        set_error(error, error_size, "Twitch HTTP headers contain invalid UTF-8");
        return 0;
    }
    internet = WinHttpOpen(L"NC-TK17-WebM/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!internet) goto done;
    WinHttpSetTimeouts(internet, 5000, 5000, 5000, 10000);
    connection = WinHttpConnect(internet, host_w, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connection) goto done;
    request = WinHttpOpenRequest(connection, verb_w, path_w, NULL,
                                 WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                 WINHTTP_FLAG_SECURE);
    if (!request) goto done;
    if (!WinHttpSendRequest(request,
                            headers_w[0] ? headers_w : WINHTTP_NO_ADDITIONAL_HEADERS,
                            headers_w[0] ? (DWORD)-1L : 0,
                            body && body[0] ? (LPVOID)body : WINHTTP_NO_REQUEST_DATA,
                            body && body[0] ? (DWORD)strlen(body) : 0,
                            body && body[0] ? (DWORD)strlen(body) : 0, 0) ||
        !WinHttpReceiveResponse(request, NULL)) {
        goto done;
    }
    WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &code, &code_size,
                        WINHTTP_NO_HEADER_INDEX);
    for (;;) {
        DWORD available = 0;
        DWORD read = 0;
        BYTE *grown;
        if (!WinHttpQueryDataAvailable(request, &available)) goto done;
        if (!available) break;
        if (size + available + 1 > TWITCH_HTTP_LIMIT) {
            set_error(error, error_size, "Twitch HTTP response exceeded safety limit");
            goto done;
        }
        if (size + available + 1 > capacity) {
            capacity = size + available + 1;
            grown = (BYTE*)realloc(data, capacity);
            if (!grown) goto done;
            data = grown;
        }
        if (!WinHttpReadData(request, data + size, available, &read)) goto done;
        size += read;
        if (!read) break;
    }
    if (!data) {
        data = (BYTE*)calloc(1, 1);
        if (!data) goto done;
    } else {
        data[size] = 0;
    }
    if (response) {
        *response = (char*)data;
        data = NULL;
    }
    if (status) *status = code;
    ok = 1;

done:
    if (!ok && error && error_size && !error[0]) {
        _snprintf(error, error_size - 1, "Twitch HTTP request failed (Windows error %lu)",
                  (unsigned long)GetLastError());
        error[error_size - 1] = 0;
    }
    free(data);
    if (request) WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    if (internet) WinHttpCloseHandle(internet);
    return ok;
}

static int json_string(const char *json, const char *key, char *out, size_t out_size)
{
    char marker[128];
    const char *p;
    size_t used = 0;
    if (!json || !key || !out || out_size == 0) return 0;
    out[0] = 0;
    _snprintf(marker, sizeof(marker) - 1, "\"%s\"", key);
    marker[sizeof(marker) - 1] = 0;
    p = strstr(json, marker);
    if (!p) return 0;
    p += strlen(marker);
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p++ != ':') return 0;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p++ != '"') return 0;
    while (*p && *p != '"' && used + 1 < out_size) {
        char c = *p++;
        if (c == '\\' && *p) {
            c = *p++;
            if (c == 'n') c = '\n';
            else if (c == 'r') c = '\r';
            else if (c == 't') c = '\t';
        }
        out[used++] = c;
    }
    out[used] = 0;
    return *p == '"';
}

static int json_int(const char *json, const char *key, int *value)
{
    char marker[128];
    const char *p;
    char *end;
    long parsed;
    if (!json || !key || !value) return 0;
    _snprintf(marker, sizeof(marker) - 1, "\"%s\"", key);
    marker[sizeof(marker) - 1] = 0;
    p = strstr(json, marker);
    if (!p) return 0;
    p += strlen(marker);
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p++ != ':') return 0;
    while (*p && isspace((unsigned char)*p)) p++;
    parsed = strtol(p, &end, 10);
    if (end == p) return 0;
    *value = (int)parsed;
    return 1;
}

static int json_bool(const char *json, const char *key, int *value)
{
    char marker[128];
    const char *p;
    if (!json || !key || !value) return 0;
    _snprintf(marker, sizeof(marker) - 1, "\"%s\"", key);
    marker[sizeof(marker) - 1] = 0;
    p = strstr(json, marker);
    if (!p) return 0;
    p += strlen(marker);
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p++ != ':') return 0;
    while (*p && isspace((unsigned char)*p)) p++;
    if (_strnicmp(p, "true", 4) == 0) { *value = 1; return 1; }
    if (_strnicmp(p, "false", 5) == 0) { *value = 0; return 1; }
    return 0;
}

static void url_encode(const char *input, char *output, size_t output_size)
{
    static const char hex[] = "0123456789ABCDEF";
    size_t used = 0;
    const unsigned char *p = (const unsigned char*)(input ? input : "");
    if (!output || output_size == 0) return;
    while (*p && used + 1 < output_size) {
        unsigned char c = *p++;
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            output[used++] = (char)c;
        } else if (used + 3 < output_size) {
            output[used++] = '%';
            output[used++] = hex[c >> 4];
            output[used++] = hex[c & 15];
        } else {
            break;
        }
    }
    output[used] = 0;
}

static void auth_path(const char *binary_dir, char *path, size_t path_size)
{
    _snprintf(path, path_size - 1, "%s\\NC-TK17-WebM-twitch\\auth.dat",
              binary_dir ? binary_dir : "");
    path[path_size - 1] = 0;
}

static int load_auth(const char *binary_dir, const char *client_id, twitch_auth_record_t *record)
{
    char path[MAX_PATH * 3];
    HANDLE file;
    DWORD size;
    DWORD read;
    BYTE *encrypted;
    DATA_BLOB input;
    DATA_BLOB output;
    int ok = 0;
    if (!record) return 0;
    memset(record, 0, sizeof(*record));
    auth_path(binary_dir, path, sizeof(path));
    file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (file == INVALID_HANDLE_VALUE) return 0;
    size = GetFileSize(file, NULL);
    if (!size || size > 65536) { CloseHandle(file); return 0; }
    encrypted = (BYTE*)malloc(size);
    if (!encrypted) { CloseHandle(file); return 0; }
    if (!ReadFile(file, encrypted, size, &read, NULL) || read != size) goto done;
    input.pbData = encrypted;
    input.cbData = size;
    memset(&output, 0, sizeof(output));
    if (!CryptUnprotectData(&input, NULL, NULL, NULL, NULL,
                            CRYPTPROTECT_UI_FORBIDDEN, &output)) goto done;
    if (output.cbData == sizeof(*record)) {
        memcpy(record, output.pbData, sizeof(*record));
        if (memcmp(record->magic, TWITCH_AUTH_MAGIC, 7) == 0 &&
            _stricmp(record->client_id, client_id) == 0 && record->access_token[0]) {
            ok = 1;
        }
    }
    LocalFree(output.pbData);
done:
    free(encrypted);
    CloseHandle(file);
    if (!ok) memset(record, 0, sizeof(*record));
    return ok;
}

static int save_auth(const char *binary_dir, const twitch_auth_record_t *record)
{
    char path[MAX_PATH * 3];
    DATA_BLOB input;
    DATA_BLOB output;
    HANDLE file;
    DWORD written;
    int ok = 0;
    if (!record) return 0;
    auth_path(binary_dir, path, sizeof(path));
    input.pbData = (BYTE*)record;
    input.cbData = sizeof(*record);
    memset(&output, 0, sizeof(output));
    if (!CryptProtectData(&input, L"NC-TK17-WebM Twitch", NULL, NULL, NULL,
                          CRYPTPROTECT_UI_FORBIDDEN, &output)) return 0;
    file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_HIDDEN, NULL);
    if (file != INVALID_HANDLE_VALUE) {
        ok = WriteFile(file, output.pbData, output.cbData, &written, NULL) &&
             written == output.cbData;
        CloseHandle(file);
    }
    LocalFree(output.pbData);
    return ok;
}

static int json_has_scope(const char *json, const char *scope)
{
    const char *scopes;
    const char *end;
    char marker[160];
    if (!scope || !scope[0]) return 1;
    if (!json) return 0;
    scopes = strstr(json, "\"scopes\"");
    if (!scopes || !(scopes = strchr(scopes, '['))) return 0;
    end = strchr(scopes, ']');
    if (!end) return 0;
    _snprintf(marker, sizeof(marker) - 1, "\"%s\"", scope);
    marker[sizeof(marker) - 1] = 0;
    scopes = strstr(scopes, marker);
    return scopes && scopes < end;
}

static int validate_token(const char *client_id, const char *token,
                          const char *required_scope,
                          char *user_id, size_t user_id_size,
                          char *user_login, size_t user_login_size)
{
    char headers[1024];
    char *response = NULL;
    char response_client[WEBM_TWITCH_CLIENT_ID_MAX];
    DWORD status = 0;
    char ignored[128] = "";
    int ok;
    _snprintf(headers, sizeof(headers) - 1, "Authorization: OAuth %s\r\n", token);
    headers[sizeof(headers) - 1] = 0;
    ok = http_request("id.twitch.tv", "GET", "/oauth2/validate", headers, NULL,
                      &response, &status, ignored, sizeof(ignored));
    if (!ok || status != 200 ||
        !json_string(response, "client_id", response_client, sizeof(response_client)) ||
        _stricmp(response_client, client_id) != 0 ||
        !json_has_scope(response, required_scope)) {
        free(response);
        return 0;
    }
    if (user_id && user_id_size) json_string(response, "user_id", user_id, user_id_size);
    if (user_login && user_login_size) json_string(response, "login", user_login, user_login_size);
    free(response);
    return 1;
}

static int parse_token_response(const char *client_id, const char *json,
                                twitch_auth_record_t *record)
{
    char access[TWITCH_TOKEN_MAX];
    char refresh[TWITCH_TOKEN_MAX];
    if (!json_string(json, "access_token", access, sizeof(access))) return 0;
    refresh[0] = 0;
    json_string(json, "refresh_token", refresh, sizeof(refresh));
    memset(record, 0, sizeof(*record));
    memcpy(record->magic, TWITCH_AUTH_MAGIC, 7);
    copy_string(record->client_id, sizeof(record->client_id), client_id);
    copy_string(record->access_token, sizeof(record->access_token), access);
    copy_string(record->refresh_token, sizeof(record->refresh_token), refresh);
    return 1;
}

static int refresh_token(const char *client_id, const char *refresh,
                         twitch_auth_record_t *record,
                         char *error, size_t error_size)
{
    char client_encoded[WEBM_TWITCH_CLIENT_ID_MAX * 3];
    char refresh_encoded[TWITCH_TOKEN_MAX * 3];
    char body[2048];
    char *response = NULL;
    DWORD status = 0;
    int ok;
    url_encode(client_id, client_encoded, sizeof(client_encoded));
    url_encode(refresh, refresh_encoded, sizeof(refresh_encoded));
    _snprintf(body, sizeof(body) - 1,
              "grant_type=refresh_token&refresh_token=%s&client_id=%s",
              refresh_encoded, client_encoded);
    body[sizeof(body) - 1] = 0;
    ok = http_request("id.twitch.tv", "POST", "/oauth2/token",
                      "Content-Type: application/x-www-form-urlencoded\r\n",
                      body, &response, &status, error, error_size);
    if (!ok || status != 200 || !parse_token_response(client_id, response, record)) {
        if (error && error_size && !error[0]) set_error(error, error_size, "Twitch token refresh failed");
        free(response);
        return 0;
    }
    free(response);
    return 1;
}

static int device_authorize(const char *client_id, const char *scopes,
                            volatile LONG *cancel,
                            twitch_auth_record_t *record,
                            char *error, size_t error_size)
{
    char client_encoded[WEBM_TWITCH_CLIENT_ID_MAX * 3];
    char scopes_encoded[512];
    char body[1024];
    char device_code[TWITCH_TOKEN_MAX];
    char verification_uri[1024];
    char message[128];
    char *response = NULL;
    DWORD status = 0;
    DWORD started;
    int expires_in = 1800;
    int interval = 5;
    int ok;
    url_encode(client_id, client_encoded, sizeof(client_encoded));
    url_encode(scopes ? scopes : "", scopes_encoded, sizeof(scopes_encoded));
    _snprintf(body, sizeof(body) - 1, "client_id=%s&scopes=%s",
              client_encoded, scopes_encoded);
    body[sizeof(body) - 1] = 0;
    ok = http_request("id.twitch.tv", "POST", "/oauth2/device",
                      "Content-Type: application/x-www-form-urlencoded\r\n",
                      body, &response, &status, error, error_size);
    if (!ok || status != 200 ||
        !json_string(response, "device_code", device_code, sizeof(device_code)) ||
        !json_string(response, "verification_uri", verification_uri, sizeof(verification_uri))) {
        if (error && error_size && !error[0]) set_error(error, error_size, "Twitch device authorization could not start");
        free(response);
        return 0;
    }
    json_int(response, "expires_in", &expires_in);
    json_int(response, "interval", &interval);
    free(response);
    response = NULL;
    if ((INT_PTR)ShellExecuteA(NULL, "open", verification_uri, NULL, NULL, SW_SHOWNORMAL) <= 32) {
        set_error(error, error_size, "Twitch activation page could not be opened");
        return 0;
    }
    if (interval < 1) interval = 1;
    if (interval > 30) interval = 30;
    started = GetTickCount();
    while ((DWORD)(GetTickCount() - started) < (DWORD)expires_in * 1000u) {
        char device_encoded[TWITCH_TOKEN_MAX * 3];
        if (wait_cancelled((DWORD)interval * 1000u, cancel)) {
            set_error(error, error_size, "Twitch device authorization cancelled");
            return 0;
        }
        url_encode(device_code, device_encoded, sizeof(device_encoded));
        _snprintf(body, sizeof(body) - 1,
                  "client_id=%s&scopes=%s&device_code=%s&grant_type=urn%%3Aietf%%3Aparams%%3Aoauth%%3Agrant-type%%3Adevice_code",
                  client_encoded, scopes_encoded, device_encoded);
        body[sizeof(body) - 1] = 0;
        error[0] = 0;
        ok = http_request("id.twitch.tv", "POST", "/oauth2/token",
                          "Content-Type: application/x-www-form-urlencoded\r\n",
                          body, &response, &status, error, error_size);
        if (ok && status == 200 && parse_token_response(client_id, response, record)) {
            free(response);
            return 1;
        }
        message[0] = 0;
        if (response) json_string(response, "message", message, sizeof(message));
        free(response);
        response = NULL;
        if (_stricmp(message, "authorization_pending") == 0) continue;
        if (_stricmp(message, "slow_down") == 0) { interval += 5; continue; }
        if (message[0]) set_error(error, error_size, message);
        else if (!error[0]) set_error(error, error_size, "Twitch device authorization failed");
        return 0;
    }
    set_error(error, error_size, "Twitch device authorization expired");
    return 0;
}

static int acquire_token(const webm_twitch_settings_t *settings, const char *binary_dir,
                         const char *required_scope,
                         volatile LONG *cancel, twitch_auth_record_t *record,
                         char *user_id, size_t user_id_size,
                         char *user_login, size_t user_login_size,
                         char *error, size_t error_size)
{
    twitch_auth_record_t loaded;
    twitch_auth_record_t refreshed;
    int have_loaded = load_auth(binary_dir, settings->client_id, &loaded);
    if (user_id && user_id_size) user_id[0] = 0;
    if (user_login && user_login_size) user_login[0] = 0;
    if (have_loaded && validate_token(settings->client_id, loaded.access_token,
                                      required_scope, user_id, user_id_size,
                                      user_login, user_login_size)) {
        *record = loaded;
        return 1;
    }
    if (have_loaded && loaded.refresh_token[0] &&
        refresh_token(settings->client_id, loaded.refresh_token, &refreshed, error, error_size) &&
        validate_token(settings->client_id, refreshed.access_token, required_scope,
                       user_id, user_id_size, user_login, user_login_size)) {
        *record = refreshed;
        save_auth(binary_dir, record);
        return 1;
    }
    if (!settings->device_authorization) {
        set_error(error, error_size, required_scope && required_scope[0] ?
                  "Twitch chat requires device_authorization=true" :
                  "Twitch random selection requires device_authorization=true");
        return 0;
    }
    if (InterlockedCompareExchange(&twitch_auth_busy, 1, 0) != 0) {
        set_error(error, error_size, "Twitch device authorization is already in progress");
        return 0;
    }
    error[0] = 0;
    if (!device_authorize(settings->client_id, required_scope, cancel, record, error, error_size)) {
        InterlockedExchange(&twitch_auth_busy, 0);
        return 0;
    }
    save_auth(binary_dir, record);
    InterlockedExchange(&twitch_auth_busy, 0);
    return validate_token(settings->client_id, record->access_token, required_scope,
                          user_id, user_id_size, user_login, user_login_size);
}

static int twitch_api_get(const char *client_id, const char *token, const char *path,
                          char **response, char *error, size_t error_size)
{
    char headers[1400];
    DWORD status = 0;
    int ok;
    _snprintf(headers, sizeof(headers) - 1,
              "Authorization: Bearer %s\r\nClient-Id: %s\r\n", token, client_id);
    headers[sizeof(headers) - 1] = 0;
    ok = http_request("api.twitch.tv", "GET", path, headers, NULL,
                      response, &status, error, error_size);
    if (!ok) return 0;
    if (status != 200) {
        _snprintf(error, error_size - 1, "Twitch API returned HTTP %lu", (unsigned long)status);
        error[error_size - 1] = 0;
        free(*response);
        *response = NULL;
        return 0;
    }
    return 1;
}

static int find_data_objects(const char *json, char channels[][WEBM_TWITCH_CHANNEL_MAX],
                             int max_channels, const webm_twitch_settings_t *settings,
                             const char *excluded_channel)
{
    const char *array;
    const char *p;
    int count = 0;
    array = strstr(json, "\"data\"");
    if (!array || !(array = strchr(array, '['))) return 0;
    p = array + 1;
    while (*p && *p != ']' && count < max_channels) {
        const char *start;
        const char *end;
        int depth = 0;
        int string_mode = 0;
        int escaped = 0;
        char *object;
        char login[WEBM_TWITCH_CHANNEL_MAX];
        char language[32];
        int viewers = 0;
        int mature = 0;
        while (*p && *p != '{' && *p != ']') p++;
        if (*p != '{') break;
        start = p;
        for (; *p; p++) {
            char c = *p;
            if (string_mode) {
                if (escaped) escaped = 0;
                else if (c == '\\') escaped = 1;
                else if (c == '"') string_mode = 0;
            } else if (c == '"') {
                string_mode = 1;
            } else if (c == '{') {
                depth++;
            } else if (c == '}' && --depth == 0) {
                p++;
                break;
            }
        }
        end = p;
        object = (char*)malloc((size_t)(end - start) + 1);
        if (!object) break;
        memcpy(object, start, (size_t)(end - start));
        object[end - start] = 0;
        login[0] = 0;
        language[0] = 0;
        json_string(object, "user_login", login, sizeof(login));
        json_string(object, "language", language, sizeof(language));
        json_int(object, "viewer_count", &viewers);
        json_bool(object, "is_mature", &mature);
        free(object);
        if (!login[0]) continue;
        if (excluded_channel && excluded_channel[0] && _stricmp(login, excluded_channel) == 0) continue;
        if (viewers < settings->random_min_viewers) continue;
        if (!settings->random_allow_mature && mature) continue;
        if (settings->random_language[0] && language[0] &&
            _stricmp(settings->random_language, language) != 0) continue;
        copy_string(channels[count++], WEBM_TWITCH_CHANNEL_MAX, login);
    }
    return count;
}

int webm_twitch_select_random_channel(const webm_twitch_settings_t *settings,
                                      const char *binary_dir,
                                      const char *excluded_channel,
                                      volatile LONG *cancel,
                                      char *channel, size_t channel_size,
                                      char *error, size_t error_size)
{
    twitch_auth_record_t auth;
    char game_id[64];
    char game_encoded[512];
    char language_encoded[64];
    char path[2048];
    char *response = NULL;
    char channels[100][WEBM_TWITCH_CHANNEL_MAX];
    int count;
    unsigned int pick;
    if (channel && channel_size) channel[0] = 0;
    if (error && error_size) error[0] = 0;
    if (!settings || !settings->random_enabled) {
        set_error(error, error_size, "Twitch random selection is disabled");
        return 0;
    }
    if (!settings->client_id[0]) {
        set_error(error, error_size, "Twitch random selection requires a global client_id");
        return 0;
    }
    if (!acquire_token(settings, binary_dir, NULL, cancel, &auth,
                       NULL, 0, NULL, 0, error, error_size)) return 0;
    if (cancelled(cancel)) {
        set_error(error, error_size, "Twitch random selection cancelled");
        return 0;
    }

    game_id[0] = 0;
    if (settings->random_game[0]) {
        url_encode(settings->random_game, game_encoded, sizeof(game_encoded));
        _snprintf(path, sizeof(path) - 1, "/helix/games?name=%s", game_encoded);
        path[sizeof(path) - 1] = 0;
        if (!twitch_api_get(settings->client_id, auth.access_token, path,
                            &response, error, error_size) ||
            !json_string(response, "id", game_id, sizeof(game_id))) {
            free(response);
            if (!error[0]) set_error(error, error_size, "Twitch random_game was not found");
            return 0;
        }
        free(response);
        response = NULL;
    }
    url_encode(settings->random_language, language_encoded, sizeof(language_encoded));
    _snprintf(path, sizeof(path) - 1, "/helix/streams?first=100%s%s%s%s",
              language_encoded[0] ? "&language=" : "", language_encoded,
              game_id[0] ? "&game_id=" : "", game_id);
    path[sizeof(path) - 1] = 0;
    if (!twitch_api_get(settings->client_id, auth.access_token, path,
                        &response, error, error_size)) return 0;
    count = find_data_objects(response, channels, 100, settings, excluded_channel);
    free(response);
    if (count <= 0) {
        set_error(error, error_size, "No live Twitch streams matched the random filters");
        return 0;
    }
    pick = ((unsigned int)GetTickCount() ^ (unsigned int)GetCurrentThreadId() ^
            (unsigned int)(ULONG_PTR)settings) % (unsigned int)count;
    copy_string(channel, channel_size, channels[pick]);
    return channel && channel[0];
}

int webm_twitch_auth_get_chat_credentials(const webm_twitch_settings_t *settings,
                                          const char *binary_dir,
                                          volatile LONG *cancel,
                                          char *access_token, size_t access_token_size,
                                          char *user_id, size_t user_id_size,
                                          char *user_login, size_t user_login_size,
                                          char *error, size_t error_size)
{
    twitch_auth_record_t auth;
    if (access_token && access_token_size) access_token[0] = 0;
    if (user_id && user_id_size) user_id[0] = 0;
    if (user_login && user_login_size) user_login[0] = 0;
    if (error && error_size) error[0] = 0;
    if (!settings || !settings->chat_enabled) {
        set_error(error, error_size, "Twitch chat is disabled");
        return 0;
    }
    if (!settings->client_id[0]) {
        set_error(error, error_size, "Twitch chat requires a global client_id");
        return 0;
    }
    if (!acquire_token(settings, binary_dir, "user:read:chat", cancel, &auth,
                       user_id, user_id_size, user_login, user_login_size,
                       error, error_size)) {
        return 0;
    }
    copy_string(access_token, access_token_size, auth.access_token);
    return access_token && access_token[0] && user_id && user_id[0];
}

int webm_twitch_auth_get_user_id(const char *client_id, const char *access_token,
                                 const char *login,
                                 char *user_id, size_t user_id_size,
                                 char *error, size_t error_size)
{
    char login_encoded[WEBM_TWITCH_CHANNEL_MAX * 3];
    char path[512];
    char *response = NULL;
    int ok;
    if (user_id && user_id_size) user_id[0] = 0;
    if (error && error_size) error[0] = 0;
    if (!client_id || !client_id[0] || !access_token || !access_token[0] ||
        !login || !login[0]) {
        set_error(error, error_size, "Twitch chat channel lookup is missing credentials");
        return 0;
    }
    url_encode(login, login_encoded, sizeof(login_encoded));
    _snprintf(path, sizeof(path) - 1, "/helix/users?login=%s", login_encoded);
    path[sizeof(path) - 1] = 0;
    ok = twitch_api_get(client_id, access_token, path, &response, error, error_size) &&
         json_string(response, "id", user_id, user_id_size);
    free(response);
    if (!ok && error && error_size && !error[0]) {
        set_error(error, error_size, "Twitch chat channel was not found");
    }
    return ok && user_id && user_id[0];
}

int webm_twitch_auth_subscribe_chat(const char *client_id, const char *access_token,
                                    const char *broadcaster_user_id,
                                    const char *user_id,
                                    const char *session_id,
                                    char *error, size_t error_size)
{
    char headers[1600];
    char body[2048];
    char *response = NULL;
    char message[256];
    DWORD status = 0;
    int ok;
    if (error && error_size) error[0] = 0;
    _snprintf(headers, sizeof(headers) - 1,
              "Authorization: Bearer %s\r\nClient-Id: %s\r\nContent-Type: application/json\r\n",
              access_token ? access_token : "", client_id ? client_id : "");
    headers[sizeof(headers) - 1] = 0;
    _snprintf(body, sizeof(body) - 1,
              "{\"type\":\"channel.chat.message\",\"version\":\"1\","
              "\"condition\":{\"broadcaster_user_id\":\"%s\",\"user_id\":\"%s\"},"
              "\"transport\":{\"method\":\"websocket\",\"session_id\":\"%s\"}}",
              broadcaster_user_id ? broadcaster_user_id : "",
              user_id ? user_id : "", session_id ? session_id : "");
    body[sizeof(body) - 1] = 0;
    ok = http_request("api.twitch.tv", "POST", "/helix/eventsub/subscriptions",
                      headers, body, &response, &status, error, error_size);
    if (!ok || status != 202) {
        message[0] = 0;
        if (response) json_string(response, "message", message, sizeof(message));
        if (message[0]) set_error(error, error_size, message);
        else if (error && error_size && !error[0]) {
            _snprintf(error, error_size - 1,
                      "Twitch chat subscription returned HTTP %lu", (unsigned long)status);
            error[error_size - 1] = 0;
        }
        free(response);
        return 0;
    }
    free(response);
    return 1;
}
