#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <unistd.h> 

struct MemoryStruct {
    char *memory;
    size_t size;
};

typedef struct {
    double cieplne;
    double wodne;
    double wiatrowe;
    double fotowoltaiczne;
    double inne;
} GenerationData;

typedef struct {
    double CZ, DE, SK, LT, UA, SE;
} ExchangeData;

typedef struct {
    double frequency;
} FrequencyData;

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

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

void fetch_pse_data(const char *url, GenerationData *gen_data, ExchangeData *exch_data, FrequencyData *freq_data) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "? Blad pobierania danych: %s\n", curl_easy_strerror(res));
        } else {
            cJSON *json = cJSON_Parse(chunk.memory);
            if (json == NULL) {
                printf("? Blad parsowania JSON!\n");
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
                        freq_data->frequency = get_json_double(summary, "czestotliwosc");
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
                                    if (strcmp(id->valuestring, "CZ") == 0) exch_data->CZ = value->valuedouble;
                                    else if (strcmp(id->valuestring, "DE") == 0) exch_data->DE = value->valuedouble;
                                    else if (strcmp(id->valuestring, "SK") == 0) exch_data->SK = value->valuedouble;
                                    else if (strcmp(id->valuestring, "LT") == 0) exch_data->LT = value->valuedouble;
                                    else if (strcmp(id->valuestring, "UA") == 0) exch_data->UA = value->valuedouble;
                                    else if (strcmp(id->valuestring, "SE") == 0) exch_data->SE = value->valuedouble;
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

void save_generation_to_csv(GenerationData gen_data) {
    FILE *file = fopen("generation_data.csv", "w");
    if (!file) {
        perror("? Blad otwarcia pliku generation_data.csv");
        return;
    }

    fprintf(file, "Rodzaj,Wartosc (MW)\n");
    fprintf(file, "Cieplne,%.2f\n", gen_data.cieplne);
    fprintf(file, "Wodne,%.2f\n", gen_data.wodne);
    fprintf(file, "Wiatrowe,%.2f\n", gen_data.wiatrowe);
    fprintf(file, "Fotowoltaiczne,%.2f\n", gen_data.fotowoltaiczne);
    fprintf(file, "Inne,%.2f\n", gen_data.inne);

    fclose(file);
}

void save_exchange_to_csv(ExchangeData exch_data) {
    FILE *file = fopen("exchange_data.csv", "w");
    if (!file) {
        perror("? Blad otwarcia pliku exchange_data.csv");
        return;
    }

    fprintf(file, "Kraj,Wymiana (MW)\n");
    fprintf(file, "Czechy,%.2f\n", exch_data.CZ);
    fprintf(file, "Niemcy,%.2f\n", exch_data.DE);
    fprintf(file, "Slowacja,%.2f\n", exch_data.SK);
    fprintf(file, "Litwa,%.2f\n", exch_data.LT);
    fprintf(file, "Ukraina,%.2f\n", exch_data.UA);
    fprintf(file, "Szwecja,%.2f\n", exch_data.SE);

    fclose(file);
}

void save_frequency_to_csv(FrequencyData freq_data) {
    FILE *file = fopen("frequency_data.csv", "w");
    if (!file) {
        perror("? Blad otwarcia pliku frequency_data.csv");
        return;
    }

    fprintf(file, "Czsstotliwosc (Hz)\n");
    fprintf(file, "%.3f\n", freq_data.frequency);

    fclose(file);
}
void scraper() {
    GenerationData gen_data = {0};
    ExchangeData exch_data = {0};
    FrequencyData freq_data = {0};

    fetch_pse_data("https://www.pse.pl/transmissionMapService", &gen_data, &exch_data, &freq_data);

    save_generation_to_csv(gen_data);
    save_exchange_to_csv(exch_data);
    save_frequency_to_csv(freq_data);

    printf("? Dane zapisane do plikow CSV.\n");
}

