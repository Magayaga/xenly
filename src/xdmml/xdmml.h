#ifndef XDMML_H
#define XDMML_H

/*
 * ═══════════════════════════════════════════════════════════════════════════
 * XDMML - Xenly Direct Multimedia Library
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 * Cross-platform hardware abstraction layer for multimedia applications
 * 
 * Features:
 *   • Graphics: OpenGL/Metal/Vulkan abstraction
 *   • Audio: Cross-platform audio playback/recording
 *   • Input: Keyboard, mouse, gamepad support
 *   • Window: Platform-agnostic windowing system
 *   • Events: Unified event handling
 * 
 * Supported Platforms:
 *   • Linux (X11, Wayland)
 *   • macOS (Cocoa)
 *   • FreeBSD (X11)
 *   • Windows (Win32 - future)
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ─── Version Information ─────────────────────────────────────────────────────

#define XDMML_VERSION_MAJOR 1
#define XDMML_VERSION_MINOR 0
#define XDMML_VERSION_PATCH 0

// ─── Platform Detection ──────────────────────────────────────────────────────

#if defined(__linux__)
    #define XDMML_PLATFORM_LINUX 1
#elif defined(__APPLE__)
    #define XDMML_PLATFORM_MACOS 1
#elif defined(__FreeBSD__)
    #define XDMML_PLATFORM_FREEBSD 1
#elif defined(_WIN32)
    #define XDMML_PLATFORM_WINDOWS 1
#endif

// ─── Result Codes ────────────────────────────────────────────────────────────

typedef enum {
    XDMML_OK = 0,
    XDMML_ERROR = -1,
    XDMML_ERROR_INIT = -2,
    XDMML_ERROR_INVALID_PARAM = -3,
    XDMML_ERROR_OUT_OF_MEMORY = -4,
    XDMML_ERROR_NOT_SUPPORTED = -5,
    XDMML_ERROR_DEVICE_NOT_FOUND = -6,
    XDMML_ERROR_ALREADY_INITIALIZED = -7,
} XDMML_Result;

// ─── Subsystem Flags ─────────────────────────────────────────────────────────

typedef enum {
    XDMML_INIT_VIDEO       = 0x01,
    XDMML_INIT_AUDIO       = 0x02,
    XDMML_INIT_INPUT       = 0x04,
    XDMML_INIT_GAMEPAD     = 0x08,
    XDMML_INIT_EVERYTHING  = 0xFF,
} XDMML_InitFlags;

// ─── Core Functions ──────────────────────────────────────────────────────────

/**
 * Initialize XDMML subsystems
 * @param flags Combination of XDMML_InitFlags
 * @return XDMML_OK on success
 */
XDMML_Result xdmml_init(uint32_t flags);

/**
 * Shutdown XDMML and free all resources
 */
void xdmml_quit(void);

/**
 * Get version string
 * @return Version string (e.g., "1.0.0")
 */
const char* xdmml_get_version(void);

/**
 * Get error string for result code
 */
const char* xdmml_get_error(XDMML_Result result);

// ═══════════════════════════════════════════════════════════════════════════
// WINDOW MANAGEMENT
// ═══════════════════════════════════════════════════════════════════════════

typedef struct XDMML_Window XDMML_Window;

typedef enum {
    XDMML_WINDOW_OPENGL    = 0x01,
    XDMML_WINDOW_VULKAN    = 0x02,
    XDMML_WINDOW_METAL     = 0x04,
    XDMML_WINDOW_RESIZABLE = 0x10,
    XDMML_WINDOW_FULLSCREEN = 0x20,
    XDMML_WINDOW_BORDERLESS = 0x40,
    XDMML_WINDOW_HIDDEN    = 0x80,
} XDMML_WindowFlags;

/**
 * Create a window
 * @param title Window title
 * @param x X position (XDMML_WINDOWPOS_CENTERED or XDMML_WINDOWPOS_UNDEFINED)
 * @param y Y position
 * @param width Window width
 * @param height Window height
 * @param flags Window flags
 * @return Window handle or NULL on error
 */
XDMML_Window* xdmml_create_window(const char* title, int x, int y, 
                                   int width, int height, uint32_t flags);

/**
 * Destroy a window
 */
void xdmml_destroy_window(XDMML_Window* window);

/**
 * Set window title
 */
void xdmml_set_window_title(XDMML_Window* window, const char* title);

/**
 * Get window size
 */
void xdmml_get_window_size(XDMML_Window* window, int* width, int* height);

/**
 * Set window size
 */
void xdmml_set_window_size(XDMML_Window* window, int width, int height);

/**
 * Show window
 */
void xdmml_show_window(XDMML_Window* window);

/**
 * Hide window
 */
void xdmml_hide_window(XDMML_Window* window);

/**
 * Swap window buffers (for double-buffering)
 */
void xdmml_swap_window(XDMML_Window* window);

#define XDMML_WINDOWPOS_CENTERED   0x2FFF0000
#define XDMML_WINDOWPOS_UNDEFINED  0x1FFF0000

// ═══════════════════════════════════════════════════════════════════════════
// GRAPHICS / RENDERING
// ═══════════════════════════════════════════════════════════════════════════

typedef struct {
    int major;
    int minor;
    int depth_size;
    int stencil_size;
    int multisample_samples;
    bool double_buffer;
} XDMML_GLContext;

/**
 * Create OpenGL context for window
 */
XDMML_GLContext* xdmml_gl_create_context(XDMML_Window* window);

/**
 * Delete OpenGL context
 */
void xdmml_gl_delete_context(XDMML_GLContext* context);

/**
 * Make OpenGL context current
 */
void xdmml_gl_make_current(XDMML_Window* window, XDMML_GLContext* context);

/**
 * Set VSync mode
 * @param interval 0 = immediate, 1 = vsync, -1 = adaptive vsync
 */
void xdmml_gl_set_swap_interval(int interval);

// ═══════════════════════════════════════════════════════════════════════════
// EVENT HANDLING
// ═══════════════════════════════════════════════════════════════════════════

typedef enum {
    XDMML_EVENT_QUIT,
    XDMML_EVENT_WINDOW_CLOSE,
    XDMML_EVENT_WINDOW_RESIZE,
    XDMML_EVENT_KEY_DOWN,
    XDMML_EVENT_KEY_UP,
    XDMML_EVENT_MOUSE_MOTION,
    XDMML_EVENT_MOUSE_BUTTON_DOWN,
    XDMML_EVENT_MOUSE_BUTTON_UP,
    XDMML_EVENT_MOUSE_WHEEL,
} XDMML_EventType;

typedef enum {
    XDMML_KEY_UNKNOWN = 0,
    XDMML_KEY_A = 4,
    XDMML_KEY_B, XDMML_KEY_C, XDMML_KEY_D, XDMML_KEY_E,
    XDMML_KEY_F, XDMML_KEY_G, XDMML_KEY_H, XDMML_KEY_I,
    XDMML_KEY_J, XDMML_KEY_K, XDMML_KEY_L, XDMML_KEY_M,
    XDMML_KEY_N, XDMML_KEY_O, XDMML_KEY_P, XDMML_KEY_Q,
    XDMML_KEY_R, XDMML_KEY_S, XDMML_KEY_T, XDMML_KEY_U,
    XDMML_KEY_V, XDMML_KEY_W, XDMML_KEY_X, XDMML_KEY_Y,
    XDMML_KEY_Z,
    XDMML_KEY_1 = 30,
    XDMML_KEY_2, XDMML_KEY_3, XDMML_KEY_4, XDMML_KEY_5,
    XDMML_KEY_6, XDMML_KEY_7, XDMML_KEY_8, XDMML_KEY_9,
    XDMML_KEY_0,
    XDMML_KEY_RETURN = 40,
    XDMML_KEY_ESCAPE,
    XDMML_KEY_BACKSPACE,
    XDMML_KEY_TAB,
    XDMML_KEY_SPACE,
    XDMML_KEY_LEFT = 80,
    XDMML_KEY_RIGHT,
    XDMML_KEY_UP,
    XDMML_KEY_DOWN,
} XDMML_KeyCode;

typedef enum {
    XDMML_BUTTON_LEFT = 1,
    XDMML_BUTTON_MIDDLE = 2,
    XDMML_BUTTON_RIGHT = 3,
} XDMML_MouseButton;

typedef struct {
    XDMML_EventType type;
    uint32_t timestamp;
    
    union {
        struct {
            XDMML_Window* window;
            int width;
            int height;
        } window_resize;
        
        struct {
            XDMML_KeyCode keycode;
            bool repeat;
        } key;
        
        struct {
            int x, y;
            int xrel, yrel;
        } motion;
        
        struct {
            XDMML_MouseButton button;
            int x, y;
            uint8_t clicks;
        } button;
        
        struct {
            int x, y;
        } wheel;
    };
} XDMML_Event;

/**
 * Poll for pending events
 * @param event Event structure to fill
 * @return true if event was retrieved, false if queue is empty
 */
bool xdmml_poll_event(XDMML_Event* event);

/**
 * Wait for an event (blocking)
 */
bool xdmml_wait_event(XDMML_Event* event);

// ═══════════════════════════════════════════════════════════════════════════
// AUDIO
// ═══════════════════════════════════════════════════════════════════════════

typedef struct XDMML_Audio XDMML_Audio;

typedef struct {
    int frequency;      // Sample rate (e.g., 44100)
    uint16_t format;    // Audio format
    uint8_t channels;   // 1 = mono, 2 = stereo
    uint16_t samples;   // Buffer size
} XDMML_AudioSpec;

#define XDMML_AUDIO_S16 0x8010  // Signed 16-bit
#define XDMML_AUDIO_F32 0x8120  // Float 32-bit

/**
 * Open audio device
 */
XDMML_Audio* xdmml_open_audio(XDMML_AudioSpec* desired, XDMML_AudioSpec* obtained);

/**
 * Close audio device
 */
void xdmml_close_audio(XDMML_Audio* audio);

/**
 * Play audio
 */
void xdmml_play_audio(XDMML_Audio* audio);

/**
 * Pause audio
 */
void xdmml_pause_audio(XDMML_Audio* audio);

/**
 * Load WAV file
 * @param filename Path to WAV file
 * @param spec Audio specification (output)
 * @param buffer Audio data buffer (output, must be freed)
 * @param length Buffer length (output)
 * @return XDMML_OK on success
 */
XDMML_Result xdmml_load_wav(const char* filename, XDMML_AudioSpec* spec,
                             uint8_t** buffer, uint32_t* length);

/**
 * Free audio buffer
 */
void xdmml_free_wav(uint8_t* buffer);

// ═══════════════════════════════════════════════════════════════════════════
// TIMING
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Get time in milliseconds since init
 */
uint64_t xdmml_get_ticks(void);

/**
 * Delay for specified milliseconds
 */
void xdmml_delay(uint32_t ms);

// ═══════════════════════════════════════════════════════════════════════════
// INPUT STATE
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Get keyboard state
 * @param numkeys Output for number of keys
 * @return Array of key states (1 = pressed, 0 = not pressed)
 */
const uint8_t* xdmml_get_keyboard_state(int* numkeys);

/**
 * Get mouse state
 * @param x Output for X position
 * @param y Output for Y position
 * @return Button state mask
 */
uint32_t xdmml_get_mouse_state(int* x, int* y);

#ifdef __cplusplus
}
#endif

#endif // XDMML_H
