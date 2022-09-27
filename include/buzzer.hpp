#ifndef BUZZER_HPP
#define BUZZER_HPP

void buzzer_init(uint8_t pin);
void buzzer_set_threshold(float threshold);
void buzzer_set_temperature(float temperature);

#endif