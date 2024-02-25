#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>

#define MAX_INPUT_LENGTH 256
#define MAX_RESPONSE_LENGTH 4096

typedef struct memory_struct_s {
    char* data;
    size_t size;
} memory_struct_t;

typedef struct chatgpt_conversation_t {
    const char* api_token;
    const char* organisation;
    const char* model;
    const char* conversation_file;
} chatgpt_conversation_t;

/////////// static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
/////////// {
///////////     size_t real_size = size * nmemb;
///////////     memory_struct_t* mem = (memory_struct_t*)userp;
/////////// 
///////////     mem->data = realloc(mem->data, mem->size + real_size + 1);
///////////     if (mem->data == NULL)
///////////     {
///////////         printf("Failed to allocate memory!\n");
///////////         return 0;
///////////     }
/////////// 
///////////     memcpy(&(mem->data[mem->size]), contents, real_size);
///////////     mem->size += real_size;
///////////     mem->data[mem->size] = '\0';
/////////// 
///////////     return real_size;
/////////// }
// Callback function to write the API response into a string
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char *response = (char *)userp;
    
    // Append the response to the existing string
    response = realloc(response, strlen(response) + realsize + 1);
    if (response == NULL) {
        printf("Error: Failed to allocate memory for response.");
        return 0;
    }
    strcat(response, (char *)contents);
    
    return realsize;
}


void get_organisations(char *apiToken) {
    CURL *curl;
    CURLcode res;
    
    char *response = malloc(1);
    response[0] = ' ';
    
    // Create the API endpoint URL
    char url[] = "https://api.openai.com/v1/organizations";
    
    // Initialize the libcurl library
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Create a curl handle
    curl = curl_easy_init();
    if(curl) {
        // Set the API URL
        curl_easy_setopt(curl, CURLOPT_URL, url);
        
        // Set the authorization header with your API token
        struct curl_slist *headers = NULL;
        char authHeader[50] = "Authorization: Bearer ";
        strcat(authHeader, apiToken);
        headers = curl_slist_append(headers, authHeader);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // Set the write callback function to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
        
        // Make the API call
        res = curl_easy_perform(curl);
        
        // Check for errors
        if(res != CURLE_OK)
            printf("Error: %s", curl_easy_strerror(res));
        
        // Cleanup
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        curl_global_cleanup();
        
        // Print the API response
        printf("%s", response);
        
        // Free the response buffer
        free(response);
    }
}


void list_models(const char* apiToken,const char* org)
{
    CURL* curl = curl_easy_init();
    if (curl)
    {
        memory_struct_t response;
        response.data = malloc(1);  // Will be grown as needed
        response.size = 0;

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/engines/davinci/models");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, (const char*)apiToken);
        headers = curl_slist_append(headers, (const char*)org);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else
        {
            printf("%s\n", response.data);
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(response.data);
    }
    else
    {
        printf("Failed to initialize CURL!\n");
    }
}

void conversation(const char* apiToken, const char* model, const char* organization, const char* conversationFile)
{
    FILE* file = fopen(conversationFile, "wb");
    if (file == NULL)
    {
        printf("Failed to open conversation file!\n");
        return;
    }

    CURL* curl = curl_easy_init();
    if (curl)
    {
        memory_struct_t response;
        response.data = malloc(1);  // Will be grown as needed
        response.size = 0;

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/engines/davinci/completions");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, (const char*)apiToken);
        headers = curl_slist_append(headers, (const char*)organization);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        char input[MAX_INPUT_LENGTH];
        while (1)
        {
            printf("User: ");
            if (fgets(input, sizeof(input), stdin) == NULL)
            {
                printf("Failed to read user input!\n");
                break;
            }

            // Trim trailing newline character
            size_t len = strlen(input);
            if (len > 0 && input[len - 1] == '\n')
            {
                input[len - 1] = '\0';
            }

            if (strcmp(input, "quit") == 0)
            {
                break;
            }

            char postData[MAX_RESPONSE_LENGTH];
            snprintf(postData, sizeof(postData), "{\"documents\": [\"%s\"], \"organization\": \"%s\"}", input, organization);

            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                break;
            }

            fwrite(input, sizeof(char), strlen(input), file);
            fwrite("\n", sizeof(char), 1, file);
            fwrite(response.data, sizeof(char), response.size, file);
            fwrite("\n", sizeof(char), 1, file);

            printf("ChatGPT: %.*s\n", (int)response.size, response.data);
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(response.data);
    }
    else
    {
        printf("Failed to initialize CURL!\n");
    }

    fclose(file);
}

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        printf("Usage: %s list-orgs <API_TOKEN> \n", argv[0]);
        printf("Usage: %s list-models <API_TOKEN> <ORGANIZATION>\n", argv[0]);
        printf("       %s conversation <API_TOKEN> <ORGANIZATION> <MODEL> <CONVERSATION_FILE>\n", argv[0]);
        return -1;
    }

    const char* command = argv[1];
    const char* apiToken = argv[2];

    if(strcmp(command,"list-models") == 0) {
        if (argc <= 3)
        {
            printf("<API_TOKEN> <ORGANIZATION> is missing \n", argv[0]);
            return -2;
        }
        get_organisations(apiToken);
    } else if(strcmp(command,"list-models") == 0) {
        if (argc <= 3)
        {
            printf("<API_TOKEN> <ORGANIZATION> is missing \n", argv[0]);
            return -2;
        }
        const char* organization = argv[3];
        list_models(apiToken,organization);
    } else if(strcmp(command,"list-models") == 0) {
        if (argc < 6)
        {
            printf("some parameters are missing\n", argv[0]);
            printf("Usage: %s conversation <API_TOKEN> <ORGANIZATION> <MODEL> <CONVERSATION_FILE>\n", argv[0]);
            return -3;
        }
        const char* organization = argv[3];
        const char* model = argv[4];
        const char* conversationFile = argv[5];
        conversation(apiToken, model,organization, conversationFile);
    } else {
        printf("command: %s  not found\n", argv[1]);
        printf("Usage: %s list-models <API_TOKEN> \n", argv[0]);
        printf("       %s conversation <API_TOKEN> <ORGANIZATION> <MODEL> <CONVERSATION_FILE>\n", argv[0]);
        return -4;

    }

    return 0;
}
