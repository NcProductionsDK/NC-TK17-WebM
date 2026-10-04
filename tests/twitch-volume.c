/* Test production audio/config routines without starting TK17 or a stream. */
#include "../NC-TK17-WebM.c"

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static short captured[8];
static int captured_bytes, captured_format;

static void __cdecl capture_pcm(unsigned int buffer, int format, const void *data, int bytes, int rate)
{
    CHECK(bytes <= sizeof(captured));
    memcpy(captured, data, bytes);
    captured_bytes = bytes;
    captured_format = format;
}
static void __cdecl source_int(unsigned int source, int param, int *value) { *value = 0; }
static void __cdecl queue_buffers(unsigned int source, int count, const unsigned int *buffers) {}
static void __cdecl unqueue_buffers(unsigned int source, int count, unsigned int *buffers) {}

static void check_audio(int override_audio, int spatial, float gain, int pending)
{
    short input[] = { 10000, -10000, 20000, 20000, -20000, -20000, 32767, -32768 };
    audio_graph_t ag = {0};
    video_decoder_t *dec = (video_decoder_t*)calloc(1, sizeof(*dec));
    int i, count = spatial ? 4 : 8;
    CHECK(dec);
    InitializeCriticalSection(&dec->async_frame_lock);
    dec->async_lock_initialized = 1;
    dec->audio_queue_count = 1;
    dec->audio_queue[0].data = (BYTE*)input;
    dec->audio_queue[0].bytes = sizeof(input);
    dec->audio_queue[0].samples = 4;
    dec->audio_queue[0].sample_rate = 48000;
    ag.twitch_streaming = ag.openal = ag.al_source = 1;
    ag.twitch_override_audio = override_audio;
    ag.audio_3d = spatial;
    ag.twitch_buffer_count = ag.twitch_free_count = 1;
    ag.twitch_buffers[0] = ag.twitch_free_buffers[0] = 1;
    twitch_override_master_volume = pending ? 1.0f : gain;
    twitch_master_volume_pending = pending;
    twitch_master_volume_pending_value = gain;
    captured_bytes = 0;
    audio_graph_update_twitch_stream(&ag, dec, 0);
    CHECK(captured_bytes == count * sizeof(short));
    CHECK(captured_format == (spatial ? AL_FORMAT_MONO16_ : AL_FORMAT_STEREO16_));
    for (i = 0; i < count; ++i) {
        int original = spatial ? ((int)input[2*i] + input[2*i+1]) / 2 : input[i];
        int expected = (int)(original * (override_audio ? gain : 1.0f));
        if (expected > 32767) expected = 32767;
        if (expected < -32768) expected = -32768;
        CHECK(captured[i] == expected);
    }
    CHECK(ag.twitch_queue_count == 1 && dec->audio_queue_count == 0);
    free(ag.twitch_pcm);
    DeleteCriticalSection(&dec->async_frame_lock);
    free(dec);
}

static void check_settings(void)
{
    char temp_dir[MAX_PATH], temp_file[MAX_PATH], saved[32];
    float value;
    unsigned int generation;
    webm_setting_binding_t *binding = webm_setting_binding_by_name("NCWebMTwitchOverrideMasterVolume");
    CHECK(binding);
    CHECK(GetTempPathA(sizeof(temp_dir), temp_dir));
    CHECK(GetTempFileNameA(temp_dir, "wmv", 0, temp_file));
    config_loaded = 1;
    lstrcpynA(config_path_global, temp_file, sizeof(config_path_global));
    CHECK(WritePrivateProfileStringA("NC-TK17-WebM:TwitchOverride", "master_volume", "1.0", temp_file));
    CHECK(get_file_write_time_a(temp_file, &config_write_time));
    generation = config_generation;
    twitch_master_volume_pending = 0;
    twitch_override_master_volume = 1.0f;
    queue_twitch_master_volume(0.0f);
    flush_twitch_master_volume(twitch_master_volume_pending_tick + 199);
    CHECK(twitch_master_volume_pending && twitch_override_master_volume == 1.0f);
    flush_twitch_master_volume(twitch_master_volume_pending_tick + 200);
    CHECK(!twitch_master_volume_pending && twitch_override_master_volume == 0.0f);
    CHECK(config_generation == generation); /* Saving must not reconnect. */
    CHECK(webm_setting_ini_slider_value(binding, &value) && value == 0.0f);
    queue_twitch_master_volume(1.75f);
    flush_twitch_master_volume(twitch_master_volume_pending_tick + 200);
    CHECK(config_generation == generation);
    CHECK(webm_setting_ini_slider_value(binding, &value) && value == 1.75f);
    CHECK(GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", "master_volume", "", saved, sizeof(saved), temp_file));
    CHECK(strcmp(saved, "1.750") == 0);
    queue_twitch_master_volume(-1.0f);
    CHECK(twitch_master_volume_pending_value == 0.0f);
    queue_twitch_master_volume(3.0f);
    CHECK(twitch_master_volume_pending_value == 2.0f);
    twitch_master_volume_pending = 0;
    queue_twitch_master_volume((float)NAN);
    CHECK(!twitch_master_volume_pending);
    queue_twitch_master_volume((float)INFINITY);
    CHECK(!twitch_master_volume_pending);
    CHECK(DeleteFileA(temp_file));
}

/* Use the game's Lua 5.1 runtime to execute the real settings generator. */
static void check_lua(const char *dll_path, const char *script_path)
{
    HMODULE lua = LoadLibraryA(dll_path);
    void *state;
    void *(__cdecl *newstate)(void);
    void (__cdecl *openlibs)(void*), (__cdecl *close_state)(void*);
    void (__cdecl *pushstring)(void*, const char*);
    void (__cdecl *setfield)(void*, int, const char*);
    int (__cdecl *loadstring)(void*, const char*);
    int (__cdecl *pcall)(void*, int, int, int);
    const char *(__cdecl *tolstring)(void*, int, size_t*);
    CHECK(lua);
#define LUA_FN(var, name) do { *(FARPROC*)&var = GetProcAddress(lua, name); CHECK(var); } while (0)
    LUA_FN(newstate, "luaL_newstate"); LUA_FN(openlibs, "luaL_openlibs");
    LUA_FN(close_state, "lua_close"); LUA_FN(pushstring, "lua_pushstring");
    LUA_FN(setfield, "lua_setfield"); LUA_FN(loadstring, "luaL_loadstring");
    LUA_FN(pcall, "lua_pcall"); LUA_FN(tolstring, "lua_tolstring");
    state = newstate(); CHECK(state); openlibs(state);
    pushstring(state, script_path); setfield(state, -10002, "settings_script");
    CHECK(loadstring(state,
        "files_find=function() return {} end; ts=function() end; "
        "for _,case in ipairs({{'',1},{'0',0},{'1',1},{'1.75',1.75},{'2',2},{'-1',0},{'3',2},{'bad',1}}) do "
        "file_load2=function() return '[NC-TK17-WebM:TwitchOverride]\\nmaster_volume='..case[1] end; "
        "local generated; add=function(text) generated=text end; dofile(settings_script); "
        "local block=assert(generated:match('CustomParameter :Parameter_NCWebMTwitchOverrideMasterVolume (.-)\\n};')); "
        "assert(tonumber(block:match('SliderDefault F32%((.-)%)'))==case[2]); "
        "assert(block:find('SliderRange ( F32(0) , F32(2) )',1,true)); "
        "local enabled=assert(generated:find('Override Enabled:',1,true)); "
        "local volume=assert(generated:find('Twitch Master Volume:',1,true)); "
        "local target=assert(generated:find('Target object:',1,true)); "
        "assert(enabled<volume and volume<target); "
        "local ids={}; for id in generated:gmatch('%.ParamID I32%((%d+)%)') do assert(not ids[id]); ids[id]=true end; "
        "end") == 0);
    if (pcall(state, 0, 0, 0)) {
        fprintf(stderr, "Lua: %s\n", tolstring(state, -1, NULL)); exit(1);
    }
    close_state(state); FreeLibrary(lua);
}

int main(int argc, char **argv)
{
    int spatial, pending, i;
    float gains[] = {0.0f, 0.5f, 1.0f, 1.5f, 2.0f};
    vm_alBufferData = capture_pcm; vm_alGetSourcei = source_int;
    vm_alSourceQueueBuffers = queue_buffers; vm_alSourceUnqueueBuffers = unqueue_buffers;
    for (spatial = 0; spatial <= 1; ++spatial)
        for (pending = 0; pending <= 1; ++pending)
            for (i = 0; i < sizeof(gains)/sizeof(gains[0]); ++i) {
                check_audio(1, spatial, gains[i], pending);
                check_audio(0, spatial, gains[i], pending);
            }
    check_settings();
    CHECK(argc == 3); check_lua(argv[1], argv[2]);
    puts("PASS: stereo/spatial PCM, unity, mute, attenuation, boost, clipping, override isolation, live preview, persistence without restart, slider restoration, Lua generation");
    return 0;
}
