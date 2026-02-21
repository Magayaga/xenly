/*
 * xdmml_core.c - XDMML Core Implementation
 */

#include "xdmml.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// ─── Global State ────────────────────────────────────────────────────────────

static struct {
    bool initialized;
    uint32_t init_flags;
    uint64_t start_time;
} g_xdmml = {0};

// ─── Platform-Specific Includes ──────────────────────────────────────────────

#if (defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)) && defined(XDMML_HAS_X11)
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #ifdef XDMML_HAS_OPENGL
        #include <GL/gl.h>
        #include <GL/glx.h>
    #endif
    #include <sys/time.h>
    #include <unistd.h>
    
    static Display* g_display = NULL;
    static int g_screen = 0;
    
#elif defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    // X11 not available — headless/server mode only
    #include <sys/time.h>
    #include <unistd.h>
    
#elif defined(XDMML_PLATFORM_MACOS)
    #include <CoreFoundation/CoreFoundation.h>
    #include <CoreGraphics/CoreGraphics.h>
    #include <mach/mach_time.h>
    // Note: Full Cocoa implementation would need Objective-C
#endif

// ─── Core Functions ──────────────────────────────────────────────────────────

XDMML_Result xdmml_init(uint32_t flags) {
    if (g_xdmml.initialized) {
        return XDMML_ERROR_ALREADY_INITIALIZED;
    }
    
    g_xdmml.init_flags = flags;
    
    // Initialize video subsystem
    if (flags & XDMML_INIT_VIDEO) {
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
        g_display = XOpenDisplay(NULL);
        if (!g_display) {
            fprintf(stderr, "[XDMML] Failed to open X display\n");
            return XDMML_ERROR_INIT;
        }
        g_screen = DefaultScreen(g_display);
#elif defined(XDMML_PLATFORM_MACOS)
        // macOS initialization (would need Cocoa setup)
#endif
    }
    
    // Record start time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    g_xdmml.start_time = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    
    g_xdmml.initialized = true;
    return XDMML_OK;
}

void xdmml_quit(void) {
    if (!g_xdmml.initialized) {
        return;
    }
    
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    if (g_display) {
        XCloseDisplay(g_display);
        g_display = NULL;
    }
#endif
    
    g_xdmml.initialized = false;
}

const char* xdmml_get_version(void) {
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d",
             XDMML_VERSION_MAJOR, XDMML_VERSION_MINOR, XDMML_VERSION_PATCH);
    return version;
}

const char* xdmml_get_error(XDMML_Result result) {
    switch (result) {
        case XDMML_OK: return "Success";
        case XDMML_ERROR: return "Generic error";
        case XDMML_ERROR_INIT: return "Initialization failed";
        case XDMML_ERROR_INVALID_PARAM: return "Invalid parameter";
        case XDMML_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case XDMML_ERROR_NOT_SUPPORTED: return "Not supported";
        case XDMML_ERROR_DEVICE_NOT_FOUND: return "Device not found";
        case XDMML_ERROR_ALREADY_INITIALIZED: return "Already initialized";
        default: return "Unknown error";
    }
}

// ─── Timing Functions ────────────────────────────────────────────────────────

uint64_t xdmml_get_ticks(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t now = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return now - g_xdmml.start_time;
}

void xdmml_delay(uint32_t ms) {
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD) || defined(XDMML_PLATFORM_MACOS)
    usleep(ms * 1000);
#elif defined(XDMML_PLATFORM_WINDOWS)
    Sleep(ms);
#endif
}

// ─── Window Management ───────────────────────────────────────────────────────

struct XDMML_Window {
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    Window xwindow;
#ifdef XDMML_HAS_OPENGL
    GLXContext glx_context;
#else
    void* glx_context;  // Placeholder
#endif
#elif defined(XDMML_PLATFORM_MACOS)
    void* ns_window;  // NSWindow* (opaque)
    void* ns_gl_context;  // NSOpenGLContext* (opaque)
#endif
    int width;
    int height;
    char* title;
    uint32_t flags;
};

XDMML_Window* xdmml_create_window(const char* title, int x, int y,
                                   int width, int height, uint32_t flags) {
    if (!g_xdmml.initialized) {
        return NULL;
    }
    
    XDMML_Window* window = (XDMML_Window*)calloc(1, sizeof(XDMML_Window));
    if (!window) {
        return NULL;
    }
    
    window->width = width;
    window->height = height;
    window->flags = flags;
    window->title = strdup(title ? title : "XDMML Window");
    
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    // X11 window creation
    XSetWindowAttributes swa = {0};
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                     StructureNotifyMask;
    
    if (x == XDMML_WINDOWPOS_CENTERED) {
        x = (DisplayWidth(g_display, g_screen) - width) / 2;
    }
    if (y == XDMML_WINDOWPOS_CENTERED) {
        y = (DisplayHeight(g_display, g_screen) - height) / 2;
    }
    if (x == XDMML_WINDOWPOS_UNDEFINED) x = 100;
    if (y == XDMML_WINDOWPOS_UNDEFINED) y = 100;
    
    window->xwindow = XCreateWindow(g_display, RootWindow(g_display, g_screen),
                                     x, y, width, height, 0,
                                     CopyFromParent, InputOutput, CopyFromParent,
                                     CWEventMask, &swa);
    
    // Set window title
    XStoreName(g_display, window->xwindow, window->title);
    
    // Map window (show it)
    if (!(flags & XDMML_WINDOW_HIDDEN)) {
        XMapWindow(g_display, window->xwindow);
    }
    
    XFlush(g_display);
#elif defined(XDMML_PLATFORM_MACOS)
    // macOS Cocoa window creation (would need Objective-C)
    fprintf(stderr, "[XDMML] macOS window creation not yet implemented\n");
#endif
    
    return window;
}

void xdmml_destroy_window(XDMML_Window* window) {
    if (!window) return;
    
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    if (window->glx_context) {
#ifdef XDMML_HAS_OPENGL
        glXDestroyContext(g_display, window->glx_context);
#endif
    }
    if (window->xwindow) {
        XDestroyWindow(g_display, window->xwindow);
    }
#endif
    
    free(window->title);
    free(window);
}

void xdmml_set_window_title(XDMML_Window* window, const char* title) {
    if (!window || !title) return;
    
    free(window->title);
    window->title = strdup(title);
    
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    XStoreName(g_display, window->xwindow, title);
#endif
}

void xdmml_get_window_size(XDMML_Window* window, int* width, int* height) {
    if (!window) return;
    if (width) *width = window->width;
    if (height) *height = window->height;
}

void xdmml_set_window_size(XDMML_Window* window, int width, int height) {
    if (!window) return;
    
    window->width = width;
    window->height = height;
    
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    XResizeWindow(g_display, window->xwindow, width, height);
#endif
}

void xdmml_show_window(XDMML_Window* window) {
    if (!window) return;
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    XMapWindow(g_display, window->xwindow);
#endif
}

void xdmml_hide_window(XDMML_Window* window) {
    if (!window) return;
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    XUnmapWindow(g_display, window->xwindow);
#endif
}

void xdmml_swap_window(XDMML_Window* window) {
    if (!window) return;
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    if (window->glx_context) {
#ifdef XDMML_HAS_OPENGL
        glXSwapBuffers(g_display, window->xwindow);
#endif
    }
#endif
}

// ─── OpenGL Context ──────────────────────────────────────────────────────────

XDMML_GLContext* xdmml_gl_create_context(XDMML_Window* window) {
    if (!window) return NULL;
    
#if defined(XDMML_PLATFORM_LINUX) && defined(XDMML_HAS_OPENGL)
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    XVisualInfo* vi = glXChooseVisual(g_display, 0, att);
    if (!vi) {
        fprintf(stderr, "[XDMML] No appropriate visual found\n");
        return NULL;
    }
    
    GLXContext glx_ctx = glXCreateContext(g_display, vi, NULL, GL_TRUE);
    XFree(vi);
    
    if (!glx_ctx) {
        fprintf(stderr, "[XDMML] Failed to create GLX context\n");
        return NULL;
    }
    
    window->glx_context = glx_ctx;
    
    // Return a context handle (GLXContext is already a pointer)
    return (XDMML_GLContext*)glx_ctx;
#else
    (void)window;
    fprintf(stderr, "[XDMML] OpenGL not available on this build\n");
    return NULL;
#endif
}

void xdmml_gl_delete_context(XDMML_GLContext* context) {
    if (!context) return;
#if defined(XDMML_PLATFORM_LINUX) && defined(XDMML_HAS_OPENGL)
    glXDestroyContext(g_display, (GLXContext)context);
#else
    (void)context;
#endif
}

void xdmml_gl_make_current(XDMML_Window* window, XDMML_GLContext* context) {
    if (!window) return;
#if defined(XDMML_PLATFORM_LINUX) && defined(XDMML_HAS_OPENGL)
    glXMakeCurrent(g_display, window->xwindow, (GLXContext)context);
#else
    (void)window; (void)context;
#endif
}

void xdmml_gl_set_swap_interval(int interval) {
#if defined(XDMML_PLATFORM_LINUX) && defined(XDMML_HAS_OPENGL)
    // Note: This would need GLX_EXT_swap_control extension
    (void)interval;
#else
    (void)interval;
#endif
}

// ─── Event Handling ──────────────────────────────────────────────────────────

bool xdmml_poll_event(XDMML_Event* event) {
    if (!event) return false;
    
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    if (!g_display) return false;
    
    if (XPending(g_display) == 0) {
        return false;
    }
    
    XEvent xev;
    XNextEvent(g_display, &xev);
    
    event->timestamp = xdmml_get_ticks();
    
    switch (xev.type) {
        case KeyPress:
            event->type = XDMML_EVENT_KEY_DOWN;
            event->key.keycode = xev.xkey.keycode;
            event->key.repeat = false;
            return true;
            
        case KeyRelease:
            event->type = XDMML_EVENT_KEY_UP;
            event->key.keycode = xev.xkey.keycode;
            event->key.repeat = false;
            return true;
            
        case ButtonPress:
            event->type = XDMML_EVENT_MOUSE_BUTTON_DOWN;
            event->button.button = xev.xbutton.button;
            event->button.x = xev.xbutton.x;
            event->button.y = xev.xbutton.y;
            event->button.clicks = 1;
            return true;
            
        case ButtonRelease:
            event->type = XDMML_EVENT_MOUSE_BUTTON_UP;
            event->button.button = xev.xbutton.button;
            event->button.x = xev.xbutton.x;
            event->button.y = xev.xbutton.y;
            return true;
            
        case MotionNotify:
            event->type = XDMML_EVENT_MOUSE_MOTION;
            event->motion.x = xev.xmotion.x;
            event->motion.y = xev.xmotion.y;
            return true;
            
        case ConfigureNotify:
            event->type = XDMML_EVENT_WINDOW_RESIZE;
            event->window_resize.width = xev.xconfigure.width;
            event->window_resize.height = xev.xconfigure.height;
            return true;
            
        case ClientMessage:
            // Could handle WM_DELETE_WINDOW here
            event->type = XDMML_EVENT_WINDOW_CLOSE;
            return true;
    }
#endif
    
    return false;
}

bool xdmml_wait_event(XDMML_Event* event) {
    // Simple implementation: poll with small delay
    while (!xdmml_poll_event(event)) {
        xdmml_delay(10);
    }
    return true;
}
