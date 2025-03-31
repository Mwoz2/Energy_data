#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include "parser.h"

// Funkcja do parsowania HTML
htmlDocPtr parseHTML(const char* filename) {
    // Pr�bujemy wczyta? dokument HTML z pliku
    htmlDocPtr doc = htmlReadFile(filename, NULL, HTML_PARSE_RECOVER);
    if (doc == NULL) {
        fprintf(stderr, "B??d podczas parsowania pliku HTML: %s\n", filename);
        return NULL;
    }
    return doc;
}

// Funkcja do wydobywania danych z dokumentu HTML
void extractData(htmlDocPtr doc) {
    if (doc == NULL) {
        return;
    }

    // Tutaj mo?esz wykona? przetwarzanie dokumentu, np. za pomoc? XPath
    xmlNode* root = xmlDocGetRootElement(doc);
    printf("Root element: %s\n", root->name);

    // Mo?esz doda? tutaj dodatkow? logik? parsowania
}

// Funkcja do czyszczenia zasob�w po zako?czeniu pracy z dokumentem
void freeDocument(htmlDocPtr doc) {
    if (doc != NULL) {
        xmlFreeDoc(doc);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <html file>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    htmlDocPtr doc = parseHTML(filename);  // Parsujemy plik HTML

    if (doc != NULL) {
        extractData(doc);  // Wydobywamy dane z dokumentu HTML
        freeDocument(doc); // Zwalniamy zasoby
    } else {
        fprintf(stderr, "Nie uda?o si? za?adowa? pliku HTML: %s\n", filename);
        return 1;
    }

    return 0;
}
