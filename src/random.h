#include <math.h>
#include <stdint.h>

uint8_t weighted_choice(uint8_t weights[], uint8_t n_items) {
  uint16_t weight_sum = 0;
  for (uint8_t i = 0; i < n_items; ++i)
    weight_sum += weights[i];

  uint16_t choice = rand() % weight_sum;
  for (uint8_t i = 0; i < n_items; ++i) {
    if (choice < weights[i])
      return i;
    choice -= weights[i];
  }

  return 0;
}
