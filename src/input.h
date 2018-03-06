#include <avr/io.h>
#include <stdbool.h>

#ifndef INPUT_H
#define INPUT_H

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
bool set = false;
bool repeat = false;

uint8_t Vborders[16];
uint8_t Vdefs[] = {
  255, 230, 209, 192, 178, 166, 156, 149, 142, 133, 125, 121, 116, 109, 105, 99
  //1023,923, 839, 770, 715, 664, 625, 598,
  //571, 532, 503, 486, 467, 438, 420, 397
};


void init_input(bool repeat) {
  repeat = repeat;

  // set control input pins
  DDRB &= ~(1 << PB4); //pinMode(RIGHT_INPUT, INPUT);
  DDRB &= ~(1 << PB3); //pinMode(LEFT_INPUT, INPUT);

  // set ADC prescaler
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

  // left adjust ADC result for 8 bit value
  ADMUX |= (1 << ADLAR);

  // enable ADC
  ADCSRA |= (1 << ADEN);

  for (uint8_t i = 0; i < 15; ++i)
    Vborders[i] = (Vdefs[i] + Vdefs[i+1]) / 2;
  Vborders[15] = 0;
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
  int8_t left_input = -1;
  while(Vborders[++left_input] > left_voltage)
    continue;

  uint8_t right_voltage = read_adc(RIGHT_INPUT);
  int8_t right_input = -1;
  while(Vborders[++right_input] > right_voltage)
    continue;

  uint8_t immediate_input = (left_input << 4) + right_input;

  if (immediate_input == prev_input)
    input = immediate_input;
  prev_input = immediate_input;

  if (!repeat) {
    if (input && set) return 0;
    set = input;
  }

  return input;
}

#endif
