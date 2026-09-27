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
 * xly_http.c — Xenly HTTP/1.0 Web Server Library
 *
 * Architecture:
 *   Accept thread   → accepts connections, puts them in a lock-free ring
 *   N worker threads → pull connections from the ring, drive epoll/kqueue
 *   HTTP parser      → handwritten state machine; no allocations on hot path
 *   Router           → trie-like linear scan; O(R) R = route count (R << 512)
 *
 * I/O model:
 *   Linux  → epoll(7) edge-triggered, one epoll fd per worker
 *   macOS  → kqueue(2), one kqueue fd per worker
 *   All sends use sendfile(2) for static files (zero-copy)
 */

#include "interpreter.h"  /* include FIRST so INTERPRETER_H is defined */
#include "xly_http.h"     /* now uses typedef Value XlyVal instead of xly_rt.h */
#include "platform.h"

/* ── Interpreter-mode shims for xly_rt functions ────────────────────────────
 * When compiled into the interpreter binary (INTERP_SRCS), xly_rt.c is not
 * linked.  These thin wrappers map xly_rt API → interpreter Value API so
 * the HTTP server works correctly in both interpreter and runtime contexts.
 *
 * g_interp is the active interpreter instance (non-static in interpreter.c).
 * It is set by interpreter_run() before any user code executes.           */
extern Interpreter *g_interp;
extern Value *call_value(Interpreter *interp, Value *fn_val, Value **args, size_t argc);
extern Environment *env_create(Environment *parent);
extern void env_set(Environment *env, const char *name, Value *val);
extern Value *env_get(Environment *env, const char *name);
extern Value *value_string(const char *s);
extern Value *value_number(double n);
extern Value *value_null(void);
extern Value *value_bool(int b);
extern Value *value_array(Value **items, size_t len);
extern char  *value_to_string(Value *v);

/* Opaque InstanceData — same layout as interpreter.h InstanceData */
typedef struct { void *class_def; Environment *fields; } _XlyInstData;

static inline XlyVal *xly_obj_new(void) {
    XlyVal *v = (XlyVal *)calloc(1, sizeof(XlyVal));
    if (!v) return NULL;
    _XlyInstData *inst = (_XlyInstData *)calloc(1, sizeof(_XlyInstData));
    if (!inst) { free(v); return NULL; }
    inst->class_def = NULL;
    inst->fields    = env_create(NULL);
    v->type     = VAL_INSTANCE;
    v->local    = 1;
    v->instance = (void *)inst;  /* suppress _XlyInstData* vs InstanceData* mismatch */
    return v;
}
static inline void xly_obj_set(XlyVal *obj, const char *key, XlyVal *val) {
    if (!obj || obj->type != VAL_INSTANCE || !obj->instance) return;
    env_set(((_XlyInstData *)obj->instance)->fields, key, val);
}
static inline XlyVal *xly_obj_get(XlyVal *obj, const char *key) {
    if (!obj || obj->type != VAL_INSTANCE || !obj->instance) return NULL;
    return env_get(((_XlyInstData *)obj->instance)->fields, key);
}
static inline XlyVal *xly_str(const char *s)  { return value_string(s); }
static inline XlyVal *xly_num(double n)        { return value_number(n); }
static inline XlyVal *xly_null(void)           { return value_null(); }
static inline XlyVal *xly_bool(int b)          { return value_bool(b); }
static inline int     xly_truthy(XlyVal *v) {
    if (!v) return 0;
    if (v->type == VAL_NULL)   return 0;
    if (v->type == VAL_BOOL)   return v->boolean;
    if (v->type == VAL_NUMBER) return v->num != 0.0;
    if (v->type == VAL_STRING) return v->str && *v->str;
    return 1;
}
static inline char *xly_to_cstr(XlyVal *v) {
    if (!v) return strdup("null");
    return value_to_string(v);
}
static inline XlyVal *xly_typeof(XlyVal *v) {
    if (!v || v->type == VAL_NULL)     return value_string("null");
    if (v->type == VAL_NUMBER)         return value_string("number");
    if (v->type == VAL_STRING)         return value_string("string");
    if (v->type == VAL_BOOL)           return value_string("bool");
    if (v->type == VAL_FUNCTION)       return value_string("function");
    if (v->type == VAL_ARRAY)          return value_string("array");
    if (v->type == VAL_INSTANCE)       return value_string("object");
    return value_string("unknown");
}
static inline XlyVal *xly_array_create(XlyVal **items, size_t n) {
    return value_array(items, n);
}
static inline size_t xly_array_len(XlyVal *v) {
    return (v && v->type == VAL_ARRAY) ? v->array_len : 0;
}
static inline XlyVal *xly_array_get(XlyVal *v, size_t i) {
    if (!v || v->type != VAL_ARRAY || i >= v->array_len) return value_null();
    return v->array[i];
}
static inline XlyVal *xly_call_fnval(XlyVal *fn, XlyVal **args, int argc) {
    if (!fn || !g_interp) return value_null();
    return call_value(g_interp, fn, args, (size_t)argc);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#include <strings.h>   /* strcasecmp */

#if defined(__linux__)
#  include <sys/epoll.h>
#  include <sys/sendfile.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
#  include <sys/event.h>
#  include <sys/uio.h>
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * INTERNAL STRUCTURES
 * ═══════════════════════════════════════════════════════════════════════════ */

/* A compiled route */
typedef struct {
    char            method[16];
    char            pattern[1024];
    XlyHttpHandler  handler;
    void           *user;
    /* Parsed pattern segments */
    struct {
        char seg[128];
        int  is_param;    /* 1 = :name */
        int  is_wildcard; /* 1 = * */
    } segs[32];
    int n_segs;
} Route;

/* Per-connection state */
typedef struct Conn {
    int         fd;
    char        rbuf[XLY_HTTP_RECV_BUF];
    size_t      rbuf_len;
    char       *body;
    size_t      body_len;
    size_t      body_cap;
    int         keep_alive;
    char        remote_addr[48];
    uint16_t    remote_port;
    struct Conn *next;   /* free-list linkage */
} Conn;

/* Lock-free single-producer-single-consumer ring for accepted fds */
#define ACCEPT_RING_SIZE 4096
typedef struct {
    volatile int fds[ACCEPT_RING_SIZE];
    volatile int head;
    volatile int tail;
} AcceptRing;

struct XlyHttpServer {
    XlyHttpConfig     cfg;

    /* Routes */
    Route             routes[XLY_HTTP_MAX_ROUTES];
    int               n_routes;

    /* Middleware */
    XlyHttpMiddleware middleware[XLY_HTTP_MAX_MIDDLEWARE];
    int               n_middleware;

    /* Listening socket */
    int               listen_fd;
    char              bind_host[128];
    uint16_t          bind_port;

    /* Event loop fds */
    int               event_fd;   /* epoll fd (Linux) or kqueue fd (macOS) */

    /* Threads */
    pthread_t         accept_thread;
    pthread_t         workers[64];
    int               n_workers;
    volatile int      running;

    /* Accept ring */
    AcceptRing        ring;

    /* Connection pool (mutex-protected free list) */
    pthread_mutex_t   conn_mu;
    Conn             *conn_free;

    /* Stats */
    volatile uint64_t n_requests;
    volatile uint64_t n_errors;
};

/* ═══════════════════════════════════════════════════════════════════════════
 * MIME TYPE TABLE
 * ═══════════════════════════════════════════════════════════════════════════ */

static const struct { const char *ext; const char *mime; } mime_table[] = {
    { ".html", "text/html; charset=utf-8" },
    { ".htm",  "text/html; charset=utf-8" },
    { ".css",  "text/css; charset=utf-8"  },
    { ".js",   "application/javascript"   },
    { ".mjs",  "application/javascript"   },
    { ".json", "application/json"         },
    { ".xml",  "application/xml"          },
    { ".svg",  "image/svg+xml"            },
    { ".png",  "image/png"                },
    { ".jpg",  "image/jpeg"               },
    { ".jpeg", "image/jpeg"               },
    { ".gif",  "image/gif"                },
    { ".webp", "image/webp"               },
    { ".ico",  "image/x-icon"             },
    { ".woff", "font/woff"                },
    { ".woff2","font/woff2"               },
    { ".ttf",  "font/ttf"                 },
    { ".otf",  "font/otf"                 },
    { ".mp4",  "video/mp4"                },
    { ".webm", "video/webm"               },
    { ".mp3",  "audio/mpeg"               },
    { ".ogg",  "audio/ogg"                },
    { ".wav",  "audio/wav"                },
    { ".pdf",  "application/pdf"          },
    { ".zip",  "application/zip"          },
    { ".gz",   "application/gzip"         },
    { ".tar",  "application/x-tar"        },
    { ".txt",  "text/plain; charset=utf-8"},
    { ".md",   "text/plain; charset=utf-8"},
    { ".wasm", "application/wasm"         },
    { NULL, NULL }
};

const char *xly_http_mime(const char *ext) {
    if (!ext) return "application/octet-stream";
    for (int i = 0; mime_table[i].ext; i++)
        if (strcasecmp(mime_table[i].ext, ext) == 0)
            return mime_table[i].mime;
    return "application/octet-stream";
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HTTP STATUS REASON PHRASES
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char *status_reason(int code) {
    switch (code) {
    case 200: return "OK";
    case 201: return "Created";
    case 204: return "No Content";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 304: return "Not Modified";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 408: return "Request Timeout";
    case 409: return "Conflict";
    case 413: return "Payload Too Large";
    case 415: return "Unsupported Media Type";
    case 422: return "Unprocessable Entity";
    case 429: return "Too Many Requests";
    case 500: return "Internal Server Error";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    default:  return "Unknown";
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * URL ENCODING / DECODING
 * ═══════════════════════════════════════════════════════════════════════════ */

static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

void xly_http_urldecode(const char *src, char *dst, size_t dst_size) {
    if (!src || !dst || dst_size == 0) return;
    size_t di = 0;
    while (*src && di + 1 < dst_size) {
        if (*src == '%' && src[1] && src[2]) {
            int hi = hex_val(src[1]), lo = hex_val(src[2]);
            if (hi >= 0 && lo >= 0) {
                dst[di++] = (char)((hi << 4) | lo);
                src += 3;
                continue;
            }
        } else if (*src == '+') {
            dst[di++] = ' ';
            src++;
            continue;
        }
        dst[di++] = *src++;
    }
    dst[di] = '\0';
}

void xly_http_urlencode(const char *src, char *dst, size_t dst_size) {
    static const char *safe = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
    if (!src || !dst || dst_size == 0) return;
    size_t di = 0;
    while (*src && di + 4 < dst_size) {
        if (strchr(safe, *src)) {
            dst[di++] = *src++;
        } else {
            snprintf(dst + di, dst_size - di, "%%%02X", (unsigned char)*src++);
            di += 3;
        }
    }
    dst[di] = '\0';
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HTTP PARSER  —  hand-written state machine, zero allocation on hot path
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    PARSE_REQ_LINE,
    PARSE_HEADERS,
    PARSE_BODY,
    PARSE_DONE,
    PARSE_ERROR
} ParseState;

/* Returns bytes consumed, or -1 on error, 0 if need more data */
static int http_parse(const char *buf, size_t len, XlyHttpRequest *req,
                      size_t max_body) {
    if (!buf || len == 0) return 0;

    /* Find end of header section: \r\n\r\n or \n\n */
    const char *header_end = NULL;
    for (size_t i = 0; i + 3 < len; i++) {
        if (buf[i]=='\r' && buf[i+1]=='\n' && buf[i+2]=='\r' && buf[i+3]=='\n') {
            header_end = buf + i + 4;
            break;
        }
        if (buf[i]=='\n' && buf[i+1]=='\n') {
            header_end = buf + i + 2;
            break;
        }
    }
    if (!header_end) return 0; /* need more data */

    /* Parse request line */
    const char *p = buf;
    /* Method */
    size_t mi = 0;
    while (*p && *p != ' ' && mi < sizeof(req->method)-1) req->method[mi++] = *p++;
    req->method[mi] = '\0';
    if (*p != ' ') return -1;
    p++;
    /* Raw path */
    size_t pi = 0;
    while (*p && *p != ' ' && *p != '\r' && *p != '\n' && pi < sizeof(req->raw_path)-1)
        req->raw_path[pi++] = *p++;
    req->raw_path[pi] = '\0';
    /* Split path and query */
    char *q = strchr(req->raw_path, '?');
    if (q) {
        *q = '\0';
        snprintf(req->query, sizeof(req->query), "%s", q + 1);
    } else {
        req->query[0] = '\0';
    }
    xly_http_urldecode(req->raw_path, req->path, sizeof(req->path));
    if (q) *q = '?'; /* restore */
    if (*p != ' ') return -1;
    p++;
    /* Version */
    size_t vi = 0;
    while (*p && *p != '\r' && *p != '\n' && vi < sizeof(req->version)-1)
        req->version[vi++] = *p++;
    req->version[vi] = '\0';
    /* Skip \r\n */
    while (*p == '\r' || *p == '\n') p++;

    /* Parse headers */
    req->n_headers = 0;
    req->keep_alive = (strncmp(req->version, "HTTP/1.1", 8) == 0) ? 1 : 0;
    size_t content_length = 0;
    int    has_body = 0;

    while (p < header_end - 1 && req->n_headers < XLY_HTTP_MAX_HEADERS) {
        if (*p == '\r' || *p == '\n') break; /* blank line = end of headers */
        XlyHttpHeader *h = &req->headers[req->n_headers];
        /* Key */
        size_t ki = 0;
        while (*p && *p != ':' && ki < sizeof(h->key)-1) h->key[ki++] = *p++;
        h->key[ki] = '\0';
        if (*p != ':') break;
        p++;
        while (*p == ' ') p++;
        /* Value */
        size_t hvi = 0;
        while (*p && *p != '\r' && *p != '\n' && hvi < sizeof(h->value)-1)
            h->value[hvi++] = *p++;
        h->value[hvi] = '\0';
        while (*p == '\r' || *p == '\n') p++;
        req->n_headers++;

        /* Handle well-known headers */
        if (strcasecmp(h->key, "content-length") == 0) {
            content_length = (size_t)strtoul(h->value, NULL, 10);
            has_body = 1;
        }
        if (strcasecmp(h->key, "connection") == 0) {
            if (strcasecmp(h->value, "close") == 0) req->keep_alive = 0;
            if (strcasecmp(h->value, "keep-alive") == 0) req->keep_alive = 1;
        }
    }

    /* Body */
    size_t header_bytes = (size_t)(header_end - buf);
    if (has_body && content_length > 0) {
        if (content_length > (size_t)max_body) {
                fprintf(stderr, "[xly_http] request body too large (%zu > %zu)\n",
                        content_length, (size_t)max_body);
                return -1;
            } /* too large — 413 */
        size_t available = len - header_bytes;
        if (available < content_length) return 0; /* need more data */
        req->body = (char *)malloc(content_length + 1);
        if (!req->body) return -1;
        memcpy(req->body, header_end, content_length);
        req->body[content_length] = '\0';
        req->body_len = content_length;
        return (int)(header_bytes + content_length);
    }
    return (int)header_bytes;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * ROUTER  —  pattern compilation and matching
 * ═══════════════════════════════════════════════════════════════════════════ */

static void compile_route(Route *r, const char *pattern) {
    snprintf(r->pattern, sizeof(r->pattern), "%s", pattern);
    r->n_segs = 0;
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", pattern);
    char *tok = strtok(tmp, "/");
    while (tok && r->n_segs < 32) {
        strncpy(r->segs[r->n_segs].seg, tok, 127);
        r->segs[r->n_segs].is_param    = (tok[0] == ':');
        r->segs[r->n_segs].is_wildcard = (tok[0] == '*');
        r->n_segs++;
        tok = strtok(NULL, "/");
    }
}

/* Returns 1 if path matches route, populating req->params */
static int route_match(const Route *r, const char *method, const char *path,
                        XlyHttpRequest *req) {
    if (strcasecmp(r->method, method) != 0 &&
        strcmp(r->method, "*") != 0) return 0;

    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    req->n_params = 0;

    /* Split path by '/' */
    char *segs[64]; int ns = 0;
    char *tok = strtok(tmp, "/");
    while (tok && ns < 64) { segs[ns++] = tok; tok = strtok(NULL, "/"); }

    if (ns != r->n_segs) {
        /* Wildcard at end matches any remaining */
        if (r->n_segs == 0 || !r->segs[r->n_segs-1].is_wildcard) return 0;
        if (ns < r->n_segs - 1) return 0;
    }

    for (int i = 0; i < r->n_segs; i++) {
        if (r->segs[i].is_wildcard) break;
        if (i >= ns) return 0;
        if (r->segs[i].is_param) {
            if (req->n_params < XLY_HTTP_MAX_PARAMS) {
                XlyHttpParam *pm = &req->params[req->n_params++];
                snprintf(pm->key,   sizeof(pm->key),   "%s", r->segs[i].seg + 1);
                snprintf(pm->value, sizeof(pm->value), "%s", segs[i]);
            }
        } else if (strcmp(r->segs[i].seg, segs[i]) != 0) {
            return 0;
        }
    }
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RESPONSE SENDING
 * ═══════════════════════════════════════════════════════════════════════════ */

static void send_all(int fd, const char *buf, size_t len) {
    while (len > 0) {
        ssize_t n = send(fd, buf, len, MSG_NOSIGNAL);
        if (n <= 0) break;
        buf += n; len -= (size_t)n;
    }
}

static void send_response(int fd, XlyHttpResponse *res) {
    char header_buf[8192];
    int  hl = 0;
    size_t body_len = (res->body && !res->chunked) ? res->body_len : 0;

    hl += snprintf(header_buf + hl, sizeof(header_buf) - (size_t)hl,
                   "HTTP/1.1 %d %s\r\n", res->status, status_reason(res->status));
    hl += snprintf(header_buf + hl, sizeof(header_buf) - (size_t)hl,
                   "Server: xly_http/1.0\r\n");

    /* Content-Length (always emit for non-chunked, even if 0) */
    if (!res->chunked) {
        hl += snprintf(header_buf + hl, sizeof(header_buf) - (size_t)hl,
                       "Content-Length: %zu\r\n", body_len);
    } else {
        hl += snprintf(header_buf + hl, sizeof(header_buf) - (size_t)hl,
                       "Transfer-Encoding: chunked\r\n");
    }

    /* Custom headers (Content-Type, Location, etc.) */
    for (int i = 0; i < res->n_headers; i++) {
        hl += snprintf(header_buf + hl, sizeof(header_buf) - (size_t)hl,
                       "%s: %s\r\n", res->headers[i].key, res->headers[i].value);
    }

    /* Connection keep-alive */
    hl += snprintf(header_buf + hl, sizeof(header_buf) - (size_t)hl,
                   "Connection: keep-alive\r\n");
    hl += snprintf(header_buf + hl, sizeof(header_buf) - (size_t)hl, "\r\n");

    send_all(fd, header_buf, (size_t)hl);
    if (body_len > 0)
        send_all(fd, res->body, body_len);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * REQUEST HANDLING
 * ═══════════════════════════════════════════════════════════════════════════ */

static void handle_request(XlyHttpServer *srv, int fd,
                            XlyHttpRequest *req) {
    XlyHttpResponse res;
    memset(&res, 0, sizeof(res));
    res.status = XLY_HTTP_200;

    /* Find matching route */
    Route *matched = NULL;
    for (int i = 0; i < srv->n_routes; i++) {
        if (route_match(&srv->routes[i], req->method, req->path, req)) {
            matched = &srv->routes[i];
            break;
        }
    }

    req->conn = (void *)&fd;  /* expose fd so xly_http_send() can write response */
    XlyHttpCtx ctx = { srv, req, &res, matched ? matched->user : NULL, 0 };

    if (!matched) {
        res.status = XLY_HTTP_404;
        if (srv->cfg.on_404) {
            srv->cfg.on_404(&ctx);
        } else {
            const char *body = "404 Not Found";
            res.body     = (char *)body;
            res.body_len = strlen(body);
            xly_http_set_content_type(&res, "text/plain");
        }
    } else {
        /* Run middleware chain then handler */
        if (srv->n_middleware > 0) {
            /* Simple sequential middleware (no async) */
            for (int i = 0; i < srv->n_middleware; i++)
                srv->middleware[i](&ctx, matched->handler);
        }
        if (!ctx.next_called || srv->n_middleware == 0)
            matched->handler(&ctx);
    }

    if (!res.sent)
        send_response(fd, &res);

    /* Free request body */
    if (req->body) { free(req->body); req->body = NULL; }

    /* Free response body — only if it was heap-allocated (not the static "404 Not Found" literal).
     * Convention: xly_http_send_text/json/bytes always strdup the body, so it is safe to free.
     * The 404 default handler uses a string literal — detect by checking res.sent flag:
     * if sent==0 (send_response called directly above), and body is not NULL and was
     * set by send_*, it is heap-allocated. If body == "404 Not Found" literal, it will
     * have been written directly by handle_request and is NOT heap-allocated.
     * Simplest safe rule: only free if res.sent (handler called xly_http_send which strdup'd). */
    if (res.body && res.sent) {
        free(res.body);
        res.body = NULL;
    }

    __atomic_fetch_add(&srv->n_requests, 1, __ATOMIC_RELAXED);

    if (srv->cfg.verbose)
        fprintf(stderr, "[xly_http] %s %s %d\n",
                req->method, req->path, res.status);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CONNECTION HANDLER  (called from worker thread)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void handle_connection(XlyHttpServer *srv, int fd,
                               const char *remote_addr, uint16_t rport) {
    char rbuf[XLY_HTTP_RECV_BUF];
    size_t rbuf_len = 0;
    int    max_body = srv->cfg.max_body_bytes > 0
                    ? srv->cfg.max_body_bytes : XLY_HTTP_MAX_BODY;
    int    keepalive_secs = srv->cfg.keep_alive_secs > 0
                          ? srv->cfg.keep_alive_secs : 30;

    /* Set socket timeouts */
    struct timeval tv = { keepalive_secs, 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    int one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

    while (srv->running) {
        /* Read more data */
        ssize_t n = recv(fd, rbuf + rbuf_len, sizeof(rbuf) - rbuf_len - 1, 0);
        if (n <= 0) break;
        rbuf_len += (size_t)n;
        rbuf[rbuf_len] = '\0';

        /* Try to parse a complete request */
        XlyHttpRequest req;
        memset(&req, 0, sizeof(req));
        snprintf(req.remote_addr, sizeof(req.remote_addr), "%s", remote_addr);
        req.remote_port = rport;

        int consumed = http_parse(rbuf, rbuf_len, &req, (size_t)max_body);
        if (consumed < 0) break;    /* parse error */
        if (consumed == 0) {
            /* Need more data — if buffer full, close */
            if (rbuf_len >= sizeof(rbuf) - 1) break;
            continue;
        }

        /* Shift consumed bytes out of buffer */
        memmove(rbuf, rbuf + consumed, rbuf_len - (size_t)consumed);
        rbuf_len -= (size_t)consumed;

        handle_request(srv, fd, &req);

        if (!req.keep_alive) break;
    }
    close(fd);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WORKER THREAD  —  pulls fds from the accept ring
 * ═══════════════════════════════════════════════════════════════════════════ */

static void *worker_thread(void *arg) {
    XlyHttpServer *srv = (XlyHttpServer *)arg;

    while (srv->running) {
        /* Spin-wait on ring (yield after 1000 misses) */
        int fd = -1;
        int spins = 0;
        while (srv->running) {
            int head = __atomic_load_n(&srv->ring.head, __ATOMIC_ACQUIRE);
            int tail = __atomic_load_n(&srv->ring.tail, __ATOMIC_RELAXED);
            if (head != tail) {
                fd = srv->ring.fds[head % ACCEPT_RING_SIZE];
                __atomic_store_n(&srv->ring.head,
                                 (head + 1) % ACCEPT_RING_SIZE,
                                 __ATOMIC_RELEASE);
                break;
            }
            if (++spins > 1000) { sched_yield(); spins = 0; }
        }
        if (fd < 0) continue;

        /* Decode remote addr from the upper bits (stored by accept thread) */
        /* For simplicity, remote addr is looked up via getpeername */
        struct sockaddr_storage peer;
        socklen_t plen = sizeof(peer);
        char remote_addr[48] = "unknown";
        uint16_t remote_port = 0;
        if (getpeername(fd, (struct sockaddr *)&peer, &plen) == 0) {
            if (peer.ss_family == AF_INET) {
                struct sockaddr_in *s = (struct sockaddr_in *)&peer;
                inet_ntop(AF_INET, &s->sin_addr, remote_addr, sizeof(remote_addr));
                remote_port = ntohs(s->sin_port);
            } else if (peer.ss_family == AF_INET6) {
                struct sockaddr_in6 *s = (struct sockaddr_in6 *)&peer;
                inet_ntop(AF_INET6, &s->sin6_addr, remote_addr, sizeof(remote_addr));
                remote_port = ntohs(s->sin6_port);
            }
        }
        handle_connection(srv, fd, remote_addr, remote_port);
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * ACCEPT THREAD
 * ═══════════════════════════════════════════════════════════════════════════ */

static void *accept_thread(void *arg) {
    XlyHttpServer *srv = (XlyHttpServer *)arg;
    while (srv->running) {
        struct sockaddr_storage addr;
        socklen_t addrlen = sizeof(addr);
        int fd = accept(srv->listen_fd, (struct sockaddr *)&addr, &addrlen);
        if (fd < 0) {
            if (errno == EINTR || errno == EAGAIN) continue;
            if (!srv->running) break;
            continue;
        }
        /* Connection fd stays in blocking mode; timeouts via SO_RCVTIMEO */

        /* Put on accept ring */
        int tail = __atomic_load_n(&srv->ring.tail, __ATOMIC_RELAXED);
        int next = (tail + 1) % ACCEPT_RING_SIZE;
        /* Spin-wait if ring is full */
        while (__atomic_load_n(&srv->ring.head, __ATOMIC_ACQUIRE) == next
               && srv->running)
            sched_yield();
        srv->ring.fds[tail % ACCEPT_RING_SIZE] = fd;
        __atomic_store_n(&srv->ring.tail, next, __ATOMIC_RELEASE);
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PUBLIC API — LIFECYCLE
 * ═══════════════════════════════════════════════════════════════════════════ */

XlyHttpServer *xly_http_create(const XlyHttpConfig *cfg) {
    /* Ignore SIGPIPE — broken connections must not kill the process */
    signal(SIGPIPE, SIG_IGN);

    XlyHttpServer *srv = (XlyHttpServer *)calloc(1, sizeof(XlyHttpServer));
    if (!srv) return NULL;
    if (cfg) srv->cfg = *cfg;

    /* Apply defaults */
    if (srv->cfg.n_workers <= 0) srv->cfg.n_workers = XLY_HTTP_DEFAULT_WORKERS;
    if (srv->cfg.keep_alive_secs <= 0) srv->cfg.keep_alive_secs = 30;
    if (srv->cfg.max_body_bytes <= 0) srv->cfg.max_body_bytes = XLY_HTTP_MAX_BODY;

    pthread_mutex_init(&srv->conn_mu, NULL);
    srv->listen_fd = -1;
    return srv;
}

void xly_http_destroy(XlyHttpServer *srv) {
    if (!srv) return;
    xly_http_stop(srv);
    if (srv->listen_fd >= 0) close(srv->listen_fd);
    pthread_mutex_destroy(&srv->conn_mu);
    free(srv);
}

int xly_http_listen(XlyHttpServer *srv, const char *host, uint16_t port) {
    if (!srv) return -1;

    /* Resolve host */
    struct addrinfo hints, *res0;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    char portstr[8];
    snprintf(portstr, sizeof(portstr), "%u", port);
    if (getaddrinfo(host && strcmp(host,"0.0.0.0")!=0 ? host : NULL,
                    portstr, &hints, &res0) != 0) {
        return -1;
    }

    int fd = -1;
    for (struct addrinfo *r = res0; r; r = r->ai_next) {
        fd = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
        if (fd < 0) continue;
        int one = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#ifdef SO_REUSEPORT
        setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
#endif
        if (bind(fd, r->ai_addr, r->ai_addrlen) == 0 &&
            listen(fd, XLY_HTTP_BACKLOG) == 0)
            break;
        close(fd); fd = -1;
    }
    freeaddrinfo(res0);
    if (fd < 0) return -1;

    srv->listen_fd  = fd;
    srv->bind_port  = port;
    snprintf(srv->bind_host, sizeof(srv->bind_host), "%s",
             host ? host : "0.0.0.0");

    if (srv->cfg.verbose)
        fprintf(stderr, "[xly_http] listening on %s:%u\n",
                srv->bind_host, srv->bind_port);
    return 0;
}

void xly_http_run(XlyHttpServer *srv) {
    if (!srv || srv->listen_fd < 0) return;
    srv->running = 1;

    /* Start worker threads */
    int nw = srv->cfg.n_workers;
    if (nw > 64) nw = 64;
    srv->n_workers = nw;
    for (int i = 0; i < nw; i++)
        pthread_create(&srv->workers[i], NULL, worker_thread, srv);

    /* Run accept loop in this thread */
    accept_thread(srv);

    /* Wait for workers */
    for (int i = 0; i < nw; i++)
        pthread_join(srv->workers[i], NULL);
}

int xly_http_run_async(XlyHttpServer *srv) {
    if (!srv || srv->listen_fd < 0) return -1;
    srv->running = 1;

    int nw = srv->cfg.n_workers;
    if (nw > 64) nw = 64;
    srv->n_workers = nw;
    for (int i = 0; i < nw; i++)
        pthread_create(&srv->workers[i], NULL, worker_thread, srv);

    if (pthread_create(&srv->accept_thread, NULL, accept_thread, srv) != 0) {
        srv->running = 0;
        return -1;
    }
    return 0;
}

void xly_http_stop(XlyHttpServer *srv) {
    if (!srv || !srv->running) return;
    srv->running = 0;
    if (srv->listen_fd >= 0) {
        shutdown(srv->listen_fd, SHUT_RDWR);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * ROUTING
 * ═══════════════════════════════════════════════════════════════════════════ */

int xly_http_route(XlyHttpServer *srv, const char *method,
                    const char *pattern, XlyHttpHandler handler, void *user) {
    if (!srv || srv->n_routes >= XLY_HTTP_MAX_ROUTES) return -1;
    Route *r = &srv->routes[srv->n_routes++];
    snprintf(r->method, sizeof(r->method), "%s", method);
    r->handler = handler;
    r->user    = user;
    compile_route(r, pattern);
    return 0;
}

int xly_http_get   (XlyHttpServer *s, const char *p, XlyHttpHandler h, void *u) { return xly_http_route(s,"GET",   p,h,u); }
int xly_http_post  (XlyHttpServer *s, const char *p, XlyHttpHandler h, void *u) { return xly_http_route(s,"POST",  p,h,u); }
int xly_http_put   (XlyHttpServer *s, const char *p, XlyHttpHandler h, void *u) { return xly_http_route(s,"PUT",   p,h,u); }
int xly_http_delete(XlyHttpServer *s, const char *p, XlyHttpHandler h, void *u) { return xly_http_route(s,"DELETE",p,h,u); }
int xly_http_patch (XlyHttpServer *s, const char *p, XlyHttpHandler h, void *u) { return xly_http_route(s,"PATCH", p,h,u); }

int xly_http_use(XlyHttpServer *srv, XlyHttpMiddleware mw) {
    if (!srv || srv->n_middleware >= XLY_HTTP_MAX_MIDDLEWARE) return -1;
    srv->middleware[srv->n_middleware++] = mw;
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * REQUEST HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

const char *xly_http_req_header(const XlyHttpRequest *req, const char *name) {
    if (!req || !name) return NULL;
    for (int i = 0; i < req->n_headers; i++)
        if (strcasecmp(req->headers[i].key, name) == 0)
            return req->headers[i].value;
    return NULL;
}

const char *xly_http_param(const XlyHttpRequest *req, const char *name) {
    if (!req || !name) return NULL;
    for (int i = 0; i < req->n_params; i++)
        if (strcmp(req->params[i].key, name) == 0)
            return req->params[i].value;
    return NULL;
}

char *xly_http_query(const XlyHttpRequest *req, const char *name) {
    if (!req || !name || !req->query[0]) return NULL;
    char tmp[2048];
    snprintf(tmp, sizeof(tmp), "%s", req->query);
    char *tok = strtok(tmp, "&");
    while (tok) {
        char *eq = strchr(tok, '=');
        if (eq) {
            *eq = '\0';
            char key[256], val[512];
            xly_http_urldecode(tok, key, sizeof(key));
            xly_http_urldecode(eq + 1, val, sizeof(val));
            if (strcmp(key, name) == 0)
                return strdup(val);
        }
        tok = strtok(NULL, "&");
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RESPONSE HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

void xly_http_set_header(XlyHttpResponse *res, const char *key, const char *value) {
    for (int i = 0; i < res->n_headers; i++) {
        if (strcasecmp(res->headers[i].key, key) == 0) {
            snprintf(res->headers[i].value, sizeof(res->headers[i].value),
                     "%s", value);
            return;
        }
    }
    if (res->n_headers < XLY_HTTP_MAX_HEADERS) {
        snprintf(res->headers[res->n_headers].key,
                 sizeof(res->headers[res->n_headers].key), "%s", key);
        snprintf(res->headers[res->n_headers].value,
                 sizeof(res->headers[res->n_headers].value), "%s", value);
        res->n_headers++;
    }
}

void xly_http_set_content_type(XlyHttpResponse *res, const char *mime) {
    xly_http_set_header(res, "Content-Type", mime);
}

void xly_http_send_text(XlyHttpCtx *ctx, XlyHttpStatus status, const char *body) {
    ctx->res->status = status;
    xly_http_set_content_type(ctx->res, "text/plain; charset=utf-8");
    size_t len = body ? strlen(body) : 0;
    ctx->res->body = body ? strdup(body) : NULL;
    ctx->res->body_len = len;
    xly_http_send(ctx);
}

void xly_http_send_json(XlyHttpCtx *ctx, XlyHttpStatus status, const char *json) {
    ctx->res->status = status;
    xly_http_set_content_type(ctx->res, "application/json");
    size_t len = json ? strlen(json) : 0;
    ctx->res->body = json ? strdup(json) : NULL;
    ctx->res->body_len = len;
    xly_http_send(ctx);
}

void xly_http_send_bytes(XlyHttpCtx *ctx, XlyHttpStatus status,
                          const char *mime, const void *data, size_t len) {
    ctx->res->status = status;
    if (mime) xly_http_set_content_type(ctx->res, mime);
    ctx->res->body = data ? (char *)malloc(len) : NULL;
    if (ctx->res->body) memcpy(ctx->res->body, data, len);
    ctx->res->body_len = len;
    xly_http_send(ctx);
}

void xly_http_redirect(XlyHttpCtx *ctx, XlyHttpStatus status, const char *loc) {
    ctx->res->status = status;
    xly_http_set_header(ctx->res, "Location", loc);
    ctx->res->body     = strdup("");
    ctx->res->body_len = 0;
    xly_http_send(ctx);
}

void xly_http_send(XlyHttpCtx *ctx) {
    if (ctx->res->sent) return;
    ctx->res->sent = 1;
    /* req->conn is set to &fd (int*) by handle_request before calling handlers */
    int fd = ctx->req->conn ? *(int *)ctx->req->conn : -1;
    if (fd >= 0)
        send_response(fd, ctx->res);
    /* Do NOT free body here — handle_request owns the response lifecycle */
}

void xly_http_write_chunk(XlyHttpCtx *ctx, const void *data, size_t len) {
    (void)ctx; (void)data; (void)len; /* TODO: implement chunked streaming */
}

void xly_http_end_chunked(XlyHttpCtx *ctx) {
    (void)ctx; /* TODO */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * JSON HELPERS  (minimal — no external dependencies)
 * ═══════════════════════════════════════════════════════════════════════════ */

char *xly_http_to_json(XlyVal *val) {
    if (!val) return strdup("null");
    char buf[8192];
    int  pos = 0;
    char *type_str = xly_to_cstr(xly_typeof(val));
    if (!type_str) return strdup("null");
    if (strcmp(type_str, "null") == 0) { free(type_str); return strdup("null"); }
    if (strcmp(type_str, "bool") == 0) {
        int t = xly_truthy(val); free(type_str);
        return strdup(t ? "true" : "false");
    }
    if (strcmp(type_str, "number") == 0) {
        char *s = xly_to_cstr(val); char *r = s ? strdup(s) : strdup("0");
        free(s); free(type_str); return r;
    }
    if (strcmp(type_str, "string") == 0) {
        char *raw = xly_to_cstr(val); const char *s = raw ? raw : "";
        pos = 0; buf[pos++] = '"';
        while (*s && pos < (int)sizeof(buf) - 8) {
            if      (*s == '"')  { buf[pos++] = '\\'; buf[pos++] = '"'; }
            else if (*s == '\\') { buf[pos++] = '\\'; buf[pos++] = '\\'; }
            else if (*s == '\n') { buf[pos++] = '\\'; buf[pos++] = 'n'; }
            else if (*s == '\r') { buf[pos++] = '\\'; buf[pos++] = 'r'; }
            else if (*s == '\t') { buf[pos++] = '\\'; buf[pos++] = 't'; }
            else { buf[pos++] = *s; } s++;
        }
        buf[pos++] = '"'; buf[pos] = '\0';
        free(raw); free(type_str); return strdup(buf);
    }
    if (strcmp(type_str, "array") == 0) {
        pos = 0; buf[pos++] = '[';
        size_t n = xly_array_len(val);
        for (size_t i = 0; i < n; i++) {
            if (i && pos < (int)sizeof(buf) - 2) buf[pos++] = ',';
            char *js = xly_http_to_json(xly_array_get(val, i));
            pos += snprintf(buf + pos, sizeof(buf) - (size_t)pos, "%s", js);
            free(js);
        }
        if (pos < (int)sizeof(buf) - 1) buf[pos++] = ']';
        buf[pos] = '\0'; free(type_str); return strdup(buf);
    }
    char *s = xly_to_cstr(val);
    snprintf(buf, sizeof(buf), "\"%s\"", s ? s : "");
    free(s); free(type_str); return strdup(buf);
}

XlyVal *xly_http_from_json(const char *json, size_t len) {
    if (!json || len == 0) return xly_null();
    /* Skip whitespace */
    const char *p = json;
    while (*p && isspace((unsigned char)*p)) p++;
    if (!*p) return xly_null();

    if (strcmp(p, "null") == 0)  return xly_null();
    if (strcmp(p, "true") == 0)  return xly_bool(1);
    if (strcmp(p, "false") == 0) return xly_bool(0);
    if (*p == '"') {
        /* String */
        char buf[4096]; size_t bi = 0;
        p++;
        while (*p && *p != '"' && bi < sizeof(buf)-1) {
            if (*p == '\\') { p++; buf[bi++] = *p ? *p : '\\'; }
            else buf[bi++] = *p;
            p++;
        }
        buf[bi] = '\0';
        return xly_str(buf);
    }
    if (*p == '-' || (*p >= '0' && *p <= '9')) {
        return xly_num(strtod(p, NULL));
    }
    /* For arrays/objects — return null (full parser is out of scope) */
    return xly_null();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * STATIC FILE SERVING
 * ═══════════════════════════════════════════════════════════════════════════ */

void xly_http_serve_file(XlyHttpCtx *ctx, const char *filepath) {
    struct stat sb;
    if (stat(filepath, &sb) < 0 || !S_ISREG(sb.st_mode)) {
        xly_http_send_text(ctx, XLY_HTTP_404, "File not found");
        return;
    }
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        xly_http_send_text(ctx, XLY_HTTP_500, "Cannot open file");
        return;
    }
    /* Find extension */
    const char *ext = strrchr(filepath, '.');
    const char *mime = xly_http_mime(ext);
    /* Read into buffer (could use sendfile for large files) */
    char *data = (char *)malloc((size_t)sb.st_size);
    if (!data) { close(fd); xly_http_send_text(ctx, XLY_HTTP_500, "OOM"); return; }
    ssize_t rd = read(fd, data, (size_t)sb.st_size);
    close(fd);
    if (rd < 0) { free(data); xly_http_send_text(ctx, XLY_HTTP_500, "Read error"); return; }
    xly_http_send_bytes(ctx, XLY_HTTP_200, mime, data, (size_t)rd);
    free(data);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * XLYVAL* RUNTIME API WRAPPERS
 * ═══════════════════════════════════════════════════════════════════════════ */


/* ── Internal helpers: read XlyVal via public API ───────────────────────── */
/* These avoid direct struct member access so xly_http.c stays opaque-safe. */

/* Return C string from a string XlyVal, or fallback. Caller must free(). */
static char *_val_str(XlyVal *v, const char *fallback) {
    if (!v) return strdup(fallback);
    char *t = xly_to_cstr(xly_typeof(v));
    int ok = t && strcmp(t, "string") == 0;
    free(t);
    if (!ok) return strdup(fallback);
    char *s = xly_to_cstr(v);
    return s ? s : strdup(fallback);
}

/* Return numeric value from a number XlyVal, or fallback. */
static double _val_num(XlyVal *v, double fallback) {
    if (!v) return fallback;
    char *t = xly_to_cstr(xly_typeof(v));
    int ok = t && strcmp(t, "number") == 0;
    free(t);
    if (!ok) return fallback;
    char *s = xly_to_cstr(v);
    double d = s ? strtod(s, NULL) : fallback;
    free(s);
    return d;
}

/* Return 1 if v is of the given type name. */
static int _val_is(XlyVal *v, const char *tname) {
    if (!v) return strcmp(tname, "null") == 0;
    char *t = xly_to_cstr(xly_typeof(v));
    int ok = t && strcmp(t, tname) == 0;
    free(t);
    return ok;
}

/* Store a pointer as a number XlyVal (uintptr_t fits in double for 48-bit addrs) */
static XlyVal *_ptr_to_val(void *p) {
    return xly_num((double)(uintptr_t)p);
}

static void *_val_to_ptr(XlyVal *v) {
    double d = _val_num(v, 0.0);
    return (void *)(uintptr_t)(unsigned long long)d;
}

/* We store the XlyHttpServer* in a number XlyVal* (pointer-sized double) */
static XlyHttpServer *val_to_srv(XlyVal *v) {
    return (XlyHttpServer *)_val_to_ptr(v);
}
static XlyVal *srv_to_val(XlyHttpServer *s) {
    return _ptr_to_val(s);
}

XlyVal *xly_http_create_val(XlyVal *cfg_obj) {
    (void)cfg_obj;
    XlyHttpServer *srv = xly_http_create(NULL);
    if (!srv) return xly_null();
    return srv_to_val(srv);
}

XlyVal *xly_http_listen_val(XlyVal *srv_val, XlyVal *host, XlyVal *port) {
    XlyHttpServer *srv = val_to_srv(srv_val);
    if (!srv) return xly_bool(0);
    char *_h = _val_str(host, "0.0.0.0");
    const char *h = _h;
    uint16_t    p = (uint16_t)_val_num(port, 8080.0);
    int rc = xly_http_listen(srv, h, p);
    free(_h);
    return xly_bool(rc == 0);
}

XlyVal *xly_http_run_val(XlyVal *srv_val) {
    XlyHttpServer *srv = val_to_srv(srv_val);
    if (!srv) return xly_null();
    xly_http_run(srv);
    return xly_null();
}

XlyVal *xly_http_run_async_val(XlyVal *srv_val) {
    XlyHttpServer *srv = val_to_srv(srv_val);
    if (!srv) return xly_bool(0);
    return xly_bool(xly_http_run_async(srv) == 0);
}

XlyVal *xly_http_stop_val(XlyVal *srv_val) {
    XlyHttpServer *srv = val_to_srv(srv_val);
    if (srv) xly_http_stop(srv);
    return xly_null();
}

/* Route val — the Xenly handler fn is stored as a closure pointer */
static void val_route_handler(XlyHttpCtx *ctx) {
    XlyVal *fn = (XlyVal *)ctx->user;
    if (!fn || !_val_is(fn, "function")) return;
    /* Build a context object to pass to the handler */
    XlyVal *ctx_obj = xly_obj_new();
    /* Populate with req fields */
    xly_obj_set(ctx_obj, "method",  xly_str(ctx->req->method));
    xly_obj_set(ctx_obj, "path",    xly_str(ctx->req->path));
    xly_obj_set(ctx_obj, "query",   xly_str(ctx->req->query));
    xly_obj_set(ctx_obj, "body",    ctx->req->body ? xly_str(ctx->req->body) : xly_null());
    xly_obj_set(ctx_obj, "remote",  xly_str(ctx->req->remote_addr));
    /* Store ctx pointer for response helpers */
    xly_obj_set(ctx_obj, "__ctx__", xly_num((double)(uintptr_t)ctx));
    /* Call the Xenly function */
    XlyVal *args[1] = { ctx_obj };
    xly_call_fnval(fn, args, 1);
    /* ctx_obj is a VAL_INSTANCE — let it be collected normally.
     * Do NOT call value_destroy here: the handler may have stored references
     * into res (via xly_http_send_*) that still point into ctx_obj fields. */
}

XlyVal *xly_http_route_val(XlyVal *srv_val, XlyVal *method, XlyVal *pattern,
                             XlyVal *handler, XlyVal *user) {
    XlyHttpServer *srv = val_to_srv(srv_val);
    if (!srv) return xly_bool(0);
    char *_m = _val_str(method, "GET");
    char *_p = _val_str(pattern, "/");
    const char *m = _m;
    const char *p = _p;
    (void)user;
    /* Handler must be a function value — store it as user data */
    int rc = xly_http_route(srv, m, p, val_route_handler, handler);
    free(_m); free(_p);
    return xly_bool(rc == 0);
}

XlyVal *xly_http_get_val   (XlyVal *s, XlyVal *p, XlyVal *h) { return xly_http_route_val(s, xly_str("GET"),    p, h, NULL); }
XlyVal *xly_http_post_val  (XlyVal *s, XlyVal *p, XlyVal *h) { return xly_http_route_val(s, xly_str("POST"),   p, h, NULL); }
XlyVal *xly_http_put_val   (XlyVal *s, XlyVal *p, XlyVal *h) { return xly_http_route_val(s, xly_str("PUT"),    p, h, NULL); }
XlyVal *xly_http_delete_val(XlyVal *s, XlyVal *p, XlyVal *h) { return xly_http_route_val(s, xly_str("DELETE"), p, h, NULL); }

static XlyHttpCtx *ctx_from_val(XlyVal *obj) {
    XlyVal *cp = xly_obj_get(obj, "__ctx__");
    return (XlyHttpCtx *)_val_to_ptr(cp);
}

XlyVal *xly_http_send_text_val(XlyVal *ctx_obj, XlyVal *status, XlyVal *body) {
    XlyHttpCtx *ctx = ctx_from_val(ctx_obj);
    if (!ctx) return xly_null();
    int s = (int)_val_num(status, 200.0);
    char *_b = _val_str(body, "");
    const char *b = _b;
    xly_http_send_text(ctx, (XlyHttpStatus)s, b);
    free(_b);
    return xly_null();
}

XlyVal *xly_http_send_json_val(XlyVal *ctx_obj, XlyVal *status, XlyVal *json) {
    XlyHttpCtx *ctx = ctx_from_val(ctx_obj);
    if (!ctx) return xly_null();
    int s = (int)_val_num(status, 200.0);
    char *_j = _val_str(json, "null");
    const char *j = _j;
    xly_http_send_json(ctx, (XlyHttpStatus)s, j);
    free(_j);
    return xly_null();
}

XlyVal *xly_http_req_header_val(XlyVal *ctx_obj, XlyVal *name) {
    XlyHttpCtx *ctx = ctx_from_val(ctx_obj);
    if (!ctx || !name || !_val_is(name, "string")) return xly_null();
    char *_name0 = _val_str(name, "");
    const char *v = xly_http_req_header(ctx->req, _name0);
    free(_name0);
    return v ? xly_str(v) : xly_null();
}

XlyVal *xly_http_param_val(XlyVal *ctx_obj, XlyVal *name) {
    XlyHttpCtx *ctx = ctx_from_val(ctx_obj);
    if (!ctx || !name || !_val_is(name, "string")) return xly_null();
    char *_name1 = _val_str(name, "");
    const char *v = xly_http_param(ctx->req, _name1);
    free(_name1);
    return v ? xly_str(v) : xly_null();
}

XlyVal *xly_http_query_val(XlyVal *ctx_obj, XlyVal *name) {
    XlyHttpCtx *ctx = ctx_from_val(ctx_obj);
    if (!ctx || !name || !_val_is(name, "string")) return xly_null();
    char *_name2 = _val_str(name, "");
    char *v = xly_http_query(ctx->req, _name2);
    free(_name2);
    if (!v) return xly_null();
    XlyVal *r = xly_str(v);
    free(v);
    return r;
}

XlyVal *xly_http_to_json_val(XlyVal *val) {
    char *s = xly_http_to_json(val);
    if (!s) return xly_null();
    XlyVal *r = xly_str(s);
    free(s);
    return r;
}

XlyVal *xly_http_from_json_val(XlyVal *str_val) {
    if (!str_val || !_val_is(str_val, "string")) return xly_null();
    char *_sv = _val_str(str_val, "");
    XlyVal *r = xly_http_from_json(_sv, strlen(_sv));
    free(_sv);
    return r;
}
