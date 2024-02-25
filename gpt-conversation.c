#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define MAX_INPUT_LEN 1024
#define MAX_RESPONSE_LEN 4096

struct MemoryStruct {
  char *memory;
  size_t size;
};

// Callback function to handle the response from the API
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL) {
    // out of memory!
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

// Function to send a request to OpenAI API and get response
char* chatGPTResponse(const char *apiKey, const char *modelName, const char *question) {
  CURL *curl;
  CURLcode res;
  struct MemoryStruct chunk;
  chunk.memory = malloc(1);  // will be grown as needed
  chunk.size = 0;             // no data at this point

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  if (curl) {
    char url[256];
    snprintf(url, sizeof(url), "https://api.openai.com/v1/chat/%s", modelName);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, question);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Authorization: Bearer ");
    strcat(headers->data, apiKey);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }

  return chunk.memory;
}

int main(int argc, char *argv[]) {
  if (argc != 5 || strcmp(argv[1], "-api") != 0 || strcmp(argv[3], "-model") != 0) {
    printf("Usage: %s -api <API_KEY> -model <MODEL_NAME>\n", argv[0]);
    return 1;
  }

  const char *apiKey = argv[2];
  const char *modelName = argv[4];

  char input[MAX_INPUT_LEN];
  printf("Ask a question (type 'quit' to exit):\n");
  while (fgets(input, sizeof(input), stdin)) {
    if (strcmp(input, "quit\n") == 0)
      break;

    // Remove trailing newline character
    input[strcspn(input, "\n")] = 0;

    char *response = chatGPTResponse(apiKey, modelName, input);
    printf("Response: %s\n", response);

    free(response);
    printf("\nAsk another question (type 'quit' to exit):\n");
  }

  return 0;
}
