#ifndef OPC_CLIENT_H
#define OPC_CLIENT_H

#include "scraper.h"

void send_to_opcua(const char *server_url, GenerationData *gen, ExchangeData *exch, FrequencyData *freq);

#endif
