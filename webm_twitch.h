#ifndef NC_TK17_WEBM_TWITCH_H
#define NC_TK17_WEBM_TWITCH_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>

#define WEBM_TWITCH_CHANNEL_MAX 128
#define WEBM_TWITCH_CHANNEL_LIST_MAX 2048
#define WEBM_TWITCH_CHANNEL_LIST_COUNT 16
#define WEBM_TWITCH_QUALITY_MAX 32
#define WEBM_TWITCH_URL_MAX 4096
#define WEBM_TWITCH_CLIENT_ID_MAX 128
#define WEBM_TWITCH_USER_ID_MAX 64
#define WEBM_TWITCH_TOKEN_MAX 512

typedef enum {
    WEBM_TWITCH_DISABLED = 0,
    WEBM_TWITCH_IDLE,
    WEBM_TWITCH_RESOLVING,
    WEBM_TWITCH_READY,
    WEBM_TWITCH_FAILED
} webm_twitch_state_t;

typedef struct {
    int enabled;
    char channel[WEBM_TWITCH_CHANNEL_LIST_MAX];
    int random_enabled;
    int fallback_enabled;
    int device_authorization;
    char client_id[WEBM_TWITCH_CLIENT_ID_MAX];
    char quality[WEBM_TWITCH_QUALITY_MAX];
    DWORD buffer_ms;
    DWORD connect_timeout_ms;
    DWORD reconnect_interval_ms;
    int audio_volume;
    int audio_3d_min_distance;
    int audio_3d_min_distance_set;
    int audio_3d_max_distance;
    int audio_3d_max_distance_set;
    int audio_3d_rolloff;
    int audio_3d_rolloff_set;
    char random_language[16];
    char random_game[128];
    int random_allow_mature;
    int random_min_viewers;
    int chat_enabled;
    int chat_position;
    int chat_overlay;
    float chat_width;
    float chat_background_opacity;
    int chat_text_size;
    int chat_horizontal_padding;
    int chat_emotes;
    int chat_bttv;
    int chat_7tv;
    float chat_emote_scale;
    int chat_animated_emotes;
    int chat_animated_emote_fps;
} webm_twitch_settings_t;

typedef struct webm_twitch_session webm_twitch_session_t;

void webm_twitch_set_binary_dir(const char *binary_dir);
const char *webm_twitch_get_binary_dir(void);
int webm_twitch_sidecar_is_compatible(const char *sidecar_ini);
int webm_twitch_sidecar_has_section(const char *sidecar_ini);
int webm_twitch_load_settings(const char *global_ini, const char *sidecar_ini,
                              webm_twitch_settings_t *settings);
webm_twitch_session_t *webm_twitch_session_create(const webm_twitch_settings_t *settings);
void webm_twitch_session_destroy(webm_twitch_session_t *session);
webm_twitch_state_t webm_twitch_session_poll(webm_twitch_session_t *session, DWORD now,
                                              char *url, size_t url_size);
void webm_twitch_session_get_error(webm_twitch_session_t *session, char *error, size_t error_size);
void webm_twitch_session_get_channel(webm_twitch_session_t *session, char *channel, size_t channel_size);
int webm_twitch_session_has_pending_switch(webm_twitch_session_t *session);
void webm_twitch_session_accept_pending_switch(webm_twitch_session_t *session);
void webm_twitch_session_reject_pending_switch(webm_twitch_session_t *session, DWORD now,
                                                const char *error);
void webm_twitch_session_reject_url(webm_twitch_session_t *session, DWORD now, const char *error);

#endif
