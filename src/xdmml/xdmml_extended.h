#ifndef XDMML_EXTENDED_H
#define XDMML_EXTENDED_H

/*
 * ═══════════════════════════════════════════════════════════════════════════
 * XDMML Extended - Complete System Abstraction Layer
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 * Additional Features:
 *   • Threading: Cross-platform thread management
 *   • Networking: Sockets, HTTP, WebSocket support
 *   • Dynamic Loading: Shared libraries (.so, .dylib)
 *   • Advanced Audio: Mixing, effects, streaming
 *   • Video: Decoding, playback, recording
 *   • Advanced Input: Gamepad, joystick, touch
 */

#include "xdmml.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ═══════════════════════════════════════════════════════════════════════════
// THREADING
// ═══════════════════════════════════════════════════════════════════════════

typedef struct XDMML_Thread XDMML_Thread;
typedef struct XDMML_Mutex XDMML_Mutex;
typedef struct XDMML_Condition XDMML_Condition;
typedef struct XDMML_Semaphore XDMML_Semaphore;

typedef int (*XDMML_ThreadFunc)(void* data);

/**
 * Create a new thread
 * @param fn Thread function
 * @param name Thread name (optional)
 * @param data User data passed to function
 * @return Thread handle or NULL
 */
XDMML_Thread* xdmml_create_thread(XDMML_ThreadFunc fn, const char* name, void* data);

/**
 * Wait for thread to finish
 * @param thread Thread handle
 * @param status Output for return value (optional)
 */
void xdmml_wait_thread(XDMML_Thread* thread, int* status);

/**
 * Detach thread (automatic cleanup on exit)
 */
void xdmml_detach_thread(XDMML_Thread* thread);

/**
 * Get current thread ID
 */
uint64_t xdmml_thread_id(void);

/**
 * Create mutex
 */
XDMML_Mutex* xdmml_create_mutex(void);

/**
 * Destroy mutex
 */
void xdmml_destroy_mutex(XDMML_Mutex* mutex);

/**
 * Lock mutex
 */
void xdmml_lock_mutex(XDMML_Mutex* mutex);

/**
 * Try to lock mutex (non-blocking)
 * @return true if locked, false if already locked
 */
bool xdmml_try_lock_mutex(XDMML_Mutex* mutex);

/**
 * Unlock mutex
 */
void xdmml_unlock_mutex(XDMML_Mutex* mutex);

/**
 * Create condition variable
 */
XDMML_Condition* xdmml_create_condition(void);

/**
 * Destroy condition variable
 */
void xdmml_destroy_condition(XDMML_Condition* cond);

/**
 * Wait on condition variable
 */
void xdmml_wait_condition(XDMML_Condition* cond, XDMML_Mutex* mutex);

/**
 * Wait on condition with timeout
 * @param ms Timeout in milliseconds
 * @return true if signaled, false if timeout
 */
bool xdmml_wait_condition_timeout(XDMML_Condition* cond, XDMML_Mutex* mutex, uint32_t ms);

/**
 * Signal one waiting thread
 */
void xdmml_signal_condition(XDMML_Condition* cond);

/**
 * Signal all waiting threads
 */
void xdmml_broadcast_condition(XDMML_Condition* cond);

/**
 * Create semaphore
 * @param initial_value Initial semaphore count
 */
XDMML_Semaphore* xdmml_create_semaphore(uint32_t initial_value);

/**
 * Destroy semaphore
 */
void xdmml_destroy_semaphore(XDMML_Semaphore* sem);

/**
 * Wait on semaphore (decrement)
 */
void xdmml_wait_semaphore(XDMML_Semaphore* sem);

/**
 * Try wait on semaphore (non-blocking)
 */
bool xdmml_try_wait_semaphore(XDMML_Semaphore* sem);

/**
 * Post to semaphore (increment)
 */
void xdmml_post_semaphore(XDMML_Semaphore* sem);

// ═══════════════════════════════════════════════════════════════════════════
// NETWORKING
// ═══════════════════════════════════════════════════════════════════════════

typedef struct XDMML_Socket XDMML_Socket;

typedef enum {
    XDMML_SOCKET_TCP,
    XDMML_SOCKET_UDP,
} XDMML_SocketType;

/**
 * Create socket
 */
XDMML_Socket* xdmml_create_socket(XDMML_SocketType type);

/**
 * Close socket
 */
void xdmml_close_socket(XDMML_Socket* socket);

/**
 * Connect to remote host
 * @param host Hostname or IP address
 * @param port Port number
 * @return XDMML_OK on success
 */
XDMML_Result xdmml_connect(XDMML_Socket* socket, const char* host, uint16_t port);

/**
 * Bind socket to address
 */
XDMML_Result xdmml_bind(XDMML_Socket* socket, const char* address, uint16_t port);

/**
 * Listen for connections (TCP server)
 * @param backlog Maximum pending connections
 */
XDMML_Result xdmml_listen(XDMML_Socket* socket, int backlog);

/**
 * Accept incoming connection
 * @return New socket for accepted connection
 */
XDMML_Socket* xdmml_accept(XDMML_Socket* socket);

/**
 * Send data
 * @param data Buffer to send
 * @param len Length of buffer
 * @return Number of bytes sent, or -1 on error
 */
int xdmml_send(XDMML_Socket* socket, const void* data, size_t len);

/**
 * Receive data
 * @param buffer Buffer to receive into
 * @param len Maximum bytes to receive
 * @return Number of bytes received, 0 on disconnect, -1 on error
 */
int xdmml_recv(XDMML_Socket* socket, void* buffer, size_t len);

/**
 * Set socket blocking mode
 */
void xdmml_set_socket_blocking(XDMML_Socket* socket, bool blocking);

/**
 * Set socket timeout
 * @param ms Timeout in milliseconds (0 = infinite)
 */
void xdmml_set_socket_timeout(XDMML_Socket* socket, uint32_t ms);

// HTTP Client
typedef struct {
    int status_code;
    char* body;
    size_t body_length;
    char* content_type;
} XDMML_HTTPResponse;

/**
 * Perform HTTP GET request
 * @param url URL to fetch
 * @return Response (must be freed with xdmml_free_http_response)
 */
XDMML_HTTPResponse* xdmml_http_get(const char* url);

/**
 * Perform HTTP POST request
 * @param url URL to post to
 * @param data Data to post
 * @param length Data length
 * @param content_type Content-Type header
 */
XDMML_HTTPResponse* xdmml_http_post(const char* url, const void* data, 
                                     size_t length, const char* content_type);

/**
 * Free HTTP response
 */
void xdmml_free_http_response(XDMML_HTTPResponse* response);

// ═══════════════════════════════════════════════════════════════════════════
// SHARED LIBRARY LOADING
// ═══════════════════════════════════════════════════════════════════════════

typedef struct XDMML_SharedObject XDMML_SharedObject;

/**
 * Load shared library
 * @param path Path to library (.so, .dylib, .dll)
 * @return Library handle or NULL
 */
XDMML_SharedObject* xdmml_load_library(const char* path);

/**
 * Unload shared library
 */
void xdmml_unload_library(XDMML_SharedObject* library);

/**
 * Get symbol from library
 * @param name Symbol name
 * @return Function pointer or NULL
 */
void* xdmml_get_symbol(XDMML_SharedObject* library, const char* name);

/**
 * Get last library error
 */
const char* xdmml_library_error(void);

// ═══════════════════════════════════════════════════════════════════════════
// ADVANCED AUDIO
// ═══════════════════════════════════════════════════════════════════════════

typedef struct XDMML_AudioMixer XDMML_AudioMixer;
typedef struct XDMML_AudioStream XDMML_AudioStream;

/**
 * Create audio mixer
 * @param spec Audio specification
 * @param channels Maximum simultaneous sounds
 */
XDMML_AudioMixer* xdmml_create_mixer(XDMML_AudioSpec* spec, int channels);

/**
 * Destroy mixer
 */
void xdmml_destroy_mixer(XDMML_AudioMixer* mixer);

/**
 * Load sound into mixer
 * @param filename Path to audio file (WAV, OGG, MP3)
 * @return Sound ID, or -1 on error
 */
int xdmml_mixer_load_sound(XDMML_AudioMixer* mixer, const char* filename);

/**
 * Play sound
 * @param sound_id Sound ID from load_sound
 * @param loops Number of loops (-1 = infinite)
 * @return Channel number playing on
 */
int xdmml_mixer_play(XDMML_AudioMixer* mixer, int sound_id, int loops);

/**
 * Stop channel
 */
void xdmml_mixer_stop(XDMML_AudioMixer* mixer, int channel);

/**
 * Set channel volume (0.0 - 1.0)
 */
void xdmml_mixer_set_volume(XDMML_AudioMixer* mixer, int channel, float volume);

/**
 * Create audio stream for real-time generation
 */
XDMML_AudioStream* xdmml_create_audio_stream(XDMML_AudioSpec* spec);

/**
 * Write samples to stream
 */
void xdmml_stream_write(XDMML_AudioStream* stream, const void* data, size_t bytes);

/**
 * Close stream
 */
void xdmml_close_audio_stream(XDMML_AudioStream* stream);

// ═══════════════════════════════════════════════════════════════════════════
// VIDEO
// ═══════════════════════════════════════════════════════════════════════════

typedef struct XDMML_Video XDMML_Video;

typedef struct {
    int width;
    int height;
    int fps;
    double duration;
    const char* codec;
} XDMML_VideoInfo;

/**
 * Load video file
 * @param filename Path to video file
 * @return Video handle or NULL
 */
XDMML_Video* xdmml_load_video(const char* filename);

/**
 * Close video
 */
void xdmml_close_video(XDMML_Video* video);

/**
 * Get video information
 */
XDMML_VideoInfo xdmml_get_video_info(XDMML_Video* video);

/**
 * Decode next frame
 * @param pixels Output buffer (RGBA format)
 * @return true if frame decoded, false if end of video
 */
bool xdmml_video_next_frame(XDMML_Video* video, uint8_t* pixels);

/**
 * Seek to timestamp
 * @param timestamp Time in seconds
 */
void xdmml_video_seek(XDMML_Video* video, double timestamp);

// ═══════════════════════════════════════════════════════════════════════════
// ADVANCED INPUT
// ═══════════════════════════════════════════════════════════════════════════

typedef struct XDMML_Gamepad XDMML_Gamepad;

typedef enum {
    XDMML_BUTTON_A,
    XDMML_BUTTON_B,
    XDMML_BUTTON_X,
    XDMML_BUTTON_Y,
    XDMML_BUTTON_BACK,
    XDMML_BUTTON_GUIDE,
    XDMML_BUTTON_START,
    XDMML_BUTTON_LEFTSTICK,
    XDMML_BUTTON_RIGHTSTICK,
    XDMML_BUTTON_LEFTSHOULDER,
    XDMML_BUTTON_RIGHTSHOULDER,
    XDMML_BUTTON_DPAD_UP,
    XDMML_BUTTON_DPAD_DOWN,
    XDMML_BUTTON_DPAD_LEFT,
    XDMML_BUTTON_DPAD_RIGHT,
} XDMML_GamepadButton;

typedef enum {
    XDMML_AXIS_LEFTX,
    XDMML_AXIS_LEFTY,
    XDMML_AXIS_RIGHTX,
    XDMML_AXIS_RIGHTY,
    XDMML_AXIS_TRIGGERLEFT,
    XDMML_AXIS_TRIGGERRIGHT,
} XDMML_GamepadAxis;

/**
 * Get number of connected gamepads
 */
int xdmml_num_gamepads(void);

/**
 * Open gamepad
 * @param index Gamepad index (0 to num_gamepads-1)
 */
XDMML_Gamepad* xdmml_open_gamepad(int index);

/**
 * Close gamepad
 */
void xdmml_close_gamepad(XDMML_Gamepad* gamepad);

/**
 * Get gamepad button state
 * @return true if pressed
 */
bool xdmml_gamepad_button(XDMML_Gamepad* gamepad, XDMML_GamepadButton button);

/**
 * Get gamepad axis value
 * @return Value from -1.0 to 1.0
 */
float xdmml_gamepad_axis(XDMML_Gamepad* gamepad, XDMML_GamepadAxis axis);

/**
 * Set gamepad rumble
 * @param low_frequency Low frequency rumble (0.0 - 1.0)
 * @param high_frequency High frequency rumble (0.0 - 1.0)
 * @param duration Duration in milliseconds
 */
void xdmml_gamepad_rumble(XDMML_Gamepad* gamepad, float low_frequency, 
                          float high_frequency, uint32_t duration);

/**
 * Get gamepad name
 */
const char* xdmml_gamepad_name(XDMML_Gamepad* gamepad);

// ═══════════════════════════════════════════════════════════════════════════
// FILE SYSTEM
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Get user's home directory
 */
const char* xdmml_get_home_dir(void);

/**
 * Get application preference directory
 * @param org Organization name
 * @param app Application name
 */
const char* xdmml_get_pref_dir(const char* org, const char* app);

/**
 * Get base path (executable directory)
 */
const char* xdmml_get_base_path(void);

/**
 * Check if path exists
 */
bool xdmml_path_exists(const char* path);

/**
 * Check if path is directory
 */
bool xdmml_is_directory(const char* path);

/**
 * Create directory (recursive)
 */
bool xdmml_mkdir(const char* path);

/**
 * List directory contents
 * @param path Directory path
 * @param count Output for number of entries
 * @return Array of filenames (must be freed)
 */
char** xdmml_list_directory(const char* path, int* count);

/**
 * Free directory listing
 */
void xdmml_free_directory_list(char** list, int count);

#ifdef __cplusplus
}
#endif

#endif // XDMML_EXTENDED_H
