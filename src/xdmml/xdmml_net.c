/*
 * xdmml_net.c - Cross-platform Networking Implementation
 */

#include "xdmml_extended.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_MACOS) || defined(XDMML_PLATFORM_FREEBSD)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #define XDMML_HAS_POSIX_SOCKETS 1
#endif

// ─── Socket Structure ────────────────────────────────────────────────────────

struct XDMML_Socket {
#ifdef XDMML_HAS_POSIX_SOCKETS
    int fd;
#endif
    XDMML_SocketType type;
};

// ─── Socket Functions ────────────────────────────────────────────────────────

XDMML_Socket* xdmml_create_socket(XDMML_SocketType type) {
    XDMML_Socket* sock = (XDMML_Socket*)calloc(1, sizeof(XDMML_Socket));
    if (!sock) return NULL;
    
    sock->type = type;
    
#ifdef XDMML_HAS_POSIX_SOCKETS
    int sock_type = (type == XDMML_SOCKET_TCP) ? SOCK_STREAM : SOCK_DGRAM;
    sock->fd = socket(AF_INET, sock_type, 0);
    
    if (sock->fd < 0) {
        free(sock);
        return NULL;
    }
    
    // Set SO_REUSEADDR
    int opt = 1;
    setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    
    return sock;
}

void xdmml_close_socket(XDMML_Socket* socket) {
    if (!socket) return;
    
#ifdef XDMML_HAS_POSIX_SOCKETS
    if (socket->fd >= 0) {
        close(socket->fd);
    }
#endif
    
    free(socket);
}

XDMML_Result xdmml_connect(XDMML_Socket* socket, const char* host, uint16_t port) {
    if (!socket || !host) return XDMML_ERROR_INVALID_PARAM;
    
#ifdef XDMML_HAS_POSIX_SOCKETS
    struct hostent* server = gethostbyname(host);
    if (!server) {
        return XDMML_ERROR;
    }
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr.s_addr, server->h_addr, server->h_length);
    
    if (connect(socket->fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return XDMML_ERROR;
    }
    
    return XDMML_OK;
#else
    return XDMML_ERROR_NOT_SUPPORTED;
#endif
}

XDMML_Result xdmml_bind(XDMML_Socket* socket, const char* address, uint16_t port) {
    if (!socket) return XDMML_ERROR_INVALID_PARAM;
    
#ifdef XDMML_HAS_POSIX_SOCKETS
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (address && strlen(address) > 0) {
        addr.sin_addr.s_addr = inet_addr(address);
    } else {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    
    if (bind(socket->fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return XDMML_ERROR;
    }
    
    return XDMML_OK;
#else
    return XDMML_ERROR_NOT_SUPPORTED;
#endif
}

XDMML_Result xdmml_listen(XDMML_Socket* socket, int backlog) {
    if (!socket) return XDMML_ERROR_INVALID_PARAM;
    
#ifdef XDMML_HAS_POSIX_SOCKETS
    if (listen(socket->fd, backlog) < 0) {
        return XDMML_ERROR;
    }
    return XDMML_OK;
#else
    return XDMML_ERROR_NOT_SUPPORTED;
#endif
}

XDMML_Socket* xdmml_accept(XDMML_Socket* socket) {
    if (!socket) return NULL;
    
#ifdef XDMML_HAS_POSIX_SOCKETS
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    int client_fd = accept(socket->fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        return NULL;
    }
    
    XDMML_Socket* client = (XDMML_Socket*)calloc(1, sizeof(XDMML_Socket));
    if (!client) {
        close(client_fd);
        return NULL;
    }
    
    client->fd = client_fd;
    client->type = socket->type;
    
    return client;
#else
    return NULL;
#endif
}

int xdmml_send(XDMML_Socket* socket, const void* data, size_t len) {
    if (!socket || !data) return -1;
    
#ifdef XDMML_HAS_POSIX_SOCKETS
    return send(socket->fd, data, len, 0);
#else
    return -1;
#endif
}

int xdmml_recv(XDMML_Socket* socket, void* buffer, size_t len) {
    if (!socket || !buffer) return -1;
    
#ifdef XDMML_HAS_POSIX_SOCKETS
    return recv(socket->fd, buffer, len, 0);
#else
    return -1;
#endif
}

void xdmml_set_socket_blocking(XDMML_Socket* socket, bool blocking) {
    if (!socket) return;
    
#ifdef XDMML_HAS_POSIX_SOCKETS
    int flags = fcntl(socket->fd, F_GETFL, 0);
    if (blocking) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }
    fcntl(socket->fd, F_SETFL, flags);
#endif
}

void xdmml_set_socket_timeout(XDMML_Socket* socket, uint32_t ms) {
    if (!socket) return;
    
#ifdef XDMML_HAS_POSIX_SOCKETS
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    
    setsockopt(socket->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(socket->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
}

// ─── HTTP Client (Simple Implementation) ─────────────────────────────────────

XDMML_HTTPResponse* xdmml_http_get(const char* url) {
    if (!url) return NULL;
    
    // Parse URL (simplified - assumes http://host:port/path format)
    char host[256] = {0};
    int port = 80;
    char path[512] = "/";
    
    // Simple URL parsing
    if (strncmp(url, "http://", 7) == 0) {
        const char* start = url + 7;
        const char* slash = strchr(start, '/');
        const char* colon = strchr(start, ':');
        
        if (colon && (!slash || colon < slash)) {
            size_t host_len = colon - start;
            strncpy(host, start, host_len);
            host[host_len] = '\0';
            port = atoi(colon + 1);
            if (slash) {
                strncpy(path, slash, sizeof(path) - 1);
            }
        } else if (slash) {
            size_t host_len = slash - start;
            strncpy(host, start, host_len);
            host[host_len] = '\0';
            strncpy(path, slash, sizeof(path) - 1);
        } else {
            strncpy(host, start, sizeof(host) - 1);
        }
    } else {
        return NULL;
    }
    
    // Create socket and connect
    XDMML_Socket* sock = xdmml_create_socket(XDMML_SOCKET_TCP);
    if (!sock) return NULL;
    
    if (xdmml_connect(sock, host, port) != XDMML_OK) {
        xdmml_close_socket(sock);
        return NULL;
    }
    
    // Send HTTP request
    char request[1024];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             path, host);
    
    xdmml_send(sock, request, strlen(request));
    
    // Receive response
    char* response_data = NULL;
    size_t response_size = 0;
    size_t response_capacity = 4096;
    response_data = (char*)malloc(response_capacity);
    
    while (1) {
        char buffer[1024];
        int received = xdmml_recv(sock, buffer, sizeof(buffer));
        if (received <= 0) break;
        
        if (response_size + received > response_capacity) {
            response_capacity *= 2;
            response_data = (char*)realloc(response_data, response_capacity);
        }
        
        memcpy(response_data + response_size, buffer, received);
        response_size += received;
    }
    
    xdmml_close_socket(sock);
    
    // Parse response
    XDMML_HTTPResponse* resp = (XDMML_HTTPResponse*)calloc(1, sizeof(XDMML_HTTPResponse));
    
    // Parse status code
    if (response_data && response_size > 12) {
        resp->status_code = atoi(response_data + 9);
        
        // Find body (after \r\n\r\n)
        char* body_start = strstr(response_data, "\r\n\r\n");
        if (body_start) {
            body_start += 4;
            size_t body_len = response_size - (body_start - response_data);
            resp->body = (char*)malloc(body_len + 1);
            memcpy(resp->body, body_start, body_len);
            resp->body[body_len] = '\0';
            resp->body_length = body_len;
        }
    }
    
    free(response_data);
    return resp;
}

XDMML_HTTPResponse* xdmml_http_post(const char* url, const void* data,
                                     size_t length, const char* content_type) {
    // Similar to GET but with POST method and body
    (void)url; (void)data; (void)length; (void)content_type;
    // TODO: Implement POST
    return NULL;
}

void xdmml_free_http_response(XDMML_HTTPResponse* response) {
    if (!response) return;
    free(response->body);
    free(response->content_type);
    free(response);
}
