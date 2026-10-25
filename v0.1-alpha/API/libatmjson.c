
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

char *AutumnAPI_JSONCreateRequest(const char *ep, const char *act, int id) {
    if (ep == NULL || act == NULL)
        return NULL;

    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
        return NULL;

    cJSON_AddStringToObject(root, "endpoint", ep);
    cJSON_AddStringToObject(root, "action", act);
    cJSON_AddNumberToObject(root, "id", id);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json_str;
}

int AutumnAPI_JSONParseResp(const char *str, char *out_stat, int max_len) 
{
    if (str == NULL || out_stat == NULL || max_len <= 0)
        return -1;

    out_stat[0] = '\0';

    cJSON *root = cJSON_Parse(str);
    if (root == NULL) {
        snprintf(out_stat, (size_t)max_len, "INVALID_JSON");
        return -1;
    }

    cJSON *status =
        cJSON_GetObjectItemCaseSensitive(root, "status");

    cJSON *code =
        cJSON_GetObjectItemCaseSensitive(root, "code");

    if (cJSON_IsString(status) && status->valuestring != NULL) {
        snprintf(out_stat, (size_t)max_len, "%s",
                 status->valuestring);
    } else {
        snprintf(out_stat, (size_t)max_len, "UNKNOWN");
    }

    int result_code =
        cJSON_IsNumber(code) ? code->valueint : -1;

    cJSON_Delete(root);
    return result_code;
}

