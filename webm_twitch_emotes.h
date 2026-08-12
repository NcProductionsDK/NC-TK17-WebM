#ifndef NC_TK17_WEBM_TWITCH_EMOTES_H
#define NC_TK17_WEBM_TWITCH_EMOTES_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>

#define WEBM_TWITCH_EMOTE_CODE_MAX 64
#define WEBM_TWITCH_EMOTE_ID_MAX 64

typedef struct {
    char code[WEBM_TWITCH_EMOTE_CODE_MAX];
    char id[WEBM_TWITCH_EMOTE_ID_MAX];
} webm_twitch_provider_emote_t;

typedef struct {
    webm_twitch_provider_emote_t *entries;
    size_t capacity;
    size_t count;
    int bttv_count;
    int seventv_count;
} webm_twitch_emote_catalog_t;

int webm_twitch_emote_catalog_load(webm_twitch_emote_catalog_t *catalog,
                                    const char *twitch_user_id,
                                    int load_bttv, int load_seventv,
                                    volatile LONG *stop);
void webm_twitch_emote_catalog_destroy(webm_twitch_emote_catalog_t *catalog);
const char *webm_twitch_emote_catalog_lookup(
    const webm_twitch_emote_catalog_t *catalog, const char *code, size_t code_length);
int webm_twitch_emote_download(const char *id, unsigned char **data_out,
                               size_t *size_out);

#endif
