/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for the Linux and macOS operating systems.
 *
 */
/*
 * xly_http.h — Xenly HTTP/1.0 Web Server Library
 *
 * A zero-dependency, single-file HTTP/1.1 web server for Linux and macOS.
 * Designed to be embedded inside Xenly programs via the runtime ABI.
 *
 * Features:
 *   • HTTP/1.0 persistent connections (keep-alive)
 *   • Non-blocking I/O with epoll (Linux) / kqueue (macOS)
 *   • URL routing with pattern matching (:param, *wildcard)
 *   • Chunked and content-length response bodies
 *   • Static file serving with MIME type detection
 *   • Query string and URL-encoded form parsing
 *   • JSON helper (serialize/deserialize via XlyVal*)
 *   • Middleware pipeline (before/after hooks)
 *   • TLS via system OpenSSL/SecureTransport (optional)
 *   • Thread-safe: one accept thread + N worker threads
 *
 * Usage example (C API):
 *
 *   XlyHttpServer *srv = xly_http_create(NULL);
 *   xly_http_route(srv, "GET", "/hello/:name", my_handler, NULL);
 *   xly_http_listen(srv, "0.0.0.0", 8080);
 *   xly_http_run(srv);
 *
 * Usage example (Xenly runtime API via XlyVal*):
 *
 *   XlyVal *srv = xly_http_create_val(NULL);
 *   xly_http_route_val(srv, xly_str("GET"), xly_str("/api/users"), my_handler_val, NULL);
 *   xly_http_listen_val(srv, xly_str("0.0.0.0"), xly_num(8080));
 *   xly_http_run_val(srv);
 */
#ifndef XLY_HTTP_H
#define XLY_HTTP_H

#include <stddef.h>
#include <stdint.h>
/* When included after interpreter.h, use Value* as XlyVal to avoid
 * conflicts between the interpreter and runtime type systems.
 * Both structs have identical memory layout so the ABI is compatible. */
#ifdef INTERPRETER_H
    struct Value;
    typedef struct Value XlyVal;
#else
#  include "xly_rt.h"   /* XlyVal* runtime ABI */
#endif

/* ── Version ─────────────────────────────────────────────────────────────── */
#define XLY_HTTP_VERSION       "1.0.0"
#define XLY_HTTP_VERSION_MAJOR 1
#define XLY_HTTP_VERSION_MINOR 0
#define XLY_HTTP_VERSION_PATCH 0

/* ── Constants ───────────────────────────────────────────────────────────── */
#define XLY_HTTP_MAX_HEADERS      64
#define XLY_HTTP_MAX_PARAMS       32
#define XLY_HTTP_MAX_ROUTES       512
#define XLY_HTTP_MAX_MIDDLEWARE   32
#define XLY_HTTP_BACKLOG          128
#define XLY_HTTP_RECV_BUF         65536  /* 64 KiB read buffer per connection */
#define XLY_HTTP_MAX_BODY         16777216 /* 16 MiB max request body          */
#define XLY_HTTP_DEFAULT_WORKERS  4

/* ── Forward declarations ────────────────────────────────────────────────── */
typedef struct XlyHttpServer  XlyHttpServer;
typedef struct XlyHttpRequest XlyHttpRequest;
typedef struct XlyHttpResponse XlyHttpResponse;
typedef struct XlyHttpCtx     XlyHttpCtx;

/* ── HTTP status codes ───────────────────────────────────────────────────── */
typedef enum {
    XLY_HTTP_200 = 200,  XLY_HTTP_201 = 201,  XLY_HTTP_204 = 204,
    XLY_HTTP_301 = 301,  XLY_HTTP_302 = 302,  XLY_HTTP_304 = 304,
    XLY_HTTP_400 = 400,  XLY_HTTP_401 = 401,  XLY_HTTP_403 = 403,
    XLY_HTTP_404 = 404,  XLY_HTTP_405 = 405,  XLY_HTTP_408 = 408,
    XLY_HTTP_409 = 409,  XLY_HTTP_413 = 413,  XLY_HTTP_415 = 415,
    XLY_HTTP_422 = 422,  XLY_HTTP_429 = 429,
    XLY_HTTP_500 = 500,  XLY_HTTP_502 = 502,  XLY_HTTP_503 = 503,
} XlyHttpStatus;

/* ── Header key-value pair ───────────────────────────────────────────────── */
typedef struct {
    char key[128];
    char value[512];
} XlyHttpHeader;

/* ── URL parameter (e.g. :name in /users/:name) ─────────────────────────── */
typedef struct {
    char key[64];
    char value[256];
} XlyHttpParam;

/* ── HTTP Request ────────────────────────────────────────────────────────── */
struct XlyHttpRequest {
    char          method[16];              /* "GET", "POST", … */
    char          path[1024];              /* URL path (decoded)  */
    char          raw_path[1024];          /* URL path (raw, %xx intact) */
    char          query[2048];             /* raw query string    */
    char          version[16];             /* "HTTP/1.1"          */
    XlyHttpHeader headers[XLY_HTTP_MAX_HEADERS];
    int           n_headers;
    XlyHttpParam  params[XLY_HTTP_MAX_PARAMS];  /* route :params */
    int           n_params;
    char         *body;                    /* request body (heap, may be NULL) */
    size_t        body_len;
    int           keep_alive;             /* 1 if Connection: keep-alive */
    /* Remote address */
    char          remote_addr[48];
    uint16_t      remote_port;
    /* Opaque connection handle (internal) */
    void         *conn;
};

/* ── HTTP Response builder ───────────────────────────────────────────────── */
struct XlyHttpResponse {
    XlyHttpStatus  status;
    XlyHttpHeader  headers[XLY_HTTP_MAX_HEADERS];
    int            n_headers;
    char          *body;          /* heap-allocated response body */
    size_t         body_len;
    int            chunked;       /* 1 = use chunked transfer encoding */
    int            sent;          /* internal: 1 = headers already sent */
};

/* ── Request context (passed to handlers) ────────────────────────────────── */
struct XlyHttpCtx {
    XlyHttpServer   *server;
    XlyHttpRequest  *req;
    XlyHttpResponse *res;
    void            *user;       /* user data registered with the route */
    int              next_called; /* internal: 1 if next() was called */
};

/* ── Handler and middleware function types ───────────────────────────────── */
typedef void (*XlyHttpHandler)   (XlyHttpCtx *ctx);
typedef void (*XlyHttpMiddleware)(XlyHttpCtx *ctx, XlyHttpHandler next_fn);

/* ── Server configuration ────────────────────────────────────────────────── */
typedef struct {
    int    n_workers;        /* worker threads (default: XLY_HTTP_DEFAULT_WORKERS) */
    int    keep_alive_secs;  /* Connection: keep-alive timeout (default: 30) */
    int    max_body_bytes;   /* max request body (default: XLY_HTTP_MAX_BODY) */
    int    verbose;          /* log each request to stderr */
    int    enable_tls;       /* attempt TLS (requires OpenSSL on Linux) */
    char   cert_file[512];   /* path to PEM certificate */
    char   key_file[512];    /* path to PEM private key */
    /* Static file root (empty = disabled) */
    char   static_root[512];
    char   static_prefix[128]; /* URL prefix for static files (default: "/") */
    /* Error handlers */
    XlyHttpHandler on_404;
    XlyHttpHandler on_500;
} XlyHttpConfig;

/* ══════════════════════════════════════════════════════════════════════════════
 * C API
 * ══════════════════════════════════════════════════════════════════════════════ */

/* ── Lifecycle ───────────────────────────────────────────────────────────── */

/*
 * xly_http_create — allocate and initialise a server.
 * cfg may be NULL to use all defaults.
 */
XlyHttpServer *xly_http_create(const XlyHttpConfig *cfg);

/*
 * xly_http_destroy — shut down server and free all resources.
 */
void xly_http_destroy(XlyHttpServer *srv);

/*
 * xly_http_listen — bind to host:port (does NOT block).
 * Returns 0 on success, -1 on error (errno set).
 */
int xly_http_listen(XlyHttpServer *srv, const char *host, uint16_t port);

/*
 * xly_http_run — enter the event loop; blocks until xly_http_stop() is called.
 */
void xly_http_run(XlyHttpServer *srv);

/*
 * xly_http_run_async — start server in background threads; returns immediately.
 */
int xly_http_run_async(XlyHttpServer *srv);

/*
 * xly_http_stop — signal all worker threads to exit gracefully.
 */
void xly_http_stop(XlyHttpServer *srv);

/* ── Routing ─────────────────────────────────────────────────────────────── */

/*
 * xly_http_route — register a handler for method + pattern.
 * Pattern may contain:
 *   :name        — named parameter (any chars except '/')
 *   *            — wildcard (matches rest of path)
 *   /literal     — exact segment match
 * Returns 0 on success, -1 if route table is full.
 */
int xly_http_route(XlyHttpServer *srv, const char *method,
                   const char *pattern, XlyHttpHandler handler, void *user);

/* Shorthand helpers */
int xly_http_get   (XlyHttpServer *srv, const char *pattern, XlyHttpHandler h, void *u);
int xly_http_post  (XlyHttpServer *srv, const char *pattern, XlyHttpHandler h, void *u);
int xly_http_put   (XlyHttpServer *srv, const char *pattern, XlyHttpHandler h, void *u);
int xly_http_delete(XlyHttpServer *srv, const char *pattern, XlyHttpHandler h, void *u);
int xly_http_patch (XlyHttpServer *srv, const char *pattern, XlyHttpHandler h, void *u);

/* ── Middleware ───────────────────────────────────────────────────────────── */

/*
 * xly_http_use — register global middleware (runs before every handler).
 * Middleware is called in registration order.
 */
int xly_http_use(XlyHttpServer *srv, XlyHttpMiddleware mw);

/* ── Request helpers ─────────────────────────────────────────────────────── */

/* Get a request header value by name (case-insensitive); NULL if absent */
const char *xly_http_req_header(const XlyHttpRequest *req, const char *name);

/* Get a URL parameter value by name; NULL if absent */
const char *xly_http_param(const XlyHttpRequest *req, const char *name);

/* Get a query string parameter value by name; NULL if absent */
/* Returns a newly allocated string that the caller must free() */
char *xly_http_query(const XlyHttpRequest *req, const char *name);

/* URL-decode a string into dst (in-place safe: dst == src is allowed) */
void xly_http_urldecode(const char *src, char *dst, size_t dst_size);

/* URL-encode a string into dst */
void xly_http_urlencode(const char *src, char *dst, size_t dst_size);

/* ── Response helpers ────────────────────────────────────────────────────── */

/* Set a response header (overwrites if already present) */
void xly_http_set_header(XlyHttpResponse *res, const char *key, const char *value);

/* Set the Content-Type header */
void xly_http_set_content_type(XlyHttpResponse *res, const char *mime);

/* Send plain-text body */
void xly_http_send_text(XlyHttpCtx *ctx, XlyHttpStatus status, const char *body);

/* Send JSON body (body must be a valid JSON string) */
void xly_http_send_json(XlyHttpCtx *ctx, XlyHttpStatus status, const char *json);

/* Send raw bytes */
void xly_http_send_bytes(XlyHttpCtx *ctx, XlyHttpStatus status,
                          const char *mime, const void *data, size_t len);

/* Send an HTTP redirect */
void xly_http_redirect(XlyHttpCtx *ctx, XlyHttpStatus status, const char *location);

/* Send a built response object (called implicitly by send_* helpers) */
void xly_http_send(XlyHttpCtx *ctx);

/* Write a chunk (chunked transfer encoding must be enabled on the response) */
void xly_http_write_chunk(XlyHttpCtx *ctx, const void *data, size_t len);

/* Finalize a chunked response */
void xly_http_end_chunked(XlyHttpCtx *ctx);

/* ── JSON helpers ────────────────────────────────────────────────────────── */

/* Serialize an XlyVal* to a JSON string (caller must free()) */
char *xly_http_to_json(XlyVal *val);

/* Parse a JSON string to an XlyVal* (returns xly_null() on error) */
XlyVal *xly_http_from_json(const char *json, size_t len);

/* ── MIME type detection ─────────────────────────────────────────────────── */

/* Return MIME type string for a file extension (e.g. ".html" → "text/html") */
const char *xly_http_mime(const char *ext);

/* ── Static file serving ─────────────────────────────────────────────────── */

/* Serve a specific file (ETag + Last-Modified caching handled automatically) */
void xly_http_serve_file(XlyHttpCtx *ctx, const char *filepath);

/* ── XlyVal* Runtime API ─────────────────────────────────────────────────── */
/*
 * These wrap the C API for use from compiled Xenly programs.
 * All string arguments accept VAL_STRING XlyVal*.
 * Numeric arguments accept VAL_NUMBER XlyVal*.
 * Handler is a VAL_FUNCTION XlyVal* with signature fn(ctx_obj).
 */

XlyVal *xly_http_create_val    (XlyVal *cfg_obj);
XlyVal *xly_http_listen_val    (XlyVal *srv, XlyVal *host, XlyVal *port);
XlyVal *xly_http_run_val       (XlyVal *srv);
XlyVal *xly_http_run_async_val (XlyVal *srv);
XlyVal *xly_http_stop_val      (XlyVal *srv);
XlyVal *xly_http_route_val     (XlyVal *srv, XlyVal *method, XlyVal *pattern,
                                 XlyVal *handler, XlyVal *user);
XlyVal *xly_http_get_val       (XlyVal *srv, XlyVal *pattern, XlyVal *handler);
XlyVal *xly_http_post_val      (XlyVal *srv, XlyVal *pattern, XlyVal *handler);
XlyVal *xly_http_put_val       (XlyVal *srv, XlyVal *pattern, XlyVal *handler);
XlyVal *xly_http_delete_val    (XlyVal *srv, XlyVal *pattern, XlyVal *handler);
XlyVal *xly_http_send_text_val (XlyVal *ctx_obj, XlyVal *status, XlyVal *body);
XlyVal *xly_http_send_json_val (XlyVal *ctx_obj, XlyVal *status, XlyVal *json);
XlyVal *xly_http_req_header_val(XlyVal *ctx_obj, XlyVal *name);
XlyVal *xly_http_param_val     (XlyVal *ctx_obj, XlyVal *name);
XlyVal *xly_http_query_val     (XlyVal *ctx_obj, XlyVal *name);
XlyVal *xly_http_to_json_val   (XlyVal *val);
XlyVal *xly_http_from_json_val (XlyVal *str);

#endif /* XLY_HTTP_H */
