#define _XOPEN_SOURCE 700

#include "fetch.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct memory {
    char *data;
    size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) 
{
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)userp;
    if (!mem) return 0;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) return 0;

    mem->data = ptr;
    memcpy(mem->data + mem->size, contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = '\0';
    return realsize;
}

char *url_encode(CURL *curl, const char *s)
{
    if (!s) return NULL;
    return curl_easy_escape(curl, s, 0);
}

int fetch_url(const char *url, FetchBuffer *out, int timeout_sec)
{
    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "wiki-extract/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_sec);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) return -1;

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code < 200 || http_code >= 300) return -1;

    return 0;
}
void fetch_free(FetchBuffer *buf) 
{
    if (buf && buf->data) {
        free(buf->data);
        buf->data = NULL;
        buf->size = 0;
    }
}
