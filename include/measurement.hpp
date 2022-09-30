#ifndef MEASUREMENT_HPP
#define MEASUREMENT_HPP

static const uint8_t MAX_UNIT_LENGTH = 4;
static const uint8_t MAX_COMMAND_LENGTH = 9;
static const uint8_t MAX_CAPTION_LENGTH = 30;

typedef struct {
    uint8_t id;                                                                 /*Index*/
    char caption[MAX_CAPTION_LENGTH];                                           /*Caption*/
    char command[MAX_COMMAND_LENGTH];                                           /*Command to send*/
    char unit[MAX_UNIT_LENGTH];                                                 /*Unit [g,km,min]*/
    float value;                                                                /*Calculated value*/
    bool (*calcFunPtr)(char* data, size_t data_len, float* val, float divider); /*A pointer to a function to retrieve and calculate values*/
    float divider;                                                              /*Parameter to calc_fun*/
    bool enabled;                                                               /*Is measurement on?*/
    void (*dataReadFunPtr)(float value);                                        /*Function executed after reading the value*/
} measurement_t;

#endif