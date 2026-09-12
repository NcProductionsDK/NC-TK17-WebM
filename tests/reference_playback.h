/* Frozen pre-optimization routines for differential tests only. */

static int reference_video_decoder_live_prepare_frame(video_decoder_t *dec, DWORD target_ms)
{
    double frame_slack_ms;
    double presentation_ms;
    int selected = -1;
    int prepared = 0;
    int cache_built = 0;
    int present_width = 0;
    int present_height = 0;
    int present_stride = 0;
    long present_size = 0;
    long present_frame_index = -1;
    d3d8_cache_build_t cache_build;
    webm_live_frame_t *queued;
    if (!dec || !dec->network_source || !dec->live_buffer_ms ||
        target_ms < dec->live_buffer_ms) {
        return 0;
    }
    EnterCriticalSection(&dec->live_queue_lock);
    if (dec->live_queue_count <= 0) {
        LeaveCriticalSection(&dec->live_queue_lock);
        return 0;
    }
    presentation_ms = (double)(target_ms - dec->live_buffer_ms);
    frame_slack_ms = 500.0 / (dec->fps > 0.1 ? dec->fps : 30.0);
    while (dec->live_queue_count > 0) {
        queued = &dec->live_queue[dec->live_queue_head];
        if (queued->pts_ms > presentation_ms + frame_slack_ms) break;
        selected = dec->live_queue_head;
        dec->live_queue_head = (dec->live_queue_head + 1) % WEBM_LIVE_QUEUE_CAPACITY;
        dec->live_queue_count--;
    }
    if (selected < 0) {
        LeaveCriticalSection(&dec->live_queue_lock);
        return 0;
    }
    queued = &dec->live_queue[selected];
    if (queued->data && queued->size > 0) {
        if (dec->live_present_frame_size < queued->size) {
            BYTE *frame = (BYTE*)realloc(dec->live_present_frame,
                                         (size_t)queued->size);
            if (frame) {
                dec->live_present_frame = frame;
                dec->live_present_frame_size = queued->size;
            }
        }
        if (dec->live_present_frame &&
            dec->live_present_frame_size >= queued->size) {
            memcpy(dec->live_present_frame, queued->data, (size_t)queued->size);
            present_size = queued->size;
            present_width = queued->width;
            present_height = queued->height;
            present_stride = queued->stride;
            present_frame_index = queued->frame_index;
            prepared = 1;
        }
    }
    LeaveCriticalSection(&dec->live_queue_lock);

    if (prepared) {
        BYTE *old_async_frame;
        long old_async_frame_size;
        cache_built = video_decoder_build_d3d8_cache(
            dec, &cache_build, dec->live_present_frame, present_size,
            present_width, present_height, present_stride, present_frame_index);

        EnterCriticalSection(&dec->async_frame_lock);
        old_async_frame = dec->async_frame;
        old_async_frame_size = dec->async_frame_size;
        dec->async_frame = dec->live_present_frame;
        dec->async_frame_size = dec->live_present_frame_size;
        dec->live_present_frame = old_async_frame;
        dec->live_present_frame_size = old_async_frame_size;
        {
            int frame_changed =
                dec->async_decoded_frame_index != present_frame_index;
            dec->async_width = present_width;
            dec->async_height = present_height;
            dec->async_stride = present_stride;
            dec->async_decoded_frame_index = present_frame_index;
            if (cache_built && cache_build.ready &&
                cache_build.generation == dec->async_d3d8_cache_generation &&
                dec->async_d3d8_cache_enabled) {
                int level;
                dec->async_d3d8_cache_front = cache_build.buffer_index;
                dec->async_d3d8_cache_ready = 1;
                dec->async_d3d8_cache_ready_levels = cache_build.levels;
                dec->async_d3d8_cache_ready_generation = cache_build.generation;
                dec->async_d3d8_cache_frame_index = present_frame_index;
                for (level = 0; level < cache_build.levels; level++) {
                    dec->async_d3d8_cache_level_offset[level] =
                        cache_build.level_offset[level];
                    dec->async_d3d8_cache_level_pitch[level] =
                        cache_build.level_pitch[level];
                    dec->async_d3d8_cache_level_width[level] =
                        cache_build.level_width[level];
                    dec->async_d3d8_cache_level_height[level] =
                        cache_build.level_height[level];
                }
            } else if (frame_changed) {
                dec->async_d3d8_cache_ready = 0;
                dec->async_d3d8_cache_ready_levels = 0;
            }
            dec->live_presented_frame_index = present_frame_index;
        }
        LeaveCriticalSection(&dec->async_frame_lock);
    }
    if (dec->live_decode_wake_event) SetEvent(dec->live_decode_wake_event);
    return prepared;
}

static void reference_video_decoder_async_publish_frame(video_decoder_t *dec, long frame_index)
{
    int cache_built = 0;
    d3d8_cache_build_t cache_build;
    if (!dec || !dec->frame || dec->frame_size <= 0 || frame_index < 0) return;
    cache_built = video_decoder_build_d3d8_cache(dec, &cache_build,
                                                  dec->frame, dec->frame_size,
                                                  dec->width, dec->height,
                                                  dec->stride, frame_index);
    EnterCriticalSection(&dec->async_frame_lock);
    if (dec->frame_size > dec->async_frame_size) {
        BYTE *new_frame = (BYTE*)realloc(dec->async_frame, (size_t)dec->frame_size);
        if (new_frame) {
            dec->async_frame = new_frame;
            dec->async_frame_size = dec->frame_size;
        }
    }
    if (dec->async_frame && dec->async_frame_size >= dec->frame_size) {
        int frame_changed = dec->async_decoded_frame_index != frame_index;
        memcpy(dec->async_frame, dec->frame, (size_t)dec->frame_size);
        dec->async_width = dec->width;
        dec->async_height = dec->height;
        dec->async_stride = dec->stride;
        dec->async_decoded_frame_index = frame_index;
        if (cache_built && cache_build.ready &&
            cache_build.generation == dec->async_d3d8_cache_generation &&
            dec->async_d3d8_cache_enabled) {
            int level;
            dec->async_d3d8_cache_front = cache_build.buffer_index;
            dec->async_d3d8_cache_ready = 1;
            dec->async_d3d8_cache_ready_levels = cache_build.levels;
            dec->async_d3d8_cache_ready_generation = cache_build.generation;
            dec->async_d3d8_cache_frame_index = frame_index;
            for (level = 0; level < cache_build.levels; level++) {
                dec->async_d3d8_cache_level_offset[level] = cache_build.level_offset[level];
                dec->async_d3d8_cache_level_pitch[level] = cache_build.level_pitch[level];
                dec->async_d3d8_cache_level_width[level] = cache_build.level_width[level];
                dec->async_d3d8_cache_level_height[level] = cache_build.level_height[level];
            }
        } else if (frame_changed) {
            dec->async_d3d8_cache_ready = 0;
            dec->async_d3d8_cache_ready_levels = 0;
        }
    }
    if (dec->looped) {
        dec->async_looped = 1;
        dec->looped = 0;
    }
    LeaveCriticalSection(&dec->async_frame_lock);
}
