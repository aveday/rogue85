#include <avr/io.h>
#include <stdbool.h>
#ifndef INPUT_H
#define INPUT_H

#define SAMPLES 20
#define BORDER_ADJUST 2

#define UP 1<<4
#define DOWN 1<<7
#define LEFT 1<<6
#define RIGHT 1<<5
#define A 1<<3
#define B 1<<2
#define X 1<<1
#define Y 1<<0

uint8_t prev_input = 0;
uint8_t input = 0;
bool hold = false;
bool repeat = false;

uint8_t Vdefs[] = {
  255, 230, 209, 192, 178, 166, 156, 149,
  142, 133, 125, 121, 116, 109, 105, 99
};

void init_input(bool r) {
  repeat = r;

  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // set ADC prescaler
  ADMUX |= (1 << ADLAR); // left adjust ADC result for 8 bit value
  ADCSRA |= (1 << ADEN); // enable ADC
}

uint8_t read_adc(uint8_t mux) {
  ADMUX = 1 << ADLAR | mux;   // set pin input
  ADCSRA |= (1 << ADSC);      // start conversion
  while(ADCSRA  & (1 << ADSC)) continue; // wait 
  return ADCH;
}

uint8_t get_input() {
  //TODO auto trigger

  uint8_t left_voltage = read_adc(LEFT_INPUT);
  if (left_voltage < 250)
    for (uint8_t i = 0; i < SAMPLES; ++i) {
      uint8_t v = read_adc(LEFT_INPUT);
      left_voltage = v < left_voltage ? v : left_voltage;
    }


  int8_t left_input = -1;
  while(Vdefs[++left_input] - BORDER_ADJUST > left_voltage)
    continue;

  uint8_t right_voltage = read_adc(RIGHT_INPUT);
  if (right_voltage < 250)
    for (uint8_t i = 0; i < SAMPLES; ++i) {
      uint8_t v = read_adc(RIGHT_INPUT);
      right_voltage = v < right_voltage ? v : right_voltage;
    }

  int8_t right_input = -1;
  while(Vdefs[++right_input] - BORDER_ADJUST > right_voltage)
    continue;

  uint8_t input = (left_input << 4) + right_input;

  if (!repeat) {
    if (input && hold) return 0;
    hold = input;
  }

  return input;
}

#endif
