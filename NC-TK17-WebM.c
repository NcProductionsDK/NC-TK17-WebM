#define WIN32_LEAN_AND_MEAN
#define CINTERFACE
#define COBJMACROS
#include <windows.h>
#include <objbase.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dshow.h>
#include <qedit.h>
#include <d3d8.h>
#include <d3d11.h>
#include <float.h>
#include <math.h>
#if defined(__GNUC__) && (defined(__i386__) || defined(_M_IX86))
#include <emmintrin.h>
#endif

#include "webm_twitch.h"
#include "webm_twitch_chat.h"
#include "webm_channel_dialog.h"

typedef HANDLE (WINAPI *CreateFileA_t)(LPCSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE);
typedef HANDLE (WINAPI *CreateFileW_t)(LPCWSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE);
typedef HANDLE (WINAPI *FindFirstFileA_t)(LPCSTR,LPWIN32_FIND_DATAA);
typedef BOOL (WINAPI *FindNextFileA_t)(HANDLE,LPWIN32_FIND_DATAA);
typedef HANDLE (WINAPI *FindFirstFileW_t)(LPCWSTR,LPWIN32_FIND_DATAW);
typedef BOOL (WINAPI *FindNextFileW_t)(HANDLE,LPWIN32_FIND_DATAW);
typedef BOOL (WINAPI *FindClose_t)(HANDLE);
typedef BOOL (WINAPI *ReadFile_t)(HANDLE,LPVOID,DWORD,LPDWORD,LPOVERLAPPED);
typedef BOOL (WINAPI *CloseHandle_t)(HANDLE);
typedef HRESULT (WINAPI *CoCreateInstance_t)(REFCLSID,LPUNKNOWN,DWORD,REFIID,LPVOID*);
typedef FILE *(__cdecl *fopen_t)(const char *, const char *);
typedef FILE *(__cdecl *_wfopen_t)(const wchar_t *, const wchar_t *);
typedef unsigned int GLenum;
typedef int GLint;
typedef int GLsizei;
typedef void (WINAPI *glBindTexture_t)(GLenum, unsigned int);
typedef void (WINAPI *glGenTextures_t)(GLsizei, unsigned int *);
typedef void (WINAPI *glDeleteTextures_t)(GLsizei, const unsigned int *);
typedef void (WINAPI *glTexImage2D_t)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *);
typedef void (WINAPI *glTexSubImage2D_t)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *);
typedef void (WINAPI *glTexParameteri_t)(GLenum, GLenum, GLint);
typedef void (WINAPI *glPixelStorei_t)(GLenum, GLint);
typedef GLenum (WINAPI *glGetError_t)(void);
typedef const unsigned char *(WINAPI *glGetString_t)(GLenum);
typedef void (WINAPI *glGetFloatv_t)(GLenum, float *);
typedef void (WINAPI *glGetIntegerv_t)(GLenum, GLint *);
typedef void (WINAPI *glMatrixMode_t)(GLenum);
typedef void (WINAPI *glPushMatrix_t)(void);
typedef void (WINAPI *glPopMatrix_t)(void);
typedef void (WINAPI *glLoadMatrixf_t)(const float *);
typedef void (WINAPI *glDrawArrays_t)(GLenum, GLint, GLsizei);
typedef void (WINAPI *glDrawElements_t)(GLenum, GLsizei, GLenum, const void *);
typedef void (WINAPI *glGenerateMipmapEXT_t)(GLenum);
typedef PROC (WINAPI *wglGetProcAddress_t)(LPCSTR);
typedef FARPROC (WINAPI *GetProcAddress_t)(HMODULE, LPCSTR);
typedef BOOL (WINAPI *SwapBuffers_t)(HDC);
typedef IDirect3D8 *(WINAPI *Direct3DCreate8_t)(UINT);
typedef HRESULT (WINAPI *d3d8_CreateDevice_t)(IDirect3D8 *, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *, IDirect3DDevice8 **);
typedef HRESULT (WINAPI *d3d8_CreateTexture_t)(IDirect3DDevice8 *, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DTexture8 **);
typedef HRESULT (WINAPI *d3d8_Present_t)(IDirect3DDevice8 *, const RECT *, const RECT *, HWND, const RGNDATA *);
typedef HRESULT (WINAPI *d3d8_EndScene_t)(IDirect3DDevice8 *);
typedef HRESULT (WINAPI *d3d8_SetTexture_t)(IDirect3DDevice8 *, DWORD, IDirect3DBaseTexture8 *);
typedef HRESULT (WINAPI *d3d8_DrawPrimitive_t)(IDirect3DDevice8 *, D3DPRIMITIVETYPE, UINT, UINT);
typedef HRESULT (WINAPI *d3d8_DrawIndexedPrimitive_t)(IDirect3DDevice8 *, D3DPRIMITIVETYPE, UINT, UINT, UINT, UINT);
typedef HRESULT (WINAPI *d3d8_DrawPrimitiveUP_t)(IDirect3DDevice8 *, D3DPRIMITIVETYPE, UINT, const void *, UINT);
typedef HRESULT (WINAPI *d3d8_DrawIndexedPrimitiveUP_t)(IDirect3DDevice8 *, D3DPRIMITIVETYPE, UINT, UINT, UINT,
                                                        const void *, D3DFORMAT, const void *, UINT);
typedef HRESULT (WINAPI *d3d11_CreateDeviceAndSwapChain_t)(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT,
                                                           const D3D_FEATURE_LEVEL *, UINT, UINT,
                                                           const DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **,
                                                           ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
typedef HRESULT (WINAPI *d3d11_CreateDevice_t)(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT,
                                               const D3D_FEATURE_LEVEL *, UINT, UINT, ID3D11Device **,
                                               D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
typedef HRESULT (STDMETHODCALLTYPE *d3d11_CreateTexture2D_t)(ID3D11Device *,
                                                             const D3D11_TEXTURE2D_DESC *,
                                                             const D3D11_SUBRESOURCE_DATA *,
                                                             ID3D11Texture2D **);

typedef void *(__cdecl *CreateVideoDecoderByExtension_t)(const char *);
typedef void *(__cdecl *CreateTexture2D_t)(void);
typedef void *(__cdecl *CreateExTexture2D_t)(int);
typedef void *(__cdecl *CreateTexture2DVideo_t)(void);
typedef void *(__cdecl *CreateExTexture2DVideo_t)(int);
typedef void *(__cdecl *CreateImage_t)(void);
typedef void *(__cdecl *CreateExImage_t)(int);
typedef void *(__cdecl *CreateWImage_t)(void);
typedef void *(__cdecl *CreateExWImage_t)(int);

#if defined(__GNUC__)
#define THISCALL __attribute__((thiscall))
#else
#define THISCALL __thiscall
#endif

typedef void (THISCALL *SetVideoPath_t)(void *, void *, void *, void *, int);
typedef void (THISCALL *SetVideoDimension_t)(void *, void *, int);
typedef int (__cdecl *ReplaceImage_t)(void *, void *, void *, void *);
typedef void (THISCALL *vd_m0_t)(void *);
typedef void (THISCALL *vd_m1_t)(void *, void *, void *, void *);
typedef int (THISCALL *vd_m2_t)(void *, void *);
typedef double (THISCALL *vd_md0_t)(void *);
typedef int (THISCALL *vd_mi0_t)(void *);
typedef int (THISCALL *vd_mi1_t)(void *, int);
typedef int (THISCALL *vd_md1_t)(void *, double);
typedef int (THISCALL *stream_read_t)(void *, void *, int);
typedef int (THISCALL *image_import_t)(void *, int, void *, int, int);
typedef int (__stdcall *execute_file_call_t)(int, int, int, int);
typedef HRESULT (WINAPI *graph_renderfile_t)(void *, LPCWSTR, LPCWSTR);
typedef HRESULT (WINAPI *graph_addfilter_t)(void *, void *, LPCWSTR);
typedef HRESULT (WINAPI *graph_addsourcefilter_t)(void *, LPCWSTR, LPCWSTR, void **);
typedef HRESULT (WINAPI *graph_render_t)(void *, void *);
typedef HRESULT (WINAPI *DllGetClassObject_t)(REFCLSID, REFIID, LPVOID *);
typedef HRESULT (WINAPI *com_qi_t)(void *, REFIID, void **);
typedef ULONG (WINAPI *com_ref_t)(void *);
typedef HRESULT (WINAPI *class_create_t)(void *, IUnknown *, REFIID, void **);
typedef HRESULT (WINAPI *filesource_load_t)(void *, LPCWSTR, const void *);
typedef void *(__cdecl *app_find_objc_t)(const char *);
typedef void (__cdecl *model_pivot_t)(void *, float *);
typedef void (THISCALL *sound_receiver_get_all_parameters_t)(void *, float *, float *, float *, float *, float *, float *, float *);
typedef void *(__cdecl *sound_device_create_t)(int, int, int, void *);
typedef void *(THISCALL *sound_device_create_source_t)(void *, const char *, unsigned int, int);
typedef void *(__cdecl *sound_source_create_t)(void *, const char *, unsigned int, int);
typedef int (THISCALL *sound_source_play_t)(void *, int, int);
typedef void (THISCALL *sound_source_stop_t)(void *);
typedef int (THISCALL *sound_source_is_playing_t)(void *);
typedef int (THISCALL *sound_source_set_play_position_t)(void *, unsigned int);
typedef void (THISCALL *sound_source_set_position_t)(void *, float *, int);
typedef void (THISCALL *sound_source_set_distances_t)(void *, float, float, int);
typedef void (THISCALL *sound_source_set_volume_t)(void *, float, int);
typedef float (THISCALL *sound_source_get_volume_t)(void *);
typedef void (THISCALL *sound_source_destroy_t)(void *);
typedef void *(THISCALL *apptracker_get_camera_transform_t)(void *);
typedef void (THISCALL *apptracker_set_camera_transform_t)(void *, void *);
typedef void (THISCALL *apptracker_set_world_matrix_inverse_t)(void *, const float *);
typedef void (THISCALL *configeditor_param_change_t)(void *, const char *, const char *, DWORD, DWORD);
typedef int (THISCALL *customizer_build_controls_t)(void *, void *, void *, void *);
typedef int (THISCALL *customizer_create_button_t)(void *, void *, void *, void *, float);
typedef int (THISCALL *customizer_create_slider_t)(void *, void *, void *, void *, float, int, int);
typedef int (THISCALL *widget_dispatch_event_t)(void *, DWORD, DWORD);
typedef int (THISCALL *script_get_i32_t)(void *, DWORD);
typedef void (THISCALL *script_get_string_t)(void *, DWORD, char **);
typedef void (THISCALL *engine_string_release_t)(char **);
typedef void (THISCALL *engine_string_construct_cstr_t)(char **, const char *);
typedef float (THISCALL *widget_get_float_t)(void *, DWORD);
typedef void (THISCALL *widget_set_float_t)(void *, DWORD, float);
typedef void (THISCALL *widget_set_string_t)(void *, DWORD, const char *);
typedef void *(__cdecl *storage_openstream_t)(char *, unsigned int, char *);
typedef void *(__cdecl *bionic_new_t)(unsigned int, int);
typedef void *(__cdecl *msvc_new_t)(unsigned int);
typedef void *(__stdcall *streampipefilecache_ctor_t)(char *, int);
typedef void (__cdecl *al_gen_sources_t)(int, unsigned int *);
typedef void (__cdecl *al_delete_sources_t)(int, const unsigned int *);
typedef void (__cdecl *al_source_play_t)(unsigned int);
typedef void (__cdecl *al_source_stop_t)(unsigned int);
typedef void (__cdecl *al_sourcei_t)(unsigned int, int, int);
typedef void (__cdecl *al_sourcef_t)(unsigned int, int, float);
typedef void (__cdecl *al_sourcefv_t)(unsigned int, int, const float *);
typedef void (__cdecl *al_source_queue_buffers_t)(unsigned int, int, const unsigned int *);
typedef void (__cdecl *al_source_unqueue_buffers_t)(unsigned int, int, unsigned int *);
typedef void (__cdecl *al_get_sourcei_t)(unsigned int, int, int *);
typedef void (__cdecl *al_get_sourcef_t)(unsigned int, int, float *);
typedef void (__cdecl *al_gen_buffers_t)(int, unsigned int *);
typedef void (__cdecl *al_delete_buffers_t)(int, const unsigned int *);
typedef void (__cdecl *al_buffer_data_t)(unsigned int, int, const void *, int, int);
typedef int (__cdecl *al_get_error_t)(void);
typedef void *(__cdecl *alc_get_current_context_t)(void);

#define AL_BUFFER_ 0x1009
#define AL_LOOPING_ 0x1007
#define AL_SOURCE_RELATIVE_ 0x0202
#define AL_GAIN_ 0x100A
#define AL_FORMAT_MONO16_ 0x1101
#define AL_FORMAT_STEREO16_ 0x1103
#define AL_SOURCE_STATE_ 0x1010
#define AL_PLAYING_ 0x1012
#define AL_BUFFERS_QUEUED_ 0x1015
#define AL_BUFFERS_PROCESSED_ 0x1016
#define AL_SEC_OFFSET_ 0x1024
#define AL_REFERENCE_DISTANCE_ 0x1020
#define AL_ROLLOFF_FACTOR_ 0x1021
#define AL_MAX_DISTANCE_ 0x1023

static CreateFileA_t real_CreateFileA;
static CreateFileW_t real_CreateFileW;
static FindFirstFileA_t real_FindFirstFileA;
static FindNextFileA_t real_FindNextFileA;
static FindFirstFileW_t real_FindFirstFileW;
static FindNextFileW_t real_FindNextFileW;
static FindClose_t real_FindClose;
static ReadFile_t real_ReadFile;
static CloseHandle_t real_CloseHandle;
static CoCreateInstance_t real_CoCreateInstance;
static fopen_t real_fopen;
static _wfopen_t real__wfopen;
static glBindTexture_t real_glBindTexture;
static glGenTextures_t real_glGenTextures;
static glDeleteTextures_t real_glDeleteTextures;
static glTexImage2D_t real_glTexImage2D;
static glTexSubImage2D_t real_glTexSubImage2D;
static glTexParameteri_t real_glTexParameteri;
static glPixelStorei_t real_glPixelStorei;
static glGetError_t real_glGetError;
static glGetString_t real_glGetString;
static glGetFloatv_t real_glGetFloatv;
static glGetIntegerv_t real_glGetIntegerv;
static glMatrixMode_t real_glMatrixMode;
static glPushMatrix_t real_glPushMatrix;
static glPopMatrix_t real_glPopMatrix;
static glLoadMatrixf_t real_glLoadMatrixf;
static glDrawArrays_t real_glDrawArrays;
static glDrawElements_t real_glDrawElements;
static glGenerateMipmapEXT_t real_glGenerateMipmapEXT;
static wglGetProcAddress_t real_wglGetProcAddress;
static GetProcAddress_t real_GetProcAddress;
static SwapBuffers_t real_SwapBuffers;
static Direct3DCreate8_t real_Direct3DCreate8;
static d3d8_CreateDevice_t real_d3d8_CreateDevice;
static d3d8_CreateTexture_t real_d3d8_CreateTexture;
static d3d8_Present_t real_d3d8_Present;
static d3d8_EndScene_t real_d3d8_EndScene;
static d3d8_SetTexture_t real_d3d8_SetTexture;
static d3d8_DrawPrimitive_t real_d3d8_DrawPrimitive;
static d3d8_DrawIndexedPrimitive_t real_d3d8_DrawIndexedPrimitive;
static d3d8_DrawPrimitiveUP_t real_d3d8_DrawPrimitiveUP;
static d3d8_DrawIndexedPrimitiveUP_t real_d3d8_DrawIndexedPrimitiveUP;
static d3d11_CreateDeviceAndSwapChain_t real_D3D11CreateDeviceAndSwapChain;
static d3d11_CreateDevice_t real_D3D11CreateDevice;
static d3d11_CreateTexture2D_t real_d3d11_CreateTexture2D;
static ID3D11Device *captured_d3d11_device;
static ID3D11DeviceContext *captured_d3d11_context;
static unsigned int captured_d3d11_generation;
typedef int (__cdecl *hook5_acquire_d3d11_runtime_t)(
    unsigned int, void **, void **, unsigned int *);
static hook5_acquire_d3d11_runtime_t hook5_acquire_d3d11_runtime;
typedef int (__cdecl *hook5_begin_texture_resource_alias_t)(
    unsigned int, void *, void *);
typedef int (__cdecl *hook5_restore_texture_resource_aliases_t)(unsigned int);
static hook5_begin_texture_resource_alias_t
    hook5_begin_texture_resource_alias;
static hook5_restore_texture_resource_aliases_t
    hook5_restore_texture_resource_aliases;
static unsigned int hook5_d3d11_generation;
static DWORD hook5_resource_alias_bridge_last_lookup_tick;
static int hook5_resource_aliases_active;

typedef struct {
    int active;
    DWORD thread_id;
    UINT width;
    UINT height;
    ID3D11Texture2D *candidate;
    int candidate_count;
} d3d11_texture_capture_t;

static d3d11_texture_capture_t d3d11_texture_capture;

static void d3d11_texture_capture_begin(UINT width, UINT height);
static ID3D11Texture2D *d3d11_texture_capture_finish(void);
static void capture_d3d11_runtime(
    ID3D11Device *device, ID3D11DeviceContext *context);

static CreateVideoDecoderByExtension_t real_CreateVideoDecoderByExtension;
static CreateTexture2D_t real_CreateTexture2D;
static CreateExTexture2D_t real_CreateExTexture2D;
static CreateTexture2DVideo_t real_CreateTexture2DVideo;
static CreateExTexture2DVideo_t real_CreateExTexture2DVideo;
static CreateImage_t real_CreateImage;
static CreateExImage_t real_CreateExImage;
static CreateWImage_t real_CreateWImage;
static CreateExWImage_t real_CreateExWImage;
static SetVideoPath_t real_SetVideoPath;
static SetVideoDimension_t real_SetVideoDimension;
static ReplaceImage_t real_ReplaceImage;
static ReplaceImage_t tramp_ReplaceImage;
static stream_read_t real_StreamPipeFileRead;
static stream_read_t tramp_StreamPipeFileRead;
static stream_read_t real_StreamPipeFileCacheRead;
static stream_read_t tramp_StreamPipeFileCacheRead;
static image_import_t real_PngImport;
static image_import_t tramp_PngImport;
static execute_file_call_t real_ExecuteFileCall;

static CRITICAL_SECTION log_lock;
static int log_ready;
static DWORD video_decoder_orig_vt[15];
static DWORD video_decoder_hook_vt[15];
static int video_decoder_vt_ready;
static DWORD graph_builder_orig_vt[18];
static DWORD graph_builder_hook_vt[18];
static int graph_builder_vt_ready;
static HINSTANCE self_module;
static char target_texture_probe_path[MAX_PATH * 4];
static char target_texture_sidecar_path[MAX_PATH * 4];
static DWORD target_texture_probe_tick;
static int target_texture_probe_width;
static int target_texture_probe_height;
static DWORD target_texture_interval_ms;
static int target_texture_max_width;
static int target_texture_max_height;
static int target_texture_d3d8_mip_levels;
static int target_texture_video_filtering;
static int target_texture_anisotropy;
static char active_video_scene_base[MAX_PATH];
static char active_video_scene_sidecar[MAX_PATH * 4];
static DWORD active_video_scene_tick;
static int scene_video_rewrite_enabled;
static HANDLE scene_handles[16];
static HANDLE audio_script_handles[16];
static int replace_image_inline_installed;
static int stream_read_inline_installed;
static int png_import_inline_installed;
static int execute_file_call_installed;
static unsigned int current_gl_texture_2d;
static int directshow_texture_decoder_enabled;
static int ffmpeg_texture_decoder_enabled = 1;
static int texture_video_fps = 20;
static DWORD texture_video_interval_ms = 50;
static int texture_max_width = 2048;
static int texture_max_height = 2048;
static int d3d8_mip_levels = 4;
typedef enum {
    VIDEO_FILTER_NEAREST = 0,
    VIDEO_FILTER_LINEAR,
    VIDEO_FILTER_TRILINEAR,
    VIDEO_FILTER_ANISOTROPIC
} video_filtering_t;
static int video_filtering = VIDEO_FILTER_LINEAR;
static int video_anisotropy = 1;
static int gl_filter_caps_checked;
static int gl_filter_has_anisotropy;
static float gl_filter_max_anisotropy = 1.0f;
static int texture_audio_enabled = 0;
static int texture_audio_engine_enabled = 0;
static int texture_audio_lead_ms = 0;
static int texture_audio_volume = 0;
static int texture_audio_3d_enabled = 0;
static int texture_audio_3d_min_distance = 150;
static int texture_audio_3d_max_distance = 1200;
static int texture_audio_3d_rolloff = 3;
static char texture_audio_effect[32] = "none";
static int debug_logging = 0;
static int performance_profile = 0;
static int async_decoding = 0;
static int directx_d3d11_upload = 1;
static char retired_sidecar_paths[8][MAX_PATH * 4];
static DWORD retired_sidecar_until[8];
static int config_loaded;
static char config_path_global[MAX_PATH * 2];
static FILETIME config_write_time;
static DWORD last_global_config_check_tick;
static unsigned int config_generation = 1;
static int twitch_override_enabled;
static char twitch_override_target[MAX_PATH * 4];
#define TWITCH_OVERRIDE_AUTO_ROOM "auto_room"
static char twitch_override_auto_room[MAX_PATH];
static char twitch_override_auto_target[MAX_PATH * 4];
static char twitch_override_channel[WEBM_TWITCH_CHANNEL_LIST_MAX];
static int twitch_override_channel_offline_random;
static char twitch_override_quality[WEBM_TWITCH_QUALITY_MAX];
static int twitch_override_chat_enabled;
static int twitch_override_chat_position = 1;
static int twitch_override_chat_overlay;
static int twitch_override_chat_animated_emotes;
static float twitch_override_chat_width = 0.30f;
static float twitch_override_chat_background_opacity = 0.75f;
static configeditor_param_change_t tramp_ConfigEditor_ParamChange;
static int configeditor_param_change_installed;
static customizer_build_controls_t tramp_Customizer_BuildControls;
static int customizer_build_controls_installed;
static customizer_create_button_t tramp_Customizer_CreateButton;
static int customizer_create_button_installed;
static customizer_create_slider_t tramp_Customizer_CreateSlider;
static int customizer_create_slider_installed;
static void *twitch_channel_text_widget;
static void *twitch_channel_button_widget;
static void *twitch_channel_button_vtable[32];
static widget_dispatch_event_t twitch_channel_button_original_dispatch;
static DWORD twitch_channel_apply_last_tick;
static void *twitch_chat_opacity_slider_widget;
static float twitch_chat_opacity_pending_value;
static DWORD twitch_chat_opacity_pending_tick;
static int twitch_chat_opacity_pending;
static void *twitch_chat_width_slider_widget;
static float twitch_chat_width_pending_value;
static DWORD twitch_chat_width_pending_tick;
static int twitch_chat_width_pending;
static int webm_setting_sync_depth;

typedef enum webm_setting_control_type_t {
    WEBM_SETTING_SPINBOX,
    WEBM_SETTING_SLIDER
} webm_setting_control_type_t;

typedef struct webm_setting_binding_t {
    const char *param_name;
    const char *key;
    webm_setting_control_type_t type;
    void *slider_widget;
} webm_setting_binding_t;

static webm_setting_binding_t webm_setting_bindings[] = {
    { "NCWebMOverrideEnabled", "enabled", WEBM_SETTING_SPINBOX, NULL },
    { "NCWebMOverrideTarget", "target", WEBM_SETTING_SPINBOX, NULL },
    { "NCWebMTwitchOverrideChannelOfflineFallback", "channel_offline_fallback", WEBM_SETTING_SPINBOX, NULL },
    { "NCWebMTwitchOverrideChatEnabled", "chat_enabled", WEBM_SETTING_SPINBOX, NULL },
    { "NCWebMTwitchOverrideChatPosition", "chat_position", WEBM_SETTING_SPINBOX, NULL },
    { "NCWebMTwitchOverrideChatOverlay", "chat_overlay", WEBM_SETTING_SPINBOX, NULL },
    { "NCWebMTwitchOverrideChatAnimatedEmotesEnabled", "chat_animated_emotes", WEBM_SETTING_SPINBOX, NULL },
    { "NCWebMTwitchOverrideChatOverlayOpacity", "chat_background_opacity", WEBM_SETTING_SLIDER, NULL },
    { "NCWebMTwitchOverrideChatContainerWidth", "chat_width", WEBM_SETTING_SLIDER, NULL },
    { "NCWebMTwitchOverrideQuality", "quality", WEBM_SETTING_SPINBOX, NULL }
};

static int twitch_channel_dialog_active;
static DWORD twitch_channel_dialog_last_tick;
static app_find_objc_t engine_FindObjC;
static model_pivot_t engine_GetModelViewRotationPivot;
static sound_receiver_get_all_parameters_t engine_SoundReceiver_GetAllParameters;
static void **engine_SoundReceiver_ptr;
static void **engine_SoundDevice_ptr;
static sound_device_create_t real_SoundDevice_Create;
static sound_device_create_t tramp_SoundDevice_Create;
static sound_device_create_source_t real_SoundDevice_CreateSource;
static sound_source_create_t engine_SoundSource_Create;
static sound_source_play_t engine_SoundSource_Play;
static sound_source_stop_t engine_SoundSource_Stop;
static sound_source_is_playing_t engine_SoundSource_IsPlaying;
static sound_source_set_play_position_t engine_SoundSource_SetPlayPosition;
static sound_source_set_position_t engine_SoundSource_SetPosition;
static sound_source_set_distances_t engine_SoundSource_SetDistances;
static sound_source_set_volume_t engine_SoundSource_SetVolume;
static sound_source_get_volume_t engine_SoundSource_GetVolume;
static sound_source_destroy_t real_SoundSource_Destroy;
static sound_source_destroy_t tramp_SoundSource_Destroy;
static apptracker_get_camera_transform_t real_AppTracker_GetCameraTransform;
static apptracker_get_camera_transform_t tramp_AppTracker_GetCameraTransform;
static apptracker_set_camera_transform_t real_AppTracker_SetCameraTransform;
static apptracker_set_camera_transform_t tramp_AppTracker_SetCameraTransform;
static apptracker_set_world_matrix_inverse_t real_AppTracker_SetWorldMatrixInverse;
static apptracker_set_world_matrix_inverse_t tramp_AppTracker_SetWorldMatrixInverse;
static void *engine_captured_camera_transform;
static float engine_captured_camera_inverse[16];
static int engine_captured_camera_inverse_valid;
static DWORD engine_captured_camera_inverse_tick;
static int engine_audio_symbols_attempted;
static int sound_device_create_inline_installed;
static int sound_device_create_source_vtable_installed;
static int sound_source_destroy_inline_installed;
static int apptracker_camera_transform_inline_installed;
static int apptracker_set_camera_transform_inline_installed;
static int apptracker_world_matrix_inverse_inline_installed;
static int engine_audio_native_playback_enabled;
static void *engine_captured_sound_device;
static int openal_symbols_attempted;
static al_gen_sources_t vm_alGenSources;
static al_delete_sources_t vm_alDeleteSources;
static al_source_play_t vm_alSourcePlay;
static al_source_stop_t vm_alSourceStop;
static al_sourcei_t vm_alSourcei;
static al_sourcef_t vm_alSourcef;
static al_sourcefv_t vm_alSourcefv;
static al_source_queue_buffers_t vm_alSourceQueueBuffers;
static al_source_unqueue_buffers_t vm_alSourceUnqueueBuffers;
static al_get_sourcei_t vm_alGetSourcei;
static al_get_sourcef_t vm_alGetSourcef;
static al_gen_buffers_t vm_alGenBuffers;
static al_delete_buffers_t vm_alDeleteBuffers;
static al_buffer_data_t vm_alBufferData;
static al_get_error_t vm_alGetError;
static alc_get_current_context_t vm_alcGetCurrentContext;
static storage_openstream_t real_StorageOpenStream;
static storage_openstream_t tramp_StorageOpenStream;
static bionic_new_t engine_BionicNew;
static msvc_new_t engine_MsvcNew;
static streampipefilecache_ctor_t engine_StreamPipeFileCache_Ctor;
static int storage_openstream_installed;

typedef struct {
    void *vtable;
    int field_4;
    int field_8;
    int field_C;
    HANDLE file_handle;
    int field_14;
    int field_18;
    void *lpCriticalSection;
    int field_20;
    int bytes_to_write;
    char *out_buf;
    int field_2C;
    int field_30;
    int field_34;
    char *in_buf;
    int field_3C;
} bionic_stream_pipe_file_cache_t;

typedef struct {
    int active;
    char sound_id[128];
    char cache_path[MAX_PATH * 4];
} engine_sound_cache_t;

static engine_sound_cache_t engine_sound_cache[32];

typedef struct {
    int active;
    void *source;
    void *parent_obj;
    int volume;
    int min_distance;
    int max_distance;
    DWORD last_position_tick;
    char parent_path[256];
    char sound_name[160];
    char webm_path[MAX_PATH * 4];
} engine_audio_player_t;

typedef struct AVFormatContext AVFormatContext;
typedef struct AVCodecContext AVCodecContext;
typedef struct AVCodec AVCodec;
typedef struct AVDictionary AVDictionary;
typedef struct SwsContext SwsContext;

typedef struct {
    int codec_type;
    int codec_id;
} vm_AVCodecParameters;

typedef struct {
    int num;
    int den;
} vm_AVRational;

typedef struct {
    const void *av_class;
    int index;
    int id;
    vm_AVCodecParameters *codecpar;
    void *priv_data;
    vm_AVRational time_base;
    long long start_time;
    long long duration;
    long long nb_frames;
} vm_AVStream;

typedef struct {
    const void *av_class;
    void *iformat;
    void *oformat;
    void *priv_data;
    void *pb;
    int ctx_flags;
    unsigned int nb_streams;
    vm_AVStream **streams;
} vm_AVFormatContext;

typedef struct {
    void *buf;
    long long pts;
    long long dts;
    unsigned char *data;
    int size;
    int stream_index;
} vm_AVPacket;

typedef struct {
    unsigned char *data[8];
    int linesize[8];
    unsigned char **extended_data;
    int width;
    int height;
    int nb_samples;
    int format;
    int key_frame;
    int pict_type;
    vm_AVRational sample_aspect_ratio;
    long long pts;
} vm_AVFrame;

typedef int (*avformat_open_input_t)(AVFormatContext **, const char *, void *, AVDictionary **);
typedef int (*avformat_find_stream_info_t)(AVFormatContext *, AVDictionary **);
typedef int (*av_find_best_stream_t)(AVFormatContext *, int, int, int, const AVCodec **, int);
typedef int (*av_read_frame_t)(AVFormatContext *, vm_AVPacket *);
typedef int (*av_seek_frame_t)(AVFormatContext *, int, long long, int);
typedef vm_AVRational (*av_guess_frame_rate_t)(AVFormatContext *, vm_AVStream *, vm_AVFrame *);
typedef int (*avformat_alloc_output_context2_t)(AVFormatContext **, void *, const char *, const char *);
typedef vm_AVStream *(*avformat_new_stream_t)(AVFormatContext *, const AVCodec *);
typedef int (*avformat_write_header_t)(AVFormatContext *, AVDictionary **);
typedef int (*av_interleaved_write_frame_t)(AVFormatContext *, vm_AVPacket *);
typedef int (*av_write_trailer_t)(AVFormatContext *);
typedef void (*avformat_free_context_t)(AVFormatContext *);
typedef int (*avio_open_t)(void **, const char *, int);
typedef int (*avio_closep_t)(void **);
typedef void (*avformat_close_input_t)(AVFormatContext **);
typedef int (*avcodec_parameters_copy_t)(vm_AVCodecParameters *, const vm_AVCodecParameters *);
typedef AVCodec *(*avcodec_find_decoder_t)(int);
typedef AVCodecContext *(*avcodec_alloc_context3_t)(const AVCodec *);
typedef int (*avcodec_parameters_to_context_t)(AVCodecContext *, const vm_AVCodecParameters *);
typedef int (*avcodec_open2_t)(AVCodecContext *, const AVCodec *, AVDictionary **);
typedef int (*avcodec_send_packet_t)(AVCodecContext *, const vm_AVPacket *);
typedef int (*avcodec_receive_frame_t)(AVCodecContext *, vm_AVFrame *);
typedef void (*avcodec_flush_buffers_t)(AVCodecContext *);
typedef void (*avcodec_free_context_t)(AVCodecContext **);
typedef vm_AVPacket *(*av_packet_alloc_t)(void);
typedef void (*av_packet_free_t)(vm_AVPacket **);
typedef void (*av_packet_unref_t)(vm_AVPacket *);
typedef vm_AVFrame *(*av_frame_alloc_t)(void);
typedef void (*av_frame_free_t)(vm_AVFrame **);
typedef void (*av_frame_unref_t)(vm_AVFrame *);
typedef void *(*av_malloc_t)(size_t);
typedef void (*av_free_t)(void *);
typedef int (*av_opt_get_int_t)(void *, const char *, int, long long *);
typedef void (*av_log_set_level_t)(int);
typedef void (__cdecl *av_log_callback_t)(void *, int, const char *, va_list);
typedef void (*av_log_set_callback_t)(av_log_callback_t);
typedef SwsContext *(*sws_getContext_t)(int, int, int, int, int, int, int, void *, void *, const double *);
typedef int (*sws_scale_t)(SwsContext *, const unsigned char * const [], const int [], int, int, unsigned char * const [], const int []);
typedef void (*sws_freeContext_t)(SwsContext *);

typedef struct {
    int loaded;
    int failed;
    HMODULE avformat;
    HMODULE avcodec;
    HMODULE avutil;
    HMODULE swscale;
    avformat_open_input_t avformat_open_input;
    avformat_find_stream_info_t avformat_find_stream_info;
    av_find_best_stream_t av_find_best_stream;
    av_read_frame_t av_read_frame;
    av_seek_frame_t av_seek_frame;
    av_guess_frame_rate_t av_guess_frame_rate;
    avformat_alloc_output_context2_t avformat_alloc_output_context2;
    avformat_new_stream_t avformat_new_stream;
    avformat_write_header_t avformat_write_header;
    av_interleaved_write_frame_t av_interleaved_write_frame;
    av_write_trailer_t av_write_trailer;
    avformat_free_context_t avformat_free_context;
    avio_open_t avio_open;
    avio_closep_t avio_closep;
    avformat_close_input_t avformat_close_input;
    avcodec_parameters_copy_t avcodec_parameters_copy;
    avcodec_find_decoder_t avcodec_find_decoder;
    avcodec_alloc_context3_t avcodec_alloc_context3;
    avcodec_parameters_to_context_t avcodec_parameters_to_context;
    avcodec_open2_t avcodec_open2;
    avcodec_send_packet_t avcodec_send_packet;
    avcodec_receive_frame_t avcodec_receive_frame;
    avcodec_flush_buffers_t avcodec_flush_buffers;
    avcodec_free_context_t avcodec_free_context;
    av_packet_alloc_t av_packet_alloc;
    av_packet_free_t av_packet_free;
    av_packet_unref_t av_packet_unref;
    av_frame_alloc_t av_frame_alloc;
    av_frame_free_t av_frame_free;
    av_frame_unref_t av_frame_unref;
    av_malloc_t av_malloc;
    av_free_t av_free;
    av_opt_get_int_t av_opt_get_int;
    av_log_set_level_t av_log_set_level;
    av_log_callback_t av_log_default_callback;
    av_log_set_callback_t av_log_set_callback;
    sws_getContext_t sws_getContext;
    sws_scale_t sws_scale;
    sws_freeContext_t sws_freeContext;
} ffmpeg_api_t;

static ffmpeg_api_t ffmpeg_api;

static void __cdecl webm_ffmpeg_log_callback(void *avcl, int level,
                                              const char *format, va_list args)
{
    /* Some live MP4 streams repeat a valid MOOV atom as they refresh. FFmpeg
       safely skips it, but emits this warning for every refresh. Keep every
       other FFmpeg diagnostic on its normal path. */
    if (format && strstr(format, "Found duplicated MOOV Atom. Skipped it") != NULL) {
        return;
    }
    if (ffmpeg_api.av_log_default_callback) {
        ffmpeg_api.av_log_default_callback(avcl, level, format, args);
    }
}

#define VM_AV_NOPTS_VALUE (-9223372036854775807LL - 1)
#define VM_AV_LOG_WARNING 24
#define VM_AV_LOG_INFO 32
#define WEBM_D3D8_CACHE_LEVELS 8
#define WEBM_D3D8_CACHE_MAX_TOTAL_BYTES (32u * 1024u * 1024u)
#define WEBM_CACHE_FORMAT_GL_RGB (-1)
#define WEBM_CACHE_FORMAT_GL_RGBA (-2)
#define WEBM_LIVE_QUEUE_CAPACITY 32
#define WEBM_TWITCH_AUDIO_QUEUE_CAPACITY 64
#define WEBM_TWITCH_OPENAL_BUFFERS 12

typedef struct {
    BYTE *data;
    size_t capacity;
    long size;
    int width;
    int height;
    int stride;
    double pts_ms;
    long frame_index;
} webm_live_frame_t;

typedef struct {
    BYTE *data;
    size_t capacity;
    int bytes;
    int samples;
    int sample_rate;
    double pts_ms;
} webm_twitch_audio_chunk_t;

typedef struct {
    int active;
    IGraphBuilder *graph;
    IMediaControl *control;
    IBaseFilter *source_filter;
    IBaseFilter *video_filter;
    IBaseFilter *sample_filter;
    IBaseFilter *null_filter;
    ISampleGrabber *grabber;
    BYTE *frame;
    long frame_size;
    int width;
    int height;
    int stride;
    int got_format;
    int use_ffmpeg;
    AVFormatContext *fmt;
    AVCodecContext *codec;
    AVCodecContext *audio_codec;
    vm_AVPacket *packet;
    vm_AVFrame *src_frame;
    vm_AVFrame *audio_frame;
    SwsContext *sws;
    int stream_index;
    int audio_stream_index;
    int audio_time_base_num;
    int audio_time_base_den;
    int audio_sample_rate;
    int audio_channels;
    double audio_first_pts_ms;
    int audio_first_pts_set;
    double audio_next_pts_ms;
    BYTE *audio_convert_buffer;
    size_t audio_convert_capacity;
    webm_twitch_audio_chunk_t audio_queue[WEBM_TWITCH_AUDIO_QUEUE_CAPACITY];
    int audio_queue_head;
    int audio_queue_count;
    unsigned int audio_queue_dropped;
    int rgb_stride;
    double fps;
    int time_base_num;
    int time_base_den;
    double current_frame_ms;
    double first_frame_pts_ms;
    int first_frame_pts_set;
    int network_source;
    DWORD live_buffer_ms;
    double live_audio_clock_ms;
    double live_audio_clock_offset_ms;
    int live_audio_clock_valid;
    webm_live_frame_t live_queue[WEBM_LIVE_QUEUE_CAPACITY];
    int live_queue_head;
    int live_queue_count;
    long live_presented_frame_index;
    long decoded_frame_index;
    int looped;
    int async_enabled;
    int async_lock_initialized;
    int live_queue_lock_initialized;
    HANDLE async_thread;
    HANDLE async_wake_event;
    HANDLE live_decode_thread;
    HANDLE live_decode_wake_event;
    volatile LONG live_read_failures;
    volatile LONG live_failure_tick;
    volatile LONG live_last_frame_tick;
    CRITICAL_SECTION async_frame_lock;
    CRITICAL_SECTION live_queue_lock;
    volatile LONG async_stop;
    volatile LONG async_target_ms;
    BYTE *async_frame;
    long async_frame_size;
    BYTE *live_present_frame;
    long live_present_frame_size;
    int async_width;
    int async_height;
    int async_stride;
    long async_decoded_frame_index;
    int async_looped;
    int async_d3d8_cache_enabled;
    int async_d3d8_cache_width;
    int async_d3d8_cache_height;
    int async_d3d8_cache_format;
    int async_d3d8_cache_levels;
    unsigned int async_d3d8_cache_generation;
    unsigned int async_d3d8_cache_ready_generation;
    BYTE *async_d3d8_cache[2];
    size_t async_d3d8_cache_capacity[2];
    volatile LONG async_d3d8_cache_readers[2];
    int async_d3d8_cache_front;
    int async_d3d8_cache_ready;
    int async_d3d8_cache_ready_levels;
    long async_d3d8_cache_frame_index;
    webm_twitch_chat_session_t *async_d3d8_chat;
    webm_twitch_settings_t async_d3d8_chat_settings;
    size_t async_d3d8_cache_level_offset[WEBM_D3D8_CACHE_LEVELS];
    int async_d3d8_cache_level_pitch[WEBM_D3D8_CACHE_LEVELS];
    int async_d3d8_cache_level_width[WEBM_D3D8_CACHE_LEVELS];
    int async_d3d8_cache_level_height[WEBM_D3D8_CACHE_LEVELS];
} video_decoder_t;

typedef struct {
    int ready;
    unsigned int generation;
    int buffer_index;
    int levels;
    size_t level_offset[WEBM_D3D8_CACHE_LEVELS];
    int level_pitch[WEBM_D3D8_CACHE_LEVELS];
    int level_width[WEBM_D3D8_CACHE_LEVELS];
    int level_height[WEBM_D3D8_CACHE_LEVELS];
} d3d8_cache_build_t;

static int video_decoder_build_d3d8_cache(video_decoder_t *dec,
                                           d3d8_cache_build_t *build,
                                           const BYTE *source_frame,
                                           long source_frame_size,
                                           int source_width,
                                           int source_height,
                                           int source_stride,
                                           long frame_index);

typedef struct {
    HANDLE thread;
    volatile LONG done;
    video_decoder_t *decoder;
    char url[WEBM_TWITCH_URL_MAX];
} twitch_decoder_open_task_t;

typedef struct video_decoder_retire_task_s {
    HANDLE thread;
    volatile LONG done;
    video_decoder_t *decoder;
    struct video_decoder_retire_task_s *next;
} video_decoder_retire_task_t;

static CRITICAL_SECTION video_decoder_retire_lock;
static int video_decoder_retire_lock_ready;
static video_decoder_retire_task_t *video_decoder_retire_tasks;

static void twitch_decoder_open_task_release(twitch_decoder_open_task_t *task);
static void video_decoder_retire_async(video_decoder_t *decoder);
static void video_decoder_reap_retired(void);
static void video_decoder_drain_retired(void);

typedef struct {
    int active;
    int openal;
    IGraphBuilder *graph;
    IMediaControl *control;
    IBaseFilter *source_filter;
    IBaseFilter *audio_filter;
    IBaseFilter *renderer_filter;
    IMediaSeeking *seeking;
    void *basic_audio;
    int lead_ms;
    int volume;
    int audio_3d;
    int min_distance;
    int max_distance;
    int rolloff;
    int current_volume;
    DWORD last_3d_tick;
    DWORD last_find_tick;
    DWORD last_listener_find_tick;
    void *source_obj;
    void *listener_obj;
    int logged_missing_symbols;
    int logged_listener_missing;
    int logged_listener_found;
    int logged_source_missing;
    int logged_source_found;
    int source_zero_count;
    int source_position_disabled;
    unsigned int al_source;
    unsigned int al_buffer;
    int twitch_streaming;
    int twitch_started;
    unsigned int twitch_buffers[WEBM_TWITCH_OPENAL_BUFFERS];
    unsigned int twitch_free_buffers[WEBM_TWITCH_OPENAL_BUFFERS];
    int twitch_buffer_count;
    int twitch_free_count;
    unsigned int twitch_underruns;
    unsigned int twitch_sync_dropped_chunks;
    double twitch_start_pts_ms;
    int twitch_start_pts_set;
    unsigned int twitch_queue_order[WEBM_TWITCH_OPENAL_BUFFERS];
    int twitch_queue_head;
    int twitch_queue_count;
    double twitch_buffer_pts_ms[WEBM_TWITCH_OPENAL_BUFFERS];
    double twitch_buffer_duration_ms[WEBM_TWITCH_OPENAL_BUFFERS];
    BYTE *twitch_pcm;
    size_t twitch_pcm_capacity;
    char source_name[128];
    char audio_effect[32];
    char path[MAX_PATH * 4];
} audio_graph_t;

static audio_graph_t *audio_graph_create(const char *path, int lead_ms, int volume,
                                         int audio_3d, int min_distance, int max_distance,
                                         int rolloff, const char *source_name, const char *audio_effect);
static audio_graph_t *audio_graph_create_twitch_stream(const char *path, int volume,
                                                        int audio_3d, int min_distance,
                                                        int max_distance, int rolloff,
                                                        const char *source_name);
static void audio_graph_update_twitch_stream(audio_graph_t *ag, video_decoder_t *dec,
                                             DWORD target_ms);
static int video_decoder_twitch_audio_pop(video_decoder_t *dec, BYTE **buffer, size_t *capacity,
                                          int *bytes, int *samples, int *sample_rate,
                                          double *pts_ms);
static void audio_graph_release(audio_graph_t *ag);
static float engine_audio_volume_to_float(int volume);
static engine_audio_player_t *engine_audio_player_create(const char *webm, const char *parent_path,
                                                         int volume, int min_distance, int max_distance);
static void engine_audio_player_release(engine_audio_player_t *ep);
static void engine_audio_player_update(engine_audio_player_t *ep, DWORD now);
static int remux_webm_audio_to_ogg_a(const char *webm, char *sound_id, size_t sound_id_sz, char *cache_path, size_t cache_path_sz);
static void clear_all_video_texture_slots(void);
static void restore_hook5_proxy_resource_aliases(void);

typedef struct {
    int parked;
    int existed;
    FILETIME write_time;
    DWORD size_high;
    DWORD size_low;
    DWORD last_probe_tick;
    DWORD retry_tick;
} webm_source_failure_state_t;

enum {
    WEBM_SOURCE_REFRESH_NONE = 0,
    WEBM_SOURCE_REFRESH_CHANGED = 1,
    WEBM_SOURCE_REFRESH_RETRY = 2
};

enum {
    WEBM_UV_MODE_OFF = 0,
    WEBM_UV_MODE_FULL_TEXTURE = 1
};

#define WEBM_GAME_AUDIO_MUTE_MAX_NAMES 16
#define WEBM_GAME_AUDIO_MUTE_NAME_MAX 128
#define WEBM_GAME_AUDIO_MUTE_MAX_SOURCES 128
#define WEBM_GAME_AUDIO_SILENT_VOLUME (-100.0f)

typedef struct {
    int count;
    char names[WEBM_GAME_AUDIO_MUTE_MAX_NAMES][WEBM_GAME_AUDIO_MUTE_NAME_MAX];
} game_audio_mute_list_t;

typedef struct {
    void *source;
    char full_name[WEBM_GAME_AUDIO_MUTE_NAME_MAX];
    char base_name[WEBM_GAME_AUDIO_MUTE_NAME_MAX];
    float saved_volume;
    int muted;
} game_audio_mute_source_t;

static CRITICAL_SECTION game_audio_mute_lock;
static int game_audio_mute_lock_ready;
static game_audio_mute_source_t game_audio_mute_sources[WEBM_GAME_AUDIO_MUTE_MAX_SOURCES];
static int game_audio_mute_source_count;
static game_audio_mute_list_t game_audio_active_mutes;
static DWORD game_audio_mute_last_enforce_tick;

typedef struct {
    int active;
    unsigned int texture;
    unsigned int video_texture;
    int width;
    int height;
    GLenum format;
    GLenum type;
    BYTE *pixels;
    size_t pixels_size;
    DWORD last_update_tick;
    DWORD update_interval_ms;
    DWORD first_seen_tick;
    DWORD last_bound_tick;
    DWORD bind_log_count;
    int frame;
    long uploaded_frame_index;
    int atlas_region;
    int wide_region;
    int video_filtering;
    int anisotropy;
    int uv_mode_base;
    int uv_mode_twitch;
    int uv_rect_valid;
    float uv_min_u;
    float uv_min_v;
    float uv_max_u;
    float uv_max_v;
    int uv_override_logged;
    game_audio_mute_list_t game_audio_mutes;
    int decoder_attempted;
    webm_source_failure_state_t source_failure;
    int audio_enabled;
    int audio_engine;
    int engine_audio_runtime_failed;
    int audio_lead_ms;
    int audio_volume;
    int audio_3d;
    int audio_3d_min_distance;
    int audio_3d_max_distance;
    int audio_3d_rolloff;
    char audio_effect[32];
    char audio_node[MAX_PATH * 4];
    DWORD sync_hold_until_tick;
    int sync_hold_logged;
    audio_graph_t *audio_graph;
    engine_audio_player_t *engine_audio;
    DWORD audio_ready_tick;
    DWORD playback_start_tick;
    FILETIME sidecar_ini_write_time;
    DWORD last_config_check_tick;
    unsigned int config_generation;
    char image_path[MAX_PATH * 4];
    char sidecar_path[MAX_PATH * 4];
    char sidecar_ini_path[MAX_PATH * 4];
    webm_twitch_settings_t twitch_settings;
    webm_twitch_session_t *twitch_session;
    webm_twitch_chat_session_t *twitch_chat;
    char twitch_chat_channel[WEBM_TWITCH_CHANNEL_MAX];
    int twitch_active;
    int twitch_fallback_active;
    int twitch_logged_state;
    twitch_decoder_open_task_t *twitch_open_task;
    video_decoder_t *decoder;
} video_gl_texture_t;

static video_gl_texture_t video_gl_textures[16];
static video_gl_texture_t *video_gl_active_slots[16];
static int video_gl_active_count;
static video_gl_texture_t *current_gl_video_slot;
static void clear_video_gl_texture_slot(video_gl_texture_t *slot);

#define WEBM_TWITCH_CHAT_MIP_LEVELS 16

typedef struct {
    int active;
    IDirect3DTexture8 *texture;
    IDirect3DTexture8 *video_texture;
    int width;
    int height;
    UINT levels;
    UINT level_count;
    DWORD usage;
    D3DFORMAT format;
    D3DPOOL pool;
    DWORD last_update_tick;
    DWORD update_interval_ms;
    DWORD first_seen_tick;
    DWORD last_bound_tick;
    unsigned int last_bound_present_serial;
    DWORD bind_log_count;
    int frame;
    long uploaded_frame_index;
    unsigned int uploaded_frame_serial;
    int d3d8_mip_levels;
    int video_filtering;
    int anisotropy;
    int uv_mode_base;
    int uv_mode_twitch;
    int uv_rect_valid;
    float uv_min_u;
    float uv_min_v;
    float uv_max_u;
    float uv_max_v;
    int uv_override_logged;
    int hook5_resource_alias_logged;
    game_audio_mute_list_t game_audio_mutes;
    unsigned int filter_log_generation;
    int filter_log_filtering;
    int filter_log_anisotropy;
    ID3D11SamplerState *d3d11_sampler;
    ID3D11Texture2D *d3d11_texture;
    int d3d11_direct_upload_ready;
    int d3d11_direct_upload_logged;
    unsigned int d3d11_runtime_generation;
    UINT d3d11_mip_levels;
    unsigned int d3d11_sampler_generation;
    int d3d11_sampler_filtering;
    int d3d11_sampler_anisotropy;
    int decoder_attempted;
    webm_source_failure_state_t source_failure;
    int audio_enabled;
    int audio_engine;
    int engine_audio_runtime_failed;
    int audio_lead_ms;
    int audio_volume;
    int audio_3d;
    int audio_3d_min_distance;
    int audio_3d_max_distance;
    int audio_3d_rolloff;
    char audio_effect[32];
    char audio_node[MAX_PATH * 4];
    DWORD sync_hold_until_tick;
    int sync_hold_logged;
    audio_graph_t *audio_graph;
    engine_audio_player_t *engine_audio;
    DWORD audio_ready_tick;
    DWORD playback_start_tick;
    FILETIME sidecar_ini_write_time;
    DWORD last_config_check_tick;
    unsigned int config_generation;
    char image_path[MAX_PATH * 4];
    char sidecar_path[MAX_PATH * 4];
    char sidecar_ini_path[MAX_PATH * 4];
    webm_twitch_settings_t twitch_settings;
    webm_twitch_session_t *twitch_session;
    webm_twitch_chat_session_t *twitch_chat;
    char twitch_chat_channel[WEBM_TWITCH_CHANNEL_MAX];
    BYTE *twitch_chat_base_pixels;
    size_t twitch_chat_base_size;
    int twitch_chat_base_pitch;
    int twitch_chat_base_bpp;
    BYTE *twitch_chat_mip_pixels[WEBM_TWITCH_CHAT_MIP_LEVELS];
    size_t twitch_chat_mip_sizes[WEBM_TWITCH_CHAT_MIP_LEVELS];
    int twitch_chat_mip_pitches[WEBM_TWITCH_CHAT_MIP_LEVELS];
    int twitch_active;
    int twitch_fallback_active;
    int twitch_logged_state;
    twitch_decoder_open_task_t *twitch_open_task;
    video_decoder_t *decoder;
} video_d3d8_texture_t;

static video_d3d8_texture_t video_d3d8_textures[16];
static video_d3d8_texture_t *video_d3d8_active_slots[16];
static int video_d3d8_active_count;
static unsigned int d3d8_present_serial = 1;
static IDirect3DBaseTexture8 *d3d8_bound_textures[8];
static video_d3d8_texture_t *d3d8_bound_video_slots[8];

#define WEBM_D3D11_SAMPLER_STAGES 8
typedef struct {
    int active;
    IDirect3DBaseTexture8 *texture;
    ID3D11SamplerState *saved_sampler;
} d3d11_filter_override_t;

static d3d11_filter_override_t d3d11_filter_overrides[WEBM_D3D11_SAMPLER_STAGES];

typedef enum {
    WEBM_PERF_D3D8_TOTAL = 0,
    WEBM_PERF_D3D8_DECODE,
    WEBM_PERF_D3D8_UPLOAD,
    WEBM_PERF_D3D8_LOCK,
    WEBM_PERF_D3D8_CONVERT,
    WEBM_PERF_D3D8_UNLOCK,
    WEBM_PERF_D3D8_AUDIO,
    WEBM_PERF_GL_TOTAL,
    WEBM_PERF_GL_DECODE,
    WEBM_PERF_GL_CONVERT,
    WEBM_PERF_GL_UPLOAD,
    WEBM_PERF_GL_AUDIO,
    WEBM_PERF_PHASE_COUNT
} webm_perf_phase_t;

typedef struct {
    LARGE_INTEGER frequency;
    LONGLONG accumulated[WEBM_PERF_PHASE_COUNT];
    LONGLONG maximum[WEBM_PERF_PHASE_COUNT];
    DWORD calls[WEBM_PERF_PHASE_COUNT];
    DWORD decoded_d3d8;
    DWORD decoded_gl;
    DWORD uploaded_d3d8_mips;
    DWORD cached_d3d8_mips;
    DWORD contiguous_d3d8_mips;
    DWORD uploaded_gl_frames;
    DWORD cached_gl_frames;
    LONGLONG d3d8_mip_convert_accumulated[8];
    DWORD d3d8_mip_convert_calls[8];
    DWORD report_tick;
    int ready;
} webm_perf_state_t;

static webm_perf_state_t webm_perf_state;
static void stop_older_d3d8_sidecar_audio(video_d3d8_texture_t *owner, DWORD now);
static void stop_older_gl_sidecar_audio(video_gl_texture_t *owner, DWORD now);
static int is_recently_retired_sidecar_a(const char *path, DWORD now);
static int d3d8_has_active_same_target_audio(video_d3d8_texture_t *owner);
static int gl_has_active_same_target_audio(video_gl_texture_t *owner);
static DWORD d3d8_last_global_update_tick;
static int d3d8_video_write_enabled = 1;
static int d3d8_video_decode_enabled = 1;

static void register_active_gl_slot(video_gl_texture_t *slot)
{
    int i;
    if (!slot) return;
    for (i = 0; i < video_gl_active_count; i++) {
        if (video_gl_active_slots[i] == slot) return;
    }
    if (video_gl_active_count < (int)(sizeof(video_gl_active_slots) / sizeof(video_gl_active_slots[0]))) {
        video_gl_active_slots[video_gl_active_count++] = slot;
    }
}

static void unregister_active_gl_slot(video_gl_texture_t *slot)
{
    int i;
    if (!slot) return;
    for (i = 0; i < video_gl_active_count; i++) {
        if (video_gl_active_slots[i] == slot) {
            video_gl_active_count--;
            video_gl_active_slots[i] = video_gl_active_slots[video_gl_active_count];
            video_gl_active_slots[video_gl_active_count] = NULL;
            return;
        }
    }
}

static void register_active_d3d8_slot(video_d3d8_texture_t *slot)
{
    int i;
    if (!slot) return;
    for (i = 0; i < video_d3d8_active_count; i++) {
        if (video_d3d8_active_slots[i] == slot) return;
    }
    if (video_d3d8_active_count < (int)(sizeof(video_d3d8_active_slots) / sizeof(video_d3d8_active_slots[0]))) {
        video_d3d8_active_slots[video_d3d8_active_count++] = slot;
    }
}

static void unregister_active_d3d8_slot(video_d3d8_texture_t *slot)
{
    int i;
    if (!slot) return;
    for (i = 0; i < video_d3d8_active_count; i++) {
        if (video_d3d8_active_slots[i] == slot) {
            video_d3d8_active_count--;
            video_d3d8_active_slots[i] = video_d3d8_active_slots[video_d3d8_active_count];
            video_d3d8_active_slots[video_d3d8_active_count] = NULL;
            return;
        }
    }
}

static const GUID CLSID_FilterGraph_ = {0xe436ebb3,0x524f,0x11ce,{0x9f,0x53,0x00,0x20,0xaf,0x0b,0xa7,0x70}};
static const GUID IID_IGraphBuilder_ = {0x56a868a9,0x0ad4,0x11ce,{0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70}};
static const GUID IID_IClassFactory_ = {0x00000001,0x0000,0x0000,{0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
static const GUID IID_IBaseFilter_ = {0x56a86895,0x0ad4,0x11ce,{0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70}};
static const GUID IID_IFileSourceFilter_ = {0x56a868a6,0x0ad4,0x11ce,{0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70}};
static const GUID CLSID_LAVSplitterSource_ = {0xb98d13e7,0x55db,0x4385,{0xa3,0x3d,0x09,0xfd,0x1b,0xa2,0x63,0x38}};
static const GUID CLSID_LAVVideoDecoder_ = {0xee30215d,0x164f,0x4a92,{0xa4,0xeb,0x9d,0x4c,0x13,0x39,0x0f,0x9f}};
static const GUID CLSID_LAVAudioDecoder_ = {0xe8e73b6b,0x4cb3,0x44a4,{0xbe,0x99,0x4f,0x7b,0xcb,0x96,0xe4,0x91}};
static const GUID CLSID_DSoundRender_ = {0x79376820,0x07d0,0x11cf,{0xa2,0x4d,0x00,0x20,0xaf,0xd7,0x97,0x67}};
static const GUID CLSID_SampleGrabber_ = {0xc1f400a0,0x3f08,0x11d3,{0x9f,0x0b,0x00,0x60,0x08,0x03,0x9e,0x37}};
static const GUID CLSID_NullRenderer_ = {0xc1f400a4,0x3f08,0x11d3,{0x9f,0x0b,0x00,0x60,0x08,0x03,0x9e,0x37}};
static const GUID IID_IMediaControl_ = {0x56a868b1,0x0ad4,0x11ce,{0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70}};
static const GUID IID_IMediaSeeking_ = {0x36b73880,0xc2c8,0x11cf,{0x8b,0x46,0x00,0x80,0x5f,0x6c,0xef,0x60}};
static const GUID IID_IMediaEvent_ = {0x56a868b6,0x0ad4,0x11ce,{0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70}};
static const GUID IID_IBasicAudio_ = {0x56a868b3,0x0ad4,0x11ce,{0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70}};
static const GUID IID_IUnknown_ = {0x00000000,0x0000,0x0000,{0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
static const GUID IID_ISampleGrabber_ = {0x6b652fff,0x11fe,0x4fce,{0x92,0xad,0x02,0x66,0xb5,0xd7,0xc7,0x8f}};
static const GUID IID_ISampleGrabberCB_ = {0x0579154a,0x2b53,0x4994,{0xb0,0xd0,0xe7,0x73,0x14,0x8e,0xff,0x85}};
static const GUID MEDIATYPE_Video_ = {0x73646976,0x0000,0x0010,{0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71}};
static const GUID MEDIATYPE_Audio_ = {0x73647561,0x0000,0x0010,{0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71}};
static const GUID MEDIASUBTYPE_RGB24_ = {0xe436eb7d,0x524f,0x11ce,{0x9f,0x53,0x00,0x20,0xaf,0x0b,0xa7,0x70}};
static const GUID MEDIASUBTYPE_PCM_ = {0x00000001,0x0000,0x0010,{0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71}};
static const GUID FORMAT_VideoInfo_ = {0x05589f80,0xc356,0x11ce,{0xbf,0x01,0x00,0xaa,0x00,0x55,0x59,0x5a}};
static const GUID FORMAT_WaveFormatEx_ = {0x05589f81,0xc356,0x11ce,{0xbf,0x01,0x00,0xaa,0x00,0x55,0x59,0x5a}};

static void wide_to_mb(LPCWSTR in, char *out, size_t outsz);
static int guid_equal(REFGUID a, REFGUID b);
static void guid_to_text(REFGUID g, char *out, size_t outsz);
static void dirname_inplace(char *path);
static void path_join(char *out, size_t outsz, const char *a, const char *b);
static int webm_extension_root_a(char *out, size_t outsz);
static void webm_component_dir_a(char *out, size_t outsz,
                                 const char *component_name);
static void cache_root_a(char *out, size_t outsz);
static int webm_alias_to_real_w(LPCWSTR alias, wchar_t *real, size_t real_count);
static int ends_with_i(const char *s, const char *suffix);
static int ends_with_w_i(LPCWSTR s, LPCWSTR suffix);
static const char *find_i(const char *s, const char *needle);
static const char *find_scene_decl_i(const char *src, const char *needle);
static int contains_i(const char *s, const char *needle);
static int ptr_readable(const void *p, size_t bytes);
static void patch_all_modules(void);
static void resolve_gl_runtime_functions(void);
static void patch_iat(HMODULE mod, const char *dll, const char *name, void *hook, void **real);
static int patch_iat_pointer(HMODULE mod, void *target, void *hook);
static int create_scene_video_rewrite_w(LPCWSTR original, wchar_t *redirect, size_t redirect_count);
static int create_engine_audio_script_rewrite_a(const char *original, char *redirect, size_t redirect_count);
static int create_engine_audio_script_rewrite_w(LPCWSTR original, wchar_t *redirect, size_t redirect_count);
static int should_log_engine_audio_toy_probe_a(const char *path);
static void basename_no_ext_a(const char *path, char *out, size_t outsz);
static int infer_toy_audio_parent_path_a(const char *sidecar, char *out, size_t outsz);
static int infer_room_audio_parent_path_a(const char *sidecar, char *out, size_t outsz);
static int infer_room_audio_geometry_path_a(const char *sidecar, char *out, size_t outsz);
static int infer_room_audio_transform_position_a(const char *sidecar, char *out, size_t outsz);
static int infer_room_audio_named_transform_position_a(const char *sidecar, const char *node, char *out, size_t outsz);
static int infer_room_audio_source_position_a(const char *sidecar, char *out, size_t outsz);
static int infer_room_audio_sound_anchor_path_a(const char *sidecar, char *out, size_t outsz);
static int path_is_addon_toy_a(const char *path);
static int path_is_addon_room_a(const char *path);
static int is_exact_texture_probe_name_a(const char *s);
static int clamp_int(int value, int min_value, int max_value);
static void load_config(void);
static void refresh_global_config(DWORD now);
static void reload_global_config_now(void);
static int slider_widget_value(void *slider, float *out_value);
static void THISCALL hook_ConfigEditor_ParamChange(void *self, const char *parameter,
                                                   const char *value, DWORD arg3, DWORD arg4);
static void flush_twitch_chat_opacity(DWORD now);
static void video_decoder_release(video_decoder_t *dec);
static void video_decoder_stop_async(video_decoder_t *dec);
static void patch_d3d8_object(IDirect3D8 *d3d);
static void patch_d3d8_device(IDirect3DDevice8 *dev);
static int patch_vtable_slot(void *obj, int index, void *hook, void **real);
static void game_audio_track_source(void *source, const char *name);
static void *THISCALL hook_SoundDeviceCreateSource(void *device, const char *name,
                                                    unsigned int flags, int cache_mode);

typedef struct {
    HANDLE handle;
    WIN32_FIND_DATAW entries[64];
    int count;
    int index;
    int after_real;
} virtual_find_t;

static virtual_find_t virtual_finds[16];
static HANDLE webm_handles[32];

static void vlog_line(const char *fmt, va_list ap)
{
    FILE *f;
    if (!log_ready) return;
    EnterCriticalSection(&log_lock);
    CreateDirectoryA("..\\Logs", NULL);
    f = fopen("..\\Logs\\NC-TK17-WebM.log", "ab");
    if (f) {
        vfprintf(f, fmt, ap);
        fputc('\n', f);
        fclose(f);
    }
    LeaveCriticalSection(&log_lock);
}

static void log_line(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vlog_line(fmt, ap);
    va_end(ap);
}

static void debug_line(const char *fmt, ...)
{
    va_list ap;
    if (!debug_logging) return;
    va_start(ap, fmt);
    vlog_line(fmt, ap);
    va_end(ap);
}

static void webm_perf_prepare(DWORD now)
{
    if (!performance_profile) {
        if (webm_perf_state.ready) memset(&webm_perf_state, 0, sizeof(webm_perf_state));
        return;
    }
    if (webm_perf_state.ready) return;
    memset(&webm_perf_state, 0, sizeof(webm_perf_state));
    if (!QueryPerformanceFrequency(&webm_perf_state.frequency) ||
        webm_perf_state.frequency.QuadPart <= 0) {
        return;
    }
    webm_perf_state.report_tick = now;
    webm_perf_state.ready = 1;
}

static LONGLONG webm_perf_counter(void)
{
    LARGE_INTEGER value;
    if (!performance_profile || !webm_perf_state.ready) return 0;
    QueryPerformanceCounter(&value);
    return value.QuadPart;
}

static void webm_perf_add(webm_perf_phase_t phase, LONGLONG start)
{
    LARGE_INTEGER end;
    LONGLONG elapsed;
    if (!start || phase < 0 || phase >= WEBM_PERF_PHASE_COUNT ||
        !performance_profile || !webm_perf_state.ready) {
        return;
    }
    QueryPerformanceCounter(&end);
    elapsed = end.QuadPart - start;
    if (elapsed < 0) return;
    webm_perf_state.accumulated[phase] += elapsed;
    if (elapsed > webm_perf_state.maximum[phase]) webm_perf_state.maximum[phase] = elapsed;
    webm_perf_state.calls[phase]++;
}

static void webm_perf_add_d3d8_mip_convert(UINT level, LONGLONG start)
{
    LARGE_INTEGER end;
    LONGLONG elapsed;
    if (!start || level >= 8 || !performance_profile || !webm_perf_state.ready) return;
    QueryPerformanceCounter(&end);
    elapsed = end.QuadPart - start;
    if (elapsed < 0) return;
    webm_perf_state.accumulated[WEBM_PERF_D3D8_CONVERT] += elapsed;
    if (elapsed > webm_perf_state.maximum[WEBM_PERF_D3D8_CONVERT]) {
        webm_perf_state.maximum[WEBM_PERF_D3D8_CONVERT] = elapsed;
    }
    webm_perf_state.calls[WEBM_PERF_D3D8_CONVERT]++;
    webm_perf_state.d3d8_mip_convert_accumulated[level] += elapsed;
    webm_perf_state.d3d8_mip_convert_calls[level]++;
}

static double webm_perf_ms(LONGLONG ticks)
{
    if (!webm_perf_state.ready || webm_perf_state.frequency.QuadPart <= 0) return 0.0;
    return (double)ticks * 1000.0 / (double)webm_perf_state.frequency.QuadPart;
}

static double webm_perf_average(webm_perf_phase_t phase)
{
    DWORD calls = webm_perf_state.calls[phase];
    if (!calls) return 0.0;
    return webm_perf_ms(webm_perf_state.accumulated[phase]) / (double)calls;
}

static void webm_perf_report(DWORD now)
{
    DWORD window_ms;
    if (!performance_profile || !webm_perf_state.ready) return;
    window_ms = now - webm_perf_state.report_tick;
    if (window_ms < 2000u) return;
    log_line("performance profile window_ms=%lu active d3d8=%d gl=%d calls d3d8_present=%lu gl_swap=%lu d3d8_decode=%lu d3d8_upload=%lu d3d8_audio=%lu gl_decode=%lu gl_convert=%lu gl_upload=%lu gl_audio=%lu avg_ms d3d8_total=%.3f d3d8_decode=%.3f d3d8_upload=%.3f d3d8_audio=%.3f gl_total=%.3f gl_decode=%.3f gl_convert=%.3f gl_upload=%.3f gl_audio=%.3f max_ms d3d8_total=%.3f d3d8_decode=%.3f d3d8_upload=%.3f gl_total=%.3f gl_decode=%.3f gl_convert=%.3f gl_upload=%.3f work decoded_d3d8=%lu decoded_gl=%lu uploaded_d3d8_mips=%lu cached_d3d8_mips=%lu uploaded_gl_frames=%lu cached_gl_frames=%lu contiguous_d3d8_mips=%lu",
             (unsigned long)window_ms,
             video_d3d8_active_count, video_gl_active_count,
             (unsigned long)webm_perf_state.calls[WEBM_PERF_D3D8_TOTAL],
             (unsigned long)webm_perf_state.calls[WEBM_PERF_GL_TOTAL],
             (unsigned long)webm_perf_state.calls[WEBM_PERF_D3D8_DECODE],
             (unsigned long)webm_perf_state.calls[WEBM_PERF_D3D8_UPLOAD],
             (unsigned long)webm_perf_state.calls[WEBM_PERF_D3D8_AUDIO],
             (unsigned long)webm_perf_state.calls[WEBM_PERF_GL_DECODE],
             (unsigned long)webm_perf_state.calls[WEBM_PERF_GL_CONVERT],
             (unsigned long)webm_perf_state.calls[WEBM_PERF_GL_UPLOAD],
             (unsigned long)webm_perf_state.calls[WEBM_PERF_GL_AUDIO],
             webm_perf_average(WEBM_PERF_D3D8_TOTAL),
             webm_perf_average(WEBM_PERF_D3D8_DECODE),
             webm_perf_average(WEBM_PERF_D3D8_UPLOAD),
             webm_perf_average(WEBM_PERF_D3D8_AUDIO),
             webm_perf_average(WEBM_PERF_GL_TOTAL),
             webm_perf_average(WEBM_PERF_GL_DECODE),
             webm_perf_average(WEBM_PERF_GL_CONVERT),
             webm_perf_average(WEBM_PERF_GL_UPLOAD),
             webm_perf_average(WEBM_PERF_GL_AUDIO),
             webm_perf_ms(webm_perf_state.maximum[WEBM_PERF_D3D8_TOTAL]),
             webm_perf_ms(webm_perf_state.maximum[WEBM_PERF_D3D8_DECODE]),
             webm_perf_ms(webm_perf_state.maximum[WEBM_PERF_D3D8_UPLOAD]),
             webm_perf_ms(webm_perf_state.maximum[WEBM_PERF_GL_TOTAL]),
             webm_perf_ms(webm_perf_state.maximum[WEBM_PERF_GL_DECODE]),
             webm_perf_ms(webm_perf_state.maximum[WEBM_PERF_GL_CONVERT]),
             webm_perf_ms(webm_perf_state.maximum[WEBM_PERF_GL_UPLOAD]),
             (unsigned long)webm_perf_state.decoded_d3d8,
             (unsigned long)webm_perf_state.decoded_gl,
             (unsigned long)webm_perf_state.uploaded_d3d8_mips,
             (unsigned long)webm_perf_state.cached_d3d8_mips,
             (unsigned long)webm_perf_state.uploaded_gl_frames,
             (unsigned long)webm_perf_state.cached_gl_frames,
             (unsigned long)webm_perf_state.contiguous_d3d8_mips);
    if (webm_perf_state.calls[WEBM_PERF_D3D8_CONVERT]) {
        log_line("performance d3d8 upload detail calls lock=%lu convert=%lu unlock=%lu avg_ms lock=%.3f convert=%.3f unlock=%.3f max_ms lock=%.3f convert=%.3f unlock=%.3f mip_calls=%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu mip_convert_avg_ms=%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f mip_convert_total_ms=%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
                 (unsigned long)webm_perf_state.calls[WEBM_PERF_D3D8_LOCK],
                 (unsigned long)webm_perf_state.calls[WEBM_PERF_D3D8_CONVERT],
                 (unsigned long)webm_perf_state.calls[WEBM_PERF_D3D8_UNLOCK],
                 webm_perf_average(WEBM_PERF_D3D8_LOCK),
                 webm_perf_average(WEBM_PERF_D3D8_CONVERT),
                 webm_perf_average(WEBM_PERF_D3D8_UNLOCK),
                 webm_perf_ms(webm_perf_state.maximum[WEBM_PERF_D3D8_LOCK]),
                 webm_perf_ms(webm_perf_state.maximum[WEBM_PERF_D3D8_CONVERT]),
                 webm_perf_ms(webm_perf_state.maximum[WEBM_PERF_D3D8_UNLOCK]),
                 (unsigned long)webm_perf_state.d3d8_mip_convert_calls[0],
                 (unsigned long)webm_perf_state.d3d8_mip_convert_calls[1],
                 (unsigned long)webm_perf_state.d3d8_mip_convert_calls[2],
                 (unsigned long)webm_perf_state.d3d8_mip_convert_calls[3],
                 (unsigned long)webm_perf_state.d3d8_mip_convert_calls[4],
                 (unsigned long)webm_perf_state.d3d8_mip_convert_calls[5],
                 (unsigned long)webm_perf_state.d3d8_mip_convert_calls[6],
                 (unsigned long)webm_perf_state.d3d8_mip_convert_calls[7],
                 webm_perf_state.d3d8_mip_convert_calls[0] ? webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[0]) / (double)webm_perf_state.d3d8_mip_convert_calls[0] : 0.0,
                 webm_perf_state.d3d8_mip_convert_calls[1] ? webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[1]) / (double)webm_perf_state.d3d8_mip_convert_calls[1] : 0.0,
                 webm_perf_state.d3d8_mip_convert_calls[2] ? webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[2]) / (double)webm_perf_state.d3d8_mip_convert_calls[2] : 0.0,
                 webm_perf_state.d3d8_mip_convert_calls[3] ? webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[3]) / (double)webm_perf_state.d3d8_mip_convert_calls[3] : 0.0,
                 webm_perf_state.d3d8_mip_convert_calls[4] ? webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[4]) / (double)webm_perf_state.d3d8_mip_convert_calls[4] : 0.0,
                 webm_perf_state.d3d8_mip_convert_calls[5] ? webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[5]) / (double)webm_perf_state.d3d8_mip_convert_calls[5] : 0.0,
                 webm_perf_state.d3d8_mip_convert_calls[6] ? webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[6]) / (double)webm_perf_state.d3d8_mip_convert_calls[6] : 0.0,
                 webm_perf_state.d3d8_mip_convert_calls[7] ? webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[7]) / (double)webm_perf_state.d3d8_mip_convert_calls[7] : 0.0,
                 webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[0]),
                 webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[1]),
                 webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[2]),
                 webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[3]),
                 webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[4]),
                 webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[5]),
                 webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[6]),
                 webm_perf_ms(webm_perf_state.d3d8_mip_convert_accumulated[7]));
    }
    memset(webm_perf_state.accumulated, 0, sizeof(webm_perf_state.accumulated));
    memset(webm_perf_state.maximum, 0, sizeof(webm_perf_state.maximum));
    memset(webm_perf_state.calls, 0, sizeof(webm_perf_state.calls));
    memset(webm_perf_state.d3d8_mip_convert_accumulated, 0, sizeof(webm_perf_state.d3d8_mip_convert_accumulated));
    memset(webm_perf_state.d3d8_mip_convert_calls, 0, sizeof(webm_perf_state.d3d8_mip_convert_calls));
    webm_perf_state.decoded_d3d8 = 0;
    webm_perf_state.decoded_gl = 0;
    webm_perf_state.uploaded_d3d8_mips = 0;
    webm_perf_state.cached_d3d8_mips = 0;
    webm_perf_state.contiguous_d3d8_mips = 0;
    webm_perf_state.uploaded_gl_frames = 0;
    webm_perf_state.cached_gl_frames = 0;
    webm_perf_state.report_tick = now;
}

static void dump_recent_object_vtable(const char *label, void *obj, int entries)
{
    (void)label;
    (void)obj;
    (void)entries;
}

static int should_skip_active_mod_static_texture_a(const char *path)
{
    char base[MAX_PATH];
    if (!scene_video_rewrite_enabled) return 0;
    if (!path || !active_video_scene_base[0]) return 0;
    if ((GetTickCount() - active_video_scene_tick) > 60000) return 0;
    if (!contains_i(path, "ActiveMod")) return 0;
    if (!is_exact_texture_probe_name_a(path)) return 0;
    basename_no_ext_a(path, base, sizeof(base));
    return _stricmp(base, active_video_scene_base) == 0;
}

static void log_target_pending_object(const char *label, void *obj)
{
    (void)label;
    (void)obj;
}

static int ptr_readable(const void *p, size_t bytes)
{
    MEMORY_BASIC_INFORMATION mbi;
    DWORD protect;
    if (!p || !VirtualQuery(p, &mbi, sizeof(mbi))) return 0;
    protect = mbi.Protect & 0xff;
    if (mbi.State != MEM_COMMIT) return 0;
    if (protect == PAGE_NOACCESS || protect == PAGE_EXECUTE || (mbi.Protect & PAGE_GUARD)) return 0;
    return (const BYTE*)p + bytes <= (const BYTE*)mbi.BaseAddress + mbi.RegionSize;
}

static int ptr_executable(const void *p)
{
    MEMORY_BASIC_INFORMATION mbi;
    DWORD protect;
    if (!p || !VirtualQuery(p, &mbi, sizeof(mbi)) ||
        mbi.State != MEM_COMMIT || (mbi.Protect & PAGE_GUARD)) return 0;
    protect = mbi.Protect & 0xff;
    return protect == PAGE_EXECUTE || protect == PAGE_EXECUTE_READ ||
           protect == PAGE_EXECUTE_READWRITE ||
           protect == PAGE_EXECUTE_WRITECOPY;
}

static int copy_engine_string_a(const char *value, char *out, size_t outsz)
{
    int length;
    size_t i;
    if (!out || !outsz) return 0;
    out[0] = 0;
    if (!value || !ptr_readable(value, 1)) return 0;
    if (ptr_readable(value - sizeof(int), sizeof(int))) {
        memcpy(&length, value - sizeof(int), sizeof(length));
        if (length >= 0 && length < 32768 && ptr_readable(value, (size_t)length + 1)) {
            size_t copy_length = (size_t)length;
            if (copy_length >= outsz) copy_length = outsz - 1;
            memcpy(out, value, copy_length);
            out[copy_length] = 0;
            return 1;
        }
    }
    for (i = 0; i + 1 < outsz; i++) {
        if (!ptr_readable(value + i, 1)) return 0;
        out[i] = value[i];
        if (!out[i]) return 1;
    }
    out[outsz - 1] = 0;
    return 1;
}

static int override_value_is_enabled_a(const char *value)
{
    return value && (_stricmp(value, "ON") == 0 || _stricmp(value, "true") == 0 ||
                     _stricmp(value, "yes") == 0 || strcmp(value, "1") == 0);
}

static void write_twitch_override_value_a(const char *key, const char *value)
{
    if (!key || !value) return;
    load_config();
    if (!config_path_global[0]) return;
    if (!WritePrivateProfileStringA("NC-TK17-WebM:TwitchOverride", key, value,
                                    config_path_global)) {
        log_line("Twitch override write failed key=\"%s\"", key);
        return;
    }
    reload_global_config_now();
}

static void show_twitch_channel_dialog(void)
{
    char channel[WEBM_TWITCH_CHANNEL_LIST_MAX];
    DWORD now = GetTickCount();
    if (twitch_channel_dialog_active ||
        (twitch_channel_dialog_last_tick && now - twitch_channel_dialog_last_tick < 500)) {
        return;
    }
    twitch_channel_dialog_active = 1;
    lstrcpynA(channel, twitch_override_channel, sizeof(channel));
    if (webm_channel_dialog_show(self_module, twitch_override_channel,
                                 channel, sizeof(channel))) {
        write_twitch_override_value_a("channel", channel);
    }
    twitch_channel_dialog_last_tick = GetTickCount();
    twitch_channel_dialog_active = 0;
}

static int custom_parameter_id(void *parameter, int *out_id)
{
    HMODULE executable;
    BYTE *base;
    DWORD member_id;
    BYTE dispatch_slot;
    BYTE *metadata;
    BYTE *dispatch_table;
    script_get_i32_t getter;
    if (!parameter || !out_id || !ptr_readable((BYTE*)parameter - 0x18, sizeof(void*))) return 0;
    executable = GetModuleHandleA(NULL);
    if (!executable) return 0;
    base = (BYTE*)executable;
    if (!ptr_readable(base + 0x002b0434, sizeof(member_id))) return 0;
    memcpy(&member_id, base + 0x002b0434, sizeof(member_id));
    dispatch_slot = *(base + 0x002b0437);
    memcpy(&metadata, (BYTE*)parameter - 0x18, sizeof(metadata));
    if (!metadata ||
        !ptr_readable(metadata + (member_id & 0x0fff) * sizeof(void*), sizeof(dispatch_table))) {
        return 0;
    }
    memcpy(&dispatch_table, metadata + (member_id & 0x0fff) * sizeof(void*),
           sizeof(dispatch_table));
    if (!dispatch_table ||
        !ptr_readable(dispatch_table + ((size_t)dispatch_slot << 6), sizeof(getter))) {
        return 0;
    }
    memcpy(&getter, dispatch_table + ((size_t)dispatch_slot << 6), sizeof(getter));
    if (!getter) return 0;
    *out_id = getter(parameter, member_id);
    return 1;
}

static int custom_parameter_name(void *parameter, char *out, size_t outsz)
{
    HMODULE executable;
    BYTE *base;
    DWORD member_id;
    BYTE dispatch_slot;
    BYTE *metadata;
    BYTE *dispatch_table;
    script_get_string_t getter;
    engine_string_release_t release_string = NULL;
    char *engine_value;
    int copied;
    if (!out || !outsz) return 0;
    out[0] = 0;
    if (!parameter || !ptr_readable((BYTE*)parameter - 0x18, sizeof(void*))) return 0;
    executable = GetModuleHandleA(NULL);
    if (!executable) return 0;
    base = (BYTE*)executable;
    if (!ptr_readable(base + 0x002b0438, sizeof(member_id)) ||
        !ptr_readable(base + 0x002b0d80, sizeof(engine_value))) {
        return 0;
    }
    memcpy(&member_id, base + 0x002b0438, sizeof(member_id));
    dispatch_slot = *(base + 0x002b043b);
    memcpy(&metadata, (BYTE*)parameter - 0x18, sizeof(metadata));
    if (!metadata ||
        !ptr_readable(metadata + (member_id & 0x0fff) * sizeof(void*), sizeof(dispatch_table))) {
        return 0;
    }
    memcpy(&dispatch_table, metadata + (member_id & 0x0fff) * sizeof(void*),
           sizeof(dispatch_table));
    if (!dispatch_table ||
        !ptr_readable(dispatch_table + ((size_t)dispatch_slot << 6), sizeof(getter))) {
        return 0;
    }
    memcpy(&getter, dispatch_table + ((size_t)dispatch_slot << 6), sizeof(getter));
    if (!getter) return 0;
    memcpy(&engine_value, base + 0x002b0d80, sizeof(engine_value));
    getter(parameter, member_id, &engine_value);
    copied = copy_engine_string_a(engine_value, out, outsz);
    if (ptr_readable(base + 0x0024a9e8, sizeof(release_string))) {
        memcpy(&release_string, base + 0x0024a9e8, sizeof(release_string));
        if (release_string) release_string(&engine_value);
    }
    return copied;
}

static int widget_text_value(void *widget, char *out, size_t outsz)
{
    HMODULE executable;
    BYTE *base;
    BYTE *metadata;
    BYTE *dispatch_table;
    script_get_string_t getter;
    engine_string_release_t release_string = NULL;
    char *engine_value;
    int copied;
    const DWORD member_id = 0x04fff0eb;
    const BYTE dispatch_slot = 4;
    if (!out || !outsz) return 0;
    out[0] = 0;
    if (!widget || !ptr_readable((BYTE*)widget - 0x18, sizeof(metadata))) return 0;
    executable = GetModuleHandleA(NULL);
    if (!executable) return 0;
    base = (BYTE*)executable;
    if (!ptr_readable(base + 0x002b0d80, sizeof(engine_value))) return 0;
    memcpy(&metadata, (BYTE*)widget - 0x18, sizeof(metadata));
    if (!metadata ||
        !ptr_readable(metadata + (member_id & 0x0fff) * sizeof(void*),
                      sizeof(dispatch_table))) {
        return 0;
    }
    memcpy(&dispatch_table,
           metadata + (member_id & 0x0fff) * sizeof(void*),
           sizeof(dispatch_table));
    if (!dispatch_table ||
        !ptr_readable(dispatch_table + ((size_t)dispatch_slot << 6), sizeof(getter))) {
        return 0;
    }
    memcpy(&getter, dispatch_table + ((size_t)dispatch_slot << 6), sizeof(getter));
    if (!getter) return 0;
    memcpy(&engine_value, base + 0x002b0d80, sizeof(engine_value));
    getter(widget, member_id, &engine_value);
    copied = copy_engine_string_a(engine_value, out, outsz);
    if (ptr_readable(base + 0x0024a9e8, sizeof(release_string))) {
        memcpy(&release_string, base + 0x0024a9e8, sizeof(release_string));
        if (release_string) release_string(&engine_value);
    }
    return copied;
}

static webm_setting_binding_t *webm_setting_binding_by_name(const char *name)
{
    size_t index;
    if (!name) return NULL;
    for (index = 0;
         index < sizeof(webm_setting_bindings) / sizeof(webm_setting_bindings[0]);
         ++index) {
        if (strcmp(webm_setting_bindings[index].param_name, name) == 0)
            return &webm_setting_bindings[index];
    }
    return NULL;
}

static int widget_text_set_value(void *widget, const char *value)
{
    HMODULE executable;
    BYTE *base;
    BYTE *metadata;
    BYTE *dispatch_table;
    engine_string_construct_cstr_t construct_string;
    engine_string_release_t release_string;
    widget_set_string_t setter;
    char *engine_value = NULL;
    const DWORD member_id = 0x04fff0eb;
    if (!widget || !value ||
        !ptr_readable((BYTE*)widget - 0x18, sizeof(metadata))) return 0;
    executable = GetModuleHandleA(NULL);
    if (!executable) return 0;
    base = (BYTE*)executable;
    if (!ptr_readable(base + 0x0024a9e4, sizeof(construct_string)) ||
        !ptr_readable(base + 0x0024a9e8, sizeof(release_string))) return 0;
    memcpy(&metadata, (BYTE*)widget - 0x18, sizeof(metadata));
    if (!metadata ||
        !ptr_readable(metadata + (member_id & 0x0fff) * sizeof(void*),
                      sizeof(dispatch_table))) return 0;
    memcpy(&dispatch_table,
           metadata + (member_id & 0x0fff) * sizeof(void*),
           sizeof(dispatch_table));
    if (!dispatch_table ||
        !ptr_readable(dispatch_table + 0x104, sizeof(setter))) return 0;
    memcpy(&setter, dispatch_table + 0x104, sizeof(setter));
    memcpy(&construct_string, base + 0x0024a9e4, sizeof(construct_string));
    memcpy(&release_string, base + 0x0024a9e8, sizeof(release_string));
    if (!ptr_executable((const void*)setter) ||
        !ptr_executable((const void*)construct_string) ||
        !ptr_executable((const void*)release_string)) return 0;
    construct_string(&engine_value, value);
    if (!engine_value) return 0;
    setter(widget, member_id, engine_value);
    release_string(&engine_value);
    return 1;
}

static int slider_widget_set_value(void *slider, float value)
{
    BYTE *metadata;
    BYTE *dispatch_table;
    widget_set_float_t setter;
    if (!slider || !_finite(value) ||
        !ptr_readable((BYTE*)slider - 0x18, sizeof(metadata))) return 0;
    memcpy(&metadata, (BYTE*)slider - 0x18, sizeof(metadata));
    if (!metadata || !ptr_readable(metadata + 0x3b4, sizeof(dispatch_table)))
        return 0;
    memcpy(&dispatch_table, metadata + 0x3b4, sizeof(dispatch_table));
    if (!dispatch_table ||
        !ptr_readable(dispatch_table + 0x84, sizeof(setter))) return 0;
    memcpy(&setter, dispatch_table + 0x84, sizeof(setter));
    if (!ptr_executable((const void*)setter)) return 0;
    setter(slider, 0x02fff0ed, value);
    return 1;
}

static int webm_setting_ini_spinbox_value(
    const webm_setting_binding_t *binding, char *out, size_t outsz)
{
    if (!binding || !out || !outsz || binding->type != WEBM_SETTING_SPINBOX)
        return 0;
    load_config();
    out[0] = 0;
    if (!config_path_global[0] ||
        !GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", binding->key,
                                  "", out, (DWORD)outsz,
                                  config_path_global) || !out[0]) return 0;
    if (strcmp(binding->key, "enabled") == 0 ||
        strcmp(binding->key, "chat_enabled") == 0) {
        lstrcpynA(out, override_value_is_enabled_a(out) ? "ON" : "OFF",
                  (int)outsz);
    } else if (strcmp(binding->key, "chat_overlay") == 0 ||
               strcmp(binding->key, "chat_animated_emotes") == 0) {
        lstrcpynA(out, override_value_is_enabled_a(out) ? "true" : "false",
                  (int)outsz);
    } else if (strcmp(binding->key, "channel_offline_fallback") == 0) {
        lstrcpynA(out, _stricmp(out, "random") == 0 ? "random" : "fallback",
                  (int)outsz);
    } else if (strcmp(binding->key, "chat_position") == 0) {
        lstrcpynA(out, _stricmp(out, "left") == 0 ? "left" : "right",
                  (int)outsz);
    }
    return 1;
}

static void webm_sync_spinbox_from_ini(
    const webm_setting_binding_t *binding, void *widget)
{
    char requested[MAX_PATH * 4];
    char current[MAX_PATH * 4];
    if (!binding || !widget ||
        !webm_setting_ini_spinbox_value(binding, requested,
                                         sizeof(requested))) return;
    if (widget_text_value(widget, current, sizeof(current)) &&
        strcmp(current, requested) == 0) return;
    webm_setting_sync_depth++;
    if (!widget_text_set_value(widget, requested)) {
        webm_setting_sync_depth--;
        log_line("ConfigEditor spinbox sync failed param=\"%s\" requested=\"%s\"",
                 binding->param_name, requested);
        return;
    }
    webm_setting_sync_depth--;
    debug_line("ConfigEditor spinbox synced param=\"%s\" ini=\"%s\"",
               binding->param_name, requested);
}

static int webm_setting_ini_slider_value(
    const webm_setting_binding_t *binding, float *out_value)
{
    char text[64];
    char *end = NULL;
    double parsed;
    if (!binding || !out_value || binding->type != WEBM_SETTING_SLIDER)
        return 0;
    load_config();
    text[0] = 0;
    if (!config_path_global[0] ||
        !GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", binding->key,
                                  "", text, sizeof(text),
                                  config_path_global) || !text[0]) return 0;
    parsed = strtod(text, &end);
    if (end == text || !_finite(parsed)) return 0;
    while (*end && isspace((unsigned char)*end)) ++end;
    if (*end) return 0;
    if (strcmp(binding->key, "chat_width") == 0) {
        if (parsed < 0.20) parsed = 0.20;
        if (parsed > 0.60) parsed = 0.60;
    } else {
        if (parsed < 0.10) parsed = 0.10;
        if (parsed > 1.0) parsed = 1.0;
    }
    *out_value = (float)parsed;
    return 1;
}

static void webm_sync_slider_from_ini(webm_setting_binding_t *binding,
                                       void *slider)
{
    float requested;
    float current;
    if (!binding || !slider ||
        !webm_setting_ini_slider_value(binding, &requested)) return;
    if (slider_widget_value(slider, &current) &&
        fabsf(current - requested) <= 0.00001f) return;
    webm_setting_sync_depth++;
    if (!slider_widget_set_value(slider, requested)) {
        webm_setting_sync_depth--;
        log_line("ConfigEditor slider sync failed param=\"%s\" requested=%.6g",
                 binding->param_name, requested);
        return;
    }
    webm_setting_sync_depth--;
    debug_line("ConfigEditor slider synced param=\"%s\" ini=%.6g",
               binding->param_name, requested);
}

static void trim_ascii_inplace(char *value)
{
    char *start;
    char *end;
    size_t length;
    if (!value) return;
    start = value;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != value) {
        length = strlen(start);
        memmove(value, start, length + 1);
    }
    end = value + strlen(value);
    while (end > value && isspace((unsigned char)end[-1])) end--;
    *end = 0;
}

static int THISCALL hook_Customizer_BuildControls(void *self, void *arg1,
                                                   void *arg2, void *arg3)
{
    int result;
    void **parameters = NULL;
    void **records = NULL;
    int parameter_count = 0;
    int record_count = 0;
    int count;
    int i;
    result = tramp_Customizer_BuildControls ?
        tramp_Customizer_BuildControls(self, arg1, arg2, arg3) : 0;
    if (!self ||
        !ptr_readable((BYTE*)self + 0x14, sizeof(parameters)) ||
        !ptr_readable((BYTE*)self + 0x18, sizeof(records))) {
        return result;
    }
    memcpy(&parameters, (BYTE*)self + 0x14, sizeof(parameters));
    memcpy(&records, (BYTE*)self + 0x18, sizeof(records));
    if (!parameters || !records ||
        !ptr_readable((BYTE*)parameters - sizeof(parameter_count), sizeof(parameter_count)) ||
        !ptr_readable((BYTE*)records - sizeof(record_count), sizeof(record_count))) {
        return result;
    }
    memcpy(&parameter_count, (BYTE*)parameters - sizeof(parameter_count),
           sizeof(parameter_count));
    memcpy(&record_count, (BYTE*)records - sizeof(record_count), sizeof(record_count));
    if (parameter_count <= 0 || record_count <= 0 ||
        parameter_count > 4096 || record_count > 4096) {
        return result;
    }
    count = parameter_count < record_count ? parameter_count : record_count;
    if (!ptr_readable(parameters, (size_t)count * sizeof(*parameters)) ||
        !ptr_readable(records, (size_t)count * sizeof(*records))) {
        return result;
    }
    for (i = 0; i < count; i++) {
        char parameter_name[128];
        void *record = records[i];
        void *widget = NULL;
        webm_setting_binding_t *binding;
        if (!parameters[i] || !record ||
            !custom_parameter_name(parameters[i], parameter_name, sizeof(parameter_name)) ||
            !ptr_readable((BYTE*)record + 0x24, sizeof(widget))) {
            continue;
        }
        memcpy(&widget, (BYTE*)record + 0x24, sizeof(widget));
        if (!widget) continue;
        if (strcmp(parameter_name, "NCWebMTwitchOverrideChannelInput") == 0) {
            twitch_channel_text_widget = widget;
            debug_line("ConfigEditor Twitch channel text connected widget=%p", widget);
            continue;
        }
        binding = webm_setting_binding_by_name(parameter_name);
        if (binding && binding->type == WEBM_SETTING_SPINBOX)
            webm_sync_spinbox_from_ini(binding, widget);
    }
    return result;
}

static int slider_widget_value(void *slider, float *out_value)
{
    BYTE *metadata;
    BYTE *dispatch_table;
    widget_get_float_t getter;
    float value;
    if (!slider || !out_value ||
        !ptr_readable((BYTE*)slider - 0x18, sizeof(metadata))) {
        return 0;
    }
    memcpy(&metadata, (BYTE*)slider - 0x18, sizeof(metadata));
    if (!metadata || !ptr_readable(metadata + 0x3b4, sizeof(dispatch_table))) return 0;
    memcpy(&dispatch_table, metadata + 0x3b4, sizeof(dispatch_table));
    if (!dispatch_table || !ptr_readable(dispatch_table + 0x80, sizeof(getter))) return 0;
    memcpy(&getter, dispatch_table + 0x80, sizeof(getter));
    if (!getter) return 0;
    value = getter(slider, 0x02fff0ed);
    if (!_finite(value)) return 0;
    *out_value = value;
    return 1;
}

static void queue_twitch_chat_opacity(float value)
{
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    twitch_chat_opacity_pending_value = value;
    twitch_chat_opacity_pending_tick = GetTickCount();
    twitch_chat_opacity_pending = 1;
}

static void queue_twitch_chat_width(float value)
{
    if (value < 0.15f) value = 0.15f;
    if (value > 0.60f) value = 0.60f;
    twitch_chat_width_pending_value = value;
    twitch_chat_width_pending_tick = GetTickCount();
    twitch_chat_width_pending = 1;
}

static void flush_twitch_chat_opacity(DWORD now)
{
    char normalized[32];
    float difference;
    if (!twitch_chat_opacity_pending ||
        (now - twitch_chat_opacity_pending_tick) < 200) {
        return;
    }
    twitch_chat_opacity_pending = 0;
    difference = twitch_chat_opacity_pending_value -
                 twitch_override_chat_background_opacity;
    if (difference < 0.0f) difference = -difference;
    if (difference < 0.0005f) return;
    _snprintf(normalized, sizeof(normalized), "%.3f",
              twitch_chat_opacity_pending_value);
    normalized[sizeof(normalized) - 1] = 0;
    write_twitch_override_value_a("chat_background_opacity", normalized);
}

static void flush_twitch_chat_width(DWORD now)
{
    char normalized[32];
    float difference;
    if (!twitch_chat_width_pending ||
        (now - twitch_chat_width_pending_tick) < 200) {
        return;
    }
    twitch_chat_width_pending = 0;
    difference = twitch_chat_width_pending_value - twitch_override_chat_width;
    if (difference < 0.0f) difference = -difference;
    if (difference < 0.0005f) return;
    _snprintf(normalized, sizeof(normalized), "%.3f",
              twitch_chat_width_pending_value);
    normalized[sizeof(normalized) - 1] = 0;
    write_twitch_override_value_a("chat_width", normalized);
}

static int THISCALL hook_TwitchChannelButton_Dispatch(void *self, DWORD event_id, DWORD argument)
{
    int result = 0;
    DWORD now;
    char channel[WEBM_TWITCH_CHANNEL_LIST_MAX];
    if (twitch_channel_button_original_dispatch) {
        result = twitch_channel_button_original_dispatch(self, event_id, argument);
    }
    if (self == twitch_channel_button_widget &&
        (event_id == 0x0ffff0d5 || event_id == 0x25fff0d5)) {
        now = GetTickCount();
        if (!twitch_channel_apply_last_tick ||
            now - twitch_channel_apply_last_tick >= 250) {
            twitch_channel_apply_last_tick = now;
            if (widget_text_value(twitch_channel_text_widget,
                                  channel, sizeof(channel))) {
                trim_ascii_inplace(channel);
                write_twitch_override_value_a("channel", channel);
                debug_line("ConfigEditor Twitch channel applied channel=\"%s\"", channel);
            } else {
                log_line("ConfigEditor Twitch channel apply failed: text widget unavailable");
            }
        }
    }
    return result;
}

static void patch_twitch_channel_button(void *button)
{
    void **vtable;
    if (!button || !ptr_readable(button, sizeof(vtable))) {
        debug_line("ConfigEditor Twitch channel button unavailable button=%p", button);
        return;
    }
    memcpy(&vtable, button, sizeof(vtable));
    if (!vtable || vtable == twitch_channel_button_vtable ||
        !ptr_readable(vtable, sizeof(twitch_channel_button_vtable))) {
        debug_line("ConfigEditor Twitch channel button vtable unavailable button=%p vtable=%p",
                 button, (void*)vtable);
        return;
    }
    memcpy(twitch_channel_button_vtable, vtable, sizeof(twitch_channel_button_vtable));
    twitch_channel_button_original_dispatch =
        (widget_dispatch_event_t)twitch_channel_button_vtable[0x28 / sizeof(void*)];
    if (!twitch_channel_button_original_dispatch) {
        debug_line("ConfigEditor Twitch channel button dispatch unavailable button=%p", button);
        return;
    }
    twitch_channel_button_vtable[0x28 / sizeof(void*)] =
        (void*)hook_TwitchChannelButton_Dispatch;
    *(void***)button = twitch_channel_button_vtable;
    twitch_channel_button_widget = button;
    debug_line("ConfigEditor Twitch channel button connected widget=%p dispatch=%p",
               button, (void*)twitch_channel_button_original_dispatch);
}

static int THISCALL hook_Customizer_CreateButton(void *self, void *parameter,
                                                  void *record, void *parent, float y)
{
    int result;
    char parameter_name[128];
    void *button = NULL;
    result = tramp_Customizer_CreateButton ?
        tramp_Customizer_CreateButton(self, parameter, record, parent, y) : 0;
    if (custom_parameter_name(parameter, parameter_name, sizeof(parameter_name)) &&
        strcmp(parameter_name, "NCWebMTwitchOverrideChannelApply") == 0 &&
        record && ptr_readable((BYTE*)record + 0x20, sizeof(button))) {
        memcpy(&button, (BYTE*)record + 0x20, sizeof(button));
        patch_twitch_channel_button(button);
    }
    return result;
}

static int THISCALL hook_Customizer_CreateSlider(void *self, void *parameter,
                                                  void *record, void *parent, float y,
                                                  int preset_index, int has_labels)
{
    int result;
    char parameter_name[128];
    void *slider = NULL;
    webm_setting_binding_t *binding;
    result = tramp_Customizer_CreateSlider ?
        tramp_Customizer_CreateSlider(self, parameter, record, parent, y,
                                      preset_index, has_labels) : 0;
    if (!custom_parameter_name(parameter, parameter_name,
                               sizeof(parameter_name)) || !record) {
        return result;
    }
    binding = webm_setting_binding_by_name(parameter_name);
    if (!binding || binding->type != WEBM_SETTING_SLIDER) return result;
    if (preset_index < 0) {
        if (ptr_readable((BYTE*)record + 0x04, sizeof(slider))) {
            memcpy(&slider, (BYTE*)record + 0x04, sizeof(slider));
        }
    } else if (preset_index < 1024 &&
               ptr_readable((BYTE*)record + 0x08 + (size_t)preset_index * sizeof(void*),
                            sizeof(slider))) {
        memcpy(&slider, (BYTE*)record + 0x08 + (size_t)preset_index * sizeof(void*),
               sizeof(slider));
    }
    if (slider) {
        binding->slider_widget = slider;
        if (strcmp(parameter_name, "NCWebMTwitchOverrideChatOverlayOpacity") == 0) {
            twitch_chat_opacity_slider_widget = slider;
            debug_line("ConfigEditor Twitch chat opacity slider connected widget=%p",
                       slider);
        } else {
            twitch_chat_width_slider_widget = slider;
            debug_line("ConfigEditor Twitch chat width slider connected widget=%p",
                       slider);
        }
        if (preset_index < 0) {
            webm_sync_slider_from_ini(binding, slider);
            if (strcmp(binding->key, "chat_background_opacity") == 0)
                twitch_chat_opacity_pending = 0;
            else
                twitch_chat_width_pending = 0;
        }
    } else {
        debug_line("ConfigEditor Twitch chat slider unavailable parameter=\"%s\" record=%p preset=%d",
                 parameter_name, record, preset_index);
    }
    return result;
}

static void THISCALL hook_ConfigEditor_ParamChange(void *self, const char *parameter,
                                                   const char *value, DWORD arg3, DWORD arg4)
{
    char parameter_text[128];
    char value_text[MAX_PATH * 4];
    int has_parameter;
    int has_value;
    has_parameter = copy_engine_string_a(parameter, parameter_text, sizeof(parameter_text));
    has_value = copy_engine_string_a(value, value_text, sizeof(value_text));
    if (tramp_ConfigEditor_ParamChange) {
        tramp_ConfigEditor_ParamChange(self, parameter, value, arg3, arg4);
    }
    if (!has_parameter || webm_setting_sync_depth) return;

    if (strcmp(parameter_text, "NCWebMOverrideEnabled") == 0) {
        write_twitch_override_value_a("enabled",
                                      has_value && override_value_is_enabled_a(value_text) ? "1" : "0");
    } else if (strcmp(parameter_text, "NCWebMOverrideTarget") == 0) {
        if (has_value) write_twitch_override_value_a("target", value_text);
    } else if (strcmp(parameter_text,
                      "NCWebMTwitchOverrideChannelOfflineFallback") == 0) {
        if (has_value && (_stricmp(value_text, "fallback") == 0 ||
                          _stricmp(value_text, "random") == 0)) {
            write_twitch_override_value_a(
                "channel_offline_fallback",
                _stricmp(value_text, "random") == 0 ? "random" : "fallback");
        }
    } else if (strcmp(parameter_text, "NCWebMTwitchOverrideQuality") == 0) {
        if (has_value && value_text[0]) {
            write_twitch_override_value_a("quality", value_text);
        }
    } else if (strcmp(parameter_text, "NCWebMTwitchOverrideChatEnabled") == 0) {
        write_twitch_override_value_a(
            "chat_enabled",
            has_value && override_value_is_enabled_a(value_text) ? "true" : "false");
    } else if (strcmp(parameter_text, "NCWebMTwitchOverrideChatPosition") == 0) {
        if (has_value && (_stricmp(value_text, "left") == 0 ||
                          _stricmp(value_text, "right") == 0)) {
            write_twitch_override_value_a(
                "chat_position", _stricmp(value_text, "left") == 0 ? "left" : "right");
        }
    } else if (strcmp(parameter_text, "NCWebMTwitchOverrideChatOverlay") == 0) {
        write_twitch_override_value_a(
            "chat_overlay",
            has_value && override_value_is_enabled_a(value_text) ? "true" : "false");
    } else if (strcmp(parameter_text,
                      "NCWebMTwitchOverrideChatAnimatedEmotesEnabled") == 0) {
        write_twitch_override_value_a(
            "chat_animated_emotes",
            has_value && override_value_is_enabled_a(value_text) ? "true" : "false");
    } else if (strcmp(parameter_text, "NCWebMTwitchOverrideChatOverlayOpacity") == 0) {
        float slider_value;
        if (slider_widget_value(twitch_chat_opacity_slider_widget, &slider_value)) {
            queue_twitch_chat_opacity(slider_value);
        } else if (has_value) {
            char *end = NULL;
            double parsed = strtod(value_text, &end);
            if (end != value_text && parsed == parsed) {
                queue_twitch_chat_opacity((float)parsed);
            }
        }
    } else if (strcmp(parameter_text, "NCWebMTwitchOverrideChatContainerWidth") == 0) {
        float slider_value;
        if (slider_widget_value(twitch_chat_width_slider_widget, &slider_value)) {
            queue_twitch_chat_width(slider_value);
        } else if (has_value) {
            char *end = NULL;
            double parsed = strtod(value_text, &end);
            if (end != value_text && parsed == parsed) {
                queue_twitch_chat_width((float)parsed);
            }
        }
    }
}

static void THISCALL hook_vd_0(void *self)
{
    ((vd_m0_t)video_decoder_orig_vt[0])(self);
}

static void THISCALL hook_vd_1(void *self, void *a, void *b, void *c)
{
    ((vd_m1_t)video_decoder_orig_vt[1])(self, a, b, c);
}

static int THISCALL hook_vd_2(void *self, void *name)
{
    int ret;
    ret = ((vd_m2_t)video_decoder_orig_vt[2])(self, name);
    return ret;
}

static double THISCALL hook_vd_3(void *self)
{
    double ret = ((vd_md0_t)video_decoder_orig_vt[3])(self);
    return ret;
}

static int THISCALL hook_vd_4(void *self)
{
    int ret = ((vd_mi0_t)video_decoder_orig_vt[4])(self);
    return ret;
}

static int THISCALL hook_vd_5(void *self)
{
    int ret = ((vd_mi0_t)video_decoder_orig_vt[5])(self);
    return ret;
}

static int THISCALL hook_vd_6(void *self, double pos)
{
    int ret;
    ret = ((vd_md1_t)video_decoder_orig_vt[6])(self, pos);
    return ret;
}

static double THISCALL hook_vd_7(void *self)
{
    double ret = ((vd_md0_t)video_decoder_orig_vt[7])(self);
    return ret;
}

static int THISCALL hook_vd_8(void *self, int paused)
{
    int ret;
    ret = ((vd_mi1_t)video_decoder_orig_vt[8])(self, paused);
    return ret;
}

static int THISCALL hook_vd_9(void *self)
{
    int ret = ((vd_mi0_t)video_decoder_orig_vt[9])(self);
    return ret;
}

static int THISCALL hook_vd_10(void *self)
{
    int ret = ((vd_mi0_t)video_decoder_orig_vt[10])(self);
    return ret;
}

static int THISCALL hook_vd_11(void *self)
{
    int ret = ((vd_mi0_t)video_decoder_orig_vt[11])(self);
    return ret;
}

static int THISCALL hook_vd_12(void *self)
{
    int ret;
    ret = ((vd_mi0_t)video_decoder_orig_vt[12])(self);
    return ret;
}

static int THISCALL hook_vd_13(void *self)
{
    int ret;
    ret = ((vd_mi0_t)video_decoder_orig_vt[13])(self);
    return ret;
}

static void THISCALL hook_vd_14(void *self)
{
    ((vd_m0_t)video_decoder_orig_vt[14])(self);
}

static void hook_video_decoder_object(void *obj)
{
    DWORD *vt;
    if (!obj) return;
    vt = *(DWORD**)obj;
    if (!vt) return;
    if (!video_decoder_vt_ready) {
        int i;
        for (i = 0; i < 15; i++) {
            video_decoder_orig_vt[i] = vt[i];
            video_decoder_hook_vt[i] = vt[i];
        }
        video_decoder_hook_vt[0] = (DWORD)hook_vd_0;
        video_decoder_hook_vt[1] = (DWORD)hook_vd_1;
        video_decoder_hook_vt[2] = (DWORD)hook_vd_2;
        video_decoder_hook_vt[3] = (DWORD)hook_vd_3;
        video_decoder_hook_vt[4] = (DWORD)hook_vd_4;
        video_decoder_hook_vt[5] = (DWORD)hook_vd_5;
        video_decoder_hook_vt[6] = (DWORD)hook_vd_6;
        video_decoder_hook_vt[7] = (DWORD)hook_vd_7;
        video_decoder_hook_vt[8] = (DWORD)hook_vd_8;
        video_decoder_hook_vt[9] = (DWORD)hook_vd_9;
        video_decoder_hook_vt[10] = (DWORD)hook_vd_10;
        video_decoder_hook_vt[11] = (DWORD)hook_vd_11;
        video_decoder_hook_vt[12] = (DWORD)hook_vd_12;
        video_decoder_hook_vt[13] = (DWORD)hook_vd_13;
        video_decoder_hook_vt[14] = (DWORD)hook_vd_14;
        video_decoder_vt_ready = 1;
    }
    *(DWORD**)obj = video_decoder_hook_vt;
}

static HRESULT WINAPI hook_graph_RenderFile(void *self, LPCWSTR file, LPCWSTR playlist)
{
    HRESULT hr;
    char file_mb[MAX_PATH * 4];
    char playlist_mb[MAX_PATH * 4];
    wide_to_mb(file, file_mb, sizeof(file_mb));
    wide_to_mb(playlist, playlist_mb, sizeof(playlist_mb));
    hr = ((graph_renderfile_t)graph_builder_orig_vt[13])(self, file, playlist);
    if (FAILED(hr)) {
        log_line("IGraphBuilder::RenderFile failed file=\"%s\" playlist=\"%s\" hr=0x%08lx",
                 file_mb, playlist_mb, (DWORD)hr);
    }
    return hr;
}

static HRESULT create_lav_source_filter_for_file(LPCWSTR file, void **filter_out)
{
    wchar_t real_path[MAX_PATH * 4];
    char lav_dir[MAX_PATH * 2];
    char splitter_path[MAX_PATH * 2];
    char file_mb[MAX_PATH * 4];
    HMODULE mod;
    DllGetClassObject_t get_class;
    void *factory = NULL;
    void *filter = NULL;
    void *source = NULL;
    DWORD **factory_vt;
    DWORD **filter_vt;
    DWORD **source_vt;
    HRESULT hr;

    if (filter_out) *filter_out = NULL;
    if (!filter_out || !file) {
        return E_INVALIDARG;
    }
    if (!webm_alias_to_real_w(file, real_path, sizeof(real_path) / sizeof(real_path[0]))) {
        if (!ends_with_w_i(file, L".webm")) return E_INVALIDARG;
        lstrcpynW(real_path, file, (int)(sizeof(real_path) / sizeof(real_path[0])));
    }

    wide_to_mb(real_path, file_mb, sizeof(file_mb));
    webm_component_dir_a(lav_dir, sizeof(lav_dir),
                         "NC-TK17-WebM-lav");
    if (!lav_dir[0]) {
        log_line("LAV source create failed: cannot locate Extensions\\WebM");
        return E_FAIL;
    }
    path_join(splitter_path, sizeof(splitter_path), lav_dir, "LAVSplitter.ax");

    SetDllDirectoryA(lav_dir);
    mod = LoadLibraryA(splitter_path);
    SetDllDirectoryA(NULL);
    if (!mod) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_line("LAV source LoadLibrary failed file=\"%s\" hr=0x%08lx", splitter_path, (DWORD)hr);
        return hr;
    }

    get_class = (DllGetClassObject_t)GetProcAddress(mod, "DllGetClassObject");
    if (!get_class) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_line("LAV source DllGetClassObject missing hr=0x%08lx", (DWORD)hr);
        return hr;
    }

    hr = get_class(&CLSID_LAVSplitterSource_, &IID_IClassFactory_, &factory);
    if (FAILED(hr) || !factory) {
        log_line("LAV source DllGetClassObject failed hr=0x%08lx", (DWORD)hr);
        return hr;
    }

    factory_vt = *(DWORD***)factory;
    hr = ((class_create_t)factory_vt[3])(factory, NULL, &IID_IBaseFilter_, &filter);
    ((com_ref_t)factory_vt[2])(factory);
    if (FAILED(hr) || !filter) {
        log_line("LAV source CreateInstance failed hr=0x%08lx", (DWORD)hr);
        return hr;
    }

    filter_vt = *(DWORD***)filter;
    hr = ((com_qi_t)filter_vt[0])(filter, &IID_IFileSourceFilter_, &source);
    if (FAILED(hr) || !source) {
        log_line("LAV source QueryInterface IFileSourceFilter failed hr=0x%08lx", (DWORD)hr);
        ((com_ref_t)filter_vt[2])(filter);
        return hr;
    }

    source_vt = *(DWORD***)source;
    hr = ((filesource_load_t)source_vt[3])(source, real_path, NULL);
    ((com_ref_t)source_vt[2])(source);
    if (FAILED(hr)) {
        log_line("LAV source Load failed file=\"%s\" hr=0x%08lx", file_mb, (DWORD)hr);
        ((com_ref_t)filter_vt[2])(filter);
        return hr;
    }

    *filter_out = filter;
    return S_OK;
}

static HRESULT create_lav_filter_object(const char *ax_name, REFGUID clsid, void **filter_out)
{
    char lav_dir[MAX_PATH * 2];
    char ax_path[MAX_PATH * 2];
    HMODULE mod;
    DllGetClassObject_t get_class;
    void *factory = NULL;
    void *filter = NULL;
    DWORD **factory_vt;
    HRESULT hr;

    if (filter_out) *filter_out = NULL;
    if (!filter_out || !ax_name || !clsid) return E_INVALIDARG;

    webm_component_dir_a(lav_dir, sizeof(lav_dir),
                         "NC-TK17-WebM-lav");
    if (!lav_dir[0]) {
        log_line("LAV filter create failed: cannot locate Extensions\\WebM");
        return E_FAIL;
    }
    path_join(ax_path, sizeof(ax_path), lav_dir, ax_name);

    SetDllDirectoryA(lav_dir);
    mod = LoadLibraryA(ax_path);
    SetDllDirectoryA(NULL);
    if (!mod) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_line("LAV filter LoadLibrary failed file=\"%s\" hr=0x%08lx", ax_path, (DWORD)hr);
        return hr;
    }

    get_class = (DllGetClassObject_t)GetProcAddress(mod, "DllGetClassObject");
    if (!get_class) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_line("LAV filter DllGetClassObject missing file=\"%s\" hr=0x%08lx", ax_path, (DWORD)hr);
        return hr;
    }

    hr = get_class(clsid, &IID_IClassFactory_, &factory);
    if (FAILED(hr) || !factory) {
        log_line("LAV filter DllGetClassObject failed file=\"%s\" hr=0x%08lx", ax_path, (DWORD)hr);
        return hr;
    }

    factory_vt = *(DWORD***)factory;
    hr = ((class_create_t)factory_vt[3])(factory, NULL, &IID_IBaseFilter_, &filter);
    ((com_ref_t)factory_vt[2])(factory);
    if (FAILED(hr) || !filter) {
        log_line("LAV filter CreateInstance failed file=\"%s\" hr=0x%08lx", ax_path, (DWORD)hr);
        return hr;
    }

    *filter_out = filter;
    return S_OK;
}

static void free_media_type_local(AM_MEDIA_TYPE *mt)
{
    if (!mt) return;
    if (mt->cbFormat && mt->pbFormat) CoTaskMemFree(mt->pbFormat);
    if (mt->pUnk) IUnknown_Release(mt->pUnk);
    ZeroMemory(mt, sizeof(*mt));
}

static IPin *find_filter_pin(IBaseFilter *filter, PIN_DIRECTION direction, int require_unconnected)
{
    IEnumPins *pins = NULL;
    IPin *pin = NULL;
    if (!filter) return NULL;
    if (FAILED(IBaseFilter_EnumPins(filter, &pins)) || !pins) return NULL;
    while (IEnumPins_Next(pins, 1, &pin, NULL) == S_OK && pin) {
        PIN_DIRECTION got_dir;
        if (SUCCEEDED(IPin_QueryDirection(pin, &got_dir)) && got_dir == direction) {
            if (require_unconnected) {
                IPin *connected = NULL;
                HRESULT hr = IPin_ConnectedTo(pin, &connected);
                if (connected) IPin_Release(connected);
                if (hr == VFW_E_NOT_CONNECTED) {
                    IEnumPins_Release(pins);
                    return pin;
                }
            } else {
                IEnumPins_Release(pins);
                return pin;
            }
        }
        IPin_Release(pin);
        pin = NULL;
    }
    IEnumPins_Release(pins);
    return NULL;
}

static HRESULT connect_filters(IGraphBuilder *graph, IBaseFilter *out_filter, IBaseFilter *in_filter)
{
    IPin *out_pin;
    IPin *in_pin;
    HRESULT hr;
    if (!graph || !out_filter || !in_filter) return E_INVALIDARG;
    out_pin = find_filter_pin(out_filter, PINDIR_OUTPUT, 1);
    in_pin = find_filter_pin(in_filter, PINDIR_INPUT, 1);
    if (!out_pin || !in_pin) {
        if (out_pin) IPin_Release(out_pin);
        if (in_pin) IPin_Release(in_pin);
        return E_FAIL;
    }
    hr = IGraphBuilder_Connect(graph, out_pin, in_pin);
    IPin_Release(out_pin);
    IPin_Release(in_pin);
    return hr;
}

static HRESULT connect_filters_any_pin(IGraphBuilder *graph, IBaseFilter *out_filter, IBaseFilter *in_filter)
{
    IEnumPins *out_enum = NULL;
    IPin *out_pin = NULL;
    HRESULT last_hr = E_FAIL;
    if (!graph || !out_filter || !in_filter) return E_INVALIDARG;
    if (FAILED(IBaseFilter_EnumPins(out_filter, &out_enum)) || !out_enum) return E_FAIL;
    while (IEnumPins_Next(out_enum, 1, &out_pin, NULL) == S_OK && out_pin) {
        PIN_DIRECTION out_dir;
        if (SUCCEEDED(IPin_QueryDirection(out_pin, &out_dir)) && out_dir == PINDIR_OUTPUT) {
            IEnumPins *in_enum = NULL;
            IPin *in_pin = NULL;
            if (SUCCEEDED(IBaseFilter_EnumPins(in_filter, &in_enum)) && in_enum) {
                while (IEnumPins_Next(in_enum, 1, &in_pin, NULL) == S_OK && in_pin) {
                    PIN_DIRECTION in_dir;
                    if (SUCCEEDED(IPin_QueryDirection(in_pin, &in_dir)) && in_dir == PINDIR_INPUT) {
                        last_hr = IGraphBuilder_Connect(graph, out_pin, in_pin);
                        if (SUCCEEDED(last_hr)) {
                            IPin_Release(in_pin);
                            IEnumPins_Release(in_enum);
                            IPin_Release(out_pin);
                            IEnumPins_Release(out_enum);
                            return S_OK;
                        }
                    }
                    IPin_Release(in_pin);
                    in_pin = NULL;
                }
                IEnumPins_Release(in_enum);
            }
        }
        IPin_Release(out_pin);
        out_pin = NULL;
    }
    IEnumPins_Release(out_enum);
    return last_hr;
}

static void video_decoder_release(video_decoder_t *dec)
{
    int live_index;
    int audio_index;
    if (!dec) return;
    video_decoder_stop_async(dec);
    if (dec->async_d3d8_chat) {
        webm_twitch_chat_release(dec->async_d3d8_chat);
        dec->async_d3d8_chat = NULL;
    }
    free(dec->async_d3d8_cache[0]);
    free(dec->async_d3d8_cache[1]);
    for (live_index = 0; live_index < WEBM_LIVE_QUEUE_CAPACITY; live_index++) {
        free(dec->live_queue[live_index].data);
    }
    for (audio_index = 0; audio_index < WEBM_TWITCH_AUDIO_QUEUE_CAPACITY; audio_index++) {
        free(dec->audio_queue[audio_index].data);
    }
    free(dec->audio_convert_buffer);
    if (dec->use_ffmpeg) {
        if (dec->sws && ffmpeg_api.sws_freeContext) ffmpeg_api.sws_freeContext(dec->sws);
        if (dec->audio_frame && ffmpeg_api.av_frame_free) ffmpeg_api.av_frame_free(&dec->audio_frame);
        if (dec->audio_codec && ffmpeg_api.avcodec_free_context) ffmpeg_api.avcodec_free_context(&dec->audio_codec);
        if (dec->src_frame && ffmpeg_api.av_frame_free) ffmpeg_api.av_frame_free(&dec->src_frame);
        if (dec->packet && ffmpeg_api.av_packet_free) ffmpeg_api.av_packet_free(&dec->packet);
        if (dec->codec && ffmpeg_api.avcodec_free_context) ffmpeg_api.avcodec_free_context(&dec->codec);
        if (dec->fmt && ffmpeg_api.avformat_close_input) ffmpeg_api.avformat_close_input(&dec->fmt);
        if (dec->frame && ffmpeg_api.av_free) ffmpeg_api.av_free(dec->frame);
        free(dec->async_frame);
        free(dec->live_present_frame);
        free(dec);
        return;
    }
    if (dec->control) {
        IMediaControl_Stop(dec->control);
        IMediaControl_Release(dec->control);
    }
    if (dec->grabber) ISampleGrabber_Release(dec->grabber);
    if (dec->null_filter) IBaseFilter_Release(dec->null_filter);
    if (dec->sample_filter) IBaseFilter_Release(dec->sample_filter);
    if (dec->video_filter) IBaseFilter_Release(dec->video_filter);
    if (dec->source_filter) IBaseFilter_Release(dec->source_filter);
    if (dec->graph) IGraphBuilder_Release(dec->graph);
    free(dec->frame);
    free(dec->async_frame);
    free(dec->live_present_frame);
    free(dec);
}

static DWORD WINAPI video_decoder_retire_thread(void *parameter)
{
    video_decoder_retire_task_t *task = (video_decoder_retire_task_t*)parameter;
    if (!task) return 0;
    video_decoder_release(task->decoder);
    task->decoder = NULL;
    InterlockedExchange(&task->done, 1);
    return 0;
}

static void video_decoder_reap_retired(void)
{
    video_decoder_retire_task_t **link;
    video_decoder_retire_task_t *task;
    if (!video_decoder_retire_lock_ready) return;
    EnterCriticalSection(&video_decoder_retire_lock);
    link = &video_decoder_retire_tasks;
    while ((task = *link) != NULL) {
        if (InterlockedCompareExchange(&task->done, 0, 0) &&
            WaitForSingleObject(task->thread, 0) == WAIT_OBJECT_0) {
            *link = task->next;
            CloseHandle(task->thread);
            free(task);
            continue;
        }
        link = &task->next;
    }
    LeaveCriticalSection(&video_decoder_retire_lock);
}

static void video_decoder_retire_async(video_decoder_t *decoder)
{
    video_decoder_retire_task_t *task;
    if (!decoder) return;
    video_decoder_reap_retired();
    task = (video_decoder_retire_task_t*)calloc(1, sizeof(*task));
    if (!task || !video_decoder_retire_lock_ready) {
        free(task);
        video_decoder_release(decoder);
        return;
    }
    task->decoder = decoder;
    task->thread = CreateThread(NULL, 0, video_decoder_retire_thread, task, 0, NULL);
    if (!task->thread) {
        free(task);
        video_decoder_release(decoder);
        return;
    }
    EnterCriticalSection(&video_decoder_retire_lock);
    task->next = video_decoder_retire_tasks;
    video_decoder_retire_tasks = task;
    LeaveCriticalSection(&video_decoder_retire_lock);
}

static void video_decoder_drain_retired(void)
{
    video_decoder_retire_task_t *task;
    if (!video_decoder_retire_lock_ready) return;
    EnterCriticalSection(&video_decoder_retire_lock);
    task = video_decoder_retire_tasks;
    video_decoder_retire_tasks = NULL;
    LeaveCriticalSection(&video_decoder_retire_lock);
    while (task) {
        video_decoder_retire_task_t *next = task->next;
        WaitForSingleObject(task->thread, INFINITE);
        CloseHandle(task->thread);
        free(task);
        task = next;
    }
}

static long audio_volume_to_directshow(int volume)
{
    volume = clamp_int(volume, -10000, 10000);
    if (volume >= 0) return 0;
    return (long)volume;
}

static void resolve_engine_audio_symbols(void)
{
    HMODULE sys, app;
    if (engine_audio_symbols_attempted) return;
    engine_audio_symbols_attempted = 1;
    sys = GetModuleHandleA("ThriXXX010278-SYS.dll");
    app = GetModuleHandleA("ThriXXX010278-APP.dll");
    if (app) {
        engine_FindObjC = (app_find_objc_t)GetProcAddress(app, "?FindObjC@AppMain@@YAPAVScriptObject@Bionic@@PBD@Z");
    }
    if (sys) {
        engine_GetModelViewRotationPivot = (model_pivot_t)GetProcAddress(sys, "?GetModelViewRotationPivot@Bionic@@YAXPAVScriptObject@1@AAVVector3f@1@@Z");
        engine_SoundReceiver_GetAllParameters =
            (sound_receiver_get_all_parameters_t)GetProcAddress(sys, "?GetAllParameters@SoundReceiver@Bionic@@UBEXAAVVector3f@2@000AAM11@Z");
        engine_SoundReceiver_ptr = (void**)GetProcAddress(sys, "?s_SoundReceiver@SoundReceiver_I@Bionic@@2PAVSoundReceiver@2@A");
        engine_SoundDevice_ptr = (void**)GetProcAddress(sys, "?s_SoundDevice@SoundDevice_I@Bionic@@2PAVSoundDevice@2@A");
        real_SoundDevice_Create = real_SoundDevice_Create ? real_SoundDevice_Create :
            (sound_device_create_t)GetProcAddress(sys, "?Create@SoundDevice@Bionic@@SAPAV12@W4SDType@12@W4SpeakerType@12@W4Quality@12@PAX@Z");
        engine_SoundSource_Create =
            (sound_source_create_t)GetProcAddress(sys, "?Create@SoundSource@Bionic@@SAPAV12@PAVSoundDevice@2@PBDIW4CacheMode@AudioSrcFile@2@@Z");
        engine_SoundSource_Play =
            (sound_source_play_t)GetProcAddress(sys, "?Play@SoundSource@Bionic@@UAEH_NN@Z");
        engine_SoundSource_Stop =
            (sound_source_stop_t)GetProcAddress(sys, "?Stop@SoundSource@Bionic@@UAEXXZ");
        engine_SoundSource_IsPlaying =
            (sound_source_is_playing_t)GetProcAddress(sys, "?IsPlaying@SoundSource@Bionic@@UAEHXZ");
        engine_SoundSource_SetPlayPosition =
            (sound_source_set_play_position_t)GetProcAddress(sys, "?SetPlayPosition@SoundSource@Bionic@@UAEHI@Z");
        engine_SoundSource_SetPosition =
            (sound_source_set_position_t)GetProcAddress(sys, "?SetPosition@SoundSource@Bionic@@UAEXABVVector3f@2@_N@Z");
        engine_SoundSource_SetDistances =
            (sound_source_set_distances_t)GetProcAddress(sys, "?SetDistances@SoundSource@Bionic@@UAEXMM_N@Z");
        engine_SoundSource_SetVolume =
            (sound_source_set_volume_t)GetProcAddress(sys, "?SetVolume@SoundSource@Bionic@@UAEXM_N@Z");
        engine_SoundSource_GetVolume =
            (sound_source_get_volume_t)GetProcAddress(sys, "?GetVolume@SoundSource@Bionic@@UBEMXZ");
        real_SoundSource_Destroy =
            (sound_source_destroy_t)GetProcAddress(sys, "??1SoundSource_D@Bionic@@QAE@XZ");
    }
    debug_line("Audio3D symbols FindObjC=%p Pivot=%p ReceiverGet=%p ReceiverSlot=%p SoundDeviceSlot=%p SoundDeviceCreate=%p SourceCreate=%p SourceGetVolume=%p SourceDestroy=%p",
               (void*)engine_FindObjC, (void*)engine_GetModelViewRotationPivot,
               (void*)engine_SoundReceiver_GetAllParameters, (void*)engine_SoundReceiver_ptr,
               (void*)engine_SoundDevice_ptr, (void*)real_SoundDevice_Create,
               (void*)engine_SoundSource_Create, (void*)engine_SoundSource_GetVolume,
               (void*)real_SoundSource_Destroy);
}

static HRESULT basic_audio_put_volume(void *basic_audio, long volume)
{
    void **vt;
    typedef HRESULT (WINAPI *put_volume_t)(void *, long);
    if (!basic_audio) return E_POINTER;
    vt = *(void***)basic_audio;
    if (!vt) return E_POINTER;
    return ((put_volume_t)vt[7])(basic_audio, volume);
}

static int apptracker_inverse_listener_pose(float listener[3], float front[3], float top[3])
{
    float *m = engine_captured_camera_inverse;
    if (!engine_captured_camera_inverse_valid || !listener || !front || !top) return 0;
    listener[0] = m[12];
    listener[1] = m[13];
    listener[2] = m[14];
    front[0] = -m[8];
    front[1] = -m[9];
    front[2] = -m[10];
    top[0] = m[4];
    top[1] = m[5];
    top[2] = m[6];
    return 1;
}

static int audio_graph_get_listener_position(audio_graph_t *ag, DWORD now, float listener[3])
{
    void *receiver;
    float velocity[3] = {0, 0, 0};
    float front[3] = {0, 0, 0};
    float top[3] = {0, 0, 0};
    float distance_factor = 0, rolloff_factor = 0, doppler_factor = 0;

    if (engine_SoundReceiver_ptr && engine_SoundReceiver_GetAllParameters) {
        receiver = *engine_SoundReceiver_ptr;
        if (receiver) {
            engine_SoundReceiver_GetAllParameters(receiver, listener, velocity, front, top,
                                                  &distance_factor, &rolloff_factor, &doppler_factor);
            return 1;
        }
    }

    if (engine_captured_camera_inverse_valid) {
        float local_front[3] = {0, 0, -1};
        float local_top[3] = {0, 1, 0};
        if (apptracker_inverse_listener_pose(listener, local_front, local_top)) {
            if (!ag->logged_listener_found) {
                ag->logged_listener_found = 1;
                debug_line("Audio3D listener AppTracker inverse pos=(%.2f %.2f %.2f) front=(%.2f %.2f %.2f) top=(%.2f %.2f %.2f) source=\"%s\"",
                           listener[0], listener[1], listener[2],
                           local_front[0], local_front[1], local_front[2],
                           local_top[0], local_top[1], local_top[2],
                           ag->source_name);
            }
            return 1;
        }
    }

    if (engine_captured_camera_transform && engine_GetModelViewRotationPivot) {
        engine_GetModelViewRotationPivot(engine_captured_camera_transform, listener);
        if (!ag->logged_listener_found) {
            ag->logged_listener_found = 1;
            debug_line("Audio3D listener camera transform=%p pos=(%.2f %.2f %.2f) source=\"%s\"",
                       engine_captured_camera_transform, listener[0], listener[1], listener[2],
                       ag->source_name);
        }
        return 1;
    }

    if (!engine_FindObjC || !engine_GetModelViewRotationPivot) return 0;

    if (!ag->logged_listener_missing && (!ag->last_listener_find_tick || (now - ag->last_listener_find_tick) > 1000)) {
        ag->last_listener_find_tick = now;
        ag->logged_listener_missing = 1;
        log_line("Audio3D unavailable listener receiver=null source=\"%s\"",
                 ag->source_name);
    }

    return 0;
}

static int audio_graph_get_listener_pose(audio_graph_t *ag, DWORD now, float listener[3], float front[3], float top[3])
{
    void *receiver;
    float velocity[3] = {0, 0, 0};
    float distance_factor = 0, rolloff_factor = 0, doppler_factor = 0;

    if (engine_SoundReceiver_ptr && engine_SoundReceiver_GetAllParameters) {
        receiver = *engine_SoundReceiver_ptr;
        if (receiver) {
            engine_SoundReceiver_GetAllParameters(receiver, listener, velocity, front, top,
                                                  &distance_factor, &rolloff_factor, &doppler_factor);
            return 1;
        }
    }

    if (engine_captured_camera_inverse_valid) {
        if (apptracker_inverse_listener_pose(listener, front, top)) {
            if (!ag->logged_listener_found) {
                ag->logged_listener_found = 1;
                debug_line("Audio3D listener pose AppTracker inverse pos=(%.2f %.2f %.2f) front=(%.2f %.2f %.2f) top=(%.2f %.2f %.2f) source=\"%s\"",
                           listener[0], listener[1], listener[2],
                           front[0], front[1], front[2],
                           top[0], top[1], top[2],
                           ag->source_name);
            }
            return 1;
        }
    }

    return audio_graph_get_listener_position(ag, now, listener);
}

static int parse_fixed_audio_position_a(const char *source_name, float source[3])
{
    const char *p;
    char *end;
    if (!source_name || !source || _strnicmp(source_name, "pos:", 4) != 0) return 0;
    p = source_name + 4;
    source[0] = (float)strtod(p, &end);
    if (end == p || *end != ',') return 0;
    p = end + 1;
    source[1] = (float)strtod(p, &end);
    if (end == p || *end != ',') return 0;
    p = end + 1;
    source[2] = (float)strtod(p, &end);
    return end != p;
}

static void audio_graph_update_3d(audio_graph_t *ag, DWORD now)
{
    float listener[3] = {0, 0, 0};
    float source[3] = {0, 0, 0};
    float front[3] = {0, 0, -1};
    float top[3] = {0, 1, 0};
    float dx, dy, dz, d2, min2, max2, t;
    long base_volume, target_volume;
    if (!ag || !ag->audio_3d || !ag->source_name[0] || ag->source_position_disabled) return;
    if (!ag->openal && !ag->basic_audio) return;
    if (ag->last_3d_tick && (now - ag->last_3d_tick) < 100) return;
    ag->last_3d_tick = now;

    resolve_engine_audio_symbols();

    if (parse_fixed_audio_position_a(ag->source_name, source)) {
        if (audio_graph_get_listener_pose(ag, now, listener, front, top)) {
            float rel[3];
            float right[3];
            float len;
            dx = source[0] - listener[0];
            dy = source[1] - listener[1];
            dz = source[2] - listener[2];
            right[0] = front[1] * top[2] - front[2] * top[1];
            right[1] = front[2] * top[0] - front[0] * top[2];
            right[2] = front[0] * top[1] - front[1] * top[0];
            len = right[0] * right[0] + right[1] * right[1] + right[2] * right[2];
            if (len <= 0.000001f) {
                right[0] = 1.0f;
                right[1] = 0.0f;
                right[2] = 0.0f;
            }
            rel[0] = dx * right[0] + dy * right[1] + dz * right[2];
            rel[1] = dx * top[0] + dy * top[1] + dz * top[2];
            rel[2] = -(dx * front[0] + dy * front[1] + dz * front[2]);
            source[0] = rel[0];
            source[1] = rel[1];
            source[2] = rel[2];
        }
        if (ag->openal) {
            if (vm_alSourcefv) vm_alSourcefv(ag->al_source, 0x1004, source);
            if (ag->current_volume != 0) {
                ag->current_volume = 0;
                debug_line("AudioOpenAL fixed source position pos=(%.2f %.2f %.2f) listener=(%.2f %.2f %.2f) path=\"%s\"",
                           source[0], source[1], source[2],
                           listener[0], listener[1], listener[2], ag->path);
            }
            return;
        }
    }

    if (!engine_FindObjC || !engine_GetModelViewRotationPivot) {
        if (!ag->logged_missing_symbols) {
            ag->logged_missing_symbols = 1;
            log_line("Audio3D unavailable symbols FindObjC=%p Pivot=%p ReceiverGet=%p ReceiverSlot=%p node=\"%s\"",
                     (void*)engine_FindObjC, (void*)engine_GetModelViewRotationPivot,
                     (void*)engine_SoundReceiver_GetAllParameters, (void*)engine_SoundReceiver_ptr,
                     ag->source_name);
        }
        return;
    }

    if (!ag->source_obj && (!ag->last_find_tick || (now - ag->last_find_tick) > 1000)) {
        ag->last_find_tick = now;
        ag->source_obj = engine_FindObjC(ag->source_name);
        if (ag->source_obj) {
            if (!ag->logged_source_found) {
                ag->logged_source_found = 1;
                debug_line("Audio3D source found node=\"%s\" obj=%p path=\"%s\"",
                           ag->source_name, ag->source_obj, ag->path);
            }
        } else if (!ag->logged_source_missing) {
            ag->logged_source_missing = 1;
            log_line("Audio3D source not found node=\"%s\" path=\"%s\"", ag->source_name, ag->path);
        }
    }
    if (!ag->source_obj) return;

    engine_GetModelViewRotationPivot(ag->source_obj, source);
    if (ag->openal && source[0] == 0.0f && source[1] == 0.0f && source[2] == 0.0f) {
        ag->source_zero_count++;
        if (ag->source_zero_count >= 2) {
            ag->source_position_disabled = 1;
            log_line("AudioOpenAL source position unresolved; keeping non-positional playback node=\"%s\" path=\"%s\"",
                     ag->source_name, ag->path);
        }
    } else {
        ag->source_zero_count = 0;
    }
    if (ag->openal) {
        if (vm_alSourcefv) vm_alSourcefv(ag->al_source, 0x1004, source);
        if (ag->current_volume != 0) {
            ag->current_volume = 0;
            debug_line("AudioOpenAL source position node=\"%s\" pos=(%.1f %.1f %.1f)",
                       ag->source_name, source[0], source[1], source[2]);
        }
        return;
    }

    if (!audio_graph_get_listener_position(ag, now, listener)) return;

    dx = source[0] - listener[0];
    dy = source[1] - listener[1];
    dz = source[2] - listener[2];
    d2 = dx * dx + dy * dy + dz * dz;
    min2 = (float)(ag->min_distance * ag->min_distance);
    max2 = (float)(ag->max_distance * ag->max_distance);
    base_volume = audio_volume_to_directshow(ag->volume);
    target_volume = base_volume;
    if (d2 >= max2) {
        target_volume = -10000;
    } else if (d2 > min2 && max2 > min2) {
        t = (d2 - min2) / (max2 - min2);
        target_volume = base_volume - (long)(t * 10000.0f);
        if (target_volume < -10000) target_volume = -10000;
    }
    if (ag->current_volume != target_volume) {
        basic_audio_put_volume(ag->basic_audio, target_volume);
        ag->current_volume = target_volume;
        debug_line("Audio3D volume node=\"%s\" volume=%ld source=(%.1f %.1f %.1f) listener=(%.1f %.1f %.1f)",
                   ag->source_name, target_volume, source[0], source[1], source[2],
                   listener[0], listener[1], listener[2]);
    }
}

static void basic_audio_release(void *basic_audio)
{
    void **vt;
    if (!basic_audio) return;
    vt = *(void***)basic_audio;
    if (vt) ((com_ref_t)vt[2])(basic_audio);
}

static int openal_resolve_symbols(void)
{
    HMODULE al;
    if (openal_symbols_attempted) return vm_alGenSources && vm_alBufferData && vm_alcGetCurrentContext;
    openal_symbols_attempted = 1;
    al = GetModuleHandleA("OpenAL32.dll");
    if (!al) al = LoadLibraryA("OpenAL32.dll");
    if (!al) return 0;
    vm_alGenSources = (al_gen_sources_t)GetProcAddress(al, "alGenSources");
    vm_alDeleteSources = (al_delete_sources_t)GetProcAddress(al, "alDeleteSources");
    vm_alSourcePlay = (al_source_play_t)GetProcAddress(al, "alSourcePlay");
    vm_alSourceStop = (al_source_stop_t)GetProcAddress(al, "alSourceStop");
    vm_alSourcei = (al_sourcei_t)GetProcAddress(al, "alSourcei");
    vm_alSourcef = (al_sourcef_t)GetProcAddress(al, "alSourcef");
    vm_alSourcefv = (al_sourcefv_t)GetProcAddress(al, "alSourcefv");
    vm_alSourceQueueBuffers = (al_source_queue_buffers_t)GetProcAddress(al, "alSourceQueueBuffers");
    vm_alSourceUnqueueBuffers = (al_source_unqueue_buffers_t)GetProcAddress(al, "alSourceUnqueueBuffers");
    vm_alGetSourcei = (al_get_sourcei_t)GetProcAddress(al, "alGetSourcei");
    vm_alGetSourcef = (al_get_sourcef_t)GetProcAddress(al, "alGetSourcef");
    vm_alGenBuffers = (al_gen_buffers_t)GetProcAddress(al, "alGenBuffers");
    vm_alDeleteBuffers = (al_delete_buffers_t)GetProcAddress(al, "alDeleteBuffers");
    vm_alBufferData = (al_buffer_data_t)GetProcAddress(al, "alBufferData");
    vm_alGetError = (al_get_error_t)GetProcAddress(al, "alGetError");
    vm_alcGetCurrentContext = (alc_get_current_context_t)GetProcAddress(al, "alcGetCurrentContext");
    return vm_alGenSources && vm_alDeleteSources && vm_alSourcePlay && vm_alSourceStop &&
           vm_alSourcei && vm_alSourcef && vm_alSourcefv && vm_alGenBuffers &&
           vm_alDeleteBuffers && vm_alBufferData && vm_alcGetCurrentContext;
}

static int audio_effect_is_none(const char *effect)
{
    return !effect || !effect[0] || !_stricmp(effect, "none") || !_stricmp(effect, "off") || !_stricmp(effect, "0");
}

static void audio_graph_apply_pcm_effect(BYTE *pcm, DWORD pcm_bytes, DWORD sample_rate, const char *effect)
{
    short *samples;
    DWORD count;
    DWORD delay1;
    DWORD delay2;
    DWORD i;
    float feedback1;
    float feedback2;
    if (!pcm || pcm_bytes < 4 || !sample_rate || audio_effect_is_none(effect)) return;
    samples = (short*)pcm;
    count = pcm_bytes / 2;
    if (!_stricmp(effect, "echo")) {
        delay1 = (DWORD)((double)sample_rate * 0.18);
        delay2 = (DWORD)((double)sample_rate * 0.32);
        feedback1 = 0.42f;
        feedback2 = 0.22f;
    } else if (!_stricmp(effect, "small_room")) {
        delay1 = (DWORD)((double)sample_rate * 0.035);
        delay2 = (DWORD)((double)sample_rate * 0.071);
        feedback1 = 0.18f;
        feedback2 = 0.10f;
    } else if (!_stricmp(effect, "reverb") || !_stricmp(effect, "large_room")) {
        delay1 = (DWORD)((double)sample_rate * 0.055);
        delay2 = (DWORD)((double)sample_rate * 0.137);
        feedback1 = !_stricmp(effect, "large_room") ? 0.32f : 0.26f;
        feedback2 = !_stricmp(effect, "large_room") ? 0.22f : 0.16f;
    } else {
        return;
    }
    if (!delay1 || delay1 >= count) return;
    if (delay2 >= count) delay2 = 0;
    for (i = delay1; i < count; i++) {
        float v = (float)samples[i] + (float)samples[i - delay1] * feedback1;
        if (delay2 && i >= delay2) v += (float)samples[i - delay2] * feedback2;
        if (v > 32767.0f) v = 32767.0f;
        if (v < -32768.0f) v = -32768.0f;
        samples[i] = (short)v;
    }
    debug_line("AudioOpenAL software effect applied effect=\"%s\" bytes=%lu hz=%lu", effect, pcm_bytes, sample_rate);
}

static int read_wav_pcm_mono16_a(const char *path, BYTE **data, DWORD *data_bytes, DWORD *sample_rate)
{
    HANDLE h;
    DWORD read = 0;
    DWORD file_size;
    BYTE header[44];
    WORD format, channels, bits;
    DWORD rate, bytes;
    BYTE *buf;
    if (!path || !data || !data_bytes || !sample_rate) return 0;
    *data = NULL;
    *data_bytes = 0;
    *sample_rate = 0;
    h = real_CreateFileA ? real_CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)
                         : CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    file_size = GetFileSize(h, NULL);
    if (file_size <= sizeof(header) || !ReadFile(h, header, sizeof(header), &read, NULL) || read != sizeof(header)) {
        if (real_CloseHandle) real_CloseHandle(h); else CloseHandle(h);
        return 0;
    }
    if (memcmp(header, "RIFF", 4) || memcmp(header + 8, "WAVE", 4) ||
        memcmp(header + 12, "fmt ", 4) || memcmp(header + 36, "data", 4)) {
        if (real_CloseHandle) real_CloseHandle(h); else CloseHandle(h);
        return 0;
    }
    memcpy(&format, header + 20, 2);
    memcpy(&channels, header + 22, 2);
    memcpy(&rate, header + 24, 4);
    memcpy(&bits, header + 34, 2);
    memcpy(&bytes, header + 40, 4);
    if (format != WAVE_FORMAT_PCM || channels != 1 || bits != 16 || rate == 0 ||
        bytes == 0 || bytes > file_size - sizeof(header) || bytes > 128u * 1024u * 1024u) {
        if (real_CloseHandle) real_CloseHandle(h); else CloseHandle(h);
        return 0;
    }
    buf = (BYTE*)malloc(bytes);
    if (!buf) {
        if (real_CloseHandle) real_CloseHandle(h); else CloseHandle(h);
        return 0;
    }
    if (!ReadFile(h, buf, bytes, &read, NULL) || read != bytes) {
        free(buf);
        if (real_CloseHandle) real_CloseHandle(h); else CloseHandle(h);
        return 0;
    }
    if (real_CloseHandle) real_CloseHandle(h); else CloseHandle(h);
    *data = buf;
    *data_bytes = bytes;
    *sample_rate = rate;
    return 1;
}

static audio_graph_t *audio_graph_create_openal_3d(const char *path, int volume,
                                                   int min_distance, int max_distance, int rolloff,
                                                   const char *source_name, const char *audio_effect)
{
    audio_graph_t *ag;
    BYTE *pcm = NULL;
    DWORD pcm_bytes = 0;
    DWORD sample_rate = 0;
    char sound_id[128];
    char cache_path[MAX_PATH * 4];
    float gain;
    float ref_distance;
    float max_al_distance;
    int err = 0;
    if (!path || !path[0] || !source_name || !source_name[0]) return NULL;
    if (!openal_resolve_symbols() || !vm_alcGetCurrentContext()) return NULL;
    if (!remux_webm_audio_to_ogg_a(path, sound_id, sizeof(sound_id), cache_path, sizeof(cache_path))) return NULL;
    if (!ends_with_i(cache_path, ".wav")) {
        char root[MAX_PATH * 4];
        char dir[MAX_PATH * 4];
        cache_root_a(root, sizeof(root));
        path_join(dir, sizeof(dir), root, "Sounds\\Shared\\Effect");
        _snprintf(cache_path, sizeof(cache_path) - 1, "%s\\%s.wav", dir, sound_id);
        cache_path[sizeof(cache_path) - 1] = 0;
    }
    if (!read_wav_pcm_mono16_a(cache_path, &pcm, &pcm_bytes, &sample_rate)) {
        log_line("AudioOpenAL wav cache read failed cache=\"%s\" path=\"%s\"", cache_path, path);
        return NULL;
    }
    ag = (audio_graph_t*)calloc(1, sizeof(*ag));
    if (!ag) {
        free(pcm);
        return NULL;
    }
    ag->openal = 1;
    ag->active = 1;
    ag->volume = clamp_int(volume, -10000, 20000);
    ag->audio_3d = 1;
    ag->min_distance = clamp_int(min_distance, 1, 100000);
    ag->max_distance = clamp_int(max_distance, ag->min_distance + 1, 100000);
    ag->rolloff = clamp_int(rolloff, 0, 20);
    ag->current_volume = 12345;
    lstrcpynA(ag->path, path, sizeof(ag->path));
    lstrcpynA(ag->source_name, source_name, sizeof(ag->source_name));
    if (audio_effect) lstrcpynA(ag->audio_effect, audio_effect, sizeof(ag->audio_effect));
    vm_alGenBuffers(1, &ag->al_buffer);
    audio_graph_apply_pcm_effect(pcm, pcm_bytes, sample_rate, ag->audio_effect);
    vm_alBufferData(ag->al_buffer, AL_FORMAT_MONO16_, pcm, (int)pcm_bytes, (int)sample_rate);
    free(pcm);
    vm_alGenSources(1, &ag->al_source);
    vm_alSourcei(ag->al_source, AL_BUFFER_, (int)ag->al_buffer);
    vm_alSourcei(ag->al_source, AL_LOOPING_, 1);
    vm_alSourcei(ag->al_source, AL_SOURCE_RELATIVE_, 0);
    ref_distance = (float)ag->min_distance * 0.01f;
    max_al_distance = (float)ag->max_distance * 0.01f;
    if (ref_distance < 0.01f) ref_distance = 0.01f;
    if (max_al_distance <= ref_distance) max_al_distance = ref_distance + 0.01f;
    vm_alSourcef(ag->al_source, AL_REFERENCE_DISTANCE_, ref_distance);
    vm_alSourcef(ag->al_source, AL_MAX_DISTANCE_, max_al_distance);
    vm_alSourcef(ag->al_source, AL_ROLLOFF_FACTOR_, (float)ag->rolloff);
    gain = engine_audio_volume_to_float(ag->volume);
    vm_alSourcef(ag->al_source, AL_GAIN_, gain);
    audio_graph_update_3d(ag, GetTickCount());
    vm_alSourcePlay(ag->al_source);
    if (vm_alGetError) err = vm_alGetError();
    if (err || !ag->al_source || !ag->al_buffer) {
        log_line("AudioOpenAL start failed err=0x%04x source=%u buffer=%u path=\"%s\"",
                 err, ag->al_source, ag->al_buffer, path);
        audio_graph_release(ag);
        return NULL;
    }
    debug_line("AudioOpenAL started source=%u buffer=%u bytes=%lu hz=%lu node=\"%s\" distance=%d-%d openal_distance=%.2f-%.2f rolloff=%d effect=\"%s\" path=\"%s\"",
             ag->al_source, ag->al_buffer, pcm_bytes, sample_rate,
             ag->source_name, ag->min_distance, ag->max_distance,
             ref_distance, max_al_distance, ag->rolloff,
             audio_effect_is_none(ag->audio_effect) ? "none" : ag->audio_effect, path);
    return ag;
}

static audio_graph_t *audio_graph_create_twitch_stream(const char *path, int volume,
                                                        int audio_3d, int min_distance,
                                                        int max_distance, int rolloff,
                                                        const char *source_name)
{
    audio_graph_t *ag;
    float gain;
    float ref_distance;
    float max_al_distance;
    int i;
    int err = 0;
    if (!path || !path[0] || !openal_resolve_symbols() || !vm_alcGetCurrentContext() ||
        !vm_alSourceQueueBuffers || !vm_alSourceUnqueueBuffers || !vm_alGetSourcei) {
        return NULL;
    }
    ag = (audio_graph_t*)calloc(1, sizeof(*ag));
    if (!ag) return NULL;
    ag->openal = 1;
    ag->active = 1;
    ag->twitch_streaming = 1;
    ag->volume = clamp_int(volume, -10000, 20000);
    ag->audio_3d = audio_3d && source_name && source_name[0] ? 1 : 0;
    ag->min_distance = clamp_int(min_distance, 1, 100000);
    ag->max_distance = clamp_int(max_distance, ag->min_distance + 1, 100000);
    ag->rolloff = clamp_int(rolloff, 0, 20);
    ag->current_volume = 12345;
    lstrcpynA(ag->path, path, sizeof(ag->path));
    if (source_name) lstrcpynA(ag->source_name, source_name, sizeof(ag->source_name));
    vm_alGenBuffers(WEBM_TWITCH_OPENAL_BUFFERS, ag->twitch_buffers);
    vm_alGenSources(1, &ag->al_source);
    if (vm_alGetError) err = vm_alGetError();
    if (err || !ag->al_source) {
        log_line("Twitch audio OpenAL allocation failed err=0x%04x source=%u path=\"%s\"",
                 err, ag->al_source, path);
        audio_graph_release(ag);
        return NULL;
    }
    ag->twitch_buffer_count = WEBM_TWITCH_OPENAL_BUFFERS;
    for (i = 0; i < ag->twitch_buffer_count; i++) {
        if (!ag->twitch_buffers[i]) {
            log_line("Twitch audio OpenAL returned an invalid buffer path=\"%s\"", path);
            audio_graph_release(ag);
            return NULL;
        }
        ag->twitch_free_buffers[ag->twitch_free_count++] = ag->twitch_buffers[i];
    }
    vm_alSourcei(ag->al_source, AL_LOOPING_, 0);
    vm_alSourcei(ag->al_source, AL_SOURCE_RELATIVE_, ag->audio_3d ? 0 : 1);
    if (ag->audio_3d) {
        ref_distance = (float)ag->min_distance * 0.01f;
        max_al_distance = (float)ag->max_distance * 0.01f;
        if (ref_distance < 0.01f) ref_distance = 0.01f;
        if (max_al_distance <= ref_distance) max_al_distance = ref_distance + 0.01f;
        vm_alSourcef(ag->al_source, AL_REFERENCE_DISTANCE_, ref_distance);
        vm_alSourcef(ag->al_source, AL_MAX_DISTANCE_, max_al_distance);
        vm_alSourcef(ag->al_source, AL_ROLLOFF_FACTOR_, (float)ag->rolloff);
        audio_graph_update_3d(ag, GetTickCount());
    }
    gain = engine_audio_volume_to_float(ag->volume);
    vm_alSourcef(ag->al_source, AL_GAIN_, gain);
    if (vm_alGetError) err = vm_alGetError();
    if (err) {
        log_line("Twitch audio OpenAL source setup failed err=0x%04x path=\"%s\"", err, path);
        audio_graph_release(ag);
        return NULL;
    }
    debug_line("Twitch audio stream ready source=%u buffers=%d volume=%d spatial=%d node=\"%s\" distance=%d-%d rolloff=%d path=\"%s\"",
             ag->al_source, ag->twitch_buffer_count, ag->volume, ag->audio_3d,
             ag->source_name, ag->min_distance, ag->max_distance, ag->rolloff, path);
    return ag;
}

static int twitch_audio_buffer_index(const audio_graph_t *ag,
                                     unsigned int buffer)
{
    int index;
    if (!ag || !buffer) return -1;
    for (index = 0; index < ag->twitch_buffer_count; ++index) {
        if (ag->twitch_buffers[index] == buffer) return index;
    }
    return -1;
}

static void twitch_audio_queue_track(audio_graph_t *ag, unsigned int buffer,
                                     double pts_ms, double duration_ms)
{
    int buffer_index;
    int queue_index;
    if (!ag || !buffer ||
        ag->twitch_queue_count >= WEBM_TWITCH_OPENAL_BUFFERS) return;
    buffer_index = twitch_audio_buffer_index(ag, buffer);
    if (buffer_index < 0) return;
    ag->twitch_buffer_pts_ms[buffer_index] = pts_ms;
    ag->twitch_buffer_duration_ms[buffer_index] = duration_ms;
    queue_index = (ag->twitch_queue_head + ag->twitch_queue_count) %
                  WEBM_TWITCH_OPENAL_BUFFERS;
    ag->twitch_queue_order[queue_index] = buffer;
    ag->twitch_queue_count++;
}

static void twitch_audio_queue_untrack(audio_graph_t *ag,
                                       unsigned int buffer)
{
    int offset;
    int found = -1;
    if (!ag || !buffer || ag->twitch_queue_count <= 0) return;
    for (offset = 0; offset < ag->twitch_queue_count; ++offset) {
        int index = (ag->twitch_queue_head + offset) %
                    WEBM_TWITCH_OPENAL_BUFFERS;
        if (ag->twitch_queue_order[index] == buffer) {
            found = offset;
            break;
        }
    }
    if (found < 0) return;
    for (offset = found; offset + 1 < ag->twitch_queue_count; ++offset) {
        int destination = (ag->twitch_queue_head + offset) %
                          WEBM_TWITCH_OPENAL_BUFFERS;
        int source = (ag->twitch_queue_head + offset + 1) %
                     WEBM_TWITCH_OPENAL_BUFFERS;
        ag->twitch_queue_order[destination] =
            ag->twitch_queue_order[source];
    }
    ag->twitch_queue_count--;
}

static int twitch_audio_playback_clock(const audio_graph_t *ag,
                                       double *out_pts_ms)
{
    unsigned int buffer;
    int buffer_index;
    float seconds = 0.0f;
    double offset_ms;
    if (!ag || !out_pts_ms || !vm_alGetSourcef ||
        ag->twitch_queue_count <= 0) return 0;
    buffer = ag->twitch_queue_order[ag->twitch_queue_head];
    buffer_index = twitch_audio_buffer_index(ag, buffer);
    if (buffer_index < 0) return 0;
    vm_alGetSourcef(ag->al_source, AL_SEC_OFFSET_, &seconds);
    if (!_finite(seconds) || seconds < 0.0f) seconds = 0.0f;
    offset_ms = (double)seconds * 1000.0;
    if (offset_ms > ag->twitch_buffer_duration_ms[buffer_index])
        offset_ms = ag->twitch_buffer_duration_ms[buffer_index];
    *out_pts_ms = ag->twitch_buffer_pts_ms[buffer_index] + offset_ms;
    return 1;
}

static void audio_graph_update_twitch_stream(audio_graph_t *ag, video_decoder_t *dec,
                                             DWORD target_ms)
{
    int processed = 0;
    int queued = 0;
    int state = 0;
    int start_buffers;
    int playback_clock_ready;
    double desired_pts_ms;
    double chunk_end_ms;
    double duration_ms;
    double playback_pts_ms;
    int bytes;
    int samples;
    int sample_rate;
    int format;
    double pts_ms;
    unsigned int buffer;
    int err = 0;
    if (!ag || !dec || !ag->twitch_streaming || !ag->openal || !ag->al_source ||
        !vm_alSourceQueueBuffers || !vm_alSourceUnqueueBuffers || !vm_alGetSourcei) {
        return;
    }
    audio_graph_update_3d(ag, GetTickCount());
    if (!ag->twitch_started) dec->live_audio_clock_valid = 0;
    vm_alGetSourcei(ag->al_source, AL_BUFFERS_PROCESSED_, &processed);
    while (processed-- > 0 && ag->twitch_free_count < ag->twitch_buffer_count) {
        buffer = 0;
        vm_alSourceUnqueueBuffers(ag->al_source, 1, &buffer);
        if (buffer) {
            twitch_audio_queue_untrack(ag, buffer);
            ag->twitch_free_buffers[ag->twitch_free_count++] = buffer;
        }
    }
    playback_clock_ready = !dec->live_buffer_ms || target_ms >= dec->live_buffer_ms;
    if (!ag->twitch_started && !playback_clock_ready) return;
    desired_pts_ms = target_ms > dec->live_buffer_ms ?
                     (double)(target_ms - dec->live_buffer_ms) : 0.0;
    while (ag->twitch_free_count > 0 &&
           video_decoder_twitch_audio_pop(dec, &ag->twitch_pcm, &ag->twitch_pcm_capacity,
                                          &bytes, &samples, &sample_rate, &pts_ms)) {
        chunk_end_ms = pts_ms + ((double)samples * 1000.0) /
                       (double)(sample_rate > 0 ? sample_rate : 48000);
        duration_ms = chunk_end_ms - pts_ms;
        if (!ag->twitch_started && chunk_end_ms + 5.0 < desired_pts_ms) {
            ag->twitch_sync_dropped_chunks++;
            continue;
        }
        buffer = ag->twitch_free_buffers[--ag->twitch_free_count];
        format = AL_FORMAT_STEREO16_;
        if (ag->audio_3d) {
            short *pcm = (short*)ag->twitch_pcm;
            int i;
            for (i = 0; i < samples; i++) {
                int mixed = (int)pcm[i * 2] + (int)pcm[i * 2 + 1];
                pcm[i] = (short)(mixed / 2);
            }
            bytes = samples * (int)sizeof(short);
            format = AL_FORMAT_MONO16_;
        }
        vm_alBufferData(buffer, format, ag->twitch_pcm, bytes, sample_rate);
        vm_alSourceQueueBuffers(ag->al_source, 1, &buffer);
        twitch_audio_queue_track(ag, buffer, pts_ms, duration_ms);
        if (!ag->twitch_started && !ag->twitch_start_pts_set) {
            ag->twitch_start_pts_ms = pts_ms;
            ag->twitch_start_pts_set = 1;
        }
    }
    vm_alGetSourcei(ag->al_source, AL_BUFFERS_QUEUED_, &queued);
    vm_alGetSourcei(ag->al_source, AL_SOURCE_STATE_, &state);
    start_buffers = dec->live_buffer_ms ? 3 : 4;
    if ((!ag->twitch_started && playback_clock_ready && queued >= start_buffers) ||
        (ag->twitch_started && state != AL_PLAYING_ && queued >= 3)) {
        if (ag->twitch_started && state != AL_PLAYING_) ag->twitch_underruns++;
        vm_alSourcePlay(ag->al_source);
        if (!ag->twitch_started) {
            debug_line("Twitch audio playback started clock_ms=%lu video_pts_ms=%.1f audio_pts_ms=%.1f buffer_ms=%lu queued=%d sync_dropped=%u",
                     (unsigned long)target_ms, desired_pts_ms,
                     ag->twitch_start_pts_set ? ag->twitch_start_pts_ms : -1.0,
                     (unsigned long)dec->live_buffer_ms, queued,
                     ag->twitch_sync_dropped_chunks);
        }
        ag->twitch_started = 1;
    }
    if (ag->twitch_started && state == AL_PLAYING_ &&
        twitch_audio_playback_clock(ag, &playback_pts_ms)) {
        if (!dec->live_audio_clock_valid) {
            dec->live_audio_clock_offset_ms = desired_pts_ms - playback_pts_ms;
            dec->live_audio_clock_valid = 1;
            debug_line("Twitch A/V clock locked audio_pts_ms=%.1f video_pts_ms=%.1f offset_ms=%.1f",
                       playback_pts_ms, desired_pts_ms,
                       dec->live_audio_clock_offset_ms);
        }
        dec->live_audio_clock_ms = playback_pts_ms +
                                   dec->live_audio_clock_offset_ms;
        if (dec->live_audio_clock_ms < 0.0)
            dec->live_audio_clock_ms = 0.0;
    }
    if (vm_alGetError) err = vm_alGetError();
    if (err) {
        debug_line("Twitch audio OpenAL update err=0x%04x queued=%d free=%d path=\"%s\"",
                   err, queued, ag->twitch_free_count, ag->path);
    }
}

static DWORD video_decoder_twitch_synced_target_ms(video_decoder_t *dec,
                                                     DWORD fallback_ms)
{
    double target_ms;
    if (!dec || !dec->network_source || !dec->live_buffer_ms ||
        !dec->live_audio_clock_valid) return fallback_ms;
    target_ms = dec->live_audio_clock_ms + (double)dec->live_buffer_ms;
    if (target_ms <= 0.0) return 0;
    if (target_ms >= 4294967295.0) return 0xffffffffu;
    return (DWORD)(target_ms + 0.5);
}

static audio_graph_t *audio_graph_create(const char *path, int lead_ms, int volume,
                                         int audio_3d, int min_distance, int max_distance,
                                         int rolloff, const char *source_name, const char *audio_effect)
{
    audio_graph_t *ag = NULL;
    wchar_t wpath[MAX_PATH * 4];
    HRESULT hr;
    REFERENCE_TIME start = 0;
    if (!path || !path[0]) return NULL;
    if (audio_3d && source_name && source_name[0]) {
        ag = audio_graph_create_openal_3d(path, volume, min_distance, max_distance, rolloff, source_name, audio_effect);
        if (ag) return ag;
        log_line("AudioOpenAL unavailable; falling back to regular DirectShow audio path=\"%s\" node=\"%s\"",
                 path, source_name);
    }
    if (!MultiByteToWideChar(CP_ACP, 0, path, -1, wpath, (int)(sizeof(wpath) / sizeof(wpath[0])))) return NULL;
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ag = (audio_graph_t*)calloc(1, sizeof(*ag));
    if (!ag) return NULL;
    lstrcpynA(ag->path, path, sizeof(ag->path));
    ag->lead_ms = lead_ms;
    ag->volume = clamp_int(volume, -10000, 20000);
    ag->audio_3d = audio_3d ? 1 : 0;
    ag->min_distance = clamp_int(min_distance, 1, 100000);
    ag->max_distance = clamp_int(max_distance, ag->min_distance + 1, 100000);
    ag->rolloff = clamp_int(rolloff, 0, 20);
    ag->current_volume = 12345;
    if (source_name) lstrcpynA(ag->source_name, source_name, sizeof(ag->source_name));
    if (audio_effect) lstrcpynA(ag->audio_effect, audio_effect, sizeof(ag->audio_effect));
    hr = CoCreateInstance(&CLSID_FilterGraph_, NULL, CLSCTX_INPROC_SERVER, &IID_IGraphBuilder_, (void**)&ag->graph);
    if (FAILED(hr) || !ag->graph) {
        log_line("AudioGraph CoCreate graph failed hr=0x%08lx path=\"%s\"", (DWORD)hr, path);
        audio_graph_release(ag);
        return NULL;
    }
    hr = create_lav_source_filter_for_file(wpath, (void**)&ag->source_filter);
    if (FAILED(hr) || !ag->source_filter) {
        log_line("AudioGraph source failed hr=0x%08lx path=\"%s\"", (DWORD)hr, path);
        audio_graph_release(ag);
        return NULL;
    }
    hr = IGraphBuilder_AddFilter(ag->graph, ag->source_filter, L"NC-TK17-WebM audio source");
    if (FAILED(hr)) {
        log_line("AudioGraph add source failed hr=0x%08lx path=\"%s\"", (DWORD)hr, path);
        audio_graph_release(ag);
        return NULL;
    }
    hr = create_lav_filter_object("LAVAudio.ax", &CLSID_LAVAudioDecoder_, (void**)&ag->audio_filter);
    if (FAILED(hr) || !ag->audio_filter) {
        log_line("AudioGraph LAV audio failed hr=0x%08lx path=\"%s\"", (DWORD)hr, path);
        audio_graph_release(ag);
        return NULL;
    }
    hr = IGraphBuilder_AddFilter(ag->graph, ag->audio_filter, L"NC-TK17-WebM LAV audio");
    if (FAILED(hr)) {
        log_line("AudioGraph add audio failed hr=0x%08lx path=\"%s\"", (DWORD)hr, path);
        audio_graph_release(ag);
        return NULL;
    }
    hr = CoCreateInstance(&CLSID_DSoundRender_, NULL, CLSCTX_INPROC_SERVER, &IID_IBaseFilter_, (void**)&ag->renderer_filter);
    if (FAILED(hr) || !ag->renderer_filter) {
        log_line("AudioGraph renderer create failed hr=0x%08lx path=\"%s\"", (DWORD)hr, path);
        audio_graph_release(ag);
        return NULL;
    }
    hr = IGraphBuilder_AddFilter(ag->graph, ag->renderer_filter, L"NC-TK17-WebM DirectSound renderer");
    if (FAILED(hr)) {
        log_line("AudioGraph add renderer failed hr=0x%08lx path=\"%s\"", (DWORD)hr, path);
        audio_graph_release(ag);
        return NULL;
    }
    hr = connect_filters_any_pin(ag->graph, ag->source_filter, ag->audio_filter);
    if (FAILED(hr)) {
        log_line("AudioGraph connect source->audio failed hr=0x%08lx path=\"%s\"", (DWORD)hr, path);
        audio_graph_release(ag);
        return NULL;
    }
    hr = connect_filters_any_pin(ag->graph, ag->audio_filter, ag->renderer_filter);
    if (FAILED(hr)) {
        log_line("AudioGraph connect audio->renderer failed hr=0x%08lx path=\"%s\"", (DWORD)hr, path);
        audio_graph_release(ag);
        return NULL;
    }
    IGraphBuilder_QueryInterface(ag->graph, &IID_IMediaControl_, (void**)&ag->control);
    IGraphBuilder_QueryInterface(ag->graph, &IID_IMediaSeeking_, (void**)&ag->seeking);
    IGraphBuilder_QueryInterface(ag->graph, &IID_IBasicAudio_, (void**)&ag->basic_audio);
    if (ag->basic_audio) {
        hr = basic_audio_put_volume(ag->basic_audio, audio_volume_to_directshow(ag->volume));
        if (FAILED(hr)) {
            log_line("AudioGraph volume failed hr=0x%08lx volume=%d path=\"%s\"", (DWORD)hr, ag->volume, path);
        }
    } else {
        log_line("AudioGraph volume interface unavailable volume=%d path=\"%s\"", ag->volume, path);
    }
    if (lead_ms < 0 && ag->seeking) {
        start = (REFERENCE_TIME)(-lead_ms) * 10000;
        IMediaSeeking_SetPositions(ag->seeking, &start, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
    }
    if (ag->control) {
        hr = IMediaControl_Run(ag->control);
        if (FAILED(hr)) {
            log_line("AudioGraph Run failed hr=0x%08lx lead_ms=%d path=\"%s\"", (DWORD)hr, lead_ms, path);
            audio_graph_release(ag);
            return NULL;
        }
    }
    ag->active = 1;
    debug_line("AudioGraph started lead_ms=%d volume=%d audio_3d=%d node=\"%s\" path=\"%s\"",
               lead_ms, ag->volume, ag->audio_3d, ag->source_name, path);
    return ag;
}

static void audio_graph_release(audio_graph_t *ag)
{
    if (!ag) return;
    if (ag->openal) {
        if (vm_alSourceStop && ag->al_source) vm_alSourceStop(ag->al_source);
        if (vm_alDeleteSources && ag->al_source) vm_alDeleteSources(1, &ag->al_source);
        if (ag->twitch_streaming) {
            if (vm_alDeleteBuffers && ag->twitch_buffer_count > 0) {
                vm_alDeleteBuffers(ag->twitch_buffer_count, ag->twitch_buffers);
            }
            free(ag->twitch_pcm);
        } else if (vm_alDeleteBuffers && ag->al_buffer) {
            vm_alDeleteBuffers(1, &ag->al_buffer);
        }
        debug_line("AudioOpenAL stopped path=\"%s\"", ag->path);
        free(ag);
        return;
    }
    if (ag->control) {
        IMediaControl_Stop(ag->control);
        IMediaControl_Release(ag->control);
    }
    if (ag->seeking) IMediaSeeking_Release(ag->seeking);
    if (ag->basic_audio) basic_audio_release(ag->basic_audio);
    if (ag->renderer_filter) IBaseFilter_Release(ag->renderer_filter);
    if (ag->audio_filter) IBaseFilter_Release(ag->audio_filter);
    if (ag->source_filter) IBaseFilter_Release(ag->source_filter);
    if (ag->graph) IGraphBuilder_Release(ag->graph);
    debug_line("AudioGraph stopped path=\"%s\"", ag->path);
    free(ag);
}

static float engine_audio_volume_to_float(int volume)
{
    if (volume <= -10000) return 0.0f;
    if (volume >= 20000) return 3.0f;
    if (volume > 0) return 1.0f + ((float)volume / 10000.0f);
    return 1.0f + ((float)volume / 10000.0f);
}

static void *engine_audio_current_device(void)
{
    if (engine_SoundDevice_ptr && *engine_SoundDevice_ptr) return *engine_SoundDevice_ptr;
    return engine_captured_sound_device;
}

static engine_audio_player_t *engine_audio_player_create(const char *webm, const char *parent_path,
                                                         int volume, int min_distance, int max_distance)
{
    engine_audio_player_t *ep;
    void *device;
    char sound_id[128];
    char cache_path[MAX_PATH * 4];
    char sound_name[160];
    float pos[3] = {0, 0, 0};
    int play_ok;
    if (!webm || !webm[0] || !parent_path || !parent_path[0]) return NULL;
    if (!engine_audio_native_playback_enabled) {
        static DWORD last_disabled_log_tick;
        DWORD now = GetTickCount();
        if (!last_disabled_log_tick || now - last_disabled_log_tick > 5000) {
            last_disabled_log_tick = now;
            log_line("EngineAudio native playback disabled; using regular texture audio until engine-compatible Vorbis cache is available webm=\"%s\" parent=\"%s\"",
                     webm, parent_path);
        }
        return NULL;
    }
    resolve_engine_audio_symbols();
    device = engine_audio_current_device();
    if (!device || !engine_SoundSource_Create ||
        !engine_SoundSource_Play || !engine_SoundSource_Stop || !engine_FindObjC ||
        !engine_GetModelViewRotationPivot) {
        static DWORD last_unavailable_log_tick;
        DWORD now = GetTickCount();
        if (!last_unavailable_log_tick || now - last_unavailable_log_tick > 5000) {
            last_unavailable_log_tick = now;
            log_line("EngineAudio unavailable device_slot=%p device=%p captured=%p create=%p play=%p stop=%p FindObjC=%p Pivot=%p webm=\"%s\"",
                     (void*)engine_SoundDevice_ptr, engine_SoundDevice_ptr ? *engine_SoundDevice_ptr : NULL,
                     engine_captured_sound_device, (void*)engine_SoundSource_Create, (void*)engine_SoundSource_Play,
                     (void*)engine_SoundSource_Stop, (void*)engine_FindObjC,
                     (void*)engine_GetModelViewRotationPivot, webm);
        }
        return NULL;
    }
    if (!remux_webm_audio_to_ogg_a(webm, sound_id, sizeof(sound_id), cache_path, sizeof(cache_path))) {
        log_line("EngineAudio cache failed webm=\"%s\"", webm);
        return NULL;
    }
    ep = (engine_audio_player_t*)calloc(1, sizeof(*ep));
    if (!ep) return NULL;
    lstrcpynA(ep->webm_path, webm, sizeof(ep->webm_path));
    lstrcpynA(ep->parent_path, parent_path, sizeof(ep->parent_path));
    lstrcpynA(ep->sound_name, sound_id, sizeof(ep->sound_name));
    ep->volume = volume;
    ep->min_distance = clamp_int(min_distance, 1, 100000);
    ep->max_distance = clamp_int(max_distance, ep->min_distance + 1, 100000);
    ep->parent_obj = engine_FindObjC(parent_path);
    if (!ep->parent_obj) {
        log_line("EngineAudio parent not found parent=\"%s\" webm=\"%s\"", parent_path, webm);
        free(ep);
        return NULL;
    }
    engine_GetModelViewRotationPivot(ep->parent_obj, pos);
    _snprintf(sound_name, sizeof(sound_name) - 1, "Shared/Effect/%s", sound_id);
    sound_name[sizeof(sound_name) - 1] = 0;
    ep->source = engine_SoundSource_Create(device, sound_name, 1u, 1);
    if (!ep->source) {
        log_line("EngineAudio SoundSource create failed sound=\"%s\" cache=\"%s\" parent=\"%s\"", sound_name, cache_path, parent_path);
        free(ep);
        return NULL;
    }
    if (engine_SoundSource_SetPosition) engine_SoundSource_SetPosition(ep->source, pos, 0);
    if (engine_SoundSource_SetDistances) engine_SoundSource_SetDistances(ep->source, (float)ep->min_distance, (float)ep->max_distance, 0);
    if (engine_SoundSource_SetVolume) engine_SoundSource_SetVolume(ep->source, engine_audio_volume_to_float(volume), 0);
    if (engine_SoundSource_SetPlayPosition) engine_SoundSource_SetPlayPosition(ep->source, 0);
    play_ok = engine_SoundSource_Play(ep->source, 1, 1);
    if (!play_ok) {
        log_line("EngineAudio SoundSource play failed sound=\"%s\" parent=\"%s\"", sound_name, parent_path);
        engine_audio_player_release(ep);
        return NULL;
    }
    ep->active = 1;
    debug_line("EngineAudio started sound=\"%s\" parent=\"%s\" pos=(%.1f %.1f %.1f) volume=%d distance=%d-%d",
             sound_name, parent_path, pos[0], pos[1], pos[2], volume, ep->min_distance, ep->max_distance);
    return ep;
}

static int engine_audio_parent_exists_a(const char *parent_path)
{
    resolve_engine_audio_symbols();
    if (!engine_FindObjC || !parent_path || !parent_path[0]) return 0;
    return engine_FindObjC(parent_path) != NULL;
}

static int engine_audio_runtime_available_a(void)
{
    if (!engine_audio_native_playback_enabled) return 1;
    resolve_engine_audio_symbols();
    return engine_audio_current_device() &&
           engine_SoundSource_Create && engine_SoundSource_Play &&
           engine_SoundSource_Stop && engine_FindObjC &&
           engine_GetModelViewRotationPivot;
}

static int build_room_parent_candidate_a(const char *tail, int room_index, int variant,
                                         char *out, size_t out_sz)
{
    if (!tail || !out || !out_sz) return 0;
    if (variant == 0) {
        _snprintf(out, out_sz - 1, "/Room%02d%s", room_index, tail);
    } else if (variant == 1) {
        _snprintf(out, out_sz - 1, "/Room%d%s", room_index, tail);
    } else if (variant == 2) {
        _snprintf(out, out_sz - 1, "/Room/Room%02d%s", room_index, tail);
    } else {
        _snprintf(out, out_sz - 1, "/Room/Room%d%s", room_index, tail);
    }
    out[out_sz - 1] = 0;
    return out[0] != 0;
}

static int resolve_parent_expr_to_existing_a(const char *parent_expr, char *out, size_t out_sz)
{
    char expr[MAX_PATH * 4];
    char candidate[MAX_PATH * 4];
    const char *room_token;
    const char *tail;
    int room_index;
    int variant;
    if (out && out_sz) out[0] = 0;
    if (!parent_expr || !parent_expr[0] || !out || !out_sz) return 0;
    lstrcpynA(expr, parent_expr, sizeof(expr));
    while (expr[0] == ' ' || expr[0] == '\t' || expr[0] == '"') memmove(expr, expr + 1, strlen(expr));
    while (expr[0]) {
        size_t len = strlen(expr);
        if (len && (expr[len - 1] == ' ' || expr[len - 1] == '\t' || expr[len - 1] == '"')) {
            expr[len - 1] = 0;
            continue;
        }
        break;
    }
    if (!strchr(expr, '+') && !strchr(expr, ':')) {
        if (engine_audio_parent_exists_a(expr)) {
            lstrcpynA(out, expr, (int)out_sz);
            return 1;
        }
        return 0;
    }
    room_token = strstr(expr, ":room");
    if (!room_token) return 0;
    tail = strchr(room_token, '"');
    tail = tail ? tail + 1 : room_token + 5;
    if (!tail[0]) tail = "";
    for (room_index = 1; room_index <= 20; room_index++) {
        for (variant = 0; variant < 4; variant++) {
            if (!build_room_parent_candidate_a(tail, room_index, variant, candidate, sizeof(candidate))) continue;
            if (engine_audio_parent_exists_a(candidate)) {
                lstrcpynA(out, candidate, (int)out_sz);
                return 1;
            }
        }
    }
    return 0;
}

static engine_audio_player_t *engine_audio_player_create_from_expr(const char *webm, const char *parent_expr,
                                                                   int volume, int min_distance, int max_distance)
{
    char expr[MAX_PATH * 4];
    char candidate[MAX_PATH * 4];
    const char *room_token;
    const char *tail;
    int room_index;
    int variant;
    if (!parent_expr || !parent_expr[0]) return NULL;
    lstrcpynA(expr, parent_expr, sizeof(expr));
    while (expr[0] == ' ' || expr[0] == '\t' || expr[0] == '"') memmove(expr, expr + 1, strlen(expr));
    while (expr[0]) {
        size_t len = strlen(expr);
        if (len && (expr[len - 1] == ' ' || expr[len - 1] == '\t' || expr[len - 1] == '"')) {
            expr[len - 1] = 0;
            continue;
        }
        break;
    }
    if (resolve_parent_expr_to_existing_a(expr, candidate, sizeof(candidate))) {
        debug_line("EngineAudio runtime parent resolved expr=\"%s\" parent=\"%s\" webm=\"%s\"",
                 expr, candidate, webm);
        return engine_audio_player_create(webm, candidate, volume, min_distance, max_distance);
    }
    if (!strchr(expr, '+') && !strchr(expr, ':')) {
        return engine_audio_player_create(webm, expr, volume, min_distance, max_distance);
    }
    room_token = strstr(expr, ":room");
    if (!room_token) {
        log_line("EngineAudio runtime parent expression unsupported parent=\"%s\" webm=\"%s\"", expr, webm);
        return NULL;
    }
    tail = strchr(room_token, '"');
    tail = tail ? tail + 1 : room_token + 5;
    if (!tail[0]) tail = "";
    for (room_index = 1; room_index <= 20; room_index++) {
        for (variant = 0; variant < 4; variant++) {
            if (!build_room_parent_candidate_a(tail, room_index, variant, candidate, sizeof(candidate))) continue;
            if (engine_audio_parent_exists_a(candidate)) {
                debug_line("EngineAudio runtime parent resolved expr=\"%s\" parent=\"%s\" webm=\"%s\"",
                         expr, candidate, webm);
                return engine_audio_player_create(webm, candidate, volume, min_distance, max_distance);
            }
        }
    }
    log_line("EngineAudio runtime parent not found expr=\"%s\" tail=\"%s\" webm=\"%s\"", expr, tail, webm);
    return NULL;
}

static void engine_audio_player_release(engine_audio_player_t *ep)
{
    if (!ep) return;
    if (ep->source && engine_SoundSource_Stop) engine_SoundSource_Stop(ep->source);
    free(ep);
}

static void engine_audio_player_update(engine_audio_player_t *ep, DWORD now)
{
    float pos[3] = {0, 0, 0};
    if (!ep || !ep->active || !ep->source) return;
    if (ep->last_position_tick && (now - ep->last_position_tick) < 250) return;
    ep->last_position_tick = now;
    if (!ep->parent_obj && engine_FindObjC) ep->parent_obj = engine_FindObjC(ep->parent_path);
    if (ep->parent_obj && engine_GetModelViewRotationPivot && engine_SoundSource_SetPosition) {
        engine_GetModelViewRotationPivot(ep->parent_obj, pos);
        engine_SoundSource_SetPosition(ep->source, pos, 0);
    }
    if (engine_SoundSource_IsPlaying && !engine_SoundSource_IsPlaying(ep->source)) {
        if (engine_SoundSource_SetPlayPosition) engine_SoundSource_SetPlayPosition(ep->source, 0);
        if (engine_SoundSource_Play) engine_SoundSource_Play(ep->source, 1, 1);
        debug_line("EngineAudio loop restart sound=\"%s\" parent=\"%s\"", ep->sound_name, ep->parent_path);
    }
}

static HRESULT video_decoder_read_format(video_decoder_t *dec)
{
    AM_MEDIA_TYPE mt;
    VIDEOINFOHEADER *vih;
    if (!dec || !dec->grabber) return E_INVALIDARG;
    ZeroMemory(&mt, sizeof(mt));
    if (FAILED(ISampleGrabber_GetConnectedMediaType(dec->grabber, &mt))) return E_FAIL;
    if (!mt.pbFormat || mt.cbFormat < sizeof(VIDEOINFOHEADER)) {
        free_media_type_local(&mt);
        return E_FAIL;
    }
    vih = (VIDEOINFOHEADER*)mt.pbFormat;
    dec->width = vih->bmiHeader.biWidth;
    dec->height = vih->bmiHeader.biHeight < 0 ? -vih->bmiHeader.biHeight : vih->bmiHeader.biHeight;
    dec->stride = ((dec->width * 3 + 3) / 4) * 4;
    dec->got_format = dec->width > 0 && dec->height > 0;
    free_media_type_local(&mt);
    return dec->got_format ? S_OK : E_FAIL;
}

static video_decoder_t *video_decoder_create(const char *path)
{
    video_decoder_t *dec;
    wchar_t wpath[MAX_PATH * 4];
    HRESULT hr;
    AM_MEDIA_TYPE mt;
    IPin *sample_out = NULL;
    IPin *null_in = NULL;
    char step[64] = "start";

    if (!path || !path[0]) return NULL;
    if (!MultiByteToWideChar(CP_ACP, 0, path, -1, wpath, (int)(sizeof(wpath) / sizeof(wpath[0])))) return NULL;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        log_line("VideoDecoder CoInitializeEx failed hr=0x%08lx path=\"%s\"", (DWORD)hr, path);
        return NULL;
    }

    dec = (video_decoder_t*)calloc(1, sizeof(*dec));
    if (!dec) return NULL;

    strcpy(step, "FilterGraph");
    hr = CoCreateInstance(&CLSID_FilterGraph_, NULL, CLSCTX_INPROC_SERVER, &IID_IGraphBuilder_, (void**)&dec->graph);
    if (FAILED(hr) || !dec->graph) goto fail;

    strcpy(step, "LAVSource");
    hr = create_lav_source_filter_for_file(wpath, (void**)&dec->source_filter);
    if (FAILED(hr) || !dec->source_filter) goto fail;

    strcpy(step, "LAVVideo");
    hr = create_lav_filter_object("LAVVideo.ax", &CLSID_LAVVideoDecoder_, (void**)&dec->video_filter);
    if (FAILED(hr) || !dec->video_filter) goto fail;

    strcpy(step, "SampleGrabber");
    hr = CoCreateInstance(&CLSID_SampleGrabber_, NULL, CLSCTX_INPROC_SERVER, &IID_IBaseFilter_, (void**)&dec->sample_filter);
    if (FAILED(hr) || !dec->sample_filter) goto fail;
    hr = IBaseFilter_QueryInterface(dec->sample_filter, &IID_ISampleGrabber_, (void**)&dec->grabber);
    if (FAILED(hr) || !dec->grabber) goto fail;

    ZeroMemory(&mt, sizeof(mt));
    mt.majortype = MEDIATYPE_Video_;
    mt.subtype = MEDIASUBTYPE_RGB24_;
    mt.formattype = FORMAT_VideoInfo_;
    hr = ISampleGrabber_SetMediaType(dec->grabber, &mt);
    if (FAILED(hr)) {
        strcpy(step, "SampleMediaType");
        goto fail;
    }
    ISampleGrabber_SetBufferSamples(dec->grabber, TRUE);
    ISampleGrabber_SetOneShot(dec->grabber, FALSE);

    strcpy(step, "NullRenderer");
    hr = CoCreateInstance(&CLSID_NullRenderer_, NULL, CLSCTX_INPROC_SERVER, &IID_IBaseFilter_, (void**)&dec->null_filter);
    if (FAILED(hr) || !dec->null_filter) goto fail;

    strcpy(step, "AddFilters");
    hr = IGraphBuilder_AddFilter(dec->graph, dec->source_filter, L"NC-TK17-WebM LAV Source");
    if (FAILED(hr)) goto fail;
    hr = IGraphBuilder_AddFilter(dec->graph, dec->video_filter, L"NC-TK17-WebM LAV Video");
    if (FAILED(hr)) goto fail;
    hr = IGraphBuilder_AddFilter(dec->graph, dec->sample_filter, L"NC-TK17-WebM SampleGrabber");
    if (FAILED(hr)) goto fail;
    hr = IGraphBuilder_AddFilter(dec->graph, dec->null_filter, L"NC-TK17-WebM NullRenderer");
    if (FAILED(hr)) goto fail;

    strcpy(step, "ConnectSourceVideo");
    hr = connect_filters(dec->graph, dec->source_filter, dec->video_filter);
    if (FAILED(hr)) goto fail;
    strcpy(step, "ConnectVideoSample");
    hr = connect_filters(dec->graph, dec->video_filter, dec->sample_filter);
    if (FAILED(hr)) goto fail;
    strcpy(step, "ConnectSampleNull");
    sample_out = find_filter_pin(dec->sample_filter, PINDIR_OUTPUT, 1);
    null_in = find_filter_pin(dec->null_filter, PINDIR_INPUT, 1);
    if (!sample_out || !null_in) {
        hr = E_FAIL;
        goto fail;
    }
    hr = IGraphBuilder_ConnectDirect(dec->graph, sample_out, null_in, NULL);
    IPin_Release(sample_out);
    IPin_Release(null_in);
    sample_out = NULL;
    null_in = NULL;
    if (FAILED(hr)) goto fail;

    strcpy(step, "MediaControl");
    hr = IGraphBuilder_QueryInterface(dec->graph, &IID_IMediaControl_, (void**)&dec->control);
    if (FAILED(hr) || !dec->control) goto fail;
    hr = IMediaControl_Run(dec->control);
    if (FAILED(hr)) goto fail;
    Sleep(30);
    video_decoder_read_format(dec);
    dec->active = 1;
    debug_line("VideoDecoder ready path=\"%s\" size=%dx%d stride=%d", path, dec->width, dec->height, dec->stride);
    return dec;

fail:
    if (sample_out) IPin_Release(sample_out);
    if (null_in) IPin_Release(null_in);
    log_line("VideoDecoder failed step=%s hr=0x%08lx path=\"%s\"", step, (DWORD)hr, path);
    video_decoder_release(dec);
    return NULL;
}

static void video_decoder_live_queue_push(video_decoder_t *dec)
{
    int index;
    webm_live_frame_t *queued;
    BYTE *data;
    if (!dec || !dec->network_source || !dec->live_buffer_ms || !dec->async_enabled ||
        !dec->frame || dec->frame_size <= 0 || dec->width <= 0 || dec->height <= 0 || dec->stride <= 0) {
        return;
    }
    EnterCriticalSection(&dec->live_queue_lock);
    if (dec->live_queue_count >= WEBM_LIVE_QUEUE_CAPACITY) {
        dec->live_queue_head = (dec->live_queue_head + 1) % WEBM_LIVE_QUEUE_CAPACITY;
        dec->live_queue_count--;
    }
    index = (dec->live_queue_head + dec->live_queue_count) % WEBM_LIVE_QUEUE_CAPACITY;
    queued = &dec->live_queue[index];
    if (queued->capacity < (size_t)dec->frame_size) {
        data = (BYTE*)realloc(queued->data, (size_t)dec->frame_size);
        if (!data) {
            LeaveCriticalSection(&dec->live_queue_lock);
            return;
        }
        queued->data = data;
        queued->capacity = (size_t)dec->frame_size;
    }
    memcpy(queued->data, dec->frame, (size_t)dec->frame_size);
    queued->size = dec->frame_size;
    queued->width = dec->width;
    queued->height = dec->height;
    queued->stride = dec->stride;
    queued->pts_ms = dec->current_frame_ms;
    queued->frame_index = dec->decoded_frame_index;
    dec->live_queue_count++;
    LeaveCriticalSection(&dec->live_queue_lock);
}

static int video_decoder_live_prepare_frame(video_decoder_t *dec, DWORD target_ms)
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
        /* This slot has been consumed. Recycle the presentation buffer into
         * it while the producer is excluded, instead of copying its pixels.
         * Both buffers use malloc/free (the FFmpeg frame does not). */
        BYTE *spare = dec->live_present_frame;
        size_t spare_capacity = (size_t)dec->live_present_frame_size;
        dec->live_present_frame = queued->data;
        dec->live_present_frame_size = (long)queued->capacity;
        queued->data = spare;
        queued->capacity = spare_capacity;
        present_size = queued->size;
        present_width = queued->width;
        present_height = queued->height;
        present_stride = queued->stride;
        present_frame_index = queued->frame_index;
        prepared = 1;
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

static double twitch_audio_sample_as_double(const vm_AVFrame *frame, int channels,
                                            int sample, int channel)
{
    int fmt;
    int planar;
    int index;
    const unsigned char *plane;
    if (!frame || !frame->extended_data || sample < 0) return 0.0;
    if (channels < 1) channels = 1;
    if (channel >= channels) channel = channels - 1;
    fmt = frame->format;
    planar = fmt >= 5;
    plane = frame->extended_data[planar ? channel : 0];
    if (!plane && planar) plane = frame->extended_data[0];
    if (!plane) return 0.0;
    index = planar ? sample : sample * channels + channel;
    switch (fmt) {
    case 0: return ((double)((const unsigned char*)plane)[index] - 128.0) / 128.0;
    case 1: return (double)((const short*)plane)[index] / 32768.0;
    case 2: return (double)((const int*)plane)[index] / 2147483648.0;
    case 3: return (double)((const float*)plane)[index];
    case 4: return ((const double*)plane)[index];
    case 5: return ((double)((const unsigned char*)plane)[index] - 128.0) / 128.0;
    case 6: return (double)((const short*)plane)[index] / 32768.0;
    case 7: return (double)((const int*)plane)[index] / 2147483648.0;
    case 8: return (double)((const float*)plane)[index];
    case 9: return ((const double*)plane)[index];
    default: return 0.0;
    }
}

static int video_decoder_twitch_audio_queue_frame(video_decoder_t *dec, vm_AVFrame *frame)
{
    webm_twitch_audio_chunk_t *queued;
    BYTE *new_buffer;
    short *pcm;
    int channels;
    int samples;
    int bytes;
    int i;
    int index;
    double pts_ms;
    if (!dec || !frame || !dec->network_source || !dec->async_enabled ||
        !dec->async_lock_initialized || dec->audio_stream_index < 0 ||
        frame->nb_samples <= 0 || !frame->extended_data || !frame->extended_data[0]) {
        return 0;
    }
    samples = frame->nb_samples;
    channels = dec->audio_channels > 0 ? dec->audio_channels : 2;
    if (channels > 2) channels = 2;
    bytes = samples * 2 * (int)sizeof(short);
    if (bytes <= 0 || bytes > 1024 * 1024) return 0;
    if (dec->audio_convert_capacity < (size_t)bytes) {
        new_buffer = (BYTE*)realloc(dec->audio_convert_buffer, (size_t)bytes);
        if (!new_buffer) return 0;
        dec->audio_convert_buffer = new_buffer;
        dec->audio_convert_capacity = (size_t)bytes;
    }
    pcm = (short*)dec->audio_convert_buffer;
    for (i = 0; i < samples; i++) {
        double left = twitch_audio_sample_as_double(frame, channels, i, 0);
        double right = channels > 1 ? twitch_audio_sample_as_double(frame, channels, i, 1) : left;
        if (left > 1.0) left = 1.0;
        if (left < -1.0) left = -1.0;
        if (right > 1.0) right = 1.0;
        if (right < -1.0) right = -1.0;
        pcm[i * 2] = (short)(left * 32767.0);
        pcm[i * 2 + 1] = (short)(right * 32767.0);
    }
    if (frame->pts != VM_AV_NOPTS_VALUE &&
        dec->audio_time_base_num > 0 && dec->audio_time_base_den > 0) {
        pts_ms = ((double)frame->pts * (double)dec->audio_time_base_num * 1000.0) /
                 (double)dec->audio_time_base_den;
        if (!dec->audio_first_pts_set) {
            dec->audio_first_pts_ms = pts_ms;
            dec->audio_first_pts_set = 1;
        }
        pts_ms -= dec->audio_first_pts_ms;
        if (pts_ms < 0.0) pts_ms = 0.0;
    } else {
        pts_ms = dec->audio_next_pts_ms;
    }
    dec->audio_next_pts_ms = pts_ms + ((double)samples * 1000.0) /
                             (double)(dec->audio_sample_rate > 0 ? dec->audio_sample_rate : 48000);

    EnterCriticalSection(&dec->async_frame_lock);
    if (dec->audio_queue_count >= WEBM_TWITCH_AUDIO_QUEUE_CAPACITY) {
        dec->audio_queue_head = (dec->audio_queue_head + 1) % WEBM_TWITCH_AUDIO_QUEUE_CAPACITY;
        dec->audio_queue_count--;
        dec->audio_queue_dropped++;
    }
    index = (dec->audio_queue_head + dec->audio_queue_count) % WEBM_TWITCH_AUDIO_QUEUE_CAPACITY;
    queued = &dec->audio_queue[index];
    if (queued->capacity < (size_t)bytes) {
        new_buffer = (BYTE*)realloc(queued->data, (size_t)bytes);
        if (!new_buffer) {
            LeaveCriticalSection(&dec->async_frame_lock);
            return 0;
        }
        queued->data = new_buffer;
        queued->capacity = (size_t)bytes;
    }
    memcpy(queued->data, dec->audio_convert_buffer, (size_t)bytes);
    queued->bytes = bytes;
    queued->samples = samples;
    queued->sample_rate = dec->audio_sample_rate > 0 ? dec->audio_sample_rate : 48000;
    queued->pts_ms = pts_ms;
    dec->audio_queue_count++;
    LeaveCriticalSection(&dec->async_frame_lock);
    return 1;
}

static int video_decoder_twitch_audio_pop(video_decoder_t *dec, BYTE **buffer, size_t *capacity,
                                          int *bytes, int *samples, int *sample_rate,
                                          double *pts_ms)
{
    webm_twitch_audio_chunk_t *queued;
    BYTE *new_buffer;
    if (!dec || !buffer || !capacity || !bytes || !samples || !sample_rate || !pts_ms ||
        !dec->async_lock_initialized) return 0;
    EnterCriticalSection(&dec->async_frame_lock);
    if (dec->audio_queue_count <= 0) {
        LeaveCriticalSection(&dec->async_frame_lock);
        return 0;
    }
    queued = &dec->audio_queue[dec->audio_queue_head];
    if (!queued->data || queued->bytes <= 0) {
        dec->audio_queue_head = (dec->audio_queue_head + 1) % WEBM_TWITCH_AUDIO_QUEUE_CAPACITY;
        dec->audio_queue_count--;
        LeaveCriticalSection(&dec->async_frame_lock);
        return 0;
    }
    if (*capacity < (size_t)queued->bytes) {
        new_buffer = (BYTE*)realloc(*buffer, (size_t)queued->bytes);
        if (!new_buffer) {
            LeaveCriticalSection(&dec->async_frame_lock);
            return 0;
        }
        *buffer = new_buffer;
        *capacity = (size_t)queued->bytes;
    }
    memcpy(*buffer, queued->data, (size_t)queued->bytes);
    *bytes = queued->bytes;
    *samples = queued->samples;
    *sample_rate = queued->sample_rate;
    *pts_ms = queued->pts_ms;
    dec->audio_queue_head = (dec->audio_queue_head + 1) % WEBM_TWITCH_AUDIO_QUEUE_CAPACITY;
    dec->audio_queue_count--;
    LeaveCriticalSection(&dec->async_frame_lock);
    return 1;
}

static int video_decoder_grab(video_decoder_t *dec)
{
    long size = 0;
    HRESULT hr;
    if (dec && dec->use_ffmpeg) {
        int ret;
        int eof_retries = 0;
        if (!dec->active || !dec->fmt || !dec->codec || !dec->packet || !dec->src_frame) return 0;
        for (;;) {
            ret = ffmpeg_api.avcodec_receive_frame(dec->codec, dec->src_frame);
            if (ret == 0) break;
            ret = ffmpeg_api.av_read_frame(dec->fmt, dec->packet);
            if (ret < 0) {
                if (dec->network_source) {
                    LONG failures = InterlockedIncrement(&dec->live_read_failures);
                    if (failures == 1) {
                        InterlockedExchange(&dec->live_failure_tick, (LONG)GetTickCount());
                        debug_line("FFmpeg Twitch read stopped ret=%d", ret);
                    }
                }
                if (!dec->network_source && eof_retries++ < 1 && ffmpeg_api.av_seek_frame &&
                    ffmpeg_api.av_seek_frame(dec->fmt, dec->stream_index, 0, 1) >= 0) {
                    if (ffmpeg_api.avcodec_flush_buffers) ffmpeg_api.avcodec_flush_buffers(dec->codec);
                    ffmpeg_api.av_frame_unref(dec->src_frame);
                    dec->decoded_frame_index = 0;
                    dec->current_frame_ms = 0.0;
                    dec->first_frame_pts_ms = 0.0;
                    dec->first_frame_pts_set = 0;
                    dec->looped = 1;
                    debug_line("FFmpeg decoder loop path stream=%d", dec->stream_index);
                    continue;
                }
                return 0;
            }
            if (dec->network_source) {
                InterlockedExchange(&dec->live_read_failures, 0);
                InterlockedExchange(&dec->live_failure_tick, 0);
            }
            if (dec->packet->stream_index == dec->stream_index) {
                ffmpeg_api.avcodec_send_packet(dec->codec, dec->packet);
            } else if (dec->audio_codec && dec->audio_frame &&
                       dec->packet->stream_index == dec->audio_stream_index) {
                if (ffmpeg_api.avcodec_send_packet(dec->audio_codec, dec->packet) >= 0) {
                    while (ffmpeg_api.avcodec_receive_frame(dec->audio_codec, dec->audio_frame) == 0) {
                        video_decoder_twitch_audio_queue_frame(dec, dec->audio_frame);
                        ffmpeg_api.av_frame_unref(dec->audio_frame);
                    }
                }
            }
            ffmpeg_api.av_packet_unref(dec->packet);
        }
        if (dec->src_frame->width <= 0 || dec->src_frame->height <= 0 || !dec->src_frame->data[0]) {
            ffmpeg_api.av_frame_unref(dec->src_frame);
            return 0;
        }
        if (!dec->frame || dec->width != dec->src_frame->width || dec->height != dec->src_frame->height) {
            if (dec->sws) {
                ffmpeg_api.sws_freeContext(dec->sws);
                dec->sws = NULL;
            }
            if (dec->frame) {
                ffmpeg_api.av_free(dec->frame);
                dec->frame = NULL;
            }
            dec->width = dec->src_frame->width;
            dec->height = dec->src_frame->height;
            dec->rgb_stride = dec->width * 3;
            dec->frame_size = dec->rgb_stride * dec->height;
            dec->frame = (BYTE*)ffmpeg_api.av_malloc((size_t)dec->frame_size);
            dec->sws = ffmpeg_api.sws_getContext(dec->width, dec->height, dec->src_frame->format,
                                                 dec->width, dec->height, 2, 2, NULL, NULL, NULL);
            if (!dec->frame || !dec->sws) {
                ffmpeg_api.av_frame_unref(dec->src_frame);
                return 0;
            }
            dec->stride = dec->rgb_stride;
            dec->got_format = 1;
            debug_line("FFmpeg frame format size=%dx%d pixfmt=%d stride=%d", dec->width, dec->height, dec->src_frame->format, dec->rgb_stride);
        }
        {
            unsigned char *dst_data[4] = { dec->frame, NULL, NULL, NULL };
            int dst_linesize[4] = { dec->rgb_stride, 0, 0, 0 };
            ret = ffmpeg_api.sws_scale(dec->sws,
                                       (const unsigned char * const *)dec->src_frame->data,
                                       dec->src_frame->linesize,
                                       0, dec->height,
                                       dst_data, dst_linesize);
        }
        if (ret > 0) {
            if (dec->src_frame->pts != VM_AV_NOPTS_VALUE &&
                dec->time_base_num > 0 && dec->time_base_den > 0) {
                double pts_ms = ((double)dec->src_frame->pts *
                                 (double)dec->time_base_num * 1000.0) /
                                (double)dec->time_base_den;
                if (!dec->first_frame_pts_set) {
                    dec->first_frame_pts_ms = pts_ms;
                    dec->first_frame_pts_set = 1;
                    debug_line("FFmpeg timestamp baseline stream=%d pts_ms=%.3f network=%d",
                               dec->stream_index, pts_ms, dec->network_source);
                }
                dec->current_frame_ms = pts_ms - dec->first_frame_pts_ms;
                if (dec->current_frame_ms < 0.0) dec->current_frame_ms = 0.0;
            } else {
                double fps = dec->fps > 0.1 ? dec->fps : 30.0;
                dec->current_frame_ms = ((double)dec->decoded_frame_index * 1000.0) / fps;
            }
            dec->decoded_frame_index++;
            if (dec->network_source) {
                InterlockedExchange(&dec->live_last_frame_tick, (LONG)GetTickCount());
            }
            video_decoder_live_queue_push(dec);
        }
        ffmpeg_api.av_frame_unref(dec->src_frame);
        return ret > 0;
    }
    if (!dec || !dec->active || !dec->grabber) return 0;
    if (!dec->got_format) video_decoder_read_format(dec);
    hr = ISampleGrabber_GetCurrentBuffer(dec->grabber, &size, NULL);
    if (FAILED(hr) || size <= 0) return 0;
    if (size > dec->frame_size) {
        BYTE *p = (BYTE*)realloc(dec->frame, (size_t)size);
        if (!p) return 0;
        dec->frame = p;
        dec->frame_size = size;
    }
    hr = ISampleGrabber_GetCurrentBuffer(dec->grabber, &size, (long*)dec->frame);
    return SUCCEEDED(hr) && size > 0;
}

static int video_decoder_grab_to_time_sync(video_decoder_t *dec, DWORD target_ms)
{
    int decoded = 0;
    int steps = 0;
    double fps;
    if (!dec || !dec->use_ffmpeg) return video_decoder_grab(dec);
    fps = dec->fps > 0.1 ? dec->fps : 30.0;
    if (!dec->frame) {
        if (!video_decoder_grab(dec)) return 0;
        decoded = 1;
    }
    while (steps < 60) {
        double current_ms = dec->current_frame_ms;
        if (current_ms <= 0.0 && dec->decoded_frame_index > 0) {
            current_ms = ((double)dec->decoded_frame_index * 1000.0) / fps;
        }
        if (current_ms + (500.0 / fps) >= (double)target_ms) break;
        if (!video_decoder_grab(dec)) break;
        decoded = 1;
        if (dec->looped) break;
        steps++;
    }
    return decoded;
}

static int d3d8_native_format_bpp(int format)
{
    if (format == WEBM_CACHE_FORMAT_GL_RGB) return 3;
    if (format == WEBM_CACHE_FORMAT_GL_RGBA) return 4;
    if (format == D3DFMT_R5G6B5) return 2;
    if (format == D3DFMT_R8G8B8) return 3;
    if (format == D3DFMT_A8R8G8B8 || format == D3DFMT_X8R8G8B8) return 4;
    return 0;
}

static int convert_rgb24_to_d3d8_scalar(BYTE *dst_base, int dst_pitch, int format,
                                  int width, int height, const BYTE *source_frame,
                                  int source_width, int source_height, int source_stride)
{
    int x, y;
    int bpp = d3d8_native_format_bpp(format);
    int source_x_step;
    int source_x_remainder;
    if (!dst_base || !source_frame || bpp <= 0 || width <= 0 || height <= 0 ||
        source_width <= 0 || source_height <= 0 || source_stride <= 0 ||
        dst_pitch < width * bpp) {
        return 0;
    }
    source_x_step = source_width / width;
    source_x_remainder = source_width % width;
    for (y = 0; y < height; y++) {
        BYTE *dst = dst_base + (size_t)y * (size_t)dst_pitch;
        int sy = ((height - 1 - y) * source_height) / height;
        int sx = 0;
        int sx_error = 0;
        const BYTE *src_row;
        if (sy < 0) sy = 0;
        if (sy >= source_height) sy = source_height - 1;
        src_row = source_frame + (size_t)sy * (size_t)source_stride;
        for (x = 0; x < width; x++) {
            const BYTE *src;
            BYTE r, g, b;
            if (sx < 0) sx = 0;
            if (sx >= source_width) sx = source_width - 1;
            src = src_row + sx * 3;
            r = src[0];
            g = src[1];
            b = src[2];
            if (format == D3DFMT_R5G6B5) {
                WORD packed = (WORD)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
                dst[0] = (BYTE)(packed & 0xff);
                dst[1] = (BYTE)(packed >> 8);
            } else if (format == WEBM_CACHE_FORMAT_GL_RGB ||
                       format == WEBM_CACHE_FORMAT_GL_RGBA) {
                dst[0] = r;
                dst[1] = g;
                dst[2] = b;
                if (bpp == 4) dst[3] = 255;
            } else {
                dst[0] = b;
                dst[1] = g;
                dst[2] = r;
                if (bpp == 4) dst[3] = 255;
            }
            dst += bpp;
            sx += source_x_step;
            sx_error += source_x_remainder;
            if (sx_error >= width) {
                sx++;
                sx_error -= width;
            }
        }
    }
    return 1;
}

/* Preserve the scalar converter's exact nearest-neighbour coordinates,
 * vertical flip, channel order and opaque alpha. The supported texture width
 * is at most 4096; retain the original routine for larger callers. */
static int convert_rgb24_to_d3d8(BYTE *dst_base, int dst_pitch, int format,
                                  int width, int height, const BYTE *source_frame,
                                  int source_width, int source_height, int source_stride)
{
    int offsets[4096];
    int x, y, sx = 0, error = 0, previous_sy = -1;
    int step, remainder;
    int bpp = d3d8_native_format_bpp(format);
    if (!dst_base || !source_frame || bpp <= 0 || width <= 0 || height <= 0 ||
        source_width <= 0 || source_height <= 0 || source_stride <= 0 ||
        dst_pitch < width * bpp) return 0;
    if (width > (int)(sizeof(offsets) / sizeof(offsets[0]))) {
        return convert_rgb24_to_d3d8_scalar(dst_base, dst_pitch, format, width, height,
                                            source_frame, source_width, source_height,
                                            source_stride);
    }
    /* Calculate horizontal sampling once, instead of once for every row. */
    step = source_width / width;
    remainder = source_width % width;
    for (x = 0; x < width; x++) {
        offsets[x] = sx * 3;
        sx += step;
        error += remainder;
        if (error >= width) {
            sx++;
            error -= width;
        }
    }
    for (y = 0; y < height; y++) {
        BYTE *dst = dst_base + (size_t)y * (size_t)dst_pitch;
        int sy = ((height - 1 - y) * source_height) / height;
        const BYTE *row = source_frame + (size_t)sy * (size_t)source_stride;
        if (sy == previous_sy) {
            memcpy(dst, dst - dst_pitch, (size_t)width * (size_t)bpp);
            continue;
        }
        previous_sy = sy;
        /* Keep format decisions out of the pixel loop. memcpy stores permit
         * unaligned pitches without type-punning or alignment assumptions. */
        switch (format) {
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8:
            for (x = 0; x < width; x++) {
                const BYTE *src = row + offsets[x];
                DWORD pixel = 0xff000000u | ((DWORD)src[0] << 16) |
                              ((DWORD)src[1] << 8) | (DWORD)src[2];
                memcpy(dst + x * 4, &pixel, sizeof(pixel));
            }
            break;
        case WEBM_CACHE_FORMAT_GL_RGBA:
            for (x = 0; x < width; x++) {
                const BYTE *src = row + offsets[x];
                DWORD pixel = 0xff000000u | ((DWORD)src[2] << 16) |
                              ((DWORD)src[1] << 8) | (DWORD)src[0];
                memcpy(dst + x * 4, &pixel, sizeof(pixel));
            }
            break;
        case D3DFMT_R5G6B5:
            for (x = 0; x < width; x++) {
                const BYTE *src = row + offsets[x];
                WORD pixel = (WORD)(((src[0] >> 3) << 11) |
                                    ((src[1] >> 2) << 5) | (src[2] >> 3));
                memcpy(dst + x * 2, &pixel, sizeof(pixel));
            }
            break;
        case WEBM_CACHE_FORMAT_GL_RGB:
            if (width == source_width) {
                memcpy(dst, row, (size_t)width * 3u);
            } else {
                for (x = 0; x < width; x++) {
                    memcpy(dst + x * 3, row + offsets[x], 3);
                }
            }
            break;
        default: /* D3DFMT_R8G8B8 */
            for (x = 0; x < width; x++) {
                const BYTE *src = row + offsets[x];
                dst[x * 3] = src[2];
                dst[x * 3 + 1] = src[1];
                dst[x * 3 + 2] = src[0];
            }
            break;
        }
    }
    return 1;
}

static int d3d8_cache_layout(int width, int height, int format, int requested_levels,
                             size_t *offsets, int *pitches, int *widths, int *heights,
                             size_t *total_size)
{
    int level;
    int levels = requested_levels;
    int bpp = d3d8_native_format_bpp(format);
    size_t per_buffer_limit = (size_t)WEBM_D3D8_CACHE_MAX_TOTAL_BYTES / 2u;
    if (levels > WEBM_D3D8_CACHE_LEVELS) levels = WEBM_D3D8_CACHE_LEVELS;
    if (levels < 1 || bpp <= 0 || width <= 0 || height <= 0) return 0;
    while (levels > 0) {
        size_t total = 0;
        for (level = 0; level < levels; level++) {
            int level_width = width >> level;
            int level_height = height >> level;
            size_t level_size;
            if (level_width < 1) level_width = 1;
            if (level_height < 1) level_height = 1;
            level_size = (size_t)level_width * (size_t)level_height * (size_t)bpp;
            if (level_size > per_buffer_limit - total) {
                total = per_buffer_limit + 1u;
                break;
            }
            offsets[level] = total;
            pitches[level] = level_width * bpp;
            widths[level] = level_width;
            heights[level] = level_height;
            total += level_size;
        }
        if (total <= per_buffer_limit) {
            *total_size = total;
            return levels;
        }
        levels--;
    }
    return 0;
}

static int twitch_chat_render_settings_equal(const webm_twitch_settings_t *left,
                                             const webm_twitch_settings_t *right)
{
    if (!left || !right) return left == right;
    return left->chat_enabled == right->chat_enabled &&
           left->chat_position == right->chat_position &&
           left->chat_overlay == right->chat_overlay &&
           left->chat_width == right->chat_width &&
           left->chat_background_opacity == right->chat_background_opacity &&
           left->chat_text_size == right->chat_text_size &&
           left->chat_horizontal_padding == right->chat_horizontal_padding &&
           left->chat_emotes == right->chat_emotes &&
           left->chat_emote_scale == right->chat_emote_scale &&
           left->chat_animated_emotes == right->chat_animated_emotes &&
           left->chat_animated_emote_fps == right->chat_animated_emote_fps;
}

static void video_decoder_configure_d3d8_cache(video_decoder_t *dec, int width, int height,
                                                int format, int levels,
                                                webm_twitch_chat_session_t *chat,
                                                const webm_twitch_settings_t *chat_settings)
{
    int enabled;
    int actual_levels;
    int chat_changed;
    webm_twitch_chat_session_t *old_chat = NULL;
    size_t offsets[WEBM_D3D8_CACHE_LEVELS];
    int pitches[WEBM_D3D8_CACHE_LEVELS];
    int widths[WEBM_D3D8_CACHE_LEVELS];
    int heights[WEBM_D3D8_CACHE_LEVELS];
    size_t total_size = 0;
    if (!dec || !dec->async_enabled || !dec->async_lock_initialized) return;
    actual_levels = d3d8_cache_layout(width, height, format, levels, offsets, pitches,
                                      widths, heights, &total_size);
    enabled = actual_levels > 0;
    EnterCriticalSection(&dec->async_frame_lock);
    chat_changed = dec->async_d3d8_chat != chat ||
                   (chat && (!chat_settings ||
                    !twitch_chat_render_settings_equal(
                        &dec->async_d3d8_chat_settings, chat_settings)));
    if (dec->async_d3d8_cache_enabled != enabled ||
        dec->async_d3d8_cache_width != width ||
        dec->async_d3d8_cache_height != height ||
        dec->async_d3d8_cache_format != format ||
        dec->async_d3d8_cache_levels != actual_levels || chat_changed) {
        dec->async_d3d8_cache_enabled = enabled;
        dec->async_d3d8_cache_width = width;
        dec->async_d3d8_cache_height = height;
        dec->async_d3d8_cache_format = format;
        dec->async_d3d8_cache_levels = actual_levels;
        dec->async_d3d8_cache_generation++;
        dec->async_d3d8_cache_ready = 0;
        dec->async_d3d8_cache_ready_levels = 0;
        if (chat_changed) {
            old_chat = dec->async_d3d8_chat;
            dec->async_d3d8_chat = webm_twitch_chat_retain(chat);
            if (chat && chat_settings) {
                dec->async_d3d8_chat_settings = *chat_settings;
            } else {
                memset(&dec->async_d3d8_chat_settings, 0,
                       sizeof(dec->async_d3d8_chat_settings));
            }
        }
    }
    LeaveCriticalSection(&dec->async_frame_lock);
    if (old_chat) webm_twitch_chat_release(old_chat);
}

/*
 * LockRect commonly returns write-combined memory.  The normal CRT memcpy is
 * correct, but on large video frames it can pollute the CPU caches and write
 * that memory much more slowly than non-temporal stores.  Keep the optimized
 * path local to texture uploads and retain memcpy for small copies and CPUs
 * without SSE2.
 */
#if defined(__GNUC__) && (defined(__i386__) || defined(_M_IX86))
__attribute__((target("sse2"), noinline))
static void webm_copy_texture_bytes_sse2(void *destination, const void *source,
                                         size_t bytes)
{
    BYTE *dst = (BYTE*)destination;
    const BYTE *src = (const BYTE*)source;
    size_t prefix = ((size_t)0 - (size_t)dst) & 15u;
    if (prefix > bytes) prefix = bytes;
    if (prefix) {
        memcpy(dst, src, prefix);
        dst += prefix;
        src += prefix;
        bytes -= prefix;
    }
    while (bytes >= 64u) {
        __m128i a = _mm_loadu_si128((const __m128i*)(src + 0));
        __m128i b = _mm_loadu_si128((const __m128i*)(src + 16));
        __m128i c = _mm_loadu_si128((const __m128i*)(src + 32));
        __m128i d = _mm_loadu_si128((const __m128i*)(src + 48));
        _mm_stream_si128((__m128i*)(dst + 0), a);
        _mm_stream_si128((__m128i*)(dst + 16), b);
        _mm_stream_si128((__m128i*)(dst + 32), c);
        _mm_stream_si128((__m128i*)(dst + 48), d);
        dst += 64;
        src += 64;
        bytes -= 64;
    }
    while (bytes >= 16u) {
        __m128i value = _mm_loadu_si128((const __m128i*)src);
        _mm_stream_si128((__m128i*)dst, value);
        dst += 16;
        src += 16;
        bytes -= 16;
    }
    _mm_sfence();
    if (bytes) memcpy(dst, src, bytes);
}
#endif

static void webm_copy_texture_bytes(void *destination, const void *source,
                                    size_t bytes)
{
#if defined(__GNUC__) && (defined(__i386__) || defined(_M_IX86))
    static volatile LONG sse2_state;
    LONG state = InterlockedCompareExchange(&sse2_state, 0, 0);
    if (!state) {
#ifndef PF_XMMI64_INSTRUCTIONS_AVAILABLE
#define PF_XMMI64_INSTRUCTIONS_AVAILABLE 10
#endif
        state = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) ? 2 : 1;
        InterlockedCompareExchange(&sse2_state, state, 0);
    }
    if (state == 2 && bytes >= 262144u) {
        webm_copy_texture_bytes_sse2(destination, source, bytes);
        return;
    }
#endif
    memcpy(destination, source, bytes);
}

static int video_decoder_wait_d3d8_cache_writable(video_decoder_t *dec,
                                                    int buffer_index)
{
    int spins = 0;
    if (!dec || buffer_index < 0 || buffer_index >= 2) return 0;
    while (InterlockedCompareExchange(
               &dec->async_d3d8_cache_readers[buffer_index], 0, 0) != 0) {
        if (InterlockedCompareExchange(&dec->async_stop, 0, 0)) return 0;
        if (spins++ < 8) {
            SwitchToThread();
        } else {
            Sleep(1);
        }
    }
    return 1;
}

static int video_decoder_build_d3d8_cache(video_decoder_t *dec,
                                           d3d8_cache_build_t *build,
                                           const BYTE *source_frame,
                                           long source_frame_size,
                                           int source_width,
                                           int source_height,
                                           int source_stride,
                                           long frame_index)
{
    int width, height, format, requested_levels, front;
    int level, levels;
    int bpp;
    int converted = 1;
    unsigned int generation;
    size_t total_size = 0;
    BYTE *buffer;
    webm_twitch_chat_session_t *chat = NULL;
    webm_twitch_settings_t chat_settings;
    if (!dec || !build || !source_frame || source_frame_size <= 0 ||
        source_width <= 0 || source_height <= 0 || source_stride <= 0 ||
        frame_index < 0) return 0;
    memset(build, 0, sizeof(*build));
    EnterCriticalSection(&dec->async_frame_lock);
    if (!dec->async_d3d8_cache_enabled ||
        (dec->async_d3d8_cache_ready &&
         dec->async_d3d8_cache_ready_generation == dec->async_d3d8_cache_generation &&
         dec->async_d3d8_cache_frame_index == frame_index)) {
        LeaveCriticalSection(&dec->async_frame_lock);
        return 0;
    }
    width = dec->async_d3d8_cache_width;
    height = dec->async_d3d8_cache_height;
    format = dec->async_d3d8_cache_format;
    requested_levels = dec->async_d3d8_cache_levels;
    generation = dec->async_d3d8_cache_generation;
    front = dec->async_d3d8_cache_front;
    chat = webm_twitch_chat_retain(dec->async_d3d8_chat);
    if (chat) chat_settings = dec->async_d3d8_chat_settings;
    LeaveCriticalSection(&dec->async_frame_lock);

    levels = d3d8_cache_layout(width, height, format, requested_levels,
                               build->level_offset, build->level_pitch,
                               build->level_width, build->level_height, &total_size);
    if (levels <= 0 || total_size == 0) {
        if (chat) webm_twitch_chat_release(chat);
        return 0;
    }
    build->buffer_index = front ? 0 : 1;
    if (!video_decoder_wait_d3d8_cache_writable(dec, build->buffer_index)) {
        if (chat) webm_twitch_chat_release(chat);
        return 0;
    }
    if (dec->async_d3d8_cache_capacity[build->buffer_index] < total_size) {
        BYTE *new_buffer = (BYTE*)realloc(dec->async_d3d8_cache[build->buffer_index], total_size);
        if (!new_buffer) {
            if (chat) webm_twitch_chat_release(chat);
            return 0;
        }
        dec->async_d3d8_cache[build->buffer_index] = new_buffer;
        dec->async_d3d8_cache_capacity[build->buffer_index] = total_size;
    }
    buffer = dec->async_d3d8_cache[build->buffer_index];
    if (chat && chat_settings.chat_enabled) {
        webm_twitch_chat_pixel_format_t chat_format =
            format == D3DFMT_R5G6B5 ? WEBM_TWITCH_CHAT_RGB565 :
            (format == WEBM_CACHE_FORMAT_GL_RGB ||
             format == WEBM_CACHE_FORMAT_GL_RGBA) ? WEBM_TWITCH_CHAT_RGB :
                                                     WEBM_TWITCH_CHAT_BGR;
        bpp = d3d8_native_format_bpp(format);
        converted = convert_rgb24_to_d3d8(
            buffer + build->level_offset[0], build->level_pitch[0], format,
            build->level_width[0], build->level_height[0],
            source_frame, source_width, source_height, source_stride);
        if (converted) {
            webm_twitch_chat_compose(
                chat, &chat_settings,
                buffer + build->level_offset[0],
                build->level_width[0], build->level_height[0],
                build->level_pitch[0], chat_format, bpp, 1);
        }
        for (level = 1; converted && level < levels; level++) {
            webm_twitch_chat_downsample_half(
                buffer + build->level_offset[level - 1],
                build->level_width[level - 1], build->level_height[level - 1],
                build->level_pitch[level - 1],
                buffer + build->level_offset[level],
                build->level_width[level], build->level_height[level],
                build->level_pitch[level], bpp);
        }
    } else {
        for (level = 0; level < levels; level++) {
            if (!convert_rgb24_to_d3d8(buffer + build->level_offset[level],
                                       build->level_pitch[level], format,
                                       build->level_width[level], build->level_height[level],
                                       source_frame, source_width, source_height, source_stride)) {
                converted = 0;
                break;
            }
        }
    }
    if (chat) webm_twitch_chat_release(chat);
    if (!converted) return 0;
    build->generation = generation;
    build->levels = levels;
    build->ready = 1;
    return 1;
}

static int video_decoder_copy_d3d8_cached_mip(video_decoder_t *dec, UINT level,
                                               D3DLOCKED_RECT *lr, int width, int height,
                                               int format)
{
    int y;
    int copied = 0;
    int buffer_index = -1;
    int source_pitch = 0;
    int bpp = d3d8_native_format_bpp(format);
    BYTE *source = NULL;
    size_t row_size = 0;
    if (!dec || !dec->async_enabled || !lr || !lr->pBits || bpp <= 0 || level >= WEBM_D3D8_CACHE_LEVELS) {
        return 0;
    }
    EnterCriticalSection(&dec->async_frame_lock);
    if (dec->async_d3d8_cache_ready &&
        dec->async_d3d8_cache_ready_generation == dec->async_d3d8_cache_generation &&
        dec->async_d3d8_cache_frame_index == dec->async_decoded_frame_index &&
        dec->async_d3d8_cache_format == format &&
        level < (UINT)dec->async_d3d8_cache_ready_levels &&
        dec->async_d3d8_cache_level_width[level] == width &&
        dec->async_d3d8_cache_level_height[level] == height &&
        dec->async_d3d8_cache[dec->async_d3d8_cache_front] &&
        lr->Pitch >= width * bpp) {
        buffer_index = dec->async_d3d8_cache_front;
        source = dec->async_d3d8_cache[buffer_index] +
                 dec->async_d3d8_cache_level_offset[level];
        source_pitch = dec->async_d3d8_cache_level_pitch[level];
        row_size = (size_t)width * (size_t)bpp;
        InterlockedIncrement(&dec->async_d3d8_cache_readers[buffer_index]);
    }
    LeaveCriticalSection(&dec->async_frame_lock);

    if (buffer_index < 0 || !source) return 0;
    if (lr->Pitch == source_pitch && source_pitch == (int)row_size) {
        webm_copy_texture_bytes(lr->pBits, source,
                                row_size * (size_t)height);
        copied = 2;
    } else {
        for (y = 0; y < height; y++) {
            memcpy((BYTE*)lr->pBits + (size_t)y * (size_t)lr->Pitch,
                   source + (size_t)y * (size_t)source_pitch, row_size);
        }
        copied = 1;
    }
    InterlockedDecrement(&dec->async_d3d8_cache_readers[buffer_index]);
    return copied;
}

typedef struct {
    const BYTE *pixels;
    int pitch;
    int buffer_index;
} d3d8_cached_mip_view_t;

static int video_decoder_pin_d3d8_cached_mip(video_decoder_t *dec, UINT level,
                                              int width, int height, int format,
                                              d3d8_cached_mip_view_t *view)
{
    int buffer_index = -1;
    if (!view) return 0;
    memset(view, 0, sizeof(*view));
    view->buffer_index = -1;
    if (!dec || !dec->async_enabled || level >= WEBM_D3D8_CACHE_LEVELS) return 0;
    EnterCriticalSection(&dec->async_frame_lock);
    if (dec->async_d3d8_cache_ready &&
        dec->async_d3d8_cache_ready_generation == dec->async_d3d8_cache_generation &&
        dec->async_d3d8_cache_frame_index == dec->async_decoded_frame_index &&
        dec->async_d3d8_cache_format == format &&
        level < (UINT)dec->async_d3d8_cache_ready_levels &&
        dec->async_d3d8_cache_level_width[level] == width &&
        dec->async_d3d8_cache_level_height[level] == height &&
        dec->async_d3d8_cache[dec->async_d3d8_cache_front]) {
        buffer_index = dec->async_d3d8_cache_front;
        InterlockedIncrement(&dec->async_d3d8_cache_readers[buffer_index]);
        view->pixels = dec->async_d3d8_cache[buffer_index] +
                       dec->async_d3d8_cache_level_offset[level];
        view->pitch = dec->async_d3d8_cache_level_pitch[level];
        view->buffer_index = buffer_index;
    }
    LeaveCriticalSection(&dec->async_frame_lock);
    return view->pixels != NULL;
}

static void video_decoder_unpin_d3d8_cached_mip(video_decoder_t *dec,
                                                 d3d8_cached_mip_view_t *view)
{
    if (!dec || !view || view->buffer_index < 0 || view->buffer_index >= 2) return;
    InterlockedDecrement(&dec->async_d3d8_cache_readers[view->buffer_index]);
    memset(view, 0, sizeof(*view));
    view->buffer_index = -1;
}

static void video_decoder_async_publish_frame(video_decoder_t *dec, long frame_index)
{
    int cache_built = 0;
    int frame_changed;
    int index_changed;
    int frame_prepared = 0;
    d3d8_cache_build_t cache_build;
    if (!dec || !dec->frame || dec->frame_size <= 0 || frame_index < 0) return;
    cache_built = video_decoder_build_d3d8_cache(dec, &cache_build,
                                                  dec->frame, dec->frame_size,
                                                  dec->width, dec->height,
                                                  dec->stride, frame_index);
    EnterCriticalSection(&dec->async_frame_lock);
    index_changed = dec->async_decoded_frame_index != frame_index;
    frame_changed = !dec->async_frame || dec->looped ||
                    index_changed ||
                    dec->async_width != dec->width ||
                    dec->async_height != dec->height ||
                    dec->async_stride != dec->stride;
    LeaveCriticalSection(&dec->async_frame_lock);
    /* Only this presentation worker owns the spare buffer. Keep allocations
     * and full-frame copies outside the lock used by the render thread.
     * Still build/commit a changed cache when the video frame is unchanged. */
    if (frame_changed) {
        if (dec->frame_size > dec->live_present_frame_size) {
            BYTE *new_frame = (BYTE*)realloc(dec->live_present_frame, (size_t)dec->frame_size);
            if (new_frame) {
                dec->live_present_frame = new_frame;
                dec->live_present_frame_size = dec->frame_size;
            }
        }
        if (dec->live_present_frame && dec->live_present_frame_size >= dec->frame_size) {
            memcpy(dec->live_present_frame, dec->frame, (size_t)dec->frame_size);
            frame_prepared = 1;
        }
    }
    EnterCriticalSection(&dec->async_frame_lock);
    if (frame_prepared) {
        BYTE *old_frame = dec->async_frame;
        long old_capacity = dec->async_frame_size;
        dec->async_frame = dec->live_present_frame;
        dec->async_frame_size = dec->live_present_frame_size;
        dec->live_present_frame = old_frame;
        dec->live_present_frame_size = old_capacity;
    }
    if (frame_changed && !frame_prepared) {
        /* The spare is an optimization, not a playback requirement. Under
         * memory pressure retain the original locked-copy fallback. */
        if (dec->frame_size > dec->async_frame_size) {
            BYTE *new_frame = (BYTE*)realloc(dec->async_frame, (size_t)dec->frame_size);
            if (new_frame) {
                dec->async_frame = new_frame;
                dec->async_frame_size = dec->frame_size;
            }
        }
        if (dec->async_frame && dec->async_frame_size >= dec->frame_size) {
            memcpy(dec->async_frame, dec->frame, (size_t)dec->frame_size);
            frame_prepared = 1;
        }
    }
    if (frame_prepared || !frame_changed) {
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
        } else if (index_changed) {
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

static DWORD WINAPI video_decoder_async_thread(void *param)
{
    video_decoder_t *dec = (video_decoder_t*)param;
    if (!dec) return 0;
    for (;;) {
        DWORD wait_result = WaitForSingleObject(dec->async_wake_event, INFINITE);
        DWORD target_ms;
        int decoded;
        if (wait_result != WAIT_OBJECT_0 || InterlockedCompareExchange(&dec->async_stop, 0, 0)) break;
        target_ms = (DWORD)InterlockedCompareExchange(&dec->async_target_ms, 0, 0);
        if (dec->network_source && dec->live_buffer_ms && dec->live_decode_thread) {
            video_decoder_live_prepare_frame(dec, target_ms);
            continue;
        }
        if (dec->network_source && dec->live_buffer_ms) {
            video_decoder_live_prepare_frame(dec, target_ms);
        }
        decoded = video_decoder_grab_to_time_sync(dec, target_ms);
        if (dec->network_source && dec->live_buffer_ms) {
            video_decoder_live_prepare_frame(dec, target_ms);
        } else if (decoded || (dec->frame && dec->frame_size > 0)) {
            video_decoder_async_publish_frame(dec, dec->decoded_frame_index);
        }
    }
    return 0;
}

static DWORD WINAPI video_decoder_live_decode_thread(void *param)
{
    video_decoder_t *dec = (video_decoder_t*)param;
    if (!dec) return 0;
    for (;;) {
        int queue_count;
        double queue_span_ms = 0.0;
        double target_span_ms;
        if (InterlockedCompareExchange(&dec->async_stop, 0, 0)) break;

        EnterCriticalSection(&dec->live_queue_lock);
        queue_count = dec->live_queue_count;
        if (queue_count > 1) {
            int tail = (dec->live_queue_head + queue_count - 1) % WEBM_LIVE_QUEUE_CAPACITY;
            queue_span_ms = dec->live_queue[tail].pts_ms -
                            dec->live_queue[dec->live_queue_head].pts_ms;
        }
        LeaveCriticalSection(&dec->live_queue_lock);

        target_span_ms = (double)dec->live_buffer_ms + 120.0;
        if (queue_count >= WEBM_LIVE_QUEUE_CAPACITY - 2 ||
            (queue_count >= 3 && queue_span_ms >= target_span_ms)) {
            WaitForSingleObject(dec->live_decode_wake_event, 50);
            continue;
        }
        if (!video_decoder_grab(dec)) {
            if (InterlockedCompareExchange(&dec->async_stop, 0, 0)) break;
            WaitForSingleObject(dec->live_decode_wake_event, 10);
        }
    }
    return 0;
}

static int video_decoder_start_live_decode(video_decoder_t *dec)
{
    if (!dec || !dec->network_source || !dec->live_buffer_ms ||
        !dec->async_enabled || dec->live_decode_thread) {
        return dec && dec->live_decode_thread != NULL;
    }
    dec->live_decode_wake_event = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (!dec->live_decode_wake_event) return 0;
    dec->live_decode_thread = CreateThread(NULL, 0, video_decoder_live_decode_thread, dec, 0, NULL);
    if (!dec->live_decode_thread) {
        CloseHandle(dec->live_decode_wake_event);
        dec->live_decode_wake_event = NULL;
        return 0;
    }
    debug_line("FFmpeg Twitch split decode/presentation pipeline started buffer_ms=%lu",
               (unsigned long)dec->live_buffer_ms);
    return 1;
}

static int video_decoder_live_stream_failed(video_decoder_t *dec, DWORD now,
                                            DWORD playback_start_tick, DWORD timeout_ms,
                                            char *reason, size_t reason_size)
{
    LONG failures;
    DWORD failure_tick;
    DWORD last_frame_tick;
    int queue_count;
    if (reason && reason_size) reason[0] = 0;
    if (!dec || !dec->network_source) return 0;
    if (timeout_ms < 5000u) timeout_ms = 5000u;

    failures = InterlockedCompareExchange(&dec->live_read_failures, 0, 0);
    failure_tick = (DWORD)InterlockedCompareExchange(&dec->live_failure_tick, 0, 0);
    last_frame_tick = (DWORD)InterlockedCompareExchange(&dec->live_last_frame_tick, 0, 0);
    if (dec->live_queue_lock_initialized) {
        EnterCriticalSection(&dec->live_queue_lock);
        queue_count = dec->live_queue_count;
        LeaveCriticalSection(&dec->live_queue_lock);
    } else {
        queue_count = 0;
    }

    if (queue_count > 0) return 0;
    if (failures >= 3 && failure_tick && (DWORD)(now - failure_tick) >= 750u) {
        if (reason && reason_size) {
            _snprintf(reason, reason_size - 1, "Twitch stream ended (read failures=%ld)", failures);
            reason[reason_size - 1] = 0;
        }
        return 1;
    }
    if (last_frame_tick && (DWORD)(now - last_frame_tick) >= timeout_ms) {
        if (reason && reason_size) {
            _snprintf(reason, reason_size - 1, "Twitch stream stalled for %lu ms",
                      (unsigned long)(now - last_frame_tick));
            reason[reason_size - 1] = 0;
        }
        return 1;
    }
    if (!last_frame_tick && playback_start_tick &&
        (DWORD)(now - playback_start_tick) >= timeout_ms) {
        if (reason && reason_size) {
            _snprintf(reason, reason_size - 1, "Twitch stream produced no frames for %lu ms",
                      (unsigned long)(now - playback_start_tick));
            reason[reason_size - 1] = 0;
        }
        return 1;
    }
    return 0;
}

static int video_decoder_start_async(video_decoder_t *dec)
{
    if (!dec || !dec->use_ffmpeg || dec->async_enabled) return dec && dec->async_enabled;
    InitializeCriticalSection(&dec->async_frame_lock);
    dec->async_lock_initialized = 1;
    InitializeCriticalSection(&dec->live_queue_lock);
    dec->live_queue_lock_initialized = 1;
    dec->async_wake_event = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (!dec->async_wake_event) {
        DeleteCriticalSection(&dec->live_queue_lock);
        dec->live_queue_lock_initialized = 0;
        DeleteCriticalSection(&dec->async_frame_lock);
        dec->async_lock_initialized = 0;
        return 0;
    }
    dec->async_enabled = 1;
    dec->async_thread = CreateThread(NULL, 0, video_decoder_async_thread, dec, 0, NULL);
    if (!dec->async_thread) {
        dec->async_enabled = 0;
        CloseHandle(dec->async_wake_event);
        dec->async_wake_event = NULL;
        DeleteCriticalSection(&dec->live_queue_lock);
        dec->live_queue_lock_initialized = 0;
        DeleteCriticalSection(&dec->async_frame_lock);
        dec->async_lock_initialized = 0;
        return 0;
    }
    return 1;
}

static void video_decoder_stop_async(video_decoder_t *dec)
{
    if (!dec || !dec->async_enabled) return;
    InterlockedExchange(&dec->async_stop, 1);
    if (dec->async_wake_event) SetEvent(dec->async_wake_event);
    if (dec->live_decode_wake_event) SetEvent(dec->live_decode_wake_event);
    if (dec->async_thread) {
        WaitForSingleObject(dec->async_thread, INFINITE);
        CloseHandle(dec->async_thread);
        dec->async_thread = NULL;
    }
    if (dec->live_decode_thread) {
        WaitForSingleObject(dec->live_decode_thread, INFINITE);
        CloseHandle(dec->live_decode_thread);
        dec->live_decode_thread = NULL;
    }
    if (dec->async_wake_event) {
        CloseHandle(dec->async_wake_event);
        dec->async_wake_event = NULL;
    }
    if (dec->live_decode_wake_event) {
        CloseHandle(dec->live_decode_wake_event);
        dec->live_decode_wake_event = NULL;
    }
    dec->async_enabled = 0;
    if (dec->async_lock_initialized) {
        DeleteCriticalSection(&dec->async_frame_lock);
        dec->async_lock_initialized = 0;
    }
    if (dec->live_queue_lock_initialized) {
        DeleteCriticalSection(&dec->live_queue_lock);
        dec->live_queue_lock_initialized = 0;
    }
}

static int video_decoder_grab_to_time(video_decoder_t *dec, DWORD target_ms)
{
    if (!dec || !dec->async_enabled) return video_decoder_grab_to_time_sync(dec, target_ms);
    InterlockedExchange(&dec->async_target_ms, (LONG)target_ms);
    SetEvent(dec->async_wake_event);
    return 0;
}

static int video_decoder_has_frame(video_decoder_t *dec)
{
    int have_frame;
    if (!dec) return 0;
    if (!dec->async_enabled) return dec->frame && dec->frame_size > 0;
    EnterCriticalSection(&dec->async_frame_lock);
    have_frame = dec->async_frame && dec->async_frame_size > 0 &&
                 dec->async_width > 0 && dec->async_height > 0 && dec->async_stride > 0;
    LeaveCriticalSection(&dec->async_frame_lock);
    return have_frame;
}

static long video_decoder_frame_index(video_decoder_t *dec)
{
    long frame_index;
    if (!dec) return -1;
    if (!dec->async_enabled) return dec->decoded_frame_index;
    EnterCriticalSection(&dec->async_frame_lock);
    frame_index = dec->async_decoded_frame_index;
    LeaveCriticalSection(&dec->async_frame_lock);
    return frame_index;
}

static int video_decoder_take_looped(video_decoder_t *dec)
{
    int looped;
    if (!dec) return 0;
    if (!dec->async_enabled) {
        looped = dec->looped;
        dec->looped = 0;
        return looped;
    }
    EnterCriticalSection(&dec->async_frame_lock);
    looped = dec->async_looped;
    dec->async_looped = 0;
    LeaveCriticalSection(&dec->async_frame_lock);
    return looped;
}

static void add_optional_lav_filter(void *graph, const char *ax_name, REFGUID clsid, LPCWSTR graph_name)
{
    void *filter = NULL;
    HRESULT hr = create_lav_filter_object(ax_name, clsid, &filter);
    if (SUCCEEDED(hr) && filter) {
        HRESULT add_hr = ((graph_addfilter_t)graph_builder_orig_vt[3])(graph, filter, graph_name);
        if (FAILED(add_hr)) {
            log_line("LAV optional AddFilter failed name=\"%S\" hr=0x%08lx", graph_name, (DWORD)add_hr);
        }
        {
            DWORD **vt = *(DWORD***)filter;
            ((com_ref_t)vt[2])(filter);
        }
    } else {
        log_line("LAV optional create failed name=\"%S\" hr=0x%08lx", graph_name, (DWORD)hr);
    }
}

static FARPROC load_ffmpeg_proc(HMODULE mod, const char *name)
{
    FARPROC p = mod ? GetProcAddress(mod, name) : NULL;
    if (!p) log_line("FFmpeg missing export %s", name);
    return p;
}

static int ffmpeg_load_api(void)
{
    char lav_dir[MAX_PATH * 2];
    if (ffmpeg_api.loaded) return 1;
    if (ffmpeg_api.failed) return 0;
    webm_component_dir_a(lav_dir, sizeof(lav_dir),
                         "NC-TK17-WebM-lav");
    if (!lav_dir[0]) {
        ffmpeg_api.failed = 1;
        return 0;
    }
    SetDllDirectoryA(lav_dir);
    ffmpeg_api.avutil = LoadLibraryA("avutil-lav-60.dll");
    ffmpeg_api.avcodec = LoadLibraryA("avcodec-lav-62.dll");
    ffmpeg_api.avformat = LoadLibraryA("avformat-lav-62.dll");
    ffmpeg_api.swscale = LoadLibraryA("swscale-lav-9.dll");
    SetDllDirectoryA(NULL);
    if (!ffmpeg_api.avutil || !ffmpeg_api.avcodec || !ffmpeg_api.avformat || !ffmpeg_api.swscale) {
        log_line("FFmpeg LoadLibrary failed avutil=%p avcodec=%p avformat=%p swscale=%p gle=%lu",
                 ffmpeg_api.avutil, ffmpeg_api.avcodec, ffmpeg_api.avformat, ffmpeg_api.swscale, GetLastError());
        ffmpeg_api.failed = 1;
        return 0;
    }

#define LOAD_FF(field, mod) ffmpeg_api.field = (field##_t)load_ffmpeg_proc(ffmpeg_api.mod, #field)
    LOAD_FF(avformat_open_input, avformat);
    LOAD_FF(avformat_find_stream_info, avformat);
    LOAD_FF(av_find_best_stream, avformat);
    LOAD_FF(av_read_frame, avformat);
    LOAD_FF(av_seek_frame, avformat);
    LOAD_FF(av_guess_frame_rate, avformat);
    LOAD_FF(avformat_alloc_output_context2, avformat);
    LOAD_FF(avformat_new_stream, avformat);
    LOAD_FF(avformat_write_header, avformat);
    LOAD_FF(av_interleaved_write_frame, avformat);
    LOAD_FF(av_write_trailer, avformat);
    LOAD_FF(avformat_free_context, avformat);
    LOAD_FF(avio_open, avformat);
    LOAD_FF(avio_closep, avformat);
    LOAD_FF(avformat_close_input, avformat);
    LOAD_FF(avcodec_parameters_copy, avcodec);
    LOAD_FF(avcodec_find_decoder, avcodec);
    LOAD_FF(avcodec_alloc_context3, avcodec);
    LOAD_FF(avcodec_parameters_to_context, avcodec);
    LOAD_FF(avcodec_open2, avcodec);
    LOAD_FF(avcodec_send_packet, avcodec);
    LOAD_FF(avcodec_receive_frame, avcodec);
    LOAD_FF(avcodec_flush_buffers, avcodec);
    LOAD_FF(avcodec_free_context, avcodec);
    LOAD_FF(av_packet_alloc, avcodec);
    LOAD_FF(av_packet_free, avcodec);
    LOAD_FF(av_packet_unref, avcodec);
    LOAD_FF(av_frame_alloc, avutil);
    LOAD_FF(av_frame_free, avutil);
    LOAD_FF(av_frame_unref, avutil);
    LOAD_FF(av_malloc, avutil);
    LOAD_FF(av_free, avutil);
    ffmpeg_api.av_opt_get_int = (av_opt_get_int_t)GetProcAddress(ffmpeg_api.avutil, "av_opt_get_int");
    ffmpeg_api.av_log_set_level = (av_log_set_level_t)GetProcAddress(ffmpeg_api.avutil, "av_log_set_level");
    ffmpeg_api.av_log_default_callback =
        (av_log_callback_t)GetProcAddress(ffmpeg_api.avutil, "av_log_default_callback");
    ffmpeg_api.av_log_set_callback =
        (av_log_set_callback_t)GetProcAddress(ffmpeg_api.avutil, "av_log_set_callback");
    if (ffmpeg_api.av_log_set_callback && ffmpeg_api.av_log_default_callback) {
        ffmpeg_api.av_log_set_callback(webm_ffmpeg_log_callback);
    }
    if (ffmpeg_api.av_log_set_level) {
        ffmpeg_api.av_log_set_level(debug_logging ? VM_AV_LOG_INFO : VM_AV_LOG_WARNING);
    }
    LOAD_FF(sws_getContext, swscale);
    LOAD_FF(sws_scale, swscale);
    LOAD_FF(sws_freeContext, swscale);
#undef LOAD_FF

    if (!ffmpeg_api.avformat_open_input || !ffmpeg_api.avformat_find_stream_info ||
        !ffmpeg_api.av_find_best_stream || !ffmpeg_api.av_read_frame ||
        !ffmpeg_api.avformat_alloc_output_context2 || !ffmpeg_api.avformat_new_stream ||
        !ffmpeg_api.avformat_write_header || !ffmpeg_api.av_interleaved_write_frame ||
        !ffmpeg_api.av_write_trailer || !ffmpeg_api.avformat_free_context ||
        !ffmpeg_api.avio_open || !ffmpeg_api.avio_closep ||
        !ffmpeg_api.avformat_close_input || !ffmpeg_api.avcodec_find_decoder ||
        !ffmpeg_api.avcodec_parameters_copy ||
        !ffmpeg_api.avcodec_alloc_context3 || !ffmpeg_api.avcodec_parameters_to_context ||
        !ffmpeg_api.avcodec_open2 || !ffmpeg_api.avcodec_send_packet ||
        !ffmpeg_api.avcodec_receive_frame || !ffmpeg_api.avcodec_free_context ||
        !ffmpeg_api.av_packet_alloc || !ffmpeg_api.av_packet_free ||
        !ffmpeg_api.av_packet_unref || !ffmpeg_api.av_frame_alloc ||
        !ffmpeg_api.av_frame_free || !ffmpeg_api.av_frame_unref ||
        !ffmpeg_api.av_malloc || !ffmpeg_api.av_free ||
        !ffmpeg_api.sws_getContext || !ffmpeg_api.sws_scale || !ffmpeg_api.sws_freeContext) {
        ffmpeg_api.failed = 1;
        return 0;
    }
    ffmpeg_api.loaded = 1;
    debug_line("FFmpeg API loaded from \"%s\"", lav_dir);
    return 1;
}

static video_decoder_t *ffmpeg_decoder_create(const char *path)
{
    video_decoder_t *dec;
    const char *log_path;
    vm_AVFormatContext *fmt_public;
    vm_AVStream *stream = NULL;
    vm_AVStream *audio_stream = NULL;
    const AVCodec *best_decoder = NULL;
    const AVCodec *best_audio_decoder = NULL;
    AVCodec *decoder = NULL;
    AVCodec *audio_decoder = NULL;
    long long audio_sample_rate = 0;
    long long audio_channels = 0;
    int ret;
    if (!path || !path[0] || !ffmpeg_load_api()) return NULL;
    dec = (video_decoder_t*)calloc(1, sizeof(*dec));
    if (!dec) return NULL;
    dec->audio_stream_index = -1;
    dec->live_presented_frame_index = -1;
    dec->network_source = (_strnicmp(path, "http://", 7) == 0 ||
                           _strnicmp(path, "https://", 8) == 0);
    log_path = dec->network_source ? "<network stream>" : path;
    ret = ffmpeg_api.avformat_open_input(&dec->fmt, path, NULL, NULL);
    if (ret < 0 || !dec->fmt) {
        log_line("FFmpeg open failed ret=%d path=\"%s\"", ret, log_path);
        video_decoder_release(dec);
        return NULL;
    }
    ret = ffmpeg_api.avformat_find_stream_info(dec->fmt, NULL);
    if (ret < 0) {
        log_line("FFmpeg stream info failed ret=%d path=\"%s\"", ret, log_path);
        video_decoder_release(dec);
        return NULL;
    }
    ret = ffmpeg_api.av_find_best_stream(dec->fmt, 0, -1, -1, &best_decoder, 0);
    if (ret < 0) {
        log_line("FFmpeg video stream not found ret=%d path=\"%s\"", ret, log_path);
        video_decoder_release(dec);
        return NULL;
    }
    dec->stream_index = ret;
    fmt_public = (vm_AVFormatContext*)dec->fmt;
    if (!fmt_public->streams || dec->stream_index < 0 || (unsigned int)dec->stream_index >= fmt_public->nb_streams) {
        log_line("FFmpeg stream layout invalid stream=%d nb=%u path=\"%s\"",
                 dec->stream_index, fmt_public->nb_streams, log_path);
        video_decoder_release(dec);
        return NULL;
    }
    stream = fmt_public->streams[dec->stream_index];
    if (!stream || !stream->codecpar) {
        log_line("FFmpeg stream codecpar missing stream=%d path=\"%s\"", dec->stream_index, log_path);
        video_decoder_release(dec);
        return NULL;
    }
    decoder = best_decoder ? (AVCodec*)best_decoder : ffmpeg_api.avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        log_line("FFmpeg decoder missing codec=%d path=\"%s\"", stream->codecpar->codec_id, log_path);
        video_decoder_release(dec);
        return NULL;
    }
    dec->codec = ffmpeg_api.avcodec_alloc_context3(decoder);
    if (!dec->codec) {
        video_decoder_release(dec);
        return NULL;
    }
    ret = ffmpeg_api.avcodec_parameters_to_context(dec->codec, stream->codecpar);
    if (ret < 0) {
        log_line("FFmpeg parameters_to_context failed ret=%d path=\"%s\"", ret, log_path);
        video_decoder_release(dec);
        return NULL;
    }
    ret = ffmpeg_api.avcodec_open2(dec->codec, decoder, NULL);
    if (ret < 0) {
        log_line("FFmpeg codec open failed ret=%d path=\"%s\"", ret, log_path);
        video_decoder_release(dec);
        return NULL;
    }
    if (dec->network_source) {
        ret = ffmpeg_api.av_find_best_stream(dec->fmt, 1, -1, dec->stream_index,
                                             &best_audio_decoder, 0);
        if (ret >= 0 && fmt_public->streams && (unsigned int)ret < fmt_public->nb_streams) {
            audio_stream = fmt_public->streams[ret];
        }
        if (audio_stream && audio_stream->codecpar) {
            audio_decoder = best_audio_decoder ? (AVCodec*)best_audio_decoder :
                            ffmpeg_api.avcodec_find_decoder(audio_stream->codecpar->codec_id);
            if (audio_decoder) {
                dec->audio_codec = ffmpeg_api.avcodec_alloc_context3(audio_decoder);
            }
            if (dec->audio_codec &&
                ffmpeg_api.avcodec_parameters_to_context(dec->audio_codec, audio_stream->codecpar) >= 0 &&
                ffmpeg_api.avcodec_open2(dec->audio_codec, audio_decoder, NULL) >= 0) {
                dec->audio_frame = ffmpeg_api.av_frame_alloc();
                if (dec->audio_frame) {
                    dec->audio_stream_index = ret;
                    dec->audio_time_base_num = audio_stream->time_base.num;
                    dec->audio_time_base_den = audio_stream->time_base.den;
                    if (ffmpeg_api.av_opt_get_int) {
                        ffmpeg_api.av_opt_get_int(dec->audio_codec, "sample_rate", 0, &audio_sample_rate);
                        ffmpeg_api.av_opt_get_int(dec->audio_codec, "channels", 0, &audio_channels);
                    }
                    dec->audio_sample_rate = (audio_sample_rate >= 8000 && audio_sample_rate <= 192000) ?
                                             (int)audio_sample_rate : 48000;
                    dec->audio_channels = (audio_channels >= 1 && audio_channels <= 2) ?
                                          (int)audio_channels : 2;
                    debug_line("FFmpeg Twitch audio ready stream=%d codec=%d hz=%d channels=%d time_base=%d/%d",
                               dec->audio_stream_index, audio_stream->codecpar->codec_id,
                               dec->audio_sample_rate, dec->audio_channels,
                               dec->audio_time_base_num, dec->audio_time_base_den);
                }
            }
            if (dec->audio_stream_index < 0) {
                if (dec->audio_frame) ffmpeg_api.av_frame_free(&dec->audio_frame);
                if (dec->audio_codec) ffmpeg_api.avcodec_free_context(&dec->audio_codec);
                log_line("FFmpeg Twitch audio decoder unavailable; continuing video-only path=\"%s\"", log_path);
            }
        } else {
            log_line("FFmpeg Twitch audio stream not found; continuing video-only path=\"%s\"", log_path);
        }
    }
    dec->packet = ffmpeg_api.av_packet_alloc();
    dec->src_frame = ffmpeg_api.av_frame_alloc();
    if (!dec->packet || !dec->src_frame) {
        video_decoder_release(dec);
        return NULL;
    }
    dec->use_ffmpeg = 1;
    dec->active = 1;
    dec->fps = 30.0;
    dec->time_base_num = stream->time_base.num;
    dec->time_base_den = stream->time_base.den;
    if (dec->time_base_num <= 0 || dec->time_base_den <= 0) {
        dec->time_base_num = 0;
        dec->time_base_den = 0;
    }
    if (ffmpeg_api.av_guess_frame_rate) {
        vm_AVRational rate = ffmpeg_api.av_guess_frame_rate(dec->fmt, stream, NULL);
        if (rate.num > 0 && rate.den > 0) {
            dec->fps = (double)rate.num / (double)rate.den;
        }
    }
    if (dec->fps < 1.0 || dec->fps > 240.0) dec->fps = 30.0;
    debug_line("FFmpeg decoder ready stream=%d codec=%d fps=%.3f time_base=%d/%d duration=%lld frames=%lld path=\"%s\"",
             dec->stream_index, stream->codecpar->codec_id, dec->fps,
             dec->time_base_num, dec->time_base_den, stream->duration, stream->nb_frames, log_path);
    if (async_decoding) {
        if (video_decoder_start_async(dec)) {
            debug_line("FFmpeg asynchronous decoder started path=\"%s\"", log_path);
        } else {
            log_line("FFmpeg asynchronous decoder unavailable; using synchronous decoding path=\"%s\"", log_path);
        }
    }
    return dec;
}

static HRESULT WINAPI hook_graph_AddSourceFilter(void *self, LPCWSTR file, LPCWSTR filter_name, void **filter_out)
{
    HRESULT hr;
    char file_mb[MAX_PATH * 4];
    char name_mb[MAX_PATH * 4];
    wide_to_mb(file, file_mb, sizeof(file_mb));
    wide_to_mb(filter_name, name_mb, sizeof(name_mb));

    if (file && ends_with_w_i(file, L".webm.avi")) {
        void *filter = NULL;
        hr = create_lav_source_filter_for_file(file, &filter);
        if (SUCCEEDED(hr) && filter) {
            HRESULT add_hr = ((graph_addfilter_t)graph_builder_orig_vt[3])(self, filter, filter_name ? filter_name : L"LAV Splitter Source");
            if (SUCCEEDED(add_hr)) {
                add_optional_lav_filter(self, "LAVVideo.ax", &CLSID_LAVVideoDecoder_, L"LAV Video Decoder");
                add_optional_lav_filter(self, "LAVAudio.ax", &CLSID_LAVAudioDecoder_, L"LAV Audio Decoder");
                debug_line("WebM graph prepared \"%s\"", file_mb);
                if (filter_out) *filter_out = filter;
                return S_OK;
            }
            log_line("IGraphBuilder::AddSourceFilter local LAV AddFilter failed hr=0x%08lx", (DWORD)add_hr);
            {
                DWORD **vt = *(DWORD***)filter;
                ((com_ref_t)vt[2])(filter);
            }
            hr = add_hr;
        }
        log_line("IGraphBuilder::AddSourceFilter local LAV failed hr=0x%08lx", (DWORD)hr);
    }

    hr = ((graph_addsourcefilter_t)graph_builder_orig_vt[14])(self, file, filter_name, filter_out);
    if (file && ends_with_w_i(file, L".webm.avi")) {
        log_line("IGraphBuilder::AddSourceFilter original webm hr=0x%08lx filter=%p", (DWORD)hr, filter_out ? *filter_out : NULL);
    }
    return hr;
}

static void hook_graph_builder_object(void *obj)
{
    DWORD *vt;
    if (!obj) return;
    vt = *(DWORD**)obj;
    if (!vt) return;
    if (!graph_builder_vt_ready) {
        int i;
        for (i = 0; i < 18; i++) {
            graph_builder_orig_vt[i] = vt[i];
            graph_builder_hook_vt[i] = vt[i];
        }
        graph_builder_hook_vt[13] = (DWORD)hook_graph_RenderFile;
        graph_builder_hook_vt[14] = (DWORD)hook_graph_AddSourceFilter;
        graph_builder_vt_ready = 1;
    }
    *(DWORD**)obj = graph_builder_hook_vt;
}

static HRESULT WINAPI hook_CoCreateInstance(REFCLSID rclsid, LPUNKNOWN outer, DWORD context, REFIID riid, LPVOID *ppv)
{
    HRESULT hr;
    char clsid_text[64];
    char iid_text[64];
    guid_to_text(rclsid, clsid_text, sizeof(clsid_text));
    guid_to_text(riid, iid_text, sizeof(iid_text));
    hr = real_CoCreateInstance(rclsid, outer, context, riid, ppv);
    if (guid_equal(rclsid, &CLSID_FilterGraph_) || guid_equal(riid, &IID_IGraphBuilder_)) {
        if (FAILED(hr)) {
            log_line("CoCreateInstance graph failed clsid=%s iid=%s hr=0x%08lx",
                     clsid_text, iid_text, (DWORD)hr);
        }
    }
    if (SUCCEEDED(hr) && ppv && *ppv && guid_equal(riid, &IID_IGraphBuilder_)) {
        hook_graph_builder_object(*ppv);
    }
    return hr;
}

static int contains_i(const char *s, const char *needle)
{
    return find_i(s, needle) != NULL;
}

static const char *find_i(const char *s, const char *needle)
{
    size_t nl;
    if (!s || !needle) return NULL;
    nl = strlen(needle);
    if (!nl) return s;
    for (; *s; s++) {
        if (_strnicmp(s, needle, nl) == 0) return s;
    }
    return NULL;
}

static int is_mod_texture_path_a(const char *s)
{
    if (!s) return 0;
    return contains_i(s, "ActiveMod") ||
           contains_i(s, "Addons") ||
           contains_i(s, "Shared/Cloth") ||
           contains_i(s, "Shared\\Cloth");
}

static int is_texture_probe_name_a(const char *s)
{
    const char *dot;
    if (!s) return 0;
    dot = strrchr(s, '.');
    if (!dot) return strchr(s, '*') || strchr(s, '?');
    return _stricmp(dot, ".png") == 0 ||
           _stricmp(dot, ".jp2") == 0 ||
           _stricmp(dot, ".j2k") == 0 ||
           _stricmp(dot, ".webm") == 0 ||
           _stricmp(dot, ".avi") == 0 ||
           strchr(s, '*') || strchr(s, '?');
}

static int is_exact_texture_probe_name_a(const char *s)
{
    const char *dot;
    if (!s || strchr(s, '*') || strchr(s, '?')) return 0;
    dot = strrchr(s, '.');
    if (!dot) return 0;
    return _stricmp(dot, ".png") == 0 ||
           _stricmp(dot, ".jp2") == 0 ||
           _stricmp(dot, ".j2k") == 0 ||
           _stricmp(dot, ".webm") == 0 ||
           _stricmp(dot, ".avi") == 0;
}

static const char *active_mod_tail_a(const char *path)
{
    const char *p;
    static const char marker1[] = "Mod\\ActiveMod\\";
    static const char marker2[] = "Mod/ActiveMod/";
    if (!path) return NULL;
    for (p = path; *p; p++) {
        if (_strnicmp(p, marker1, sizeof(marker1) - 1) == 0) return p + sizeof(marker1) - 1;
        if (_strnicmp(p, marker2, sizeof(marker2) - 1) == 0) return p + sizeof(marker2) - 1;
    }
    return NULL;
}

static int active_mod_root_dir_a(const char *path, char *out, size_t outsz)
{
    const char *tail;
    const char *end;
    size_t n;
    if (out && outsz) out[0] = 0;
    if (!path || !out || !outsz) return 0;
    tail = active_mod_tail_a(path);
    if (!tail || !*tail) return 0;
    end = tail;
    while (*end && *end != '\\' && *end != '/') end++;
    if (!*end) return 0;
    n = (size_t)(end - path);
    if (!n || n >= outsz) return 0;
    memcpy(out, path, n);
    out[n] = 0;
    return 1;
}

static int active_mod_root_has_file_a(const char *path, const char *name)
{
    char root[MAX_PATH * 4];
    char probe[MAX_PATH * 4];
    DWORD attr;
    if (!active_mod_root_dir_a(path, root, sizeof(root))) return 0;
    path_join(probe, sizeof(probe), root, name);
    attr = GetFileAttributesA(probe);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

static int active_mod_texture_video_allowed_a(const char *path)
{
    if (!contains_i(path, "Mod\\ActiveMod\\") && !contains_i(path, "Mod/ActiveMod/")) return 1;
    return contains_i(path, "\\Luder\\Room\\") ||
           contains_i(path, "/Luder/Room/") ||
           contains_i(path, "\\Luder\\Item\\") ||
           contains_i(path, "/Luder/Item/") ||
           contains_i(path, "\\Shared\\Item\\") ||
           contains_i(path, "/Shared/Item/") ||
           active_mod_root_has_file_a(path, "_level_definition.txt");
}

static void sidecar_ini_path_a(const char *sidecar, char *out, size_t outsz);

static void normalize_override_path_a(const char *path, char *out, size_t outsz)
{
    size_t i;
    if (!out || !outsz) return;
    out[0] = 0;
    if (!path) return;
    lstrcpynA(out, path, (int)outsz);
    for (i = 0; out[i]; i++) {
        if (out[i] == '\\') out[i] = '/';
        else out[i] = (char)tolower((unsigned char)out[i]);
    }
}

static int twitch_override_uses_auto_room_a(void)
{
    return _stricmp(twitch_override_target, TWITCH_OVERRIDE_AUTO_ROOM) == 0;
}

static int room_name_from_path_a(const char *path, char *out, size_t outsz)
{
    static const char slash_marker[] = "/Luder/Room/";
    static const char backslash_marker[] = "\\Luder\\Room\\";
    const char *room;
    const char *end;
    size_t length;
    size_t i;
    if (out && outsz) out[0] = 0;
    if (!path || !out || outsz < 2) return 0;
    room = find_i(path, slash_marker);
    if (room) {
        room += sizeof(slash_marker) - 1;
    } else {
        room = find_i(path, backslash_marker);
        if (!room) return 0;
        room += sizeof(backslash_marker) - 1;
    }
    end = room;
    while (*end && *end != '/' && *end != '\\') end++;
    length = (size_t)(end - room);
    if (!length || length >= outsz) return 0;
    for (i = 0; i < length; i++) {
        out[i] = (char)tolower((unsigned char)room[i]);
    }
    out[length] = 0;
    return 1;
}

static void twitch_override_auto_advance_generation_a(void)
{
    config_generation++;
    if (!config_generation) config_generation = 1;
}

static void twitch_override_auto_set_room_a(const char *room)
{
    int changed;
    if (!room || !room[0]) return;
    changed = _stricmp(twitch_override_auto_room, room) != 0;
    if (!changed) return;
    if (twitch_override_auto_room[0] || twitch_override_auto_target[0]) {
        log_line("Twitch override auto room changed old=\"%s\" new=\"%s\"; target cleared",
                 twitch_override_auto_room, room);
    }
    lstrcpynA(twitch_override_auto_room, room, sizeof(twitch_override_auto_room));
    twitch_override_auto_target[0] = 0;
    twitch_override_auto_advance_generation_a();
}

static void twitch_override_auto_observe_sidecar_a(const char *sidecar_ini)
{
    char room[MAX_PATH];
    if (!twitch_override_uses_auto_room_a() || !sidecar_ini || !sidecar_ini[0] ||
        !room_name_from_path_a(sidecar_ini, room, sizeof(room))) {
        return;
    }
    twitch_override_auto_set_room_a(room);
    if (twitch_override_auto_target[0] ||
        !webm_twitch_sidecar_is_compatible(sidecar_ini)) {
        return;
    }
    lstrcpynA(twitch_override_auto_target, sidecar_ini,
              sizeof(twitch_override_auto_target));
    twitch_override_auto_advance_generation_a();
    log_line("Twitch override auto assigned room=\"%s\" sidecar=\"%s\"",
             twitch_override_auto_room, twitch_override_auto_target);
}

static void twitch_override_auto_observe_texture_a(const char *texture_path)
{
    char sidecar_ini[MAX_PATH * 4];
    DWORD attributes;
    if (!twitch_override_uses_auto_room_a() || !texture_path) {
        return;
    }
    if (twitch_override_auto_target[0]) return;
    sidecar_ini_path_a(texture_path, sidecar_ini, sizeof(sidecar_ini));
    attributes = GetFileAttributesA(sidecar_ini);
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return;
    }
    twitch_override_auto_observe_sidecar_a(sidecar_ini);
}

static int normalized_paths_match_a(const char *left, const char *right)
{
    char normalized_left[MAX_PATH * 4];
    char normalized_right[MAX_PATH * 4];
    if (!left || !left[0] || !right || !right[0]) return 0;
    normalize_override_path_a(left, normalized_left, sizeof(normalized_left));
    normalize_override_path_a(right, normalized_right, sizeof(normalized_right));
    return strcmp(normalized_left, normalized_right) == 0;
}

static void twitch_override_auto_release_sidecar_a(const char *sidecar_ini)
{
    if (!twitch_override_uses_auto_room_a() || !twitch_override_auto_target[0] ||
        !normalized_paths_match_a(twitch_override_auto_target, sidecar_ini)) {
        return;
    }
    log_line("Twitch override auto target unloaded room=\"%s\" sidecar=\"%s\"; target cleared",
             twitch_override_auto_room, twitch_override_auto_target);
    twitch_override_auto_room[0] = 0;
    twitch_override_auto_target[0] = 0;
    twitch_override_auto_advance_generation_a();
}

static void twitch_override_auto_reset_a(void)
{
    twitch_override_auto_room[0] = 0;
    twitch_override_auto_target[0] = 0;
}

static void twitch_override_auto_seed_active_a(void)
{
    int i;
    if (!twitch_override_uses_auto_room_a() || twitch_override_auto_target[0]) return;
    for (i = 0; i < video_d3d8_active_count; i++) {
        video_d3d8_texture_t *slot = video_d3d8_active_slots[i];
        if (slot && slot->active && slot->sidecar_ini_path[0]) {
            twitch_override_auto_observe_sidecar_a(slot->sidecar_ini_path);
            if (twitch_override_auto_target[0]) return;
        }
    }
    for (i = 0; i < video_gl_active_count; i++) {
        video_gl_texture_t *slot = video_gl_active_slots[i];
        if (slot && slot->active && slot->sidecar_ini_path[0]) {
            twitch_override_auto_observe_sidecar_a(slot->sidecar_ini_path);
            if (twitch_override_auto_target[0]) return;
        }
    }
}

static int twitch_override_matches_sidecar_a(const char *sidecar_ini)
{
    const char *selected_target;
    char configured[MAX_PATH * 4];
    char candidate[MAX_PATH * 4];
    size_t configured_length;
    size_t candidate_length;
    selected_target = twitch_override_uses_auto_room_a() ?
        twitch_override_auto_target : twitch_override_target;
    if (!twitch_override_enabled || !selected_target[0] ||
        !twitch_override_channel[0] || !sidecar_ini || !sidecar_ini[0]) {
        return 0;
    }
    normalize_override_path_a(selected_target, configured, sizeof(configured));
    normalize_override_path_a(sidecar_ini, candidate, sizeof(candidate));
    if (strcmp(configured, candidate) == 0) return 1;

    /* Older settings generators stored a path relative to the game folder. */
    configured_length = strlen(configured);
    candidate_length = strlen(candidate);
    if (configured_length < candidate_length &&
        candidate_length - configured_length > 0 &&
        candidate[candidate_length - configured_length - 1] == '/' &&
        strcmp(candidate + candidate_length - configured_length, configured) == 0) {
        return 1;
    }
    return 0;
}

static int texture_sidecar_webm_a(const char *path, char *out, size_t outsz)
{
    const char *dot;
    DWORD attr;
    char ini[MAX_PATH * 4];
    if (out && outsz) out[0] = 0;
    if (!path || !out || outsz < 6) return 0;
    if (!active_mod_texture_video_allowed_a(path)) return 0;
    twitch_override_auto_observe_texture_a(path);
    dot = strrchr(path, '.');
    if (!dot) return 0;
    if (_stricmp(dot, ".png") != 0 && _stricmp(dot, ".jp2") != 0 && _stricmp(dot, ".j2k") != 0) return 0;
    lstrcpynA(out, path, (int)outsz);
    dot = strrchr(out, '.');
    if (!dot) return 0;
    lstrcpynA((char*)dot, ".webm", (int)(outsz - (dot - out)));
    attr = GetFileAttributesA(out);
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) return 1;
    lstrcpynA(ini, out, sizeof(ini));
    dot = strrchr(ini, '.');
    if (!dot) return 0;
    lstrcpynA((char*)dot, ".ini", (int)(sizeof(ini) - (dot - ini)));
    return webm_twitch_sidecar_has_section(ini) || twitch_override_matches_sidecar_a(ini);
}

static int read_png_size_a(const char *path, int *width, int *height)
{
    HANDLE h;
    BYTE hdr[24];
    DWORD got = 0;
    if (width) *width = 0;
    if (height) *height = 0;
    if (!path || !ends_with_i(path, ".png")) return 0;
    if (!real_CreateFileA || !real_ReadFile || !real_CloseHandle) return 0;
    h = real_CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    if (!real_ReadFile(h, hdr, sizeof(hdr), &got, NULL) || got != sizeof(hdr)) {
        real_CloseHandle(h);
        return 0;
    }
    real_CloseHandle(h);
    if (hdr[0] != 0x89 || hdr[1] != 'P' || hdr[2] != 'N' || hdr[3] != 'G') return 0;
    if (memcmp(hdr + 12, "IHDR", 4) != 0) return 0;
    if (width) *width = ((int)hdr[16] << 24) | ((int)hdr[17] << 16) | ((int)hdr[18] << 8) | (int)hdr[19];
    if (height) *height = ((int)hdr[20] << 24) | ((int)hdr[21] << 16) | ((int)hdr[22] << 8) | (int)hdr[23];
    return width && height && *width > 0 && *height > 0;
}

static void sidecar_ini_path_a(const char *sidecar, char *out, size_t outsz)
{
    char *dot;
    if (!out || !outsz) return;
    out[0] = 0;
    if (!sidecar) return;
    lstrcpynA(out, sidecar, (int)outsz);
    dot = strrchr(out, '.');
    if (!dot) return;
    lstrcpynA(dot, ".ini", (int)(outsz - (dot - out)));
}

static int regular_file_exists_a(const char *path)
{
    DWORD attributes;
    if (!path || !path[0]) return 0;
    attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

static int webm_source_signature_a(const char *path, FILETIME *write_time,
                                   DWORD *size_high, DWORD *size_low)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (write_time) memset(write_time, 0, sizeof(*write_time));
    if (size_high) *size_high = 0;
    if (size_low) *size_low = 0;
    if (!path || !path[0] ||
        !GetFileAttributesExA(path, GetFileExInfoStandard, &data) ||
        (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return 0;
    }
    if (write_time) *write_time = data.ftLastWriteTime;
    if (size_high) *size_high = data.nFileSizeHigh;
    if (size_low) *size_low = data.nFileSizeLow;
    return 1;
}

static void webm_source_failure_park_a(webm_source_failure_state_t *state,
                                       const char *path, DWORD now,
                                       const char *reason)
{
    if (!state || state->parked) return;
    memset(state, 0, sizeof(*state));
    state->existed = webm_source_signature_a(path, &state->write_time,
                                             &state->size_high, &state->size_low);
    state->parked = 1;
    state->last_probe_tick = now;
    state->retry_tick = now + 30000u;
    if (state->existed) {
        log_line("WebM source parked after %s; retry_ms=30000 path=\"%s\"",
                 reason ? reason : "playback failure", path ? path : "");
    } else {
        log_line("WebM source missing; parked until file or configuration changes path=\"%s\"",
                 path ? path : "");
    }
}

static int webm_source_available_for_attempt_a(webm_source_failure_state_t *state,
                                                const char *path, DWORD now)
{
    if (!state || !path || !path[0] || state->parked) return 0;
    if (!webm_source_signature_a(path, NULL, NULL, NULL)) {
        webm_source_failure_park_a(state, path, now, "missing source");
        return 0;
    }
    return 1;
}

static int webm_source_failure_refresh_a(webm_source_failure_state_t *state,
                                         const char *path, DWORD now)
{
    FILETIME write_time;
    DWORD size_high;
    DWORD size_low;
    int existed;
    int changed;
    if (!state || !state->parked || !path || !path[0]) return 0;
    if (state->last_probe_tick && (DWORD)(now - state->last_probe_tick) < 1000u) {
        return 0;
    }
    state->last_probe_tick = now;
    existed = webm_source_signature_a(path, &write_time, &size_high, &size_low);
    changed = existed != state->existed ||
              (existed && (write_time.dwLowDateTime != state->write_time.dwLowDateTime ||
                           write_time.dwHighDateTime != state->write_time.dwHighDateTime ||
                           size_high != state->size_high || size_low != state->size_low));
    if (changed) {
        memset(state, 0, sizeof(*state));
        return WEBM_SOURCE_REFRESH_CHANGED;
    }
    if (existed && (LONG)(now - state->retry_tick) >= 0) {
        memset(state, 0, sizeof(*state));
        return WEBM_SOURCE_REFRESH_RETRY;
    }
    if (!existed && (LONG)(now - state->retry_tick) >= 0) {
        state->retry_tick = now + 30000u;
    }
    return 0;
}

static webm_twitch_session_t *create_twitch_session_a(const char *sidecar_ini,
                                                       webm_twitch_settings_t *settings,
                                                       int *logged_state)
{
    webm_twitch_session_t *session;
    int configured;
    int forced_override;
    if (settings) memset(settings, 0, sizeof(*settings));
    if (logged_state) *logged_state = -1;
    if (!settings || !config_path_global[0]) {
        return NULL;
    }
    configured = webm_twitch_load_settings(config_path_global, sidecar_ini, settings);
    forced_override = twitch_override_matches_sidecar_a(sidecar_ini);
    if (forced_override) {
        settings->enabled = 1;
        /* Reuse the sidecar resolver's existing fixed-channel-first random
           fallback and preferred-channel recovery behavior. */
        settings->random_enabled = twitch_override_channel_offline_random;
        settings->fallback_enabled = 1;
        lstrcpynA(settings->channel, twitch_override_channel, sizeof(settings->channel));
        if (twitch_override_quality[0]) {
            lstrcpynA(settings->quality, twitch_override_quality, sizeof(settings->quality));
        }
        settings->chat_enabled = twitch_override_chat_enabled;
        settings->chat_position = twitch_override_chat_position;
        settings->chat_overlay = twitch_override_chat_overlay;
        settings->chat_animated_emotes = twitch_override_chat_animated_emotes;
        settings->chat_width = twitch_override_chat_width;
        settings->chat_background_opacity = twitch_override_chat_background_opacity;
        configured = 1;
    }
    if (!configured) return NULL;
    /* Emote diagnostics are capped to one line per lifecycle state/session. */
    webm_twitch_chat_set_debug_logger(debug_line);
    session = webm_twitch_session_create(settings);
    if (session) {
        debug_line("Twitch configured channel=\"%s\" random=%d device_auth=%d quality=%s fallback=%d audio_volume=%d chat=%d overlay=%d emotes=%d override=%d sidecar=\"%s\"",
                 settings->channel, settings->random_enabled, settings->device_authorization,
                 settings->quality, settings->fallback_enabled,
                 settings->audio_volume, settings->chat_enabled,
                 settings->chat_overlay, settings->chat_emotes, forced_override, sidecar_ini);
    }
    return session;
}

static DWORD WINAPI twitch_decoder_open_thread(void *parameter)
{
    twitch_decoder_open_task_t *task = (twitch_decoder_open_task_t*)parameter;
    if (!task) return 0;
    task->decoder = ffmpeg_decoder_create(task->url);
    InterlockedExchange(&task->done, 1);
    return 0;
}

static twitch_decoder_open_task_t *twitch_decoder_open_task_create(const char *url)
{
    twitch_decoder_open_task_t *task;
    if (!url || !url[0]) return NULL;
    task = (twitch_decoder_open_task_t*)calloc(1, sizeof(*task));
    if (!task) return NULL;
    lstrcpynA(task->url, url, sizeof(task->url));
    task->thread = CreateThread(NULL, 0, twitch_decoder_open_thread, task, 0, NULL);
    if (!task->thread) {
        free(task);
        return NULL;
    }
    return task;
}

static int twitch_decoder_open_task_poll(twitch_decoder_open_task_t **task_ptr,
                                         video_decoder_t **decoder)
{
    twitch_decoder_open_task_t *task;
    if (decoder) *decoder = NULL;
    if (!task_ptr || !*task_ptr) return -1;
    task = *task_ptr;
    if (!InterlockedCompareExchange(&task->done, 0, 0) ||
        WaitForSingleObject(task->thread, 0) != WAIT_OBJECT_0) {
        return 0;
    }
    CloseHandle(task->thread);
    task->thread = NULL;
    if (decoder) *decoder = task->decoder;
    task->decoder = NULL;
    free(task);
    *task_ptr = NULL;
    return 1;
}

static void twitch_decoder_open_task_release(twitch_decoder_open_task_t *task)
{
    if (!task) return;
    if (task->thread) {
        WaitForSingleObject(task->thread, INFINITE);
        CloseHandle(task->thread);
    }
    if (task->decoder) video_decoder_release(task->decoder);
    free(task);
}

static int update_twitch_decoder_a(webm_twitch_session_t *session,
                                   const webm_twitch_settings_t *settings,
                                   int *twitch_active, int *fallback_active, int *logged_state,
                                   video_decoder_t **decoder,
                                   int *decoder_attempted,
                                   twitch_decoder_open_task_t **open_task,
                                   audio_graph_t **audio_graph,
                                   engine_audio_player_t **engine_audio, DWORD *playback_start_tick,
                                   int audio_enabled, int audio_volume, int audio_3d,
                                   int audio_3d_min_distance, int audio_3d_max_distance,
                                   int audio_3d_rolloff, const char *audio_node,
                                   const char *local_webm, const char *label, DWORD now)
{
    webm_twitch_state_t state;
    char url[WEBM_TWITCH_URL_MAX];
    int twitch_audio_volume;
    int twitch_min_distance;
    int twitch_max_distance;
    int twitch_rolloff;
    int switch_pending;
    if (!session || !settings || !twitch_active || !fallback_active || !decoder) return 1;
    twitch_audio_volume = settings->audio_volume;
    twitch_min_distance = settings->audio_3d_min_distance_set ?
                          settings->audio_3d_min_distance : audio_3d_min_distance;
    twitch_max_distance = settings->audio_3d_max_distance_set ?
                          settings->audio_3d_max_distance : audio_3d_max_distance;
    twitch_rolloff = settings->audio_3d_rolloff_set ?
                     settings->audio_3d_rolloff : audio_3d_rolloff;
    if (twitch_max_distance < twitch_min_distance + 1) {
        twitch_max_distance = twitch_min_distance + 1;
    }
    state = webm_twitch_session_poll(session, now, url, sizeof(url));
    switch_pending = webm_twitch_session_has_pending_switch(session);
    if (*twitch_active && *decoder) {
        char failure_reason[160];
        DWORD start_tick = playback_start_tick ? *playback_start_tick : 0;
        if (video_decoder_live_stream_failed(*decoder, now, start_tick,
                                             settings->connect_timeout_ms,
                                             failure_reason, sizeof(failure_reason))) {
            log_line("Twitch playback ended source=%s reason=\"%s\"; selecting fallback",
                     label, failure_reason);
            if (audio_graph && *audio_graph) {
                audio_graph_release(*audio_graph);
                *audio_graph = NULL;
            }
            if (engine_audio && *engine_audio) {
                engine_audio_player_release(*engine_audio);
                *engine_audio = NULL;
            }
            video_decoder_retire_async(*decoder);
            *decoder = NULL;
            *twitch_active = 0;
            *fallback_active = 0;
            if (decoder_attempted) *decoder_attempted = 0;
            if (playback_start_tick) *playback_start_tick = 0;
            webm_twitch_session_reject_url(session, now, failure_reason);
            if (logged_state) *logged_state = -1;
            state = WEBM_TWITCH_FAILED;
        }
    }
    if (logged_state && *logged_state != (int)state) {
        char error[256];
        error[0] = 0;
        if (state == WEBM_TWITCH_FAILED) webm_twitch_session_get_error(session, error, sizeof(error));
        if (state == WEBM_TWITCH_FAILED) {
            log_line("Twitch state=%d source=%s error=\"%s\" sidecar=\"%s\"",
                     (int)state, label, error, local_webm);
        } else {
            debug_line("Twitch state=%d source=%s sidecar=\"%s\"",
                       (int)state, label, local_webm);
        }
        *logged_state = (int)state;
    }
    if (state == WEBM_TWITCH_READY && (!*twitch_active || switch_pending)) {
        video_decoder_t *candidate = NULL;
        int open_state;
        if (!open_task) return 0;
        if (!*open_task) {
            *open_task = twitch_decoder_open_task_create(url);
            if (!*open_task) {
                webm_twitch_session_reject_url(session, now, "could not start Twitch decoder worker");
                if (logged_state) *logged_state = -1;
            }
            return 0;
        }
        open_state = twitch_decoder_open_task_poll(open_task, &candidate);
        if (open_state <= 0) return 0;
        if (!candidate) {
            if (*twitch_active && switch_pending) {
                webm_twitch_session_reject_pending_switch(
                    session, now, "FFmpeg could not open preferred Twitch stream");
                log_line("Twitch preferred-channel switch failed source=%s; keeping current stream",
                         label);
            } else {
                webm_twitch_session_reject_url(session, now,
                                               "FFmpeg could not open resolved Twitch stream");
                if (logged_state) *logged_state = -1;
            }
        } else {
            char active_channel[WEBM_TWITCH_CHANNEL_MAX];
            char previous_channel[WEBM_TWITCH_CHANNEL_MAX];
            active_channel[0] = 0;
            previous_channel[0] = 0;
            if (*twitch_active && switch_pending) {
                webm_twitch_session_get_channel(session, previous_channel,
                                                sizeof(previous_channel));
                webm_twitch_session_accept_pending_switch(session);
            }
            webm_twitch_session_get_channel(session, active_channel, sizeof(active_channel));
            candidate->live_buffer_ms = candidate->async_enabled ? settings->buffer_ms : 0;
            if (candidate->live_buffer_ms && !video_decoder_start_live_decode(candidate)) {
                log_line("FFmpeg Twitch split decode/presentation unavailable; using combined worker");
            }
            if (audio_graph && *audio_graph) {
                audio_graph_release(*audio_graph);
                *audio_graph = NULL;
            }
            if (engine_audio && *engine_audio) {
                engine_audio_player_release(*engine_audio);
                *engine_audio = NULL;
            }
            if (*decoder) video_decoder_retire_async(*decoder);
            *decoder = candidate;
            *twitch_active = 1;
            *fallback_active = 0;
            if (playback_start_tick) *playback_start_tick = now;
            if (audio_enabled && audio_graph && candidate->audio_stream_index >= 0) {
                *audio_graph = audio_graph_create_twitch_stream(local_webm, twitch_audio_volume,
                                                                audio_3d,
                                                                twitch_min_distance,
                                                                twitch_max_distance,
                                                                twitch_rolloff,
                                                                audio_node);
                if (!*audio_graph) {
                    log_line("Twitch audio unavailable; continuing video-only source=%s channel=\"%s\"",
                             label, active_channel[0] ? active_channel : settings->channel);
                }
            }
            log_line("Twitch playback active source=%s channel=\"%s\" quality=%s buffer_ms=%lu",
                     label, active_channel[0] ? active_channel : settings->channel,
                     settings->quality,
                     (unsigned long)candidate->live_buffer_ms);
            if (previous_channel[0]) {
                log_line("Twitch preferred channel became live source=%s switched=\"%s\"->\"%s\"",
                         label, previous_channel,
                         active_channel[0] ? active_channel : settings->channel);
            }
            return 0;
        }
    }
    if (*twitch_active) {
        if (audio_enabled && audio_graph && !*audio_graph && *decoder &&
            (*decoder)->audio_stream_index >= 0) {
            *audio_graph = audio_graph_create_twitch_stream(local_webm, twitch_audio_volume,
                                                            audio_3d,
                                                            twitch_min_distance,
                                                            twitch_max_distance,
                                                            twitch_rolloff,
                                                            audio_node);
        }
        return 0;
    }
    if (state == WEBM_TWITCH_FAILED && settings->fallback_enabled && regular_file_exists_a(local_webm)) {
        if (!*fallback_active) {
            *fallback_active = 1;
            log_line("Twitch using WebM fallback source=%s sidecar=\"%s\"", label, local_webm);
        }
        return 1;
    }
    return *fallback_active ? 1 : 0;
}

static void update_twitch_chat_a(webm_twitch_session_t *twitch_session,
                                 const webm_twitch_settings_t *settings,
                                 int twitch_active,
                                 webm_twitch_chat_session_t **chat_session,
                                 char *chat_channel, size_t chat_channel_size)
{
    char active_channel[WEBM_TWITCH_CHANNEL_MAX];
    int channel_changed;
    if (!chat_session || !chat_channel || !chat_channel_size) return;
    active_channel[0] = 0;
    if (twitch_active && settings && settings->chat_enabled && twitch_session) {
        webm_twitch_session_get_channel(twitch_session, active_channel,
                                        sizeof(active_channel));
    }
    channel_changed = _stricmp(chat_channel, active_channel) != 0;
    if (*chat_session && (!active_channel[0] || channel_changed)) {
        webm_twitch_chat_release(*chat_session);
        *chat_session = NULL;
        chat_channel[0] = 0;
    }
    if (!*chat_session && active_channel[0]) {
        *chat_session = webm_twitch_chat_acquire(settings, active_channel);
        if (*chat_session) lstrcpynA(chat_channel, active_channel, (int)chat_channel_size);
    }
}

static int ini_key_int_a(const char *path, const char *key, int fallback)
{
    char buf[64];
    char *end;
    long value;
    if (!path || !key) return fallback;
    buf[0] = 0;
    GetPrivateProfileStringA("NC-TK17-WebM", key, "", buf, sizeof(buf), path);
    if (!buf[0]) return fallback;
    value = strtol(buf, &end, 10);
    if (end == buf) return fallback;
    return (int)value;
}

static int parse_bool_value_a(const char *value, int fallback)
{
    char normalized[32];
    const char *start;
    const char *end;
    char *number_end;
    long number;
    size_t length;
    if (!value) return fallback;
    start = value;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') start++;
    end = start + strlen(start);
    while (end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) end--;
    length = (size_t)(end - start);
    if (!length || length >= sizeof(normalized)) return fallback;
    memcpy(normalized, start, length);
    normalized[length] = 0;
    if (_stricmp(normalized, "true") == 0 || _stricmp(normalized, "yes") == 0 ||
        _stricmp(normalized, "on") == 0) return 1;
    if (_stricmp(normalized, "false") == 0 || _stricmp(normalized, "no") == 0 ||
        _stricmp(normalized, "off") == 0) return 0;
    number = strtol(normalized, &number_end, 10);
    if (number_end != normalized && !*number_end) return number != 0;
    return fallback;
}

static int profile_key_bool_a(const char *path, const char *section, const char *key, int fallback)
{
    char value[64];
    if (!path || !section || !key) return fallback;
    value[0] = 0;
    GetPrivateProfileStringA(section, key, "", value, sizeof(value), path);
    if (!value[0]) return fallback;
    return parse_bool_value_a(value, fallback);
}

static int ini_key_bool_a(const char *path, const char *key, int fallback)
{
    return profile_key_bool_a(path, "NC-TK17-WebM", key, fallback);
}

static void ini_key_string_a(const char *path, const char *key, char *out, size_t outsz)
{
    if (!out || !outsz) return;
    out[0] = 0;
    if (!path || !key) return;
    GetPrivateProfileStringA("NC-TK17-WebM", key, "", out, (DWORD)outsz, path);
    out[outsz - 1] = 0;
}

static int ini_has_key_a(const char *path, const char *key)
{
    char buf[64];
    if (!path || !key) return 0;
    buf[0] = 0;
    GetPrivateProfileStringA("NC-TK17-WebM", key, "", buf, sizeof(buf), path);
    return buf[0] != 0;
}

static int get_file_write_time_a(const char *path, FILETIME *write_time)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (write_time) {
        write_time->dwLowDateTime = 0;
        write_time->dwHighDateTime = 0;
    }
    if (!path || !path[0]) return 0;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &data)) return 0;
    if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return 0;
    if (write_time) *write_time = data.ftLastWriteTime;
    return 1;
}

static int filetime_differs(const FILETIME *a, const FILETIME *b)
{
    if (!a || !b) return 1;
    return a->dwLowDateTime != b->dwLowDateTime || a->dwHighDateTime != b->dwHighDateTime;
}

static const char *video_filtering_name(int filtering)
{
    switch (filtering) {
        case VIDEO_FILTER_NEAREST: return "nearest";
        case VIDEO_FILTER_TRILINEAR: return "trilinear";
        case VIDEO_FILTER_ANISOTROPIC: return "anisotropic";
        case VIDEO_FILTER_LINEAR:
        default: return "linear";
    }
}

static int parse_video_filtering_a(const char *value, int fallback)
{
    if (!value || !value[0]) return fallback;
    if (_stricmp(value, "nearest") == 0 || _stricmp(value, "point") == 0) return VIDEO_FILTER_NEAREST;
    if (_stricmp(value, "linear") == 0 || _stricmp(value, "bilinear") == 0) return VIDEO_FILTER_LINEAR;
    if (_stricmp(value, "trilinear") == 0) return VIDEO_FILTER_TRILINEAR;
    if (_stricmp(value, "anisotropic") == 0) return VIDEO_FILTER_ANISOTROPIC;
    return fallback;
}

static int normalize_video_anisotropy(int value)
{
    if (value >= 16) return 16;
    if (value >= 8) return 8;
    if (value >= 4) return 4;
    if (value >= 2) return 2;
    return 1;
}

static void load_sidecar_settings_values_a(const char *sidecar, DWORD *interval_ms, int *max_width,
                                           int *max_height, int *mip_levels, int *filtering,
                                           int *anisotropy, FILETIME *write_time, int log_enabled)
{
    char ini[MAX_PATH * 4];
    char filtering_value[32];
    int fps;
    DWORD local_interval = texture_video_interval_ms;
    int local_max_width = texture_max_width;
    int local_max_height = texture_max_height;
    int local_mip_levels = d3d8_mip_levels;
    int local_filtering = video_filtering;
    int local_anisotropy = video_anisotropy;

    if (write_time) {
        write_time->dwLowDateTime = 0;
        write_time->dwHighDateTime = 0;
    }
    sidecar_ini_path_a(sidecar, ini, sizeof(ini));
    if (!get_file_write_time_a(ini, write_time)) {
        if (interval_ms) *interval_ms = local_interval;
        if (max_width) *max_width = local_max_width;
        if (max_height) *max_height = local_max_height;
        if (mip_levels) *mip_levels = local_mip_levels;
        if (filtering) *filtering = local_filtering;
        if (anisotropy) *anisotropy = local_anisotropy;
        return;
    }
    fps = clamp_int(ini_key_int_a(ini, "texture_fps", texture_video_fps), 1, 60);
    local_interval = (DWORD)(1000 / fps);
    if (!local_interval) local_interval = 1;
    local_max_width = clamp_int(ini_key_int_a(ini, "max_texture_width", texture_max_width), 64, 4096);
    local_max_height = clamp_int(ini_key_int_a(ini, "max_texture_height", texture_max_height), 64, 4096);
    if (ini_has_key_a(ini, "directx_mip_levels")) {
        local_mip_levels = ini_key_int_a(ini, "directx_mip_levels", d3d8_mip_levels);
    } else if (ini_has_key_a(ini, "directx_update_mips")) {
        local_mip_levels = ini_key_bool_a(ini, "directx_update_mips", 1) ? 8 : 1;
    }
    local_mip_levels = clamp_int(local_mip_levels, 0, 8);
    ini_key_string_a(ini, "video_filtering", filtering_value, sizeof(filtering_value));
    local_filtering = parse_video_filtering_a(filtering_value, video_filtering);
    if (ini_has_key_a(ini, "anisotropy")) {
        local_anisotropy = normalize_video_anisotropy(ini_key_int_a(ini, "anisotropy", video_anisotropy));
    }
    if (interval_ms) *interval_ms = local_interval;
    if (max_width) *max_width = local_max_width;
    if (max_height) *max_height = local_max_height;
    if (mip_levels) *mip_levels = local_mip_levels;
    if (filtering) *filtering = local_filtering;
    if (anisotropy) *anisotropy = local_anisotropy;
    if (log_enabled) {
        debug_line("sidecar config \"%s\" interval_ms=%lu max_texture=%dx%d directx_mip_levels=%d video_filtering=%s anisotropy=%d",
                 ini, (unsigned long)local_interval, local_max_width, local_max_height, local_mip_levels,
                 video_filtering_name(local_filtering), local_anisotropy);
    }
}

static void load_sidecar_settings_a(const char *sidecar)
{
    load_sidecar_settings_values_a(sidecar, &target_texture_interval_ms, &target_texture_max_width,
                                   &target_texture_max_height, &target_texture_d3d8_mip_levels,
                                   &target_texture_video_filtering, &target_texture_anisotropy,
                                   NULL, 1);
}

static void load_sidecar_audio_settings_a(const char *sidecar, int *enabled, int *engine, int *lead_ms, int *volume,
                                          int *audio_3d, int *min_distance, int *max_distance,
                                          int *rolloff, char *node, size_t node_sz,
                                          char *effect, size_t effect_sz)
{
    char ini[MAX_PATH * 4];
    char raw_parent[MAX_PATH * 4];
    int local_enabled = texture_audio_enabled;
    int local_engine = texture_audio_engine_enabled;
    int local_lead = texture_audio_lead_ms;
    int local_volume = texture_audio_volume;
    int local_3d = texture_audio_3d_enabled;
    int local_min = texture_audio_3d_min_distance;
    int local_max = texture_audio_3d_max_distance;
    int local_rolloff = texture_audio_3d_rolloff;
    char local_effect[32];
    int explicit_node = 0;
    int inferred_room_node = 0;
    char local_node[MAX_PATH * 4];
    local_node[0] = 0;
    lstrcpynA(local_effect, texture_audio_effect, sizeof(local_effect));
    raw_parent[0] = 0;
    basename_no_ext_a(sidecar, local_node, sizeof(local_node));
    sidecar_ini_path_a(sidecar, ini, sizeof(ini));
    if (get_file_write_time_a(ini, NULL)) {
        if (ini_has_key_a(ini, "texture_audio")) {
            local_enabled = ini_key_bool_a(ini, "texture_audio", local_enabled);
        }
        if (ini_has_key_a(ini, "texture_audio_engine")) {
            local_engine = ini_key_bool_a(ini, "texture_audio_engine", local_engine);
        }
        if (ini_has_key_a(ini, "audio_lead_ms")) {
            local_lead = clamp_int(ini_key_int_a(ini, "audio_lead_ms", local_lead), -10000, 10000);
        }
        if (ini_has_key_a(ini, "audio_volume")) {
            local_volume = clamp_int(ini_key_int_a(ini, "audio_volume", local_volume), -10000, 20000);
        }
        if (ini_has_key_a(ini, "audio_engine_volume")) {
            local_volume = clamp_int(ini_key_int_a(ini, "audio_engine_volume", local_volume), -10000, 20000);
        }
        if (ini_has_key_a(ini, "texture_audio_3d")) {
            local_3d = ini_key_bool_a(ini, "texture_audio_3d", local_3d);
        }
        if (ini_has_key_a(ini, "audio_3d_min_distance")) {
            local_min = clamp_int(ini_key_int_a(ini, "audio_3d_min_distance", local_min), 1, 100000);
        }
        if (ini_has_key_a(ini, "audio_3d_max_distance")) {
            local_max = clamp_int(ini_key_int_a(ini, "audio_3d_max_distance", local_max), 1, 100000);
        }
        if (ini_has_key_a(ini, "audio_3d_rolloff")) {
            local_rolloff = clamp_int(ini_key_int_a(ini, "audio_3d_rolloff", local_rolloff), 0, 20);
        }
        if (ini_has_key_a(ini, "audio_effect")) {
            ini_key_string_a(ini, "audio_effect", local_effect, sizeof(local_effect));
        }
        if (ini_has_key_a(ini, "audio_node")) {
            ini_key_string_a(ini, "audio_node", local_node, sizeof(local_node));
            explicit_node = 1;
        } else if (ini_has_key_a(ini, "audio_parent_path")) {
            char *p;
            ini_key_string_a(ini, "audio_parent_path", raw_parent, sizeof(raw_parent));
            for (p = raw_parent; *p == ' ' || *p == '\t' || *p == '"'; p++) {}
            if (*p && !strchr(p, '+') && !strchr(p, ':')) {
                char *end = p + strlen(p);
                while (end > p && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '"')) {
                    *--end = 0;
                }
                lstrcpynA(local_node, p, sizeof(local_node));
            }
        }
    }
    if (local_engine && !explicit_node && !raw_parent[0]) {
        if (!infer_toy_audio_parent_path_a(sidecar, local_node, sizeof(local_node))) {
            infer_room_audio_parent_path_a(sidecar, local_node, sizeof(local_node));
        }
    }
    if (local_3d && !explicit_node && path_is_addon_room_a(sidecar)) {
        char inferred_node[MAX_PATH * 4];
        char inferred_pos[128];
        if (infer_room_audio_transform_position_a(sidecar, inferred_pos, sizeof(inferred_pos))) {
            lstrcpynA(local_node, inferred_pos, sizeof(local_node));
            inferred_room_node = 1;
        } else if (infer_room_audio_parent_path_a(sidecar, inferred_node, sizeof(inferred_node)) &&
                   infer_room_audio_named_transform_position_a(sidecar, inferred_node, inferred_pos, sizeof(inferred_pos))) {
            lstrcpynA(local_node, inferred_pos, sizeof(local_node));
            inferred_room_node = 1;
        } else if (infer_room_audio_source_position_a(sidecar, inferred_pos, sizeof(inferred_pos))) {
            lstrcpynA(local_node, inferred_pos, sizeof(local_node));
            inferred_room_node = 1;
        } else if (infer_room_audio_parent_path_a(sidecar, inferred_node, sizeof(inferred_node))) {
            lstrcpynA(local_node, inferred_node, sizeof(local_node));
        } else {
            debug_line("Audio3D room source inference failed; keeping node=\"%s\" sidecar=\"%s\"",
                     local_node, sidecar);
        }
    }
    if (local_engine && path_is_addon_room_a(sidecar) && !explicit_node && !raw_parent[0] && inferred_room_node) {
        debug_line("EngineAudio room runtime skipped; using inferred OpenAL room source node=\"%s\" sidecar=\"%s\"",
                   local_node, sidecar);
        local_engine = 0;
        local_enabled = 1;
        local_3d = 1;
    } else if (local_engine && path_is_addon_room_a(sidecar) && !explicit_node && !raw_parent[0]) {
        debug_line("EngineAudio room runtime parked; no explicit audio_parent_path/audio_node, using regular texture audio sidecar=\"%s\"", sidecar);
        local_engine = 0;
        local_enabled = 1;
        local_3d = 0;
    }
    if (local_engine) {
        if (path_is_addon_toy_a(sidecar)) {
            debug_line("EngineAudio native runtime skipped for toy; using inferred OpenAL texture audio sidecar=\"%s\" node=\"%s\"",
                       sidecar, local_node);
            local_engine = 0;
            local_enabled = 1;
            local_3d = 1;
        } else {
            local_enabled = 1;
            local_3d = 1;
            if (!raw_parent[0]) {
                sidecar_ini_path_a(sidecar, ini, sizeof(ini));
                if (get_file_write_time_a(ini, NULL)) {
                    ini_key_string_a(ini, "audio_parent_path", raw_parent, sizeof(raw_parent));
                }
            }
            if (raw_parent[0]) {
                char *p = raw_parent;
                while (*p == ' ' || *p == '\t' || *p == '"') p++;
                lstrcpynA(local_node, p, sizeof(local_node));
            }
            if (!local_node[0]) {
                debug_line("EngineAudio runtime missing audio_parent_path; using regular texture audio sidecar=\"%s\"", sidecar);
                local_engine = 0;
            }
        }
    }
    if (local_max < local_min + 1) local_max = local_min + 1;
    if (enabled) *enabled = local_enabled;
    if (engine) *engine = local_engine;
    if (lead_ms) *lead_ms = local_lead;
    if (volume) *volume = local_volume;
    if (audio_3d) *audio_3d = local_3d;
    if (min_distance) *min_distance = local_min;
    if (max_distance) *max_distance = local_max;
    if (rolloff) *rolloff = local_rolloff;
    if (node && node_sz) lstrcpynA(node, local_node, (int)node_sz);
    if (effect && effect_sz) lstrcpynA(effect, local_effect, (int)effect_sz);
}

static void log_texture_sidecar_if_present(const char *path)
{
    char sidecar[MAX_PATH * 4];
    if (texture_sidecar_webm_a(path, sidecar, sizeof(sidecar))) {
        int w = 0, h = 0;
        DWORD now = GetTickCount();
        refresh_global_config(now);
        if (target_texture_probe_path[0] &&
            _stricmp(target_texture_probe_path, path) == 0 &&
            (now - target_texture_probe_tick) < 1500) {
            return;
        }
        read_png_size_a(path, &w, &h);
        lstrcpynA(target_texture_probe_path, path, sizeof(target_texture_probe_path));
        lstrcpynA(target_texture_sidecar_path, sidecar, sizeof(target_texture_sidecar_path));
        load_sidecar_settings_a(sidecar);
        target_texture_probe_width = w;
        target_texture_probe_height = h;
        target_texture_probe_tick = now;
    }
}

static void basename_no_ext_a(const char *path, char *out, size_t outsz)
{
    const char *name;
    const char *dot;
    size_t n;
    if (!outsz) return;
    out[0] = 0;
    if (!path) return;
    name = strrchr(path, '\\');
    if (!name) name = strrchr(path, '/');
    name = name ? name + 1 : path;
    dot = strrchr(name, '.');
    n = dot ? (size_t)(dot - name) : strlen(name);
    if (n >= outsz) n = outsz - 1;
    memcpy(out, name, n);
    out[n] = 0;
}

static int addon_root_from_path_a(const char *path, char *root, size_t root_sz)
{
    const char *marker;
    const char *p;
    size_t n;
    if (root && root_sz) root[0] = 0;
    if (!path || !root || !root_sz) return 0;
    marker = find_i(path, "\\Addons\\");
    if (!marker) marker = find_i(path, "/Addons/");
    if (!marker) {
        char texture_base[MAX_PATH];
        char scene_id[MAX_PATH];
        char dll_path[MAX_PATH * 2];
        char bin_dir[MAX_PATH * 2];
        char game_dir[MAX_PATH * 2];
        char addons_dir[MAX_PATH * 2];
        char pattern[MAX_PATH * 4];
        const char *underscore;
        size_t id_len;
        WIN32_FIND_DATAA data;
        HANDLE h;

        if (!contains_i(path, "\\Mod\\ActiveMod\\") && !contains_i(path, "/Mod/ActiveMod/")) return 0;
        basename_no_ext_a(path, texture_base, sizeof(texture_base));
        underscore = strchr(texture_base, '_');
        id_len = underscore ? (size_t)(underscore - texture_base) : strlen(texture_base);
        if (!id_len || id_len >= sizeof(scene_id)) return 0;
        memcpy(scene_id, texture_base, id_len);
        scene_id[id_len] = 0;
        if (!self_module || !GetModuleFileNameA(self_module, dll_path, sizeof(dll_path))) return 0;
        dll_path[sizeof(dll_path) - 1] = 0;
        strcpy(bin_dir, dll_path);
        dirname_inplace(bin_dir);
        strcpy(game_dir, bin_dir);
        dirname_inplace(game_dir);
        path_join(addons_dir, sizeof(addons_dir), game_dir, "Addons");
        for (n = 0; n < 2; n++) {
            path_join(pattern, sizeof(pattern), addons_dir, "*");
            h = FindFirstFileA(pattern, &data);
            if (h == INVALID_HANDLE_VALUE) return 0;
            do {
                static const char *scene_suffixes[] = {
                    "%s_core.bs",
                    "%s_Core.bs",
                    "%s.bs",
                    "%s.lua"
                };
                char candidate_root[MAX_PATH * 4];
                char scene_file[MAX_PATH];
                char scene_rel[MAX_PATH * 2];
                char scene_path[MAX_PATH * 4];
                DWORD attr;
                size_t si;
                if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0) continue;
                if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
                if (n == 0 && !contains_i(data.cFileName, scene_id)) continue;
                path_join(candidate_root, sizeof(candidate_root), addons_dir, data.cFileName);
                for (si = 0; si < sizeof(scene_suffixes) / sizeof(scene_suffixes[0]); si++) {
                    _snprintf(scene_file, sizeof(scene_file) - 1, scene_suffixes[si], scene_id);
                    scene_file[sizeof(scene_file) - 1] = 0;
                    _snprintf(scene_rel, sizeof(scene_rel) - 1, "Scenes\\Luder\\Room\\%s\\%s", scene_id, scene_file);
                    scene_rel[sizeof(scene_rel) - 1] = 0;
                    path_join(scene_path, sizeof(scene_path), candidate_root, scene_rel);
                    attr = GetFileAttributesA(scene_path);
                    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                        lstrcpynA(root, candidate_root, (int)root_sz);
                        FindClose(h);
                        debug_line("EngineAudio ActiveMod mapped sidecar=\"%s\" room_root=\"%s\" scene=\"%s\" pass=%u",
                                   path, root, scene_path, (unsigned)n + 1);
                        return 1;
                    }
                }
            } while (FindNextFileA(h, &data));
            FindClose(h);
        }
        debug_line("EngineAudio ActiveMod failed to map sidecar=\"%s\" scene_id=\"%s\"", path, scene_id);
        return 0;
    }
    p = marker + 8;
    while (*p && *p != '\\' && *p != '/') p++;
    if (!*p) return 0;
    n = (size_t)(p - path);
    if (!n || n >= root_sz) return 0;
    memcpy(root, path, n);
    root[n] = 0;
    return 1;
}

static char *read_text_file_a(const char *path)
{
    HANDLE h;
    DWORD size, got;
    char *buf = NULL;
    CreateFileA_t cf = real_CreateFileA ? real_CreateFileA : CreateFileA;
    ReadFile_t rf = real_ReadFile ? real_ReadFile : ReadFile;
    CloseHandle_t ch = real_CloseHandle ? real_CloseHandle : CloseHandle;
    if (!path || !path[0]) return NULL;
    h = cf(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return NULL;
    size = GetFileSize(h, NULL);
    if (size == INVALID_FILE_SIZE || size > 64 * 1024 * 1024) goto done;
    buf = (char*)malloc(size + 1);
    if (!buf) goto done;
    if (!rf(h, buf, size, &got, NULL) || got != size) {
        free(buf);
        buf = NULL;
        goto done;
    }
    buf[size] = 0;
done:
    ch(h);
    return buf;
}

static int copy_quoted_value_after_a(const char *start, const char *key, char *out, size_t outsz)
{
    const char *p;
    const char *q;
    size_t n;
    if (out && outsz) out[0] = 0;
    if (!start || !key || !out || !outsz) return 0;
    p = strstr(start, key);
    if (!p) return 0;
    p += strlen(key);
    q = strchr(p, '"');
    if (!q) return 0;
    p = q + 1;
    q = strchr(p, '"');
    if (!q || q <= p) return 0;
    n = (size_t)(q - p);
    if (n >= outsz) n = outsz - 1;
    memcpy(out, p, n);
    out[n] = 0;
    return 1;
}

static int copy_local_ref_after_a(const char *start, const char *end_limit, const char *key, char *out, size_t outsz)
{
    const char *p;
    const char *end;
    size_t n;
    if (out && outsz) out[0] = 0;
    if (!start || !key || !out || !outsz) return 0;
    p = strstr(start, key);
    if (!p || (end_limit && p > end_limit)) return 0;
    p += strlen(key);
    end = p;
    while (*end && *end != ';' && *end != '\r' && *end != '\n' && *end != ' ' && *end != '\t') end++;
    if (end_limit && end > end_limit) return 0;
    n = (size_t)(end - p);
    if (!n || n >= outsz) return 0;
    memcpy(out, p, n);
    out[n] = 0;
    return 1;
}

static int copy_local_decl_name_at_a(const char *p, const char *type_name, char *out, size_t outsz)
{
    char needle[128];
    const char *q;
    const char *end;
    size_t n;
    if (out && outsz) out[0] = 0;
    if (!p || !type_name || !out || !outsz) return 0;
    _snprintf(needle, sizeof(needle) - 1, "%s :local_", type_name);
    needle[sizeof(needle) - 1] = 0;
    if (_strnicmp(p, needle, strlen(needle)) != 0) return 0;
    q = p + strlen(needle);
    end = q;
    while (*end && *end != ' ' && *end != '\t' && *end != '\r' && *end != '\n' && *end != '.') end++;
    n = (size_t)(end - q);
    if (!n || n >= outsz) return 0;
    memcpy(out, q, n);
    out[n] = 0;
    return 1;
}

static int copy_local_decl_name_before_a(const char *src, const char *p, const char *type_name, char *out, size_t outsz)
{
    char needle[128];
    const char *q;
    if (out && outsz) out[0] = 0;
    if (!src || !p || !type_name || !out || !outsz || p < src) return 0;
    _snprintf(needle, sizeof(needle) - 1, "%s :local_", type_name);
    needle[sizeof(needle) - 1] = 0;
    q = p;
    while (q > src) {
        q--;
        if ((q == src || q[-1] == '\n' || q[-1] == '\r') &&
            _strnicmp(q, needle, strlen(needle)) == 0) {
            return copy_local_decl_name_at_a(q, type_name, out, outsz);
        }
    }
    return 0;
}

static int find_room_geom_by_texture_file_a(const char *src, const char *texture_base, const char **geom_out)
{
    const char *p;
    char file_local[128];
    char tex_local[128];
    char shadertex_local[128];
    char phong_local[128];
    char render_local[128];
    char file_needle[256];
    char tex_needle[256];
    char shadertex_needle[256];
    char phong_needle[256];
    char render_needle[256];
    char file_name[MAX_PATH * 2];
    char file_base[MAX_PATH];
    if (geom_out) *geom_out = NULL;
    if (!src || !texture_base || !geom_out) return 0;

    p = src;
    while ((p = find_i(p, "FileObject.FileName \"")) != NULL) {
        const char *line = p;
        while (line > src && line[-1] != '\n' && line[-1] != '\r') line--;
        if (!copy_quoted_value_after_a(line, "FileObject.FileName", file_name, sizeof(file_name))) {
            p += strlen("FileObject.FileName \"");
            continue;
        }
        basename_no_ext_a(file_name, file_base, sizeof(file_base));
        if (_stricmp(file_base, texture_base) != 0) {
            p += strlen("FileObject.FileName \"");
            continue;
        }
        if (!copy_local_decl_name_at_a(line, "FileObject", file_local, sizeof(file_local))) {
            p += strlen("FileObject.FileName \"");
            continue;
        }

        _snprintf(file_needle, sizeof(file_needle) - 1, "Texture.FileObject FileObject :local_%s", file_local);
        file_needle[sizeof(file_needle) - 1] = 0;
        p = find_i(src, file_needle);
        if (!p || !copy_local_decl_name_before_a(src, p, "Texture2D", tex_local, sizeof(tex_local))) break;

        _snprintf(tex_needle, sizeof(tex_needle) - 1, "ShaderTexture.Texture Texture2D :local_%s", tex_local);
        tex_needle[sizeof(tex_needle) - 1] = 0;
        p = find_i(src, tex_needle);
        if (!p || !copy_local_decl_name_before_a(src, p, "ShaderTexture", shadertex_local, sizeof(shadertex_local))) break;

        _snprintf(shadertex_needle, sizeof(shadertex_needle) - 1, "ShaderPhong.Color ShaderTexture :local_%s", shadertex_local);
        shadertex_needle[sizeof(shadertex_needle) - 1] = 0;
        p = find_i(src, shadertex_needle);
        if (!p || !copy_local_decl_name_before_a(src, p, "ShaderPhong", phong_local, sizeof(phong_local))) break;

        _snprintf(phong_needle, sizeof(phong_needle) - 1, "RenderShader.Surface ShaderPhong :local_%s", phong_local);
        phong_needle[sizeof(phong_needle) - 1] = 0;
        p = find_i(src, phong_needle);
        if (!p || !copy_local_decl_name_before_a(src, p, "RenderShader", render_local, sizeof(render_local))) break;

        _snprintf(render_needle, sizeof(render_needle) - 1, "TGeometry.Shader RenderShader :local_%s", render_local);
        render_needle[sizeof(render_needle) - 1] = 0;
        p = find_i(src, render_needle);
        if (!p) break;
        while (p > src && _strnicmp(p, "TPolygonGeometry :", 18) != 0) p--;
        if (_strnicmp(p, "TPolygonGeometry :", 18) != 0) break;
        *geom_out = p;
        return 1;
    }
    return 0;
}

static const char *scene_block_end_a(const char *start)
{
    const char *end;
    if (!start) return NULL;
    end = strstr(start, "\n};");
    if (!end) end = strstr(start, "\r\n};");
    return end ? end + 3 : start + strlen(start);
}

static int count_token_i_a(const char *src, const char *needle)
{
    const char *p = src;
    int count = 0;
    size_t n;
    if (!src || !needle || !needle[0]) return 0;
    n = strlen(needle);
    while ((p = find_i(p, needle)) != NULL) {
        count++;
        p += n;
    }
    return count;
}

static int scene_array_tuple_count_a(const char *start, const char *end_limit, const char *key)
{
    const char *p;
    const char *end;
    int count = 0;
    if (!start || !key) return -1;
    p = find_i(start, key);
    if (!p || (end_limit && p >= end_limit)) return -1;
    p = strchr(p, '[');
    if (!p || (end_limit && p >= end_limit)) return -1;
    end = strstr(p, "];");
    if (!end || (end_limit && end > end_limit)) return -1;
    while ((p = strchr(p + 1, '(')) != NULL && p < end) count++;
    return count;
}

static int scene_index_array_is_quad_a(const char *start, const char *end_limit)
{
    const char *p;
    const char *end;
    int count = 0;
    int seen[4] = {0, 0, 0, 0};
    if (!start) return 0;
    p = find_i(start, "SPolygonGeometry.IndexArray Array_I32");
    if (!p || (end_limit && p >= end_limit)) return 0;
    p = strchr(p, '[');
    if (!p || (end_limit && p >= end_limit)) return 0;
    end = strstr(p, "];");
    if (!end || (end_limit && end > end_limit)) return 0;
    p++;
    while (p < end) {
        char *next;
        long value;
        while (p < end && (isspace((unsigned char)*p) || *p == ',')) p++;
        if (p >= end) break;
        value = strtol(p, &next, 10);
        if (next == p || value < 0 || value > 3 || count >= 6) return 0;
        seen[value] = 1;
        count++;
        p = next;
    }
    return count == 6 && seen[0] && seen[1] && seen[2] && seen[3];
}

static int scene_uv_rect_a(const char *start, const char *end_limit,
                           float *min_u, float *min_v, float *max_u, float *max_v)
{
    const char *p;
    const char *end;
    double uv[4][2];
    double lo_u, lo_v, hi_u, hi_v;
    int corners[4] = {0, 0, 0, 0};
    int count = 0;
    int i;
    const double epsilon = 0.0001;
    if (!start || !min_u || !min_v || !max_u || !max_v) return 0;
    p = find_i(start, "VertexDataVector2f.DataArray Array_Vector2f");
    if (!p || (end_limit && p >= end_limit)) return 0;
    p = strchr(p, '[');
    if (!p || (end_limit && p >= end_limit)) return 0;
    end = strstr(p, "];");
    if (!end || (end_limit && end > end_limit)) return 0;
    while ((p = strchr(p + 1, '(')) != NULL && p < end) {
        char *next;
        if (count >= 4) return 0;
        uv[count][0] = strtod(p + 1, &next);
        if (next == p + 1) return 0;
        while (next < end && isspace((unsigned char)*next)) next++;
        if (*next != ',') return 0;
        uv[count][1] = strtod(next + 1, &next);
        if (next == NULL) return 0;
        count++;
        p = next;
    }
    if (count != 4) return 0;
    lo_u = hi_u = uv[0][0];
    lo_v = hi_v = uv[0][1];
    for (i = 1; i < 4; i++) {
        if (uv[i][0] < lo_u) lo_u = uv[i][0];
        if (uv[i][0] > hi_u) hi_u = uv[i][0];
        if (uv[i][1] < lo_v) lo_v = uv[i][1];
        if (uv[i][1] > hi_v) hi_v = uv[i][1];
    }
    if (hi_u - lo_u <= 0.000001 || hi_v - lo_v <= 0.000001) return 0;
    for (i = 0; i < 4; i++) {
        int x = fabs(uv[i][0] - hi_u) <= epsilon ? 1 : 0;
        int y = fabs(uv[i][1] - hi_v) <= epsilon ? 1 : 0;
        if (fabs(uv[i][0] - (x ? hi_u : lo_u)) > epsilon ||
            fabs(uv[i][1] - (y ? hi_v : lo_v)) > epsilon) return 0;
        if (corners[y * 2 + x]) return 0;
        corners[y * 2 + x] = 1;
    }
    *min_u = (float)lo_u;
    *min_v = (float)lo_v;
    *max_u = (float)hi_u;
    *max_v = (float)hi_v;
    return 1;
}

static int infer_room_video_uv_rect_a(const char *sidecar,
                                      float *min_u, float *min_v, float *max_u, float *max_v)
{
    char root[MAX_PATH * 4];
    char texture_base[MAX_PATH];
    char scene_id[MAX_PATH];
    char scene_rel[MAX_PATH * 2];
    char scene_path[MAX_PATH * 4];
    char render_local[128];
    char render_needle[256];
    char spoly_local[128];
    char spoly_decl[256];
    char uv_local[128];
    char uv_decl[256];
    const char *underscore;
    const char *geom;
    const char *geom_end;
    const char *spoly;
    const char *spoly_end;
    const char *uv_data;
    const char *uv_end;
    char *src = NULL;
    size_t id_len;
    int ok = 0;
    if (!sidecar || !min_u || !min_v || !max_u || !max_v) return 0;
    if (!addon_root_from_path_a(sidecar, root, sizeof(root))) return 0;
    basename_no_ext_a(sidecar, texture_base, sizeof(texture_base));
    underscore = strchr(texture_base, '_');
    id_len = underscore ? (size_t)(underscore - texture_base) : strlen(texture_base);
    if (!id_len || id_len >= sizeof(scene_id)) return 0;
    memcpy(scene_id, texture_base, id_len);
    scene_id[id_len] = 0;
    _snprintf(scene_rel, sizeof(scene_rel) - 1, "Scenes\\Luder\\Room\\%s\\%s_core.bs", scene_id, scene_id);
    scene_rel[sizeof(scene_rel) - 1] = 0;
    path_join(scene_path, sizeof(scene_path), root, scene_rel);
    src = read_text_file_a(scene_path);
    if (!src) goto done;
    if (!find_room_geom_by_texture_file_a(src, texture_base, &geom)) goto done;
    geom_end = scene_block_end_a(geom);
    if (!copy_local_ref_after_a(geom, geom_end, "TGeometry.Shader RenderShader :local_",
                                render_local, sizeof(render_local))) goto done;
    _snprintf(render_needle, sizeof(render_needle) - 1,
              "TGeometry.Shader RenderShader :local_%s", render_local);
    render_needle[sizeof(render_needle) - 1] = 0;
    if (count_token_i_a(src, render_needle) != 1) goto done;
    if (!copy_local_ref_after_a(geom, geom_end, "TNode.SNode SPolygonGeometry :local_",
                                spoly_local, sizeof(spoly_local))) goto done;
    _snprintf(spoly_decl, sizeof(spoly_decl) - 1, "SPolygonGeometry :local_%s", spoly_local);
    spoly_decl[sizeof(spoly_decl) - 1] = 0;
    spoly = find_scene_decl_i(src, spoly_decl);
    if (!spoly) goto done;
    spoly_end = scene_block_end_a(spoly);
    if (!find_i(spoly, "SPolygonGeometry.PrimCount I32(2)") ||
        find_i(spoly, "SPolygonGeometry.PrimCount I32(2)") >= spoly_end) goto done;
    if (!scene_index_array_is_quad_a(spoly, spoly_end)) goto done;
    if (scene_array_tuple_count_a(spoly, spoly_end,
                                  "SPolygonGeometry.VertexArray Array_Vector3f") != 4) goto done;
    if (!copy_local_ref_after_a(spoly, spoly_end, "VertexDataVector2f :local_",
                                uv_local, sizeof(uv_local))) goto done;
    _snprintf(uv_decl, sizeof(uv_decl) - 1, "VertexDataVector2f :local_%s", uv_local);
    uv_decl[sizeof(uv_decl) - 1] = 0;
    uv_data = find_scene_decl_i(src, uv_decl);
    if (!uv_data) goto done;
    uv_end = scene_block_end_a(uv_data);
    if (!scene_uv_rect_a(uv_data, uv_end, min_u, min_v, max_u, max_v)) goto done;
    ok = 1;
done:
    if (ok) {
        debug_line("VideoUV detected simple quad rect=(%.6f,%.6f)-(%.6f,%.6f) sidecar=\"%s\" scene=\"%s\"",
                   *min_u, *min_v, *max_u, *max_v, sidecar, scene_path);
    } else {
        log_line("VideoUV full_texture disabled: target is not a unique simple quad sidecar=\"%s\" scene=\"%s\"",
                 sidecar, scene_path);
    }
    free(src);
    return ok;
}

static int parse_video_uv_mode_a(const char *value, int fallback, const char *section, const char *ini)
{
    char normalized[64];
    const char *start;
    size_t n;
    if (!value) return fallback;
    start = value;
    while (*start && isspace((unsigned char)*start)) start++;
    n = strlen(start);
    while (n && isspace((unsigned char)start[n - 1])) n--;
    if (!n) return fallback;
    if (n >= sizeof(normalized)) n = sizeof(normalized) - 1;
    memcpy(normalized, start, n);
    normalized[n] = 0;
    if (_stricmp(normalized, "full_texture") == 0 ||
        _stricmp(normalized, "full") == 0 ||
        _stricmp(normalized, "stretch") == 0) {
        return WEBM_UV_MODE_FULL_TEXTURE;
    }
    if (_stricmp(normalized, "off") == 0 ||
        _stricmp(normalized, "none") == 0 ||
        _stricmp(normalized, "false") == 0 ||
        strcmp(normalized, "0") == 0) {
        return WEBM_UV_MODE_OFF;
    }
    log_line("VideoUV ignored unknown uv_mode=\"%s\" section=\"%s\" ini=\"%s\"",
             normalized, section ? section : "", ini ? ini : "");
    return fallback;
}

static int read_video_uv_mode_section_a(const char *ini, const char *section,
                                        int fallback, int *specified)
{
    char value[64];
    if (specified) *specified = 0;
    if (!ini || !section) return fallback;
    value[0] = 0;
    GetPrivateProfileStringA(section, "uv_mode", "", value, sizeof(value), ini);
    value[sizeof(value) - 1] = 0;
    if (!value[0]) return fallback;
    if (specified) *specified = 1;
    return parse_video_uv_mode_a(value, fallback, section, ini);
}

static void normalize_game_audio_name_a(const char *value, char *out, size_t outsz)
{
    const char *start;
    const char *end;
    size_t length;
    size_t i;
    if (!out || !outsz) return;
    out[0] = 0;
    if (!value) return;
    start = value;
    while (*start && isspace((unsigned char)*start)) start++;
    end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1])) end--;
    if (end - start >= 2 && start[0] == '"' && end[-1] == '"') {
        start++;
        end--;
        while (start < end && isspace((unsigned char)*start)) start++;
        while (end > start && isspace((unsigned char)end[-1])) end--;
    }
    length = (size_t)(end - start);
    if (length >= outsz) length = outsz - 1;
    for (i = 0; i < length; i++) {
        char ch = start[i];
        out[i] = ch == '\\' ? '/' : ch;
    }
    out[length] = 0;
    while (length && out[length - 1] == '/') out[--length] = 0;
    if (length > 4 && _stricmp(out + length - 4, ".ogg") == 0) {
        out[length - 4] = 0;
    }
}

static void game_audio_mute_list_add_a(game_audio_mute_list_t *list, const char *value)
{
    char normalized[WEBM_GAME_AUDIO_MUTE_NAME_MAX];
    int i;
    if (!list || !value || list->count >= WEBM_GAME_AUDIO_MUTE_MAX_NAMES) return;
    normalize_game_audio_name_a(value, normalized, sizeof(normalized));
    if (!normalized[0]) return;
    for (i = 0; i < list->count; i++) {
        if (_stricmp(list->names[i], normalized) == 0) return;
    }
    lstrcpynA(list->names[list->count], normalized, WEBM_GAME_AUDIO_MUTE_NAME_MAX);
    list->count++;
}

static void game_audio_mute_list_parse_a(game_audio_mute_list_t *list, char *value)
{
    char *item;
    char *next;
    if (!list || !value) return;
    item = value;
    while (item && *item) {
        next = strchr(item, ',');
        if (next) *next++ = 0;
        game_audio_mute_list_add_a(list, item);
        item = next;
    }
}

static void load_sidecar_game_audio_mutes_a(const char *sidecar, game_audio_mute_list_t *list)
{
    static const char *sections[] = {"NC-TK17-WebM", "NC-TK17-WebM:Twitch"};
    char ini[MAX_PATH * 4];
    char value[1024];
    int i;
    if (!list) return;
    memset(list, 0, sizeof(*list));
    if (!sidecar || !sidecar[0]) return;
    sidecar_ini_path_a(sidecar, ini, sizeof(ini));
    if (!ini[0] || !regular_file_exists_a(ini)) return;
    for (i = 0; i < (int)(sizeof(sections) / sizeof(sections[0])); i++) {
        value[0] = 0;
        GetPrivateProfileStringA(sections[i], "mute_game_audio", "",
                                 value, sizeof(value), ini);
        value[sizeof(value) - 1] = 0;
        game_audio_mute_list_parse_a(list, value);
    }
    if (list->count) {
        debug_line("GameAudio sidecar mute targets=%d ini=\"%s\"", list->count, ini);
    }
}

static void load_sidecar_uv_settings_a(const char *sidecar,
                                       int *base_mode, int *twitch_mode, int *rect_valid,
                                       float *min_u, float *min_v, float *max_u, float *max_v)
{
    char ini[MAX_PATH * 4];
    int base = WEBM_UV_MODE_OFF;
    int twitch;
    int twitch_specified = 0;
    int valid = 0;
    float lo_u = 0.0f, lo_v = 0.0f, hi_u = 1.0f, hi_v = 1.0f;
    sidecar_ini_path_a(sidecar, ini, sizeof(ini));
    if (ini[0] && regular_file_exists_a(ini)) {
        base = read_video_uv_mode_section_a(ini, "NC-TK17-WebM", WEBM_UV_MODE_OFF, NULL);
        twitch = read_video_uv_mode_section_a(ini, "NC-TK17-WebM:Twitch", base, &twitch_specified);
        if (!twitch_specified) twitch = base;
        if (base == WEBM_UV_MODE_FULL_TEXTURE || twitch == WEBM_UV_MODE_FULL_TEXTURE) {
            valid = infer_room_video_uv_rect_a(sidecar, &lo_u, &lo_v, &hi_u, &hi_v);
            if (!valid) {
                if (base == WEBM_UV_MODE_FULL_TEXTURE) base = WEBM_UV_MODE_OFF;
                if (twitch == WEBM_UV_MODE_FULL_TEXTURE) twitch = WEBM_UV_MODE_OFF;
            }
        }
    } else {
        twitch = base;
    }
    if (base_mode) *base_mode = base;
    if (twitch_mode) *twitch_mode = twitch;
    if (rect_valid) *rect_valid = valid;
    if (min_u) *min_u = lo_u;
    if (min_v) *min_v = lo_v;
    if (max_u) *max_u = hi_u;
    if (max_v) *max_v = hi_v;
}

static int video_uv_proxy_dimensions(int source_width, int source_height,
                                     int max_width, int max_height,
                                     float min_u, float min_v, float max_u, float max_v,
                                     int *proxy_width, int *proxy_height)
{
    double region_width;
    double region_height;
    double aspect;
    int width;
    int height;
    if (proxy_width) *proxy_width = 0;
    if (proxy_height) *proxy_height = 0;
    if (!proxy_width || !proxy_height || source_width <= 0 || source_height <= 0 ||
        max_width <= 0 || max_height <= 0) return 0;
    region_width = (double)(max_u - min_u) * (double)source_width;
    region_height = (double)(max_v - min_v) * (double)source_height;
    if (region_width <= 0.000001 || region_height <= 0.000001) return 0;
    aspect = region_width / region_height;
    width = max_width;
    height = (int)((double)width / aspect + 0.5);
    if (height > max_height) {
        height = max_height;
        width = (int)((double)height * aspect + 0.5);
    }
    if (width < 1) width = 1;
    if (height < 1) height = 1;
    if (width > max_width) width = max_width;
    if (height > max_height) height = max_height;
    /* Even dimensions avoid chroma-edge artifacts in common YUV video sizes. */
    if (width > 1) width &= ~1;
    if (height > 1) height &= ~1;
    *proxy_width = width;
    *proxy_height = height;
    return width > 0 && height > 0;
}

static int infer_toy_audio_parent_path_a(const char *sidecar, char *out, size_t outsz)
{
    char root[MAX_PATH * 4];
    char texture_base[MAX_PATH];
    char scene_id[MAX_PATH];
    char scene_rel[MAX_PATH * 2];
    char scene_path[MAX_PATH * 4];
    char shader_needle[512];
    char parent_local[128];
    char parent_decl[256];
    char parent_name[128];
    const char *underscore;
    const char *geom;
    const char *geom_end;
    const char *parent_pos;
    char *src = NULL;
    size_t id_len;
    int ok = 0;
    if (out && outsz) out[0] = 0;
    if (!sidecar || !out || !outsz) return 0;
    if (!addon_root_from_path_a(sidecar, root, sizeof(root))) return 0;
    basename_no_ext_a(sidecar, texture_base, sizeof(texture_base));
    underscore = strchr(texture_base, '_');
    id_len = underscore ? (size_t)(underscore - texture_base) : strlen(texture_base);
    if (!id_len || id_len >= sizeof(scene_id)) return 0;
    memcpy(scene_id, texture_base, id_len);
    scene_id[id_len] = 0;
    _snprintf(scene_rel, sizeof(scene_rel) - 1, "Scenes\\Shared\\Item\\%s.bs", scene_id);
    scene_rel[sizeof(scene_rel) - 1] = 0;
    path_join(scene_path, sizeof(scene_path), root, scene_rel);
    src = read_text_file_a(scene_path);
    if (!src) return 0;

    _snprintf(shader_needle, sizeof(shader_needle) - 1, "TGeometry.Shader RenderShader :local_%s_MATERIAL", texture_base);
    shader_needle[sizeof(shader_needle) - 1] = 0;
    geom = find_i(src, shader_needle);
    if (!geom) goto done;
    while (geom > src && _strnicmp(geom, "TPolygonGeometry :", 18) != 0) geom--;
    if (_strnicmp(geom, "TPolygonGeometry :", 18) != 0) goto done;
    geom_end = strstr(geom, "\n}");
    if (!geom_end) geom_end = geom + strlen(geom);
    parent_pos = strstr(geom, "TNode.Parent TTransform :local_");
    if (!parent_pos || parent_pos > geom_end) goto done;
    parent_pos += strlen("TNode.Parent TTransform :local_");
    {
        const char *end = parent_pos;
        size_t n;
        while (*end && *end != ';' && *end != '\r' && *end != '\n' && *end != ' ' && *end != '\t') end++;
        n = (size_t)(end - parent_pos);
        if (!n || n >= sizeof(parent_local)) goto done;
        memcpy(parent_local, parent_pos, n);
        parent_local[n] = 0;
    }
    _snprintf(parent_decl, sizeof(parent_decl) - 1, "TTransform :local_%s", parent_local);
    parent_decl[sizeof(parent_decl) - 1] = 0;
    parent_pos = find_i(src, parent_decl);
    if (!parent_pos) goto done;
    if (!copy_quoted_value_after_a(parent_pos, "Object.Name", parent_name, sizeof(parent_name))) goto done;
    _snprintf(out, outsz - 1, "%s:%s", scene_id, parent_name);
    out[outsz - 1] = 0;
    ok = 1;
done:
    free(src);
    if (ok) {
        debug_line("EngineAudio inferred toy parent path=\"%s\" sidecar=\"%s\" scene=\"%s\"",
                   out, sidecar, scene_path);
    }
    if (!ok) {
        _snprintf(out, outsz - 1, "%s:tool_group", scene_id);
        out[outsz - 1] = 0;
        debug_line("EngineAudio inferred toy fallback parent path=\"%s\" sidecar=\"%s\"", out, sidecar);
        return 1;
    }
    return ok;
}

static int infer_room_audio_parent_path_a(const char *sidecar, char *out, size_t outsz)
{
    char root[MAX_PATH * 4];
    char texture_base[MAX_PATH];
    char scene_id[MAX_PATH];
    char material_base[MAX_PATH];
    char scene_rel[MAX_PATH * 2];
    char scene_path[MAX_PATH * 4];
    char shader_needle[512];
    char parent_local[128];
    char mesh_parent_local[128];
    char parent_decl[256];
    char parent_name[128];
    const char *underscore;
    const char *geom;
    const char *geom_end;
    const char *parent_pos;
    char *src = NULL;
    size_t id_len;
    int ok = 0;
    if (out && outsz) out[0] = 0;
    if (!sidecar || !out || !outsz) return 0;
    if (!addon_root_from_path_a(sidecar, root, sizeof(root))) return 0;

    basename_no_ext_a(sidecar, texture_base, sizeof(texture_base));
    underscore = strchr(texture_base, '_');
    id_len = underscore ? (size_t)(underscore - texture_base) : strlen(texture_base);
    if (!id_len || id_len >= sizeof(scene_id)) return 0;
    memcpy(scene_id, texture_base, id_len);
    scene_id[id_len] = 0;
    if (underscore && underscore[1]) {
        lstrcpynA(material_base, underscore + 1, sizeof(material_base));
    } else {
        lstrcpynA(material_base, texture_base, sizeof(material_base));
    }

    _snprintf(scene_rel, sizeof(scene_rel) - 1, "Scenes\\Luder\\Room\\%s\\%s_core.bs", scene_id, scene_id);
    scene_rel[sizeof(scene_rel) - 1] = 0;
    path_join(scene_path, sizeof(scene_path), root, scene_rel);
    src = read_text_file_a(scene_path);
    if (!src) return 0;

    if (find_room_geom_by_texture_file_a(src, texture_base, &geom)) {
        debug_line("EngineAudio room parent matched texture refs texture=\"%s\" sidecar=\"%s\"",
                   texture_base, sidecar);
    } else {
        _snprintf(shader_needle, sizeof(shader_needle) - 1, "TGeometry.Shader RenderShader :local_%s_material", material_base);
        shader_needle[sizeof(shader_needle) - 1] = 0;
        geom = find_i(src, shader_needle);
        if (!geom) {
            _snprintf(shader_needle, sizeof(shader_needle) - 1, "TGeometry.Shader RenderShader :local_%s_MATERIAL", material_base);
            shader_needle[sizeof(shader_needle) - 1] = 0;
            geom = find_i(src, shader_needle);
        }
    }
    if (!geom) goto done;
    while (geom > src && _strnicmp(geom, "TPolygonGeometry :", 18) != 0) geom--;
    if (_strnicmp(geom, "TPolygonGeometry :", 18) != 0) goto done;
    geom_end = strstr(geom, "\n}");
    if (!geom_end) geom_end = geom + strlen(geom);

    if (copy_local_ref_after_a(geom, geom_end, "TNode.Parent TTransform :local_", parent_local, sizeof(parent_local))) {
        parent_pos = NULL;
    } else if (copy_local_ref_after_a(geom, geom_end, "TNode.Parent TMeshGroup :local_", mesh_parent_local, sizeof(mesh_parent_local))) {
        _snprintf(parent_decl, sizeof(parent_decl) - 1, "TMeshGroup :local_%s", mesh_parent_local);
        parent_decl[sizeof(parent_decl) - 1] = 0;
        parent_pos = find_i(src, parent_decl);
        if (!parent_pos) goto done;
        geom_end = strstr(parent_pos, "\n}");
        if (!geom_end) geom_end = parent_pos + strlen(parent_pos);
        if (!copy_local_ref_after_a(parent_pos, geom_end, "TNode.Parent TTransform :local_", parent_local, sizeof(parent_local))) {
            size_t n = strlen(mesh_parent_local);
            if (n > 5 && _stricmp(mesh_parent_local + n - 5, "_MESH") == 0) {
                n -= 5;
                if (!n || n >= sizeof(parent_local)) goto done;
                memcpy(parent_local, mesh_parent_local, n);
                parent_local[n] = 0;
            } else {
                goto done;
            }
        }
    } else {
        goto done;
    }

    _snprintf(parent_decl, sizeof(parent_decl) - 1, "TTransform :local_%s", parent_local);
    parent_decl[sizeof(parent_decl) - 1] = 0;
    parent_pos = find_i(src, parent_decl);
    if (!parent_pos) goto done;
    if (!copy_quoted_value_after_a(parent_pos, "Object.Name", parent_name, sizeof(parent_name))) goto done;
    _snprintf(out, outsz - 1, "%s:%s", scene_id, parent_name);
    out[outsz - 1] = 0;
    ok = 1;
done:
    free(src);
    if (ok) {
        debug_line("EngineAudio inferred room parent path=\"%s\" sidecar=\"%s\" scene=\"%s\"",
                   out, sidecar, scene_path);
    } else {
        debug_line("EngineAudio failed to infer room parent sidecar=\"%s\" scene=\"%s\"", sidecar, scene_path);
    }
    return ok;
}

static int infer_room_audio_geometry_path_a(const char *sidecar, char *out, size_t outsz)
{
    char root[MAX_PATH * 4];
    char texture_base[MAX_PATH];
    char scene_id[MAX_PATH];
    char material_base[MAX_PATH];
    char scene_rel[MAX_PATH * 2];
    char scene_path[MAX_PATH * 4];
    char shader_needle[512];
    char geom_name[128];
    const char *underscore;
    const char *geom;
    char *src = NULL;
    size_t id_len;
    int ok = 0;

    if (out && outsz) out[0] = 0;
    if (!sidecar || !out || !outsz) return 0;
    if (!addon_root_from_path_a(sidecar, root, sizeof(root))) return 0;

    basename_no_ext_a(sidecar, texture_base, sizeof(texture_base));
    underscore = strchr(texture_base, '_');
    id_len = underscore ? (size_t)(underscore - texture_base) : strlen(texture_base);
    if (!id_len || id_len >= sizeof(scene_id)) return 0;
    memcpy(scene_id, texture_base, id_len);
    scene_id[id_len] = 0;
    if (underscore && underscore[1]) {
        lstrcpynA(material_base, underscore + 1, sizeof(material_base));
    } else {
        lstrcpynA(material_base, texture_base, sizeof(material_base));
    }

    _snprintf(scene_rel, sizeof(scene_rel) - 1, "Scenes\\Luder\\Room\\%s\\%s_core.bs", scene_id, scene_id);
    scene_rel[sizeof(scene_rel) - 1] = 0;
    path_join(scene_path, sizeof(scene_path), root, scene_rel);
    src = read_text_file_a(scene_path);
    if (!src) return 0;

    if (!find_room_geom_by_texture_file_a(src, texture_base, &geom)) {
        _snprintf(shader_needle, sizeof(shader_needle) - 1, "TGeometry.Shader RenderShader :local_%s_material", material_base);
        shader_needle[sizeof(shader_needle) - 1] = 0;
        geom = find_i(src, shader_needle);
        if (!geom) {
            _snprintf(shader_needle, sizeof(shader_needle) - 1, "TGeometry.Shader RenderShader :local_%s_MATERIAL", material_base);
            shader_needle[sizeof(shader_needle) - 1] = 0;
            geom = find_i(src, shader_needle);
        }
    }
    if (!geom) goto done;
    while (geom > src && _strnicmp(geom, "TPolygonGeometry :", 18) != 0) geom--;
    if (_strnicmp(geom, "TPolygonGeometry :", 18) != 0) goto done;
    if (!copy_quoted_value_after_a(geom, "Object.Name", geom_name, sizeof(geom_name))) goto done;

    _snprintf(out, outsz - 1, "%s:%s", scene_id, geom_name);
    out[outsz - 1] = 0;
    ok = 1;

done:
    free(src);
    if (ok) {
        debug_line("EngineAudio inferred room geometry path=\"%s\" sidecar=\"%s\" scene=\"%s\"",
                   out, sidecar, scene_path);
    } else {
        debug_line("EngineAudio failed to infer room geometry sidecar=\"%s\" scene=\"%s\"",
                 sidecar, scene_path);
    }
    return ok;
}

static int infer_room_audio_transform_position_a(const char *sidecar, char *out, size_t outsz)
{
    char root[MAX_PATH * 4];
    char texture_base[MAX_PATH];
    char scene_id[MAX_PATH];
    char material_base[MAX_PATH];
    char scene_rel[MAX_PATH * 2];
    char scene_path[MAX_PATH * 4];
    char shader_needle[512];
    char parent_local[128];
    char mesh_parent_local[128];
    char snode_local[128];
    char parent_decl[256];
    char snode_decl[256];
    const char *underscore;
    const char *geom;
    const char *geom_end;
    const char *parent_pos;
    const char *snode_pos;
    const char *translation;
    char *src = NULL;
    size_t id_len;
    double x = 0.0, y = 0.0, z = 0.0;
    int ok = 0;

    if (out && outsz) out[0] = 0;
    if (!sidecar || !out || !outsz) return 0;
    if (!addon_root_from_path_a(sidecar, root, sizeof(root))) return 0;

    basename_no_ext_a(sidecar, texture_base, sizeof(texture_base));
    underscore = strchr(texture_base, '_');
    id_len = underscore ? (size_t)(underscore - texture_base) : strlen(texture_base);
    if (!id_len || id_len >= sizeof(scene_id)) return 0;
    memcpy(scene_id, texture_base, id_len);
    scene_id[id_len] = 0;
    if (underscore && underscore[1]) {
        lstrcpynA(material_base, underscore + 1, sizeof(material_base));
    } else {
        lstrcpynA(material_base, texture_base, sizeof(material_base));
    }

    _snprintf(scene_rel, sizeof(scene_rel) - 1, "Scenes\\Luder\\Room\\%s\\%s_core.bs", scene_id, scene_id);
    scene_rel[sizeof(scene_rel) - 1] = 0;
    path_join(scene_path, sizeof(scene_path), root, scene_rel);
    src = read_text_file_a(scene_path);
    if (!src) return 0;

    if (!find_room_geom_by_texture_file_a(src, texture_base, &geom)) {
        _snprintf(shader_needle, sizeof(shader_needle) - 1, "TGeometry.Shader RenderShader :local_%s_material", material_base);
        shader_needle[sizeof(shader_needle) - 1] = 0;
        geom = find_i(src, shader_needle);
        if (!geom) {
            _snprintf(shader_needle, sizeof(shader_needle) - 1, "TGeometry.Shader RenderShader :local_%s_MATERIAL", material_base);
            shader_needle[sizeof(shader_needle) - 1] = 0;
            geom = find_i(src, shader_needle);
        }
    }
    if (!geom) goto done;
    while (geom > src && _strnicmp(geom, "TPolygonGeometry :", 18) != 0) geom--;
    if (_strnicmp(geom, "TPolygonGeometry :", 18) != 0) goto done;
    geom_end = strstr(geom, "\n}");
    if (!geom_end) geom_end = geom + strlen(geom);

    if (copy_local_ref_after_a(geom, geom_end, "TNode.Parent TTransform :local_", parent_local, sizeof(parent_local))) {
        parent_pos = NULL;
    } else if (copy_local_ref_after_a(geom, geom_end, "TNode.Parent TMeshGroup :local_", mesh_parent_local, sizeof(mesh_parent_local))) {
        _snprintf(parent_decl, sizeof(parent_decl) - 1, "TMeshGroup :local_%s", mesh_parent_local);
        parent_decl[sizeof(parent_decl) - 1] = 0;
        parent_pos = find_i(src, parent_decl);
        if (!parent_pos) goto done;
        geom_end = strstr(parent_pos, "\n}");
        if (!geom_end) geom_end = parent_pos + strlen(parent_pos);
        if (!copy_local_ref_after_a(parent_pos, geom_end, "TNode.Parent TTransform :local_", parent_local, sizeof(parent_local))) {
            size_t n = strlen(mesh_parent_local);
            if (n > 5 && _stricmp(mesh_parent_local + n - 5, "_MESH") == 0) {
                n -= 5;
                if (!n || n >= sizeof(parent_local)) goto done;
                memcpy(parent_local, mesh_parent_local, n);
                parent_local[n] = 0;
            } else {
                goto done;
            }
        }
    } else {
        goto done;
    }

    _snprintf(parent_decl, sizeof(parent_decl) - 1, "TTransform :local_%s", parent_local);
    parent_decl[sizeof(parent_decl) - 1] = 0;
    parent_pos = find_i(src, parent_decl);
    if (!parent_pos) goto done;
    geom_end = strstr(parent_pos, "\n}");
    if (!geom_end) geom_end = parent_pos + strlen(parent_pos);
    if (!copy_local_ref_after_a(parent_pos, geom_end, "TNode.SNode STransform :local_", snode_local, sizeof(snode_local))) {
        goto done;
    }

    _snprintf(snode_decl, sizeof(snode_decl) - 1, "STransform :local_%s", snode_local);
    snode_decl[sizeof(snode_decl) - 1] = 0;
    snode_pos = find_i(src, snode_decl);
    if (!snode_pos) goto done;
    geom_end = strstr(snode_pos, "\n}");
    if (!geom_end) geom_end = snode_pos + strlen(snode_pos);
    translation = find_i(snode_pos, "SSimpleTransform.Translation Vector3f(");
    if (!translation || translation > geom_end) goto done;
    translation += strlen("SSimpleTransform.Translation Vector3f(");
    if (sscanf(translation, " %lf , %lf , %lf", &x, &y, &z) != 3) goto done;
    _snprintf(out, outsz - 1, "pos:%.4f,%.4f,%.4f", x, y, z);
    out[outsz - 1] = 0;
    ok = 1;

done:
    free(src);
    if (ok) {
        debug_line("EngineAudio inferred room transform position=\"%s\" sidecar=\"%s\" scene=\"%s\"",
                   out, sidecar, scene_path);
    } else {
        debug_line("EngineAudio failed to infer room transform position sidecar=\"%s\" scene=\"%s\"",
                 sidecar, scene_path);
    }
    return ok;
}

static int infer_room_audio_named_transform_position_a(const char *sidecar, const char *node, char *out, size_t outsz)
{
    char root[MAX_PATH * 4];
    char texture_base[MAX_PATH];
    char scene_id[MAX_PATH];
    char scene_rel[MAX_PATH * 2];
    char scene_path[MAX_PATH * 4];
    char object_needle[256];
    char tform_decl[256];
    char snode_local[128];
    char snode_decl[256];
    const char *underscore;
    const char *object_name;
    const char *name;
    const char *tform;
    const char *tform_end;
    const char *snode_pos;
    const char *snode_end;
    const char *translation;
    char *endp;
    char *src = NULL;
    size_t id_len;
    double x = 0.0, y = 0.0, z = 0.0;
    int ok = 0;

    if (out && outsz) out[0] = 0;
    if (!sidecar || !node || !out || !outsz) return 0;
    if (!addon_root_from_path_a(sidecar, root, sizeof(root))) return 0;

    basename_no_ext_a(sidecar, texture_base, sizeof(texture_base));
    underscore = strchr(texture_base, '_');
    id_len = underscore ? (size_t)(underscore - texture_base) : strlen(texture_base);
    if (!id_len || id_len >= sizeof(scene_id)) return 0;
    memcpy(scene_id, texture_base, id_len);
    scene_id[id_len] = 0;

    name = strchr(node, ':');
    name = name ? name + 1 : node;
    if (!name[0]) return 0;

    _snprintf(scene_rel, sizeof(scene_rel) - 1, "Scenes\\Luder\\Room\\%s\\%s_core.bs", scene_id, scene_id);
    scene_rel[sizeof(scene_rel) - 1] = 0;
    path_join(scene_path, sizeof(scene_path), root, scene_rel);
    src = read_text_file_a(scene_path);
    if (!src) return 0;

    _snprintf(snode_decl, sizeof(snode_decl) - 1, "STransform :local_S%s", name);
    snode_decl[sizeof(snode_decl) - 1] = 0;
    snode_pos = find_scene_decl_i(src, snode_decl);
    if (snode_pos) goto parse_snode;
    debug_line("EngineAudio room named transform stage miss direct_snode decl=\"%s\" node=\"%s\" scene=\"%s\"",
               snode_decl, node, scene_path);

    _snprintf(tform_decl, sizeof(tform_decl) - 1, "TTransform :local_%s", name);
    tform_decl[sizeof(tform_decl) - 1] = 0;
    tform = find_scene_decl_i(src, tform_decl);
    if (!tform) {
        _snprintf(object_needle, sizeof(object_needle) - 1, "Object.Name \"%s\"", name);
        object_needle[sizeof(object_needle) - 1] = 0;
        object_name = find_i(src, object_needle);
        if (!object_name) {
            debug_line("EngineAudio room named transform stage miss object_name needle=\"%s\" node=\"%s\" scene=\"%s\"",
                       object_needle, node, scene_path);
            goto done;
        }

        tform = object_name;
        while (tform > src && _strnicmp(tform, "TTransform :", 12) != 0) tform--;
        if (_strnicmp(tform, "TTransform :", 12) != 0 || find_scene_decl_i(tform, "TTransform :") != tform) {
            debug_line("EngineAudio room named transform stage miss tform_from_object node=\"%s\" scene=\"%s\"",
                       node, scene_path);
            goto done;
        }
    }
    tform_end = strstr(tform, "\n};");
    if (!tform_end) tform_end = tform + strlen(tform);
    if (!copy_local_ref_after_a(tform, tform_end, "TNode.SNode STransform :local_", snode_local, sizeof(snode_local))) {
        _snprintf(snode_decl, sizeof(snode_decl) - 1, "STransform :local_S%s", name);
        snode_decl[sizeof(snode_decl) - 1] = 0;
        snode_pos = find_scene_decl_i(src, snode_decl);
        if (!snode_pos) {
            debug_line("EngineAudio room named transform stage miss snode_ref tform=\"%.64s\" decl=\"%s\" node=\"%s\"",
                       tform, snode_decl, node);
            goto done;
        }
        goto parse_snode;
    }

    _snprintf(snode_decl, sizeof(snode_decl) - 1, "STransform :local_%s", snode_local);
    snode_decl[sizeof(snode_decl) - 1] = 0;
    snode_pos = find_scene_decl_i(src, snode_decl);
    if (!snode_pos) {
        debug_line("EngineAudio room named transform stage miss snode_decl decl=\"%s\" node=\"%s\"",
                   snode_decl, node);
        goto done;
    }

parse_snode:
    snode_end = strstr(snode_pos, "\n};");
    if (!snode_end) snode_end = snode_pos + strlen(snode_pos);
    translation = find_i(snode_pos, "SSimpleTransform.Translation Vector3f(");
    if (!translation || translation > snode_end) {
        debug_line("EngineAudio room named transform stage miss translation snode=\"%.64s\" node=\"%s\"",
                   snode_pos, node);
        goto done;
    }
    translation += strlen("SSimpleTransform.Translation Vector3f(");
    x = strtod(translation, &endp);
    if (endp == translation) {
        debug_line("EngineAudio room named transform stage miss parse_x text=\"%.64s\" node=\"%s\"",
                   translation, node);
        goto done;
    }
    translation = endp;
    while (*translation == ' ' || *translation == '\t') translation++;
    if (*translation != ',') {
        debug_line("EngineAudio room named transform stage miss comma_x text=\"%.32s\" node=\"%s\"",
                   translation, node);
        goto done;
    }
    translation++;
    y = strtod(translation, &endp);
    if (endp == translation) {
        debug_line("EngineAudio room named transform stage miss parse_y text=\"%.64s\" node=\"%s\"",
                   translation, node);
        goto done;
    }
    translation = endp;
    while (*translation == ' ' || *translation == '\t') translation++;
    if (*translation != ',') {
        debug_line("EngineAudio room named transform stage miss comma_y text=\"%.32s\" node=\"%s\"",
                   translation, node);
        goto done;
    }
    translation++;
    z = strtod(translation, &endp);
    if (endp == translation) {
        debug_line("EngineAudio room named transform stage miss parse_z text=\"%.64s\" node=\"%s\"",
                   translation, node);
        goto done;
    }

    _snprintf(out, outsz - 1, "pos:%.4f,%.4f,%.4f", x, y, z);
    out[outsz - 1] = 0;
    ok = 1;

done:
    free(src);
    if (ok) {
        debug_line("EngineAudio inferred room named transform position=\"%s\" node=\"%s\" sidecar=\"%s\" scene=\"%s\"",
                   out, node, sidecar, scene_path);
    } else {
        debug_line("EngineAudio failed to infer room named transform position node=\"%s\" sidecar=\"%s\" scene=\"%s\"",
                 node, sidecar, scene_path);
    }
    return ok;
}

static const char *find_scene_decl_i(const char *src, const char *needle)
{
    const char *p;
    size_t n;

    if (!src || !needle || !needle[0]) return NULL;
    n = strlen(needle);
    p = src;
    while ((p = find_i(p, needle)) != NULL) {
        const char *line = p;
        char after = p[n];

        while (line > src && (line[-1] == ' ' || line[-1] == '\t')) line--;
        if ((line == src || line[-1] == '\n' || line[-1] == '\r') &&
            (after == ' ' || after == '\t' || after == '.' || after == '\r' || after == '\n' || after == '{')) {
            return p;
        }
        p += n;
    }
    return NULL;
}

static int infer_room_audio_source_position_a(const char *sidecar, char *out, size_t outsz)
{
    char root[MAX_PATH * 4];
    char texture_base[MAX_PATH];
    char scene_id[MAX_PATH];
    char material_base[MAX_PATH];
    char scene_rel[MAX_PATH * 2];
    char scene_path[MAX_PATH * 4];
    char shader_needle[512];
    char snode_local[128];
    char spoly_decl[256];
    const char *underscore;
    const char *geom;
    const char *geom_end;
    const char *spoly;
    const char *arr;
    const char *arr_end;
    const char *p;
    char *src = NULL;
    size_t id_len;
    double sx = 0.0, sy = 0.0, sz = 0.0;
    int count = 0;
    int ok = 0;

    if (out && outsz) out[0] = 0;
    if (!sidecar || !out || !outsz) return 0;
    if (!addon_root_from_path_a(sidecar, root, sizeof(root))) return 0;

    basename_no_ext_a(sidecar, texture_base, sizeof(texture_base));
    underscore = strchr(texture_base, '_');
    id_len = underscore ? (size_t)(underscore - texture_base) : strlen(texture_base);
    if (!id_len || id_len >= sizeof(scene_id)) return 0;
    memcpy(scene_id, texture_base, id_len);
    scene_id[id_len] = 0;
    if (underscore && underscore[1]) {
        lstrcpynA(material_base, underscore + 1, sizeof(material_base));
    } else {
        lstrcpynA(material_base, texture_base, sizeof(material_base));
    }

    _snprintf(scene_rel, sizeof(scene_rel) - 1, "Scenes\\Luder\\Room\\%s\\%s_core.bs", scene_id, scene_id);
    scene_rel[sizeof(scene_rel) - 1] = 0;
    path_join(scene_path, sizeof(scene_path), root, scene_rel);
    src = read_text_file_a(scene_path);
    if (!src) return 0;

    if (!find_room_geom_by_texture_file_a(src, texture_base, &geom)) {
        _snprintf(shader_needle, sizeof(shader_needle) - 1, "TGeometry.Shader RenderShader :local_%s_material", material_base);
        shader_needle[sizeof(shader_needle) - 1] = 0;
        geom = find_i(src, shader_needle);
        if (!geom) {
            _snprintf(shader_needle, sizeof(shader_needle) - 1, "TGeometry.Shader RenderShader :local_%s_MATERIAL", material_base);
            shader_needle[sizeof(shader_needle) - 1] = 0;
            geom = find_i(src, shader_needle);
        }
    }
    if (!geom) goto done;
    while (geom > src && _strnicmp(geom, "TPolygonGeometry :", 18) != 0) geom--;
    if (_strnicmp(geom, "TPolygonGeometry :", 18) != 0) goto done;
    geom_end = strstr(geom, "\n}");
    if (!geom_end) geom_end = geom + strlen(geom);
    if (!copy_local_ref_after_a(geom, geom_end, "TNode.SNode SPolygonGeometry :local_", snode_local, sizeof(snode_local))) {
        goto done;
    }

    _snprintf(spoly_decl, sizeof(spoly_decl) - 1, "SPolygonGeometry :local_%s", snode_local);
    spoly_decl[sizeof(spoly_decl) - 1] = 0;
    spoly = find_i(src, spoly_decl);
    if (!spoly) goto done;
    arr = find_i(spoly, "SPolygonGeometry.VertexArray Array_Vector3f [");
    if (!arr) goto done;
    arr_end = strstr(arr, "];");
    if (!arr_end) goto done;

    p = arr;
    while (p < arr_end) {
        double x, y, z;
        char *end;
        if (*p++ != '(') continue;
        x = strtod(p, &end);
        if (end == p || *end != ',') continue;
        p = end + 1;
        y = strtod(p, &end);
        if (end == p || *end != ',') continue;
        p = end + 1;
        z = strtod(p, &end);
        if (end == p || *end != ')') continue;
        sx += x;
        sy += y;
        sz += z;
        count++;
        p = end + 1;
    }
    if (count <= 0) goto done;
    _snprintf(out, outsz - 1, "pos:%.4f,%.4f,%.4f", sx / count, sy / count, sz / count);
    out[outsz - 1] = 0;
    ok = 1;

done:
    free(src);
    if (ok) {
        debug_line("EngineAudio inferred room source position=\"%s\" vertices=%d sidecar=\"%s\" scene=\"%s\"",
                   out, count, sidecar, scene_path);
    } else {
        debug_line("EngineAudio failed to infer room source position sidecar=\"%s\" scene=\"%s\"",
                 sidecar, scene_path);
    }
    return ok;
}

static int infer_room_audio_sound_anchor_path_a(const char *sidecar, char *out, size_t outsz)
{
    char root[MAX_PATH * 4];
    char texture_base[MAX_PATH];
    char scene_id[MAX_PATH];
    char material_base[MAX_PATH];
    char scene_rel[MAX_PATH * 2];
    char scene_path[MAX_PATH * 4];
    char target_pos[128];
    const char *underscore;
    const char *p;
    char *src = NULL;
    size_t id_len;
    double tx = 0.0, ty = 0.0, tz = 0.0;
    double best_dist = 1.0e30;
    char best_name[128];
    int have_target = 0;
    int have_best = 0;
    int exact = 0;

    if (out && outsz) out[0] = 0;
    if (!sidecar || !out || !outsz) return 0;
    if (!addon_root_from_path_a(sidecar, root, sizeof(root))) return 0;

    basename_no_ext_a(sidecar, texture_base, sizeof(texture_base));
    underscore = strchr(texture_base, '_');
    id_len = underscore ? (size_t)(underscore - texture_base) : strlen(texture_base);
    if (!id_len || id_len >= sizeof(scene_id)) return 0;
    memcpy(scene_id, texture_base, id_len);
    scene_id[id_len] = 0;
    if (underscore && underscore[1]) {
        lstrcpynA(material_base, underscore + 1, sizeof(material_base));
    } else {
        lstrcpynA(material_base, texture_base, sizeof(material_base));
    }

    _snprintf(scene_rel, sizeof(scene_rel) - 1, "Scenes\\Luder\\Room\\%s\\%s_core.bs", scene_id, scene_id);
    scene_rel[sizeof(scene_rel) - 1] = 0;
    path_join(scene_path, sizeof(scene_path), root, scene_rel);
    src = read_text_file_a(scene_path);
    if (!src) return 0;

    if (infer_room_audio_source_position_a(sidecar, target_pos, sizeof(target_pos))) {
        if (sscanf(target_pos, "pos:%lf,%lf,%lf", &tx, &ty, &tz) == 3) {
            have_target = 1;
        }
    }

    p = src;
    while ((p = find_i(p, "SSimpleTransform.Translation Vector3f(")) != NULL) {
        const char *block_start = p;
        const char *block_end = strstr(p, "\n};");
        const char *vec = p;
        char obj_name[128];
        char anchor_name[128];
        char token[128];
        double x, y, z;
        int is_match = 0;
        while (block_start > src && _strnicmp(block_start, "STransform :local_Ssound_", 25) != 0) {
            block_start--;
        }
        if (_strnicmp(block_start, "STransform :local_Ssound_", 25) != 0) {
            p = vec + strlen("SSimpleTransform.Translation Vector3f(");
            continue;
        }
        if (!block_end) block_end = p + strlen(p);
        vec += strlen("SSimpleTransform.Translation Vector3f(");
        if (sscanf(vec, " %lf , %lf , %lf", &x, &y, &z) != 3) {
            p = block_end;
            continue;
        }
        if (!copy_quoted_value_after_a(block_start, "Object.Name", obj_name, sizeof(obj_name))) {
            p = block_end;
            continue;
        }
        if (_strnicmp(obj_name, "Ssound_", 7) == 0) {
            lstrcpynA(anchor_name, obj_name + 1, sizeof(anchor_name));
        } else if (_strnicmp(obj_name, "sound_", 6) == 0) {
            lstrcpynA(anchor_name, obj_name, sizeof(anchor_name));
        } else {
            p = block_end;
            continue;
        }
        if (strlen(anchor_name) + 8 >= sizeof(anchor_name)) {
            p = block_end;
            continue;
        }
        lstrcpynA(token, anchor_name + 6, sizeof(token));
        if (contains_i(material_base, token) || contains_i(texture_base, token)) {
            is_match = 1;
        }
        if (is_match || (!exact && have_target)) {
            double dx = x - tx;
            double dy = y - ty;
            double dz = z - tz;
            double dist = is_match ? 0.0 : dx * dx + dy * dy + dz * dz;
            if (is_match || dist < best_dist) {
                lstrcpynA(best_name, anchor_name, sizeof(best_name));
                best_dist = dist;
                have_best = 1;
                exact = is_match;
            }
        } else if (!have_best) {
            lstrcpynA(best_name, anchor_name, sizeof(best_name));
            have_best = 1;
        }
        p = block_end;
    }

    free(src);
    if (!have_best) {
        debug_line("EngineAudio failed to infer room sound anchor sidecar=\"%s\" scene=\"%s\"", sidecar, scene_path);
        return 0;
    }

    _snprintf(out, outsz - 1, "/Room01/%s/%s_offset", best_name, best_name);
    out[outsz - 1] = 0;
    debug_line("EngineAudio inferred room sound anchor path=\"%s\" sidecar=\"%s\" scene=\"%s\" exact=%d distance=%.3f",
               out, sidecar, scene_path, exact, best_dist);
    return 1;
}

static int path_is_addon_toy_a(const char *path)
{
    if (!path) return 0;
    return contains_i(path, "\\Shared\\Item\\") ||
           contains_i(path, "/Shared/Item/") ||
           contains_i(path, "\\Luder\\Item\\") ||
           contains_i(path, "/Luder/Item/");
}

static int path_is_addon_room_a(const char *path)
{
    if (!path) return 0;
    return contains_i(path, "\\Luder\\Room\\") ||
           contains_i(path, "/Luder/Room/");
}

static int path_contains_scene_cloth_bs_a(const char *path)
{
    return path &&
           ((contains_i(path, "Scenes\\Shared\\Cloth\\") || contains_i(path, "Scenes/Shared/Cloth/")) &&
            ends_with_i(path, ".bs"));
}

static int find_sidecar_recursive_a(const char *dir, const char *base, char *out, size_t outsz, int depth)
{
    char pattern[MAX_PATH * 4];
    WIN32_FIND_DATAA data;
    HANDLE h;
    if (!dir || !base || !out || depth > 12) return 0;
    path_join(pattern, sizeof(pattern), dir, "*");
    h = FindFirstFileA(pattern, &data);
    if (h == INVALID_HANDLE_VALUE) return 0;
    do {
        char child[MAX_PATH * 4];
        if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0) continue;
        path_join(child, sizeof(child), dir, data.cFileName);
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (find_sidecar_recursive_a(child, base, out, outsz, depth + 1)) {
                FindClose(h);
                return 1;
            }
        } else if (ends_with_i(data.cFileName, ".webm")) {
            char child_base[MAX_PATH];
            basename_no_ext_a(data.cFileName, child_base, sizeof(child_base));
            if (_stricmp(child_base, base) == 0) {
                lstrcpynA(out, child, (int)outsz);
                FindClose(h);
                return 1;
            }
        }
    } while (FindNextFileA(h, &data));
    FindClose(h);
    return 0;
}

static int find_active_mod_sidecar_a(const char *scene_path, char *sidecar, size_t sidecar_sz)
{
    char dll_path[MAX_PATH * 2];
    char bin_dir[MAX_PATH * 2];
    char game_dir[MAX_PATH * 2];
    char mod_dir[MAX_PATH * 2];
    char base[MAX_PATH];
    if (sidecar && sidecar_sz) sidecar[0] = 0;
    if (!scene_path || !sidecar || !sidecar_sz) return 0;
    basename_no_ext_a(scene_path, base, sizeof(base));
    if (!base[0]) return 0;
    if (!self_module || !GetModuleFileNameA(self_module, dll_path, sizeof(dll_path))) return 0;
    dll_path[sizeof(dll_path) - 1] = 0;
    strcpy(bin_dir, dll_path);
    dirname_inplace(bin_dir);
    strcpy(game_dir, bin_dir);
    dirname_inplace(game_dir);
    path_join(mod_dir, sizeof(mod_dir), game_dir, "Mod\\ActiveMod");
    return find_sidecar_recursive_a(mod_dir, base, sidecar, sidecar_sz, 0);
}

static int temp_bsb_to_addon_scene_a(const char *temp_path, char *out, size_t outsz)
{
    char dll_path[MAX_PATH * 2];
    char bin_dir[MAX_PATH * 2];
    char game_dir[MAX_PATH * 2];
    const char *marker;
    const char *at;
    const char *tail;
    char addon[MAX_PATH * 2];
    char rel[MAX_PATH * 2];
    size_t addon_len, rel_len, i;
    if (out && outsz) out[0] = 0;
    if (!temp_path || !out || !outsz) return 0;
    if (!contains_i(temp_path, "\\Temp\\addon_") && !contains_i(temp_path, "/Temp/addon_")) return 0;
    if (!contains_i(temp_path, "@Scenes#Shared#Cloth#") || !ends_with_i(temp_path, ".[bsb]")) return 0;

    marker = strstr(temp_path, "addon_");
    if (!marker) return 0;
    marker += 6;
    at = strchr(marker, '@');
    if (!at) return 0;
    addon_len = (size_t)(at - marker);
    if (!addon_len || addon_len >= sizeof(addon)) return 0;
    memcpy(addon, marker, addon_len);
    addon[addon_len] = 0;

    tail = at + 1;
    rel_len = strlen(tail);
    if (rel_len < 6 || rel_len >= sizeof(rel)) return 0;
    memcpy(rel, tail, rel_len + 1);
    if (ends_with_i(rel, ".[bsb]")) {
        rel[strlen(rel) - 6] = 0;
        lstrcatA(rel, ".bs");
    }
    for (i = 0; rel[i]; i++) {
        if (rel[i] == '#') rel[i] = '\\';
    }

    if (!self_module || !GetModuleFileNameA(self_module, dll_path, sizeof(dll_path))) return 0;
    dll_path[sizeof(dll_path) - 1] = 0;
    strcpy(bin_dir, dll_path);
    dirname_inplace(bin_dir);
    strcpy(game_dir, bin_dir);
    dirname_inplace(game_dir);
    _snprintf(out, outsz - 1, "%s\\Addons\\%s\\%s", game_dir, addon, rel);
    out[outsz - 1] = 0;
    return GetFileAttributesA(out) != INVALID_FILE_ATTRIBUTES;
}

static int temp_bsb_to_addon_script_a(const char *temp_path, char *out, size_t outsz)
{
    char dll_path[MAX_PATH * 2];
    char bin_dir[MAX_PATH * 2];
    char game_dir[MAX_PATH * 2];
    const char *marker;
    const char *at;
    const char *tail;
    char addon[MAX_PATH * 2];
    char rel[MAX_PATH * 2];
    size_t addon_len, rel_len, i;
    if (out && outsz) out[0] = 0;
    if (!temp_path || !out || !outsz) return 0;
    if (!contains_i(temp_path, "\\Temp\\addon_") && !contains_i(temp_path, "/Temp/addon_")) return 0;
    if (!contains_i(temp_path, "@Scripts#") || !ends_with_i(temp_path, ".[bsb]")) return 0;

    marker = strstr(temp_path, "addon_");
    if (!marker) return 0;
    marker += 6;
    at = strchr(marker, '@');
    if (!at) return 0;
    addon_len = (size_t)(at - marker);
    if (!addon_len || addon_len >= sizeof(addon)) return 0;
    memcpy(addon, marker, addon_len);
    addon[addon_len] = 0;

    tail = at + 1;
    rel_len = strlen(tail);
    if (rel_len < 6 || rel_len >= sizeof(rel)) return 0;
    memcpy(rel, tail, rel_len + 1);
    if (ends_with_i(rel, ".[bsb]")) {
        rel[strlen(rel) - 6] = 0;
        lstrcatA(rel, ".bs");
    }
    for (i = 0; rel[i]; i++) {
        if (rel[i] == '#') rel[i] = '\\';
    }

    if (!self_module || !GetModuleFileNameA(self_module, dll_path, sizeof(dll_path))) return 0;
    dll_path[sizeof(dll_path) - 1] = 0;
    strcpy(bin_dir, dll_path);
    dirname_inplace(bin_dir);
    strcpy(game_dir, bin_dir);
    dirname_inplace(game_dir);
    _snprintf(out, outsz - 1, "%s\\Addons\\%s\\%s", game_dir, addon, rel);
    out[outsz - 1] = 0;
    return GetFileAttributesA(out) != INVALID_FILE_ATTRIBUTES;
}

static char *replace_all_alloc(const char *src, const char *needle, const char *replacement)
{
    size_t src_len, needle_len, repl_len, count = 0;
    const char *p;
    char *out, *w;
    if (!src || !needle || !replacement) return NULL;
    needle_len = strlen(needle);
    if (!needle_len) return NULL;
    repl_len = strlen(replacement);
    src_len = strlen(src);
    for (p = src; (p = strstr(p, needle)) != NULL; p += needle_len) count++;
    out = (char*)malloc(src_len + count * (repl_len - needle_len) + 1);
    if (!out) return NULL;
    w = out;
    p = src;
    while (*p) {
        const char *hit = strstr(p, needle);
        size_t n;
        if (!hit) {
            strcpy(w, p);
            break;
        }
        n = (size_t)(hit - p);
        memcpy(w, p, n);
        w += n;
        memcpy(w, replacement, repl_len);
        w += repl_len;
        p = hit + needle_len;
    }
    return out;
}

static char *rewrite_scene_for_video_a(const char *src, const char *base, const char *sidecar)
{
    char needle[MAX_PATH * 4];
    char repl[MAX_PATH * 4];
    char sidecar_bs[MAX_PATH * 4];
    char tex_ref_needle[MAX_PATH * 4];
    char tex_ref_repl[MAX_PATH * 4];
    char tex_decl_needle[MAX_PATH * 4];
    char tex_decl_repl[MAX_PATH * 4];
    char *rewritten;
    char *rewritten2;
    size_t i;
    if (!src || !base || !sidecar) return NULL;
    lstrcpynA(sidecar_bs, sidecar, sizeof(sidecar_bs));
    for (i = 0; sidecar_bs[i]; i++) {
        if (sidecar_bs[i] == '\\') sidecar_bs[i] = '/';
    }

    _snprintf(tex_ref_needle, sizeof(tex_ref_needle), "ShaderTexture.Texture Texture2D :local_%s_TEX2D", base);
    _snprintf(tex_ref_repl, sizeof(tex_ref_repl), "ShaderTexture.Texture Texture2DVideo :local_%s_TEX2D", base);
    rewritten = replace_all_alloc(src, tex_ref_needle, tex_ref_repl);
    if (!rewritten || strcmp(rewritten, src) == 0) {
        if (rewritten) free(rewritten);
        return NULL;
    }

    _snprintf(tex_decl_needle, sizeof(tex_decl_needle), "Texture2D :local_%s_TEX2D", base);
    _snprintf(tex_decl_repl, sizeof(tex_decl_repl), "Texture2DVideo :local_%s_TEX2D", base);
    rewritten2 = replace_all_alloc(rewritten, tex_decl_needle, tex_decl_repl);
    free(rewritten);
    if (!rewritten2) return NULL;
    rewritten = rewritten2;

    _snprintf(needle, sizeof(needle), "FileObject.FileName \"Shared/Cloth/%s\"", base);
    _snprintf(repl, sizeof(repl), "FileObject.FileName \"%s.avi\"", sidecar_bs);
    rewritten2 = replace_all_alloc(rewritten, needle, repl);
    free(rewritten);
    if (!rewritten2 || strcmp(rewritten2, src) == 0) {
        if (rewritten2) free(rewritten2);
        return NULL;
    }
    if (!contains_i(rewritten2, "Texture2DVideo")) {
        free(rewritten2);
        return NULL;
    }
    if (!contains_i(rewritten2, ".webm.avi")) {
        free(rewritten2);
        return NULL;
    }
    rewritten = rewritten2;
    if (!rewritten || strcmp(rewritten, src) == 0) {
        if (rewritten) free(rewritten);
        return NULL;
    }
    return rewritten;
}

static int create_scene_video_rewrite_w(LPCWSTR original, wchar_t *redirect, size_t redirect_count)
{
    char original_mb[MAX_PATH * 4];
    char source_mb[MAX_PATH * 4];
    char sidecar[MAX_PATH * 4];
    char base[MAX_PATH];
    char cache_dir[MAX_PATH * 2];
    char temp_path[MAX_PATH * 4];
    HANDLE in = INVALID_HANDLE_VALUE;
    HANDLE out = INVALID_HANDLE_VALUE;
    DWORD size, got, wrote;
    char *src = NULL;
    char *rewritten = NULL;
    int ok = 0;

    if (!original || !redirect || !redirect_count) return 0;
    wide_to_mb(original, original_mb, sizeof(original_mb));
    source_mb[0] = 0;
    if (path_contains_scene_cloth_bs_a(original_mb)) {
        lstrcpynA(source_mb, original_mb, sizeof(source_mb));
    } else if (!temp_bsb_to_addon_scene_a(original_mb, source_mb, sizeof(source_mb))) {
        return 0;
    }
    if (!find_active_mod_sidecar_a(source_mb, sidecar, sizeof(sidecar))) return 0;
    basename_no_ext_a(source_mb, base, sizeof(base));
    if (!base[0]) return 0;

    {
        wchar_t source_w[MAX_PATH * 4];
        MultiByteToWideChar(CP_ACP, 0, source_mb, -1, source_w, (int)(sizeof(source_w) / sizeof(source_w[0])));
        in = real_CreateFileW(source_w, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (in == INVALID_HANDLE_VALUE) return 0;
    size = GetFileSize(in, NULL);
    if (size == INVALID_FILE_SIZE || size > 16 * 1024 * 1024) goto done;
    src = (char*)malloc(size + 1);
    if (!src) goto done;
    if (!real_ReadFile(in, src, size, &got, NULL) || got != size) goto done;
    src[size] = 0;

    rewritten = rewrite_scene_for_video_a(src, base, sidecar);
    if (!rewritten) goto done;
    webm_component_dir_a(cache_dir, sizeof(cache_dir),
                         "NC-TK17-WebM-Cache");
    if (!cache_dir[0]) goto done;
    CreateDirectoryA(cache_dir, NULL);
    _snprintf(temp_path, sizeof(temp_path), "%s\\%s.video.bs", cache_dir, base);
    out = CreateFileA(temp_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (out == INVALID_HANDLE_VALUE) goto done;
    if (!WriteFile(out, rewritten, (DWORD)strlen(rewritten), &wrote, NULL)) goto done;
    CloseHandle(out);
    out = INVALID_HANDLE_VALUE;
    MultiByteToWideChar(CP_ACP, 0, temp_path, -1, redirect, (int)redirect_count);
    debug_line("SceneVideoRewrite original=\"%s\" source=\"%s\" temp=\"%s\" sidecar=\"%s\"", original_mb, source_mb, temp_path, sidecar);
    if (active_video_scene_sidecar[0] && _stricmp(active_video_scene_sidecar, sidecar) != 0) {
        debug_line("SceneVideoRewrite sidecar changed; clearing old video audio old=\"%s\" new=\"%s\"",
                 active_video_scene_sidecar, sidecar);
        clear_all_video_texture_slots();
    }
    lstrcpynA(active_video_scene_base, base, sizeof(active_video_scene_base));
    lstrcpynA(active_video_scene_sidecar, sidecar, sizeof(active_video_scene_sidecar));
    active_video_scene_tick = GetTickCount();
    ok = 1;

done:
    if (out != INVALID_HANDLE_VALUE) CloseHandle(out);
    if (in != INVALID_HANDLE_VALUE) real_CloseHandle(in);
    if (src) free(src);
    if (rewritten) free(rewritten);
    return ok;
}

static int is_movie_path_a(const char *s)
{
    return contains_i(s, "Movies") || contains_i(s, "Movie");
}

static int ends_with_i(const char *s, const char *suffix)
{
    size_t ls, lx;
    if (!s || !suffix) return 0;
    ls = strlen(s);
    lx = strlen(suffix);
    if (lx > ls) return 0;
    return _stricmp(s + ls - lx, suffix) == 0;
}

static void wide_to_mb(LPCWSTR in, char *out, size_t outsz)
{
    if (!outsz) return;
    out[0] = 0;
    if (!in) return;
    WideCharToMultiByte(CP_ACP, 0, in, -1, out, (int)outsz, NULL, NULL);
}

static int guid_equal(REFGUID a, REFGUID b)
{
    return a && b && memcmp(a, b, sizeof(GUID)) == 0;
}

static void guid_to_text(REFGUID g, char *out, size_t outsz)
{
    if (!outsz) return;
    out[0] = 0;
    if (!g) return;
    _snprintf(out, outsz,
              "{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
              g->Data1, g->Data2, g->Data3,
              g->Data4[0], g->Data4[1], g->Data4[2], g->Data4[3],
              g->Data4[4], g->Data4[5], g->Data4[6], g->Data4[7]);
    out[outsz - 1] = 0;
}

static int ends_with_w_i(LPCWSTR s, LPCWSTR suffix)
{
    size_t ls, lx;
    if (!s || !suffix) return 0;
    ls = wcslen(s);
    lx = wcslen(suffix);
    if (lx > ls) return 0;
    return _wcsicmp(s + ls - lx, suffix) == 0;
}

static int is_movie_avi_pattern_w(LPCWSTR p)
{
    char mb[MAX_PATH * 4];
    wide_to_mb(p, mb, sizeof(mb));
    return is_movie_path_a(mb) && (ends_with_i(mb, "\\*.avi") || ends_with_i(mb, "/*.avi"));
}

static int webm_alias_to_real_w(LPCWSTR alias, wchar_t *real, size_t real_count)
{
    size_t len;
    if (!alias || !real || !real_count) return 0;
    if (!ends_with_w_i(alias, L".webm.avi")) return 0;
    len = wcslen(alias);
    if (len - 4 + 1 > real_count) return 0;
    wcsncpy(real, alias, len - 4);
    real[len - 4] = 0;
    return 1;
}

static void alias_basename_w(LPCWSTR path, wchar_t *out, size_t out_count)
{
    const wchar_t *slash;
    if (!out_count) return;
    out[0] = 0;
    if (!path) return;
    slash = wcsrchr(path, L'\\');
    if (!slash) slash = wcsrchr(path, L'/');
    lstrcpynW(out, slash ? slash + 1 : path, (int)out_count);
}

static void avi_pattern_to_webm_pattern(LPCWSTR avi_pattern, wchar_t *webm_pattern, size_t webm_count)
{
    wchar_t *slash;
    if (!webm_count) return;
    webm_pattern[0] = 0;
    if (!avi_pattern) return;
    lstrcpynW(webm_pattern, avi_pattern, (int)webm_count);
    slash = wcsrchr(webm_pattern, L'\\');
    if (!slash) slash = wcsrchr(webm_pattern, L'/');
    if (!slash) return;
    lstrcpynW(slash + 1, L"*.webm", (int)(webm_count - (slash + 1 - webm_pattern)));
}

static void remove_virtual_find(HANDLE h)
{
    int i;
    for (i = 0; i < (int)(sizeof(virtual_finds) / sizeof(virtual_finds[0])); i++) {
        if (virtual_finds[i].handle == h) {
            memset(&virtual_finds[i], 0, sizeof(virtual_finds[i]));
            return;
        }
    }
}

static void remember_webm_handle(HANDLE h)
{
    int i;
    if (h == INVALID_HANDLE_VALUE || !h) return;
    for (i = 0; i < (int)(sizeof(webm_handles) / sizeof(webm_handles[0])); i++) {
        if (!webm_handles[i] || webm_handles[i] == h) {
            webm_handles[i] = h;
            return;
        }
    }
}

static int is_webm_handle(HANDLE h)
{
    int i;
    for (i = 0; i < (int)(sizeof(webm_handles) / sizeof(webm_handles[0])); i++) {
        if (webm_handles[i] == h) return 1;
    }
    return 0;
}

static void forget_webm_handle(HANDLE h)
{
    int i;
    for (i = 0; i < (int)(sizeof(webm_handles) / sizeof(webm_handles[0])); i++) {
        if (webm_handles[i] == h) webm_handles[i] = NULL;
    }
}

static void remember_scene_handle(HANDLE h)
{
    int i;
    if (h == INVALID_HANDLE_VALUE || !h) return;
    for (i = 0; i < (int)(sizeof(scene_handles) / sizeof(scene_handles[0])); i++) {
        if (!scene_handles[i] || scene_handles[i] == h) {
            scene_handles[i] = h;
            return;
        }
    }
}

static unsigned int fnv1a_hash_a(const char *s)
{
    unsigned int h = 2166136261u;
    if (!s) return h;
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 16777619u;
    }
    return h;
}

static void ensure_dir_a(const char *path)
{
    char tmp[MAX_PATH * 4];
    char *p;
    if (!path || !path[0]) return;
    lstrcpynA(tmp, path, sizeof(tmp));
    for (p = tmp; *p; p++) {
        if (*p == '/' || *p == '\\') {
            char c = *p;
            *p = 0;
            if (tmp[0] && tmp[1] == ':') CreateDirectoryA(tmp, NULL);
            *p = c;
        }
    }
    CreateDirectoryA(tmp, NULL);
}

static void cache_root_a(char *out, size_t outsz)
{
    webm_component_dir_a(out, outsz, "NC-TK17-WebM-Cache");
}

static int path_under_dir_a(const char *path, const char *dir)
{
    size_t n;
    if (!path || !dir || !path[0] || !dir[0]) return 0;
    n = strlen(dir);
    if (_strnicmp(path, dir, n) != 0) return 0;
    return path[n] == 0 || path[n] == '\\' || path[n] == '/';
}

static void json_write_string_a(FILE *f, const char *s)
{
    const unsigned char *p = (const unsigned char*)s;
    fputc('"', f);
    if (p) {
        while (*p) {
            if (*p == '\\' || *p == '"') {
                fputc('\\', f);
                fputc(*p, f);
            } else if (*p == '\r') {
                fputs("\\r", f);
            } else if (*p == '\n') {
                fputs("\\n", f);
            } else if (*p == '\t') {
                fputs("\\t", f);
            } else if (*p < 32) {
                fprintf(f, "\\u%04x", (unsigned int)*p);
            } else {
                fputc(*p, f);
            }
            p++;
        }
    }
    fputc('"', f);
}

static const char *json_read_string_a(const char *p, char *out, size_t outsz)
{
    size_t n = 0;
    if (out && outsz) out[0] = 0;
    if (!p || *p != '"') return NULL;
    p++;
    while (*p && *p != '"') {
        char c = *p++;
        if (c == '\\') {
            c = *p++;
            if (!c) return NULL;
            if (c == 'n') c = '\n';
            else if (c == 'r') c = '\r';
            else if (c == 't') c = '\t';
            else if (c == 'u') {
                int i;
                for (i = 0; i < 4 && *p; i++) p++;
                c = '?';
            }
        }
        if (out && outsz && n + 1 < outsz) out[n++] = c;
    }
    if (*p != '"') return NULL;
    if (out && outsz) out[n] = 0;
    return p + 1;
}

typedef struct {
    char sidecar[MAX_PATH * 4];
    char sound_id[128];
    char files[4][MAX_PATH * 4];
    int file_count;
} audio_cache_manifest_entry_t;

static int audio_cache_manifest_load_a(audio_cache_manifest_entry_t *entries, int max_entries)
{
    char root[MAX_PATH * 4];
    char manifest_path[MAX_PATH * 4];
    FILE *f;
    long len;
    char *text;
    char *p;
    int count = 0;
    if (!entries || max_entries <= 0) return 0;
    cache_root_a(root, sizeof(root));
    if (!root[0]) return 0;
    path_join(manifest_path, sizeof(manifest_path), root, "audio-cache.json");
    f = fopen(manifest_path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    len = ftell(f);
    if (len <= 0 || len > 1024 * 1024) { fclose(f); return 0; }
    rewind(f);
    text = (char*)malloc((size_t)len + 1);
    if (!text) { fclose(f); return 0; }
    if (fread(text, 1, (size_t)len, f) != (size_t)len) { free(text); fclose(f); return 0; }
    fclose(f);
    text[len] = 0;
    p = text;
    while (count < max_entries && (p = (char*)strstr(p, "\"sidecar\"")) != NULL) {
        char *q;
        audio_cache_manifest_entry_t *e = &entries[count];
        memset(e, 0, sizeof(*e));
        q = strchr(p, ':');
        if (!q) break;
        while (*++q == ' ' || *q == '\t' || *q == '\r' || *q == '\n') {}
        q = (char*)json_read_string_a(q, e->sidecar, sizeof(e->sidecar));
        if (!q) { p++; continue; }
        p = q;
        q = strstr(p, "\"sound_id\"");
        if (q) {
            q = strchr(q, ':');
            if (q) {
                while (*++q == ' ' || *q == '\t' || *q == '\r' || *q == '\n') {}
                json_read_string_a(q, e->sound_id, sizeof(e->sound_id));
            }
        }
        q = strstr(p, "\"files\"");
        if (q) {
            q = strchr(q, '[');
            if (q) {
                q++;
                while (*q && *q != ']' && e->file_count < 4) {
                    while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n' || *q == ',') q++;
                    if (*q == '"') {
                        q = (char*)json_read_string_a(q, e->files[e->file_count], sizeof(e->files[e->file_count]));
                        if (!q) break;
                        e->file_count++;
                    } else {
                        q++;
                    }
                }
            }
        }
        count++;
    }
    free(text);
    return count;
}

static int audio_cache_manifest_write_a(audio_cache_manifest_entry_t *entries, int count)
{
    char root[MAX_PATH * 4];
    char manifest_path[MAX_PATH * 4];
    FILE *f;
    int i, j, written = 0;
    cache_root_a(root, sizeof(root));
    if (!root[0]) return 0;
    ensure_dir_a(root);
    path_join(manifest_path, sizeof(manifest_path), root, "audio-cache.json");
    f = fopen(manifest_path, "wb");
    if (!f) return 0;
    fputs("{\r\n  \"version\": 1,\r\n  \"entries\": [\r\n", f);
    for (i = 0; i < count; i++) {
        audio_cache_manifest_entry_t *e = &entries[i];
        if (!e->sidecar[0] || e->file_count <= 0) continue;
        if (written) fputs(",\r\n", f);
        fputs("    {\r\n      \"sidecar\": ", f);
        json_write_string_a(f, e->sidecar);
        fputs(",\r\n      \"sound_id\": ", f);
        json_write_string_a(f, e->sound_id);
        fputs(",\r\n      \"files\": [", f);
        for (j = 0; j < e->file_count; j++) {
            if (j) fputs(", ", f);
            json_write_string_a(f, e->files[j]);
        }
        fputs("]\r\n    }", f);
        written++;
    }
    fputs("\r\n  ]\r\n}\r\n", f);
    fclose(f);
    return 1;
}

static void audio_cache_manifest_record_a(const char *sidecar, const char *sound_id,
                                          const char *cache_path, const char *snf_path)
{
    audio_cache_manifest_entry_t *entries;
    char root[MAX_PATH * 4];
    int count, i, idx = -1;
    if (!sidecar || !sidecar[0] || !sound_id || !sound_id[0]) return;
    cache_root_a(root, sizeof(root));
    if (!root[0]) return;
    entries = (audio_cache_manifest_entry_t*)calloc(256, sizeof(audio_cache_manifest_entry_t));
    if (!entries) return;
    count = audio_cache_manifest_load_a(entries, 256);
    for (i = 0; i < count; i++) {
        if (_stricmp(entries[i].sidecar, sidecar) == 0 || _stricmp(entries[i].sound_id, sound_id) == 0) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        if (count >= 256) idx = count - 1;
        else idx = count++;
    }
    memset(&entries[idx], 0, sizeof(entries[idx]));
    lstrcpynA(entries[idx].sidecar, sidecar, sizeof(entries[idx].sidecar));
    lstrcpynA(entries[idx].sound_id, sound_id, sizeof(entries[idx].sound_id));
    if (cache_path && cache_path[0] && path_under_dir_a(cache_path, root)) {
        lstrcpynA(entries[idx].files[entries[idx].file_count++], cache_path, sizeof(entries[idx].files[0]));
    }
    if (snf_path && snf_path[0] && path_under_dir_a(snf_path, root) && entries[idx].file_count < 4) {
        lstrcpynA(entries[idx].files[entries[idx].file_count++], snf_path, sizeof(entries[idx].files[0]));
    }
    if (entries[idx].file_count > 0) audio_cache_manifest_write_a(entries, count);
    free(entries);
}

static void audio_cache_manifest_cleanup_a(void)
{
    static int ran;
    audio_cache_manifest_entry_t *entries;
    audio_cache_manifest_entry_t *kept;
    char root[MAX_PATH * 4];
    int count, kept_count = 0, i, j, changed = 0;
    if (ran) return;
    ran = 1;
    cache_root_a(root, sizeof(root));
    if (!root[0]) return;
    entries = (audio_cache_manifest_entry_t*)calloc(256, sizeof(audio_cache_manifest_entry_t));
    kept = (audio_cache_manifest_entry_t*)calloc(256, sizeof(audio_cache_manifest_entry_t));
    if (!entries || !kept) {
        if (entries) free(entries);
        if (kept) free(kept);
        return;
    }
    count = audio_cache_manifest_load_a(entries, 256);
    for (i = 0; i < count; i++) {
        if (entries[i].sidecar[0] && GetFileAttributesA(entries[i].sidecar) != INVALID_FILE_ATTRIBUTES) {
            audio_cache_manifest_entry_t fixed;
            memset(&fixed, 0, sizeof(fixed));
            lstrcpynA(fixed.sidecar, entries[i].sidecar, sizeof(fixed.sidecar));
            lstrcpynA(fixed.sound_id, entries[i].sound_id, sizeof(fixed.sound_id));
            for (j = 0; j < entries[i].file_count && fixed.file_count < 4; j++) {
                const char *path = entries[i].files[j];
                if (path[0] && path_under_dir_a(path, root) &&
                    GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
                    lstrcpynA(fixed.files[fixed.file_count++], path, sizeof(fixed.files[0]));
                } else {
                    changed = 1;
                    if (path[0]) {
                        debug_line("NC-TK17-WebM pruned missing audio cache entry \"%s\"", path);
                    }
                }
            }
            if (fixed.file_count > 0) {
                kept[kept_count++] = fixed;
            } else {
                changed = 1;
                debug_line("NC-TK17-WebM removed empty audio cache manifest entry sidecar=\"%s\"",
                         entries[i].sidecar);
            }
            continue;
        }
        changed = 1;
        for (j = 0; j < entries[i].file_count; j++) {
            const char *path = entries[i].files[j];
            if (path[0] && path_under_dir_a(path, root) && DeleteFileA(path)) {
                debug_line("NC-TK17-WebM removed stale audio cache \"%s\"", path);
            }
        }
    }
    if (changed) audio_cache_manifest_write_a(kept, kept_count);
    free(entries);
    free(kept);
}

static void remember_engine_sound_cache_ex(const char *sound_id, const char *cache_path, const char *snf_path)
{
    int i, free_i = -1;
    (void)snf_path;
    if (!sound_id || !sound_id[0] || !cache_path || !cache_path[0]) return;
    for (i = 0; i < (int)(sizeof(engine_sound_cache) / sizeof(engine_sound_cache[0])); i++) {
        if (engine_sound_cache[i].active && _stricmp(engine_sound_cache[i].sound_id, sound_id) == 0) {
            lstrcpynA(engine_sound_cache[i].cache_path, cache_path, sizeof(engine_sound_cache[i].cache_path));
            return;
        }
        if (!engine_sound_cache[i].active && free_i < 0) free_i = i;
    }
    if (free_i < 0) free_i = 0;
    engine_sound_cache[free_i].active = 1;
    lstrcpynA(engine_sound_cache[free_i].sound_id, sound_id, sizeof(engine_sound_cache[free_i].sound_id));
    lstrcpynA(engine_sound_cache[free_i].cache_path, cache_path, sizeof(engine_sound_cache[free_i].cache_path));
}

static void remember_engine_sound_cache(const char *sound_id, const char *cache_path)
{
    remember_engine_sound_cache_ex(sound_id, cache_path, NULL);
}

static int engine_sound_redirect_path_a(const char *path, char *out, size_t outsz)
{
    int i;
    char base[MAX_PATH];
    if (out && outsz) out[0] = 0;
    if (!path || !out || !outsz) return 0;
    if (!contains_i(path, "Shared\\Effect\\") && !contains_i(path, "Shared/Effect/")) return 0;
    if (!ends_with_i(path, ".ogg")) return 0;
    basename_no_ext_a(path, base, sizeof(base));
    for (i = 0; i < (int)(sizeof(engine_sound_cache) / sizeof(engine_sound_cache[0])); i++) {
        if (!engine_sound_cache[i].active) continue;
        if (_stricmp(engine_sound_cache[i].sound_id, base) == 0) {
            lstrcpynA(out, engine_sound_cache[i].cache_path, (int)outsz);
            return 1;
        }
    }
    return 0;
}

static int write_engine_sound_snf_a(const char *snf_path, double length_seconds)
{
    HANDLE h;
    DWORD wrote = 0;
    char text[128];
    if (!snf_path || !snf_path[0]) return 0;
    if (length_seconds <= 0.0 || length_seconds > 86400.0) length_seconds = 3600.0;
    _snprintf(text, sizeof(text) - 1, ".Length %.3f;\r\n", length_seconds);
    text[sizeof(text) - 1] = 0;
    h = real_CreateFileA ? real_CreateFileA(snf_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
                         : CreateFileA(snf_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    if (!WriteFile(h, text, (DWORD)strlen(text), &wrote, NULL)) {
        if (real_CloseHandle) real_CloseHandle(h); else CloseHandle(h);
        return 0;
    }
    if (real_CloseHandle) real_CloseHandle(h); else CloseHandle(h);
    return 1;
}

typedef struct {
    ISampleGrabberCB iface;
    LONG refs;
    HANDLE wav;
    DWORD data_bytes;
    DWORD max_bytes;
} wav_grabber_cb_t;

static HRESULT STDMETHODCALLTYPE wav_cb_QueryInterface(ISampleGrabberCB *self, REFIID riid, void **out)
{
    if (!out) return E_POINTER;
    *out = NULL;
    if (guid_equal(riid, &IID_IUnknown_) || guid_equal(riid, &IID_ISampleGrabberCB_)) {
        *out = self;
        InterlockedIncrement(&((wav_grabber_cb_t*)self)->refs);
        return S_OK;
    }
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE wav_cb_AddRef(ISampleGrabberCB *self)
{
    return (ULONG)InterlockedIncrement(&((wav_grabber_cb_t*)self)->refs);
}

static ULONG STDMETHODCALLTYPE wav_cb_Release(ISampleGrabberCB *self)
{
    LONG refs = InterlockedDecrement(&((wav_grabber_cb_t*)self)->refs);
    return refs > 0 ? (ULONG)refs : 0;
}

static HRESULT STDMETHODCALLTYPE wav_cb_SampleCB(ISampleGrabberCB *self, double time, IMediaSample *sample)
{
    (void)self;
    (void)time;
    (void)sample;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE wav_cb_BufferCB(ISampleGrabberCB *self, double time, BYTE *buffer, long len)
{
    wav_grabber_cb_t *cb = (wav_grabber_cb_t*)self;
    DWORD wrote = 0;
    DWORD to_write;
    (void)time;
    if (!cb || cb->wav == INVALID_HANDLE_VALUE || !buffer || len <= 0) return S_OK;
    if (cb->data_bytes >= cb->max_bytes) return S_OK;
    to_write = (DWORD)len;
    if (cb->data_bytes + to_write > cb->max_bytes) to_write = cb->max_bytes - cb->data_bytes;
    if (to_write && WriteFile(cb->wav, buffer, to_write, &wrote, NULL)) cb->data_bytes += wrote;
    return S_OK;
}

static ISampleGrabberCBVtbl wav_grabber_vtbl = {
    wav_cb_QueryInterface,
    wav_cb_AddRef,
    wav_cb_Release,
    wav_cb_SampleCB,
    wav_cb_BufferCB
};

static void write_wav_header_a(HANDLE h, const WAVEFORMATEX *wfx, DWORD data_bytes)
{
    DWORD wrote;
    DWORD riff_size = 36 + data_bytes;
    WORD audio_format = 1;
    WORD channels = wfx && wfx->nChannels ? wfx->nChannels : 2;
    DWORD sample_rate = wfx && wfx->nSamplesPerSec ? wfx->nSamplesPerSec : 44100;
    WORD bits = wfx && wfx->wBitsPerSample ? wfx->wBitsPerSample : 16;
    WORD block_align = wfx && wfx->nBlockAlign ? wfx->nBlockAlign : (WORD)(channels * bits / 8);
    DWORD byte_rate = wfx && wfx->nAvgBytesPerSec ? wfx->nAvgBytesPerSec : sample_rate * block_align;
    BYTE header[44];
    memcpy(header + 0, "RIFF", 4);
    memcpy(header + 4, &riff_size, 4);
    memcpy(header + 8, "WAVEfmt ", 8);
    header[16] = 16; header[17] = 0; header[18] = 0; header[19] = 0;
    memcpy(header + 20, &audio_format, 2);
    memcpy(header + 22, &channels, 2);
    memcpy(header + 24, &sample_rate, 4);
    memcpy(header + 28, &byte_rate, 4);
    memcpy(header + 32, &block_align, 2);
    memcpy(header + 34, &bits, 2);
    memcpy(header + 36, "data", 4);
    memcpy(header + 40, &data_bytes, 4);
    SetFilePointer(h, 0, NULL, FILE_BEGIN);
    WriteFile(h, header, sizeof(header), &wrote, NULL);
}

static int decode_webm_audio_to_wav_cache_a(const char *webm, const char *sound_id,
                                            char *cache_path, size_t cache_path_sz,
                                            const char *dir, vm_AVStream *in_stream)
{
    IGraphBuilder *graph = NULL;
    IMediaControl *control = NULL;
    IMediaEvent *event = NULL;
    IBaseFilter *source = NULL, *audio = NULL, *sample_filter = NULL, *null_filter = NULL;
    ISampleGrabber *grabber = NULL;
    wchar_t wpath[MAX_PATH * 4];
    HRESULT hr;
    AM_MEDIA_TYPE mt;
    WAVEFORMATEX desired;
    WAVEFORMATEX connected;
    wav_grabber_cb_t cb;
    HANDLE wav = INVALID_HANDLE_VALUE;
    long evcode = 0;
    double length_seconds = 3600.0;
    int ok = 0;

    if (!webm || !sound_id || !cache_path || !cache_path_sz || !dir) return 0;
    _snprintf(cache_path, cache_path_sz, "%s\\%s.wav", dir, sound_id);
    cache_path[cache_path_sz - 1] = 0;
    if (in_stream && in_stream->duration > 0 && in_stream->time_base.num > 0 && in_stream->time_base.den > 0) {
        length_seconds = ((double)in_stream->duration * (double)in_stream->time_base.num) / (double)in_stream->time_base.den;
    }
    if (!MultiByteToWideChar(CP_ACP, 0, webm, -1, wpath, (int)(sizeof(wpath) / sizeof(wpath[0])))) return 0;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    hr = CoCreateInstance(&CLSID_FilterGraph_, NULL, CLSCTX_INPROC_SERVER, &IID_IGraphBuilder_, (void**)&graph);
    if (FAILED(hr) || !graph) { log_line("EngineAudio wav graph failed hr=0x%08lx", (DWORD)hr); goto done; }
    hr = create_lav_source_filter_for_file(wpath, (void**)&source);
    if (FAILED(hr) || !source) { log_line("EngineAudio wav source failed hr=0x%08lx webm=\"%s\"", (DWORD)hr, webm); goto done; }
    hr = create_lav_filter_object("LAVAudio.ax", &CLSID_LAVAudioDecoder_, (void**)&audio);
    if (FAILED(hr) || !audio) { log_line("EngineAudio wav LAV audio failed hr=0x%08lx webm=\"%s\"", (DWORD)hr, webm); goto done; }
    hr = CoCreateInstance(&CLSID_SampleGrabber_, NULL, CLSCTX_INPROC_SERVER, &IID_IBaseFilter_, (void**)&sample_filter);
    if (FAILED(hr) || !sample_filter) { log_line("EngineAudio wav sample filter failed hr=0x%08lx", (DWORD)hr); goto done; }
    hr = IBaseFilter_QueryInterface(sample_filter, &IID_ISampleGrabber_, (void**)&grabber);
    if (FAILED(hr) || !grabber) { log_line("EngineAudio wav sample grabber failed hr=0x%08lx", (DWORD)hr); goto done; }
    hr = CoCreateInstance(&CLSID_NullRenderer_, NULL, CLSCTX_INPROC_SERVER, &IID_IBaseFilter_, (void**)&null_filter);
    if (FAILED(hr) || !null_filter) { log_line("EngineAudio wav null renderer failed hr=0x%08lx", (DWORD)hr); goto done; }

    ZeroMemory(&desired, sizeof(desired));
    desired.wFormatTag = WAVE_FORMAT_PCM;
    desired.nChannels = 1;
    desired.nSamplesPerSec = 44100;
    desired.wBitsPerSample = 16;
    desired.nBlockAlign = (WORD)(desired.nChannels * desired.wBitsPerSample / 8);
    desired.nAvgBytesPerSec = desired.nSamplesPerSec * desired.nBlockAlign;
    ZeroMemory(&mt, sizeof(mt));
    mt.majortype = MEDIATYPE_Audio_;
    mt.subtype = MEDIASUBTYPE_PCM_;
    mt.formattype = FORMAT_WaveFormatEx_;
    mt.pbFormat = (BYTE*)&desired;
    mt.cbFormat = sizeof(desired);
    ISampleGrabber_SetMediaType(grabber, &mt);
    ISampleGrabber_SetBufferSamples(grabber, FALSE);
    ISampleGrabber_SetOneShot(grabber, FALSE);

    hr = IGraphBuilder_AddFilter(graph, source, L"NC-TK17-WebM wav source");
    if (FAILED(hr)) goto done;
    hr = IGraphBuilder_AddFilter(graph, audio, L"NC-TK17-WebM wav audio");
    if (FAILED(hr)) goto done;
    hr = IGraphBuilder_AddFilter(graph, sample_filter, L"NC-TK17-WebM wav grabber");
    if (FAILED(hr)) goto done;
    hr = IGraphBuilder_AddFilter(graph, null_filter, L"NC-TK17-WebM wav null");
    if (FAILED(hr)) goto done;
    hr = connect_filters_any_pin(graph, source, audio);
    if (FAILED(hr)) { log_line("EngineAudio wav connect source->audio failed hr=0x%08lx", (DWORD)hr); goto done; }
    hr = connect_filters_any_pin(graph, audio, sample_filter);
    if (FAILED(hr)) { log_line("EngineAudio wav connect audio->grabber failed hr=0x%08lx", (DWORD)hr); goto done; }
    hr = connect_filters_any_pin(graph, sample_filter, null_filter);
    if (FAILED(hr)) { log_line("EngineAudio wav connect grabber->null failed hr=0x%08lx", (DWORD)hr); goto done; }

    ZeroMemory(&mt, sizeof(mt));
    if (SUCCEEDED(ISampleGrabber_GetConnectedMediaType(grabber, &mt)) && mt.pbFormat && mt.cbFormat >= sizeof(WAVEFORMATEX)) {
        memcpy(&connected, mt.pbFormat, sizeof(connected));
    } else {
        connected = desired;
    }
    free_media_type_local(&mt);

    wav = real_CreateFileA ? real_CreateFileA(cache_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
                           : CreateFileA(cache_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (wav == INVALID_HANDLE_VALUE) { log_line("EngineAudio wav create failed path=\"%s\" err=%lu", cache_path, GetLastError()); goto done; }
    write_wav_header_a(wav, &connected, 0);
    ZeroMemory(&cb, sizeof(cb));
    cb.iface.lpVtbl = &wav_grabber_vtbl;
    cb.refs = 1;
    cb.wav = wav;
    cb.max_bytes = 64u * 1024u * 1024u;
    ISampleGrabber_SetCallback(grabber, (ISampleGrabberCB*)&cb, 1);

    IGraphBuilder_QueryInterface(graph, &IID_IMediaControl_, (void**)&control);
    IGraphBuilder_QueryInterface(graph, &IID_IMediaEvent_, (void**)&event);
    if (!control || !event) { log_line("EngineAudio wav control/event missing"); goto done; }
    hr = IMediaControl_Run(control);
    if (FAILED(hr)) { log_line("EngineAudio wav run failed hr=0x%08lx", (DWORD)hr); goto done; }
    IMediaEvent_WaitForCompletion(event, 30000, &evcode);
    IMediaControl_Stop(control);
    ISampleGrabber_SetCallback(grabber, NULL, 1);
    if (cb.data_bytes <= 44) { log_line("EngineAudio wav captured no audio bytes=%lu webm=\"%s\"", cb.data_bytes, webm); goto done; }
    write_wav_header_a(wav, &connected, cb.data_bytes);
    if (real_CloseHandle) real_CloseHandle(wav); else CloseHandle(wav);
    wav = INVALID_HANDLE_VALUE;
    remember_engine_sound_cache(sound_id, cache_path);
    audio_cache_manifest_record_a(webm, sound_id, cache_path, NULL);
    debug_line("EngineAudio wav cache ready sound=\"%s\" wav=\"%s\" bytes=%lu format=%u ch=%u hz=%lu bits=%u",
             sound_id, cache_path, cb.data_bytes, connected.wFormatTag,
             connected.nChannels, connected.nSamplesPerSec, connected.wBitsPerSample);
    ok = 1;

done:
    if (control) { IMediaControl_Stop(control); IMediaControl_Release(control); }
    if (event) IMediaEvent_Release(event);
    if (wav != INVALID_HANDLE_VALUE) { if (real_CloseHandle) real_CloseHandle(wav); else CloseHandle(wav); }
    if (grabber) ISampleGrabber_Release(grabber);
    if (null_filter) IBaseFilter_Release(null_filter);
    if (sample_filter) IBaseFilter_Release(sample_filter);
    if (audio) IBaseFilter_Release(audio);
    if (source) IBaseFilter_Release(source);
    if (graph) IGraphBuilder_Release(graph);
    return ok;
}

static void wav_write_s16_mono_samples(HANDLE wav, vm_AVFrame *frame, DWORD *data_bytes, DWORD max_bytes)
{
    int i;
    int samples;
    int fmt;
    int channels;
    DWORD wrote;
    if (!wav || wav == INVALID_HANDLE_VALUE || !frame || !data_bytes) return;
    samples = frame->nb_samples;
    fmt = frame->format;
    if (samples <= 0 || !frame->extended_data || !frame->extended_data[0]) return;
    channels = frame->extended_data[1] ? 2 : 1;
    if (channels < 1) channels = 1;
    for (i = 0; i < samples; i++) {
        short out = 0;
        if (*data_bytes + 2 > max_bytes) return;
        if (fmt == 8) {
            float *a = (float*)frame->extended_data[0];
            float v = a[i];
            if (channels > 1 && frame->extended_data[1]) {
                float *b = (float*)frame->extended_data[1];
                v = (v + b[i]) * 0.5f;
            }
            if (v > 1.0f) v = 1.0f;
            if (v < -1.0f) v = -1.0f;
            out = (short)(v * 32767.0f);
        } else if (fmt == 3) {
            float *p = (float*)frame->extended_data[0];
            float v = channels > 1 ? (p[i * channels] + p[i * channels + 1]) * 0.5f : p[i];
            if (v > 1.0f) v = 1.0f;
            if (v < -1.0f) v = -1.0f;
            out = (short)(v * 32767.0f);
        } else if (fmt == 6) {
            short *a = (short*)frame->extended_data[0];
            int v = a[i];
            if (channels > 1 && frame->extended_data[1]) {
                short *b = (short*)frame->extended_data[1];
                v = (v + b[i]) / 2;
            }
            out = (short)v;
        } else if (fmt == 1) {
            short *p = (short*)frame->extended_data[0];
            int v = channels > 1 ? (p[i * channels] + p[i * channels + 1]) / 2 : p[i];
            out = (short)v;
        } else if (fmt == 0) {
            unsigned char *p = (unsigned char*)frame->extended_data[0];
            int v = channels > 1 ? ((int)p[i * channels] + (int)p[i * channels + 1]) / 2 : p[i];
            out = (short)((v - 128) << 8);
        } else if (fmt == 5) {
            unsigned char *a = (unsigned char*)frame->extended_data[0];
            int v = a[i];
            if (channels > 1 && frame->extended_data[1]) {
                unsigned char *b = (unsigned char*)frame->extended_data[1];
                v = ((int)v + (int)b[i]) / 2;
            }
            out = (short)((v - 128) << 8);
        } else {
            return;
        }
        WriteFile(wav, &out, 2, &wrote, NULL);
        *data_bytes += wrote;
        if (wrote != 2) return;
    }
}

static DWORD read_wav_sample_rate_a(const char *path)
{
    HANDLE h;
    BYTE header[44];
    DWORD got = 0;
    DWORD rate = 0;

    if (!path || !path[0]) return 0;
    h = real_CreateFileA ? real_CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)
                         : CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    if ((real_ReadFile ? real_ReadFile(h, header, sizeof(header), &got, NULL) : ReadFile(h, header, sizeof(header), &got, NULL)) &&
        got >= sizeof(header) && memcmp(header, "RIFF", 4) == 0 && memcmp(header + 8, "WAVE", 4) == 0) {
        memcpy(&rate, header + 24, 4);
    }
    if (real_CloseHandle) real_CloseHandle(h); else CloseHandle(h);
    return rate;
}

static DWORD detect_webm_audio_sample_rate_a(const char *webm)
{
    HANDLE h;
    BYTE *buf;
    DWORD got = 0;
    DWORD limit = 1024u * 1024u;
    DWORD rate = 0;
    DWORD i;

    if (!webm || !webm[0]) return 0;
    buf = (BYTE*)malloc(limit);
    if (!buf) return 0;

    h = real_CreateFileA ? real_CreateFileA(webm, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)
                         : CreateFileA(webm, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        free(buf);
        return 0;
    }

    if (!(real_ReadFile ? real_ReadFile(h, buf, limit, &got, NULL) : ReadFile(h, buf, limit, &got, NULL))) got = 0;
    if (real_CloseHandle) real_CloseHandle(h); else CloseHandle(h);

    for (i = 0; i + 19 <= got; i++) {
        if (memcmp(buf + i, "OpusHead", 8) == 0) {
            rate = 48000;
            break;
        }
    }

    if (!rate) {
        for (i = 1; i + 15 <= got; i++) {
            if (buf[i - 1] == 1 && memcmp(buf + i, "vorbis", 6) == 0) {
                DWORD vorbis_rate;
                BYTE channels = buf[i + 10];
                memcpy(&vorbis_rate, buf + i + 11, 4);
                if (channels >= 1 && channels <= 8 && vorbis_rate >= 8000 && vorbis_rate <= 192000) {
                    rate = vorbis_rate;
                    break;
                }
            }
        }
    }

    free(buf);
    return rate;
}

static int decode_webm_audio_to_wav_ffmpeg_a(AVFormatContext *in_ctx, int audio_stream,
                                             vm_AVStream *in_stream, const char *webm,
                                             const char *sound_id, char *cache_path,
                                             size_t cache_path_sz, const char *dir)
{
    AVCodecContext *codec = NULL;
    const AVCodec *decoder = NULL;
    vm_AVPacket *pkt = NULL;
    vm_AVFrame *frame = NULL;
    HANDLE wav = INVALID_HANDLE_VALUE;
    WAVEFORMATEX wfx;
    DWORD data_bytes = 0;
    DWORD max_bytes = 128u * 1024u * 1024u;
    DWORD webm_sample_rate = 0;
    long long decoded_sample_rate = 0;
    int ret;
    int ok = 0;

    if (!in_ctx || audio_stream < 0 || !in_stream || !in_stream->codecpar ||
        !webm || !sound_id || !cache_path || !cache_path_sz || !dir) return 0;

    _snprintf(cache_path, cache_path_sz, "%s\\%s.wav", dir, sound_id);
    cache_path[cache_path_sz - 1] = 0;
    decoder = ffmpeg_api.avcodec_find_decoder(in_stream->codecpar->codec_id);
    if (!decoder) { log_line("EngineAudio wav ffmpeg decoder missing codec=%d webm=\"%s\"", in_stream->codecpar->codec_id, webm); goto done; }
    codec = ffmpeg_api.avcodec_alloc_context3(decoder);
    if (!codec) { log_line("EngineAudio wav ffmpeg codec ctx failed webm=\"%s\"", webm); goto done; }
    ret = ffmpeg_api.avcodec_parameters_to_context(codec, in_stream->codecpar);
    if (ret < 0) { log_line("EngineAudio wav ffmpeg params failed ret=%d webm=\"%s\"", ret, webm); goto done; }
    ret = ffmpeg_api.avcodec_open2(codec, decoder, NULL);
    if (ret < 0) { log_line("EngineAudio wav ffmpeg open failed ret=%d webm=\"%s\"", ret, webm); goto done; }
    if (ffmpeg_api.av_opt_get_int) {
        if (ffmpeg_api.av_opt_get_int(codec, "sample_rate", 0, &decoded_sample_rate) < 0) {
            decoded_sample_rate = 0;
        }
    }
    webm_sample_rate = detect_webm_audio_sample_rate_a(webm);
    pkt = ffmpeg_api.av_packet_alloc();
    frame = ffmpeg_api.av_frame_alloc();
    if (!pkt || !frame) goto done;

    wav = real_CreateFileA ? real_CreateFileA(cache_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
                           : CreateFileA(cache_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (wav == INVALID_HANDLE_VALUE) { log_line("EngineAudio wav ffmpeg create failed path=\"%s\" err=%lu", cache_path, GetLastError()); goto done; }
    ZeroMemory(&wfx, sizeof(wfx));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    if (webm_sample_rate >= 8000 && webm_sample_rate <= 192000)
        wfx.nSamplesPerSec = webm_sample_rate;
    else if (decoded_sample_rate >= 8000 && decoded_sample_rate <= 192000)
        wfx.nSamplesPerSec = (DWORD)decoded_sample_rate;
    else
        wfx.nSamplesPerSec = 48000;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (WORD)(wfx.nChannels * wfx.wBitsPerSample / 8);
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    write_wav_header_a(wav, &wfx, 0);

    while (ffmpeg_api.av_read_frame(in_ctx, pkt) >= 0) {
        if (pkt->stream_index == audio_stream) {
            ret = ffmpeg_api.avcodec_send_packet(codec, pkt);
            if (ret >= 0) {
                while ((ret = ffmpeg_api.avcodec_receive_frame(codec, frame)) >= 0) {
                    wav_write_s16_mono_samples(wav, frame, &data_bytes, max_bytes);
                    ffmpeg_api.av_frame_unref(frame);
                    if (data_bytes >= max_bytes) break;
                }
            }
        }
        ffmpeg_api.av_packet_unref(pkt);
        if (data_bytes >= max_bytes) break;
    }
    ffmpeg_api.avcodec_send_packet(codec, NULL);
    while (ffmpeg_api.avcodec_receive_frame(codec, frame) >= 0) {
        wav_write_s16_mono_samples(wav, frame, &data_bytes, max_bytes);
        ffmpeg_api.av_frame_unref(frame);
        if (data_bytes >= max_bytes) break;
    }

    if (data_bytes <= 0) { log_line("EngineAudio wav ffmpeg captured no audio webm=\"%s\"", webm); goto done; }
    write_wav_header_a(wav, &wfx, data_bytes);
    if (real_CloseHandle) real_CloseHandle(wav); else CloseHandle(wav);
    wav = INVALID_HANDLE_VALUE;
    remember_engine_sound_cache(sound_id, cache_path);
    audio_cache_manifest_record_a(webm, sound_id, cache_path, NULL);
    debug_line("EngineAudio wav ffmpeg cache ready sound=\"%s\" wav=\"%s\" bytes=%lu ch=1 hz=%lu bits=16 format=mono decoded_hz=%lld webm_hz=%lu",
             sound_id, cache_path, data_bytes, wfx.nSamplesPerSec, decoded_sample_rate, webm_sample_rate);
    ok = 1;

done:
    if (wav != INVALID_HANDLE_VALUE) { if (real_CloseHandle) real_CloseHandle(wav); else CloseHandle(wav); }
    if (frame) ffmpeg_api.av_frame_free(&frame);
    if (pkt) ffmpeg_api.av_packet_free(&pkt);
    if (codec) ffmpeg_api.avcodec_free_context(&codec);
    return ok;
}

static int register_webm_as_engine_sound_fallback_a(const char *webm, const char *sound_id,
                                                     char *cache_path, size_t cache_path_sz,
                                                     const char *dir, vm_AVStream *in_stream)
{
    char snf_path[MAX_PATH * 4];
    double length_seconds = 3600.0;
    if (!webm || !sound_id || !cache_path || !cache_path_sz || !dir) return 0;
    if (in_stream && in_stream->duration > 0 && in_stream->time_base.num > 0 && in_stream->time_base.den > 0) {
        length_seconds = ((double)in_stream->duration * (double)in_stream->time_base.num) / (double)in_stream->time_base.den;
    }
    _snprintf(snf_path, sizeof(snf_path) - 1, "%s\\%s.snf", dir, sound_id);
    snf_path[sizeof(snf_path) - 1] = 0;
    if (!write_engine_sound_snf_a(snf_path, length_seconds)) {
        log_line("EngineAudio WebM sound fallback snf failed snf=\"%s\" webm=\"%s\"", snf_path, webm);
        return 0;
    }
    lstrcpynA(cache_path, webm, (int)cache_path_sz);
    remember_engine_sound_cache_ex(sound_id, cache_path, snf_path);
    audio_cache_manifest_record_a(webm, sound_id, NULL, snf_path);
    debug_line("EngineAudio WebM sound fallback registered sound=\"%s\" webm=\"%s\" snf=\"%s\" length=%.3f",
             sound_id, webm, snf_path, length_seconds);
    return 1;
}

static int remux_webm_audio_to_ogg_a(const char *webm, char *sound_id, size_t sound_id_sz, char *cache_path, size_t cache_path_sz)
{
    AVFormatContext *in_ctx = NULL;
    AVFormatContext *out_ctx = NULL;
    vm_AVFormatContext *in_pub;
    vm_AVFormatContext *out_pub;
    vm_AVStream *in_stream;
    vm_AVStream *out_stream;
    vm_AVPacket *pkt = NULL;
    int audio_stream;
    int ret;
    char root[MAX_PATH * 4];
    char dir[MAX_PATH * 4];
    WIN32_FILE_ATTRIBUTE_DATA webm_attr, ogg_attr, wav_attr;

    if (sound_id && sound_id_sz) sound_id[0] = 0;
    if (cache_path && cache_path_sz) cache_path[0] = 0;
    if (!webm || !sound_id || !sound_id_sz || !cache_path || !cache_path_sz || !ffmpeg_load_api()) return 0;

    _snprintf(sound_id, sound_id_sz, "NC_TK17_WebM_%08x", fnv1a_hash_a(webm));
    sound_id[sound_id_sz - 1] = 0;
    cache_root_a(root, sizeof(root));
    path_join(dir, sizeof(dir), root, "Sounds\\Shared\\Effect");
    ensure_dir_a(dir);
    _snprintf(cache_path, cache_path_sz, "%s\\%s.ogg", dir, sound_id);
    cache_path[cache_path_sz - 1] = 0;

    if (GetFileAttributesExA(webm, GetFileExInfoStandard, &webm_attr)) {
        char wav_path[MAX_PATH * 4];
        DWORD webm_rate = detect_webm_audio_sample_rate_a(webm);
        _snprintf(wav_path, sizeof(wav_path) - 1, "%s\\%s.wav", dir, sound_id);
        wav_path[sizeof(wav_path) - 1] = 0;
        if (GetFileAttributesExA(wav_path, GetFileExInfoStandard, &wav_attr) &&
            CompareFileTime(&wav_attr.ftLastWriteTime, &webm_attr.ftLastWriteTime) >= 0) {
            DWORD wav_rate = read_wav_sample_rate_a(wav_path);
            if (webm_rate >= 8000 && webm_rate <= 192000 && wav_rate != webm_rate) {
                log_line("EngineAudio wav cache stale rate sound=\"%s\" wav_hz=%lu webm_hz=%lu wav=\"%s\" webm=\"%s\"",
                         sound_id, wav_rate, webm_rate, wav_path, webm);
            } else {
            lstrcpynA(cache_path, wav_path, (int)cache_path_sz);
            remember_engine_sound_cache(sound_id, wav_path);
            audio_cache_manifest_record_a(webm, sound_id, wav_path, NULL);
            debug_line("EngineAudio wav cache reused sound=\"%s\" wav=\"%s\" webm=\"%s\"", sound_id, wav_path, webm);
            return 1;
            }
        }
    }

    if (GetFileAttributesExA(cache_path, GetFileExInfoStandard, &ogg_attr) &&
        GetFileAttributesExA(webm, GetFileExInfoStandard, &webm_attr) &&
        CompareFileTime(&ogg_attr.ftLastWriteTime, &webm_attr.ftLastWriteTime) >= 0) {
        remember_engine_sound_cache(sound_id, cache_path);
        audio_cache_manifest_record_a(webm, sound_id, cache_path, NULL);
        return 1;
    }

    ret = ffmpeg_api.avformat_open_input(&in_ctx, webm, NULL, NULL);
    if (ret < 0 || !in_ctx) {
        log_line("EngineAudio remux open failed ret=%d webm=\"%s\"", ret, webm);
        return 0;
    }
    ret = ffmpeg_api.avformat_find_stream_info(in_ctx, NULL);
    if (ret < 0) {
        log_line("EngineAudio remux stream info failed ret=%d webm=\"%s\"", ret, webm);
        goto done;
    }
    audio_stream = ffmpeg_api.av_find_best_stream(in_ctx, 1, -1, -1, NULL, 0);
    if (audio_stream < 0) {
        log_line("EngineAudio remux no audio stream ret=%d webm=\"%s\"", audio_stream, webm);
        goto done;
    }
    in_pub = (vm_AVFormatContext*)in_ctx;
    if (!in_pub->streams || (unsigned int)audio_stream >= in_pub->nb_streams) goto done;
    in_stream = in_pub->streams[audio_stream];
    if (!in_stream || !in_stream->codecpar) goto done;

    ret = decode_webm_audio_to_wav_ffmpeg_a(in_ctx, audio_stream, in_stream, webm,
                                            sound_id, cache_path, cache_path_sz, dir) ? 1 : 0;
    goto finish;

    ret = ffmpeg_api.avformat_alloc_output_context2(&out_ctx, NULL, "ogg", cache_path);
    if (ret < 0 || !out_ctx) {
        log_line("EngineAudio remux output context fmt=ogg failed ret=%d ogg=\"%s\"", ret, cache_path);
        ret = ffmpeg_api.avformat_alloc_output_context2(&out_ctx, NULL, NULL, cache_path);
    }
    if (ret < 0 || !out_ctx) {
        log_line("EngineAudio remux output context ext=ogg failed ret=%d ogg=\"%s\"", ret, cache_path);
        ret = decode_webm_audio_to_wav_cache_a(webm, sound_id, cache_path, cache_path_sz, dir, in_stream) ? 1 : 0;
        goto finish;
    }
    out_stream = ffmpeg_api.avformat_new_stream(out_ctx, NULL);
    if (!out_stream || !out_stream->codecpar) {
        log_line("EngineAudio remux output stream failed ogg=\"%s\"", cache_path);
        goto done;
    }
    ret = ffmpeg_api.avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
    if (ret < 0) {
        log_line("EngineAudio remux codec copy failed ret=%d ogg=\"%s\"", ret, cache_path);
        goto done;
    }
    out_stream->time_base = in_stream->time_base;
    out_pub = (vm_AVFormatContext*)out_ctx;
    ret = ffmpeg_api.avio_open(&out_pub->pb, cache_path, 2);
    if (ret < 0) {
        log_line("EngineAudio remux avio_open failed ret=%d ogg=\"%s\"", ret, cache_path);
        goto done;
    }
    ret = ffmpeg_api.avformat_write_header(out_ctx, NULL);
    if (ret < 0) {
        log_line("EngineAudio remux write_header failed ret=%d ogg=\"%s\"", ret, cache_path);
        goto done;
    }
    pkt = ffmpeg_api.av_packet_alloc();
    if (!pkt) goto done;
    while (ffmpeg_api.av_read_frame(in_ctx, pkt) >= 0) {
        if (pkt->stream_index == audio_stream) {
            pkt->stream_index = 0;
            ret = ffmpeg_api.av_interleaved_write_frame(out_ctx, pkt);
            if (ret < 0) {
                log_line("EngineAudio remux packet write failed ret=%d ogg=\"%s\"", ret, cache_path);
                ffmpeg_api.av_packet_unref(pkt);
                goto done;
            }
        }
        ffmpeg_api.av_packet_unref(pkt);
    }
    ffmpeg_api.av_write_trailer(out_ctx);
    remember_engine_sound_cache(sound_id, cache_path);
    audio_cache_manifest_record_a(webm, sound_id, cache_path, NULL);
    debug_line("EngineAudio remux ready sound=\"%s\" ogg=\"%s\" webm=\"%s\"", sound_id, cache_path, webm);
    ret = 1;
    goto finish;

done:
    ret = 0;
finish:
    if (pkt) ffmpeg_api.av_packet_free(&pkt);
    if (out_ctx) {
        vm_AVFormatContext *pub = (vm_AVFormatContext*)out_ctx;
        if (pub->pb) ffmpeg_api.avio_closep(&pub->pb);
        ffmpeg_api.avformat_free_context(out_ctx);
    }
    if (in_ctx) ffmpeg_api.avformat_close_input(&in_ctx);
    return ret;
}

typedef struct {
    char text[32768];
    int count;
} engine_audio_blocks_t;

static int script_audio_root_a(const char *path, char *root, size_t rootsz)
{
    const char *marker = NULL;
    const char *p;
    size_t n;
    if (root && rootsz) root[0] = 0;
    if (!path || !root || !rootsz) return 0;
    if (!contains_i(path, "\\Scripts\\") && !contains_i(path, "/Scripts/")) return 0;
    if (!contains_i(path, "\\Addons\\") && !contains_i(path, "/Addons/") &&
        !contains_i(path, "\\Mod\\ActiveMod\\") && !contains_i(path, "/Mod/ActiveMod/")) return 0;
    for (p = path; *p; p++) {
        if (_strnicmp(p, "\\Scripts\\", 9) == 0 || _strnicmp(p, "/Scripts/", 9) == 0) {
            marker = p;
            break;
        }
    }
    if (!marker) return 0;
    n = (size_t)(marker - path);
    if (!n || n >= rootsz) return 0;
    memcpy(root, path, n);
    root[n] = 0;
    return 1;
}

static int game_root_a(char *out, size_t outsz)
{
    char dll_path[MAX_PATH * 2];
    char bin_dir[MAX_PATH * 2];
    if (!out || !outsz) return 0;
    out[0] = 0;
    if (!self_module || !GetModuleFileNameA(self_module, dll_path, sizeof(dll_path))) return 0;
    dll_path[sizeof(dll_path) - 1] = 0;
    strcpy(bin_dir, dll_path);
    dirname_inplace(bin_dir);
    strcpy(out, bin_dir);
    dirname_inplace(out);
    return out[0] != 0;
}

static int find_relative_script_root_a(const char *rel, char *resolved, size_t resolved_sz, char *root, size_t root_sz)
{
    char game_dir[MAX_PATH * 2];
    char base[MAX_PATH * 2];
    char pattern[MAX_PATH * 4];
    WIN32_FIND_DATAA data;
    HANDLE h;
    if (resolved && resolved_sz) resolved[0] = 0;
    if (root && root_sz) root[0] = 0;
    if (!rel || !resolved || !resolved_sz || !root || !root_sz) return 0;
    if (!game_root_a(game_dir, sizeof(game_dir))) return 0;

    path_join(base, sizeof(base), game_dir, "Addons");
    path_join(pattern, sizeof(pattern), base, "*");
    h = FindFirstFileA(pattern, &data);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            char addon_dir[MAX_PATH * 4];
            char candidate[MAX_PATH * 4];
            DWORD attr;
            if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0) continue;
            if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            path_join(addon_dir, sizeof(addon_dir), base, data.cFileName);
            path_join(candidate, sizeof(candidate), addon_dir, rel);
            attr = GetFileAttributesA(candidate);
            if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY) &&
                script_audio_root_a(candidate, root, root_sz)) {
                lstrcpynA(resolved, candidate, (int)resolved_sz);
                FindClose(h);
                return 1;
            }
        } while (FindNextFileA(h, &data));
        FindClose(h);
    }

    path_join(base, sizeof(base), game_dir, "Mod\\ActiveMod");
    path_join(pattern, sizeof(pattern), base, "*");
    h = FindFirstFileA(pattern, &data);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            char mod_dir[MAX_PATH * 4];
            char candidate[MAX_PATH * 4];
            DWORD attr;
            if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0) continue;
            if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            path_join(mod_dir, sizeof(mod_dir), base, data.cFileName);
            path_join(candidate, sizeof(candidate), mod_dir, rel);
            attr = GetFileAttributesA(candidate);
            if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY) &&
                script_audio_root_a(candidate, root, root_sz)) {
                lstrcpynA(resolved, candidate, (int)resolved_sz);
                FindClose(h);
                return 1;
            }
        } while (FindNextFileA(h, &data));
        FindClose(h);
    }
    return 0;
}

static int resolve_script_audio_root_a(const char *path, char *resolved, size_t resolved_sz, char *root, size_t root_sz)
{
    char rel[MAX_PATH * 4];
    char with_ext[MAX_PATH * 4];
    char script_from_bsb[MAX_PATH * 4];
    const char *p;
    if (resolved && resolved_sz) resolved[0] = 0;
    if (root && root_sz) root[0] = 0;
    if (!path || !resolved || !resolved_sz || !root || !root_sz) return 0;
    if (temp_bsb_to_addon_script_a(path, script_from_bsb, sizeof(script_from_bsb)) &&
        script_audio_root_a(script_from_bsb, root, root_sz)) {
        lstrcpynA(resolved, script_from_bsb, (int)resolved_sz);
        return 1;
    }
    if (script_audio_root_a(path, root, root_sz)) {
        lstrcpynA(resolved, path, (int)resolved_sz);
        return 1;
    }
    if (!ends_with_i(path, ".bs") && strlen(path) + 3 < sizeof(with_ext)) {
        _snprintf(with_ext, sizeof(with_ext) - 1, "%s.bs", path);
        with_ext[sizeof(with_ext) - 1] = 0;
        if (script_audio_root_a(with_ext, root, root_sz)) {
            lstrcpynA(resolved, with_ext, (int)resolved_sz);
            return 1;
        }
    }
    p = path;
    while (*p == '\\' || *p == '/') p++;
    lstrcpynA(rel, p, sizeof(rel));
    if (!contains_i(rel, "Scripts\\") && !contains_i(rel, "Scripts/")) return 0;
    if (find_relative_script_root_a(rel, resolved, resolved_sz, root, root_sz)) return 1;
    if (!ends_with_i(rel, ".bs") && strlen(rel) + 3 < sizeof(with_ext)) {
        _snprintf(with_ext, sizeof(with_ext) - 1, "%s.bs", rel);
        with_ext[sizeof(with_ext) - 1] = 0;
        return find_relative_script_root_a(with_ext, resolved, resolved_sz, root, root_sz);
    }
    return 0;
}

static int script_looks_audio_host_a(const char *path)
{
    char base[MAX_PATH];
    char resolved[MAX_PATH * 4];
    if (!path) return 0;
    if (strchr(path, '.') && !ends_with_i(path, ".bs") && !ends_with_i(path, ".[bsb]")) return 0;
    if (temp_bsb_to_addon_script_a(path, resolved, sizeof(resolved))) {
        basename_no_ext_a(resolved, base, sizeof(base));
        return _strnicmp(base, "Ac", 2) == 0;
    }
    if (!contains_i(path, "\\Scripts\\Luder\\") && !contains_i(path, "/Scripts/Luder/") &&
        !contains_i(path, "@Scripts#Luder#")) return 0;
    basename_no_ext_a(path, base, sizeof(base));
    return _strnicmp(base, "Ac", 2) == 0;
}

static int engine_audio_sidecar_settings_a(const char *webm, char *parent_expr, size_t parent_expr_sz, int *volume)
{
    char ini[MAX_PATH * 4];
    char raw[MAX_PATH * 4];
    int enabled = texture_audio_engine_enabled;
    int local_volume = texture_audio_volume;
    if (parent_expr && parent_expr_sz) parent_expr[0] = 0;
    if (volume) *volume = local_volume;
    if (!webm || !parent_expr || !parent_expr_sz) return 0;
    debug_line("EngineAudio script rewrite skipped sidecar=\"%s\"", webm);
    return 0;
    sidecar_ini_path_a(webm, ini, sizeof(ini));
    if (!get_file_write_time_a(ini, NULL)) return 0;
    if (ini_has_key_a(ini, "texture_audio_engine")) {
        enabled = ini_key_bool_a(ini, "texture_audio_engine", enabled);
    }
    if (!enabled) return 0;
    if (ini_has_key_a(ini, "audio_engine_volume")) {
        local_volume = clamp_int(ini_key_int_a(ini, "audio_engine_volume", local_volume), -10000, 20000);
    } else if (ini_has_key_a(ini, "audio_volume")) {
        local_volume = clamp_int(ini_key_int_a(ini, "audio_volume", local_volume), -10000, 20000);
    }
    raw[0] = 0;
    ini_key_string_a(ini, "audio_parent_path", raw, sizeof(raw));
    if (!raw[0]) {
        infer_toy_audio_parent_path_a(webm, raw, sizeof(raw));
    }
    if (!raw[0]) {
        log_line("EngineAudio missing audio_parent_path ini=\"%s\" webm=\"%s\"", ini, webm);
        return 0;
    }
    if (raw[0] == '"') {
        lstrcpynA(parent_expr, raw, (int)parent_expr_sz);
    } else if (strchr(raw, '+')) {
        _snprintf(parent_expr, parent_expr_sz - 1, "\"%s\"", raw);
        parent_expr[parent_expr_sz - 1] = 0;
    } else if (strchr(raw, ':')) {
        lstrcpynA(parent_expr, raw, (int)parent_expr_sz);
    } else {
        _snprintf(parent_expr, parent_expr_sz - 1, "\"%s\"", raw);
        parent_expr[parent_expr_sz - 1] = 0;
    }
    if (volume) *volume = local_volume;
    return 1;
}

static void engine_audio_append_block(engine_audio_blocks_t *blocks, const char *webm,
                                      const char *sound_id, const char *parent_expr, int volume)
{
    char block[2048];
    size_t used, avail, n;
    if (!blocks || !sound_id || !parent_expr || blocks->count >= 16) return;
    _snprintf(block, sizeof(block) - 1,
              "\t\tAppImportSound . {\r\n"
              "\t\t\t.NodeName \"%s_node\";\r\n"
              "\t\t\t.ParentPath %s;\r\n"
              "\t\t\t.SoundFile \"Shared/Effect/%s\";\r\n"
              "\t\t\t.MixerFlags \"SC3D\";\r\n"
              "\t\t\t.Category \"Ambient\";\r\n"
              "\t\t\t.Volume F32(%d);\r\n"
              "\t\t};\r\n",
              sound_id, parent_expr, sound_id, volume);
    block[sizeof(block) - 1] = 0;
    used = strlen(blocks->text);
    avail = sizeof(blocks->text) - used - 1;
    n = strlen(block);
    if (n > avail) return;
    memcpy(blocks->text + used, block, n + 1);
    blocks->count++;
    debug_line("EngineAudio block webm=\"%s\" sound=\"%s\" parent=%s volume=%d",
               webm ? webm : "", sound_id, parent_expr, volume);
}

static void scan_engine_audio_sidecars_a(const char *dir, engine_audio_blocks_t *blocks, int depth)
{
    char pattern[MAX_PATH * 4];
    char child[MAX_PATH * 4];
    WIN32_FIND_DATAA data;
    HANDLE h;
    if (!dir || !blocks || depth > 8 || blocks->count >= 16) return;
    path_join(pattern, sizeof(pattern), dir, "*");
    h = FindFirstFileA(pattern, &data);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0) continue;
        path_join(child, sizeof(child), dir, data.cFileName);
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            scan_engine_audio_sidecars_a(child, blocks, depth + 1);
        } else if (ends_with_i(data.cFileName, ".webm")) {
            char parent_expr[MAX_PATH * 4];
            char sound_id[128];
            char cache_path[MAX_PATH * 4];
            int volume = 0;
            if (engine_audio_sidecar_settings_a(child, parent_expr, sizeof(parent_expr), &volume) &&
                remux_webm_audio_to_ogg_a(child, sound_id, sizeof(sound_id), cache_path, sizeof(cache_path))) {
                engine_audio_append_block(blocks, child, sound_id, parent_expr, volume);
            }
        }
    } while (FindNextFileA(h, &data));
    FindClose(h);
}

static const char *component_array_insert_point_a(const char *src)
{
    const char *comp;
    const char *p;
    if (!src) return NULL;
    comp = strstr(src, ".ComponentArray");
    if (!comp) return NULL;
    p = strstr(comp, "\r\n\t];");
    if (p) return p + 2;
    p = strstr(comp, "\n\t];");
    if (p) return p + 1;
    p = strstr(comp, "\r\n];");
    if (p) return p + 2;
    p = strstr(comp, "\n];");
    if (p) return p + 1;
    return NULL;
}

static char *rewrite_script_for_engine_audio_a(const char *src, const char *blocks)
{
    const char *insert;
    size_t prefix_len, src_len, blocks_len;
    char *out;
    if (!src || !blocks || !blocks[0]) return NULL;
    insert = component_array_insert_point_a(src);
    if (!insert) return NULL;
    src_len = strlen(src);
    blocks_len = strlen(blocks);
    prefix_len = (size_t)(insert - src);
    out = (char*)malloc(src_len + blocks_len + 1);
    if (!out) return NULL;
    memcpy(out, src, prefix_len);
    memcpy(out + prefix_len, blocks, blocks_len);
    strcpy(out + prefix_len + blocks_len, insert);
    return out;
}

static int create_engine_audio_script_rewrite_a(const char *original_mb, char *redirect, size_t redirect_count)
{
    char root[MAX_PATH * 4];
    char script_path[MAX_PATH * 4];
    char cache_dir[MAX_PATH * 2];
    char base[MAX_PATH];
    char temp_path[MAX_PATH * 4];
    HANDLE in = INVALID_HANDLE_VALUE;
    HANDLE out = INVALID_HANDLE_VALUE;
    DWORD size, got, wrote;
    char *src = NULL;
    char *rewritten = NULL;
    engine_audio_blocks_t blocks;
    int ok = 0;

    if (!original_mb || !redirect || !redirect_count) return 0;
    redirect[0] = 0;
    if (!script_looks_audio_host_a(original_mb)) {
        if (should_log_engine_audio_toy_probe_a(original_mb)) {
            debug_line("EngineAudio rewrite skip not-audio-host path=\"%s\"", original_mb);
        }
        return 0;
    }
    if (!resolve_script_audio_root_a(original_mb, script_path, sizeof(script_path), root, sizeof(root))) {
        if (should_log_engine_audio_toy_probe_a(original_mb)) {
            debug_line("EngineAudio rewrite skip unresolved path=\"%s\"", original_mb);
        }
        return 0;
    }

    blocks.text[0] = 0;
    blocks.count = 0;
    scan_engine_audio_sidecars_a(root, &blocks, 0);
    if (!blocks.count) {
        if (should_log_engine_audio_toy_probe_a(original_mb)) {
            debug_line("EngineAudio rewrite skip no-blocks original=\"%s\" resolved=\"%s\" root=\"%s\"",
                       original_mb, script_path, root);
        }
        return 0;
    }

    in = real_CreateFileA(script_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (in == INVALID_HANDLE_VALUE) {
        if (should_log_engine_audio_toy_probe_a(original_mb)) {
            debug_line("EngineAudio rewrite skip open-failed original=\"%s\" resolved=\"%s\" err=%lu",
                       original_mb, script_path, GetLastError());
        }
        return 0;
    }
    size = GetFileSize(in, NULL);
    if (size == INVALID_FILE_SIZE || size > 16 * 1024 * 1024) goto done;
    src = (char*)malloc(size + 1);
    if (!src) goto done;
    if (!real_ReadFile(in, src, size, &got, NULL) || got != size) goto done;
    src[size] = 0;

    rewritten = rewrite_script_for_engine_audio_a(src, blocks.text);
    if (!rewritten) {
        log_line("EngineAudio script rewrite failed: no ComponentArray original=\"%s\"", original_mb);
        goto done;
    }
    webm_component_dir_a(cache_dir, sizeof(cache_dir),
                         "NC-TK17-WebM-Cache");
    if (!cache_dir[0]) goto done;
    ensure_dir_a(cache_dir);
    basename_no_ext_a(script_path, base, sizeof(base));
    _snprintf(temp_path, sizeof(temp_path) - 1, "%s\\%s.engineaudio.%08x.bs", cache_dir, base, fnv1a_hash_a(script_path));
    temp_path[sizeof(temp_path) - 1] = 0;
    out = real_CreateFileA ? real_CreateFileA(temp_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL)
                           : CreateFileA(temp_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (out == INVALID_HANDLE_VALUE) goto done;
    if (!WriteFile(out, rewritten, (DWORD)strlen(rewritten), &wrote, NULL)) goto done;
    real_CloseHandle(out);
    out = INVALID_HANDLE_VALUE;
    lstrcpynA(redirect, temp_path, (int)redirect_count);
    debug_line("EngineAudio script rewrite original=\"%s\" resolved=\"%s\" temp=\"%s\" blocks=%d", original_mb, script_path, temp_path, blocks.count);
    ok = 1;

done:
    if (out != INVALID_HANDLE_VALUE) real_CloseHandle(out);
    if (in != INVALID_HANDLE_VALUE) real_CloseHandle(in);
    if (src) free(src);
    if (rewritten) free(rewritten);
    return ok;
}

static int create_engine_audio_script_rewrite_w(LPCWSTR original, wchar_t *redirect, size_t redirect_count)
{
    char original_mb[MAX_PATH * 4];
    char temp_path[MAX_PATH * 4];
    if (!original || !redirect || !redirect_count) return 0;
    wide_to_mb(original, original_mb, sizeof(original_mb));
    if (!create_engine_audio_script_rewrite_a(original_mb, temp_path, sizeof(temp_path))) return 0;
    MultiByteToWideChar(CP_ACP, 0, temp_path, -1, redirect, (int)redirect_count);
    return 1;
}

static void *open_stream_pipe_file_cache_a(const char *path)
{
    bionic_stream_pipe_file_cache_t *file;
    HMODULE sys;
    int type_id;
    if (!path || !path[0] || !engine_BionicNew || !engine_MsvcNew || !engine_StreamPipeFileCache_Ctor) return NULL;
    sys = GetModuleHandleA("ThriXXX010278-SYS.dll");
    if (!sys) return NULL;
    type_id = (int)((BYTE*)sys + (0x101BE808 - 0x10000000));
    file = (bionic_stream_pipe_file_cache_t*)engine_BionicNew(sizeof(*file), type_id);
    if (!file) return NULL;
    __asm__ ("movl %0, %%ecx" : : "r"(file) : "ecx");
    engine_StreamPipeFileCache_Ctor((char*)path, 1);
    file->lpCriticalSection = engine_MsvcNew(sizeof(CRITICAL_SECTION));
    if (file->lpCriticalSection) {
        InitializeCriticalSection((CRITICAL_SECTION*)file->lpCriticalSection);
    }
    if (file->file_handle == INVALID_HANDLE_VALUE || file->file_handle == 0) {
        log_line("EngineAudio OpenStream cache pipe failed path=\"%s\" handle=%p", path, file->file_handle);
        return NULL;
    }
    return file;
}

static int should_log_engine_audio_script_probe_a(const char *path)
{
    static DWORD last_tick;
    DWORD now;
    if (!path) return 0;
    if (!contains_i(path, "NcToy7") && !debug_logging) return 0;
    now = GetTickCount();
    if (last_tick && now - last_tick < 250) return 0;
    last_tick = now;
    return 1;
}

static int should_log_engine_audio_toy_probe_a(const char *path)
{
    static DWORD last_tick;
    DWORD now;
    if (!path) return 0;
    if (!contains_i(path, "NcToy7") && !contains_i(path, "AcNcToy7")) return 0;
    now = GetTickCount();
    if (last_tick && now - last_tick < 100) return 0;
    last_tick = now;
    return 1;
}

static int safe_cstr_a(const char *path, UINT_PTR max_len)
{
    if (!path) return 0;
    if (IsBadStringPtrA(path, max_len)) return 0;
    return 1;
}

static void *__cdecl hook_StorageOpenStream(char *path, unsigned int flags, char *out_error)
{
    static int probe_count;
    char redirect[MAX_PATH * 4];
    void *pipe;
    redirect[0] = 0;
    if (probe_count < 40) {
        probe_count++;
        if (safe_cstr_a(path, 1024)) {
            debug_line("EngineAudio OpenStream call path=\"%s\" flags=%08x", path, flags);
        } else {
            debug_line("EngineAudio OpenStream call path_ptr=%p flags=%08x unsafe-string", path, flags);
        }
    } else if (safe_cstr_a(path, 1024) && should_log_engine_audio_script_probe_a(path)) {
        debug_line("EngineAudio OpenStream probe path=\"%s\" flags=%08x", path, flags);
    }
    if (safe_cstr_a(path, 4096) && create_engine_audio_script_rewrite_a(path, redirect, sizeof(redirect))) {
        pipe = open_stream_pipe_file_cache_a(redirect);
        if (pipe) {
            debug_line("EngineAudio OpenStream redirect \"%s\" -> \"%s\"", path, redirect);
            return pipe;
        }
        log_line("EngineAudio OpenStream redirect pipe failed \"%s\" -> \"%s\"", path, redirect);
    }
    if (tramp_StorageOpenStream) return tramp_StorageOpenStream(path, flags, out_error);
    return real_StorageOpenStream ? real_StorageOpenStream(path, flags, out_error) : NULL;
}

static int engine_audio_stringref_from_path_a(const char *path)
{
    static unsigned char storage[MAX_PATH * 4 + sizeof(int)];
    int len;
    if (!path) return 0;
    len = (int)strlen(path);
    if (len <= 0 || len >= (int)(sizeof(storage) - sizeof(int))) return 0;
    *(int*)storage = len;
    memcpy(storage + sizeof(int), path, (size_t)len + 1);
    return (int)(storage + sizeof(int));
}

static int __stdcall hook_ExecuteFileCall(int appBase, int name, int flags, int execute_result)
{
    void *se;
    int use_name = name;
    char redirect[MAX_PATH * 4];
    __asm__ ("movl %%ecx, %0" : "=m"(se));
    redirect[0] = 0;
    if (name && create_engine_audio_script_rewrite_a((const char*)name, redirect, sizeof(redirect))) {
        int ref = engine_audio_stringref_from_path_a(redirect);
        if (ref) {
            use_name = ref;
            debug_line("EngineAudio ExecuteFile redirect \"%s\" -> \"%s\"", (const char*)name, redirect);
        }
    }
    __asm__ ("movl %0, %%ecx" : : "m"(se));
    return real_ExecuteFileCall ? real_ExecuteFileCall(appBase, use_name, flags, execute_result) : 0;
}

static void remember_audio_script_handle(HANDLE h)
{
    int i;
    if (h == INVALID_HANDLE_VALUE || !h) return;
    for (i = 0; i < (int)(sizeof(audio_script_handles) / sizeof(audio_script_handles[0])); i++) {
        if (!audio_script_handles[i] || audio_script_handles[i] == h) {
            audio_script_handles[i] = h;
            return;
        }
    }
}

static void forget_audio_script_handle(HANDLE h)
{
    int i;
    for (i = 0; i < (int)(sizeof(audio_script_handles) / sizeof(audio_script_handles[0])); i++) {
        if (audio_script_handles[i] == h) audio_script_handles[i] = NULL;
    }
}

static void forget_scene_handle(HANDLE h)
{
    int i;
    for (i = 0; i < (int)(sizeof(scene_handles) / sizeof(scene_handles[0])); i++) {
        if (scene_handles[i] == h) scene_handles[i] = NULL;
    }
}

static BOOL WINAPI hook_FindClose(HANDLE h)
{
    remove_virtual_find(h);
    return real_FindClose(h);
}

static virtual_find_t *find_virtual_find(HANDLE h)
{
    int i;
    for (i = 0; i < (int)(sizeof(virtual_finds) / sizeof(virtual_finds[0])); i++) {
        if (virtual_finds[i].handle == h) return &virtual_finds[i];
    }
    return NULL;
}

static int claim_virtual_find_slot(HANDLE h)
{
    int i;
    if (h == INVALID_HANDLE_VALUE) return -1;
    for (i = 0; i < (int)(sizeof(virtual_finds) / sizeof(virtual_finds[0])); i++) {
        if (!virtual_finds[i].handle) {
            virtual_finds[i].handle = h;
            return i;
        }
    }
    return -1;
}

static void add_webm_virtual_entries(HANDLE h, LPCWSTR avi_pattern)
{
    wchar_t webm_pattern[MAX_PATH * 4];
    WIN32_FIND_DATAW data;
    HANDLE wh;
    int slot = -1;
    int i;

    if (h == INVALID_HANDLE_VALUE || !avi_pattern) return;
    for (i = 0; i < (int)(sizeof(virtual_finds) / sizeof(virtual_finds[0])); i++) {
        if (!virtual_finds[i].handle) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return;

    avi_pattern_to_webm_pattern(avi_pattern, webm_pattern, sizeof(webm_pattern) / sizeof(webm_pattern[0]));
    if (!webm_pattern[0]) return;

    wh = real_FindFirstFileW(webm_pattern, &data);
    if (wh == INVALID_HANDLE_VALUE) return;

    virtual_finds[slot].handle = h;
    virtual_finds[slot].after_real = 1;
    do {
        if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            virtual_finds[slot].count < (int)(sizeof(virtual_finds[slot].entries) / sizeof(virtual_finds[slot].entries[0]))) {
            int n = virtual_finds[slot].count++;
            virtual_finds[slot].entries[n] = data;
            lstrcatW(virtual_finds[slot].entries[n].cFileName, L".avi");
        }
    } while (real_FindNextFileW(wh, &data));
    real_FindClose(wh);

    if (!virtual_finds[slot].count) {
        memset(&virtual_finds[slot], 0, sizeof(virtual_finds[slot]));
    }
}

static HANDLE find_first_webm_as_avi(LPCWSTR avi_pattern, LPWIN32_FIND_DATAW d)
{
    wchar_t webm_pattern[MAX_PATH * 4];
    WIN32_FIND_DATAW data;
    HANDLE h;
    int slot;
    int first = 1;

    avi_pattern_to_webm_pattern(avi_pattern, webm_pattern, sizeof(webm_pattern) / sizeof(webm_pattern[0]));
    if (!webm_pattern[0]) return INVALID_HANDLE_VALUE;

    h = real_FindFirstFileW(webm_pattern, &data);
    if (h == INVALID_HANDLE_VALUE) return INVALID_HANDLE_VALUE;

    slot = claim_virtual_find_slot(h);
    if (slot < 0) {
        real_FindClose(h);
        return INVALID_HANDLE_VALUE;
    }

    do {
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (first) {
            *d = data;
            lstrcatW(d->cFileName, L".avi");
            first = 0;
            continue;
        }
        if (virtual_finds[slot].count < (int)(sizeof(virtual_finds[slot].entries) / sizeof(virtual_finds[slot].entries[0]))) {
            int n = virtual_finds[slot].count++;
            virtual_finds[slot].entries[n] = data;
            lstrcatW(virtual_finds[slot].entries[n].cFileName, L".avi");
        }
    } while (real_FindNextFileW(h, &data));

    if (first) {
        remove_virtual_find(h);
        real_FindClose(h);
        return INVALID_HANDLE_VALUE;
    }

    SetLastError(ERROR_SUCCESS);
    return h;
}

static HANDLE WINAPI hook_CreateFileA(LPCSTR p,DWORD a,DWORD s,LPSECURITY_ATTRIBUTES sa,DWORD c,DWORD f,HANDLE t)
{
    HANDLE h;
    char engine_sound_path[MAX_PATH * 4];
    char audio_script_path[MAX_PATH * 4];
    if (should_log_engine_audio_toy_probe_a(p)) {
            debug_line("EngineAudio CreateFileA probe path=\"%s\" access=%08lx create=%08lx", p, a, c);
    }
    if (engine_sound_redirect_path_a(p, engine_sound_path, sizeof(engine_sound_path))) {
        h = real_CreateFileA(engine_sound_path, a, s, sa, c, f, t);
        if (h != INVALID_HANDLE_VALUE) {
            debug_line("EngineAudio redirect \"%s\" -> \"%s\"", p, engine_sound_path);
            return h;
        }
    }
    if (should_skip_active_mod_static_texture_a(p)) {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }
    if (is_mod_texture_path_a(p) && is_texture_probe_name_a(p)) {
        if (is_exact_texture_probe_name_a(p)) {
            log_texture_sidecar_if_present(p);
        }
    }
    if (create_engine_audio_script_rewrite_a(p, audio_script_path, sizeof(audio_script_path))) {
        h = real_CreateFileA(audio_script_path, a, s, sa, c, f, t);
        if (h == INVALID_HANDLE_VALUE) {
            log_line("EngineAudio script open failed \"%s\" err=%lu", audio_script_path, GetLastError());
        } else {
            remember_audio_script_handle(h);
            debug_line("EngineAudio script redirect \"%s\" -> \"%s\"", p, audio_script_path);
        }
        return h;
    }
    h = real_CreateFileA(p,a,s,sa,c,f,t);
    return h;
}

static HANDLE WINAPI hook_CreateFileW(LPCWSTR p,DWORD a,DWORD s,LPSECURITY_ATTRIBUTES sa,DWORD c,DWORD f,HANDLE t)
{
    HANDLE h;
    char mb[MAX_PATH * 4];
    char engine_sound_path[MAX_PATH * 4];
    wchar_t real_path[MAX_PATH * 4];
    wchar_t scene_path[MAX_PATH * 4];
    wchar_t audio_script_path[MAX_PATH * 4];
    wide_to_mb(p, mb, sizeof(mb));
    if (should_log_engine_audio_toy_probe_a(mb)) {
        debug_line("EngineAudio CreateFileW probe path=\"%s\" access=%08lx create=%08lx", mb, a, c);
    }
    if (engine_sound_redirect_path_a(mb, engine_sound_path, sizeof(engine_sound_path))) {
        wchar_t engine_sound_w[MAX_PATH * 4];
        MultiByteToWideChar(CP_ACP, 0, engine_sound_path, -1, engine_sound_w, (int)(sizeof(engine_sound_w) / sizeof(engine_sound_w[0])));
        h = real_CreateFileW(engine_sound_w, a, s, sa, c, f, t);
        if (h != INVALID_HANDLE_VALUE) {
            debug_line("EngineAudio redirect \"%s\" -> \"%s\"", mb, engine_sound_path);
            return h;
        }
    }
    if (should_skip_active_mod_static_texture_a(mb)) {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }
    if (is_mod_texture_path_a(mb) && is_texture_probe_name_a(mb)) {
        if (is_exact_texture_probe_name_a(mb)) {
            log_texture_sidecar_if_present(mb);
        }
    }
    if (webm_alias_to_real_w(p, real_path, sizeof(real_path) / sizeof(real_path[0]))) {
        char real_mb[MAX_PATH * 4];
        HANDLE h;
        wide_to_mb(real_path, real_mb, sizeof(real_mb));
        h = real_CreateFileW(real_path,a,s,sa,c,f,t);
        if (h == INVALID_HANDLE_VALUE) {
            log_line("CreateFileW webm alias failed \"%s\" -> \"%s\" err=%lu", mb, real_mb, GetLastError());
        }
        remember_webm_handle(h);
        return h;
    }
    if (scene_video_rewrite_enabled && create_scene_video_rewrite_w(p, scene_path, sizeof(scene_path) / sizeof(scene_path[0]))) {
        char scene_mb[MAX_PATH * 4];
        wide_to_mb(scene_path, scene_mb, sizeof(scene_mb));
        h = real_CreateFileW(scene_path,a,s,sa,c,f,t);
        if (h == INVALID_HANDLE_VALUE) {
            log_line("SceneVideoRewrite open failed \"%s\" err=%lu", scene_mb, GetLastError());
        } else {
            remember_scene_handle(h);
            debug_line("SceneVideoRewrite redirect \"%s\" -> \"%s\"", mb, scene_mb);
        }
        return h;
    }
    if (create_engine_audio_script_rewrite_w(p, audio_script_path, sizeof(audio_script_path) / sizeof(audio_script_path[0]))) {
        char script_mb[MAX_PATH * 4];
        wide_to_mb(audio_script_path, script_mb, sizeof(script_mb));
        h = real_CreateFileW(audio_script_path,a,s,sa,c,f,t);
        if (h == INVALID_HANDLE_VALUE) {
            log_line("EngineAudio script open failed \"%s\" err=%lu", script_mb, GetLastError());
        } else {
            remember_audio_script_handle(h);
            debug_line("EngineAudio script redirect \"%s\" -> \"%s\"", mb, script_mb);
        }
        return h;
    }
    h = real_CreateFileW(p,a,s,sa,c,f,t);
    return h;
}

static BOOL WINAPI hook_ReadFile(HANDLE h, LPVOID buf, DWORD want, LPDWORD got, LPOVERLAPPED ov)
{
    BOOL ok = real_ReadFile(h, buf, want, got, ov);
    if (is_webm_handle(h)) {
        DWORD n = got ? *got : 0;
        if (!ok) {
            debug_line("ReadFile webm handle=%p want=%lu got=%lu ok=%d err=%lu",
                     h, want, n, ok, GetLastError());
        }
    }
    return ok;
}

static BOOL WINAPI hook_CloseHandle(HANDLE h)
{
    forget_webm_handle(h);
    forget_scene_handle(h);
    forget_audio_script_handle(h);
    return real_CloseHandle(h);
}

static HANDLE WINAPI hook_FindFirstFileA(LPCSTR p, LPWIN32_FIND_DATAA d)
{
    HANDLE h;
    if (should_skip_active_mod_static_texture_a(p)) {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }
    h = real_FindFirstFileA(p, d);
    return h;
}

static BOOL WINAPI hook_FindNextFileA(HANDLE h, LPWIN32_FIND_DATAA d)
{
    BOOL ok = real_FindNextFileA(h, d);
    return ok;
}

static HANDLE WINAPI hook_FindFirstFileW(LPCWSTR p, LPWIN32_FIND_DATAW d)
{
    HANDLE h;
    char mb[MAX_PATH * 4];
    wchar_t real_path[MAX_PATH * 4];
    wide_to_mb(p, mb, sizeof(mb));
    if (should_skip_active_mod_static_texture_a(mb)) {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }
    if (webm_alias_to_real_w(p, real_path, sizeof(real_path) / sizeof(real_path[0]))) {
        char real_mb[MAX_PATH * 4];
        wide_to_mb(real_path, real_mb, sizeof(real_mb));
        h = real_FindFirstFileW(real_path, d);
        if (h != INVALID_HANDLE_VALUE && d) {
            wchar_t alias_name[MAX_PATH];
            alias_basename_w(p, alias_name, sizeof(alias_name) / sizeof(alias_name[0]));
            lstrcpynW(d->cFileName, alias_name, sizeof(d->cFileName) / sizeof(d->cFileName[0]));
        } else {
            log_line("FindFirstFileW webm alias failed \"%s\" -> \"%s\" err=%lu", mb, real_mb, GetLastError());
        }
        return h;
    }
    h = real_FindFirstFileW(p, d);
    if (is_movie_avi_pattern_w(p)) {
        if (h == INVALID_HANDLE_VALUE && d) {
            h = find_first_webm_as_avi(p, d);
        } else {
            add_webm_virtual_entries(h, p);
        }
    }
    return h;
}

static BOOL WINAPI hook_FindNextFileW(HANDLE h, LPWIN32_FIND_DATAW d)
{
    virtual_find_t *vf = find_virtual_find(h);
    BOOL ok;
    if (vf && !vf->after_real && d) {
        if (vf && vf->index < vf->count) {
            *d = vf->entries[vf->index++];
            SetLastError(ERROR_SUCCESS);
            return TRUE;
        }
        remove_virtual_find(h);
        return FALSE;
    }

    ok = real_FindNextFileW(h, d);
    if (!ok && d) {
        vf = find_virtual_find(h);
        if (vf && vf->index < vf->count) {
            *d = vf->entries[vf->index++];
            SetLastError(ERROR_SUCCESS);
            return TRUE;
        }
        remove_virtual_find(h);
    }
    return ok;
}

static FILE *__cdecl hook_fopen(const char *path, const char *mode)
{
    char audio_script_path[MAX_PATH * 4];
    if (should_log_engine_audio_toy_probe_a(path)) {
        debug_line("EngineAudio fopen probe path=\"%s\" mode=\"%s\"", path, mode ? mode : "");
    }
    if (is_mod_texture_path_a(path) && is_texture_probe_name_a(path)) {
        if (is_exact_texture_probe_name_a(path)) {
            log_texture_sidecar_if_present(path);
        }
    }
    if (create_engine_audio_script_rewrite_a(path, audio_script_path, sizeof(audio_script_path))) {
        FILE *f = real_fopen(audio_script_path, mode);
        if (f) {
            debug_line("EngineAudio fopen redirect \"%s\" -> \"%s\"", path, audio_script_path);
            return f;
        }
        log_line("EngineAudio fopen failed \"%s\"", audio_script_path);
    }
    return real_fopen(path, mode);
}

static FILE *__cdecl hook__wfopen(const wchar_t *path, const wchar_t *mode)
{
    char mb[MAX_PATH * 4];
    char audio_script_path[MAX_PATH * 4];
    wide_to_mb(path, mb, sizeof(mb));
    if (should_log_engine_audio_toy_probe_a(mb)) {
        char mode_mb[64];
        wide_to_mb(mode, mode_mb, sizeof(mode_mb));
        debug_line("EngineAudio _wfopen probe path=\"%s\" mode=\"%s\"", mb, mode_mb);
    }
    if (is_mod_texture_path_a(mb) && is_texture_probe_name_a(mb)) {
        if (is_exact_texture_probe_name_a(mb)) {
            log_texture_sidecar_if_present(mb);
        }
    }
    if (create_engine_audio_script_rewrite_a(mb, audio_script_path, sizeof(audio_script_path))) {
        wchar_t audio_script_w[MAX_PATH * 4];
        FILE *f;
        MultiByteToWideChar(CP_ACP, 0, audio_script_path, -1, audio_script_w, (int)(sizeof(audio_script_w) / sizeof(audio_script_w[0])));
        f = real__wfopen(audio_script_w, mode);
        if (f) {
            debug_line("EngineAudio _wfopen redirect \"%s\" -> \"%s\"", mb, audio_script_path);
            return f;
        }
        log_line("EngineAudio _wfopen failed \"%s\"", audio_script_path);
    }
    return real__wfopen(path, mode);
}

static void *__cdecl hook_CreateVideoDecoderByExtension(const char *ext)
{
    void *ret;
    patch_all_modules();
    ret = real_CreateVideoDecoderByExtension ? real_CreateVideoDecoderByExtension(ext) : NULL;
    if (!ret) {
        log_line("CreateVideoDecoderByExtension failed ext=\"%s\"", ext ? ext : "(null)");
    }
    patch_all_modules();
    hook_video_decoder_object(ret);
    return ret;
}

static void *__cdecl hook_CreateTexture2D(void)
{
    void *ret = real_CreateTexture2D ? real_CreateTexture2D() : NULL;
    log_target_pending_object("Texture2D", ret);
    dump_recent_object_vtable("Texture2D", ret, 16);
    return ret;
}

static void *__cdecl hook_CreateExTexture2D(int arg)
{
    void *ret = real_CreateExTexture2D ? real_CreateExTexture2D(arg) : NULL;
    log_target_pending_object("Texture2DEx", ret);
    dump_recent_object_vtable("Texture2DEx", ret, 16);
    return ret;
}

static void *__cdecl hook_CreateTexture2DVideo(void)
{
    void *ret = real_CreateTexture2DVideo ? real_CreateTexture2DVideo() : NULL;
    log_target_pending_object("Texture2DVideo", ret);
    dump_recent_object_vtable("Texture2DVideo", ret, 16);
    return ret;
}

static void *__cdecl hook_CreateExTexture2DVideo(int arg)
{
    void *ret = real_CreateExTexture2DVideo ? real_CreateExTexture2DVideo(arg) : NULL;
    log_target_pending_object("Texture2DVideoEx", ret);
    dump_recent_object_vtable("Texture2DVideoEx", ret, 16);
    return ret;
}

static void *__cdecl hook_CreateImage(void)
{
    void *ret = real_CreateImage ? real_CreateImage() : NULL;
    log_target_pending_object("Image", ret);
    dump_recent_object_vtable("Image", ret, 20);
    return ret;
}

static void *__cdecl hook_CreateExImage(int arg)
{
    void *ret = real_CreateExImage ? real_CreateExImage(arg) : NULL;
    log_target_pending_object("ImageEx", ret);
    dump_recent_object_vtable("ImageEx", ret, 20);
    return ret;
}

static void *__cdecl hook_CreateWImage(void)
{
    void *ret = real_CreateWImage ? real_CreateWImage() : NULL;
    log_target_pending_object("WImage", ret);
    dump_recent_object_vtable("WImage", ret, 20);
    return ret;
}

static void *__cdecl hook_CreateExWImage(int arg)
{
    void *ret = real_CreateExWImage ? real_CreateExWImage(arg) : NULL;
    log_target_pending_object("WImageEx", ret);
    dump_recent_object_vtable("WImageEx", ret, 20);
    return ret;
}

static void THISCALL hook_SetVideoPath(void *self, void *a, void *b, void *c, int mode)
{
    (void)self;
    (void)a;
    (void)b;
    (void)c;
    (void)mode;
    real_SetVideoPath(self, a, b, c, mode);
}

static void THISCALL hook_SetVideoDimension(void *self, void *vec2, int flag)
{
    real_SetVideoDimension(self, vec2, flag);
}

static int __cdecl hook_ReplaceImage(void *obj, void *texture_name, void *image_file, void *image_suffix)
{
    if (tramp_ReplaceImage) return tramp_ReplaceImage(obj, texture_name, image_file, image_suffix);
    return real_ReplaceImage ? real_ReplaceImage(obj, texture_name, image_file, image_suffix) : 0;
}

static int THISCALL hook_StreamPipeFileRead(void *self, void *buf, int len)
{
    if (tramp_StreamPipeFileRead) return tramp_StreamPipeFileRead(self, buf, len);
    return real_StreamPipeFileRead ? real_StreamPipeFileRead(self, buf, len) : 0;
}

static int THISCALL hook_StreamPipeFileCacheRead(void *self, void *buf, int len)
{
    if (tramp_StreamPipeFileCacheRead) return tramp_StreamPipeFileCacheRead(self, buf, len);
    return real_StreamPipeFileCacheRead ? real_StreamPipeFileCacheRead(self, buf, len) : 0;
}

static int THISCALL hook_PngImport(void *self, int a, void *stream, int c, int d)
{
    if (tramp_PngImport) return tramp_PngImport(self, a, stream, c, d);
    return real_PngImport ? real_PngImport(self, a, stream, c, d) : 0x80000008;
}

static void *THISCALL hook_SoundDeviceCreateSource(void *device, const char *name,
                                                    unsigned int flags, int cache_mode)
{
    void *source = real_SoundDevice_CreateSource ?
        real_SoundDevice_CreateSource(device, name, flags, cache_mode) : NULL;
    if (source) game_audio_track_source(source, name);
    return source;
}

static void *__cdecl hook_SoundDeviceCreate(int sd_type, int speaker_type, int quality, void *window)
{
    void *ret = NULL;
    if (tramp_SoundDevice_Create) {
        ret = tramp_SoundDevice_Create(sd_type, speaker_type, quality, window);
    } else if (real_SoundDevice_Create && real_SoundDevice_Create != hook_SoundDeviceCreate) {
        ret = real_SoundDevice_Create(sd_type, speaker_type, quality, window);
    }
    if (ret) {
        engine_captured_sound_device = ret;
        if (sound_source_destroy_inline_installed &&
            patch_vtable_slot(ret, 1, (void*)hook_SoundDeviceCreateSource,
                              (void**)&real_SoundDevice_CreateSource)) {
            if (!sound_device_create_source_vtable_installed) {
                sound_device_create_source_vtable_installed = 1;
                debug_line("GameAudio SoundDevice source vtable hook installed device=%p original=%p",
                           ret, (void*)real_SoundDevice_CreateSource);
            }
        } else if (sound_source_destroy_inline_installed) {
            log_line("GameAudio SoundDevice source vtable hook install failed device=%p", ret);
        }
    }
    debug_line("EngineAudio SoundDevice::Create type=%d speaker=%d quality=%d window=%p ret=%p slot=%p slot_device=%p",
             sd_type, speaker_type, quality, window, ret,
             (void*)engine_SoundDevice_ptr,
             engine_SoundDevice_ptr ? *engine_SoundDevice_ptr : NULL);
    return ret;
}

static void *THISCALL hook_AppTracker_GetCameraTransform(void *self)
{
    void *ret = NULL;
    if (tramp_AppTracker_GetCameraTransform) {
        ret = tramp_AppTracker_GetCameraTransform(self);
    } else if (real_AppTracker_GetCameraTransform && real_AppTracker_GetCameraTransform != hook_AppTracker_GetCameraTransform) {
        ret = real_AppTracker_GetCameraTransform(self);
    }
    if (ret) engine_captured_camera_transform = ret;
    return ret;
}

static void THISCALL hook_AppTracker_SetCameraTransform(void *self, void *camera_transform)
{
    if (camera_transform) {
        engine_captured_camera_transform = camera_transform;
        debug_line("Audio3D AppTracker::SetCameraTransform self=%p transform=%p", self, camera_transform);
    }
    if (tramp_AppTracker_SetCameraTransform) {
        tramp_AppTracker_SetCameraTransform(self, camera_transform);
    } else if (real_AppTracker_SetCameraTransform && real_AppTracker_SetCameraTransform != hook_AppTracker_SetCameraTransform) {
        real_AppTracker_SetCameraTransform(self, camera_transform);
    }
}

static void THISCALL hook_AppTracker_SetWorldMatrixInverse(void *self, const float *matrix)
{
    static DWORD last_log_tick;
    DWORD now;
    if (matrix) {
        memcpy(engine_captured_camera_inverse, matrix, sizeof(engine_captured_camera_inverse));
        engine_captured_camera_inverse_valid = 1;
        engine_captured_camera_inverse_tick = GetTickCount();
        now = engine_captured_camera_inverse_tick;
        if (!last_log_tick || now - last_log_tick > 2000) {
            last_log_tick = now;
            debug_line("Audio3D AppTracker::SetWorldMatrixInverse self=%p pos=(%.2f %.2f %.2f) front=(%.2f %.2f %.2f) top=(%.2f %.2f %.2f)",
                       self,
                       engine_captured_camera_inverse[12],
                       engine_captured_camera_inverse[13],
                       engine_captured_camera_inverse[14],
                       -engine_captured_camera_inverse[8],
                       -engine_captured_camera_inverse[9],
                       -engine_captured_camera_inverse[10],
                       engine_captured_camera_inverse[4],
                       engine_captured_camera_inverse[5],
                       engine_captured_camera_inverse[6]);
        }
    }
    if (tramp_AppTracker_SetWorldMatrixInverse) {
        tramp_AppTracker_SetWorldMatrixInverse(self, matrix);
    } else if (real_AppTracker_SetWorldMatrixInverse && real_AppTracker_SetWorldMatrixInverse != hook_AppTracker_SetWorldMatrixInverse) {
        real_AppTracker_SetWorldMatrixInverse(self, matrix);
    }
}

static int install_inline_hook(void *target, void *hook, size_t stolen_len, void **trampoline)
{
    BYTE *tramp;
    DWORD old;
    DWORD rel;
    size_t i;
    if (!target || !hook || !trampoline || *trampoline || stolen_len < 5) return 0;

    if (*(BYTE*)target == 0xe9) {
        DWORD old_rel;
        memcpy(&old_rel, (BYTE*)target + 1, sizeof(old_rel));
        *trampoline = (void*)((BYTE*)target + 5 + (LONG)old_rel);
        if (!VirtualProtect(target, 5, PAGE_EXECUTE_READWRITE, &old)) {
            *trampoline = NULL;
            return 0;
        }
        rel = (DWORD)((BYTE*)hook - ((BYTE*)target + 5));
        memcpy((BYTE*)target + 1, &rel, sizeof(rel));
        VirtualProtect(target, 5, old, &old);
        FlushInstructionCache(GetCurrentProcess(), target, 5);
        return 1;
    }

    tramp = (BYTE*)VirtualAlloc(NULL, stolen_len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!tramp) return 0;
    memcpy(tramp, target, stolen_len);
    tramp[stolen_len] = 0xe9;
    rel = (DWORD)(((BYTE*)target + stolen_len) - (tramp + stolen_len + 5));
    memcpy(tramp + stolen_len + 1, &rel, sizeof(rel));

    if (!VirtualProtect(target, stolen_len, PAGE_EXECUTE_READWRITE, &old)) {
        VirtualFree(tramp, 0, MEM_RELEASE);
        return 0;
    }
    ((BYTE*)target)[0] = 0xe9;
    rel = (DWORD)((BYTE*)hook - ((BYTE*)target + 5));
    memcpy((BYTE*)target + 1, &rel, sizeof(rel));
    for (i = 5; i < stolen_len; i++) ((BYTE*)target)[i] = 0x90;
    VirtualProtect(target, stolen_len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), target, stolen_len);
    *trampoline = tramp;
    return 1;
}

static video_gl_texture_t *current_gl_uv_video_texture(void)
{
    video_gl_texture_t *vt = current_gl_video_slot;
    int mode;
    if (!vt || !vt->active || vt->texture != current_gl_texture_2d ||
        !vt->uv_rect_valid || vt->uploaded_frame_index < 0) return NULL;
    mode = vt->twitch_active ? vt->uv_mode_twitch : vt->uv_mode_base;
    return mode == WEBM_UV_MODE_FULL_TEXTURE ? vt : NULL;
}

static int begin_gl_uv_override(GLint *saved_matrix_mode)
{
    video_gl_texture_t *vt = current_gl_uv_video_texture();
    float matrix[16];
    float width;
    float height;
    if (!vt || !saved_matrix_mode) return 0;
    resolve_gl_runtime_functions();
    if (!real_glGetIntegerv || !real_glMatrixMode ||
        !real_glPushMatrix || !real_glPopMatrix || !real_glLoadMatrixf) return 0;
    width = vt->uv_max_u - vt->uv_min_u;
    height = vt->uv_max_v - vt->uv_min_v;
    if (width <= 0.000001f || height <= 0.000001f) return 0;
    memset(matrix, 0, sizeof(matrix));
    matrix[0] = 1.0f / width;
    matrix[5] = 1.0f / height;
    matrix[10] = 1.0f;
    matrix[15] = 1.0f;
    matrix[12] = -vt->uv_min_u / width;
    matrix[13] = -vt->uv_min_v / height;
    real_glGetIntegerv(0x0BA0, saved_matrix_mode); /* GL_MATRIX_MODE */
    real_glMatrixMode(0x1702);                    /* GL_TEXTURE */
    real_glPushMatrix();
    real_glLoadMatrixf(matrix);
    if (vt->video_texture && real_glBindTexture) {
        real_glBindTexture(0x0DE1, vt->video_texture);
    }
    if (!vt->uv_override_logged) {
        debug_line("VideoUV full_texture active source=opengl rect=(%.6f,%.6f)-(%.6f,%.6f) sidecar=\"%s\"",
                   vt->uv_min_u, vt->uv_min_v, vt->uv_max_u, vt->uv_max_v, vt->sidecar_path);
        vt->uv_override_logged = 1;
    }
    return 1;
}

static void end_gl_uv_override(GLint saved_matrix_mode)
{
    if (!real_glMatrixMode || !real_glPopMatrix) return;
    if (real_glBindTexture) {
        real_glBindTexture(0x0DE1, current_gl_texture_2d);
    }
    real_glMatrixMode(0x1702); /* GL_TEXTURE */
    real_glPopMatrix();
    real_glMatrixMode((GLenum)saved_matrix_mode);
}

static void WINAPI hook_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    GLint saved_matrix_mode = 0;
    int active = begin_gl_uv_override(&saved_matrix_mode);
    if (real_glDrawArrays) real_glDrawArrays(mode, first, count);
    if (active) end_gl_uv_override(saved_matrix_mode);
}

static void WINAPI hook_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    GLint saved_matrix_mode = 0;
    int active = begin_gl_uv_override(&saved_matrix_mode);
    if (real_glDrawElements) real_glDrawElements(mode, count, type, indices);
    if (active) end_gl_uv_override(saved_matrix_mode);
}

static void WINAPI hook_glBindTexture(GLenum target, unsigned int texture)
{
    if (target == 0x0DE1) {
        int i;
        current_gl_texture_2d = texture;
        current_gl_video_slot = NULL;
        for (i = 0; i < video_gl_active_count; i++) {
            video_gl_texture_t *vt = video_gl_active_slots[i];
            if (vt && vt->texture == texture) {
                current_gl_video_slot = vt;
                vt->last_bound_tick = GetTickCount();
                vt->bind_log_count++;
                break;
            }
        }
    }
    if (real_glBindTexture) real_glBindTexture(target, texture);
}

static void WINAPI hook_glDeleteTextures(GLsizei count, const unsigned int *textures)
{
    int i;
    int j;
    if (count > 0 && textures) {
        for (i = 0; i < count; i++) {
            for (j = 0; j < (int)(sizeof(video_gl_textures) / sizeof(video_gl_textures[0])); j++) {
                video_gl_texture_t *slot = &video_gl_textures[j];
                if (slot->active && slot->texture == textures[i]) {
                    debug_line("VideoTexture released tex=%u sidecar=\"%s\"", textures[i], slot->sidecar_path);
                    clear_video_gl_texture_slot(slot);
                }
            }
            if (current_gl_texture_2d == textures[i]) {
                current_gl_texture_2d = 0;
                current_gl_video_slot = NULL;
            }
        }
    }
    if (real_glDeleteTextures) real_glDeleteTextures(count, textures);
}

static int gl_bytes_per_pixel(GLenum format, GLenum type)
{
    if (type != 0x1401) return 0;
    if (format == 0x1907) return 3;
    if (format == 0x1908) return 4;
    return 0;
}

static int gl_output_cache_format(GLenum format, GLenum type)
{
    if (type != 0x1401) return 0;
    if (format == 0x1907) return WEBM_CACHE_FORMAT_GL_RGB;
    if (format == 0x1908) return WEBM_CACHE_FORMAT_GL_RGBA;
    return 0;
}

static int video_decoder_copy_gl_cached_frame(video_decoder_t *dec, BYTE *pixels,
                                               size_t pixels_size, int width, int height,
                                               GLenum format, GLenum type)
{
    int cache_format = gl_output_cache_format(format, type);
    int bpp = gl_bytes_per_pixel(format, type);
    size_t bytes;
    int copied = 0;
    d3d8_cached_mip_view_t view;
    if (!dec || !dec->async_enabled || !dec->async_lock_initialized || !pixels ||
        cache_format == 0 || bpp <= 0 || width <= 0 || height <= 0) return 0;
    bytes = (size_t)width * (size_t)height * (size_t)bpp;
    if (!bytes || bytes > pixels_size) return 0;
    if (video_decoder_pin_d3d8_cached_mip(dec, 0, width, height, cache_format, &view)) {
        if (view.pitch == width * bpp) {
            memcpy(pixels, view.pixels, bytes);
            copied = 1;
        }
        video_decoder_unpin_d3d8_cached_mip(dec, &view);
    }
    return copied;
}

static void clear_pending_gl_texture_probe(void)
{
    target_texture_probe_path[0] = 0;
    target_texture_sidecar_path[0] = 0;
    target_texture_probe_tick = 0;
    target_texture_probe_width = 0;
    target_texture_probe_height = 0;
    target_texture_interval_ms = texture_video_interval_ms;
    target_texture_max_width = texture_max_width;
    target_texture_max_height = texture_max_height;
    target_texture_d3d8_mip_levels = d3d8_mip_levels;
    target_texture_video_filtering = video_filtering;
    target_texture_anisotropy = video_anisotropy;
}

static int patch_vtable_slot(void *obj, int index, void *hook, void **real)
{
    void **vt;
    DWORD old;
    if (!obj || !hook) return 0;
    vt = *(void***)obj;
    if (!vt || !ptr_readable(vt, sizeof(void*) * (index + 1))) return 0;
    if (real && !*real && vt[index] != hook) *real = vt[index];
    if (vt[index] == hook) return 1;
    if (!VirtualProtect(&vt[index], sizeof(void*), PAGE_EXECUTE_READWRITE, &old)) return 0;
    vt[index] = hook;
    VirtualProtect(&vt[index], sizeof(void*), old, &old);
    return 1;
}

static int game_audio_name_matches_a(const game_audio_mute_source_t *source, const char *target)
{
    size_t source_length;
    size_t target_length;
    if (!source || !target || !target[0]) return 0;
    if (strchr(target, '/')) {
        if (_stricmp(source->full_name, target) == 0) return 1;
        source_length = strlen(source->full_name);
        target_length = strlen(target);
        return source_length > target_length &&
               source->full_name[source_length - target_length - 1] == '/' &&
               _stricmp(source->full_name + source_length - target_length, target) == 0;
    }
    return _stricmp(source->base_name, target) == 0;
}

static int game_audio_source_should_mute(const game_audio_mute_source_t *source,
                                         const game_audio_mute_list_t *list)
{
    int i;
    if (!source || !list) return 0;
    for (i = 0; i < list->count; i++) {
        if (game_audio_name_matches_a(source, list->names[i])) return 1;
    }
    return 0;
}

static void game_audio_set_source_muted(game_audio_mute_source_t *entry, int mute)
{
    if (!entry || !entry->source || !engine_SoundSource_SetVolume) return;
    if (mute) {
        if (!entry->muted) {
            entry->saved_volume = engine_SoundSource_GetVolume ?
                engine_SoundSource_GetVolume(entry->source) : 1.0f;
            entry->muted = 1;
            debug_line("GameAudio muted source=\"%s\" volume=%.3f", entry->full_name, entry->saved_volume);
        }
        /* Apply an absolute volume. Room scripts may set their source volume
           again after creation, so active mutes are periodically reinforced. */
        engine_SoundSource_SetVolume(entry->source, WEBM_GAME_AUDIO_SILENT_VOLUME, 0);
    } else if (!mute && entry->muted) {
        engine_SoundSource_SetVolume(entry->source, entry->saved_volume, 0);
        entry->muted = 0;
        debug_line("GameAudio restored source=\"%s\" volume=%.3f", entry->full_name, entry->saved_volume);
    }
}

static void game_audio_track_source(void *source, const char *name)
{
    char normalized[WEBM_GAME_AUDIO_MUTE_NAME_MAX];
    const char *base;
    game_audio_mute_source_t *entry = NULL;
    int i;
    if (!game_audio_mute_lock_ready || !source || !name) return;
    normalize_game_audio_name_a(name, normalized, sizeof(normalized));
    if (!normalized[0]) return;
    base = strrchr(normalized, '/');
    base = base ? base + 1 : normalized;
    EnterCriticalSection(&game_audio_mute_lock);
    for (i = 0; i < game_audio_mute_source_count; i++) {
        if (game_audio_mute_sources[i].source == source) {
            entry = &game_audio_mute_sources[i];
            break;
        }
    }
    if (!entry && game_audio_mute_source_count < WEBM_GAME_AUDIO_MUTE_MAX_SOURCES) {
        entry = &game_audio_mute_sources[game_audio_mute_source_count++];
        memset(entry, 0, sizeof(*entry));
        entry->source = source;
    }
    if (entry) {
        lstrcpynA(entry->full_name, normalized, sizeof(entry->full_name));
        lstrcpynA(entry->base_name, base, sizeof(entry->base_name));
        game_audio_set_source_muted(entry,
            game_audio_source_should_mute(entry, &game_audio_active_mutes));
    }
    LeaveCriticalSection(&game_audio_mute_lock);
    if (!entry) log_line("GameAudio source registry full; source=\"%s\"", normalized);
}

static void game_audio_untrack_source(void *source)
{
    int i;
    if (!game_audio_mute_lock_ready || !source) return;
    EnterCriticalSection(&game_audio_mute_lock);
    for (i = 0; i < game_audio_mute_source_count; i++) {
        if (game_audio_mute_sources[i].source == source) {
            game_audio_mute_source_count--;
            if (i != game_audio_mute_source_count) {
                game_audio_mute_sources[i] = game_audio_mute_sources[game_audio_mute_source_count];
            }
            memset(&game_audio_mute_sources[game_audio_mute_source_count], 0,
                   sizeof(game_audio_mute_sources[0]));
            break;
        }
    }
    LeaveCriticalSection(&game_audio_mute_lock);
}

static void THISCALL hook_SoundSourceDestroy(void *source)
{
    game_audio_untrack_source(source);
    if (tramp_SoundSource_Destroy) {
        tramp_SoundSource_Destroy(source);
    } else if (real_SoundSource_Destroy && real_SoundSource_Destroy != hook_SoundSourceDestroy) {
        real_SoundSource_Destroy(source);
    }
}

static int game_audio_gl_slot_displays_plugin(const video_gl_texture_t *slot)
{
    int mode;
    if (!slot || !slot->active || slot->uploaded_frame_index < 0 || !slot->game_audio_mutes.count) return 0;
    if (!slot->video_texture) return 1;
    mode = slot->twitch_active ? slot->uv_mode_twitch : slot->uv_mode_base;
    return slot->uv_rect_valid && mode == WEBM_UV_MODE_FULL_TEXTURE;
}

static int game_audio_d3d8_slot_displays_plugin(const video_d3d8_texture_t *slot)
{
    int mode;
    if (!slot || !slot->active || slot->uploaded_frame_index < 0 || !slot->game_audio_mutes.count) return 0;
    if (!slot->video_texture) return 1;
    mode = slot->twitch_active ? slot->uv_mode_twitch : slot->uv_mode_base;
    return slot->uv_rect_valid && mode == WEBM_UV_MODE_FULL_TEXTURE;
}

static void game_audio_collect_active_mutes(game_audio_mute_list_t *active)
{
    int i;
    int j;
    if (!active) return;
    memset(active, 0, sizeof(*active));
    for (i = 0; i < video_gl_active_count; i++) {
        video_gl_texture_t *slot = video_gl_active_slots[i];
        if (!game_audio_gl_slot_displays_plugin(slot)) continue;
        for (j = 0; j < slot->game_audio_mutes.count; j++) {
            game_audio_mute_list_add_a(active, slot->game_audio_mutes.names[j]);
        }
    }
    for (i = 0; i < video_d3d8_active_count; i++) {
        video_d3d8_texture_t *slot = video_d3d8_active_slots[i];
        if (!game_audio_d3d8_slot_displays_plugin(slot)) continue;
        for (j = 0; j < slot->game_audio_mutes.count; j++) {
            game_audio_mute_list_add_a(active, slot->game_audio_mutes.names[j]);
        }
    }
}

static void game_audio_refresh_mutes(void)
{
    game_audio_mute_list_t active;
    DWORD now;
    int changed;
    int enforce;
    int i;
    if (!game_audio_mute_lock_ready) return;
    game_audio_collect_active_mutes(&active);
    now = GetTickCount();
    EnterCriticalSection(&game_audio_mute_lock);
    changed = memcmp(&active, &game_audio_active_mutes, sizeof(active)) != 0;
    enforce = changed || !game_audio_mute_last_enforce_tick ||
              (DWORD)(now - game_audio_mute_last_enforce_tick) >= 250u;
    if (changed) {
        game_audio_active_mutes = active;
    }
    if (enforce) {
        for (i = 0; i < game_audio_mute_source_count; i++) {
            game_audio_set_source_muted(&game_audio_mute_sources[i],
                game_audio_source_should_mute(&game_audio_mute_sources[i], &game_audio_active_mutes));
        }
        game_audio_mute_last_enforce_tick = now;
    }
    LeaveCriticalSection(&game_audio_mute_lock);
}

static video_d3d8_texture_t *find_d3d8_texture_slot(IDirect3DTexture8 *texture)
{
    int i;
    video_d3d8_texture_t *free_slot = NULL;
    for (i = 0; i < video_d3d8_active_count; i++) {
        if (video_d3d8_active_slots[i] && video_d3d8_active_slots[i]->texture == texture) {
            return video_d3d8_active_slots[i];
        }
    }
    for (i = 0; i < (int)(sizeof(video_d3d8_textures) / sizeof(video_d3d8_textures[0])); i++) {
        if (!video_d3d8_textures[i].active && !free_slot) free_slot = &video_d3d8_textures[i];
    }
    return free_slot;
}

static void clear_d3d8_texture_slot(video_d3d8_texture_t *slot)
{
    IDirect3DTexture8 *texture;
    IDirect3DTexture8 *video_texture;
    int mip_level;
    int stage;
    if (!slot) return;
    if (slot->video_texture) restore_hook5_proxy_resource_aliases();
    twitch_override_auto_release_sidecar_a(slot->sidecar_ini_path);
    for (stage = 0; stage < 8; stage++) {
        if (d3d8_bound_video_slots[stage] == slot) {
            d3d8_bound_video_slots[stage] = NULL;
        }
    }
    texture = slot->texture;
    video_texture = slot->video_texture;
    if (slot->active) unregister_active_d3d8_slot(slot);
    if (slot->audio_graph) {
        audio_graph_release(slot->audio_graph);
        slot->audio_graph = NULL;
    }
    if (slot->engine_audio) {
        engine_audio_player_release(slot->engine_audio);
        slot->engine_audio = NULL;
    }
    if (slot->decoder) {
        video_decoder_release(slot->decoder);
        slot->decoder = NULL;
    }
    if (slot->twitch_open_task) {
        twitch_decoder_open_task_release(slot->twitch_open_task);
        slot->twitch_open_task = NULL;
    }
    if (slot->twitch_session) {
        webm_twitch_session_destroy(slot->twitch_session);
        slot->twitch_session = NULL;
    }
    if (slot->twitch_chat) {
        webm_twitch_chat_release(slot->twitch_chat);
        slot->twitch_chat = NULL;
    }
    free(slot->twitch_chat_base_pixels);
    slot->twitch_chat_base_pixels = NULL;
    slot->twitch_chat_base_size = 0;
    for (mip_level = 0; mip_level < WEBM_TWITCH_CHAT_MIP_LEVELS; mip_level++) {
        free(slot->twitch_chat_mip_pixels[mip_level]);
        slot->twitch_chat_mip_pixels[mip_level] = NULL;
        slot->twitch_chat_mip_sizes[mip_level] = 0;
        slot->twitch_chat_mip_pitches[mip_level] = 0;
    }
    if (slot->d3d11_sampler) {
        ID3D11SamplerState_Release(slot->d3d11_sampler);
        slot->d3d11_sampler = NULL;
    }
    if (slot->d3d11_texture) {
        ID3D11Texture2D_Release(slot->d3d11_texture);
        slot->d3d11_texture = NULL;
    }
    memset(slot, 0, sizeof(*slot));
    if (video_texture) IDirect3DTexture8_Release(video_texture);
    if (texture) IDirect3DTexture8_Release(texture);
}

static void clear_all_d3d8_texture_slots(void)
{
    int i;
    for (i = 0; i < (int)(sizeof(video_d3d8_textures) / sizeof(video_d3d8_textures[0])); i++) {
        clear_d3d8_texture_slot(&video_d3d8_textures[i]);
    }
    d3d8_last_global_update_tick = 0;
}

static int d3d11_upload_format_matches(D3DFORMAT d3d8_format, DXGI_FORMAT dxgi_format)
{
    if (d3d8_format == D3DFMT_A8R8G8B8) {
        return dxgi_format == DXGI_FORMAT_B8G8R8A8_UNORM ||
               dxgi_format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    }
    if (d3d8_format == D3DFMT_X8R8G8B8) {
        return dxgi_format == DXGI_FORMAT_B8G8R8X8_UNORM ||
               dxgi_format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
    }
    if (d3d8_format == D3DFMT_R5G6B5) {
        return dxgi_format == DXGI_FORMAT_B5G6R5_UNORM;
    }
    return 0;
}

static void remember_d3d8_texture(IDirect3DDevice8 *device, IDirect3DTexture8 *texture, UINT width, UINT height,
                                  UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool,
                                  ID3D11Texture2D *d3d11_candidate)
{
    video_d3d8_texture_t *slot;
    IDirect3DTexture8 *video_texture = NULL;
    ID3D11Texture2D *proxy_d3d11_candidate = NULL;
    ID3D11Texture2D *upload_d3d11_candidate = d3d11_candidate;
    DWORD now = GetTickCount();
    int uv_mode_base = WEBM_UV_MODE_OFF;
    int uv_mode_twitch = WEBM_UV_MODE_OFF;
    int uv_rect_valid = 0;
    float uv_min_u = 0.0f, uv_min_v = 0.0f, uv_max_u = 1.0f, uv_max_v = 1.0f;
    int upload_width = (int)width;
    int upload_height = (int)height;
    int oversized;
    if (!target_texture_probe_path[0] || !texture || width <= 0 || height <= 0) return;
    if (target_texture_probe_tick && (now - target_texture_probe_tick) > 2500) {
        clear_pending_gl_texture_probe();
        return;
    }
    if (target_texture_probe_width > 0 && target_texture_probe_height > 0 &&
        ((int)width != target_texture_probe_width || (int)height != target_texture_probe_height)) {
        return;
    }
    if (format != D3DFMT_A8R8G8B8 && format != D3DFMT_X8R8G8B8 &&
        format != D3DFMT_R8G8B8 && format != D3DFMT_R5G6B5) {
        log_line("D3D8Texture skip unsupported format=%d size=%ux%u path=\"%s\"",
                 (int)format, width, height, target_texture_probe_path);
        return;
    }
    load_sidecar_uv_settings_a(target_texture_sidecar_path,
                               &uv_mode_base, &uv_mode_twitch, &uv_rect_valid,
                               &uv_min_u, &uv_min_v, &uv_max_u, &uv_max_v);
    oversized = (int)width > target_texture_max_width || (int)height > target_texture_max_height;
    if (oversized) {
        HRESULT proxy_hr;
        if (!device || !uv_rect_valid ||
            (uv_mode_base != WEBM_UV_MODE_FULL_TEXTURE &&
             uv_mode_twitch != WEBM_UV_MODE_FULL_TEXTURE) ||
            !video_uv_proxy_dimensions((int)width, (int)height,
                                       target_texture_max_width, target_texture_max_height,
                                       uv_min_u, uv_min_v, uv_max_u, uv_max_v,
                                       &upload_width, &upload_height)) {
            log_line("D3D8Texture skip oversized tex=%p size=%ux%u sidecar=\"%s\"",
                     texture, width, height, target_texture_sidecar_path);
            return;
        }
        d3d11_texture_capture_begin((UINT)upload_width, (UINT)upload_height);
        proxy_hr = real_d3d8_CreateTexture ?
            real_d3d8_CreateTexture(device, (UINT)upload_width, (UINT)upload_height, 0, 0,
                                    format, D3DPOOL_MANAGED, &video_texture) : D3DERR_INVALIDCALL;
        proxy_d3d11_candidate = d3d11_texture_capture_finish();
        if (FAILED(proxy_hr) || !video_texture) {
            if (proxy_d3d11_candidate) ID3D11Texture2D_Release(proxy_d3d11_candidate);
            log_line("D3D8Texture proxy creation failed source=%ux%u proxy=%dx%d format=%d hr=0x%08lx sidecar=\"%s\"",
                     width, height, upload_width, upload_height, (int)format,
                     (unsigned long)proxy_hr, target_texture_sidecar_path);
            return;
        }
        if (directx_d3d11_upload && captured_d3d11_context && !proxy_d3d11_candidate) {
            log_line("D3D11 direct upload proxy match unavailable source=%ux%u proxy=%dx%d fallback=D3D8 sidecar=\"%s\"",
                     width, height, upload_width, upload_height, target_texture_sidecar_path);
        }
        upload_d3d11_candidate = proxy_d3d11_candidate;
        log_line("D3D8Texture runtime proxy source=%ux%u proxy=%dx%d sidecar=\"%s\"",
                 width, height, upload_width, upload_height, target_texture_sidecar_path);
    }
    slot = find_d3d8_texture_slot(texture);
    if (!slot) {
        if (proxy_d3d11_candidate) ID3D11Texture2D_Release(proxy_d3d11_candidate);
        if (video_texture) IDirect3DTexture8_Release(video_texture);
        return;
    }
    clear_d3d8_texture_slot(slot);
    slot->active = 1;
    register_active_d3d8_slot(slot);
    slot->texture = texture;
    slot->video_texture = video_texture;
    IDirect3DTexture8_AddRef(texture);
    slot->width = upload_width;
    slot->height = upload_height;
    slot->levels = levels;
    slot->level_count = IDirect3DTexture8_GetLevelCount(video_texture ? video_texture : texture);
    slot->usage = video_texture ? 0 : usage;
    slot->format = format;
    slot->pool = video_texture ? D3DPOOL_MANAGED : pool;
    if (directx_d3d11_upload && upload_d3d11_candidate && captured_d3d11_context) {
        D3D11_TEXTURE2D_DESC desc;
        memset(&desc, 0, sizeof(desc));
        ID3D11Texture2D_GetDesc(upload_d3d11_candidate, &desc);
        if (desc.Width == (UINT)slot->width && desc.Height == (UINT)slot->height &&
            desc.ArraySize == 1 && desc.SampleDesc.Count == 1 &&
            desc.Usage == D3D11_USAGE_DEFAULT && desc.CPUAccessFlags == 0 &&
            (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0 &&
            d3d11_upload_format_matches(format, desc.Format)) {
            ID3D11Texture2D_AddRef(upload_d3d11_candidate);
            slot->d3d11_texture = upload_d3d11_candidate;
            slot->d3d11_direct_upload_ready = 1;
            slot->d3d11_runtime_generation = captured_d3d11_generation;
            slot->d3d11_mip_levels = desc.MipLevels;
            log_line("D3D11 direct upload matched d3d8=%p d3d11=%p size=%ux%u mips=%u format=%d sidecar=\"%s\"",
                     video_texture ? video_texture : texture, upload_d3d11_candidate,
                     desc.Width, desc.Height, desc.MipLevels,
                     (int)desc.Format, target_texture_sidecar_path);
        } else {
            debug_line("D3D11 direct upload rejected d3d8=%p d3d11=%p size=%ux%u expected=%dx%d mips=%u usage=%d bind=0x%08lx cpu=0x%08lx samples=%u format=%d d3d8_format=%d",
                       video_texture ? video_texture : texture, upload_d3d11_candidate,
                       desc.Width, desc.Height,
                       slot->width, slot->height, desc.MipLevels, (int)desc.Usage,
                       (unsigned long)desc.BindFlags, (unsigned long)desc.CPUAccessFlags,
                       desc.SampleDesc.Count, (int)desc.Format, (int)format);
        }
    }
    if (proxy_d3d11_candidate) ID3D11Texture2D_Release(proxy_d3d11_candidate);
    slot->first_seen_tick = now;
    slot->last_bound_tick = slot->first_seen_tick;
    slot->last_bound_present_serial = d3d8_present_serial;
    slot->update_interval_ms = target_texture_interval_ms ? target_texture_interval_ms : texture_video_interval_ms;
    slot->uploaded_frame_index = -1;
    slot->uploaded_frame_serial = 0;
    slot->d3d8_mip_levels = target_texture_d3d8_mip_levels;
    slot->video_filtering = target_texture_video_filtering;
    slot->anisotropy = target_texture_anisotropy;
    lstrcpynA(slot->image_path, target_texture_probe_path, sizeof(slot->image_path));
    lstrcpynA(slot->sidecar_path, target_texture_sidecar_path, sizeof(slot->sidecar_path));
    slot->uv_mode_base = uv_mode_base;
    slot->uv_mode_twitch = uv_mode_twitch;
    slot->uv_rect_valid = uv_rect_valid;
    slot->uv_min_u = uv_min_u;
    slot->uv_min_v = uv_min_v;
    slot->uv_max_u = uv_max_u;
    slot->uv_max_v = uv_max_v;
    load_sidecar_audio_settings_a(slot->sidecar_path, &slot->audio_enabled, &slot->audio_engine, &slot->audio_lead_ms, &slot->audio_volume,
                                  &slot->audio_3d, &slot->audio_3d_min_distance, &slot->audio_3d_max_distance,
                                  &slot->audio_3d_rolloff, slot->audio_node, sizeof(slot->audio_node),
                                  slot->audio_effect, sizeof(slot->audio_effect));
    load_sidecar_game_audio_mutes_a(slot->sidecar_path, &slot->game_audio_mutes);
    slot->audio_ready_tick = now + 3000;
    slot->playback_start_tick = 0;
    sidecar_ini_path_a(slot->sidecar_path, slot->sidecar_ini_path, sizeof(slot->sidecar_ini_path));
    twitch_override_auto_observe_sidecar_a(slot->sidecar_ini_path);
    slot->twitch_session = create_twitch_session_a(slot->sidecar_ini_path,
                                                    &slot->twitch_settings,
                                                    &slot->twitch_logged_state);
    get_file_write_time_a(slot->sidecar_ini_path, &slot->sidecar_ini_write_time);
    slot->last_config_check_tick = now;
    slot->config_generation = config_generation;
    if (slot->audio_enabled && is_recently_retired_sidecar_a(slot->sidecar_path, now) &&
        d3d8_has_active_same_target_audio(slot)) {
        debug_line("D3D8Texture suppressed retired duplicate sidecar=\"%s\"", slot->sidecar_path);
        clear_d3d8_texture_slot(slot);
        clear_pending_gl_texture_probe();
        return;
    }
    if (slot->audio_enabled) stop_older_d3d8_sidecar_audio(slot, now);
    debug_line("D3D8Texture candidate tex=%p source=%ux%u upload=%dx%d proxy=%d levels=%u actual_levels=%u usage=0x%08lx format=%d pool=%d write=%d decode=%d interval_ms=%lu mip_levels=%d video_filtering=%s anisotropy=%d sidecar=\"%s\"",
             texture, width, height, slot->width, slot->height, slot->video_texture ? 1 : 0,
             levels, slot->level_count, (unsigned long)slot->usage, (int)format, (int)slot->pool,
             d3d8_video_write_enabled, d3d8_video_decode_enabled,
             (unsigned long)slot->update_interval_ms, slot->d3d8_mip_levels,
             video_filtering_name(slot->video_filtering), slot->anisotropy, slot->sidecar_path);
    clear_pending_gl_texture_probe();
}

static video_gl_texture_t *find_video_gl_texture_slot(unsigned int texture)
{
    int i;
    video_gl_texture_t *free_slot = NULL;
    for (i = 0; i < video_gl_active_count; i++) {
        if (video_gl_active_slots[i] && video_gl_active_slots[i]->texture == texture) {
            return video_gl_active_slots[i];
        }
    }
    for (i = 0; i < (int)(sizeof(video_gl_textures) / sizeof(video_gl_textures[0])); i++) {
        if (!video_gl_textures[i].active && !free_slot) free_slot = &video_gl_textures[i];
    }
    return free_slot ? free_slot : &video_gl_textures[0];
}

static video_gl_texture_t *find_active_video_gl_texture(unsigned int texture)
{
    int i;
    for (i = 0; i < video_gl_active_count; i++) {
        if (video_gl_active_slots[i] && video_gl_active_slots[i]->texture == texture) {
            return video_gl_active_slots[i];
        }
    }
    return NULL;
}

static void clear_video_gl_texture_slot(video_gl_texture_t *slot)
{
    unsigned int video_texture;
    if (!slot) return;
    twitch_override_auto_release_sidecar_a(slot->sidecar_ini_path);
    if (current_gl_video_slot == slot) current_gl_video_slot = NULL;
    video_texture = slot->video_texture;
    if (slot->active) unregister_active_gl_slot(slot);
    if (slot->audio_graph) {
        audio_graph_release(slot->audio_graph);
        slot->audio_graph = NULL;
    }
    if (slot->engine_audio) {
        engine_audio_player_release(slot->engine_audio);
        slot->engine_audio = NULL;
    }
    if (slot->decoder) {
        video_decoder_release(slot->decoder);
        slot->decoder = NULL;
    }
    if (slot->twitch_open_task) {
        twitch_decoder_open_task_release(slot->twitch_open_task);
        slot->twitch_open_task = NULL;
    }
    if (slot->twitch_session) {
        webm_twitch_session_destroy(slot->twitch_session);
        slot->twitch_session = NULL;
    }
    if (slot->twitch_chat) {
        webm_twitch_chat_release(slot->twitch_chat);
        slot->twitch_chat = NULL;
    }
    if (slot->pixels) {
        free(slot->pixels);
        slot->pixels = NULL;
    }
    memset(slot, 0, sizeof(*slot));
    if (video_texture && real_glDeleteTextures) {
        real_glDeleteTextures(1, &video_texture);
    }
}

static void clear_all_video_gl_texture_slots(void)
{
    int i;
    for (i = 0; i < (int)(sizeof(video_gl_textures) / sizeof(video_gl_textures[0])); i++) {
        clear_video_gl_texture_slot(&video_gl_textures[i]);
    }
}

static void clear_all_video_texture_slots(void)
{
    clear_all_d3d8_texture_slots();
    clear_all_video_gl_texture_slots();
    clear_pending_gl_texture_probe();
}

static void resolve_gl_runtime_functions(void)
{
    HMODULE gl;
    GetProcAddress_t get_proc;
    if (real_glBindTexture && real_glGenTextures && real_glDeleteTextures &&
        real_glTexImage2D && real_glGetError && real_glGetIntegerv &&
        real_glMatrixMode && real_glPushMatrix && real_glPopMatrix &&
        real_glLoadMatrixf) return;
    gl = GetModuleHandleA("OPENGL32.dll");
    if (!gl) return;
    get_proc = real_GetProcAddress ? real_GetProcAddress : (GetProcAddress_t)GetProcAddress;
    if (!get_proc) return;
    if (!real_glBindTexture) {
        real_glBindTexture = (glBindTexture_t)get_proc(gl, "glBindTexture");
    }
    if (!real_glGenTextures) {
        real_glGenTextures = (glGenTextures_t)get_proc(gl, "glGenTextures");
    }
    if (!real_glDeleteTextures) {
        real_glDeleteTextures = (glDeleteTextures_t)get_proc(gl, "glDeleteTextures");
    }
    if (!real_glTexImage2D) {
        real_glTexImage2D = (glTexImage2D_t)get_proc(gl, "glTexImage2D");
    }
    if (!real_glGetError) {
        real_glGetError = (glGetError_t)get_proc(gl, "glGetError");
    }
    if (!real_glGetIntegerv) {
        real_glGetIntegerv = (glGetIntegerv_t)get_proc(gl, "glGetIntegerv");
    }
    if (!real_glMatrixMode) {
        real_glMatrixMode = (glMatrixMode_t)get_proc(gl, "glMatrixMode");
    }
    if (!real_glPushMatrix) {
        real_glPushMatrix = (glPushMatrix_t)get_proc(gl, "glPushMatrix");
    }
    if (!real_glPopMatrix) {
        real_glPopMatrix = (glPopMatrix_t)get_proc(gl, "glPopMatrix");
    }
    if (!real_glLoadMatrixf) {
        real_glLoadMatrixf = (glLoadMatrixf_t)get_proc(gl, "glLoadMatrixf");
    }
}

static video_gl_texture_t *remember_video_gl_texture(GLenum target, GLint level, GLint internalformat,
                                                     GLsizei width, GLsizei height,
                                                     GLenum format, GLenum type, const void *pixels)
{
    int i;
    int bpp;
    size_t bytes;
    video_gl_texture_t *slot;
    int uv_mode_base = WEBM_UV_MODE_OFF;
    int uv_mode_twitch = WEBM_UV_MODE_OFF;
    int uv_rect_valid = 0;
    float uv_min_u = 0.0f, uv_min_v = 0.0f, uv_max_u = 1.0f, uv_max_v = 1.0f;
    int upload_width = width;
    int upload_height = height;
    int oversized;
    if (!target_texture_probe_path[0]) return NULL;
    if (target != 0x0DE1 || level != 0 || !pixels || !current_gl_texture_2d) return NULL;
    if (width <= 0 || height <= 0) return NULL;
    if (target_texture_probe_width > 0 && target_texture_probe_height > 0 &&
        (width != target_texture_probe_width || height != target_texture_probe_height)) {
        return NULL;
    }
    bpp = gl_bytes_per_pixel(format, type);
    if (!bpp) {
        log_line("VideoTexture skip tex=%u unsupported format=0x%04x type=0x%04x path=\"%s\"",
                 current_gl_texture_2d, format, type, target_texture_probe_path);
        return NULL;
    }
    load_sidecar_uv_settings_a(target_texture_sidecar_path,
                               &uv_mode_base, &uv_mode_twitch, &uv_rect_valid,
                               &uv_min_u, &uv_min_v, &uv_max_u, &uv_max_v);
    oversized = width > target_texture_max_width || height > target_texture_max_height;
    if (oversized &&
        (!uv_rect_valid ||
         (uv_mode_base != WEBM_UV_MODE_FULL_TEXTURE &&
          uv_mode_twitch != WEBM_UV_MODE_FULL_TEXTURE) ||
         !video_uv_proxy_dimensions(width, height,
                                    target_texture_max_width, target_texture_max_height,
                                    uv_min_u, uv_min_v, uv_max_u, uv_max_v,
                                    &upload_width, &upload_height))) {
        log_line("VideoTexture skip oversized tex=%u size=%dx%d sidecar=\"%s\"",
                 current_gl_texture_2d, width, height, target_texture_sidecar_path);
        return NULL;
    }
    bytes = (size_t)upload_width * (size_t)upload_height * (size_t)bpp;
    if (!bytes || bytes > (size_t)(32 * 1024 * 1024)) return NULL;
    slot = find_video_gl_texture_slot(current_gl_texture_2d);
    if (!slot) return NULL;
    clear_video_gl_texture_slot(slot);
    slot->pixels = (BYTE*)malloc(bytes);
    if (!slot->pixels) {
        log_line("VideoTexture alloc failed bytes=%lu path=\"%s\"", (unsigned long)bytes, target_texture_probe_path);
        return NULL;
    }
    slot->pixels_size = bytes;
    slot->active = 1;
    register_active_gl_slot(slot);
    slot->texture = current_gl_texture_2d;
    current_gl_video_slot = slot;
    slot->width = upload_width;
    slot->height = upload_height;
    slot->format = format;
    slot->type = type;
    slot->last_update_tick = 0;
    slot->update_interval_ms = target_texture_interval_ms ? target_texture_interval_ms : texture_video_interval_ms;
    slot->first_seen_tick = GetTickCount();
    slot->frame = 0;
    slot->uploaded_frame_index = -1;
    slot->decoder_attempted = 0;
    slot->atlas_region = oversized ? 0 :
        (target_texture_probe_width >= 2048 && target_texture_probe_height >= 2048);
    slot->wide_region = oversized ? 1 :
        (slot->atlas_region && upload_width >= 512 && upload_height >= 256 &&
         upload_width >= (upload_height * 3) / 2 && upload_width <= upload_height * 3);
    slot->video_filtering = target_texture_video_filtering;
    slot->anisotropy = target_texture_anisotropy;
    lstrcpynA(slot->image_path, target_texture_probe_path, sizeof(slot->image_path));
    lstrcpynA(slot->sidecar_path, target_texture_sidecar_path, sizeof(slot->sidecar_path));
    slot->uv_mode_base = uv_mode_base;
    slot->uv_mode_twitch = uv_mode_twitch;
    slot->uv_rect_valid = uv_rect_valid;
    slot->uv_min_u = uv_min_u;
    slot->uv_min_v = uv_min_v;
    slot->uv_max_u = uv_max_u;
    slot->uv_max_v = uv_max_v;
    if (oversized) {
        unsigned int proxy = 0;
        GLenum error = 0;
        resolve_gl_runtime_functions();
        if (!real_glGenTextures || !real_glBindTexture || !real_glTexImage2D) {
            log_line("VideoTexture proxy unavailable source=%dx%d sidecar=\"%s\"",
                     width, height, target_texture_sidecar_path);
            clear_video_gl_texture_slot(slot);
            return NULL;
        }
        if (real_glGetError) while (real_glGetError() != 0) {
        }
        real_glGenTextures(1, &proxy);
        if (proxy) {
            real_glBindTexture(0x0DE1, proxy);
            real_glTexImage2D(0x0DE1, 0, internalformat, upload_width, upload_height,
                              0, format, type, NULL);
            if (real_glGetError) error = real_glGetError();
            real_glBindTexture(0x0DE1, current_gl_texture_2d);
        }
        if (!proxy || error) {
            if (proxy && real_glDeleteTextures) real_glDeleteTextures(1, &proxy);
            log_line("VideoTexture proxy creation failed source=%dx%d proxy=%dx%d error=0x%04x sidecar=\"%s\"",
                     width, height, upload_width, upload_height, (unsigned int)error,
                     target_texture_sidecar_path);
            clear_video_gl_texture_slot(slot);
            return NULL;
        }
        slot->video_texture = proxy;
        log_line("VideoTexture runtime proxy source=%dx%d proxy=%dx%d sidecar=\"%s\"",
                 width, height, upload_width, upload_height, target_texture_sidecar_path);
    }
    load_sidecar_audio_settings_a(slot->sidecar_path, &slot->audio_enabled, &slot->audio_engine, &slot->audio_lead_ms, &slot->audio_volume,
                                  &slot->audio_3d, &slot->audio_3d_min_distance, &slot->audio_3d_max_distance,
                                  &slot->audio_3d_rolloff, slot->audio_node, sizeof(slot->audio_node),
                                  slot->audio_effect, sizeof(slot->audio_effect));
    load_sidecar_game_audio_mutes_a(slot->sidecar_path, &slot->game_audio_mutes);
    slot->audio_ready_tick = GetTickCount() + 3000;
    slot->playback_start_tick = 0;
    sidecar_ini_path_a(slot->sidecar_path, slot->sidecar_ini_path, sizeof(slot->sidecar_ini_path));
    twitch_override_auto_observe_sidecar_a(slot->sidecar_ini_path);
    slot->twitch_session = create_twitch_session_a(slot->sidecar_ini_path,
                                                    &slot->twitch_settings,
                                                    &slot->twitch_logged_state);
    get_file_write_time_a(slot->sidecar_ini_path, &slot->sidecar_ini_write_time);
    slot->last_config_check_tick = GetTickCount();
    slot->config_generation = config_generation;
    if (slot->audio_enabled && is_recently_retired_sidecar_a(slot->sidecar_path, GetTickCount()) &&
        gl_has_active_same_target_audio(slot)) {
        debug_line("VideoTexture suppressed retired duplicate sidecar=\"%s\"", slot->sidecar_path);
        clear_video_gl_texture_slot(slot);
        clear_pending_gl_texture_probe();
        return NULL;
    }
    if (slot->audio_enabled) stop_older_gl_sidecar_audio(slot, GetTickCount());
    for (i = 0; i < (int)(sizeof(video_gl_textures) / sizeof(video_gl_textures[0])); i++) {
        video_gl_texture_t *other = &video_gl_textures[i];
        if (other == slot || !other->active || other->texture == slot->texture) continue;
        if ((other->sidecar_path[0] && _stricmp(other->sidecar_path, slot->sidecar_path) == 0) ||
            (other->image_path[0] && _stricmp(other->image_path, slot->image_path) == 0)) {
            clear_video_gl_texture_slot(other);
        }
    }
    target_texture_probe_path[0] = 0;
    target_texture_sidecar_path[0] = 0;
    target_texture_probe_tick = 0;
    target_texture_probe_width = 0;
    target_texture_probe_height = 0;
    target_texture_interval_ms = texture_video_interval_ms;
    target_texture_max_width = texture_max_width;
    target_texture_max_height = texture_max_height;
    target_texture_d3d8_mip_levels = d3d8_mip_levels;
    target_texture_video_filtering = video_filtering;
    target_texture_anisotropy = video_anisotropy;
    return slot;
}

static void fill_video_test_pattern(video_gl_texture_t *vt)
{
    int x, y, bpp;
    BYTE *p;
    int phase;
    int max_x;
    int max_y;
    int bottom_y;
    if (!vt || !vt->pixels) return;
    bpp = gl_bytes_per_pixel(vt->format, vt->type);
    if (!bpp) return;
    phase = vt->frame * 9;
    max_x = vt->width;
    max_y = vt->height;
    if (vt->wide_region) {
        max_x = vt->width;
        max_y = vt->height;
    } else if (vt->atlas_region) {
        max_x = vt->width / 8;
        max_y = (vt->height * 72) / 1024;
        if (max_x < 32) max_x = vt->width;
        if (max_y < 32) max_y = vt->height;
    }
    bottom_y = vt->height - max_y;
    if (bottom_y < 0) bottom_y = 0;
    p = vt->pixels;
    for (y = 0; y < vt->height; y++) {
        for (x = 0; x < vt->width; x++) {
            int band = ((x + y + phase * 4) / 24) & 3;
            int flash = (vt->frame / 10) & 3;
            int in_region = !vt->atlas_region || vt->wide_region ||
                            (x < max_x && (y < max_y || y >= bottom_y));
            if (in_region) {
                BYTE r = (BYTE)(flash == 0 ? 255 : flash == 1 ? 0 : flash == 2 ? 255 : 0);
                BYTE g = (BYTE)(flash == 0 ? 0 : flash == 1 ? 255 : flash == 2 ? 255 : 80);
                BYTE b = (BYTE)(flash == 0 ? 0 : flash == 1 ? 40 : flash == 2 ? 0 : 255);
                if (band & 1) {
                    r = (BYTE)(255 - r);
                    g = (BYTE)(255 - g);
                    b = (BYTE)(255 - b);
                }
                if (vt->atlas_region && !vt->wide_region && y >= bottom_y) {
                    r = (BYTE)(flash == 0 ? 0 : flash == 1 ? 255 : flash == 2 ? 0 : 255);
                    g = (BYTE)(flash == 0 ? 160 : flash == 1 ? 0 : flash == 2 ? 255 : 255);
                    b = (BYTE)(flash == 0 ? 255 : flash == 1 ? 255 : flash == 2 ? 80 : 0);
                    if (band & 1) {
                        r = (BYTE)(255 - r);
                        g = (BYTE)(255 - g);
                        b = (BYTE)(255 - b);
                    }
                }
                p[0] = r;
                p[1] = g;
                p[2] = b;
                if (bpp == 4) p[3] = 255;
            }
            p += bpp;
        }
    }
}

static int fill_video_decoded_frame(video_gl_texture_t *vt)
{
    int x, y, bpp;
    video_decoder_t *dec;
    BYTE *dst;
    BYTE *source_frame;
    int source_width;
    int source_height;
    int source_stride;
    int frame_locked = 0;
    if (!vt || !vt->pixels || !vt->decoder) return 0;
    dec = vt->decoder;
    if (video_decoder_copy_gl_cached_frame(dec, vt->pixels, vt->pixels_size,
                                           vt->width, vt->height,
                                           vt->format, vt->type)) {
        return 2;
    }
    if (dec->async_enabled) {
        EnterCriticalSection(&dec->async_frame_lock);
        frame_locked = 1;
        source_frame = dec->async_frame;
        source_width = dec->async_width;
        source_height = dec->async_height;
        source_stride = dec->async_stride;
    } else {
        source_frame = dec->frame;
        source_width = dec->width;
        source_height = dec->height;
        source_stride = dec->stride;
    }
    if (!source_frame || source_width <= 0 || source_height <= 0 || source_stride <= 0) {
        if (frame_locked) LeaveCriticalSection(&dec->async_frame_lock);
        return 0;
    }
    bpp = gl_bytes_per_pixel(vt->format, vt->type);
    if (bpp != 3 && bpp != 4) {
        if (frame_locked) LeaveCriticalSection(&dec->async_frame_lock);
        return 0;
    }
    dst = vt->pixels;
    for (y = 0; y < vt->height; y++) {
        int sy = source_height - 1 - ((y * source_height) / vt->height);
        BYTE *src_row;
        if (sy < 0) sy = 0;
        if (sy >= source_height) sy = source_height - 1;
        src_row = source_frame + (size_t)sy * (size_t)source_stride;
        for (x = 0; x < vt->width; x++) {
            int sx = (x * source_width) / vt->width;
            BYTE *src;
            if (sx < 0) sx = 0;
            if (sx >= source_width) sx = source_width - 1;
            src = src_row + sx * 3;
            dst[0] = dec->use_ffmpeg ? src[0] : src[2];
            dst[1] = src[1];
            dst[2] = dec->use_ffmpeg ? src[2] : src[0];
            if (bpp == 4) dst[3] = 255;
            dst += bpp;
        }
    }
    if (frame_locked) LeaveCriticalSection(&dec->async_frame_lock);
    return 1;
}

static void refresh_gl_texture_settings(video_gl_texture_t *vt, DWORD now)
{
    FILETIME write_time;
    DWORD interval_ms;
    int max_width, max_height, mip_levels, filtering, anisotropy;
    int audio_enabled, audio_engine, audio_lead_ms, audio_volume;
    int audio_3d, audio_3d_min_distance, audio_3d_max_distance, audio_3d_rolloff;
    char audio_effect[32];
    char audio_node[MAX_PATH * 4];
    int config_changed;
    if (!vt || !vt->active || !vt->sidecar_path[0] || !vt->sidecar_ini_path[0]) return;
    if (vt->last_config_check_tick && (now - vt->last_config_check_tick) < 1000) return;
    vt->last_config_check_tick = now;
    get_file_write_time_a(vt->sidecar_ini_path, &write_time);
    config_changed = vt->config_generation != config_generation;
    if (!config_changed && !filetime_differs(&write_time, &vt->sidecar_ini_write_time)) return;
    load_sidecar_settings_values_a(vt->sidecar_path, &interval_ms, &max_width, &max_height,
                                   &mip_levels, &filtering, &anisotropy, &write_time, 1);
    vt->update_interval_ms = interval_ms ? interval_ms : texture_video_interval_ms;
    vt->video_filtering = filtering;
    vt->anisotropy = anisotropy;
    load_sidecar_uv_settings_a(vt->sidecar_path,
                               &vt->uv_mode_base, &vt->uv_mode_twitch, &vt->uv_rect_valid,
                               &vt->uv_min_u, &vt->uv_min_v, &vt->uv_max_u, &vt->uv_max_v);
    load_sidecar_audio_settings_a(vt->sidecar_path, &audio_enabled, &audio_engine, &audio_lead_ms, &audio_volume,
                                  &audio_3d, &audio_3d_min_distance, &audio_3d_max_distance,
                                  &audio_3d_rolloff, audio_node, sizeof(audio_node),
                                  audio_effect, sizeof(audio_effect));
    load_sidecar_game_audio_mutes_a(vt->sidecar_path, &vt->game_audio_mutes);
    if (vt->audio_graph) {
        audio_graph_release(vt->audio_graph);
        vt->audio_graph = NULL;
    }
    if (vt->engine_audio) {
        engine_audio_player_release(vt->engine_audio);
        vt->engine_audio = NULL;
    }
    if (vt->decoder) {
        video_decoder_retire_async(vt->decoder);
        vt->decoder = NULL;
    }
    if (vt->twitch_open_task) {
        twitch_decoder_open_task_release(vt->twitch_open_task);
        vt->twitch_open_task = NULL;
    }
    if (vt->twitch_chat) {
        webm_twitch_chat_release(vt->twitch_chat);
        vt->twitch_chat = NULL;
        vt->twitch_chat_channel[0] = 0;
    }
    if (vt->twitch_session) {
        webm_twitch_session_destroy(vt->twitch_session);
        vt->twitch_session = NULL;
    }
    vt->decoder_attempted = 0;
    memset(&vt->source_failure, 0, sizeof(vt->source_failure));
    vt->frame = 0;
    vt->last_update_tick = 0;
    vt->audio_enabled = audio_enabled;
    vt->audio_engine = audio_engine;
    vt->engine_audio_runtime_failed = 0;
    vt->audio_lead_ms = audio_lead_ms;
    vt->audio_volume = audio_volume;
    vt->audio_3d = audio_3d;
    vt->audio_3d_min_distance = audio_3d_min_distance;
    vt->audio_3d_max_distance = audio_3d_max_distance;
    vt->audio_3d_rolloff = audio_3d_rolloff;
    lstrcpynA(vt->audio_effect, audio_effect, sizeof(vt->audio_effect));
    lstrcpynA(vt->audio_node, audio_node, sizeof(vt->audio_node));
    vt->sync_hold_logged = 0;
    vt->sync_hold_until_tick = 0;
    vt->audio_ready_tick = now;
    vt->playback_start_tick = 0;
    vt->twitch_active = 0;
    vt->twitch_fallback_active = 0;
    vt->twitch_session = create_twitch_session_a(vt->sidecar_ini_path,
                                                  &vt->twitch_settings,
                                                  &vt->twitch_logged_state);
    vt->config_generation = config_generation;
    vt->sidecar_ini_write_time = write_time;
    debug_line("VideoTexture config refresh/restart tex=%u interval_ms=%lu max_texture=%dx%d video_filtering=%s anisotropy=%d audio=%d lead_ms=%d volume=%d audio_3d=%d rolloff=%d effect=\"%s\" node=\"%s\" sidecar=\"%s\"",
             vt->texture, (unsigned long)vt->update_interval_ms, max_width, max_height,
             video_filtering_name(vt->video_filtering), vt->anisotropy,
             vt->audio_enabled, vt->audio_lead_ms, vt->audio_volume,
             vt->audio_3d, vt->audio_3d_rolloff, vt->audio_effect, vt->audio_node, vt->sidecar_path);
}

static void refresh_d3d8_texture_settings(video_d3d8_texture_t *vt, DWORD now)
{
    FILETIME write_time;
    DWORD interval_ms;
    int max_width, max_height, mip_levels, filtering, anisotropy;
    int audio_enabled, audio_engine, audio_lead_ms, audio_volume;
    int audio_3d, audio_3d_min_distance, audio_3d_max_distance, audio_3d_rolloff;
    char audio_effect[32];
    char audio_node[MAX_PATH * 4];
    int config_changed;
    if (!vt || !vt->active || !vt->sidecar_path[0] || !vt->sidecar_ini_path[0]) return;
    if (vt->last_config_check_tick && (now - vt->last_config_check_tick) < 1000) return;
    vt->last_config_check_tick = now;
    get_file_write_time_a(vt->sidecar_ini_path, &write_time);
    config_changed = vt->config_generation != config_generation;
    if (!config_changed && !filetime_differs(&write_time, &vt->sidecar_ini_write_time)) return;
    load_sidecar_settings_values_a(vt->sidecar_path, &interval_ms, &max_width, &max_height,
                                   &mip_levels, &filtering, &anisotropy, &write_time, 1);
    vt->update_interval_ms = interval_ms ? interval_ms : texture_video_interval_ms;
    vt->d3d8_mip_levels = mip_levels;
    vt->video_filtering = filtering;
    vt->anisotropy = anisotropy;
    load_sidecar_uv_settings_a(vt->sidecar_path,
                               &vt->uv_mode_base, &vt->uv_mode_twitch, &vt->uv_rect_valid,
                               &vt->uv_min_u, &vt->uv_min_v, &vt->uv_max_u, &vt->uv_max_v);
    load_sidecar_audio_settings_a(vt->sidecar_path, &audio_enabled, &audio_engine, &audio_lead_ms, &audio_volume,
                                  &audio_3d, &audio_3d_min_distance, &audio_3d_max_distance,
                                  &audio_3d_rolloff, audio_node, sizeof(audio_node),
                                  audio_effect, sizeof(audio_effect));
    load_sidecar_game_audio_mutes_a(vt->sidecar_path, &vt->game_audio_mutes);
    if (vt->audio_graph) {
        audio_graph_release(vt->audio_graph);
        vt->audio_graph = NULL;
    }
    if (vt->engine_audio) {
        engine_audio_player_release(vt->engine_audio);
        vt->engine_audio = NULL;
    }
    if (vt->decoder) {
        video_decoder_retire_async(vt->decoder);
        vt->decoder = NULL;
    }
    if (vt->twitch_open_task) {
        twitch_decoder_open_task_release(vt->twitch_open_task);
        vt->twitch_open_task = NULL;
    }
    if (vt->twitch_chat) {
        webm_twitch_chat_release(vt->twitch_chat);
        vt->twitch_chat = NULL;
        vt->twitch_chat_channel[0] = 0;
    }
    free(vt->twitch_chat_base_pixels);
    vt->twitch_chat_base_pixels = NULL;
    vt->twitch_chat_base_size = 0;
    vt->twitch_chat_base_pitch = 0;
    vt->twitch_chat_base_bpp = 0;
    if (vt->twitch_session) {
        webm_twitch_session_destroy(vt->twitch_session);
        vt->twitch_session = NULL;
    }
    vt->decoder_attempted = 0;
    memset(&vt->source_failure, 0, sizeof(vt->source_failure));
    vt->frame = 0;
    vt->last_update_tick = 0;
    vt->audio_enabled = audio_enabled;
    vt->audio_engine = audio_engine;
    vt->engine_audio_runtime_failed = 0;
    vt->audio_lead_ms = audio_lead_ms;
    vt->audio_volume = audio_volume;
    vt->audio_3d = audio_3d;
    vt->audio_3d_min_distance = audio_3d_min_distance;
    vt->audio_3d_max_distance = audio_3d_max_distance;
    vt->audio_3d_rolloff = audio_3d_rolloff;
    lstrcpynA(vt->audio_effect, audio_effect, sizeof(vt->audio_effect));
    lstrcpynA(vt->audio_node, audio_node, sizeof(vt->audio_node));
    vt->sync_hold_logged = 0;
    vt->sync_hold_until_tick = 0;
    vt->audio_ready_tick = now;
    vt->playback_start_tick = 0;
    vt->twitch_active = 0;
    vt->twitch_fallback_active = 0;
    vt->twitch_session = create_twitch_session_a(vt->sidecar_ini_path,
                                                  &vt->twitch_settings,
                                                  &vt->twitch_logged_state);
    vt->config_generation = config_generation;
    vt->sidecar_ini_write_time = write_time;
    debug_line("D3D8Texture config refresh/restart tex=%p interval_ms=%lu max_texture=%dx%d mip_levels=%d video_filtering=%s anisotropy=%d audio=%d lead_ms=%d volume=%d audio_3d=%d rolloff=%d effect=\"%s\" node=\"%s\" sidecar=\"%s\"",
             vt->texture, (unsigned long)vt->update_interval_ms, max_width, max_height,
             vt->d3d8_mip_levels, video_filtering_name(vt->video_filtering), vt->anisotropy,
             vt->audio_enabled, vt->audio_lead_ms, vt->audio_volume,
             vt->audio_3d, vt->audio_3d_rolloff, vt->audio_effect, vt->audio_node, vt->sidecar_path);
}

static void fill_d3d8_locked_frame(video_d3d8_texture_t *vt, D3DLOCKED_RECT *lr, int decoded,
                                   int width, int height)
{
    int x, y;
    int bpp;
    int source_width = 0;
    int source_height = 0;
    int source_stride = 0;
    int source_x_step = 0;
    int source_x_remainder = 0;
    int frame_locked = 0;
    BYTE *source_frame = NULL;
    BYTE *dst_base;
    if (!vt || !lr || !lr->pBits) return;
    bpp = (vt->format == D3DFMT_R5G6B5) ? 2 : ((vt->format == D3DFMT_R8G8B8) ? 3 : 4);
    if (width <= 0 || height <= 0) return;
    if (lr->Pitch <= 0 || lr->Pitch < width * bpp) return;
    if (decoded && vt->decoder) {
        if (vt->decoder->async_enabled) {
            EnterCriticalSection(&vt->decoder->async_frame_lock);
            frame_locked = 1;
            source_frame = vt->decoder->async_frame;
            source_width = vt->decoder->async_width;
            source_height = vt->decoder->async_height;
            source_stride = vt->decoder->async_stride;
        } else {
            source_frame = vt->decoder->frame;
            source_width = vt->decoder->width;
            source_height = vt->decoder->height;
            source_stride = vt->decoder->stride;
        }
    }
    if (source_frame && source_width > 0 && source_height > 0 && source_stride > 0) {
        source_x_step = source_width / width;
        source_x_remainder = source_width % width;
    } else {
        source_frame = NULL;
        source_width = 0;
    }
    dst_base = (BYTE*)lr->pBits;
    if (source_frame && source_width == width && source_height == height && bpp == 4) {
        for (y = 0; y < height; y++) {
            const BYTE *src = source_frame +
                (size_t)(height - 1 - y) * (size_t)source_stride;
            DWORD *dst = (DWORD*)(dst_base + (size_t)y * (size_t)lr->Pitch);
            for (x = 0; x < width; x++) {
                dst[x] = 0xff000000u |
                         ((DWORD)src[0] << 16) |
                         ((DWORD)src[1] << 8) |
                         (DWORD)src[2];
                src += 3;
            }
        }
        if (frame_locked) LeaveCriticalSection(&vt->decoder->async_frame_lock);
        return;
    }
    for (y = 0; y < height; y++) {
        BYTE *dst = dst_base + (size_t)y * (size_t)lr->Pitch;
        int sy = y;
        int sx = 0;
        int sx_error = 0;
        BYTE *src_row = NULL;
        if (source_frame) {
            sy = ((height - 1 - y) * source_height) / height;
            if (sy < 0) sy = 0;
            if (sy >= source_height) sy = source_height - 1;
            src_row = source_frame + (size_t)sy * (size_t)source_stride;
        }
        for (x = 0; x < width; x++) {
            BYTE r, g, b;
            if (src_row && source_width > 0) {
                BYTE *src;
                if (sx < 0) sx = 0;
                if (sx >= source_width) sx = source_width - 1;
                src = src_row + sx * 3;
                r = src[0];
                g = src[1];
                b = src[2];
                sx += source_x_step;
                sx_error += source_x_remainder;
                if (sx_error >= width) {
                    sx++;
                    sx_error -= width;
                }
            } else {
                int band = ((x + y + vt->frame * 24) / 48) & 1;
                r = band ? 255 : 0;
                g = band ? 80 : 160;
                b = band ? 0 : 255;
            }
            if (vt->format == D3DFMT_R5G6B5) {
                WORD packed = (WORD)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
                dst[0] = (BYTE)(packed & 0xff);
                dst[1] = (BYTE)(packed >> 8);
            } else {
                dst[0] = b;
                dst[1] = g;
                dst[2] = r;
                if (bpp == 4) dst[3] = 255;
            }
            dst += bpp;
        }
    }
    if (frame_locked) LeaveCriticalSection(&vt->decoder->async_frame_lock);
}

static void restart_audio_for_video_loop_d3d8(video_d3d8_texture_t *vt, DWORD now);
static void restart_audio_for_video_loop_gl(video_gl_texture_t *vt, DWORD now);

static int sidecar_audio_same_target_a(const char *a_path, const char *a_node,
                                       const char *b_path, const char *b_node)
{
    char a_base[MAX_PATH];
    char b_base[MAX_PATH];
    if (!a_path || !a_path[0] || !b_path || !b_path[0]) return 0;
    if (_stricmp(a_path, b_path) == 0) return 1;
    if (a_node && a_node[0] && b_node && b_node[0] && _stricmp(a_node, b_node) == 0) return 1;
    if (path_is_addon_room_a(a_path) && path_is_addon_room_a(b_path)) {
        basename_no_ext_a(a_path, a_base, sizeof(a_base));
        basename_no_ext_a(b_path, b_base, sizeof(b_base));
        if (a_base[0] && b_base[0] && _stricmp(a_base, b_base) == 0) return 1;
    }
    return 0;
}

static void remember_retired_sidecar_a(const char *path, DWORD now)
{
    int i, slot = 0;
    DWORD oldest = retired_sidecar_until[0];
    if (!path || !path[0]) return;
    for (i = 0; i < (int)(sizeof(retired_sidecar_paths) / sizeof(retired_sidecar_paths[0])); i++) {
        if (retired_sidecar_paths[i][0] && _stricmp(retired_sidecar_paths[i], path) == 0) {
            slot = i;
            break;
        }
        if (!retired_sidecar_paths[i][0]) {
            slot = i;
            break;
        }
        if (retired_sidecar_until[i] < oldest) {
            oldest = retired_sidecar_until[i];
            slot = i;
        }
    }
    lstrcpynA(retired_sidecar_paths[slot], path, sizeof(retired_sidecar_paths[slot]));
    retired_sidecar_until[slot] = now + 15000u;
}

static int is_recently_retired_sidecar_a(const char *path, DWORD now)
{
    int i;
    if (!path || !path[0]) return 0;
    for (i = 0; i < (int)(sizeof(retired_sidecar_paths) / sizeof(retired_sidecar_paths[0])); i++) {
        if (!retired_sidecar_paths[i][0]) continue;
        if ((long)(retired_sidecar_until[i] - now) <= 0) {
            retired_sidecar_paths[i][0] = 0;
            retired_sidecar_until[i] = 0;
            continue;
        }
        if (_stricmp(retired_sidecar_paths[i], path) == 0) return 1;
    }
    return 0;
}

static int d3d8_has_active_same_target_audio(video_d3d8_texture_t *owner)
{
    int i;
    if (!owner || !owner->sidecar_path[0]) return 0;
    for (i = 0; i < (int)(sizeof(video_d3d8_textures) / sizeof(video_d3d8_textures[0])); i++) {
        video_d3d8_texture_t *other = &video_d3d8_textures[i];
        if (other == owner || !other->active || (!other->audio_graph && !other->engine_audio) || !other->sidecar_path[0]) continue;
        if (sidecar_audio_same_target_a(other->sidecar_path, other->audio_node,
                                        owner->sidecar_path, owner->audio_node)) return 1;
    }
    return 0;
}

static int gl_has_active_same_target_audio(video_gl_texture_t *owner)
{
    int i;
    if (!owner || !owner->sidecar_path[0]) return 0;
    for (i = 0; i < (int)(sizeof(video_gl_textures) / sizeof(video_gl_textures[0])); i++) {
        video_gl_texture_t *other = &video_gl_textures[i];
        if (other == owner || !other->active || (!other->audio_graph && !other->engine_audio) || !other->sidecar_path[0]) continue;
        if (sidecar_audio_same_target_a(other->sidecar_path, other->audio_node,
                                        owner->sidecar_path, owner->audio_node)) return 1;
    }
    return 0;
}

static int is_d3d8_sidecar_audio_owner(video_d3d8_texture_t *vt, DWORD now)
{
    int i;
    if (!vt || !vt->active || !vt->sidecar_path[0]) return 0;
    if (!vt->last_bound_tick || (now - vt->last_bound_tick) > 1000) return 0;
    for (i = 0; i < (int)(sizeof(video_d3d8_textures) / sizeof(video_d3d8_textures[0])); i++) {
        video_d3d8_texture_t *other = &video_d3d8_textures[i];
        if (other == vt || !other->active || (!other->audio_graph && !other->engine_audio) || !other->sidecar_path[0]) continue;
        if (_stricmp(other->sidecar_path, vt->sidecar_path) != 0) continue;
        if (other->last_bound_tick && (now - other->last_bound_tick) <= 1000) return 0;
    }
    for (i = 0; i < (int)(sizeof(video_d3d8_textures) / sizeof(video_d3d8_textures[0])); i++) {
        video_d3d8_texture_t *other = &video_d3d8_textures[i];
        if (other == vt || !other->active || !other->sidecar_path[0]) continue;
        if (_stricmp(other->sidecar_path, vt->sidecar_path) != 0) continue;
        if (!other->last_bound_tick || (now - other->last_bound_tick) > 1000) continue;
        if (other->last_bound_tick > vt->last_bound_tick) return 0;
        if (other->last_bound_tick == vt->last_bound_tick && other->bind_log_count > vt->bind_log_count) return 0;
    }
    return 1;
}

static int is_gl_sidecar_audio_owner(video_gl_texture_t *vt, DWORD now)
{
    int i;
    if (!vt || !vt->active || !vt->sidecar_path[0]) return 0;
    if (!vt->last_bound_tick || (now - vt->last_bound_tick) > 1000) return 0;
    for (i = 0; i < (int)(sizeof(video_gl_textures) / sizeof(video_gl_textures[0])); i++) {
        video_gl_texture_t *other = &video_gl_textures[i];
        if (other == vt || !other->active || (!other->audio_graph && !other->engine_audio) || !other->sidecar_path[0]) continue;
        if (_stricmp(other->sidecar_path, vt->sidecar_path) != 0) continue;
        if (other->last_bound_tick && (now - other->last_bound_tick) <= 1000) return 0;
    }
    for (i = 0; i < (int)(sizeof(video_gl_textures) / sizeof(video_gl_textures[0])); i++) {
        video_gl_texture_t *other = &video_gl_textures[i];
        if (other == vt || !other->active || !other->sidecar_path[0]) continue;
        if (_stricmp(other->sidecar_path, vt->sidecar_path) != 0) continue;
        if (!other->last_bound_tick || (now - other->last_bound_tick) > 1000) continue;
        if (other->last_bound_tick > vt->last_bound_tick) return 0;
        if (other->last_bound_tick == vt->last_bound_tick && other->bind_log_count > vt->bind_log_count) return 0;
    }
    return 1;
}

static void stop_older_d3d8_sidecar_audio(video_d3d8_texture_t *owner, DWORD now)
{
    int i;
    if (!owner || !owner->sidecar_path[0]) return;
    for (i = 0; i < (int)(sizeof(video_d3d8_textures) / sizeof(video_d3d8_textures[0])); i++) {
        video_d3d8_texture_t *other = &video_d3d8_textures[i];
        if (other == owner || !other->active || (!other->audio_graph && !other->engine_audio) || !other->sidecar_path[0]) continue;
        if (!sidecar_audio_same_target_a(other->sidecar_path, other->audio_node,
                                         owner->sidecar_path, owner->audio_node)) continue;
        debug_line("AudioOpenAL retiring previous sidecar target old=\"%s\" new=\"%s\"",
                 other->sidecar_path, owner->sidecar_path);
        remember_retired_sidecar_a(other->sidecar_path, now);
        clear_d3d8_texture_slot(other);
    }
}

static void stop_older_gl_sidecar_audio(video_gl_texture_t *owner, DWORD now)
{
    int i;
    if (!owner || !owner->sidecar_path[0]) return;
    for (i = 0; i < (int)(sizeof(video_gl_textures) / sizeof(video_gl_textures[0])); i++) {
        video_gl_texture_t *other = &video_gl_textures[i];
        if (other == owner || !other->active || (!other->audio_graph && !other->engine_audio) || !other->sidecar_path[0]) continue;
        if (!sidecar_audio_same_target_a(other->sidecar_path, other->audio_node,
                                         owner->sidecar_path, owner->audio_node)) continue;
        debug_line("AudioOpenAL retiring previous sidecar target old=\"%s\" new=\"%s\"",
                 other->sidecar_path, owner->sidecar_path);
        remember_retired_sidecar_a(other->sidecar_path, now);
        clear_video_gl_texture_slot(other);
    }
}

static void update_d3d8_video_textures(void)
{
    int i;
    DWORD now;
    if (!d3d8_video_write_enabled) return;
    if (video_d3d8_active_count <= 0) return;
    now = GetTickCount();
    refresh_global_config(now);
    d3d8_last_global_update_tick = now;
    for (i = 0; i < (int)(sizeof(video_d3d8_textures) / sizeof(video_d3d8_textures[0])); i++) {
        video_d3d8_texture_t *vt = &video_d3d8_textures[i];
        UINT level, max_levels;
        int decoded = 0;
        int have_frame = 0;
        int uploaded_any = 0;
        int allow_local_webm = 1;
        long decoded_frame_index;
        IDirect3DTexture8 *upload_texture;
        if (!vt->active || !vt->texture) continue;
        upload_texture = vt->video_texture ? vt->video_texture : vt->texture;
        if (!vt->last_bound_tick || (now - vt->last_bound_tick) > 1000) {
            if (vt->audio_graph) {
                audio_graph_release(vt->audio_graph);
                vt->audio_graph = NULL;
                vt->audio_ready_tick = now + 3000;
                vt->sync_hold_until_tick = 0;
            }
            if (vt->engine_audio) {
                engine_audio_player_release(vt->engine_audio);
                vt->engine_audio = NULL;
                vt->audio_ready_tick = now + 3000;
                vt->sync_hold_until_tick = 0;
            }
            continue;
        }
        refresh_d3d8_texture_settings(vt, now);
        if (vt->last_update_tick && (now - vt->last_update_tick) < (vt->update_interval_ms ? vt->update_interval_ms : texture_video_interval_ms)) continue;
        vt->last_update_tick = now;
        vt->frame++;
        if (vt->twitch_session) {
            allow_local_webm = update_twitch_decoder_a(vt->twitch_session, &vt->twitch_settings,
                                                       &vt->twitch_active, &vt->twitch_fallback_active,
                                                       &vt->twitch_logged_state, &vt->decoder,
                                                       &vt->decoder_attempted,
                                                       &vt->twitch_open_task,
                                                       &vt->audio_graph, &vt->engine_audio,
                                                       &vt->playback_start_tick,
                                                       vt->audio_enabled, vt->audio_volume,
                                                       vt->audio_3d,
                                                       vt->audio_3d_min_distance,
                                                       vt->audio_3d_max_distance,
                                                       vt->audio_3d_rolloff,
                                                       vt->audio_node,
                                                       vt->sidecar_path,
                                                       "directx", now);
            if (vt->twitch_active) vt->decoder_attempted = 1;
        }
        update_twitch_chat_a(vt->twitch_session, &vt->twitch_settings,
                             vt->twitch_active, &vt->twitch_chat,
                             vt->twitch_chat_channel,
                             sizeof(vt->twitch_chat_channel));
        if (!vt->twitch_chat && vt->twitch_chat_base_pixels) {
            int mip_level;
            free(vt->twitch_chat_base_pixels);
            vt->twitch_chat_base_pixels = NULL;
            vt->twitch_chat_base_size = 0;
            vt->twitch_chat_base_pitch = 0;
            vt->twitch_chat_base_bpp = 0;
            for (mip_level = 0; mip_level < WEBM_TWITCH_CHAT_MIP_LEVELS;
                 mip_level++) {
                free(vt->twitch_chat_mip_pixels[mip_level]);
                vt->twitch_chat_mip_pixels[mip_level] = NULL;
                vt->twitch_chat_mip_sizes[mip_level] = 0;
                vt->twitch_chat_mip_pitches[mip_level] = 0;
            }
        }
        if (allow_local_webm && !vt->twitch_active) {
            int source_refresh = webm_source_failure_refresh_a(
                &vt->source_failure, vt->sidecar_path, now);
            if (source_refresh == WEBM_SOURCE_REFRESH_CHANGED) {
                if (vt->decoder) {
                    video_decoder_retire_async(vt->decoder);
                    vt->decoder = NULL;
                }
                vt->decoder_attempted = 0;
                vt->playback_start_tick = 0;
            }
            if (source_refresh) {
                vt->engine_audio_runtime_failed = 0;
                vt->audio_ready_tick = now;
            }
        }
        if (d3d8_video_decode_enabled &&
            (ffmpeg_texture_decoder_enabled || directshow_texture_decoder_enabled) &&
            allow_local_webm &&
            !vt->decoder && !vt->decoder_attempted && vt->sidecar_path[0] &&
            vt->first_seen_tick && (now - vt->first_seen_tick) > (vt->audio_engine ? 3000u : 500u)) {
            vt->decoder_attempted = 1;
            if (webm_source_available_for_attempt_a(&vt->source_failure,
                                                    vt->sidecar_path, now)) {
                debug_line("D3D8VideoDecoder deferred start tex=%p backend=%s sidecar=\"%s\"",
                         vt->texture, ffmpeg_texture_decoder_enabled ? "ffmpeg" : "directshow", vt->sidecar_path);
                vt->decoder = ffmpeg_texture_decoder_enabled ? ffmpeg_decoder_create(vt->sidecar_path) :
                                                               video_decoder_create(vt->sidecar_path);
                if (vt->decoder) {
                    vt->playback_start_tick = now;
                    vt->last_update_tick = 0;
                    continue;
                }
            }
        }
        if (allow_local_webm && !vt->twitch_active &&
            vt->audio_enabled && !vt->audio_graph && !vt->engine_audio && vt->sidecar_path[0] &&
            vt->first_seen_tick && vt->frame >= 30 && vt->bind_log_count >= 120 &&
            now >= vt->audio_ready_tick) {
            if (is_d3d8_sidecar_audio_owner(vt, now)) {
                if (!webm_source_available_for_attempt_a(&vt->source_failure,
                                                         vt->sidecar_path, now)) {
                    continue;
                }
                stop_older_d3d8_sidecar_audio(vt, now);
                if (vt->audio_engine && !vt->engine_audio_runtime_failed) {
                    if (!engine_audio_runtime_available_a() && (now - vt->first_seen_tick) < 10000u) {
                        vt->audio_ready_tick = now + 500u;
                        debug_line("EngineAudio runtime waiting for device sidecar=\"%s\" parent=\"%s\"",
                                 vt->sidecar_path, vt->audio_node);
                        continue;
                    } else if (engine_audio_runtime_available_a()) {
                        vt->engine_audio = engine_audio_player_create_from_expr(vt->sidecar_path, vt->audio_node,
                                                                               vt->audio_volume,
                                                                               vt->audio_3d_min_distance,
                                                                               vt->audio_3d_max_distance);
                        if (!vt->engine_audio) {
                            vt->engine_audio_runtime_failed = 1;
                            if (!engine_audio_native_playback_enabled) {
                                log_line("EngineAudio native playback parked; using regular texture audio sidecar=\"%s\" parent=\"%s\"",
                                         vt->sidecar_path, vt->audio_node);
                            } else {
                                log_line("EngineAudio runtime failed; falling back to regular audio sidecar=\"%s\" parent=\"%s\"",
                                         vt->sidecar_path, vt->audio_node);
                            }
                        }
                    } else {
                        vt->engine_audio_runtime_failed = 1;
                        log_line("EngineAudio runtime unavailable; falling back to regular audio sidecar=\"%s\" parent=\"%s\"",
                                 vt->sidecar_path, vt->audio_node);
                    }
                }
                if (!vt->engine_audio) {
                    vt->audio_graph = audio_graph_create(vt->sidecar_path, vt->audio_lead_ms, vt->audio_volume,
                                                         vt->audio_3d,
                                                         vt->audio_3d_min_distance,
                                                         vt->audio_3d_max_distance,
                                                         vt->audio_3d_rolloff, vt->audio_node,
                                                         vt->audio_effect);
                }
                if (vt->audio_graph && vt->audio_lead_ms > 0) {
                    vt->sync_hold_until_tick = now + (DWORD)vt->audio_lead_ms;
                    vt->sync_hold_logged = 0;
                }
                if (vt->audio_graph || vt->engine_audio) {
                    if (vt->decoder) {
                        video_decoder_release(vt->decoder);
                        vt->decoder = NULL;
                    }
                    vt->decoder_attempted = 0;
                    vt->last_update_tick = 0;
                    vt->playback_start_tick = now;
                    continue;
                }
                webm_source_failure_park_a(&vt->source_failure, vt->sidecar_path,
                                           now, "audio initialization failure");
                vt->audio_ready_tick = vt->source_failure.retry_tick;
            }
        }
        if (!vt->twitch_active && vt->audio_enabled &&
            vt->sync_hold_until_tick && now < vt->sync_hold_until_tick) {
            if (!vt->sync_hold_logged) {
                vt->sync_hold_logged = 1;
                debug_line("D3D8Video sync hold tex=%p hold_ms=%d sidecar=\"%s\"",
                         vt->texture, vt->audio_lead_ms, vt->sidecar_path);
            }
            continue;
        }
        if (vt->audio_graph || vt->engine_audio) {
            LONGLONG audio_start = webm_perf_counter();
            if (vt->audio_graph) {
                if (vt->twitch_active && vt->audio_graph->twitch_streaming && vt->decoder) {
                    DWORD audio_elapsed = vt->playback_start_tick ? now - vt->playback_start_tick : 0;
                    audio_graph_update_twitch_stream(vt->audio_graph, vt->decoder, audio_elapsed);
                } else {
                    audio_graph_update_3d(vt->audio_graph, now);
                }
            }
            if (vt->engine_audio) engine_audio_player_update(vt->engine_audio, now);
            webm_perf_add(WEBM_PERF_D3D8_AUDIO, audio_start);
        }
        if ((unsigned int)(d3d8_present_serial -
                           vt->last_bound_present_serial) > 1u) {
            continue;
        }
        if (!vt->playback_start_tick) vt->playback_start_tick = now;
        if (vt->decoder && vt->decoder->async_enabled) {
            UINT cache_levels = vt->level_count ? vt->level_count : 1;
            if (cache_levels > 8) cache_levels = 8;
            if (vt->d3d8_mip_levels > 0 && cache_levels > (UINT)vt->d3d8_mip_levels) {
                cache_levels = (UINT)vt->d3d8_mip_levels;
            }
            video_decoder_configure_d3d8_cache(vt->decoder, vt->width, vt->height,
                                                vt->format, (int)cache_levels,
                                                vt->twitch_active ? vt->twitch_chat : NULL,
                                                &vt->twitch_settings);
        }
        {
            DWORD elapsed = now - vt->playback_start_tick;
            if (!vt->twitch_active && vt->audio_lead_ms < 0) {
                elapsed += (DWORD)(-vt->audio_lead_ms);
            } else if (!vt->twitch_active && vt->audio_lead_ms > 0) {
                elapsed = elapsed > (DWORD)vt->audio_lead_ms ? elapsed - (DWORD)vt->audio_lead_ms : 0;
            }
            if (vt->decoder) {
                if (vt->twitch_active)
                    elapsed = video_decoder_twitch_synced_target_ms(
                        vt->decoder, elapsed);
                LONGLONG decode_start = webm_perf_counter();
                decoded = video_decoder_grab_to_time(vt->decoder, elapsed);
                webm_perf_add(WEBM_PERF_D3D8_DECODE, decode_start);
                if (performance_profile && decoded) webm_perf_state.decoded_d3d8++;
            }
        }
        if (video_decoder_take_looped(vt->decoder)) {
            vt->playback_start_tick = now;
            restart_audio_for_video_loop_d3d8(vt, now);
        }
        have_frame = video_decoder_has_frame(vt->decoder);
        if (!have_frame) continue;
        if (!vt->twitch_active && vt->audio_enabled && !vt->audio_graph && !vt->engine_audio) continue;
        decoded_frame_index = video_decoder_frame_index(vt->decoder);
        if (!decoded && vt->uploaded_frame_index == decoded_frame_index) continue;
        max_levels = vt->level_count ? vt->level_count : 1;
        if (max_levels > 8) max_levels = 8;
        if (vt->d3d8_mip_levels > 0 &&
            max_levels > (UINT)vt->d3d8_mip_levels) {
            max_levels = (UINT)vt->d3d8_mip_levels;
        }
        for (level = 0; level < max_levels; level++) {
            D3DLOCKED_RECT lr;
            HRESULT lock_hr;
            LONGLONG lock_start;
            int level_width = vt->width >> level;
            int level_height = vt->height >> level;
            if (max_levels > 2 && vt->uploaded_frame_serial > 0) {
                int update_near_mips = !(vt->uploaded_frame_serial & 1u);
                if (level < 2) {
                    if (!update_near_mips) continue;
                } else if (level == 2) {
                    if (update_near_mips) continue;
                } else if ((vt->uploaded_frame_serial & 3u) != 3u) {
                    continue;
                }
            }
            if (level_width < 1) level_width = 1;
            if (level_height < 1) level_height = 1;
            if (directx_d3d11_upload && vt->d3d11_direct_upload_ready &&
                vt->d3d11_texture && captured_d3d11_context &&
                vt->d3d11_runtime_generation == captured_d3d11_generation &&
                (vt->d3d11_mip_levels == 0 || level < vt->d3d11_mip_levels)) {
                d3d8_cached_mip_view_t cached_view;
                if (video_decoder_pin_d3d8_cached_mip(vt->decoder, level,
                                                       level_width, level_height,
                                                       vt->format, &cached_view)) {
                    LONGLONG upload_start = webm_perf_counter();
                    ID3D11DeviceContext_UpdateSubresource(
                        captured_d3d11_context, (ID3D11Resource*)vt->d3d11_texture,
                        level, NULL, cached_view.pixels, (UINT)cached_view.pitch,
                        (UINT)((size_t)cached_view.pitch * (size_t)level_height));
                    video_decoder_unpin_d3d8_cached_mip(vt->decoder, &cached_view);
                    webm_perf_add(WEBM_PERF_D3D8_UPLOAD, upload_start);
                    if (!vt->d3d11_direct_upload_logged) {
                        vt->d3d11_direct_upload_logged = 1;
                        log_line("D3D11 direct upload active d3d8=%p d3d11=%p sidecar=\"%s\"",
                                 upload_texture, vt->d3d11_texture, vt->sidecar_path);
                    }
                    if (performance_profile) {
                        webm_perf_state.uploaded_d3d8_mips++;
                        webm_perf_state.cached_d3d8_mips++;
                        webm_perf_state.contiguous_d3d8_mips++;
                    }
                    uploaded_any = 1;
                    continue;
                }
            }
            lock_start = webm_perf_counter();
            lock_hr = IDirect3DTexture8_LockRect(upload_texture, level, &lr, NULL, 0);
            webm_perf_add(WEBM_PERF_D3D8_LOCK, lock_start);
            if (FAILED(lock_hr)) {
                if (vt->frame <= 3) {
                    log_line("D3D8Texture LockRect failed tex=%p level=%u size=%dx%d sidecar=\"%s\"",
                             upload_texture, level, level_width, level_height, vt->sidecar_path);
                }
                continue;
            }
            {
                LONGLONG upload_start = webm_perf_counter();
                LONGLONG convert_start = webm_perf_counter();
                LONGLONG unlock_start;
                int used_chat_base = 0;
                int used_cached_frame = 0;
                int chat_bpp = vt->format == D3DFMT_R5G6B5 ? 2 :
                               (vt->format == D3DFMT_R8G8B8 ? 3 : 4);
                used_cached_frame = video_decoder_copy_d3d8_cached_mip(
                    vt->decoder, level, &lr, level_width, level_height, vt->format);
                if (!used_cached_frame && level > 0 &&
                    vt->twitch_chat && vt->twitch_active &&
                    vt->twitch_chat_base_pixels &&
                    vt->twitch_chat_base_pitch == vt->width * chat_bpp &&
                    vt->twitch_chat_base_bpp == chat_bpp &&
                    level < WEBM_TWITCH_CHAT_MIP_LEVELS) {
                    const BYTE *source_pixels;
                    int source_width = vt->width >> (level - 1);
                    int source_height = vt->height >> (level - 1);
                    int source_pitch;
                    size_t mip_pitch = (size_t)level_width * (size_t)chat_bpp;
                    size_t mip_size = mip_pitch * (size_t)level_height;
                    BYTE *mip_pixels;
                    int mip_y;
                    if (source_width < 1) source_width = 1;
                    if (source_height < 1) source_height = 1;
                    if (level == 1) {
                        source_pixels = vt->twitch_chat_base_pixels;
                        source_pitch = vt->twitch_chat_base_pitch;
                    } else {
                        source_pixels = vt->twitch_chat_mip_pixels[level - 1];
                        source_pitch = vt->twitch_chat_mip_pitches[level - 1];
                    }
                    if (source_pixels && source_pitch >= source_width * chat_bpp) {
                        if (vt->twitch_chat_mip_sizes[level] < mip_size) {
                            BYTE *grown = (BYTE*)realloc(
                                vt->twitch_chat_mip_pixels[level], mip_size);
                            if (grown) {
                                vt->twitch_chat_mip_pixels[level] = grown;
                                vt->twitch_chat_mip_sizes[level] = mip_size;
                            }
                        }
                        mip_pixels = vt->twitch_chat_mip_pixels[level];
                        if (mip_pixels &&
                            vt->twitch_chat_mip_sizes[level] >= mip_size) {
                            webm_twitch_chat_downsample_half(
                                source_pixels, source_width, source_height,
                                source_pitch, mip_pixels, level_width,
                                level_height, (int)mip_pitch, chat_bpp);
                            vt->twitch_chat_mip_pitches[level] = (int)mip_pitch;
                            for (mip_y = 0; mip_y < level_height; mip_y++) {
                                memcpy((BYTE*)lr.pBits +
                                       (size_t)mip_y * (size_t)lr.Pitch,
                                       mip_pixels + (size_t)mip_y * mip_pitch,
                                       mip_pitch);
                            }
                            used_chat_base = 1;
                        }
                    }
                }
                if (!used_chat_base && !used_cached_frame) {
                    fill_d3d8_locked_frame(vt, &lr, decoded || have_frame,
                                           level_width, level_height);
                }
                if (!used_cached_frame && level == 0 &&
                    vt->twitch_chat && vt->twitch_active &&
                    vt->twitch_settings.chat_enabled) {
                    size_t base_pitch = (size_t)level_width * (size_t)chat_bpp;
                    size_t base_size = base_pitch * (size_t)level_height;
                    webm_twitch_chat_pixel_format_t chat_format =
                        vt->format == D3DFMT_R5G6B5 ? WEBM_TWITCH_CHAT_RGB565 :
                                                     WEBM_TWITCH_CHAT_BGR;
                    webm_twitch_chat_compose(vt->twitch_chat,
                                             &vt->twitch_settings,
                                             (unsigned char*)lr.pBits,
                                             level_width, level_height,
                                             lr.Pitch, chat_format, chat_bpp, 1);
                    if (vt->twitch_chat_base_size < base_size) {
                        BYTE *grown = (BYTE*)realloc(vt->twitch_chat_base_pixels,
                                                    base_size);
                        if (grown) {
                            vt->twitch_chat_base_pixels = grown;
                            vt->twitch_chat_base_size = base_size;
                        }
                    }
                    if (vt->twitch_chat_base_pixels &&
                        vt->twitch_chat_base_size >= base_size) {
                        int chat_y;
                        for (chat_y = 0; chat_y < level_height; chat_y++) {
                            memcpy(vt->twitch_chat_base_pixels +
                                   (size_t)chat_y * base_pitch,
                                   (unsigned char*)lr.pBits +
                                   (size_t)chat_y * (size_t)lr.Pitch,
                                   base_pitch);
                        }
                        vt->twitch_chat_base_pitch = (int)base_pitch;
                        vt->twitch_chat_base_bpp = chat_bpp;
                    }
                }
                webm_perf_add_d3d8_mip_convert(level, convert_start);
                unlock_start = webm_perf_counter();
                IDirect3DTexture8_UnlockRect(upload_texture, level);
                webm_perf_add(WEBM_PERF_D3D8_UNLOCK, unlock_start);
                webm_perf_add(WEBM_PERF_D3D8_UPLOAD, upload_start);
                if (performance_profile) {
                    webm_perf_state.uploaded_d3d8_mips++;
                    if (used_cached_frame) webm_perf_state.cached_d3d8_mips++;
                    if (used_cached_frame > 1) webm_perf_state.contiguous_d3d8_mips++;
                }
                uploaded_any = 1;
            }
        }
        if (uploaded_any) {
            vt->uploaded_frame_index = decoded_frame_index;
            vt->uploaded_frame_serial++;
        }
    }
}

static void restart_audio_for_video_loop_d3d8(video_d3d8_texture_t *vt, DWORD now)
{
    if (!vt || !vt->audio_enabled || !vt->sidecar_path[0]) return;
    if ((!vt->audio_graph && !vt->engine_audio) || !is_d3d8_sidecar_audio_owner(vt, now)) return;
    stop_older_d3d8_sidecar_audio(vt, now);
    if (vt->audio_graph) {
        audio_graph_release(vt->audio_graph);
        vt->audio_graph = NULL;
    }
    if (vt->engine_audio) {
        engine_audio_player_release(vt->engine_audio);
        vt->engine_audio = NULL;
    }
    vt->audio_ready_tick = now;
    if (vt->audio_engine && !vt->engine_audio_runtime_failed && engine_audio_runtime_available_a()) {
        vt->engine_audio = engine_audio_player_create_from_expr(vt->sidecar_path, vt->audio_node,
                                                               vt->audio_volume,
                                                               vt->audio_3d_min_distance,
                                                               vt->audio_3d_max_distance);
        if (!vt->engine_audio) vt->engine_audio_runtime_failed = 1;
    }
    if (!vt->engine_audio) {
        vt->audio_graph = audio_graph_create(vt->sidecar_path, vt->audio_lead_ms, vt->audio_volume,
                                            vt->audio_3d,
                                             vt->audio_3d_min_distance,
                                             vt->audio_3d_max_distance,
                                             vt->audio_3d_rolloff, vt->audio_node,
                                             vt->audio_effect);
    }
    if (vt->audio_graph || vt->engine_audio) {
        debug_line("D3D8Texture audio loop restart tex=%p lead_ms=%d volume=%d sidecar=\"%s\"",
                 vt->texture, vt->audio_lead_ms, vt->audio_volume, vt->sidecar_path);
    }
}

static void restart_audio_for_video_loop_gl(video_gl_texture_t *vt, DWORD now)
{
    if (!vt || !vt->audio_enabled || !vt->sidecar_path[0]) return;
    if ((!vt->audio_graph && !vt->engine_audio) || !is_gl_sidecar_audio_owner(vt, now)) return;
    stop_older_gl_sidecar_audio(vt, now);
    if (vt->audio_graph) {
        audio_graph_release(vt->audio_graph);
        vt->audio_graph = NULL;
    }
    if (vt->engine_audio) {
        engine_audio_player_release(vt->engine_audio);
        vt->engine_audio = NULL;
    }
    vt->audio_ready_tick = now;
    if (vt->audio_engine && !vt->engine_audio_runtime_failed && engine_audio_runtime_available_a()) {
        vt->engine_audio = engine_audio_player_create_from_expr(vt->sidecar_path, vt->audio_node,
                                                               vt->audio_volume,
                                                               vt->audio_3d_min_distance,
                                                               vt->audio_3d_max_distance);
        if (!vt->engine_audio) vt->engine_audio_runtime_failed = 1;
    }
    if (!vt->engine_audio) {
        vt->audio_graph = audio_graph_create(vt->sidecar_path, vt->audio_lead_ms, vt->audio_volume,
                                            vt->audio_3d,
                                             vt->audio_3d_min_distance,
                                             vt->audio_3d_max_distance,
                                             vt->audio_3d_rolloff, vt->audio_node,
                                             vt->audio_effect);
    }
    if (vt->audio_graph || vt->engine_audio) {
        debug_line("VideoTexture audio loop restart tex=%u lead_ms=%d volume=%d sidecar=\"%s\"",
                 vt->texture, vt->audio_lead_ms, vt->audio_volume, vt->sidecar_path);
    }
}

static const void *make_video_test_upload(video_gl_texture_t *vt, GLint level, GLsizei width, GLsizei height,
                                          GLenum format, GLenum type, const void *pixels)
{
    int bpp;
    size_t bytes;
    video_gl_texture_t tmp;
    if (!vt || !pixels || width <= 0 || height <= 0) return pixels;
    bpp = gl_bytes_per_pixel(format, type);
    if (!bpp) return pixels;
    bytes = (size_t)width * (size_t)height * (size_t)bpp;
    if (!bytes || bytes > vt->pixels_size) return pixels;
    memcpy(vt->pixels, pixels, bytes);
    tmp = *vt;
    tmp.width = width;
    tmp.height = height;
    tmp.format = format;
    tmp.type = type;
    tmp.pixels = vt->pixels;
    tmp.pixels_size = bytes;
    tmp.frame = vt->frame + 1;
    if (!vt->decoder) return pixels;
    video_decoder_grab_to_time(vt->decoder, 0);
    if (!video_decoder_has_frame(vt->decoder) || !fill_video_decoded_frame(&tmp)) return pixels;
    if (level == 0) {
        vt->width = width;
        vt->height = height;
        vt->format = format;
        vt->type = type;
    }
    vt->frame++;
    return vt->pixels;
}

static void apply_gl_video_filter(video_gl_texture_t *vt)
{
    const unsigned char *extensions;
    GLint min_filter;
    GLint mag_filter;
    int advanced;
    int anisotropy = 1;
    if (!vt || !real_glTexParameteri) return;
    if (!gl_filter_caps_checked) {
        gl_filter_caps_checked = 1;
        extensions = real_glGetString ? real_glGetString(0x1F03) : NULL;
        if (extensions && strstr((const char*)extensions, "GL_EXT_texture_filter_anisotropic")) {
            float max_anisotropy = 1.0f;
            gl_filter_has_anisotropy = 1;
            if (real_glGetFloatv) real_glGetFloatv(0x84FF, &max_anisotropy);
            if (max_anisotropy > 1.0f) gl_filter_max_anisotropy = max_anisotropy;
        }
        if (!real_glGenerateMipmapEXT && real_wglGetProcAddress) {
            PROC proc = real_wglGetProcAddress("glGenerateMipmapEXT");
            ULONG_PTR address = (ULONG_PTR)proc;
            if (address > 4 && address != (ULONG_PTR)-1) {
                real_glGenerateMipmapEXT = (glGenerateMipmapEXT_t)proc;
            }
        }
    }
    advanced = vt->video_filtering == VIDEO_FILTER_TRILINEAR ||
               vt->video_filtering == VIDEO_FILTER_ANISOTROPIC;
    mag_filter = vt->video_filtering == VIDEO_FILTER_NEAREST ? 0x2600 : 0x2601;
    min_filter = mag_filter;
    if (advanced && real_glGenerateMipmapEXT) min_filter = 0x2703;
    if (vt->video_filtering == VIDEO_FILTER_ANISOTROPIC && gl_filter_has_anisotropy) {
        anisotropy = vt->anisotropy;
        if ((float)anisotropy > gl_filter_max_anisotropy) anisotropy = (int)gl_filter_max_anisotropy;
        if (anisotropy < 1) anisotropy = 1;
    }
    real_glTexParameteri(0x0DE1, 0x2801, min_filter);
    real_glTexParameteri(0x0DE1, 0x2800, mag_filter);
    if (gl_filter_has_anisotropy) real_glTexParameteri(0x0DE1, 0x84FE, anisotropy);
}

static void update_gl_video_mipmaps(video_gl_texture_t *vt)
{
    if (!vt || !real_glGenerateMipmapEXT) return;
    if (vt->video_filtering != VIDEO_FILTER_TRILINEAR &&
        vt->video_filtering != VIDEO_FILTER_ANISOTROPIC) return;
    real_glGenerateMipmapEXT(0x0DE1);
}

static void update_video_test_texture(void)
{
    int i;
    unsigned int previous;
    DWORD now;
    if (video_gl_active_count <= 0 || !real_glBindTexture || !real_glTexSubImage2D) return;
    now = GetTickCount();
    refresh_global_config(now);
    previous = current_gl_texture_2d;
    for (i = 0; i < (int)(sizeof(video_gl_textures) / sizeof(video_gl_textures[0])); i++) {
        video_gl_texture_t *vt = &video_gl_textures[i];
        int bpp;
        size_t bytes;
        int decoded = 0;
        int have_frame;
        int allow_local_webm = 1;
        long decoded_frame_index;
        video_gl_texture_t tmp;
        if (!vt->active || !vt->texture || !vt->pixels) continue;
        if (!vt->last_bound_tick || (now - vt->last_bound_tick) > 1000) {
            if (vt->audio_graph) {
                audio_graph_release(vt->audio_graph);
                vt->audio_graph = NULL;
                vt->audio_ready_tick = now + 3000;
                vt->sync_hold_until_tick = 0;
            }
            if (vt->engine_audio) {
                engine_audio_player_release(vt->engine_audio);
                vt->engine_audio = NULL;
                vt->audio_ready_tick = now + 3000;
                vt->sync_hold_until_tick = 0;
            }
            continue;
        }
        refresh_gl_texture_settings(vt, now);
        if (vt->last_update_tick && (now - vt->last_update_tick) < (vt->update_interval_ms ? vt->update_interval_ms : texture_video_interval_ms)) continue;
        vt->last_update_tick = now;
        vt->frame++;
        if (vt->twitch_session) {
            allow_local_webm = update_twitch_decoder_a(vt->twitch_session, &vt->twitch_settings,
                                                       &vt->twitch_active, &vt->twitch_fallback_active,
                                                       &vt->twitch_logged_state, &vt->decoder,
                                                       &vt->decoder_attempted,
                                                       &vt->twitch_open_task,
                                                       &vt->audio_graph, &vt->engine_audio,
                                                       &vt->playback_start_tick,
                                                       vt->audio_enabled, vt->audio_volume,
                                                       vt->audio_3d,
                                                       vt->audio_3d_min_distance,
                                                       vt->audio_3d_max_distance,
                                                       vt->audio_3d_rolloff,
                                                       vt->audio_node,
                                                       vt->sidecar_path,
                                                       "opengl", now);
            if (vt->twitch_active) vt->decoder_attempted = 1;
        }
        update_twitch_chat_a(vt->twitch_session, &vt->twitch_settings,
                             vt->twitch_active, &vt->twitch_chat,
                             vt->twitch_chat_channel,
                             sizeof(vt->twitch_chat_channel));
        if (allow_local_webm && !vt->twitch_active) {
            int source_refresh = webm_source_failure_refresh_a(
                &vt->source_failure, vt->sidecar_path, now);
            if (source_refresh == WEBM_SOURCE_REFRESH_CHANGED) {
                if (vt->decoder) {
                    video_decoder_retire_async(vt->decoder);
                    vt->decoder = NULL;
                }
                vt->decoder_attempted = 0;
                vt->playback_start_tick = 0;
            }
            if (source_refresh) {
                vt->engine_audio_runtime_failed = 0;
                vt->audio_ready_tick = now;
            }
        }
        if ((ffmpeg_texture_decoder_enabled || directshow_texture_decoder_enabled) &&
            allow_local_webm &&
            !vt->decoder && !vt->decoder_attempted && vt->sidecar_path[0] &&
            vt->first_seen_tick && (now - vt->first_seen_tick) > (vt->audio_engine ? 3000u : 2000u)) {
            vt->decoder_attempted = 1;
            if (webm_source_available_for_attempt_a(&vt->source_failure,
                                                    vt->sidecar_path, now)) {
                debug_line("VideoDecoder deferred start tex=%u backend=%s sidecar=\"%s\"",
                         vt->texture, ffmpeg_texture_decoder_enabled ? "ffmpeg" : "directshow", vt->sidecar_path);
                vt->decoder = ffmpeg_texture_decoder_enabled ? ffmpeg_decoder_create(vt->sidecar_path) :
                                                               video_decoder_create(vt->sidecar_path);
                if (vt->decoder) {
                    vt->playback_start_tick = now;
                    vt->last_update_tick = 0;
                    continue;
                }
            }
        }
        if (allow_local_webm && !vt->twitch_active &&
            vt->audio_enabled && !vt->audio_graph && !vt->engine_audio && vt->sidecar_path[0] &&
            vt->first_seen_tick && vt->frame >= 30 && vt->bind_log_count >= 120 &&
            now >= vt->audio_ready_tick) {
            if (is_gl_sidecar_audio_owner(vt, now)) {
                if (!webm_source_available_for_attempt_a(&vt->source_failure,
                                                         vt->sidecar_path, now)) {
                    continue;
                }
                stop_older_gl_sidecar_audio(vt, now);
                if (vt->audio_engine && !vt->engine_audio_runtime_failed) {
                    if (!engine_audio_runtime_available_a() && (now - vt->first_seen_tick) < 10000u) {
                        vt->audio_ready_tick = now + 500u;
                        debug_line("EngineAudio runtime waiting for device sidecar=\"%s\" parent=\"%s\"",
                                 vt->sidecar_path, vt->audio_node);
                        continue;
                    } else if (engine_audio_runtime_available_a()) {
                        vt->engine_audio = engine_audio_player_create_from_expr(vt->sidecar_path, vt->audio_node,
                                                                               vt->audio_volume,
                                                                               vt->audio_3d_min_distance,
                                                                               vt->audio_3d_max_distance);
                        if (!vt->engine_audio) {
                            vt->engine_audio_runtime_failed = 1;
                            if (!engine_audio_native_playback_enabled) {
                                log_line("EngineAudio native playback parked; using regular texture audio sidecar=\"%s\" parent=\"%s\"",
                                         vt->sidecar_path, vt->audio_node);
                            } else {
                                log_line("EngineAudio runtime failed; falling back to regular audio sidecar=\"%s\" parent=\"%s\"",
                                         vt->sidecar_path, vt->audio_node);
                            }
                        }
                    } else {
                        vt->engine_audio_runtime_failed = 1;
                        log_line("EngineAudio runtime unavailable; falling back to regular audio sidecar=\"%s\" parent=\"%s\"",
                                 vt->sidecar_path, vt->audio_node);
                    }
                }
                if (!vt->engine_audio) {
                    vt->audio_graph = audio_graph_create(vt->sidecar_path, vt->audio_lead_ms, vt->audio_volume,
                                                         vt->audio_3d,
                                                         vt->audio_3d_min_distance,
                                                         vt->audio_3d_max_distance,
                                                         vt->audio_3d_rolloff, vt->audio_node,
                                                         vt->audio_effect);
                }
                if (vt->audio_graph && vt->audio_lead_ms > 0) {
                    vt->sync_hold_until_tick = now + (DWORD)vt->audio_lead_ms;
                    vt->sync_hold_logged = 0;
                }
                if (vt->audio_graph || vt->engine_audio) {
                    if (vt->decoder) {
                        video_decoder_release(vt->decoder);
                        vt->decoder = NULL;
                    }
                    vt->decoder_attempted = 0;
                    vt->last_update_tick = 0;
                    vt->playback_start_tick = now;
                    continue;
                }
                webm_source_failure_park_a(&vt->source_failure, vt->sidecar_path,
                                           now, "audio initialization failure");
                vt->audio_ready_tick = vt->source_failure.retry_tick;
            }
        }
        if (!vt->twitch_active && vt->audio_enabled &&
            vt->sync_hold_until_tick && now < vt->sync_hold_until_tick) {
            if (!vt->sync_hold_logged) {
                vt->sync_hold_logged = 1;
                debug_line("VideoTexture sync hold tex=%u hold_ms=%d sidecar=\"%s\"",
                         vt->texture, vt->audio_lead_ms, vt->sidecar_path);
            }
            continue;
        }
        if (vt->audio_graph || vt->engine_audio) {
            LONGLONG audio_start = webm_perf_counter();
            if (vt->audio_graph) {
                if (vt->twitch_active && vt->audio_graph->twitch_streaming && vt->decoder) {
                    DWORD audio_elapsed = vt->playback_start_tick ? now - vt->playback_start_tick : 0;
                    audio_graph_update_twitch_stream(vt->audio_graph, vt->decoder, audio_elapsed);
                } else {
                    audio_graph_update_3d(vt->audio_graph, now);
                }
            }
            if (vt->engine_audio) engine_audio_player_update(vt->engine_audio, now);
            webm_perf_add(WEBM_PERF_GL_AUDIO, audio_start);
        }
        if (!vt->playback_start_tick) vt->playback_start_tick = now;
        if (vt->decoder && vt->decoder->async_enabled) {
            int cache_format = gl_output_cache_format(vt->format, vt->type);
            if (cache_format) {
                video_decoder_configure_d3d8_cache(
                    vt->decoder, vt->width, vt->height, cache_format, 1,
                    vt->twitch_active ? vt->twitch_chat : NULL,
                    &vt->twitch_settings);
            }
        }
        {
            DWORD elapsed = now - vt->playback_start_tick;
            if (!vt->twitch_active && vt->audio_lead_ms < 0) {
                elapsed += (DWORD)(-vt->audio_lead_ms);
            } else if (!vt->twitch_active && vt->audio_lead_ms > 0) {
                elapsed = elapsed > (DWORD)vt->audio_lead_ms ? elapsed - (DWORD)vt->audio_lead_ms : 0;
            }
            if (vt->decoder) {
                if (vt->twitch_active)
                    elapsed = video_decoder_twitch_synced_target_ms(
                        vt->decoder, elapsed);
                LONGLONG decode_start = webm_perf_counter();
                decoded = video_decoder_grab_to_time(vt->decoder, elapsed);
                webm_perf_add(WEBM_PERF_GL_DECODE, decode_start);
                if (performance_profile && decoded) webm_perf_state.decoded_gl++;
            }
        }
        if (video_decoder_take_looped(vt->decoder)) {
            vt->playback_start_tick = now;
            restart_audio_for_video_loop_gl(vt, now);
        }
        have_frame = video_decoder_has_frame(vt->decoder);
        if (!have_frame) continue;
        if (!vt->twitch_active && vt->audio_enabled && !vt->audio_graph && !vt->engine_audio) continue;
        decoded_frame_index = video_decoder_frame_index(vt->decoder);
        if (!decoded && vt->uploaded_frame_index == decoded_frame_index) continue;
        if (real_glGetError) {
            int n;
            for (n = 0; n < 8 && real_glGetError() != 0; n++) {
            }
        }
        real_glBindTexture(0x0DE1, vt->video_texture ? vt->video_texture : vt->texture);
        apply_gl_video_filter(vt);
        if (real_glPixelStorei) real_glPixelStorei(0x0CF5, 1);
        bpp = gl_bytes_per_pixel(vt->format, vt->type);
        bytes = (size_t)vt->width * (size_t)vt->height * (size_t)bpp;
        if (bpp && bytes && bytes <= vt->pixels_size) {
            int converted;
            int used_cached_frame;
            tmp = *vt;
            tmp.pixels = vt->pixels;
            tmp.pixels_size = bytes;
            {
                LONGLONG convert_start = webm_perf_counter();
                converted = (decoded || have_frame) && fill_video_decoded_frame(&tmp);
                webm_perf_add(WEBM_PERF_GL_CONVERT, convert_start);
                if (!converted) {
                    if (real_glPixelStorei) real_glPixelStorei(0x0CF5, 4);
                    continue;
                }
            }
            used_cached_frame = converted > 1;
            if (!used_cached_frame && vt->twitch_chat && vt->twitch_active &&
                vt->twitch_settings.chat_enabled) {
                webm_twitch_chat_pixel_format_t chat_format =
                    (vt->format == 0x80E0 || vt->format == 0x80E1) ?
                    WEBM_TWITCH_CHAT_BGR : WEBM_TWITCH_CHAT_RGB;
                webm_twitch_chat_compose(vt->twitch_chat,
                                         &vt->twitch_settings,
                                         vt->pixels, vt->width, vt->height,
                                         vt->width * bpp,
                                         chat_format, bpp, 1);
            }
            {
                LONGLONG upload_start = webm_perf_counter();
                real_glTexSubImage2D(0x0DE1, 0, 0, 0,
                                     vt->width, vt->height,
                                     vt->format, vt->type,
                                     vt->pixels);
                update_gl_video_mipmaps(vt);
                webm_perf_add(WEBM_PERF_GL_UPLOAD, upload_start);
                if (performance_profile) {
                    webm_perf_state.uploaded_gl_frames++;
                    if (used_cached_frame) webm_perf_state.cached_gl_frames++;
                }
                vt->uploaded_frame_index = decoded_frame_index;
            }
        }
        if (real_glPixelStorei) real_glPixelStorei(0x0CF5, 4);
    }
    real_glBindTexture(0x0DE1, previous);
}

static void WINAPI hook_glTexImage2D(GLenum target, GLint level, GLint internalformat,
                                     GLsizei width, GLsizei height, GLint border,
                                     GLenum format, GLenum type, const void *pixels)
{
    video_gl_texture_t *vt = NULL;
    vt = remember_video_gl_texture(target, level, internalformat, width, height,
                                   format, type, pixels);
    if (vt && vt->pixels) {
        apply_gl_video_filter(vt);
        pixels = make_video_test_upload(vt, level, width, height, format, type, pixels);
    } else if (target == 0x0DE1 && current_gl_texture_2d && level > 0 && pixels) {
        vt = find_active_video_gl_texture(current_gl_texture_2d);
        if (vt && vt->pixels && vt->format == format && vt->type == type) {
            pixels = make_video_test_upload(vt, level, width, height, format, type, pixels);
        }
    }
    if (real_glTexImage2D) real_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
    if (vt && level == 0) update_gl_video_mipmaps(vt);
}

static void WINAPI hook_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
                                        GLsizei width, GLsizei height, GLenum format,
                                        GLenum type, const void *pixels)
{
    if (real_glTexSubImage2D) real_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

static BOOL WINAPI hook_SwapBuffers(HDC hdc)
{
    DWORD now = GetTickCount();
    LONGLONG perf_start = 0;
    flush_twitch_chat_opacity(now);
    flush_twitch_chat_width(now);
    if (performance_profile) {
        webm_perf_prepare(now);
        perf_start = webm_perf_counter();
    }
    update_video_test_texture();
    game_audio_refresh_mutes();
    webm_perf_add(WEBM_PERF_GL_TOTAL, perf_start);
    if (performance_profile) webm_perf_report(now);
    return real_SwapBuffers ? real_SwapBuffers(hdc) : FALSE;
}

static int capture_hook5_d3d11_runtime(void)
{
    HMODULE hook5_extended;
    ID3D11Device *device = NULL;
    ID3D11DeviceContext *context = NULL;
    unsigned int generation = 0;
    FARPROC address;

    if (!directx_d3d11_upload) return 0;
    if (!hook5_acquire_d3d11_runtime) {
        hook5_extended = GetModuleHandleA("NC-TK17-Hook5-Extended.dll");
        if (!hook5_extended) return 0;
        address = GetProcAddress(
            hook5_extended, "nc_hook5_extended_acquire_d3d11_runtime");
        if (!address) return 0;
        memcpy(&hook5_acquire_d3d11_runtime, &address, sizeof(address));
    }
    if (!hook5_acquire_d3d11_runtime(
            1u, (void **)&device, (void **)&context, &generation))
        return 0;
    if (!device || !context) {
        if (context) ID3D11DeviceContext_Release(context);
        if (device) ID3D11Device_Release(device);
        return 0;
    }
    if (generation != hook5_d3d11_generation ||
        device != captured_d3d11_device ||
        context != captured_d3d11_context) {
        capture_d3d11_runtime(device, context);
        hook5_d3d11_generation = generation;
        log_line("D3D11 runtime acquired from Hook5 Extended generation=%u device=%p context=%p",
                 generation, device, context);
    }
    ID3D11DeviceContext_Release(context);
    ID3D11Device_Release(device);
    return captured_d3d11_device && captured_d3d11_context;
}

static int resolve_hook5_resource_alias_bridge(void)
{
    HMODULE hook5_extended;
    FARPROC begin_address;
    FARPROC restore_address;
    DWORD now;
    if (hook5_begin_texture_resource_alias &&
        hook5_restore_texture_resource_aliases)
        return 1;
    now = GetTickCount();
    if (hook5_resource_alias_bridge_last_lookup_tick &&
        (now - hook5_resource_alias_bridge_last_lookup_tick) < 5000u)
        return 0;
    hook5_resource_alias_bridge_last_lookup_tick = now ? now : 1u;
    hook5_extended = GetModuleHandleA("NC-TK17-Hook5-Extended.dll");
    if (!hook5_extended) return 0;
    begin_address = GetProcAddress(
        hook5_extended, "nc_hook5_extended_begin_texture_resource_alias");
    restore_address = GetProcAddress(
        hook5_extended, "nc_hook5_extended_restore_texture_resource_aliases");
    if (!begin_address || !restore_address) return 0;
    memcpy(&hook5_begin_texture_resource_alias,
           &begin_address, sizeof(begin_address));
    memcpy(&hook5_restore_texture_resource_aliases,
           &restore_address, sizeof(restore_address));
    return 1;
}

static int begin_hook5_proxy_resource_alias(video_d3d8_texture_t *vt)
{
    if (!vt || !vt->texture || !vt->video_texture ||
        !resolve_hook5_resource_alias_bridge())
        return 0;
    if (!hook5_begin_texture_resource_alias(
            1u, (void *)vt->texture, (void *)vt->video_texture))
        return 0;
    hook5_resource_aliases_active = 1;
    if (!vt->hook5_resource_alias_logged) {
        log_line("Hook5 pass preserved by D3D8 video resource alias source=%p proxy=%p sidecar=\"%s\"",
                 vt->texture, vt->video_texture, vt->sidecar_path);
        vt->hook5_resource_alias_logged = 1;
    }
    return 1;
}

static void restore_hook5_proxy_resource_aliases(void)
{
    if (!hook5_resource_aliases_active ||
        !hook5_restore_texture_resource_aliases)
        return;
    hook5_restore_texture_resource_aliases(1u);
    hook5_resource_aliases_active = 0;
}

static void d3d11_texture_capture_begin(UINT width, UINT height)
{
    if (d3d11_texture_capture.candidate) {
        ID3D11Texture2D_Release(d3d11_texture_capture.candidate);
    }
    memset(&d3d11_texture_capture, 0, sizeof(d3d11_texture_capture));
    capture_hook5_d3d11_runtime();
    if (!directx_d3d11_upload || !captured_d3d11_device ||
        !target_texture_probe_path[0] || width == 0 || height == 0) return;
    d3d11_texture_capture.active = 1;
    d3d11_texture_capture.thread_id = GetCurrentThreadId();
    d3d11_texture_capture.width = width;
    d3d11_texture_capture.height = height;
}

static ID3D11Texture2D *d3d11_texture_capture_finish(void)
{
    ID3D11Texture2D *candidate = NULL;
    if (d3d11_texture_capture.candidate_count == 1) {
        candidate = d3d11_texture_capture.candidate;
        d3d11_texture_capture.candidate = NULL;
    } else if (d3d11_texture_capture.candidate) {
        ID3D11Texture2D_Release(d3d11_texture_capture.candidate);
        d3d11_texture_capture.candidate = NULL;
    }
    memset(&d3d11_texture_capture, 0, sizeof(d3d11_texture_capture));
    return candidate;
}

static HRESULT STDMETHODCALLTYPE hook_d3d11_CreateTexture2D(
    ID3D11Device *self, const D3D11_TEXTURE2D_DESC *desc,
    const D3D11_SUBRESOURCE_DATA *initial_data, ID3D11Texture2D **texture)
{
    HRESULT hr = real_d3d11_CreateTexture2D ?
        real_d3d11_CreateTexture2D(self, desc, initial_data, texture) : E_FAIL;
    if (SUCCEEDED(hr) && texture && *texture && desc &&
        d3d11_texture_capture.active &&
        d3d11_texture_capture.thread_id == GetCurrentThreadId() &&
        desc->Width == d3d11_texture_capture.width &&
        desc->Height == d3d11_texture_capture.height &&
        desc->ArraySize == 1 && desc->SampleDesc.Count == 1 &&
        desc->Usage == D3D11_USAGE_DEFAULT && desc->CPUAccessFlags == 0 &&
        (desc->BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0) {
        d3d11_texture_capture.candidate_count++;
        if (d3d11_texture_capture.candidate_count == 1) {
            ID3D11Texture2D_AddRef(*texture);
            d3d11_texture_capture.candidate = *texture;
        }
    }
    return hr;
}

static HRESULT WINAPI hook_d3d8_CreateTexture(IDirect3DDevice8 *self, UINT width, UINT height, UINT levels,
                                              DWORD usage, D3DFORMAT format, D3DPOOL pool,
                                              IDirect3DTexture8 **texture)
{
    HRESULT hr;
    ID3D11Texture2D *d3d11_candidate;
    d3d11_texture_capture_begin(width, height);
    hr = real_d3d8_CreateTexture ? real_d3d8_CreateTexture(self, width, height, levels, usage, format, pool, texture) : D3DERR_INVALIDCALL;
    d3d11_candidate = d3d11_texture_capture_finish();
    if (SUCCEEDED(hr) && texture && *texture) {
        remember_d3d8_texture(self, *texture, width, height, levels, usage, format, pool,
                              d3d11_candidate);
    }
    if (d3d11_candidate) ID3D11Texture2D_Release(d3d11_candidate);
    return hr;
}

static video_d3d8_texture_t *find_active_d3d8_texture(IDirect3DBaseTexture8 *texture)
{
    int i;
    for (i = 0; i < video_d3d8_active_count; i++) {
        video_d3d8_texture_t *vt = video_d3d8_active_slots[i];
        if (vt && vt->texture && (IDirect3DBaseTexture8*)vt->texture == texture) return vt;
    }
    return NULL;
}

static void restore_d3d11_video_filter(DWORD stage)
{
    d3d11_filter_override_t *state;
    ID3D11SamplerState *sampler;
    if (stage >= WEBM_D3D11_SAMPLER_STAGES) return;
    state = &d3d11_filter_overrides[stage];
    if (!state->active) return;
    sampler = state->saved_sampler;
    if (captured_d3d11_context) {
        ID3D11DeviceContext_PSSetSamplers(captured_d3d11_context, stage, 1, &sampler);
    }
    if (state->saved_sampler) ID3D11SamplerState_Release(state->saved_sampler);
    memset(state, 0, sizeof(*state));
}

static void clear_d3d11_filter_overrides(int restore)
{
    DWORD stage;
    for (stage = 0; stage < WEBM_D3D11_SAMPLER_STAGES; stage++) {
        d3d11_filter_override_t *state = &d3d11_filter_overrides[stage];
        if (!state->active) continue;
        if (restore) restore_d3d11_video_filter(stage);
        else {
            if (state->saved_sampler) ID3D11SamplerState_Release(state->saved_sampler);
            memset(state, 0, sizeof(*state));
        }
    }
}

static void capture_d3d11_runtime(ID3D11Device *device, ID3D11DeviceContext *context)
{
    if (!device || !context) return;
    patch_vtable_slot(device, 5, (void*)hook_d3d11_CreateTexture2D,
                      (void**)&real_d3d11_CreateTexture2D);
    if (captured_d3d11_device == device && captured_d3d11_context == context) return;
    clear_d3d11_filter_overrides(1);
    if (captured_d3d11_context) ID3D11DeviceContext_Release(captured_d3d11_context);
    if (captured_d3d11_device) ID3D11Device_Release(captured_d3d11_device);
    ID3D11Device_AddRef(device);
    ID3D11DeviceContext_AddRef(context);
    captured_d3d11_device = device;
    captured_d3d11_context = context;
    captured_d3d11_generation++;
    if (!captured_d3d11_generation) captured_d3d11_generation = 1;
    debug_line("D3D11 runtime captured device=%p context=%p", device, context);
}

static HRESULT WINAPI hook_D3D11CreateDeviceAndSwapChain(
    IDXGIAdapter *adapter, D3D_DRIVER_TYPE driver_type, HMODULE software, UINT flags,
    const D3D_FEATURE_LEVEL *feature_levels, UINT feature_level_count, UINT sdk_version,
    const DXGI_SWAP_CHAIN_DESC *swap_chain_desc, IDXGISwapChain **swap_chain,
    ID3D11Device **device, D3D_FEATURE_LEVEL *feature_level, ID3D11DeviceContext **context)
{
    ID3D11DeviceContext *immediate_context = NULL;
    HRESULT hr = real_D3D11CreateDeviceAndSwapChain ?
        real_D3D11CreateDeviceAndSwapChain(adapter, driver_type, software, flags,
                                           feature_levels, feature_level_count, sdk_version,
                                           swap_chain_desc, swap_chain, device, feature_level, context) : E_FAIL;
    if (SUCCEEDED(hr) && device && *device) {
        if (context && *context) capture_d3d11_runtime(*device, *context);
        else {
            ID3D11Device_GetImmediateContext(*device, &immediate_context);
            if (immediate_context) {
                capture_d3d11_runtime(*device, immediate_context);
                ID3D11DeviceContext_Release(immediate_context);
            }
        }
    }
    return hr;
}

static HRESULT WINAPI hook_D3D11CreateDevice(
    IDXGIAdapter *adapter, D3D_DRIVER_TYPE driver_type, HMODULE software, UINT flags,
    const D3D_FEATURE_LEVEL *feature_levels, UINT feature_level_count, UINT sdk_version,
    ID3D11Device **device, D3D_FEATURE_LEVEL *feature_level, ID3D11DeviceContext **context)
{
    ID3D11DeviceContext *immediate_context = NULL;
    HRESULT hr = real_D3D11CreateDevice ?
        real_D3D11CreateDevice(adapter, driver_type, software, flags, feature_levels,
                               feature_level_count, sdk_version, device, feature_level, context) : E_FAIL;
    if (SUCCEEDED(hr) && device && *device) {
        if (context && *context) capture_d3d11_runtime(*device, *context);
        else {
            ID3D11Device_GetImmediateContext(*device, &immediate_context);
            if (immediate_context) {
                capture_d3d11_runtime(*device, immediate_context);
                ID3D11DeviceContext_Release(immediate_context);
            }
        }
    }
    return hr;
}

static D3D11_FILTER d3d11_video_filter_mode(int filtering)
{
    if (filtering == VIDEO_FILTER_NEAREST) return D3D11_FILTER_MIN_MAG_MIP_POINT;
    if (filtering == VIDEO_FILTER_TRILINEAR) return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    if (filtering == VIDEO_FILTER_ANISOTROPIC) return D3D11_FILTER_ANISOTROPIC;
    return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
}

static int ensure_d3d11_video_sampler(video_d3d8_texture_t *vt, ID3D11SamplerState *base_sampler)
{
    D3D11_SAMPLER_DESC desc;
    HRESULT hr;
    if (!vt || !captured_d3d11_device) return 0;
    if (vt->d3d11_sampler &&
        vt->d3d11_sampler_generation == vt->config_generation &&
        vt->d3d11_sampler_filtering == vt->video_filtering &&
        vt->d3d11_sampler_anisotropy == vt->anisotropy) return 1;
    if (vt->d3d11_sampler) {
        ID3D11SamplerState_Release(vt->d3d11_sampler);
        vt->d3d11_sampler = NULL;
    }
    memset(&desc, 0, sizeof(desc));
    if (base_sampler) ID3D11SamplerState_GetDesc(base_sampler, &desc);
    else {
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        desc.MinLOD = -FLT_MAX;
        desc.MaxLOD = FLT_MAX;
    }
    desc.Filter = d3d11_video_filter_mode(vt->video_filtering);
    desc.MaxAnisotropy = 1;
    if (vt->video_filtering == VIDEO_FILTER_ANISOTROPIC) {
        desc.MaxAnisotropy = (UINT)vt->anisotropy;
        if (desc.MaxAnisotropy < 2) desc.MaxAnisotropy = 2;
        if (desc.MaxAnisotropy > 16) desc.MaxAnisotropy = 16;
    }
    hr = ID3D11Device_CreateSamplerState(captured_d3d11_device, &desc, &vt->d3d11_sampler);
    if (FAILED(hr) || !vt->d3d11_sampler) {
        log_line("D3D11 video sampler creation failed hr=%08lx requested=%s/%d sidecar=\"%s\"",
                 (unsigned long)hr, video_filtering_name(vt->video_filtering), vt->anisotropy,
                 vt->sidecar_path);
        return 0;
    }
    vt->d3d11_sampler_generation = vt->config_generation;
    vt->d3d11_sampler_filtering = vt->video_filtering;
    vt->d3d11_sampler_anisotropy = vt->anisotropy;
    return 1;
}

static void apply_d3d11_video_filter(DWORD stage, video_d3d8_texture_t *vt)
{
    d3d11_filter_override_t *state;
    ID3D11SamplerState *actual_sampler = NULL;
    D3D11_SAMPLER_DESC actual_desc;
    if (!captured_d3d11_context || !vt || stage >= WEBM_D3D11_SAMPLER_STAGES) return;
    state = &d3d11_filter_overrides[stage];
    if (!state->active) {
        ID3D11DeviceContext_PSGetSamplers(captured_d3d11_context, stage, 1, &state->saved_sampler);
        state->active = 1;
        state->texture = (IDirect3DBaseTexture8*)vt->texture;
    }
    if (!ensure_d3d11_video_sampler(vt, state->saved_sampler)) {
        restore_d3d11_video_filter(stage);
        return;
    }
    ID3D11DeviceContext_PSSetSamplers(captured_d3d11_context, stage, 1, &vt->d3d11_sampler);
    if (vt->filter_log_generation != vt->config_generation ||
        vt->filter_log_filtering != vt->video_filtering ||
        vt->filter_log_anisotropy != vt->anisotropy) {
        memset(&actual_desc, 0, sizeof(actual_desc));
        ID3D11DeviceContext_PSGetSamplers(captured_d3d11_context, stage, 1, &actual_sampler);
        if (actual_sampler) ID3D11SamplerState_GetDesc(actual_sampler, &actual_desc);
        debug_line("D3D11 video filtering applied tex=%p stage=%lu requested=%s/%d actual_filter=%u actual_anisotropy=%u sidecar=\"%s\"",
                 vt->texture, (unsigned long)stage, video_filtering_name(vt->video_filtering),
                 vt->anisotropy, (unsigned int)actual_desc.Filter,
                 (unsigned int)actual_desc.MaxAnisotropy, vt->sidecar_path);
        if (actual_sampler) ID3D11SamplerState_Release(actual_sampler);
        vt->filter_log_generation = vt->config_generation;
        vt->filter_log_filtering = vt->video_filtering;
        vt->filter_log_anisotropy = vt->anisotropy;
    }
}

typedef struct {
    int active[8];
    int texture_replaced[8];
    D3DMATRIX transform[8];
    DWORD transform_flags[8];
} d3d8_uv_override_state_t;

static int d3d8_video_uv_override_ready(video_d3d8_texture_t *vt)
{
    int mode;
    if (!vt || !vt->active || !vt->uv_rect_valid || vt->uploaded_frame_index < 0) return 0;
    mode = vt->twitch_active ? vt->uv_mode_twitch : vt->uv_mode_base;
    return mode == WEBM_UV_MODE_FULL_TEXTURE;
}

static void begin_d3d8_uv_overrides(IDirect3DDevice8 *device, d3d8_uv_override_state_t *state)
{
    DWORD stage;
    if (!device || !state) return;
    memset(state, 0, sizeof(*state));
    for (stage = 0; stage < 8; stage++) {
        video_d3d8_texture_t *vt = d3d8_bound_video_slots[stage];
        D3DMATRIX matrix;
        float width;
        float height;
        if (!d3d8_video_uv_override_ready(vt)) continue;
        width = vt->uv_max_u - vt->uv_min_u;
        height = vt->uv_max_v - vt->uv_min_v;
        if (width <= 0.000001f || height <= 0.000001f) continue;
        if (FAILED(IDirect3DDevice8_GetTransform(device, (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage),
                                                  &state->transform[stage]))) continue;
        if (FAILED(IDirect3DDevice8_GetTextureStageState(device, stage, D3DTSS_TEXTURETRANSFORMFLAGS,
                                                         &state->transform_flags[stage]))) continue;
        memset(&matrix, 0, sizeof(matrix));
        matrix._11 = 1.0f / width;
        matrix._22 = 1.0f / height;
        matrix._33 = 1.0f;
        matrix._44 = 1.0f;
        matrix._31 = -vt->uv_min_u / width;
        matrix._32 = -vt->uv_min_v / height;
        if (FAILED(IDirect3DDevice8_SetTransform(device, (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage),
                                                  &matrix))) continue;
        if (FAILED(IDirect3DDevice8_SetTextureStageState(device, stage, D3DTSS_TEXTURETRANSFORMFLAGS,
                                                         D3DTTFF_COUNT2))) {
            IDirect3DDevice8_SetTransform(device, (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage),
                                          &state->transform[stage]);
            continue;
        }
        if (vt->video_texture) {
            if (!begin_hook5_proxy_resource_alias(vt)) {
                if (!real_d3d8_SetTexture ||
                    FAILED(real_d3d8_SetTexture(
                        device, stage,
                        (IDirect3DBaseTexture8*)vt->video_texture))) {
                    IDirect3DDevice8_SetTextureStageState(
                        device, stage, D3DTSS_TEXTURETRANSFORMFLAGS,
                        state->transform_flags[stage]);
                    IDirect3DDevice8_SetTransform(
                        device,
                        (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage),
                        &state->transform[stage]);
                    continue;
                }
                state->texture_replaced[stage] = 1;
            }
        }
        state->active[stage] = 1;
        if (!vt->uv_override_logged) {
            debug_line("VideoUV full_texture active source=directx stage=%lu rect=(%.6f,%.6f)-(%.6f,%.6f) sidecar=\"%s\"",
                       (unsigned long)stage, vt->uv_min_u, vt->uv_min_v, vt->uv_max_u, vt->uv_max_v,
                       vt->sidecar_path);
            vt->uv_override_logged = 1;
        }
    }
}

static void end_d3d8_uv_overrides(IDirect3DDevice8 *device, d3d8_uv_override_state_t *state)
{
    int stage;
    if (!device || !state) return;
    for (stage = 7; stage >= 0; stage--) {
        if (!state->active[stage]) continue;
        if (state->texture_replaced[stage] && real_d3d8_SetTexture) {
            real_d3d8_SetTexture(device, (DWORD)stage, d3d8_bound_textures[stage]);
        }
        IDirect3DDevice8_SetTransform(device, (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage),
                                      &state->transform[stage]);
        IDirect3DDevice8_SetTextureStageState(device, (DWORD)stage, D3DTSS_TEXTURETRANSFORMFLAGS,
                                              state->transform_flags[stage]);
    }
}

static HRESULT WINAPI hook_d3d8_DrawPrimitive(IDirect3DDevice8 *self, D3DPRIMITIVETYPE type,
                                               UINT start_vertex, UINT primitive_count)
{
    HRESULT hr;
    d3d8_uv_override_state_t state;
    begin_d3d8_uv_overrides(self, &state);
    hr = real_d3d8_DrawPrimitive ? real_d3d8_DrawPrimitive(self, type, start_vertex, primitive_count) : D3DERR_INVALIDCALL;
    end_d3d8_uv_overrides(self, &state);
    return hr;
}

static HRESULT WINAPI hook_d3d8_DrawIndexedPrimitive(IDirect3DDevice8 *self, D3DPRIMITIVETYPE type,
                                                      UINT min_index, UINT num_vertices,
                                                      UINT start_index, UINT primitive_count)
{
    HRESULT hr;
    d3d8_uv_override_state_t state;
    begin_d3d8_uv_overrides(self, &state);
    hr = real_d3d8_DrawIndexedPrimitive ?
        real_d3d8_DrawIndexedPrimitive(self, type, min_index, num_vertices, start_index, primitive_count) :
        D3DERR_INVALIDCALL;
    end_d3d8_uv_overrides(self, &state);
    return hr;
}

static HRESULT WINAPI hook_d3d8_DrawPrimitiveUP(IDirect3DDevice8 *self, D3DPRIMITIVETYPE type,
                                                 UINT primitive_count, const void *data, UINT stride)
{
    HRESULT hr;
    d3d8_uv_override_state_t state;
    begin_d3d8_uv_overrides(self, &state);
    hr = real_d3d8_DrawPrimitiveUP ? real_d3d8_DrawPrimitiveUP(self, type, primitive_count, data, stride) : D3DERR_INVALIDCALL;
    end_d3d8_uv_overrides(self, &state);
    return hr;
}

static HRESULT WINAPI hook_d3d8_DrawIndexedPrimitiveUP(IDirect3DDevice8 *self, D3DPRIMITIVETYPE type,
                                                        UINT min_vertex_index, UINT vertex_count,
                                                        UINT primitive_count, const void *index_data,
                                                        D3DFORMAT index_format, const void *data, UINT stride)
{
    HRESULT hr;
    d3d8_uv_override_state_t state;
    begin_d3d8_uv_overrides(self, &state);
    hr = real_d3d8_DrawIndexedPrimitiveUP ?
        real_d3d8_DrawIndexedPrimitiveUP(self, type, min_vertex_index, vertex_count, primitive_count,
                                         index_data, index_format, data, stride) : D3DERR_INVALIDCALL;
    end_d3d8_uv_overrides(self, &state);
    return hr;
}

static HRESULT WINAPI hook_d3d8_SetTexture(IDirect3DDevice8 *self, DWORD stage, IDirect3DBaseTexture8 *texture)
{
    HRESULT hr;
    video_d3d8_texture_t *vt = find_active_d3d8_texture(texture);
    d3d11_filter_override_t *state = stage < WEBM_D3D11_SAMPLER_STAGES ? &d3d11_filter_overrides[stage] : NULL;
    if (state && state->active && (!vt || state->texture != texture)) {
        restore_d3d11_video_filter(stage);
    }
    hr = real_d3d8_SetTexture ? real_d3d8_SetTexture(self, stage, texture) : D3DERR_INVALIDCALL;
    if (SUCCEEDED(hr) && stage < 8) {
        d3d8_bound_textures[stage] = texture;
        d3d8_bound_video_slots[stage] = vt;
    }
    if (SUCCEEDED(hr) && vt) {
        vt->last_bound_tick = GetTickCount();
        vt->last_bound_present_serial = d3d8_present_serial;
        vt->bind_log_count++;
        apply_d3d11_video_filter(stage, vt);
    }
    return hr;
}

static HRESULT WINAPI hook_d3d8_Present(IDirect3DDevice8 *self, const RECT *src_rect, const RECT *dst_rect,
                                        HWND dst_window_override, const RGNDATA *dirty_region)
{
    DWORD now = GetTickCount();
    LONGLONG perf_start = 0;
    HRESULT hr;
    flush_twitch_chat_opacity(now);
    flush_twitch_chat_width(now);
    if (performance_profile) {
        webm_perf_prepare(now);
        perf_start = webm_perf_counter();
    }
    update_d3d8_video_textures();
    game_audio_refresh_mutes();
    webm_perf_add(WEBM_PERF_D3D8_TOTAL, perf_start);
    if (performance_profile) webm_perf_report(now);
    d3d8_present_serial++;
    hr = real_d3d8_Present ?
        real_d3d8_Present(self, src_rect, dst_rect,
                          dst_window_override, dirty_region) :
        D3DERR_INVALIDCALL;
    restore_hook5_proxy_resource_aliases();
    return hr;
}

static HRESULT WINAPI hook_d3d8_EndScene(IDirect3DDevice8 *self)
{
    HRESULT hr = real_d3d8_EndScene ?
        real_d3d8_EndScene(self) : D3DERR_INVALIDCALL;
    restore_hook5_proxy_resource_aliases();
    return hr;
}

static HRESULT WINAPI hook_d3d8_CreateDevice(IDirect3D8 *self, UINT adapter, D3DDEVTYPE device_type,
                                             HWND focus_window, DWORD behavior_flags,
                                             D3DPRESENT_PARAMETERS *presentation_parameters,
                                             IDirect3DDevice8 **returned_device)
{
    HRESULT hr;
    hr = real_d3d8_CreateDevice ? real_d3d8_CreateDevice(self, adapter, device_type, focus_window,
                                                         behavior_flags, presentation_parameters,
                                                         returned_device) : D3DERR_INVALIDCALL;
    if (SUCCEEDED(hr) && returned_device && *returned_device) {
        restore_hook5_proxy_resource_aliases();
        clear_d3d11_filter_overrides(1);
        clear_all_d3d8_texture_slots();
        memset(d3d8_bound_textures, 0, sizeof(d3d8_bound_textures));
        memset(d3d8_bound_video_slots, 0, sizeof(d3d8_bound_video_slots));
        debug_line("D3D8 CreateDevice hooked device=%p", *returned_device);
        patch_d3d8_device(*returned_device);
    }
    return hr;
}

static IDirect3D8 *WINAPI hook_Direct3DCreate8(UINT sdk_version)
{
    HMODULE wrapper = GetModuleHandleA("d3d8.dll");
    HMODULE d3d11 = GetModuleHandleA("d3d11.dll");
    int patched_swap = 0;
    int patched_device = 0;
    if (wrapper) {
        if (d3d11 && !real_D3D11CreateDeviceAndSwapChain) {
            real_D3D11CreateDeviceAndSwapChain = (d3d11_CreateDeviceAndSwapChain_t)
                (real_GetProcAddress ? real_GetProcAddress(d3d11, "D3D11CreateDeviceAndSwapChain") :
                                      GetProcAddress(d3d11, "D3D11CreateDeviceAndSwapChain"));
        }
        if (d3d11 && !real_D3D11CreateDevice) {
            real_D3D11CreateDevice = (d3d11_CreateDevice_t)
                (real_GetProcAddress ? real_GetProcAddress(d3d11, "D3D11CreateDevice") :
                                      GetProcAddress(d3d11, "D3D11CreateDevice"));
        }
        if (real_D3D11CreateDeviceAndSwapChain) {
            patched_swap = patch_iat_pointer(wrapper, (void*)real_D3D11CreateDeviceAndSwapChain,
                                              (void*)hook_D3D11CreateDeviceAndSwapChain);
        }
        if (real_D3D11CreateDevice) {
            patched_device = patch_iat_pointer(wrapper, (void*)real_D3D11CreateDevice,
                                                (void*)hook_D3D11CreateDevice);
        }
        debug_line("D3D11 wrapper hook result swapchain=%d device=%d wrapper=%p",
                 patched_swap, patched_device, wrapper);
    }
    IDirect3D8 *d3d = real_Direct3DCreate8 ? real_Direct3DCreate8(sdk_version) : NULL;
    if (d3d) {
        debug_line("Direct3DCreate8 hooked object=%p", d3d);
        patch_d3d8_object(d3d);
    }
    return d3d;
}

static void patch_d3d8_object(IDirect3D8 *d3d)
{
    patch_vtable_slot(d3d, 15, hook_d3d8_CreateDevice, (void**)&real_d3d8_CreateDevice);
}

static void patch_d3d8_device(IDirect3DDevice8 *dev)
{
    patch_vtable_slot(dev, 15, hook_d3d8_Present, (void**)&real_d3d8_Present);
    patch_vtable_slot(dev, 20, hook_d3d8_CreateTexture, (void**)&real_d3d8_CreateTexture);
    patch_vtable_slot(dev, 35, hook_d3d8_EndScene, (void**)&real_d3d8_EndScene);
    patch_vtable_slot(dev, 61, hook_d3d8_SetTexture, (void**)&real_d3d8_SetTexture);
    patch_vtable_slot(dev, 70, hook_d3d8_DrawPrimitive, (void**)&real_d3d8_DrawPrimitive);
    patch_vtable_slot(dev, 71, hook_d3d8_DrawIndexedPrimitive, (void**)&real_d3d8_DrawIndexedPrimitive);
    patch_vtable_slot(dev, 72, hook_d3d8_DrawPrimitiveUP, (void**)&real_d3d8_DrawPrimitiveUP);
    patch_vtable_slot(dev, 73, hook_d3d8_DrawIndexedPrimitiveUP, (void**)&real_d3d8_DrawIndexedPrimitiveUP);
}

static FARPROC hook_gl_proc_by_name(const char *name, FARPROC original)
{
    if (!name) return original;
    if (strcmp(name, "glBindTexture") == 0) {
        if (!real_glBindTexture && original && original != (FARPROC)hook_glBindTexture) {
            real_glBindTexture = (glBindTexture_t)original;
        }
        return (FARPROC)hook_glBindTexture;
    }
    if (strcmp(name, "glDeleteTextures") == 0) {
        if (!real_glDeleteTextures && original && original != (FARPROC)hook_glDeleteTextures) {
            real_glDeleteTextures = (glDeleteTextures_t)original;
        }
        return (FARPROC)hook_glDeleteTextures;
    }
    if (strcmp(name, "glTexImage2D") == 0) {
        if (!real_glTexImage2D && original && original != (FARPROC)hook_glTexImage2D) {
            real_glTexImage2D = (glTexImage2D_t)original;
        }
        return (FARPROC)hook_glTexImage2D;
    }
    if (strcmp(name, "glTexSubImage2D") == 0) {
        if (!real_glTexSubImage2D && original && original != (FARPROC)hook_glTexSubImage2D) {
            real_glTexSubImage2D = (glTexSubImage2D_t)original;
        }
        return (FARPROC)hook_glTexSubImage2D;
    }
    if (strcmp(name, "glDrawArrays") == 0) {
        if (!real_glDrawArrays && original && original != (FARPROC)hook_glDrawArrays) {
            real_glDrawArrays = (glDrawArrays_t)original;
        }
        return (FARPROC)hook_glDrawArrays;
    }
    if (strcmp(name, "glDrawElements") == 0) {
        if (!real_glDrawElements && original && original != (FARPROC)hook_glDrawElements) {
            real_glDrawElements = (glDrawElements_t)original;
        }
        return (FARPROC)hook_glDrawElements;
    }
    if (strcmp(name, "glTexParameteri") == 0) {
        if (!real_glTexParameteri && original) real_glTexParameteri = (glTexParameteri_t)original;
        return original;
    }
    if (strcmp(name, "glPixelStorei") == 0) {
        if (!real_glPixelStorei && original) real_glPixelStorei = (glPixelStorei_t)original;
        return original;
    }
    if (strcmp(name, "glGetError") == 0) {
        if (!real_glGetError && original) real_glGetError = (glGetError_t)original;
        return original;
    }
    return original;
}

static PROC WINAPI hook_wglGetProcAddress(LPCSTR name)
{
    PROC ret = real_wglGetProcAddress ? real_wglGetProcAddress(name) : NULL;
    return (PROC)hook_gl_proc_by_name(name, (FARPROC)ret);
}

static FARPROC WINAPI hook_GetProcAddress(HMODULE mod, LPCSTR name)
{
    FARPROC ret = real_GetProcAddress ? real_GetProcAddress(mod, name) : NULL;
    char modname[MAX_PATH * 2];
    if (!name || !mod) return ret;
    modname[0] = 0;
    GetModuleFileNameA(mod, modname, sizeof(modname));
    if (contains_i(modname, "opengl32.dll")) {
        return hook_gl_proc_by_name(name, ret);
    }
    if (contains_i(modname, "d3d8.dll") && strcmp(name, "Direct3DCreate8") == 0) {
        if (!real_Direct3DCreate8 && ret && ret != (FARPROC)hook_Direct3DCreate8) {
            real_Direct3DCreate8 = (Direct3DCreate8_t)ret;
        }
        return (FARPROC)hook_Direct3DCreate8;
    }
    if (contains_i(modname, "gdi32.dll") && strcmp(name, "SwapBuffers") == 0) {
        if (!real_SwapBuffers && ret && ret != (FARPROC)hook_SwapBuffers) {
            real_SwapBuffers = (SwapBuffers_t)ret;
        }
        return (FARPROC)hook_SwapBuffers;
    }
    return ret;
}

static void patch_iat(HMODULE mod, const char *dll, const char *name, void *hook, void **real)
{
    BYTE *base = (BYTE*)mod;
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS *nt;
    IMAGE_IMPORT_DESCRIPTOR *imp;
    DWORD old;

    if (!base || dos->e_magic != IMAGE_DOS_SIGNATURE) return;
    nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return;
    if (!nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) return;
    imp = (IMAGE_IMPORT_DESCRIPTOR*)(base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    for (; imp->Name; imp++) {
        const char *dllname = (const char*)(base + imp->Name);
        IMAGE_THUNK_DATA *orig = imp->OriginalFirstThunk ? (IMAGE_THUNK_DATA*)(base + imp->OriginalFirstThunk) : NULL;
        IMAGE_THUNK_DATA *thunk = (IMAGE_THUNK_DATA*)(base + imp->FirstThunk);
        if (_stricmp(dllname, dll) != 0) continue;
        if (!orig) orig = thunk;
        for (; orig->u1.AddressOfData; orig++, thunk++) {
            IMAGE_IMPORT_BY_NAME *byn;
            if (orig->u1.Ordinal & IMAGE_ORDINAL_FLAG) continue;
            byn = (IMAGE_IMPORT_BY_NAME*)(base + orig->u1.AddressOfData);
            if (strcmp((char*)byn->Name, name) != 0) continue;
            if (real && !*real) *real = (void*)thunk->u1.Function;
            if ((void*)thunk->u1.Function == hook) continue;
            if (VirtualProtect(&thunk->u1.Function, sizeof(void*), PAGE_READWRITE, &old)) {
                thunk->u1.Function = (DWORD_PTR)hook;
                VirtualProtect(&thunk->u1.Function, sizeof(void*), old, &old);
            }
        }
    }
}

static int patch_iat_pointer(HMODULE mod, void *target, void *hook)
{
    BYTE *base = (BYTE*)mod;
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS *nt;
    IMAGE_IMPORT_DESCRIPTOR *imp;
    int patched = 0;
    if (!base || !target || !hook || dos->e_magic != IMAGE_DOS_SIGNATURE) return 0;
    nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return 0;
    if (!nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) return 0;
    imp = (IMAGE_IMPORT_DESCRIPTOR*)
        (base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    for (; imp->Name; imp++) {
        IMAGE_THUNK_DATA *thunk = (IMAGE_THUNK_DATA*)(base + imp->FirstThunk);
        for (; thunk->u1.Function; thunk++) {
            DWORD old;
            if ((void*)(DWORD_PTR)thunk->u1.Function != target) continue;
            if (VirtualProtect(&thunk->u1.Function, sizeof(DWORD_PTR), PAGE_READWRITE, &old)) {
                thunk->u1.Function = (DWORD_PTR)hook;
                VirtualProtect(&thunk->u1.Function, sizeof(DWORD_PTR), old, &old);
                patched++;
            }
        }
    }
    return patched;
}

static void patch_execute_file_call(void)
{
    HMODULE sys;
    DWORD old;
    void **entry;
    void *current;
    sys = GetModuleHandleA("ThriXXX010278-SYS.dll");
    if (!sys) return;
    entry = (void**)((BYTE*)sys + (0x1016B7F8 - 0x10000000));
    current = *entry;
    if (current == (void*)hook_ExecuteFileCall) {
        execute_file_call_installed = 1;
        return;
    }
    real_ExecuteFileCall = (execute_file_call_t)current;
    if (VirtualProtect(entry, sizeof(void*), PAGE_EXECUTE_READWRITE, &old)) {
        *entry = (void*)hook_ExecuteFileCall;
        VirtualProtect(entry, sizeof(void*), old, &old);
        execute_file_call_installed = 1;
        debug_line("EngineAudio ExecuteFile hook installed entry=%p original=%p", entry, current);
    }
}

static void patch_storage_openstream(void)
{
    HMODULE sys;
    HMODULE crt;
    void *target;
    if (storage_openstream_installed) return;
    sys = GetModuleHandleA("ThriXXX010278-SYS.dll");
    if (!sys) return;
    crt = GetModuleHandleA("MSVCR110.dll");
    engine_BionicNew = engine_BionicNew ? engine_BionicNew : (bionic_new_t)GetProcAddress(sys, (LPCSTR)189);
    engine_StreamPipeFileCache_Ctor = engine_StreamPipeFileCache_Ctor ?
        engine_StreamPipeFileCache_Ctor : (streampipefilecache_ctor_t)GetProcAddress(sys, (LPCSTR)115);
    if (crt) engine_MsvcNew = engine_MsvcNew ? engine_MsvcNew : (msvc_new_t)GetProcAddress(crt, "??2@YAPAXI@Z");
    target = (void*)GetProcAddress(sys, "?OpenStream@Storage@Bionic@@SAPAVStreamPipe@2@ABVStringRef@2@IAAVString@2@@Z");
    if (!target || !engine_BionicNew || !engine_StreamPipeFileCache_Ctor || !engine_MsvcNew) {
        log_line("EngineAudio OpenStream hook unavailable target=%p bnew=%p ctor=%p mnew=%p",
                 target, (void*)engine_BionicNew, (void*)engine_StreamPipeFileCache_Ctor, (void*)engine_MsvcNew);
        return;
    }
    real_StorageOpenStream = (storage_openstream_t)target;
    if (install_inline_hook(target, (void*)hook_StorageOpenStream, 5, (void**)&tramp_StorageOpenStream)) {
        storage_openstream_installed = 1;
        debug_line("EngineAudio OpenStream hook installed target=%p trampoline=%p", target, (void*)tramp_StorageOpenStream);
    } else {
        log_line("EngineAudio OpenStream hook install failed target=%p", target);
    }
}

static void patch_sound_device_create(void)
{
    HMODULE sys;
    void *target;
    if (sound_device_create_inline_installed) return;
    resolve_engine_audio_symbols();
    sys = GetModuleHandleA("ThriXXX010278-SYS.dll");
    if (!sys) return;
    target = (void*)real_SoundDevice_Create;
    if (!target) {
        target = (void*)GetProcAddress(sys, "?Create@SoundDevice@Bionic@@SAPAV12@W4SDType@12@W4SpeakerType@12@W4Quality@12@PAX@Z");
        real_SoundDevice_Create = (sound_device_create_t)target;
    }
    if (!target) {
        log_line("EngineAudio SoundDevice::Create hook unavailable");
        return;
    }
    if (install_inline_hook(target, (void*)hook_SoundDeviceCreate, 9, (void**)&tramp_SoundDevice_Create)) {
        sound_device_create_inline_installed = 1;
        debug_line("EngineAudio SoundDevice::Create hook installed target=%p trampoline=%p slot=%p slot_device=%p",
                 target, (void*)tramp_SoundDevice_Create, (void*)engine_SoundDevice_ptr,
                 engine_SoundDevice_ptr ? *engine_SoundDevice_ptr : NULL);
    } else {
        log_line("EngineAudio SoundDevice::Create hook install failed target=%p slot=%p slot_device=%p captured=%p",
                 target, (void*)engine_SoundDevice_ptr,
                 engine_SoundDevice_ptr ? *engine_SoundDevice_ptr : NULL,
                 engine_captured_sound_device);
    }
}

static void patch_game_audio_source_lifecycle(void)
{
    HMODULE sys;
    void *destroy_target;
    resolve_engine_audio_symbols();
    sys = GetModuleHandleA("ThriXXX010278-SYS.dll");
    if (!sys || !engine_SoundSource_SetVolume || !engine_SoundSource_GetVolume) {
        log_line("GameAudio hooks unavailable sys=%p set_volume=%p get_volume=%p",
                 (void*)sys, (void*)engine_SoundSource_SetVolume,
                 (void*)engine_SoundSource_GetVolume);
        return;
    }
    destroy_target = (void*)real_SoundSource_Destroy;
    if (!destroy_target) {
        destroy_target = (void*)GetProcAddress(sys, "??1SoundSource_D@Bionic@@QAE@XZ");
        real_SoundSource_Destroy = (sound_source_destroy_t)destroy_target;
    }
    if (!sound_source_destroy_inline_installed && destroy_target) {
        if (install_inline_hook(destroy_target, (void*)hook_SoundSourceDestroy, 10,
                                (void**)&tramp_SoundSource_Destroy)) {
            sound_source_destroy_inline_installed = 1;
            debug_line("GameAudio SoundSource destructor hook installed target=%p trampoline=%p",
                       destroy_target, (void*)tramp_SoundSource_Destroy);
        } else {
            log_line("GameAudio SoundSource destructor hook install failed target=%p", destroy_target);
        }
    }
    if (!destroy_target) {
        log_line("GameAudio SoundSource destructor hook unavailable");
    }
}

static void patch_configeditor_param_change(void)
{
    HMODULE executable;
    BYTE *target;
    static const BYTE expected_prologue[6] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xc0};
    if (configeditor_param_change_installed) return;
    executable = GetModuleHandleA(NULL);
    if (!executable) return;
    target = (BYTE*)executable + 0x000fe280;
    if (!ptr_readable(target, sizeof(expected_prologue))) return;
    if (target[0] != 0xe9 && memcmp(target, expected_prologue, sizeof(expected_prologue)) != 0) {
        log_line("ConfigEditor override hook unavailable: unexpected executable build target=%p", target);
        return;
    }
    if (install_inline_hook(target, (void*)hook_ConfigEditor_ParamChange,
                            sizeof(expected_prologue),
                            (void**)&tramp_ConfigEditor_ParamChange)) {
        configeditor_param_change_installed = 1;
        debug_line("ConfigEditor override hook installed target=%p trampoline=%p",
                 target, (void*)tramp_ConfigEditor_ParamChange);
    } else {
        log_line("ConfigEditor override hook install failed target=%p", target);
    }
}

static void patch_customizer_build_controls(void)
{
    HMODULE executable;
    BYTE *target;
    static const BYTE expected_prologue[9] = {
        0x55, 0x8b, 0xec, 0x81, 0xec, 0xe0, 0x01, 0x00, 0x00
    };
    if (customizer_build_controls_installed) return;
    executable = GetModuleHandleA(NULL);
    if (!executable) return;
    target = (BYTE*)executable + 0x001c5110;
    if (!ptr_readable(target, sizeof(expected_prologue))) return;
    if (target[0] != 0xe9 &&
        memcmp(target, expected_prologue, sizeof(expected_prologue)) != 0) {
        log_line("ConfigEditor controls hook unavailable: unexpected executable build target=%p",
                 target);
        return;
    }
    if (install_inline_hook(target, (void*)hook_Customizer_BuildControls,
                            sizeof(expected_prologue),
                            (void**)&tramp_Customizer_BuildControls)) {
        customizer_build_controls_installed = 1;
        debug_line("ConfigEditor controls hook installed target=%p trampoline=%p",
                 target, (void*)tramp_Customizer_BuildControls);
    } else {
        log_line("ConfigEditor controls hook install failed target=%p", target);
    }
}

static void patch_customizer_create_button(void)
{
    HMODULE executable;
    BYTE *target;
    static const BYTE expected_prologue[6] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x24};
    if (customizer_create_button_installed) return;
    executable = GetModuleHandleA(NULL);
    if (!executable) return;
    target = (BYTE*)executable + 0x001c3310;
    if (!ptr_readable(target, sizeof(expected_prologue))) return;
    if (target[0] != 0xe9 && memcmp(target, expected_prologue, sizeof(expected_prologue)) != 0) {
        log_line("ConfigEditor button hook unavailable: unexpected executable build target=%p", target);
        return;
    }
    if (install_inline_hook(target, (void*)hook_Customizer_CreateButton,
                            sizeof(expected_prologue),
                            (void**)&tramp_Customizer_CreateButton)) {
        customizer_create_button_installed = 1;
        debug_line("ConfigEditor button hook installed target=%p trampoline=%p",
                 target, (void*)tramp_Customizer_CreateButton);
    } else {
        log_line("ConfigEditor button hook install failed target=%p", target);
    }
}

static void patch_customizer_create_slider(void)
{
    HMODULE executable;
    BYTE *target;
    static const BYTE expected_prologue[6] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xc0};
    if (customizer_create_slider_installed) return;
    executable = GetModuleHandleA(NULL);
    if (!executable) return;
    target = (BYTE*)executable + 0x001c34f0;
    if (!ptr_readable(target, sizeof(expected_prologue))) return;
    if (target[0] != 0xe9 && memcmp(target, expected_prologue, sizeof(expected_prologue)) != 0) {
        log_line("ConfigEditor slider hook unavailable: unexpected executable build target=%p",
                 target);
        return;
    }
    if (install_inline_hook(target, (void*)hook_Customizer_CreateSlider,
                            sizeof(expected_prologue),
                            (void**)&tramp_Customizer_CreateSlider)) {
        customizer_create_slider_installed = 1;
        debug_line("ConfigEditor slider hook installed target=%p trampoline=%p",
                 target, (void*)tramp_Customizer_CreateSlider);
    } else {
        log_line("ConfigEditor slider hook install failed target=%p", target);
    }
}

static void patch_module(HMODULE mod)
{
    patch_iat(mod, "KERNEL32.dll", "CreateFileA", hook_CreateFileA, (void**)&real_CreateFileA);
    patch_iat(mod, "KERNEL32.dll", "CreateFileW", hook_CreateFileW, (void**)&real_CreateFileW);
    patch_iat(mod, "KERNEL32.dll", "FindFirstFileA", hook_FindFirstFileA, (void**)&real_FindFirstFileA);
    patch_iat(mod, "KERNEL32.dll", "FindNextFileA", hook_FindNextFileA, (void**)&real_FindNextFileA);
    patch_iat(mod, "KERNEL32.dll", "FindFirstFileW", hook_FindFirstFileW, (void**)&real_FindFirstFileW);
    patch_iat(mod, "KERNEL32.dll", "FindNextFileW", hook_FindNextFileW, (void**)&real_FindNextFileW);
    patch_iat(mod, "KERNEL32.dll", "FindClose", hook_FindClose, (void**)&real_FindClose);
    patch_iat(mod, "KERNEL32.dll", "ReadFile", hook_ReadFile, (void**)&real_ReadFile);
    patch_iat(mod, "KERNEL32.dll", "CloseHandle", hook_CloseHandle, (void**)&real_CloseHandle);
    patch_iat(mod, "KERNEL32.dll", "GetProcAddress", hook_GetProcAddress, (void**)&real_GetProcAddress);
    patch_iat(mod, "ole32.dll", "CoCreateInstance", hook_CoCreateInstance, (void**)&real_CoCreateInstance);
    patch_iat(mod, "MSVCR110.dll", "fopen", hook_fopen, (void**)&real_fopen);
    patch_iat(mod, "MSVCR110.dll", "_wfopen", hook__wfopen, (void**)&real__wfopen);
    patch_iat(mod, "OPENGL32.dll", "glBindTexture", hook_glBindTexture, (void**)&real_glBindTexture);
    patch_iat(mod, "OPENGL32.dll", "glDeleteTextures", hook_glDeleteTextures, (void**)&real_glDeleteTextures);
    patch_iat(mod, "OPENGL32.dll", "glTexImage2D", hook_glTexImage2D, (void**)&real_glTexImage2D);
    patch_iat(mod, "OPENGL32.dll", "glTexSubImage2D", hook_glTexSubImage2D, (void**)&real_glTexSubImage2D);
    patch_iat(mod, "OPENGL32.dll", "glDrawArrays", hook_glDrawArrays, (void**)&real_glDrawArrays);
    patch_iat(mod, "OPENGL32.dll", "glDrawElements", hook_glDrawElements, (void**)&real_glDrawElements);
    patch_iat(mod, "OPENGL32.dll", "wglGetProcAddress", hook_wglGetProcAddress, (void**)&real_wglGetProcAddress);
    patch_iat(mod, "GDI32.dll", "SwapBuffers", hook_SwapBuffers, (void**)&real_SwapBuffers);
    patch_iat(mod, "D3D8.dll", "Direct3DCreate8", hook_Direct3DCreate8, (void**)&real_Direct3DCreate8);
    patch_iat(mod, "D3D11.dll", "D3D11CreateDeviceAndSwapChain", hook_D3D11CreateDeviceAndSwapChain,
              (void**)&real_D3D11CreateDeviceAndSwapChain);
    patch_iat(mod, "D3D11.dll", "D3D11CreateDevice", hook_D3D11CreateDevice,
              (void**)&real_D3D11CreateDevice);

    patch_iat(mod, "ThriXXX010278-SYS.dll", "?CreateVideoDecoderByExtension@VideoDecoder@Bionic@@SAPAV12@PBD@Z",
              hook_CreateVideoDecoderByExtension, (void**)&real_CreateVideoDecoderByExtension);
    patch_iat(mod, "ThriXXX010278-SYS.dll", "?Create@Texture2D@Bionic@@SAPAV12@XZ",
              hook_CreateTexture2D, (void**)&real_CreateTexture2D);
    patch_iat(mod, "ThriXXX010278-SYS.dll", "?CreateEx@Texture2D@Bionic@@SAPAV12@H@Z",
              hook_CreateExTexture2D, (void**)&real_CreateExTexture2D);
    patch_iat(mod, "ThriXXX010278-SYS.dll", "?Create@Texture2DVideo@Bionic@@SAPAV12@XZ",
              hook_CreateTexture2DVideo, (void**)&real_CreateTexture2DVideo);
    patch_iat(mod, "ThriXXX010278-SYS.dll", "?CreateEx@Texture2DVideo@Bionic@@SAPAV12@H@Z",
              hook_CreateExTexture2DVideo, (void**)&real_CreateExTexture2DVideo);
    patch_iat(mod, "ThriXXX010278-SYS.dll", "?Create@Image@Bionic@@SAPAV12@XZ",
              hook_CreateImage, (void**)&real_CreateImage);
    patch_iat(mod, "ThriXXX010278-SYS.dll", "?CreateEx@Image@Bionic@@SAPAV12@H@Z",
              hook_CreateExImage, (void**)&real_CreateExImage);
    patch_iat(mod, "ThriXXX010278-SYS.dll", "?Create@WImage@Bionic@@SAPAV12@XZ",
              hook_CreateWImage, (void**)&real_CreateWImage);
    patch_iat(mod, "ThriXXX010278-SYS.dll", "?CreateEx@WImage@Bionic@@SAPAV12@H@Z",
              hook_CreateExWImage, (void**)&real_CreateExWImage);
    patch_iat(mod, "ThriXXX010278-APP.dll", "?SetVideoPath@AppBase@@QAEXABVStringRef@Bionic@@00H@Z",
              hook_SetVideoPath, (void**)&real_SetVideoPath);
    patch_iat(mod, "ThriXXX010278-APP.dll", "?SetVideoDimension@AppBase@@QAEXABVVector2f@Bionic@@_N@Z",
              hook_SetVideoDimension, (void**)&real_SetVideoDimension);
    patch_iat(mod, "ThriXXX010278-APP.dll", "?ReplaceImage@AppBase@@SA?AW4EResult@Bionic@@PAVScriptObject@3@ABVStringRef@3@11@Z",
              hook_ReplaceImage, (void**)&real_ReplaceImage);
}

static void patch_all_modules(void)
{
    HANDLE snap;
    MODULEENTRY32 me;
    DWORD pid = GetCurrentProcessId();

    patch_configeditor_param_change();
    patch_customizer_build_controls();
    patch_customizer_create_button();
    patch_customizer_create_slider();

    {
        HMODULE sys = GetModuleHandleA("ThriXXX010278-SYS.dll");
        if (sys) {
            real_CreateVideoDecoderByExtension = real_CreateVideoDecoderByExtension ? real_CreateVideoDecoderByExtension :
                (CreateVideoDecoderByExtension_t)GetProcAddress(sys, "?CreateVideoDecoderByExtension@VideoDecoder@Bionic@@SAPAV12@PBD@Z");
            real_CreateTexture2D = real_CreateTexture2D ? real_CreateTexture2D :
                (CreateTexture2D_t)GetProcAddress(sys, "?Create@Texture2D@Bionic@@SAPAV12@XZ");
            real_CreateExTexture2D = real_CreateExTexture2D ? real_CreateExTexture2D :
                (CreateExTexture2D_t)GetProcAddress(sys, "?CreateEx@Texture2D@Bionic@@SAPAV12@H@Z");
            real_CreateTexture2DVideo = real_CreateTexture2DVideo ? real_CreateTexture2DVideo :
                (CreateTexture2DVideo_t)GetProcAddress(sys, "?Create@Texture2DVideo@Bionic@@SAPAV12@XZ");
            real_CreateExTexture2DVideo = real_CreateExTexture2DVideo ? real_CreateExTexture2DVideo :
                (CreateExTexture2DVideo_t)GetProcAddress(sys, "?CreateEx@Texture2DVideo@Bionic@@SAPAV12@H@Z");
            real_CreateImage = real_CreateImage ? real_CreateImage :
                (CreateImage_t)GetProcAddress(sys, "?Create@Image@Bionic@@SAPAV12@XZ");
            real_CreateExImage = real_CreateExImage ? real_CreateExImage :
                (CreateExImage_t)GetProcAddress(sys, "?CreateEx@Image@Bionic@@SAPAV12@H@Z");
            real_CreateWImage = real_CreateWImage ? real_CreateWImage :
                (CreateWImage_t)GetProcAddress(sys, "?Create@WImage@Bionic@@SAPAV12@XZ");
            real_CreateExWImage = real_CreateExWImage ? real_CreateExWImage :
                (CreateExWImage_t)GetProcAddress(sys, "?CreateEx@WImage@Bionic@@SAPAV12@H@Z");
        }
    }
    {
        HMODULE app = GetModuleHandleA("ThriXXX010278-APP.dll");
        if (app) {
            real_SetVideoPath = real_SetVideoPath ? real_SetVideoPath :
                (SetVideoPath_t)GetProcAddress(app, "?SetVideoPath@AppBase@@QAEXABVStringRef@Bionic@@00H@Z");
            real_SetVideoDimension = real_SetVideoDimension ? real_SetVideoDimension :
                (SetVideoDimension_t)GetProcAddress(app, "?SetVideoDimension@AppBase@@QAEXABVVector2f@Bionic@@_N@Z");
            real_ReplaceImage = real_ReplaceImage ? real_ReplaceImage :
                (ReplaceImage_t)GetProcAddress(app, "?ReplaceImage@AppBase@@SA?AW4EResult@Bionic@@PAVScriptObject@3@ABVStringRef@3@11@Z");
            real_AppTracker_GetCameraTransform = real_AppTracker_GetCameraTransform ? real_AppTracker_GetCameraTransform :
                (apptracker_get_camera_transform_t)GetProcAddress(app, "?GetCameraTransform@AppTracker@@QBEPAVScriptObject@Bionic@@XZ");
            real_AppTracker_SetCameraTransform = real_AppTracker_SetCameraTransform ? real_AppTracker_SetCameraTransform :
                (apptracker_set_camera_transform_t)GetProcAddress(app, "?SetCameraTransform@AppTracker@@QAEXPAVScriptObject@Bionic@@@Z");
            real_AppTracker_SetWorldMatrixInverse = real_AppTracker_SetWorldMatrixInverse ? real_AppTracker_SetWorldMatrixInverse :
                (apptracker_set_world_matrix_inverse_t)GetProcAddress(app, "?SetWorldMatrixInverse@AppTracker@@QAEXABVMatrix4f@Bionic@@@Z");
            if (real_AppTracker_GetCameraTransform && !apptracker_camera_transform_inline_installed) {
                apptracker_camera_transform_inline_installed =
                    install_inline_hook((void*)real_AppTracker_GetCameraTransform,
                                        (void*)hook_AppTracker_GetCameraTransform,
                                        6,
                                        (void**)&tramp_AppTracker_GetCameraTransform);
                if (apptracker_camera_transform_inline_installed) {
                    debug_line("Audio3D AppTracker::GetCameraTransform hook installed target=%p trampoline=%p",
                               (void*)real_AppTracker_GetCameraTransform,
                               (void*)tramp_AppTracker_GetCameraTransform);
                } else {
                    log_line("Audio3D AppTracker::GetCameraTransform hook install failed target=%p",
                             (void*)real_AppTracker_GetCameraTransform);
                }
            }
            if (real_AppTracker_SetCameraTransform && !apptracker_set_camera_transform_inline_installed) {
                apptracker_set_camera_transform_inline_installed =
                    install_inline_hook((void*)real_AppTracker_SetCameraTransform,
                                        (void*)hook_AppTracker_SetCameraTransform,
                                        7,
                                        (void**)&tramp_AppTracker_SetCameraTransform);
                if (apptracker_set_camera_transform_inline_installed) {
                    debug_line("Audio3D AppTracker::SetCameraTransform hook installed target=%p trampoline=%p",
                               (void*)real_AppTracker_SetCameraTransform,
                               (void*)tramp_AppTracker_SetCameraTransform);
                } else {
                    log_line("Audio3D AppTracker::SetCameraTransform hook install failed target=%p",
                             (void*)real_AppTracker_SetCameraTransform);
                }
            }
            if (real_AppTracker_SetWorldMatrixInverse && !apptracker_world_matrix_inverse_inline_installed) {
                apptracker_world_matrix_inverse_inline_installed =
                    install_inline_hook((void*)real_AppTracker_SetWorldMatrixInverse,
                                        (void*)hook_AppTracker_SetWorldMatrixInverse,
                                        7,
                                        (void**)&tramp_AppTracker_SetWorldMatrixInverse);
                if (apptracker_world_matrix_inverse_inline_installed) {
                    debug_line("Audio3D AppTracker::SetWorldMatrixInverse hook installed target=%p trampoline=%p",
                               (void*)real_AppTracker_SetWorldMatrixInverse,
                               (void*)tramp_AppTracker_SetWorldMatrixInverse);
                } else {
                    log_line("Audio3D AppTracker::SetWorldMatrixInverse hook install failed target=%p",
                             (void*)real_AppTracker_SetWorldMatrixInverse);
                }
            }
            if (real_ReplaceImage && !replace_image_inline_installed) {
                replace_image_inline_installed =
                    install_inline_hook((void*)real_ReplaceImage, (void*)hook_ReplaceImage, 6, (void**)&tramp_ReplaceImage);
                debug_line("ReplaceImage inline hook %s target=%p trampoline=%p",
                         replace_image_inline_installed ? "installed" : "not-installed",
                         (void*)real_ReplaceImage, (void*)tramp_ReplaceImage);
            }
        }
    }
    {
        HMODULE sys = GetModuleHandleA("ThriXXX010278-SYS.dll");
        if (sys) {
            patch_storage_openstream();
            patch_sound_device_create();
            patch_game_audio_source_lifecycle();
            real_StreamPipeFileRead = real_StreamPipeFileRead ? real_StreamPipeFileRead :
                (stream_read_t)GetProcAddress(sys, "?Read@StreamPipeFile@Bionic@@UAEHPAXH@Z");
            real_StreamPipeFileCacheRead = real_StreamPipeFileCacheRead ? real_StreamPipeFileCacheRead :
                (stream_read_t)GetProcAddress(sys, "?Read@StreamPipeFileCache@Bionic@@UAEHPAXH@Z");
            if (!stream_read_inline_installed) {
                int file_installed = 0;
                int cache_installed = 0;
                if (real_StreamPipeFileRead) {
                    file_installed = install_inline_hook((void*)real_StreamPipeFileRead,
                                                         (void*)hook_StreamPipeFileRead,
                                                         7,
                                                         (void**)&tramp_StreamPipeFileRead);
                }
                if (real_StreamPipeFileCacheRead) {
                    cache_installed = install_inline_hook((void*)real_StreamPipeFileCacheRead,
                                                          (void*)hook_StreamPipeFileCacheRead,
                                                          8,
                                                          (void**)&tramp_StreamPipeFileCacheRead);
                }
                stream_read_inline_installed = file_installed || cache_installed;
                debug_line("Stream read inline hooks file=%s target=%p trampoline=%p cache=%s target=%p trampoline=%p",
                         file_installed ? "installed" : "not-installed",
                         (void*)real_StreamPipeFileRead, (void*)tramp_StreamPipeFileRead,
                         cache_installed ? "installed" : "not-installed",
                         (void*)real_StreamPipeFileCacheRead, (void*)tramp_StreamPipeFileCacheRead);
            }
            if (!png_import_inline_installed) {
                real_PngImport = real_PngImport ? real_PngImport : (image_import_t)((BYTE*)sys + 0x0005dfa0);
                png_import_inline_installed = 1;
                debug_line("PNG import inline hook disabled target=%p", (void*)real_PngImport);
            }
        }
    }

    snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snap == INVALID_HANDLE_VALUE) return;
    me.dwSize = sizeof(me);
    if (Module32First(snap, &me)) {
        do {
            patch_module(me.hModule);
        } while (Module32Next(snap, &me));
    }
    CloseHandle(snap);

    real_CreateFileA = real_CreateFileA ? real_CreateFileA : (CreateFileA_t)GetProcAddress(GetModuleHandleA("kernel32.dll"), "CreateFileA");
    real_CreateFileW = real_CreateFileW ? real_CreateFileW : (CreateFileW_t)GetProcAddress(GetModuleHandleA("kernel32.dll"), "CreateFileW");
    real_FindFirstFileA = real_FindFirstFileA ? real_FindFirstFileA : (FindFirstFileA_t)GetProcAddress(GetModuleHandleA("kernel32.dll"), "FindFirstFileA");
    real_FindNextFileA = real_FindNextFileA ? real_FindNextFileA : (FindNextFileA_t)GetProcAddress(GetModuleHandleA("kernel32.dll"), "FindNextFileA");
    real_FindFirstFileW = real_FindFirstFileW ? real_FindFirstFileW : (FindFirstFileW_t)GetProcAddress(GetModuleHandleA("kernel32.dll"), "FindFirstFileW");
    real_FindNextFileW = real_FindNextFileW ? real_FindNextFileW : (FindNextFileW_t)GetProcAddress(GetModuleHandleA("kernel32.dll"), "FindNextFileW");
    real_FindClose = real_FindClose ? real_FindClose : (FindClose_t)GetProcAddress(GetModuleHandleA("kernel32.dll"), "FindClose");
    real_ReadFile = real_ReadFile ? real_ReadFile : (ReadFile_t)GetProcAddress(GetModuleHandleA("kernel32.dll"), "ReadFile");
    real_CloseHandle = real_CloseHandle ? real_CloseHandle : (CloseHandle_t)GetProcAddress(GetModuleHandleA("kernel32.dll"), "CloseHandle");
    real_GetProcAddress = real_GetProcAddress ? real_GetProcAddress : (GetProcAddress_t)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetProcAddress");
    real_CoCreateInstance = real_CoCreateInstance ? real_CoCreateInstance : (CoCreateInstance_t)GetProcAddress(GetModuleHandleA("ole32.dll"), "CoCreateInstance");
    {
        HMODULE crt = GetModuleHandleA("MSVCR110.dll");
        if (crt) {
            real_fopen = real_fopen ? real_fopen : (fopen_t)GetProcAddress(crt, "fopen");
            real__wfopen = real__wfopen ? real__wfopen : (_wfopen_t)GetProcAddress(crt, "_wfopen");
        }
    }
    {
        HMODULE gl = GetModuleHandleA("OPENGL32.dll");
        if (gl) {
            real_glBindTexture = real_glBindTexture ? real_glBindTexture : (glBindTexture_t)GetProcAddress(gl, "glBindTexture");
            real_glGenTextures = real_glGenTextures ? real_glGenTextures : (glGenTextures_t)GetProcAddress(gl, "glGenTextures");
            real_glDeleteTextures = real_glDeleteTextures ? real_glDeleteTextures : (glDeleteTextures_t)GetProcAddress(gl, "glDeleteTextures");
            real_glTexImage2D = real_glTexImage2D ? real_glTexImage2D : (glTexImage2D_t)GetProcAddress(gl, "glTexImage2D");
            real_glTexSubImage2D = real_glTexSubImage2D ? real_glTexSubImage2D : (glTexSubImage2D_t)GetProcAddress(gl, "glTexSubImage2D");
            real_glTexParameteri = real_glTexParameteri ? real_glTexParameteri : (glTexParameteri_t)GetProcAddress(gl, "glTexParameteri");
            real_glPixelStorei = real_glPixelStorei ? real_glPixelStorei : (glPixelStorei_t)GetProcAddress(gl, "glPixelStorei");
            real_glGetError = real_glGetError ? real_glGetError : (glGetError_t)GetProcAddress(gl, "glGetError");
            real_glGetString = real_glGetString ? real_glGetString : (glGetString_t)GetProcAddress(gl, "glGetString");
            real_glGetFloatv = real_glGetFloatv ? real_glGetFloatv : (glGetFloatv_t)GetProcAddress(gl, "glGetFloatv");
            real_glGetIntegerv = real_glGetIntegerv ? real_glGetIntegerv : (glGetIntegerv_t)GetProcAddress(gl, "glGetIntegerv");
            real_glMatrixMode = real_glMatrixMode ? real_glMatrixMode : (glMatrixMode_t)GetProcAddress(gl, "glMatrixMode");
            real_glPushMatrix = real_glPushMatrix ? real_glPushMatrix : (glPushMatrix_t)GetProcAddress(gl, "glPushMatrix");
            real_glPopMatrix = real_glPopMatrix ? real_glPopMatrix : (glPopMatrix_t)GetProcAddress(gl, "glPopMatrix");
            real_glLoadMatrixf = real_glLoadMatrixf ? real_glLoadMatrixf : (glLoadMatrixf_t)GetProcAddress(gl, "glLoadMatrixf");
            real_glDrawArrays = real_glDrawArrays ? real_glDrawArrays : (glDrawArrays_t)GetProcAddress(gl, "glDrawArrays");
            real_glDrawElements = real_glDrawElements ? real_glDrawElements : (glDrawElements_t)GetProcAddress(gl, "glDrawElements");
            real_wglGetProcAddress = real_wglGetProcAddress ? real_wglGetProcAddress : (wglGetProcAddress_t)GetProcAddress(gl, "wglGetProcAddress");
        }
    }
    {
        HMODULE gdi = GetModuleHandleA("GDI32.dll");
        if (gdi) {
            real_SwapBuffers = real_SwapBuffers ? real_SwapBuffers : (SwapBuffers_t)GetProcAddress(gdi, "SwapBuffers");
        }
    }
    {
        HMODULE d3d8 = GetModuleHandleA("D3D8.dll");
        if (d3d8) {
            real_Direct3DCreate8 = real_Direct3DCreate8 ? real_Direct3DCreate8 : (Direct3DCreate8_t)GetProcAddress(d3d8, "Direct3DCreate8");
        }
    }
}

static void dirname_inplace(char *path)
{
    char *slash1 = strrchr(path, '\\');
    char *slash2 = strrchr(path, '/');
    char *slash = slash1 > slash2 ? slash1 : slash2;
    if (slash) {
        *slash = '\0';
    }
}

static void path_join(char *out, size_t outsz, const char *a, const char *b)
{
    size_t n;
    if (!out || !outsz) return;
    out[0] = '\0';
    if (!a) a = "";
    if (!b) b = "";
    n = strlen(a);
    _snprintf(out, outsz - 1, "%s%s%s", a, (n && a[n - 1] != '\\' && a[n - 1] != '/') ? "\\" : "", b);
    out[outsz - 1] = '\0';
}

static int webm_extension_root_a(char *out, size_t outsz)
{
    char dll_path[MAX_PATH * 2];
    char binary_dir[MAX_PATH * 2];
    char game_dir[MAX_PATH * 2];
    char extensions_dir[MAX_PATH * 2];

    if (!out || !outsz) return 0;
    out[0] = 0;
    if (!self_module ||
        !GetModuleFileNameA(self_module, dll_path, sizeof(dll_path))) {
        return 0;
    }
    dll_path[sizeof(dll_path) - 1] = 0;
    lstrcpynA(binary_dir, dll_path, sizeof(binary_dir));
    dirname_inplace(binary_dir);
    lstrcpynA(game_dir, binary_dir, sizeof(game_dir));
    dirname_inplace(game_dir);
    path_join(extensions_dir, sizeof(extensions_dir), game_dir,
              "Extensions");
    CreateDirectoryA(extensions_dir, NULL);
    path_join(out, outsz, extensions_dir, "WebM");
    CreateDirectoryA(out, NULL);
    return out[0] != 0;
}

static void webm_component_dir_a(char *out, size_t outsz,
                                 const char *component_name)
{
    char webm_dir[MAX_PATH * 2];
    if (!out || !outsz) return;
    out[0] = 0;
    if (!component_name || !component_name[0] ||
        !webm_extension_root_a(webm_dir, sizeof(webm_dir))) {
        return;
    }
    path_join(out, outsz, webm_dir, component_name);
}

static int clamp_int(int value, int min_value, int max_value)
{
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

static void config_file_path(char *out, size_t outsz)
{
    char webm_dir[MAX_PATH * 2];
    if (!out || !outsz) return;
    out[0] = 0;
    if (!webm_extension_root_a(webm_dir, sizeof(webm_dir))) return;
    path_join(out, outsz, webm_dir, "Config.ini");
}

static void write_default_config_if_missing(const char *path)
{
    DWORD attr;
    HANDLE h;
    DWORD wrote;
    static const char defaults[] =
        "[NC-TK17-WebM]\r\n"
        "; These are global defaults for WebM texture sidecars.\r\n"
        "; A matching INI next to a WebM can override individual settings for only that video.\r\n"
        "; Example: Some_Texture.png / Some_Texture.webm / Some_Texture.ini\r\n"
        "; If a setting is missing in the local INI, this global value is used.\r\n"
        "; Boolean settings accept true/false, yes/no, on/off, and 1/0.\r\n"
        "\r\n"
        "; --- texture_fps ---\r\n"
        "; Maximum video playback rate in frames per second. Range: 1-60.\r\n"
        "; Lower values reduce decoding and texture-upload cost.\r\n"
        "; Recommended: 30 or 60 for smooth screens, 15-24 for heavier videos.\r\n"
        "texture_fps=60\r\n"
        "\r\n"
        "; --- max_texture_width / max_texture_height ---\r\n"
        "; Maximum target texture dimensions accepted by the plugin. Range: 64-4096.\r\n"
        "; WebM sidecars targeting a texture above either limit are ignored.\r\n"
        "; Lower this if users accidentally target huge textures.\r\n"
        "max_texture_width=2048\r\n"
        "max_texture_height=2048\r\n"
        "\r\n"
        "; --- directx_mip_levels ---\r\n"
        "; Number of DirectX mip levels updated with the current video frame.\r\n"
        "; 1 = very close only, best FPS\r\n"
        "; 2-3 = close range\r\n"
        "; 4-5 = medium range (recommended)\r\n"
        "; 6-8 = far range\r\n"
        "; 0 = all mip levels, most visible, most expensive\r\n"
        "directx_mip_levels=5\r\n"
        "\r\n"
        "; --- directx_d3d11_upload ---\r\n"
        "; Uses Hook5's validated D3D11 backing texture directly, avoiding the duplicate\r\n"
        "; D3D8 LockRect/UnlockRect upload. Automatically falls back when Hook5 is absent,\r\n"
        "; the texture cannot be matched exactly, or the format is unsupported.\r\n"
        "; OpenGL playback is unaffected.\r\n"
        "directx_d3d11_upload=true\r\n"
        "\r\n"
        "; --- video_filtering ---\r\n"
        "; Texture sampling mode. A local sidecar can override this per video.\r\n"
        "; nearest = sharp pixels and lowest sampling cost\r\n"
        "; linear = smooth bilinear filtering (recommended default)\r\n"
        "; trilinear = smoother transitions between DirectX mip levels\r\n"
        "; anisotropic = sharpest result on angled DirectX video surfaces\r\n"
        "; OpenGL uses advanced modes when supported, otherwise it falls back to linear.\r\n"
        "video_filtering=linear\r\n"
        "\r\n"
        "; --- anisotropy ---\r\n"
        "; Anisotropic filtering strength. Values: 1, 2, 4, 8, 16.\r\n"
        "; Used only when video_filtering=anisotropic.\r\n"
        "; Higher values sharpen angled video surfaces but may cost some GPU performance.\r\n"
        "; The active DirectX device limit is applied automatically.\r\n"
        "anisotropy=4\r\n"
        "\r\n"
        "; --- uv_mode (local sidecars only) ---\r\n"
        "; Controls how plugin video is mapped onto the target model's UV area.\r\n"
        "; Supported in both [NC-TK17-WebM] and [NC-TK17-WebM:Twitch].\r\n"
        "; off          = preserve the model's original UV mapping (default)\r\n"
        "; full_texture = stretch the detected UV rectangle across the entire video\r\n"
        "; full_texture requires a uniquely detected simple four-corner plane. If the\r\n"
        "; target is not a simple quad, the override is safely disabled for that sidecar.\r\n"
        "; A Twitch value overrides the regular WebM value; otherwise Twitch inherits it.\r\n"
        "; Works with OpenGL and DirectX. Any required resizing happens only in memory;\r\n"
        "; the add-on's original image, scene, and UV data are never modified.\r\n"
        "; Example in either supported sidecar section:\r\n"
        "; uv_mode=full_texture\r\n"
        "\r\n"
        "; --- texture_audio ---\r\n"
        "; Enables the embedded audio stream from WebM texture sidecars.\r\n"
        "; false/0 disables audio. true/1 enables audio.\r\n"
        "texture_audio=true\r\n"
        "\r\n"
        "; --- texture_audio_engine ---\r\n"
        "; Enables integration with the TK17 audio source/parent inference path.\r\n"
        "; Keep this enabled so add-on toys can use positional playback automatically.\r\n"
        "; Rooms fall back to normal synced audio unless a local sidecar identifies a source.\r\n"
        "; The DLL tries to infer the parent object from the scene file automatically.\r\n"
        "texture_audio_engine=true\r\n"
        "\r\n"
        "; --- audio_lead_ms ---\r\n"
        "; Manual WebM audio/video synchronization offset. Range: -10000 to 10000 ms.\r\n"
        "; 0 keeps the original timing. Negative values make audio earlier by skipping\r\n"
        "; audio at the start; positive values delay audio.\r\n"
        "audio_lead_ms=0\r\n"
        "\r\n"
        "; --- audio_volume ---\r\n"
        "; WebM audio volume. Range: -10000 to 20000.\r\n"
        "; 0 = normal volume.\r\n"
        "; Negative values reduce volume: -1000 = quieter, -3000 = very quiet, -10000 = silent.\r\n"
        "; Positive values boost OpenAL 3D audio: 10000 = 2x gain, 20000 = 3x gain.\r\n"
        "; DirectShow fallback cannot boost above normal and treats positive values as 0.\r\n"
        "audio_volume=0\r\n"
        "\r\n"
        "; --- texture_audio_3d ---\r\n"
        "; Enables OpenAL 3D positional playback for plugin-played WebM and Twitch audio.\r\n"
        "; Twitch live audio inherits this setting and the distance/rolloff values below.\r\n"
        "; Twitch is downmixed to mono only while 3D audio is enabled; set this to 0 for stereo.\r\n"
        "; Unsupported cases fall back to normal synced texture audio.\r\n"
        "texture_audio_3d=true\r\n"
        "\r\n"
        "; --- audio_3d_min_distance / audio_3d_max_distance ---\r\n"
        "; Positional-audio attenuation range in game units.\r\n"
        "; Audio is full volume at the minimum distance and silent at the maximum distance.\r\n"
        "audio_3d_min_distance=150\r\n"
        "audio_3d_max_distance=1200\r\n"
        "\r\n"
        "; --- audio_3d_rolloff ---\r\n"
        "; Controls how strongly positional audio fades with distance.\r\n"
        "; 0 = no distance fade, 1 = gentle, 3 = recommended, 5+ = stronger/faster fade.\r\n"
        "audio_3d_rolloff=3\r\n"
        "\r\n"
        "; --- audio_effect ---\r\n"
        "; Optional software effect for OpenAL 3D WebM audio.\r\n"
        "; Normal stereo/DirectShow fallback ignores this setting.\r\n"
        "; Values: none, echo, small_room, reverb, large_room.\r\n"
        "audio_effect=none\r\n"
        "\r\n"
        "; --- audio_node / audio_parent_path ---\r\n"
        "; Optional local-sidecar-only source overrides for 3D/engine audio.\r\n"
        "; Omit both to use automatic inference and the WebM filename without its extension.\r\n"
        "; Examples:\r\n"
        "; audio_node=\r\n"
        "; audio_node=\"TV\"\r\n"
        "; audio_node=\"pos:1.0,2.0,3.0\"\r\n"
        "; audio_parent_path=\"/Room01/TV\"\r\n"
        "; audio_parent_path=\"/Primary01/Tools/NcToy7/tool_group/CRT_TV\"\r\n"
        "\r\n"
        "; --- mute_game_audio (local sidecars only) ---\r\n"
        "; Temporarily mutes one or more native TK17 sounds while this sidecar's\r\n"
        "; WebM or Twitch video is actually displayed.\r\n"
        "; Supported in both [NC-TK17-WebM] and [NC-TK17-WebM:Twitch].\r\n"
        "; Separate multiple sound resources with commas. Matching is case-insensitive,\r\n"
        "; and the .ogg extension is optional. Resource paths are also accepted.\r\n"
        "; The sounds continue playing silently to preserve their timelines and their\r\n"
        "; previous volumes are restored when the original TK17 texture returns.\r\n"
        "; Examples:\r\n"
        "; mute_game_audio=NcRoom4_TV\r\n"
        "; mute_game_audio=NcRoom4_TV, NcRoom4_TV2\r\n"
        "; mute_game_audio=Shared/Effect/NcRoom4_TV\r\n"
        "\r\n"
        "; --- debug_logging ---\r\n"
        "; Enables verbose plugin and FFmpeg HTTP/HLS diagnostics.\r\n"
        "; Keep false/0 for normal use; warnings and errors are still preserved.\r\n"
        "debug_logging=false\r\n"
        "\r\n"
        "; --- performance_profile ---\r\n"
        "; High-resolution performance report every two seconds.\r\n"
        "; Keep false/0 normally and enable it only while measuring plugin cost.\r\n"
        "performance_profile=false\r\n"
        "\r\n"
        "; --- async_decoding ---\r\n"
        "; Enables worker-thread FFmpeg decoding to reduce main-thread playback cost.\r\n"
        "; Set false/0 to use the synchronous decoder.\r\n"
        "async_decoding=false\r\n"
        "\r\n"
        "[NC-TK17-WebM:Twitch]\r\n"
        "; Global defaults for sidecars containing [NC-TK17-WebM:Twitch].\r\n"
        "; A Twitch sidecar can override any value below.\r\n"
        "; Twitch channel, random, and fallback are intentionally sidecar-only.\r\n"
        "; OAuth tokens are encrypted for the current Windows account and stored in\r\n"
        "; Extensions\\WebM\\NC-TK17-WebM-twitch\\auth.dat.\r\n"
        "device_authorization=true\r\n"
        "; Public Twitch application Client ID; this is not a secret or stream key.\r\n"
        "client_id=3qtv6mth7so1khjgx9ecp0r3hr0s05\r\n"
        "quality=720p60\r\n"
        "; Small live-frame buffer. Higher values reduce network stutter but add latency.\r\n"
        "; Recommended: 120-250. Set 0 to disable.\r\n"
        "buffer_ms=180\r\n"
        "; Twitch-only live audio gain; fallback WebM keeps its regular volume.\r\n"
        "; 0 = normal, 10000 = 2x, 20000 = 3x.\r\n"
        "audio_volume=10000\r\n"
        "; Optional Twitch-only 3D overrides; omit to inherit regular WebM values.\r\n"
        "; audio_3d_min_distance=150\r\n"
        "; audio_3d_max_distance=1200\r\n"
        "; audio_3d_rolloff=3\r\n"
        "connect_timeout_ms=10000\r\n"
        "reconnect_interval_ms=30000\r\n"
        "; Reserved for the later random-channel milestone.\r\n"
        "random_language=en\r\n"
        "random_game=\r\n"
        "random_allow_mature=false\r\n"
        "random_min_viewers=10\r\n"
        "; Read-only Twitch chat overlay. Existing sidecars inherit false.\r\n"
        "; Enabling it requires one-time device authorization with user:read:chat.\r\n"
        "chat_enabled=false\r\n"
        "; Values: left, right.\r\n"
        "chat_position=right\r\n"
        "; true overlays chat; false fits video proportionally beside the chat panel.\r\n"
        "chat_overlay=true\r\n"
        "; Chat width fraction (0.15-0.60) and black background opacity (0.0-1.0).\r\n"
        "chat_width=0.30\r\n"
        "chat_background_opacity=0.75\r\n"
        "; Chat text height in base-texture pixels. 0 selects automatic sizing.\r\n"
        "chat_text_size=0\r\n"
        "; Internal left/right chat padding in base-texture pixels.\r\n"
        "chat_horizontal_padding=10\r\n"
        "; Twitch, BetterTTV, and 7TV emotes and their scale relative to chat text.\r\n"
        "chat_emotes=true\r\n"
        "chat_bttv=true\r\n"
        "chat_7tv=true\r\n"
        "chat_emote_scale=1.0\r\n"
        "; Animated emotes are opt-in. Range: 1-30 fps; 15 is recommended.\r\n"
        "chat_animated_emotes=false\r\n"
        "chat_animated_emote_fps=15\r\n"
        "\r\n"
        "[NC-TK17-WebM:TwitchOverride]\r\n"
        "; Runtime values managed by the in-game WebM settings page.\r\n"
        "; The selected sidecar temporarily uses the channel below while enabled.\r\n"
        "; target may be a specific sidecar path or auto_room.\r\n"
        "; target=auto_room selects the first compatible sidecar detected in the loaded room.\r\n"
        "; Compatible sidecars contain [NC-TK17-WebM], [NC-TK17-WebM:Twitch], or both.\r\n"
        "; Auto mode ignores non-room add-ons, clears its target when the room unloads,\r\n"
        "; and applies no override when the loaded room has no compatible sidecar.\r\n"
        "; Disabling the override restores the sidecar's normal Twitch, WebM, or image behavior.\r\n"
        "enabled=0\r\n"
        "target=\r\n"
        "channel=\r\n"
        "; When the configured channel is unavailable: fallback or random.\r\n"
        "; random reuses Twitch discovery, then periodically checks the configured channel.\r\n"
        "channel_offline_fallback=fallback\r\n"
        "; Maximum stream quality. Preset fallback lists only select lower renditions.\r\n"
        "quality=720p60,720p,480p,360p,worst\r\n"
        "chat_enabled=false\r\n"
        "chat_position=right\r\n"
        "chat_overlay=false\r\n"
        "chat_animated_emotes=false\r\n"
        "chat_width=0.30\r\n"
        "chat_background_opacity=0.75\r\n";
    if (!path || !path[0]) return;
    attr = GetFileAttributesA(path);
    if (attr != INVALID_FILE_ATTRIBUTES) return;
    h = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    WriteFile(h, defaults, (DWORD)(sizeof(defaults) - 1), &wrote, NULL);
    CloseHandle(h);
}

static void load_config_values(int log_enabled)
{
    char path[MAX_PATH * 2];
    char binary_dir[MAX_PATH * 2];
    char filtering_value[32];
    char override_value[64];
    char *override_end;
    double override_number;
    int was_auto_room = twitch_override_uses_auto_room_a();
    config_file_path(path, sizeof(path));
    lstrcpynA(binary_dir, path, sizeof(binary_dir));
    dirname_inplace(binary_dir);
    webm_twitch_set_binary_dir(binary_dir);
    lstrcpynA(config_path_global, path, sizeof(config_path_global));
    write_default_config_if_missing(path);
    texture_video_fps = clamp_int(GetPrivateProfileIntA("NC-TK17-WebM", "texture_fps", texture_video_fps, path), 1, 60);
    texture_video_interval_ms = (DWORD)(1000 / texture_video_fps);
    if (!texture_video_interval_ms) texture_video_interval_ms = 1;
    texture_max_width = clamp_int(GetPrivateProfileIntA("NC-TK17-WebM", "max_texture_width", texture_max_width, path), 64, 4096);
    texture_max_height = clamp_int(GetPrivateProfileIntA("NC-TK17-WebM", "max_texture_height", texture_max_height, path), 64, 4096);
    d3d8_mip_levels = GetPrivateProfileIntA("NC-TK17-WebM", "directx_mip_levels", -1, path);
    if (d3d8_mip_levels < 0) {
        d3d8_mip_levels = profile_key_bool_a(path, "NC-TK17-WebM", "directx_update_mips", 1) ? 8 : 1;
    }
    d3d8_mip_levels = clamp_int(d3d8_mip_levels, 0, 8);
    directx_d3d11_upload = profile_key_bool_a(
        path, "NC-TK17-WebM", "directx_d3d11_upload", directx_d3d11_upload);
    GetPrivateProfileStringA("NC-TK17-WebM", "video_filtering", "linear",
                             filtering_value, sizeof(filtering_value), path);
    video_filtering = parse_video_filtering_a(filtering_value, VIDEO_FILTER_LINEAR);
    video_anisotropy = normalize_video_anisotropy(
        GetPrivateProfileIntA("NC-TK17-WebM", "anisotropy", 4, path));
    texture_audio_enabled = profile_key_bool_a(path, "NC-TK17-WebM", "texture_audio", texture_audio_enabled);
    texture_audio_engine_enabled = profile_key_bool_a(path, "NC-TK17-WebM", "texture_audio_engine", texture_audio_engine_enabled);
    texture_audio_lead_ms = clamp_int(GetPrivateProfileIntA("NC-TK17-WebM", "audio_lead_ms", texture_audio_lead_ms, path), -10000, 10000);
    texture_audio_volume = clamp_int(GetPrivateProfileIntA("NC-TK17-WebM", "audio_volume", texture_audio_volume, path), -10000, 20000);
    texture_audio_3d_enabled = profile_key_bool_a(path, "NC-TK17-WebM", "texture_audio_3d", texture_audio_3d_enabled);
    texture_audio_3d_min_distance = clamp_int(GetPrivateProfileIntA("NC-TK17-WebM", "audio_3d_min_distance", texture_audio_3d_min_distance, path), 1, 100000);
    texture_audio_3d_max_distance = clamp_int(GetPrivateProfileIntA("NC-TK17-WebM", "audio_3d_max_distance", texture_audio_3d_max_distance, path), 1, 100000);
    texture_audio_3d_rolloff = clamp_int(GetPrivateProfileIntA("NC-TK17-WebM", "audio_3d_rolloff", texture_audio_3d_rolloff, path), 0, 20);
    GetPrivateProfileStringA("NC-TK17-WebM", "audio_effect", texture_audio_effect,
                             texture_audio_effect, sizeof(texture_audio_effect), path);
    if (texture_audio_3d_max_distance < texture_audio_3d_min_distance + 1) {
        texture_audio_3d_max_distance = texture_audio_3d_min_distance + 1;
    }
    debug_logging = profile_key_bool_a(path, "NC-TK17-WebM", "debug_logging", debug_logging);
    if (ffmpeg_api.av_log_set_level) {
        ffmpeg_api.av_log_set_level(debug_logging ? VM_AV_LOG_INFO : VM_AV_LOG_WARNING);
    }
    performance_profile = profile_key_bool_a(path, "NC-TK17-WebM", "performance_profile", performance_profile);
    async_decoding = profile_key_bool_a(path, "NC-TK17-WebM", "async_decoding", async_decoding);
    twitch_override_enabled = profile_key_bool_a(path, "NC-TK17-WebM:TwitchOverride", "enabled", 0);
    GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", "target", "",
                             twitch_override_target, sizeof(twitch_override_target), path);
    if (!twitch_override_uses_auto_room_a()) {
        twitch_override_auto_reset_a();
    } else if (!was_auto_room) {
        twitch_override_auto_reset_a();
        twitch_override_auto_seed_active_a();
    }
    GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", "channel", "",
                             twitch_override_channel, sizeof(twitch_override_channel), path);
    GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", "channel_offline_fallback",
                             "fallback", override_value, sizeof(override_value), path);
    twitch_override_channel_offline_random =
        _stricmp(override_value, "random") == 0 ? 1 : 0;
    GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", "quality",
                             "720p60,720p,480p,360p,worst",
                             twitch_override_quality, sizeof(twitch_override_quality), path);
    GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", "chat_enabled", "false",
                             override_value, sizeof(override_value), path);
    twitch_override_chat_enabled = override_value_is_enabled_a(override_value) ? 1 : 0;
    GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", "chat_position", "right",
                             override_value, sizeof(override_value), path);
    twitch_override_chat_position = _stricmp(override_value, "left") == 0 ? 0 : 1;
    GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", "chat_overlay", "false",
                             override_value, sizeof(override_value), path);
    twitch_override_chat_overlay = override_value_is_enabled_a(override_value) ? 1 : 0;
    GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", "chat_animated_emotes", "false",
                             override_value, sizeof(override_value), path);
    twitch_override_chat_animated_emotes =
        override_value_is_enabled_a(override_value) ? 1 : 0;
    GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", "chat_width", "0.30",
                             override_value, sizeof(override_value), path);
    override_end = NULL;
    override_number = strtod(override_value, &override_end);
    if (override_end == override_value || override_number != override_number) {
        override_number = 0.30;
    }
    if (override_number < 0.15) override_number = 0.15;
    if (override_number > 0.60) override_number = 0.60;
    twitch_override_chat_width = (float)override_number;
    GetPrivateProfileStringA("NC-TK17-WebM:TwitchOverride", "chat_background_opacity", "0.75",
                             override_value, sizeof(override_value), path);
    override_end = NULL;
    override_number = strtod(override_value, &override_end);
    if (override_end == override_value || override_number != override_number) {
        override_number = 0.75;
    }
    if (override_number < 0.0) override_number = 0.0;
    if (override_number > 1.0) override_number = 1.0;
    twitch_override_chat_background_opacity = (float)override_number;
    target_texture_interval_ms = texture_video_interval_ms;
    target_texture_max_width = texture_max_width;
    target_texture_max_height = texture_max_height;
    target_texture_d3d8_mip_levels = d3d8_mip_levels;
    target_texture_video_filtering = video_filtering;
    target_texture_anisotropy = video_anisotropy;
    get_file_write_time_a(path, &config_write_time);
    if (log_enabled) {
        log_line("NC-TK17-WebM config loaded");
        debug_line("config texture_fps=%d interval_ms=%lu max_texture=%dx%d directx_mip_levels=%d directx_d3d11_upload=%d video_filtering=%s anisotropy=%d texture_audio=%d texture_audio_engine=%d audio_lead_ms=%d audio_volume=%d texture_audio_3d=%d audio_3d_distance=%d-%d audio_3d_rolloff=%d audio_effect=\"%s\" debug_logging=%d performance_profile=%d async_decoding=%d",
                   texture_video_fps, (unsigned long)texture_video_interval_ms,
                   texture_max_width, texture_max_height, d3d8_mip_levels,
                   directx_d3d11_upload,
                   video_filtering_name(video_filtering), video_anisotropy,
                   texture_audio_enabled, texture_audio_engine_enabled,
                   texture_audio_lead_ms, texture_audio_volume,
                   texture_audio_3d_enabled, texture_audio_3d_min_distance,
                   texture_audio_3d_max_distance, texture_audio_3d_rolloff,
                   texture_audio_effect, debug_logging, performance_profile, async_decoding);
        debug_line("Twitch override enabled=%d target=\"%s\" auto_room=\"%s\" auto_target=\"%s\" channel=\"%s\" offline_fallback=%s quality=\"%s\" chat=%d position=%s overlay=%d animated_emotes=%d width=%.3f opacity=%.3f",
                   twitch_override_enabled, twitch_override_target,
                   twitch_override_auto_room, twitch_override_auto_target,
                   twitch_override_channel,
                   twitch_override_channel_offline_random ? "random" : "fallback",
                   twitch_override_quality,
                   twitch_override_chat_enabled,
                   twitch_override_chat_position == 0 ? "left" : "right",
                   twitch_override_chat_overlay,
                   twitch_override_chat_animated_emotes,
                   twitch_override_chat_width,
                   twitch_override_chat_background_opacity);
    }
}

static void reload_global_config_now(void)
{
    load_config_values(0);
    config_generation++;
    if (!config_generation) config_generation = 1;
    last_global_config_check_tick = GetTickCount();
    debug_line("global config runtime refresh generation=%u", config_generation);
}

static void load_config(void)
{
    if (config_loaded) return;
    config_loaded = 1;
    load_config_values(0);
}

static void refresh_global_config(DWORD now)
{
    FILETIME write_time;
    if (!config_loaded) {
        load_config();
        return;
    }
    if (last_global_config_check_tick && (now - last_global_config_check_tick) < 1000) return;
    last_global_config_check_tick = now;
    if (!config_path_global[0]) config_file_path(config_path_global, sizeof(config_path_global));
    get_file_write_time_a(config_path_global, &write_time);
    if (!filetime_differs(&write_time, &config_write_time)) return;
    load_config_values(1);
    config_generation++;
    if (!config_generation) config_generation = 1;
    debug_line("global config refresh generation=%u", config_generation);
}

__declspec(dllexport) int loadextension(void)
{
    load_config();
    audio_cache_manifest_cleanup_a();
    debug_line("loadextension");
    patch_all_modules();
    return 1;
}

__declspec(dllexport) int on_create(void)
{
    load_config();
    audio_cache_manifest_cleanup_a();
    debug_line("on_create");
    patch_all_modules();
    return 1;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        self_module = hinst;
        DisableThreadLibraryCalls(hinst);
        InitializeCriticalSection(&log_lock);
        log_ready = 1;
        InitializeCriticalSection(&video_decoder_retire_lock);
        video_decoder_retire_lock_ready = 1;
        InitializeCriticalSection(&game_audio_mute_lock);
        game_audio_mute_lock_ready = 1;
        debug_line("NC-TK17-WebM.dll attached");
        load_config();
    } else if (reason == DLL_PROCESS_DETACH) {
        debug_line("NC-TK17-WebM.dll detached");
        video_decoder_drain_retired();
        video_decoder_retire_lock_ready = 0;
        DeleteCriticalSection(&video_decoder_retire_lock);
        game_audio_mute_lock_ready = 0;
        DeleteCriticalSection(&game_audio_mute_lock);
        log_ready = 0;
        DeleteCriticalSection(&log_lock);
    }
    return TRUE;
}
