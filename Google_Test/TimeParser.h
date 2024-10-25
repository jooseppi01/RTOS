#ifndef TIMEPARSER_H
#define TIMEPARSER_H

// Error codes
#define TIME_LEN_ERROR      -1
#define TIME_ARRAY_ERROR    -2
#define TIME_VALUE_ERROR    -3
#define COLOR_ERROR         -4


using namespace std;

struct UartParser
{
    char color;
    int time;
};

int time_parse(char *time);
UartParser uart_parse(char *input);

#endif
