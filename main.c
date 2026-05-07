/* For popen/pclose */
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

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

static char *url_encode(CURL *curl, const char *s)
{
    if (!s) return NULL;
    return curl_easy_escape(curl, s, 0);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \"Wikipedia page title\"\n", argc ? argv[0] : "wiki_extract");
        return EXIT_FAILURE;
    }

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        fprintf(stderr, "curl_global_init failed\n");
        return EXIT_FAILURE;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl_easy_init failed\n");
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    char *encoded = url_encode(curl, argv[1]);
    if (!encoded) {
        fprintf(stderr, "URL encoding failed\n");
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    char url[1024];
    int n = snprintf(url, sizeof(url),
                     "https://en.wikipedia.org/w/api.php?action=query&titles=%s&prop=extracts&explaintext=1&format=json",
                     encoded);
    curl_free(encoded);

    if (n < 0 || (size_t)n >= sizeof(url)) {
        fprintf(stderr, "URL too long\n");
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    struct memory chunk = { .data = NULL, .size = 0 };

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "wiki-extract/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
        free(chunk.data);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code < 200 || http_code >= 300) {
        fprintf(stderr, "HTTP error: %ld\n", http_code);
        free(chunk.data);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    cJSON *json = cJSON_Parse(chunk.data ? chunk.data : "");
    if (!json) {
        fprintf(stderr, "Failed to parse JSON\n");
        free(chunk.data);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    cJSON *query = cJSON_GetObjectItemCaseSensitive(json, "query");
    cJSON *pages = query ? cJSON_GetObjectItemCaseSensitive(query, "pages") : NULL;
    if (!pages) {
        fprintf(stderr, "No pages in response\n");
        cJSON_Delete(json);
        free(chunk.data);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    const cJSON *page = NULL;
    cJSON_ArrayForEach(page, pages) {
        cJSON *extract = cJSON_GetObjectItemCaseSensitive((cJSON *)page, "extract");
        if (cJSON_IsString(extract) && (extract->valuestring != NULL)) {
            FILE *less = popen("less -R", "w");
            if (less) {
                fprintf(less, "%s\n", extract->valuestring);
                pclose(less);
            } else {
                puts(extract->valuestring);
            }
            break;
        }
    }

    cJSON_Delete(json);
    free(chunk.data);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return EXIT_SUCCESS;
}
