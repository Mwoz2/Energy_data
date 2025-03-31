#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <unistd.h> 

#define CSV_FILENAME "pse_data.csv"
#define MAX_COUNTRIES 6

typedef struct {
    char *memory;
    size_t size;
} MemoryStruct;

typedef struct {
    char kraj[10];  // Kod kraju (CZ, DE, SK, LT, UA, SE)
    double wartosc; // Warto?? wymiany energii w MW
} ExchangeRecord;

typedef struct {
    double cieplne;
    double wodne;
    double wiatrowe;
    double fotowoltaiczne;
    double inne;
} GenerationRecord;

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + total_size + 1);
    if (ptr == NULL) return 0;

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, total_size);
    mem->size += total_size;
    mem->memory[mem->size] = 0;

    return total_size;
}

double get_json_double(cJSON *json, const char *key) {
    cJSON *item = cJSON_GetObjectItem(json, key);
    return (item && cJSON_IsNumber(item)) ? item->valuedouble : 0.0;
}

void fetch_pse_data(const char *url, GenerationRecord *gen_data, ExchangeRecord exch_data[]) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {malloc(1), 0};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "? B??d pobierania danych: %s\n", curl_easy_strerror(res));
        } else {
            cJSON *json = cJSON_Parse(chunk.memory);
            if (json == NULL) {
                printf("? B??d parsowania JSON!\n");
            } else {
                cJSON *data = cJSON_GetObjectItem(json, "data");
                if (data) {
                    cJSON *summary = cJSON_GetObjectItem(data, "podsumowanie");
                    if (summary) {
                        gen_data->cieplne = get_json_double(summary, "cieplne");
                        gen_data->wodne = get_json_double(summary, "wodne");
                        gen_data->wiatrowe = get_json_double(summary, "wiatrowe");
                        gen_data->fotowoltaiczne = get_json_double(summary, "PV");
                        gen_data->inne = get_json_double(summary, "inne");
                    }

                    cJSON *exchanges = cJSON_GetObjectItem(data, "przesyly");
                    if (exchanges && cJSON_IsArray(exchanges)) {
                        int size = cJSON_GetArraySize(exchanges);
                        for (int i = 0; i < size; i++) {
                            cJSON *item = cJSON_GetArrayItem(exchanges, i);
                            if (item) {
                                cJSON *id = cJSON_GetObjectItem(item, "id");
                                cJSON *value = cJSON_GetObjectItem(item, "wartosc");

                                if (id && cJSON_IsString(id) && value && cJSON_IsNumber(value)) {
                                    for (int j = 0; j < MAX_COUNTRIES; j++) {
                                        if (strcmp(exch_data[j].kraj, id->valuestring) == 0) {
                                            exch_data[j].wartosc = value->valuedouble;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                cJSON_Delete(json);
            }
        }

        free(chunk.memory);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

void save_to_csv(const GenerationRecord *gen_data, const ExchangeRecord exch_data[]) {
    FILE *file = fopen(CSV_FILENAME, "w");
    if (!file) {
        perror("? Blad otwierania pliku CSV");
        return;
    }

    fprintf(file, "Rodzaj, Wartosc (MW)\n");
    fprintf(file, "Elektrownie cieplne, %.2f\n", gen_data->cieplne);
    fprintf(file, "Elektrownie wodne, %.2f\n", gen_data->wodne);
    fprintf(file, "Elektrownie wiatrowe, %.2f\n", gen_data->wiatrowe);
    fprintf(file, "Elektrownie fotowoltaiczne, %.2f\n", gen_data->fotowoltaiczne);
    fprintf(file, "Inne odnawialne, %.2f\n", gen_data->inne);

    fprintf(file, "\nKraj, Wymiana energii (MW)\n");
    for (int i = 0; i < MAX_COUNTRIES; i++) {
        fprintf(file, "%s, %.2f\n", exch_data[i].kraj, exch_data[i].wartosc);
    }

    fclose(file);
    printf("? Dane zapisane do pliku: %s\n", CSV_FILENAME);
}

void print_data(const GenerationRecord *gen_data, const ExchangeRecord exch_data[]) {
    printf("\n? Generacja energii (MW):\n");
    printf("   Cieplne: %.2f MW\n", gen_data->cieplne);
    printf("   Wodne: %.2f MW\n", gen_data->wodne);
    printf("   Wiatrowe: %.2f MW\n", gen_data->wiatrowe);
    printf("   Fotowoltaiczne: %.2f MW\n", gen_data->fotowoltaiczne);
    printf("   Inne odnawialne: %.2f MW\n", gen_data->inne);

    printf("\n? Wymiana miedzynarodowa (MW):\n");
    for (int i = 0; i < MAX_COUNTRIES; i++) {
        printf("   ?? %s: %.2f MW\n", exch_data[i].kraj, exch_data[i].wartosc);
    }
}

int main() {
    ExchangeRecord exch_data[MAX_COUNTRIES] = {
        {"CZ", 0.0}, {"DE", 0.0}, {"SK", 0.0}, {"LT", 0.0}, {"UA", 0.0}, {"SE", 0.0}
    };

    while (1) {
        GenerationRecord gen_data = {0};

        fetch_pse_data("https://www.pse.pl/transmissionMapService", &gen_data, exch_data);
        save_to_csv(&gen_data, exch_data);
        print_data(&gen_data, exch_data);

        printf("\n------------------------------------\n");
        printf("? Odswiezanie danych za 15 sekund...\n");
        sleep(15);
    }

    return 0;
}
