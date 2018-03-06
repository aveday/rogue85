int previous_time = 0;

uint8_t time_step() {
  const uint8_t dt = millis() - previous_time;
  delay(max(0, 1000 / FPS - dt));
  previous_time = millis();
  return dt;
}
