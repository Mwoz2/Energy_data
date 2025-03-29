#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/types_generated.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


typedef struct {
    int intValue;
    char stringValue[100];
} CustomData;


// string node change value
static void updateVariable(UA_Server *server) {
    static int counter = 0;
    CustomData data;
    data.intValue = counter;
    snprintf(data.stringValue, sizeof(data.stringValue), "Message %d", counter);
    counter++;

    // Tworzenie wariantu OPC UA
    UA_Variant value;
    UA_Variant_init(&value);
    UA_String uaString = UA_STRING(data.stringValue);
    UA_Variant_setScalar(&value, &uaString, &UA_TYPES[UA_TYPES_STRING]);

    // Aktualizacja warto?ci w?z?a
    UA_NodeId nodeId = UA_NODEID_NUMERIC(1, 2101);
    UA_Server_writeValue(server, nodeId, value);


    // // update temp
    // float values[73] = {0};
    // for (int i = 0; i<73; i++){
    //     values[i] = i;
    // }

    // // Tworzenie wariantu OPC UA
    // UA_Variant valuee;
    // UA_Variant_init(&valuee);
    // UA_Variant_setArray(&valuee, values, 73, &UA_TYPES[UA_TYPES_FLOAT]);

    // // Aktualizacja warto?ci w?z?a
    // nodeId = UA_NODEID_NUMERIC(1, 2201);
    // UA_Server_writeValue(server, nodeId, value);
}


// add variable node
static void addVariable(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "Dane pogodowe");
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE; 

    UA_String initialString = UA_STRING("Initial Value");
    UA_Variant_setScalar(&attr.value, &initialString, &UA_TYPES[UA_TYPES_STRING]);

    UA_NodeId nodeId = UA_NODEID_NUMERIC(1, 2001); // ID zmiennej
    UA_QualifiedName name = UA_QUALIFIEDNAME(1, "Dane pogodowe [qn]");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, 1001); // ID rodzica
    UA_Server_addVariableNode(server, nodeId, parentNodeId, 
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), // relacja 
        name,
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), // typ zmiennej 
        attr, NULL, NULL);
}


// add object node
static void addObject(UA_Server *server, char* name, int nodeID, UA_NodeId parent_nodeID) {
    UA_NodeId myObject;
    UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    oAttr.displayName = UA_LOCALIZEDTEXT("en-US", name);

    UA_Server_addObjectNode(server, 
        UA_NODEID_NUMERIC(1, nodeID),  // ID w?z?a
        parent_nodeID, // Rodzic (ObjectsFolder)
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), // Relacja z rodzicem
        UA_QUALIFIEDNAME(1, name), // Nazwa
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), // Typ (bazowy obiekt OPC UA)
        oAttr, NULL, &myObject);
}


// add string node
static void addStringNode(UA_Server *server, char* name, int nodeID, int parent_nodeID){
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", name);
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE; 

    UA_String initialString = UA_STRING("01/01/2000 00:00");
    UA_Variant_setScalar(&attr.value, &initialString, &UA_TYPES[UA_TYPES_STRING]);

    UA_NodeId nodeId = UA_NODEID_NUMERIC(1, nodeID); // ID zmiennej
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(1, parent_nodeID); // ID rodzica
    UA_Server_addVariableNode(server, nodeId, parentNodeId, 
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), // relacja 
        UA_QUALIFIEDNAME(1, name),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), // typ zmiennej 
        attr, NULL, NULL);
}


// add float array node
static void addFloatArrayNode(UA_Server *server, char* name, int nodeID, int parent_nodeID) {
    // Tworzymy tablic? 73 element�w typu float
    float values[73] = {0}; // Inicjalizacja zerami
    for (int i = 0; i<73; i++){
        values[i] = i;
    }

    // Ustawienia atrybut�w
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", name);
    attr.dataType = UA_TYPES[UA_TYPES_FLOAT].typeId; // Typ float
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE; 

    attr.valueRank = 1; // Tablica jednowymiarowa
    attr.arrayDimensionsSize = 1;
    UA_UInt32 arrayDims = 73;
    attr.arrayDimensions = &arrayDims;
    // attr.arrayDimensions = (UA_UInt32 *)UA_Array_new(9, &UA_TYPES[UA_TYPES_FLOAT]);
    // attr.arrayDimensionsSize = 2; // TODO - check
    // attr.arrayDimensions[0] = 3;
    // attr.arrayDimensions[1] = 3; // then array size = row x col
    // Przypisanie warto?ci do node'a
    UA_Variant_setArrayCopy(&attr.value, values, 73, &UA_TYPES[UA_TYPES_FLOAT]);

    // Dodanie w?z?a
    UA_Server_addVariableNode(server,
        UA_NODEID_NUMERIC(1, nodeID),
        UA_NODEID_NUMERIC(1, parent_nodeID),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, name),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        attr, NULL, NULL);
}


// static void addDataType(UA_Server *server) {
//     UA_DataTypeAttributes dtAttr = UA_DataTypeAttributes_default;
//     dtAttr.displayName = UA_LOCALIZEDTEXT("en-US", "BasicData");
//     dtAttr.description = UA_LOCALIZEDTEXT("en-US", "Structure for weather station data");

//     UA_NodeId dataTypeNodeId = UA_NODEID_NUMERIC(1, 5002);

//     UA_Server_addDataTypeNode(server, dataTypeNodeId, UA_DataTypeAttributes
//         UA_NODEID_NUMERIC(0, UA_NS0ID(STRUCTUREDTYPE)),
//         UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
//         UA_QUALIFIEDNAME(1, "BasicData"),
//         dtAttr, NULL, NULL);

//     // Definiujemy pola w strukturze
//     UA_StructureField fields[3];

//     // Pole: stationID (int)
//     UA_StructureField_init(&fields[0]);
//     fields[0].dataType = UA_TYPES[UA_TYPES_INT32].typeId;
//     fields[0].name = UA_STRING("stationID");

//     // Pole: time (UA_DateTime)
//     UA_StructureField_init(&fields[1]);
//     fields[1].dataType = UA_TYPES[UA_TYPES_DATETIME].typeId;
//     fields[1].name = UA_STRING("time");

//     // Pole: temperature (float)
//     UA_StructureField_init(&fields[2]);
//     fields[2].dataType = UA_TYPES[UA_TYPES_FLOAT].typeId;
//     fields[2].name = UA_STRING("temperature");

//     // Tworzymy opis struktury
//     UA_StructureDefinition structure;
//     UA_StructureDefinition_init(&structure);
//     structure.structureType = UA_STRUCTURETYPE_STRUCTURE;
//     structure.fieldsSize = 3;
//     structure.fields = fields;

//     // Przypisujemy definicj? do DataType Node
//     UA_Server_setDataTypeDefinition(server, dataTypeNodeId, structure);
// }


// static void addStruct(UA_Server *server) {
//     UA_VariableAttributes vAttr = UA_VariableAttributes_default;
//     vAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Dane pogodowe");

//     // Create a sample value
//     UA_DateTimeStruct time;
//     time.year = 2025;
//     time.month = 1;
//     time.day = 1;
//     time.hour = 12;
//     time.min = 0;
//     BasicData data;
//     data.stationID = 1;
//     data.time = time;
//     data.temperature = 20.1;

//     // Convert struct to UA_Variant
//     UA_Variant value;
//     UA_Variant_setScalar(&value, &data, &UA_TYPES[UA_TYPES_STRUCTUREFIELD]);
//     vAttr.value = value;

//     // Add variable node
//     UA_NodeId variableNodeId = UA_NODEID_NUMERIC(1, 7001);
//     UA_Server_addVariableNode(server, variableNodeId,
//         UA_NODEID_NUMERIC(0, 2001),
//         UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
//         UA_QUALIFIEDNAME(1, "MachineData"),
//         UA_NODEID_NUMERIC(1, 5002), // Reference the custom structure
//         vAttr, NULL, NULL);
// }