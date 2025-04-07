#ifndef SCRAPER_H
#define SCRAPER_H

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

void scraper(GenerationData *gen, ExchangeData *exch, FrequencyData *freq);

#endif
