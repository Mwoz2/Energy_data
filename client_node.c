#include <open62541/client.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/client_config_default.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/types_generated.h>
#include <stdio.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER_URL "opc.tcp://172.29.30.45:4840/"
#define NODE_ID "C3_EnergyData"

int main(void) {
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));

    UA_StatusCode status = UA_Client_connect(client, SERVER_URL);
    if (status != UA_STATUSCODE_GOOD) {
        printf("Couldn't connect with server OPC UA!\n");
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }
    else{
    printf("Connected with server OPC UA!\n");
    }
    UA_NodeId nodeId = UA_NODEID_STRING(1, NODE_ID);

    while (true) {
        UA_Double newValue = (double)(rand() % 1000) / 10.0;
        UA_Variant value;
        UA_Variant_setScalar(&value, &newValue, &UA_TYPES[UA_TYPES_DOUBLE]);
    
        status = UA_Client_writeValueAttribute(client, nodeId, &value);
        if (status == UA_STATUSCODE_GOOD) {
            printf("Kurwaaa: %.2f\n", newValue);
        } else {
            UA_StatusCode_name(status);
        }
    
        sleep(1);
    }
    
    UA_Client_disconnect(client);
    UA_Client_delete(client);
    printf("Unconnect with server OPC UA.\n");
    return EXIT_SUCCESS;
}
