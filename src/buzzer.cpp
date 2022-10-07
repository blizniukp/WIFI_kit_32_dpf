#include <Arduino.h>
#include "buzzer.hpp"

uint8_t buzzer_pin = 12;
const float soot_load_threshold = 80.0f;

float temperature_threshold_value;
float temperature_threshold_delta = 10.0f;
bool is_temperature_alarm_on = false;

float soot_load_threshold_value;
float soot_load_threshold_delta = 1.0f;
bool is_soot_load_alarm_on = false;


void buzzer_init(uint8_t pin) {
    is_temperature_alarm_on = false;
    is_soot_load_alarm_on = false;
    soot_load_threshold_value = soot_load_threshold;
    buzzer_pin = pin;
    pinMode(buzzer_pin, OUTPUT);
}

void buzzer_set_temperature_threshold(float threshold) {
    temperature_threshold_value = threshold;
}

static void buzzer_temperature_alarm_on() {
    digitalWrite(buzzer_pin, 1);
    delay(1000);
    digitalWrite(buzzer_pin, 0);
    delay(250);
    digitalWrite(buzzer_pin, 1);
    delay(250);
    digitalWrite(buzzer_pin, 0);
    delay(250);
    digitalWrite(buzzer_pin, 1);
    delay(250);
    digitalWrite(buzzer_pin, 0);
}

static void buzzer_temperature_alarm_off() {
    digitalWrite(buzzer_pin, 1);
    delay(250);
    digitalWrite(buzzer_pin, 0);
    delay(250);
    digitalWrite(buzzer_pin, 1);
    delay(250);
    digitalWrite(buzzer_pin, 0);
    delay(250);
    digitalWrite(buzzer_pin, 1);
    delay(1000);
    digitalWrite(buzzer_pin, 0);
}

static void buzzer_soot_load_alarm_on() {
    digitalWrite(buzzer_pin, 1);
    delay(500);
    digitalWrite(buzzer_pin, 0);
    delay(500);
    digitalWrite(buzzer_pin, 1);
    delay(500);
    digitalWrite(buzzer_pin, 0);
}

void buzzer_set_temperature(float temperature) {
    if (is_temperature_alarm_on) {
        if (temperature < temperature_threshold_value - temperature_threshold_delta) {
            buzzer_temperature_alarm_off();
            is_temperature_alarm_on = false;
        }
    }
    else {
        if (temperature > temperature_threshold_value + temperature_threshold_delta) {
            buzzer_temperature_alarm_on();
            is_temperature_alarm_on = true;
        }
    }
}

void buzzer_set_soot_load(float soot_load) {
    if (is_soot_load_alarm_on) {
        if (soot_load < soot_load_threshold_value - soot_load_threshold_delta) {
            is_soot_load_alarm_on = false;
        }
    }
    else {
        if (soot_load > soot_load_threshold_value + soot_load_threshold_delta) {
            buzzer_soot_load_alarm_on();
            is_soot_load_alarm_on = true;
        }
    }
}
