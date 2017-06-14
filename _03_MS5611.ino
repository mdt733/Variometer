#include "Arduino.h"
#include<Wire.h>

#define MS5611_ADDRESS                (0x77)

#define MS5611_CMD_ADC_READ           (0x00)
#define MS5611_CMD_RESET              (0x1E)
#define MS5611_CMD_CONV_D1            (0x40)
#define MS5611_CMD_CONV_D2            (0x50)
#define MS5611_CMD_READ_PROM          (0xA2)

typedef enum
{
  MS5611_ULTRA_HIGH_RES   = 0x08,
  MS5611_HIGH_RES         = 0x06,
  MS5611_STANDARD         = 0x04,
  MS5611_LOW_POWER        = 0x02,
  MS5611_ULTRA_LOW_POWER  = 0x00
} ms5611_osr_t;

long LUT_pressure[] = {107476, 101325,  89876,  79498, 70112, 54025}; //Pa
long LUT_altitude[] = {  5000,   3000,   1500,    500,     0,  -500}; //meters

struct table_1d altitude_LUT = {6, LUT_pressure, LUT_altitude};

class mMS5611
{
  public:
    uint16_t fc[6];
    uint8_t ct;
    uint8_t uosr;
    int32_t TEMP2;
    int64_t OFF2, SENS2;

    int32_t temperature;
    uint32_t pressure;
    float QNH;

    void newQNH(float _QNH) //Update LUT for new QNH
    {
      QNH = _QNH;
      for (int i = 0; i < 6; i++)
      {
        LUT_pressure[i] = findpressure(LUT_altitude[i], _QNH);
      }
    }

    // Read 16-bit from register (oops MSB, LSB)
    uint16_t readRegister16(uint8_t reg)
    {
      uint16_t value;
      Wire.beginTransmission(MS5611_ADDRESS);
      Wire.write(reg);
      Wire.endTransmission();

      Wire.beginTransmission(MS5611_ADDRESS);
      Wire.requestFrom(MS5611_ADDRESS, 2);
      while (!Wire.available()) {};
      uint8_t vha = Wire.read();
      uint8_t vla = Wire.read();
      Wire.endTransmission();

      value = vha << 8 | vla;

      return value;
    }
    // Read 24-bit from register (oops XSB, MSB, LSB)
    uint32_t readRegister24(uint8_t reg)
    {
      uint32_t value;
      Wire.beginTransmission(MS5611_ADDRESS);
      Wire.write(reg);
      Wire.endTransmission();

      Wire.beginTransmission(MS5611_ADDRESS);
      Wire.requestFrom(MS5611_ADDRESS, 3);
      while (!Wire.available()) {};
      uint8_t vxa = Wire.read();
      uint8_t vha = Wire.read();
      uint8_t vla = Wire.read();
      Wire.endTransmission();

      value = ((int32_t)vxa << 16) | ((int32_t)vha << 8) | vla;

      return value;
    }

    bool begin(ms5611_osr_t osrfloat, float _QNH = 1013.25f)
    {
      //work out qnh parameters;
      newQNH(_QNH);

      Wire.begin();
      reset();
      setOversampling(0x08);
      delay(100);
      readPROM();
      return true;
    }

    // Set oversampling value
    void setOversampling(ms5611_osr_t osr)
    {
      switch (osr)
      {
        case MS5611_ULTRA_LOW_POWER:
          ct = 1;
          break;
        case MS5611_LOW_POWER:
          ct = 2;
          break;
        case MS5611_STANDARD:
          ct = 3;
          break;
        case MS5611_HIGH_RES:
          ct = 5;
          break;
        case MS5611_ULTRA_HIGH_RES:
          ct = 10;
          break;
      }

      uosr = osr;
    }

    void reset(void)
    {
      Wire.beginTransmission(MS5611_ADDRESS);
      Wire.write(MS5611_CMD_RESET);
      Wire.endTransmission();
    }

    void readPROM(void)
    {
      for (uint8_t offset = 0; offset < 6; offset++)
      {
        fc[offset] = readRegister16(MS5611_CMD_READ_PROM + (offset * 2));
      }
    }

    uint32_t readRawTemperature(void)
    {
      Wire.beginTransmission(MS5611_ADDRESS);
      Wire.write(MS5611_CMD_CONV_D2 + uosr);
      Wire.endTransmission();
      delay(ct);
      return readRegister24(MS5611_CMD_ADC_READ);
    }

    uint32_t readRawPressure(void)
    {
      Wire.beginTransmission(MS5611_ADDRESS);
      Wire.write(MS5611_CMD_CONV_D1 + uosr);
      Wire.endTransmission();
      delay(ct);
      return readRegister24(MS5611_CMD_ADC_READ);
    }

    int32_t readPressure(bool compensation)
    {
      uint32_t D1 = readRawPressure();
      uint32_t D2 = readRawTemperature();
      int32_t dT = D2 - (uint32_t)fc[4] * 256;

      int64_t OFF = (int64_t)fc[1] * 65536 + (int64_t)fc[3] * dT / 128;
      int64_t SENS = (int64_t)fc[0] * 32768 + (int64_t)fc[2] * dT / 256;

      if (compensation)
      {
        int32_t TEMP = 2000 + ((int64_t) dT * fc[5]) / 8388608;
        OFF2 = 0;
        SENS2 = 0;

        if (TEMP < 2000)
        {
          OFF2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 2;
          SENS2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 4;
        }

        if (TEMP < -1500)
        {
          OFF2 = OFF2 + 7 * ((TEMP + 1500) * (TEMP + 1500));
          SENS2 = SENS2 + 11 * ((TEMP + 1500) * (TEMP + 1500)) / 2;
        }

        OFF = OFF - OFF2;
        SENS = SENS - SENS2;
      }

      pressure = (D1 * SENS / 2097152 - OFF) / 32768;
      return pressure;
    }

    double readTemperature(bool compensation)
    {
      uint32_t D2 = readRawTemperature();
      int32_t dT = D2 - (uint32_t)fc[4] * 256;

      int32_t TEMP = 2000 + ((int64_t) dT * fc[5]) / 8388608;

      TEMP2 = 0;

      if (compensation)
      {
        if (TEMP < 2000)
        {
          TEMP2 = (dT * dT) / (2 << 30);
        }
      }

      TEMP = TEMP - TEMP2;
      temperature = ((double)TEMP / 100);
      return temperature;
    }

    float simple_altitude(float _pressure) //lookup table altitude
    {
      return interpolate_table_1d(&altitude_LUT, _pressure);
    }

    float calc_altitude(float _pressure, float _QNH = 1013.25)
    {
      return 44330.769 * (1 - pow((float)_pressure / _QNH, (0.1902949572)));
    }

    float findQNH(float _pressure, float _altitude)
    {
      return _pressure / pow(1 - (_altitude / 44330.769), 5.255);
    }

    float findpressure(float _altitude, float _QNH = 1013.25)
    {
      return 100 * _QNH * pow(1 - (_altitude / 44330.769), 5.255);
    }
};

mMS5611 ms5;
