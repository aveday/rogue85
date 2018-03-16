#include <avr/io.h>
#include <stdbool.h>
#ifndef INPUT_H
#define INPUT_H

#define SAMPLES 12
#define INPUT_SENSITIVITY 9

#define UP 1<<4
#define DOWN 1<<7
#define LEFT 1<<6
#define RIGHT 1<<5
#define A 1<<3
#define B 1<<2
#define X 1<<1
#define Y 1<<0

/* n-button rollover
uint8_t Vdefs[] = {
  255, 230, 209, 192, 178, 166, 156, 149,
  142, 133, 125, 121, 116, 109, 105, 99
};
*/
const uint8_t Vdefs[] = { 230, 209, 178, 142 };

void init_input() {

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

uint8_t get_input(uint8_t channel) {
  //TODO input using interrupts

  uint8_t voltage = ~0;
  for (uint8_t i = 0; i < SAMPLES; ++i) {
    uint8_t sample = read_adc(channel);
    voltage = sample < voltage ? sample : voltage;
  }

  int8_t input = 0;
  for (uint8_t i = 0; i < 4; ++i)
    if (voltage < Vdefs[i] + INPUT_SENSITIVITY &&
        voltage > Vdefs[i] - INPUT_SENSITIVITY )
      input = 1 << i;

  return input;
}

#endif
