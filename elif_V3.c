#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <curl/curl.h>
#include <jansson.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

// This is the "mail carrier" function that libcurl calls whenever it receives
// a chunk of data. Its only job is to add that chunk to our memory box.
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        printf("Error: not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// This is the main function for our toolbox. We give it a question, and it
// gives you back the raw JSON response from the web.
char* WebAns(const char* question) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    struct curl_slist *headers = NULL;
    char *response_ptr = NULL;

    // Initialize our memory box
    chunk.memory = malloc(1);
    chunk.size = 0;

curl_handle = curl_easy_init();
    if(curl_handle) {
        char api_url[512];
        char json_payload[1024]; 
        const char *api_key = getenv("GEMINI_API_KEY"); // Get API key
        if (!api_key) api_key = ""; 

        curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
        snprintf(json_payload, sizeof(json_payload), "{\"contents\": [{\"parts\": [{\"text\": \"%s\"}]}]}", question);
        curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, json_payload);

        struct curl_slist *headers_list = NULL;
        headers_list = curl_slist_append(headers_list, "Content-Type: application/json");
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers_list);

        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl_handle);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(chunk.memory); // Free memory if request failed
            chunk.memory = NULL; 
        } else {
             long http_code = 0;
             curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
             if (http_code >= 400) {
                 fprintf(stderr, "HTTP error %ld received.\n", http_code);
                 fprintf(stderr, "Response: %s\n", chunk.memory ? chunk.memory : "No response body");
                 free(chunk.memory);
                 chunk.memory = NULL;
             } else {
                 // Success! Keep the memory pointer.
                 response_ptr = chunk.memory;
             }
        }

        // Cleanup curl handle and headers
        curl_easy_cleanup(curl_handle);
        if (headers) curl_slist_free_all(headers);

    } else {
        fprintf(stderr, "curl_easy_init() failed\n");
        free(chunk.memory); // Free memory if init failed
    }

    return response_ptr; // Return pointer to response data (or NULL if error)
}

char* parseAnswer(const char* json_string){
    json_error_t error; // to log errors
    json_t *root = json_loads(json_string, 0, &error); // json_loads organises stuff 
    // json_t *root provides the memory address to the structured data (a pointer to json string) which has been structred ny json loads
    // we use pointer because it gives access to the memory address of all data in that function
    json_t *answer = json_object_get(root, "AbstractText");
    // free(json_string);

    if (!(json_is_string(answer) && strlen(json_string_value(answer)) > 0))
    {
        answer = json_object_get(root, "Answer");
    }

    if (!(json_is_string(answer) && strlen(json_string_value(answer)) > 0))
    {
        json_t *related_topic = json_object_get(root, "RelatedTopics"); // getting array of related topics first
        if (json_is_array(related_topic) && json_array_size(related_topic) > 0) // check the size and if json is array
        {
            size_t num_topics = json_array_size(related_topic);
            for (int i = 0; i <= 5 && i < num_topics; i++)
            {
            json_t *get_topic = json_array_get(related_topic, i); // getting the first topic from the array of topics related
            
            if (get_topic == NULL)
            {
                continue;
            }
            json_t *topic_text = json_object_get(get_topic, "Text");
            if (json_is_string(topic_text) && strlen(json_string_value(topic_text)) > 0)
            {
                answer  = topic_text;
                break;
            }
            // answer = json_object_get(get_topic, "Text"); // gets the specific object type in the array
            }
            
        } 
    }
    char *result = NULL;

    if(json_is_string(answer) && strlen(json_string_value(answer))>0){
        result = strdup(json_string_value(answer));
    }

    json_decref(root);
    

    // if (json_is_string(answer) && json_string_value(answer) != 0)
    // {
    //     char *ans = strdup(json_string_value(answer)); // to create a copy of the text cleaned
    //     json_decref(root);
    //     return ans;                                    // WET approach = write everything twice (not good)
    // } else {
    //     answer = json_object_get(root, "Answer"); // assigning Answer field as answer
    //     if (json_is_string(answer) && json_string_value(answer) != 0)
    //     {
    //       char *ans = strdup(json_string_value(answer)); // to create a copy of the text cleaned
    //       json_decref(root);
    //       return ans;
    //     } else {
    //         json_t *related_topic = json_object_get(root, "RelatedTopics");
    //         if (json_is_array(related_topic) && json_array_size(related_topic) > 0)
    //         {
    //             json_t *first_topic = json_array_get(related_topic, 0);
    //             answer  = json_object_get(first_topic, "Text");
    //             char *ans = strdup(json_string_value(answer)); // to create a copy of the text cleaned
    //             json_decref(root);
    //             return ans;
    //         } else {
    //             printf("\n An error occurred! ");
    //             return NULL;
    //             exit(1);
    //         }
            
    //     }
        
    // }

        // char *ans = strdup(json_string_value(answer)); // to create a copy of the text cleaned
        // json_decref(root);
        // return ans;
        return result;


}
int main() {
    char userInp[256];
    char normalInp[256];
    printf("------------------Commencing the chatbot--------------------\n");
    while (1)
    {
        printf("User: ");
        fgets(userInp, sizeof(userInp), stdin);
        
        userInp[strcspn(userInp, "\n")] = '\0'; // snippet for cleaning new line characters
    //    for (int i = 0; i < strlen(userInp) ; i++)
    //     {
    //         normalInp[i] = tolower(userInp[i]); // lower / making it case sensitive
    //     } 
        int len = strlen(userInp);
        for (int i = 0; i < len; i++) {
            normalInp[i] = tolower(userInp[i]);
            }
              normalInp[len] = '\0';
        if (strcmp(normalInp, "Hello") == 0)
        {
            printf("Hello user!\n");
        } else if (strstr(normalInp, "Bye") != NULL)
        {
            printf("Bye!\n");
            break;
        } else if (strcmp(normalInp, "Who are you") == 0)
        {
            printf("I am Ellie aka Elif and im here to assist you!\n");
        } else {
            char *jsonRef = WebAns(userInp); // sending user request to the web & getting raw json data
            if (jsonRef != NULL)
            {
                char *clean_ans = parseAnswer(jsonRef); // passing raw data to parseAnswer to get clean answer
                free(jsonRef);
                printf("Here is what I could find on the internet: %s \n ", clean_ans);
                free(clean_ans);
            } else {
                printf("\n An error occurred! ");
            }
            
        }
    }
    
    return 0;
}