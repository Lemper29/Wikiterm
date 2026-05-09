#ifndef FETCH_H
#define FETCH_H

#include <curl/curl.h>
#include <stddef.h>

typedef struct {
    char *data;
    size_t size;
} FetchBuffer;

int fetch_url(const char *url, FetchBuffer *out, int timeout_sec);

void fetch_free(FetchBuffer *buf);

char *url_encode(CURL *curl, const char *s);

#endif // FETCH_H
