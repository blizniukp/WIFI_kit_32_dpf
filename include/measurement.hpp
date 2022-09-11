#ifndef MEASUREMENT_HPP
#define MEASUREMENT_HPP

#include <Arduino.h>

#define MAX_UNIT_LENGTH (4)
#define MAX_COMMAND_LENGTH (9)
#define MAX_CAPTION_LENGTH (30)

typedef struct measurement_def
{
    byte id; /*Index*/
    char caption[MAX_CAPTION_LENGTH];
    char command[MAX_COMMAND_LENGTH];                             /*Command to send*/
    char unit[MAX_UNIT_LENGTH];                                   /*Unit [g,km,min]*/
    float value;                                                  /*Calculated value*/
    bool (*calcFunPtr)(char *command, float *val, float divider); /*A pointer to a function to retrieve and calculate values*/
    float divider;                                                /*Parameter to calc_fun*/
    bool enabled;                                                 /*Is measurement on?*/
    void (*dataReadFunPtr)(float value);                          /*Function executed after reading the value*/
} measurement_t;
#endif