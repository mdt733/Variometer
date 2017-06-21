#include<Wire.h>
#include <toneAC.h>
#include <kalmanvert.h>

#include <LCD5110_Graph.h>

kalmanvert kalAlt;

LCD5110 myGLCD(13,11,12,8,5);

extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];
extern unsigned char TinyFont[];

float map_float(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

long readVcc() {
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both
  long result = (high << 8) | low;
  return result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
}


float readv, batteryVoltage;
void readVoltages(void)
{ readv = readVcc();
  batteryVoltage = 0.01f * (analogRead(A0) * readv) / 8270;
}
