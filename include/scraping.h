#ifndef SCRAPING_H
#define SCRAPING_H

#include <stdio.h>

void fetchDataFromPSE(char *url);
void parseHTML(const char *html);

#endif