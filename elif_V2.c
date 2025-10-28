#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h> // Needed for malloc, realloc, free
#include <curl/curl.h>
#include <jansson.h>

// --- Boilerplate Toolbox (No changes needed here) ---
struct MemoryStruct {
    char *memory;
    size_t size;
};
// boilerplate
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) { return 0; }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}
// boilerplate code
char* WebAns(const char* question) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    if(curl_handle) {
        char *encoded_question = curl_easy_escape(curl_handle, question, 0);
        char full_url[512];
        snprintf(full_url, sizeof(full_url), "https://api.duckduckgo.com/?q=%s&format=json", encoded_question);
        curl_easy_setopt(curl_handle, CURLOPT_URL, full_url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        res = curl_easy_perform(curl_handle);
        curl_free(encoded_question);
        curl_easy_cleanup(curl_handle);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(chunk.memory);
            return NULL;
        }
    }
    curl_global_cleanup();
    return chunk.memory;
}

// --- FIX: This is the robust, safe version of parseAnswer ---
char* parseAnswer(const char* json_string){
    json_error_t error;
    json_t *root = json_loads(json_string, 0, &error);
    char *result = NULL;

    // Safety Check 1: Make sure the JSON is valid
    if (!root) {
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        return NULL;
    }

    // Safety Check 2: Get the object, it might not exist
    json_t *answer_obj = json_object_get(root, "AbstractText");

    // Safety Check 3: Make sure the object is a non-empty string before using it
    if (json_is_string(answer_obj)) {
        const char *answer_text = json_string_value(answer_obj);
        // Only create a copy if the answer text is valid and not empty
        if (answer_text && strlen(answer_text) > 0) {
            result = strdup(answer_text);
        }
    }
    
    // Clean up the jansson object memory BEFORE returning
    json_decref(root);
    
    return result; // This will be NULL if no valid answer was found
}

int main() {
    char userInp[256];
    char normalInp[256];
    printf("------------------Commencing the chatbot--------------------\n");
    while (1)
    {
        printf("You: ");
        fgets(userInp, sizeof(userInp), stdin);
        
        userInp[strcspn(userInp, "\n")] = '\0';

        int len = strlen(userInp);
        for (int i = 0; i < len; i++) {
            normalInp[i] = tolower(userInp[i]);
        }
        normalInp[len] = '\0';

        if (strcmp(normalInp, "bye") == 0) {
            printf("Bot: Bye!\n");
            break;
        } else if (strstr(normalInp, "hello") != NULL) {
            printf("Bot: Hello user!\n");
        } else if (strstr(normalInp, "who are you") != NULL) {
            printf("Bot: I am Ellie, and I'm here to assist you!\n");
        } else {
            printf("Bot: Let me look that up for you...\n");
            
            char *jsonResponse = WebAns(userInp);
            
            if (jsonResponse != NULL) {
                char *cleanAnswer = parseAnswer(jsonResponse);
                free(jsonResponse);
                if (cleanAnswer != NULL) {
                    printf("Bot: Here is what I found: %s\n", cleanAnswer);
                    free(cleanAnswer); // Free the clean answer's memory
                } else {
                    printf("Bot: I couldn't find a clear answer for that.\n");
                }
            } else {
                printf("Bot: An error occurred while searching the web.\n");
            }
        }
    }
    
    return 0;
}
