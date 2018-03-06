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

int Vborders[16];
int Vdefs[] = {
  1023,923, 839, 770, 715, 664, 625, 598,
  571, 532, 503, 486, 467, 438, 420, 397
};

void init_input(bool repeat = true) {
  pinMode(RIGHT_INPUT, INPUT);
  pinMode(LEFT_INPUT, INPUT);
  repeat = repeat;

  for (uint8_t i = 0; i < 15; ++i)
    Vborders[i] = (Vdefs[i] + Vdefs[i+1]) / 2;
  Vborders[15] = 0;
}

uint8_t get_input() {

  int left_voltage = analogRead(LEFT_INPUT);
  int8_t left_input = -1;
  while(Vborders[++left_input] > left_voltage) continue;

  int right_voltage = analogRead(RIGHT_INPUT);
  int8_t right_input = -1;
  while(Vborders[++right_input] > right_voltage) continue;

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
