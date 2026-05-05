#include "cJSON.h"
#include <curl/curl.h>
#include <curl/typecheck-gcc.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct memory {
 char *response;
 size_t size;
};

size_t write_callback(char *data, size_t size, size_t nmemb, void *clientp) {
    size_t realsize = nmemb;
    struct memory *mem = (struct memory *)clientp;

    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if(!ptr)
        return 0; 
 
    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;
 
    return realsize;
}

int main(int argc, char **argv) {
    struct memory chunk = { 0 }; 

    char url[1000];
    snprintf(url, sizeof(url),"https://en.wikipedia.org/w/api.php?action=query&titles=%s&prop=extracts&explaintext=1&format=json", argv[1]);

    curl_global_init(CURL_GLOBAL_ALL);
    CURL *handler = curl_easy_init();
    if (handler) {
        
        curl_easy_setopt(handler, CURLOPT_URL, url);
        curl_easy_setopt(handler, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
        curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(handler, CURLOPT_WRITEDATA, (void *)&chunk);

        CURLcode response = curl_easy_perform(handler);
        if (response == 0) {
            cJSON *json = cJSON_Parse(chunk.response);
            if (json == NULL) {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL) {
                    printf("Error: %s\n", error_ptr);
                }
                cJSON_Delete(json);
                return 1;
            }
            cJSON *query = cJSON_GetObjectItemCaseSensitive(json, "query");
            if (query == NULL) {
                printf("No 'query' field\n");
                cJSON_Delete(json);
                return 1;
            }

            cJSON *pages = cJSON_GetObjectItemCaseSensitive(query, "pages");
            if (pages == NULL) {
                printf("No 'pages' field\n");
                cJSON_Delete(json);
                return 1;
            }

            const cJSON *page = NULL;
            cJSON_ArrayForEach(page, pages) {
                cJSON *extract = cJSON_GetObjectItemCaseSensitive(page, "extract");
                if (extract != NULL && cJSON_IsString(extract)) {
                    printf("%s\n", extract->valuestring);
                    break;  
                }
            }
            cJSON_Delete(json);
        } else {
            printf("failed req. Code: %d\n", response);
        }
        free(chunk.response);
        curl_easy_cleanup(handler);
    }
}
