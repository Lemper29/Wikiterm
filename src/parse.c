#define _XOPEN_SOURCE 700

#include "parse.h"
#include "cJSON.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>

char *parse_extract(const char *value)
{
    cJSON *json = cJSON_Parse(value ?  value: "");
    if (!json) return NULL;

    cJSON *query = cJSON_GetObjectItemCaseSensitive(json, "query");
    cJSON *pages = query ? cJSON_GetObjectItemCaseSensitive(query, "pages") : NULL;

    char *result = NULL;

    const cJSON *page = NULL;
    cJSON_ArrayForEach(page, pages) {
        cJSON *extract = cJSON_GetObjectItemCaseSensitive((cJSON *)page, "extract");
        if (cJSON_IsString(extract) && (extract->valuestring != NULL)) {
            result = strdup(extract->valuestring);
            break;
        }
    }

    cJSON_Delete(json);
    return result;
}
