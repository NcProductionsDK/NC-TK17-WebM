#ifndef WEBM_CHANNEL_DIALOG_H
#define WEBM_CHANNEL_DIALOG_H

#include <windows.h>
#include <stddef.h>

int webm_channel_dialog_show(HINSTANCE instance, const char *initial_channel,
                             char *out_channel, size_t out_size);

#endif
