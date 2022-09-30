#include <Arduino.h>
#include "buzzer.hpp"

float tmp_temperature;
float threshold_value;
float threshold_delta = 10.0f;
uint8_t buzzer_pin = 12;
bool is_buzzer_on;

void buzzer_init(uint8_t pin) {
    tmp_temperature = 0.0f;
    is_buzzer_on = false;
    buzzer_pin = pin;
    pinMode(buzzer_pin, OUTPUT);
}

void buzzer_set_threshold(float threshold) {
    threshold_value = threshold;
}

static void buzzer_alarm_on() {
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

static void buzzer_alarm_off() {
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

void buzzer_set_temperature(float temperature) {
    if (is_buzzer_on) {
        if (temperature < threshold_value - threshold_delta) {
            buzzer_alarm_off();
            is_buzzer_on = false;
        }
    }
    else {
        if (temperature > threshold_value + threshold_delta) {
            buzzer_alarm_on();
            is_buzzer_on = true;
        }
    }
}
