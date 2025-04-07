#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <stdio.h>
#include "include/opc_client.h"

void write_double(UA_Client *client, int node_id, double value) {
    UA_Variant val;
    UA_Variant_init(&val);
    UA_Variant_setScalar(&val, &value, &UA_TYPES[UA_TYPES_DOUBLE]);

    UA_WriteValue wv;
    UA_WriteValue_init(&wv);
    wv.nodeId = UA_NODEID_NUMERIC(1, node_id);
    wv.attributeId = UA_ATTRIBUTEID_VALUE;
    wv.value.hasValue = true;
    wv.value.value = val;

    UA_WriteRequest wr;
    UA_WriteRequest_init(&wr);
    wr.nodesToWrite = &wv;
    wr.nodesToWriteSize = 1;

    UA_WriteResponse resp = UA_Client_Service_write(client, wr);
    if (resp.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
        printf("? B??d zapisu do node %d\n", node_id);
    }

    UA_WriteResponse_clear(&resp);
}


void send_to_opcua(const char *server_url, GenerationData *gen, ExchangeData *exch, FrequencyData *freq) {
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));

    if (UA_Client_connect(client, server_url) != UA_STATUSCODE_GOOD) {
        printf("? Brak po??czenia z OPC UA\n");
        UA_Client_delete(client);
        return;
    }

    write_double(client, 4002, gen->cieplne);
    write_double(client, 4003, gen->wodne);
    write_double(client, 4004, gen->wiatrowe);
    write_double(client, 4005, gen->fotowoltaiczne);
    write_double(client, 4006, gen->inne);
    write_double(client, 4007, freq->frequency);

    write_double(client, 4008, exch->CZ);
    write_double(client, 4009, exch->DE);
    write_double(client, 4010, exch->SK);
    write_double(client, 4011, exch->LT);
    write_double(client, 4012, exch->UA);
    write_double(client, 4013, exch->SE);

    printf("? Dane wys?ane do OPC UA\n");

    UA_Client_disconnect(client);
    UA_Client_delete(client);
}
