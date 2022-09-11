#ifndef BUZZER_HPP
#define BUZZER_HPP

void buzzer_init(int pin);
void buzzer_set_threshold(int temperature);
void buzzer_set_temperature(float temperature);

#endif