#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

char* AutumnAPI_JSONCreateRequest(const char *ep, const char *act, int id) {
	cJSON *root = cJSON_CreateObject();
	if (!root) return NULL;

	cJSON_AddStringToObject(root, "endpoint", ep);
	cJSON_AddStringToObject(root, "action", act);
	cJSON_AddNumberToObject(root, "id", id);
	
	char *json_str = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	
	return json_str; //return to result
}

int AutumnAPI_JSONParseResp(const char *str, char *out_stat, int max_len) {
	cJSON *root = cJSON_Parse(str); //parsing received response
	if (!root) {
		return -1; //fnf
	}

	cJSON *status = cJSON_GetObjectItemCaseSensitive(root, "status");

	if (cJSON_IsString(status) && (status->valuestring != NULL)) { //if response is valid and not empty
        	strncpy(out_stat, status->valuestring, max_len - 1); //copy status to out_stat parameter of function.
        	out_stat[max_len - 1] = '\0';
	}

	else {
  //destroyed string leak.
		snprintf(out_stat, "UNKNOWN", max_len);
	}

	cJSON *code = cJSON_GetObjectItemCaseSensitive(root, "code");
    	int result_code = cJSON_IsNumber(code) ? code->valueint : -1;

    	cJSON_Delete(root);
    	return result_code; //returning to result
}
