/* For popen/pclose */
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"
#include "fetch.h"
#include "parse.h"
#include "display.h"

struct memory {
    char *data;
    size_t size;
};

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
    snprintf(url, sizeof(url),
                     "https://en.wikipedia.org/w/api.php?action=query&titles=%s&prop=extracts&explaintext=1&format=json",
                     encoded);
    curl_free(encoded);

    FetchBuffer buf = {0};
    int err = fetch_url(url, &buf, 30L);
    if (err != 0) {
        fprintf(stderr, "Failed to fetch url\n");
        free(buf.data); 
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    char *text = parse_extract(buf.data);
    if (!text) {
        fprintf(stderr, "No extract found\n");
        fetch_free(&buf);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    display_text(text);

    free(text);
    fetch_free(&buf);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return EXIT_SUCCESS;
}
