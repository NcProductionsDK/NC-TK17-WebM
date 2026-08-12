#ifndef NC_TK17_WEBM_TWITCH_CHAT_H
#define NC_TK17_WEBM_TWITCH_CHAT_H

#include "webm_twitch.h"

typedef struct webm_twitch_chat_session webm_twitch_chat_session_t;

typedef enum {
    WEBM_TWITCH_CHAT_RGB = 0,
    WEBM_TWITCH_CHAT_BGR,
    WEBM_TWITCH_CHAT_RGB565
} webm_twitch_chat_pixel_format_t;

typedef void (*webm_twitch_chat_log_fn)(const char *format, ...);

void webm_twitch_chat_set_debug_logger(webm_twitch_chat_log_fn logger);

webm_twitch_chat_session_t *webm_twitch_chat_acquire(
    const webm_twitch_settings_t *settings, const char *channel);
webm_twitch_chat_session_t *webm_twitch_chat_retain(
    webm_twitch_chat_session_t *session);
void webm_twitch_chat_release(webm_twitch_chat_session_t *session);
void webm_twitch_chat_compose(webm_twitch_chat_session_t *session,
                              const webm_twitch_settings_t *settings,
                              unsigned char *pixels, int width, int height, int pitch,
                              webm_twitch_chat_pixel_format_t format,
                              int bytes_per_pixel, int flip_text_vertical);
void webm_twitch_chat_scale_frame(const unsigned char *source,
                                  int source_width, int source_height, int source_pitch,
                                  unsigned char *target,
                                  int target_width, int target_height, int target_pitch,
                                  int bytes_per_pixel);
void webm_twitch_chat_downsample_half(const unsigned char *source,
                                      int source_width, int source_height, int source_pitch,
                                      unsigned char *target,
                                      int target_width, int target_height, int target_pitch,
                                      int bytes_per_pixel);

#endif
