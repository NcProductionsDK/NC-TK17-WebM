#ifndef NC_TK17_WEBM_TWITCH_AUTH_H
#define NC_TK17_WEBM_TWITCH_AUTH_H

#include "webm_twitch.h"

int webm_twitch_select_random_channel(const webm_twitch_settings_t *settings,
                                      const char *binary_dir,
                                      const char *excluded_channel,
                                      volatile LONG *cancel,
                                      char *channel, size_t channel_size,
                                      char *error, size_t error_size);
int webm_twitch_auth_get_chat_credentials(const webm_twitch_settings_t *settings,
                                          const char *binary_dir,
                                          volatile LONG *cancel,
                                          char *access_token, size_t access_token_size,
                                          char *user_id, size_t user_id_size,
                                          char *user_login, size_t user_login_size,
                                          char *error, size_t error_size);
int webm_twitch_auth_get_user_id(const char *client_id, const char *access_token,
                                 const char *login,
                                 char *user_id, size_t user_id_size,
                                 char *error, size_t error_size);
int webm_twitch_auth_subscribe_chat(const char *client_id, const char *access_token,
                                    const char *broadcaster_user_id,
                                    const char *user_id,
                                    const char *session_id,
                                    char *error, size_t error_size);

#endif
