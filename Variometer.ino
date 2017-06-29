//#define MPU
//#define PIXEL

#include "EEPROM.h"
#include<Wire.h>
#include <toneAC.h>
#include <kalmanvert.h>

#include <LCD5110_Graph.h>

//kalmanvert kalAlt;

LCD5110 myGLCD(13, 11, 12, 8, 5);

extern uint8_t SmallFont[];
extern uint8_t b[];

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


float readv, batteryVoltage, PVVoltage;
void readVoltages(void)
{ readv = readVcc();
  batteryVoltage = 0.01f * (analogRead(A0) * readv) / 8270;
}

void readPVVoltage(void)
{
  PVVoltage = 0.01f * (analogRead(A7) * readv) / 5315;
}

long EEPROMReadlong(long address)
{
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void EEPROMWritelong(int address, long value)
{
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}
