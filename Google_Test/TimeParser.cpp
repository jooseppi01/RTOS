#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "TimeParser.h"
#include <cstdlib> // for atoi
#include <cstdio>  // for sprintf

// time format: HHMMSS (6 characters)
int time_parse(char *time) {

    // how many seconds, default returns error
    int seconds = TIME_LEN_ERROR;

    // Check that string is not null
    if (time == NULL) {
        return TIME_LEN_ERROR;
    }

	// Check that string length is 6
	if (strlen(time) != 6) {
		return TIME_LEN_ERROR;
	}

	// check that the string is not empty
	if (time[0] == '\0') {
		return TIME_LEN_ERROR;
	}

	// test that the string only contains numbers, using isdigit()
	if (isdigit(time[0]) == 0 || isdigit(time[1]) == 0 || isdigit(time[2]) == 0 || isdigit(time[3]) == 0 || isdigit(time[4]) == 0 || isdigit(time[5]) == 0) {
		return TIME_LEN_ERROR;
	}

    // Parse values from time string
    // For example: 124033 -> 12hour 40min 33sec
    int values[3];
    values[2] = atoi(time + 4); // seconds
    time[4] = 0;
    values[1] = atoi(time + 2); // minutes
    time[2] = 0;
    values[0] = atoi(time); // hours
    // Now you have:
    // values[0] hour
    // values[1] minute
    // values[2] second

    // Add boundary check time values: below zero or above limit not allowed
    if (values[0] < 0 || values[0] > 23 || values[1] < 0 || values[1] > 59 || values[2] < 0 || values[2] > 59) {
        return TIME_LEN_ERROR;
    }

    // Calculate return value from the parsed minutes and seconds
    seconds = (values[1] * 60) + values[2];

	// test if value is 0
	if (seconds == 0) {
		return TIME_LEN_ERROR;
	}

    return seconds;
}


// Parse the input string from uart to color and time
UartParser uart_parse(char *input)
{
    // example input: r1000
    UartParser result;

    // check that the string length is 5
    if (strlen(input) != 5) {
        result.color = COLOR_ERROR;
        result.time = TIME_LEN_ERROR;
        return result;
    }

    //check that the first character is r, g, or y
    if (input[0] != 'r' && input[0] != 'g' && input[0] != 'y') {
        result.color = COLOR_ERROR;
        result.time = TIME_LEN_ERROR;
        return result;
    }

    // check that the last 4 characters are numbers
    if (isdigit(input[1]) == 0 || isdigit(input[2]) == 0 || isdigit(input[3]) == 0 || isdigit(input[4]) == 0) {
        result.color = COLOR_ERROR;
        result.time = TIME_LEN_ERROR;
        return result;
    }

    char color = input[0];
    int time = atoi(input + 1);

    result.color = color;
    result.time = time;

    return result;
}
