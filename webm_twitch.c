#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "webm_twitch.h"
#include "webm_twitch_auth.h"

#define TWITCH_SECTION "NC-TK17-WebM:Twitch"

struct webm_twitch_session {
    CRITICAL_SECTION lock;
    webm_twitch_settings_t settings;
    HANDLE thread;
    HANDLE process;
    volatile LONG cancel;
    webm_twitch_state_t state;
    DWORD next_retry_tick;
    char resolved_url[WEBM_TWITCH_URL_MAX];
    char resolved_channel[WEBM_TWITCH_CHANNEL_MAX];
    char pending_url[WEBM_TWITCH_URL_MAX];
    char pending_channel[WEBM_TWITCH_CHANNEL_MAX];
    int pending_switch;
    int fixed_probe;
    char error[256];
};

static char twitch_binary_dir[MAX_PATH * 2];

static void copy_string(char *dst, size_t dst_size, const char *src)
{
    if (!dst || dst_size == 0) return;
    if (!src) src = "";
    lstrcpynA(dst, src, (int)dst_size);
    dst[dst_size - 1] = 0;
}

static int parse_bool(const char *value, int fallback)
{
    if (!value || !value[0]) return fallback;
    if (_stricmp(value, "true") == 0 || _stricmp(value, "yes") == 0 ||
        _stricmp(value, "on") == 0 || strcmp(value, "1") == 0) return 1;
    if (_stricmp(value, "false") == 0 || _stricmp(value, "no") == 0 ||
        _stricmp(value, "off") == 0 || strcmp(value, "0") == 0) return 0;
    return fallback;
}

static int read_value(const char *path, const char *key, char *value, size_t value_size)
{
    char marker[2] = { 1, 0 };
    if (!value || value_size == 0) return 0;
    value[0] = 0;
    if (!path || !path[0]) return 0;
    GetPrivateProfileStringA(TWITCH_SECTION, key, marker, value, (DWORD)value_size, path);
    if ((unsigned char)value[0] == 1 && value[1] == 0) {
        value[0] = 0;
        return 0;
    }
    return 1;
}

static void read_override_string(const char *path, const char *key, char *value, size_t value_size)
{
    char buffer[WEBM_TWITCH_CHANNEL_LIST_MAX];
    if (read_value(path, key, buffer, sizeof(buffer))) copy_string(value, value_size, buffer);
}

static void read_override_int(const char *path, const char *key, int *value)
{
    char buffer[64];
    char *end;
    long parsed;
    if (!value || !read_value(path, key, buffer, sizeof(buffer))) return;
    parsed = strtol(buffer, &end, 10);
    if (end != buffer) *value = (int)parsed;
}

static void read_optional_int(const char *path, const char *key, int *value, int *is_set)
{
    char buffer[64];
    char *end;
    long parsed;
    if (!value || !is_set || !read_value(path, key, buffer, sizeof(buffer))) return;
    parsed = strtol(buffer, &end, 10);
    if (end != buffer) {
        *value = (int)parsed;
        *is_set = 1;
    }
}

static void read_override_bool(const char *path, const char *key, int *value)
{
    char buffer[64];
    if (!value || !read_value(path, key, buffer, sizeof(buffer))) return;
    *value = parse_bool(buffer, *value);
}

static void read_override_float(const char *path, const char *key, float *value)
{
    char buffer[64];
    char *end;
    double parsed;
    if (!value || !read_value(path, key, buffer, sizeof(buffer))) return;
    parsed = strtod(buffer, &end);
    if (end != buffer) *value = (float)parsed;
}

static void read_chat_position(const char *path, int *position)
{
    char buffer[32];
    if (!position || !read_value(path, "chat_position", buffer, sizeof(buffer))) return;
    if (_stricmp(buffer, "left") == 0) *position = 0;
    else if (_stricmp(buffer, "right") == 0) *position = 1;
}

static DWORD clamp_dword(int value, DWORD minimum, DWORD maximum)
{
    if (value < (int)minimum) return minimum;
    if (value > (int)maximum) return maximum;
    return (DWORD)value;
}

static int profile_has_section(const char *path, const char *section_name)
{
    char *sections;
    char *section;
    DWORD capacity = 1024;
    DWORD length;
    int found = 0;
    if (!path || !path[0] || !section_name || !section_name[0]) return 0;
    sections = (char*)malloc(capacity);
    if (!sections) return 0;
    for (;;) {
        length = GetPrivateProfileSectionNamesA(sections, capacity, path);
        if (length < capacity - 2 || capacity >= 65536) break;
        capacity *= 2;
        {
            char *larger = (char*)realloc(sections, capacity);
            if (!larger) {
                free(sections);
                return 0;
            }
            sections = larger;
        }
    }
    for (section = sections; section && *section; section += strlen(section) + 1) {
        if (_stricmp(section, section_name) == 0) {
            found = 1;
            break;
        }
    }
    free(sections);
    return found;
}

void webm_twitch_set_binary_dir(const char *binary_dir)
{
    copy_string(twitch_binary_dir, sizeof(twitch_binary_dir), binary_dir);
}

const char *webm_twitch_get_binary_dir(void)
{
    return twitch_binary_dir;
}

int webm_twitch_sidecar_is_compatible(const char *sidecar_ini)
{
    if (!sidecar_ini || !sidecar_ini[0]) return 0;
    return profile_has_section(sidecar_ini, "NC-TK17-WebM") ||
           profile_has_section(sidecar_ini, TWITCH_SECTION);
}

int webm_twitch_sidecar_has_section(const char *sidecar_ini)
{
    int enabled = 1;
    if (!sidecar_ini || !sidecar_ini[0]) return 0;
    /* GetPrivateProfileSectionA returns zero for a present but empty section.
       Enumerating section names lets an empty Twitch section serve as a target marker. */
    if (!profile_has_section(sidecar_ini, TWITCH_SECTION)) return 0;
    read_override_bool(sidecar_ini, "enabled", &enabled);
    return enabled;
}

int webm_twitch_load_settings(const char *global_ini, const char *sidecar_ini,
                              webm_twitch_settings_t *settings)
{
    int timeout = 10000;
    int reconnect = 30000;
    int buffer_ms = 180;
    if (!settings) return 0;
    memset(settings, 0, sizeof(*settings));
    copy_string(settings->quality, sizeof(settings->quality), "720p60");
    settings->fallback_enabled = 1;
    settings->buffer_ms = 180;
    settings->connect_timeout_ms = 10000;
    settings->reconnect_interval_ms = 30000;
    settings->audio_volume = 10000;
    settings->chat_position = 1;
    settings->chat_overlay = 1;
    settings->chat_width = 0.30f;
    settings->chat_background_opacity = 0.75f;
    settings->chat_text_size = 0;
    settings->chat_horizontal_padding = 10;
    settings->chat_emotes = 1;
    settings->chat_bttv = 1;
    settings->chat_7tv = 1;
    settings->chat_emote_scale = 1.0f;
    settings->chat_animated_emotes = 0;
    settings->chat_animated_emote_fps = 15;
    copy_string(settings->random_language, sizeof(settings->random_language), "en");
    settings->random_min_viewers = 10;

    read_override_bool(global_ini, "device_authorization", &settings->device_authorization);
    read_override_string(global_ini, "client_id", settings->client_id,
                         sizeof(settings->client_id));
    read_override_string(global_ini, "quality", settings->quality, sizeof(settings->quality));
    read_override_int(global_ini, "buffer_ms", &buffer_ms);
    read_override_int(global_ini, "connect_timeout_ms", &timeout);
    read_override_int(global_ini, "reconnect_interval_ms", &reconnect);
    read_override_int(global_ini, "audio_volume", &settings->audio_volume);
    read_optional_int(global_ini, "audio_3d_min_distance",
                      &settings->audio_3d_min_distance,
                      &settings->audio_3d_min_distance_set);
    read_optional_int(global_ini, "audio_3d_max_distance",
                      &settings->audio_3d_max_distance,
                      &settings->audio_3d_max_distance_set);
    read_optional_int(global_ini, "audio_3d_rolloff",
                      &settings->audio_3d_rolloff,
                      &settings->audio_3d_rolloff_set);
    read_override_string(global_ini, "random_language", settings->random_language,
                         sizeof(settings->random_language));
    read_override_string(global_ini, "random_game", settings->random_game,
                         sizeof(settings->random_game));
    read_override_bool(global_ini, "random_allow_mature", &settings->random_allow_mature);
    read_override_int(global_ini, "random_min_viewers", &settings->random_min_viewers);
    read_override_bool(global_ini, "chat_enabled", &settings->chat_enabled);
    read_chat_position(global_ini, &settings->chat_position);
    read_override_bool(global_ini, "chat_overlay", &settings->chat_overlay);
    read_override_float(global_ini, "chat_width", &settings->chat_width);
    read_override_float(global_ini, "chat_background_opacity",
                        &settings->chat_background_opacity);
    read_override_int(global_ini, "chat_text_size", &settings->chat_text_size);
    read_override_int(global_ini, "chat_horizontal_padding",
                      &settings->chat_horizontal_padding);
    read_override_bool(global_ini, "chat_emotes", &settings->chat_emotes);
    read_override_bool(global_ini, "chat_bttv", &settings->chat_bttv);
    read_override_bool(global_ini, "chat_7tv", &settings->chat_7tv);
    read_override_float(global_ini, "chat_emote_scale", &settings->chat_emote_scale);
    read_override_bool(global_ini, "chat_animated_emotes",
                       &settings->chat_animated_emotes);
    read_override_int(global_ini, "chat_animated_emote_fps",
                      &settings->chat_animated_emote_fps);

    settings->enabled = webm_twitch_sidecar_has_section(sidecar_ini);
    if (!settings->enabled) goto finalize_settings;
    read_override_string(sidecar_ini, "channel", settings->channel, sizeof(settings->channel));
    read_override_bool(sidecar_ini, "random", &settings->random_enabled);
    read_override_bool(sidecar_ini, "fallback", &settings->fallback_enabled);
    read_override_string(sidecar_ini, "quality", settings->quality, sizeof(settings->quality));
    read_override_int(sidecar_ini, "buffer_ms", &buffer_ms);
    read_override_int(sidecar_ini, "connect_timeout_ms", &timeout);
    read_override_int(sidecar_ini, "reconnect_interval_ms", &reconnect);
    read_override_int(sidecar_ini, "audio_volume", &settings->audio_volume);
    read_optional_int(sidecar_ini, "audio_3d_min_distance",
                      &settings->audio_3d_min_distance,
                      &settings->audio_3d_min_distance_set);
    read_optional_int(sidecar_ini, "audio_3d_max_distance",
                      &settings->audio_3d_max_distance,
                      &settings->audio_3d_max_distance_set);
    read_optional_int(sidecar_ini, "audio_3d_rolloff",
                      &settings->audio_3d_rolloff,
                      &settings->audio_3d_rolloff_set);
    read_override_string(sidecar_ini, "random_language", settings->random_language,
                         sizeof(settings->random_language));
    read_override_string(sidecar_ini, "random_game", settings->random_game,
                         sizeof(settings->random_game));
    read_override_bool(sidecar_ini, "random_allow_mature", &settings->random_allow_mature);
    read_override_int(sidecar_ini, "random_min_viewers", &settings->random_min_viewers);
    read_override_bool(sidecar_ini, "chat_enabled", &settings->chat_enabled);
    read_chat_position(sidecar_ini, &settings->chat_position);
    read_override_bool(sidecar_ini, "chat_overlay", &settings->chat_overlay);
    read_override_float(sidecar_ini, "chat_width", &settings->chat_width);
    read_override_float(sidecar_ini, "chat_background_opacity",
                        &settings->chat_background_opacity);
    read_override_int(sidecar_ini, "chat_text_size", &settings->chat_text_size);
    read_override_int(sidecar_ini, "chat_horizontal_padding",
                      &settings->chat_horizontal_padding);
    read_override_bool(sidecar_ini, "chat_emotes", &settings->chat_emotes);
    read_override_bool(sidecar_ini, "chat_bttv", &settings->chat_bttv);
    read_override_bool(sidecar_ini, "chat_7tv", &settings->chat_7tv);
    read_override_float(sidecar_ini, "chat_emote_scale", &settings->chat_emote_scale);
    read_override_bool(sidecar_ini, "chat_animated_emotes",
                       &settings->chat_animated_emotes);
    read_override_int(sidecar_ini, "chat_animated_emote_fps",
                      &settings->chat_animated_emote_fps);

finalize_settings:
    /* The decoded RGB queue retains 32 frames. Keep enough headroom for
       presentation at 60 fps instead of accepting a delay the queue cannot hold. */
    settings->buffer_ms = clamp_dword(buffer_ms, 0, 400);
    settings->connect_timeout_ms = clamp_dword(timeout, 1000, 120000);
    settings->reconnect_interval_ms = clamp_dword(reconnect, 1000, 3600000);
    if (settings->audio_volume < -10000) settings->audio_volume = -10000;
    if (settings->audio_volume > 20000) settings->audio_volume = 20000;
    if (settings->audio_3d_min_distance_set && settings->audio_3d_min_distance < 1) {
        settings->audio_3d_min_distance = 1;
    }
    if (settings->audio_3d_max_distance_set && settings->audio_3d_max_distance < 1) {
        settings->audio_3d_max_distance = 1;
    }
    if (settings->audio_3d_rolloff_set) {
        if (settings->audio_3d_rolloff < 0) settings->audio_3d_rolloff = 0;
        if (settings->audio_3d_rolloff > 20) settings->audio_3d_rolloff = 20;
    }
    if (settings->random_min_viewers < 0) settings->random_min_viewers = 0;
    if (settings->chat_width < 0.15f) settings->chat_width = 0.15f;
    if (settings->chat_width > 0.60f) settings->chat_width = 0.60f;
    if (settings->chat_background_opacity < 0.0f) settings->chat_background_opacity = 0.0f;
    if (settings->chat_background_opacity > 1.0f) settings->chat_background_opacity = 1.0f;
    if (settings->chat_text_size < 0) settings->chat_text_size = 0;
    if (settings->chat_text_size > 0 && settings->chat_text_size < 8) {
        settings->chat_text_size = 8;
    }
    if (settings->chat_text_size > 64) settings->chat_text_size = 64;
    if (settings->chat_horizontal_padding < 0) settings->chat_horizontal_padding = 0;
    if (settings->chat_horizontal_padding > 128) settings->chat_horizontal_padding = 128;
    if (settings->chat_emote_scale < 0.5f) settings->chat_emote_scale = 0.5f;
    if (settings->chat_emote_scale > 2.0f) settings->chat_emote_scale = 2.0f;
    if (settings->chat_animated_emote_fps < 1) settings->chat_animated_emote_fps = 1;
    if (settings->chat_animated_emote_fps > 30) settings->chat_animated_emote_fps = 30;
    /* A section without a channel or random=true is a dormant target marker.
       It becomes active only when the global override selects it. */
    return settings->enabled && (settings->channel[0] || settings->random_enabled);
}

static const char *find_text_ci(const char *text, const char *needle)
{
    size_t needle_length;
    if (!text || !needle || !needle[0]) return NULL;
    needle_length = strlen(needle);
    while (*text) {
        if (_strnicmp(text, needle, needle_length) == 0) return text;
        text++;
    }
    return NULL;
}

static int normalize_channel(const char *input, char *channel, size_t channel_size)
{
    const char *start = input;
    const char *marker;
    size_t length = 0;
    if (!channel || channel_size == 0) return 0;
    channel[0] = 0;
    if (!input) return 0;
    while (*start && isspace((unsigned char)*start)) start++;
    marker = find_text_ci(start, "twitch.tv/");
    if (marker) start = marker + 10;
    while (*start == '/') start++;
    while (*start && *start != '/' && *start != '?' && *start != '#' &&
           !isspace((unsigned char)*start) && length + 1 < channel_size) {
        unsigned char c = (unsigned char)*start++;
        if (!(isalnum(c) || c == '_')) return 0;
        channel[length++] = (char)tolower(c);
    }
    channel[length] = 0;
    return length > 0;
}

static int parse_channel_list(const char *input,
                              char channels[][WEBM_TWITCH_CHANNEL_MAX],
                              int capacity)
{
    const char *p = input;
    int count = 0;
    if (!input || !channels || capacity <= 0) return 0;
    while (*p && count < capacity) {
        const char *start = p;
        const char *end;
        char entry[WEBM_TWITCH_CHANNEL_LIST_MAX];
        char normalized[WEBM_TWITCH_CHANNEL_MAX];
        size_t length;
        int duplicate = 0;
        int i;
        while (*p && *p != ',') p++;
        end = p;
        while (start < end && isspace((unsigned char)*start)) start++;
        while (end > start && isspace((unsigned char)end[-1])) end--;
        length = (size_t)(end - start);
        if (length && length < sizeof(entry)) {
            memcpy(entry, start, length);
            entry[length] = 0;
            if (normalize_channel(entry, normalized, sizeof(normalized))) {
                for (i = 0; i < count; i++) {
                    if (_stricmp(channels[i], normalized) == 0) {
                        duplicate = 1;
                        break;
                    }
                }
                if (!duplicate) {
                    copy_string(channels[count], WEBM_TWITCH_CHANNEL_MAX, normalized);
                    count++;
                }
            }
        }
        if (*p == ',') p++;
    }
    return count;
}

static unsigned int channel_random_next(unsigned int *state)
{
    unsigned int value = *state;
    if (!value) value = 0x9e3779b9u;
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    *state = value;
    return value;
}

static void shuffle_channels(char channels[][WEBM_TWITCH_CHANNEL_MAX], int count,
                             unsigned int seed)
{
    int i;
    for (i = count - 1; i > 0; i--) {
        int selected = (int)(channel_random_next(&seed) % (unsigned int)(i + 1));
        if (selected != i) {
            char temporary[WEBM_TWITCH_CHANNEL_MAX];
            copy_string(temporary, sizeof(temporary), channels[i]);
            copy_string(channels[i], WEBM_TWITCH_CHANNEL_MAX, channels[selected]);
            copy_string(channels[selected], WEBM_TWITCH_CHANNEL_MAX, temporary);
        }
    }
}

static int channel_list_contains(const char *list, const char *channel)
{
    char channels[WEBM_TWITCH_CHANNEL_LIST_COUNT][WEBM_TWITCH_CHANNEL_MAX];
    int count;
    int i;
    if (!channel || !channel[0]) return 0;
    count = parse_channel_list(list, channels, WEBM_TWITCH_CHANNEL_LIST_COUNT);
    for (i = 0; i < count; i++) {
        if (_stricmp(channels[i], channel) == 0) return 1;
    }
    return 0;
}

static int normalize_quality(const char *input, char *quality, size_t quality_size)
{
    size_t i;
    size_t length = 0;
    if (!quality || quality_size == 0) return 0;
    quality[0] = 0;
    if (!input || !input[0]) input = "best";
    for (i = 0; input[i] && length + 1 < quality_size; i++) {
        unsigned char c = (unsigned char)input[i];
        if (!(isalnum(c) || c == ',' || c == '_' || c == '-')) return 0;
        quality[length++] = (char)c;
    }
    quality[length] = 0;
    return length > 0;
}

static int find_streamlink(char *path, size_t path_size)
{
    static const char *relative_paths[] = {
        "NC-TK17-WebM-twitch\\bin\\streamlink.exe",
        "NC-TK17-WebM-twitch\\streamlink.exe"
    };
    int i;
    char pattern[MAX_PATH * 4];
    WIN32_FIND_DATAA find_data;
    HANDLE find_handle;
    if (!path || path_size == 0 || !twitch_binary_dir[0]) return 0;
    for (i = 0; i < (int)(sizeof(relative_paths) / sizeof(relative_paths[0])); i++) {
        DWORD attributes;
        _snprintf(path, path_size - 1, "%s\\%s", twitch_binary_dir, relative_paths[i]);
        path[path_size - 1] = 0;
        attributes = GetFileAttributesA(path);
        if (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY)) return 1;
    }
    _snprintf(pattern, sizeof(pattern) - 1, "%s\\NC-TK17-WebM-twitch\\streamlink-*", twitch_binary_dir);
    pattern[sizeof(pattern) - 1] = 0;
    find_handle = FindFirstFileA(pattern, &find_data);
    if (find_handle != INVALID_HANDLE_VALUE) {
        do {
            DWORD attributes;
            if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
                strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
                continue;
            }
            _snprintf(path, path_size - 1, "%s\\NC-TK17-WebM-twitch\\%s\\bin\\streamlink.exe",
                      twitch_binary_dir, find_data.cFileName);
            path[path_size - 1] = 0;
            attributes = GetFileAttributesA(path);
            if (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
                FindClose(find_handle);
                return 1;
            }
        } while (FindNextFileA(find_handle, &find_data));
        FindClose(find_handle);
    }
    path[0] = 0;
    return 0;
}

static void trim_output_url(char *text)
{
    char *line = text;
    char *best = NULL;
    char *end;
    if (!text) return;
    while (*line) {
        while (*line == '\r' || *line == '\n' || isspace((unsigned char)*line)) line++;
        if (_strnicmp(line, "http://", 7) == 0 || _strnicmp(line, "https://", 8) == 0) {
            best = line;
            break;
        }
        line = strchr(line, '\n');
        if (!line) break;
        line++;
    }
    if (!best) {
        text[0] = 0;
        return;
    }
    if (best != text) memmove(text, best, strlen(best) + 1);
    end = text + strcspn(text, "\r\n");
    while (end > text && isspace((unsigned char)end[-1])) end--;
    *end = 0;
}

static int resolve_channel_stream(webm_twitch_session_t *session,
                                  const webm_twitch_settings_t *settings,
                                  const char *channel,
                                  char *url, size_t url_size,
                                  char *error, size_t error_size)
{
    SECURITY_ATTRIBUTES security;
    STARTUPINFOA startup;
    PROCESS_INFORMATION process_info;
    HANDLE read_pipe = NULL;
    HANDLE write_pipe = NULL;
    HANDLE null_input = INVALID_HANDLE_VALUE;
    char executable[MAX_PATH * 4];
    char helper_dir[MAX_PATH * 4];
    char quality[WEBM_TWITCH_QUALITY_MAX + 8];
    char requested_quality[WEBM_TWITCH_QUALITY_MAX];
    char command[WEBM_TWITCH_URL_MAX + MAX_PATH * 4];
    char output[WEBM_TWITCH_URL_MAX];
    DWORD wait_result;
    DWORD exit_code = 1;
    DWORD bytes_read = 0;
    int ok = 0;
    if (!settings || !channel || !channel[0]) {
        copy_string(error, error_size, "Twitch channel is empty or invalid");
        return 0;
    }
    if (!normalize_quality(settings->quality, requested_quality, sizeof(requested_quality))) {
        copy_string(error, error_size, "Twitch quality contains unsupported characters");
        return 0;
    }
    if (_stricmp(requested_quality, "best") == 0 || strchr(requested_quality, ',')) {
        copy_string(quality, sizeof(quality), requested_quality);
    } else {
        _snprintf(quality, sizeof(quality) - 1, "%s,best", requested_quality);
        quality[sizeof(quality) - 1] = 0;
    }
    if (!find_streamlink(executable, sizeof(executable))) {
        copy_string(error, error_size, "Streamlink was not found in Extensions\\WebM\\NC-TK17-WebM-twitch");
        return 0;
    }
    copy_string(helper_dir, sizeof(helper_dir), executable);
    {
        char *slash = strrchr(helper_dir, '\\');
        if (slash) *slash = 0;
    }

    memset(&security, 0, sizeof(security));
    security.nLength = sizeof(security);
    security.bInheritHandle = TRUE;
    if (!CreatePipe(&read_pipe, &write_pipe, &security, 0)) {
        copy_string(error, error_size, "could not create Streamlink output pipe");
        return 0;
    }
    SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);
    null_input = CreateFileA("NUL", GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING, 0, NULL);

    memset(&startup, 0, sizeof(startup));
    memset(&process_info, 0, sizeof(process_info));
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = null_input != INVALID_HANDLE_VALUE ? null_input : GetStdHandle(STD_INPUT_HANDLE);
    startup.hStdOutput = write_pipe;
    startup.hStdError = null_input != INVALID_HANDLE_VALUE ? null_input : GetStdHandle(STD_ERROR_HANDLE);
    _snprintf(command, sizeof(command) - 1,
              "\"%s\" --no-config --stream-url \"https://www.twitch.tv/%s\" \"%s\"",
              executable, channel, quality);
    command[sizeof(command) - 1] = 0;

    if (!CreateProcessA(executable, command, NULL, NULL, TRUE, CREATE_NO_WINDOW,
                        NULL, helper_dir, &startup, &process_info)) {
        _snprintf(error, error_size - 1, "could not start Streamlink (Windows error %lu)",
                  (unsigned long)GetLastError());
        error[error_size - 1] = 0;
        goto done;
    }
    CloseHandle(process_info.hThread);
    process_info.hThread = NULL;
    EnterCriticalSection(&session->lock);
    session->process = process_info.hProcess;
    LeaveCriticalSection(&session->lock);
    CloseHandle(write_pipe);
    write_pipe = NULL;

    wait_result = WaitForSingleObject(process_info.hProcess, settings->connect_timeout_ms);
    if (wait_result == WAIT_TIMEOUT || InterlockedCompareExchange(&session->cancel, 0, 0)) {
        TerminateProcess(process_info.hProcess, 1);
        WaitForSingleObject(process_info.hProcess, 2000);
        copy_string(error, error_size, wait_result == WAIT_TIMEOUT ?
                    "Streamlink connection timed out" : "Streamlink resolution cancelled");
        goto process_done;
    }
    GetExitCodeProcess(process_info.hProcess, &exit_code);
    memset(output, 0, sizeof(output));
    if (ReadFile(read_pipe, output, sizeof(output) - 1, &bytes_read, NULL) && bytes_read > 0) {
        output[bytes_read] = 0;
        trim_output_url(output);
    }
    if (exit_code == 0 && output[0]) {
        copy_string(url, url_size, output);
        ok = 1;
    } else {
        _snprintf(error, error_size - 1, "Streamlink could not resolve channel '%s' (exit %lu)",
                  channel, (unsigned long)exit_code);
        error[error_size - 1] = 0;
    }

process_done:
    EnterCriticalSection(&session->lock);
    session->process = NULL;
    LeaveCriticalSection(&session->lock);
    CloseHandle(process_info.hProcess);
    process_info.hProcess = NULL;

done:
    if (process_info.hThread) CloseHandle(process_info.hThread);
    if (process_info.hProcess) CloseHandle(process_info.hProcess);
    if (write_pipe) CloseHandle(write_pipe);
    if (read_pipe) CloseHandle(read_pipe);
    if (null_input != INVALID_HANDLE_VALUE) CloseHandle(null_input);
    return ok;
}

static int resolve_configured_stream(webm_twitch_session_t *session,
                                     const webm_twitch_settings_t *settings,
                                     char *url, size_t url_size,
                                     char *resolved_channel, size_t resolved_channel_size,
                                     char *first_channel, size_t first_channel_size,
                                     int *channel_count,
                                     char *error, size_t error_size)
{
    char channels[WEBM_TWITCH_CHANNEL_LIST_COUNT][WEBM_TWITCH_CHANNEL_MAX];
    char candidate_error[256];
    int count;
    int i;
    if (first_channel && first_channel_size) first_channel[0] = 0;
    if (channel_count) *channel_count = 0;
    count = parse_channel_list(settings ? settings->channel : NULL, channels,
                               WEBM_TWITCH_CHANNEL_LIST_COUNT);
    if (channel_count) *channel_count = count;
    if (!count) {
        copy_string(error, error_size, "Twitch channel list is empty or invalid");
        return 0;
    }
    if (first_channel && first_channel_size) {
        copy_string(first_channel, first_channel_size, channels[0]);
    }
    shuffle_channels(channels, count,
                     GetTickCount() ^ GetCurrentThreadId() ^ (unsigned int)(size_t)session);
    candidate_error[0] = 0;
    for (i = 0; i < count; i++) {
        candidate_error[0] = 0;
        if (resolve_channel_stream(session, settings, channels[i],
                                   url, url_size, candidate_error,
                                   sizeof(candidate_error))) {
            copy_string(resolved_channel, resolved_channel_size, channels[i]);
            return 1;
        }
        if (InterlockedCompareExchange(&session->cancel, 0, 0)) break;
    }
    copy_string(error, error_size, candidate_error[0] ? candidate_error :
                "No configured Twitch channels are live");
    return 0;
}

static int resolve_stream(webm_twitch_session_t *session,
                          char *url, size_t url_size,
                          char *resolved_channel, size_t resolved_channel_size,
                          char *error, size_t error_size)
{
    webm_twitch_settings_t settings;
    char first_channel[WEBM_TWITCH_CHANNEL_MAX];
    char random_channel[WEBM_TWITCH_CHANNEL_MAX];
    char fixed_error[256];
    int configured_count = 0;

    if (url && url_size) url[0] = 0;
    if (resolved_channel && resolved_channel_size) resolved_channel[0] = 0;
    if (error && error_size) error[0] = 0;
    EnterCriticalSection(&session->lock);
    settings = session->settings;
    LeaveCriticalSection(&session->lock);

    fixed_error[0] = 0;
    first_channel[0] = 0;
    if (resolve_configured_stream(session, &settings,
                                  url, url_size,
                                  resolved_channel, resolved_channel_size,
                                  first_channel, sizeof(first_channel),
                                  &configured_count,
                                  fixed_error, sizeof(fixed_error))) {
        return 1;
    }
    if (!settings.random_enabled) {
        copy_string(error, error_size, fixed_error);
        return 0;
    }
    if (!webm_twitch_select_random_channel(&settings, twitch_binary_dir,
                                           configured_count > 0 ? first_channel : NULL,
                                           &session->cancel,
                                           random_channel, sizeof(random_channel),
                                           error, error_size)) {
        return 0;
    }
    if (!resolve_channel_stream(session, &settings, random_channel,
                                url, url_size, error, error_size)) {
        return 0;
    }
    copy_string(resolved_channel, resolved_channel_size, random_channel);
    return 1;
}

static DWORD WINAPI resolver_thread(void *parameter)
{
    webm_twitch_session_t *session = (webm_twitch_session_t*)parameter;
    webm_twitch_settings_t settings;
    char url[WEBM_TWITCH_URL_MAX];
    char channel[WEBM_TWITCH_CHANNEL_MAX];
    char error[256];
    int fixed_probe;
    int ok;
    url[0] = 0;
    channel[0] = 0;
    error[0] = 0;
    EnterCriticalSection(&session->lock);
    settings = session->settings;
    fixed_probe = session->fixed_probe;
    LeaveCriticalSection(&session->lock);
    if (fixed_probe) {
        ok = resolve_configured_stream(session, &settings,
                                       url, sizeof(url),
                                       channel, sizeof(channel),
                                       NULL, 0, NULL,
                                       error, sizeof(error));
        EnterCriticalSection(&session->lock);
        session->fixed_probe = 0;
        if (!InterlockedCompareExchange(&session->cancel, 0, 0)) {
            if (ok && session->state == WEBM_TWITCH_READY &&
                _stricmp(session->resolved_channel, channel) != 0) {
                copy_string(session->pending_url, sizeof(session->pending_url), url);
                copy_string(session->pending_channel, sizeof(session->pending_channel), channel);
                session->pending_switch = 1;
            }
            session->next_retry_tick = GetTickCount() +
                                       session->settings.reconnect_interval_ms;
        }
        LeaveCriticalSection(&session->lock);
        return 0;
    }
    ok = resolve_stream(session, url, sizeof(url), channel, sizeof(channel),
                        error, sizeof(error));
    EnterCriticalSection(&session->lock);
    if (!InterlockedCompareExchange(&session->cancel, 0, 0)) {
        if (ok) {
            copy_string(session->resolved_url, sizeof(session->resolved_url), url);
            copy_string(session->resolved_channel, sizeof(session->resolved_channel), channel);
            session->pending_url[0] = 0;
            session->pending_channel[0] = 0;
            session->pending_switch = 0;
            session->error[0] = 0;
            session->state = WEBM_TWITCH_READY;
            session->next_retry_tick = GetTickCount() +
                                       session->settings.reconnect_interval_ms;
        } else {
            session->resolved_url[0] = 0;
            session->resolved_channel[0] = 0;
            copy_string(session->error, sizeof(session->error), error);
            session->state = WEBM_TWITCH_FAILED;
            session->next_retry_tick = GetTickCount() + session->settings.reconnect_interval_ms;
        }
    }
    LeaveCriticalSection(&session->lock);
    return 0;
}

webm_twitch_session_t *webm_twitch_session_create(const webm_twitch_settings_t *settings)
{
    webm_twitch_session_t *session;
    if (!settings || !settings->enabled) return NULL;
    session = (webm_twitch_session_t*)calloc(1, sizeof(*session));
    if (!session) return NULL;
    InitializeCriticalSection(&session->lock);
    session->settings = *settings;
    session->state = WEBM_TWITCH_IDLE;
    return session;
}

void webm_twitch_session_destroy(webm_twitch_session_t *session)
{
    HANDLE thread;
    if (!session) return;
    InterlockedExchange(&session->cancel, 1);
    EnterCriticalSection(&session->lock);
    if (session->process) TerminateProcess(session->process, 1);
    thread = session->thread;
    LeaveCriticalSection(&session->lock);
    if (thread) {
        WaitForSingleObject(thread, 5000);
        CloseHandle(thread);
    }
    DeleteCriticalSection(&session->lock);
    free(session);
}

webm_twitch_state_t webm_twitch_session_poll(webm_twitch_session_t *session, DWORD now,
                                              char *url, size_t url_size)
{
    webm_twitch_state_t state;
    HANDLE finished_thread = NULL;
    if (url && url_size) url[0] = 0;
    if (!session) return WEBM_TWITCH_DISABLED;
    EnterCriticalSection(&session->lock);
    if (session->thread && WaitForSingleObject(session->thread, 0) == WAIT_OBJECT_0) {
        finished_thread = session->thread;
        session->thread = NULL;
    }
    if ((session->state == WEBM_TWITCH_IDLE ||
         (session->state == WEBM_TWITCH_FAILED && (LONG)(now - session->next_retry_tick) >= 0)) &&
        !session->thread) {
        session->state = WEBM_TWITCH_RESOLVING;
        session->fixed_probe = 0;
        session->thread = CreateThread(NULL, 0, resolver_thread, session, 0, NULL);
        if (!session->thread) {
            session->state = WEBM_TWITCH_FAILED;
            session->next_retry_tick = now + session->settings.reconnect_interval_ms;
            copy_string(session->error, sizeof(session->error), "could not create Twitch resolver thread");
        }
    }
    if (session->state == WEBM_TWITCH_READY && session->settings.random_enabled &&
        !session->pending_switch && !session->thread &&
        (LONG)(now - session->next_retry_tick) >= 0 &&
        !channel_list_contains(session->settings.channel,
                               session->resolved_channel)) {
        session->fixed_probe = 1;
        session->next_retry_tick = now + session->settings.reconnect_interval_ms;
        session->thread = CreateThread(NULL, 0, resolver_thread, session, 0, NULL);
        if (!session->thread) session->fixed_probe = 0;
    }
    state = session->state;
    if (state == WEBM_TWITCH_READY && url && url_size) {
        copy_string(url, url_size, session->pending_switch ?
                    session->pending_url : session->resolved_url);
    }
    LeaveCriticalSection(&session->lock);
    if (finished_thread) CloseHandle(finished_thread);
    return state;
}

void webm_twitch_session_get_error(webm_twitch_session_t *session, char *error, size_t error_size)
{
    if (!error || error_size == 0) return;
    error[0] = 0;
    if (!session) return;
    EnterCriticalSection(&session->lock);
    copy_string(error, error_size, session->error);
    LeaveCriticalSection(&session->lock);
}

void webm_twitch_session_get_channel(webm_twitch_session_t *session,
                                     char *channel, size_t channel_size)
{
    if (!channel || channel_size == 0) return;
    channel[0] = 0;
    if (!session) return;
    EnterCriticalSection(&session->lock);
    copy_string(channel, channel_size, session->resolved_channel);
    LeaveCriticalSection(&session->lock);
}

int webm_twitch_session_has_pending_switch(webm_twitch_session_t *session)
{
    int pending = 0;
    if (!session) return 0;
    EnterCriticalSection(&session->lock);
    pending = session->pending_switch;
    LeaveCriticalSection(&session->lock);
    return pending;
}

void webm_twitch_session_accept_pending_switch(webm_twitch_session_t *session)
{
    if (!session) return;
    EnterCriticalSection(&session->lock);
    if (session->pending_switch) {
        copy_string(session->resolved_url, sizeof(session->resolved_url),
                    session->pending_url);
        copy_string(session->resolved_channel, sizeof(session->resolved_channel),
                    session->pending_channel);
        session->pending_url[0] = 0;
        session->pending_channel[0] = 0;
        session->pending_switch = 0;
        session->error[0] = 0;
    }
    LeaveCriticalSection(&session->lock);
}

void webm_twitch_session_reject_pending_switch(webm_twitch_session_t *session, DWORD now,
                                                const char *error)
{
    if (!session) return;
    EnterCriticalSection(&session->lock);
    session->pending_url[0] = 0;
    session->pending_channel[0] = 0;
    session->pending_switch = 0;
    copy_string(session->error, sizeof(session->error),
                error ? error : "Twitch preferred-channel switch failed");
    session->next_retry_tick = now + session->settings.reconnect_interval_ms;
    LeaveCriticalSection(&session->lock);
}

void webm_twitch_session_reject_url(webm_twitch_session_t *session, DWORD now, const char *error)
{
    if (!session) return;
    EnterCriticalSection(&session->lock);
    session->resolved_url[0] = 0;
    session->resolved_channel[0] = 0;
    session->pending_url[0] = 0;
    session->pending_channel[0] = 0;
    session->pending_switch = 0;
    copy_string(session->error, sizeof(session->error), error ? error : "Twitch decoder rejected stream URL");
    session->state = WEBM_TWITCH_FAILED;
    session->next_retry_tick = now + session->settings.reconnect_interval_ms;
    LeaveCriticalSection(&session->lock);
}
