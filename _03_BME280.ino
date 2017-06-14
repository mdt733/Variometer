/*#include "Arduino.h"
#include<Wire.h>

long LUT_pressure[] = {107476, 101325, 95461, 89876, 84558, 79498, 70112, 54025}; //Pa
long LUT_altitude[] = { 5000, 3000, 2000, 1500, 1000, 500, 0, -500}; //meters

struct table_1d altitude_LUT = {8, LUT_pressure, LUT_altitude};

class mBME
{
    uint16_t dig_T1, dig_P1;
    int16_t dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5;
    int16_t dig_P6, dig_P7, dig_P8, dig_P9, dig_H2, dig_H4, dig_H5;
    uint8_t dig_H1, dig_H3, dig_H6;
    uint32_t adc_t, adc_p, adc_h;
    float QNHm, QNHb;

    enum dataOrder_e {
      press_msb,
      press_lsb,
      press_xlsb,
      temp_msb,
      temp_lsb,
      temp_xlsb,
      hum_msb,
      hum_lsb,
    };

  public:
    int32_t temperature, t_fine;
    uint32_t pressure, humidity;
    float QNH;

    void newQNH(float _QNH) //Update LUT for new QNH
    {
      QNH = _QNH;
      for (int i = 0; i < 8; i++)
      {
        LUT_pressure[i] = findpressure(LUT_altitude[i], _QNH);
      }
    }
    void init(float _QNH = 1013.25f)
    {
      //work out qnh parameters;
      newQNH(_QNH);

      // Read calibration data.
      Wire.beginTransmission(0x76);
      Wire.write(0x88); //regCalibStart
      Wire.endTransmission();
      Wire.requestFrom(0x76, 26);
      dig_T1 = Wire.read() | Wire.read() << 8;
      dig_T2 = Wire.read() | Wire.read() << 8;
      dig_T3 = Wire.read() | Wire.read() << 8;
      dig_P1 = Wire.read() | Wire.read() << 8;
      dig_P2 = Wire.read() | Wire.read() << 8;
      dig_P3 = Wire.read() | Wire.read() << 8;
      dig_P4 = Wire.read() | Wire.read() << 8;
      dig_P5 = Wire.read() | Wire.read() << 8;
      dig_P6 = Wire.read() | Wire.read() << 8;
      dig_P7 = Wire.read() | Wire.read() << 8;
      dig_P8 = Wire.read() | Wire.read() << 8;
      dig_P9 = Wire.read() | Wire.read() << 8;

      Wire.beginTransmission(0x76);
      Wire.write(0xA1);  //adress of H1
      Wire.endTransmission();
      Wire.requestFrom(0x76, 1);
      dig_H1 = Wire.read();

      Wire.beginTransmission(0x76);
      Wire.write(0xeE1); //H2 address
      Wire.endTransmission();
      Wire.requestFrom(0x76, 7);
      dig_H2 = Wire.read() | (uint16_t)Wire.read() << 8;
      dig_H3 = Wire.read();
      dig_H4 = (uint16_t)Wire.read() << 4; // bits 11:4
      uint16_t regE5 = Wire.read();
      dig_H4 |= regE5 & 0b00001111; // bits 11:4
      dig_H5 = regE5 >> 4;
      uint16_t regE6 = Wire.read();
      dig_H5 |= regE6 << 4;
      dig_H6 = Wire.read();

      //setup (mode, filters, frequency)
      Wire.beginTransmission(0x76);
      Wire.write(0xF4);  //ctrl_meas byte
      Wire.write((0b010 << 5) | (0b101 << 2) | (0b11)); //0btttpppmm
      Wire.endTransmission();

      Wire.beginTransmission(0x76);
      Wire.write(0xF5);  //config byte
      Wire.write((0b000 << 5) | (0b100 << 2) | (0b00)); //0btttfff-3
      Wire.endTransmission();
    }


    void read()
    {
      uint8_t data[8];
      uint8_t count;
      Wire.beginTransmission(0x76);
      Wire.write(0xF7);//regMeasurementsStart
      Wire.endTransmission();
      Wire.requestFrom(0x76, 8);
      for (count = 0; count < 8; count++) {
        data[count] = Wire.read();
      }

      adc_h = data[hum_lsb];
      adc_h |= (uint32_t)data[hum_msb] << 8;

      adc_t  = (uint32_t)data[temp_xlsb] >> 4;
      adc_t |= (uint32_t)data[temp_lsb] << 4;
      adc_t |= (uint32_t)data[temp_msb] << 12;

      adc_p  = (uint32_t)data[press_xlsb] >> 4;
      adc_p |= (uint32_t)data[press_lsb] << 4;
      adc_p |= (uint32_t)data[press_msb] << 12;

      int32_t var1, var2;

      var1 = ((((adc_t >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
      var2 = (((((adc_t >> 4) - ((int32_t)dig_T1)) * ((adc_t >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
      t_fine = var1 + var2;
      temperature = (t_fine * 5 + 128) >> 8;

      var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
      var2 = (((var1 >> 2) * (var1 >> 2)) >> 11 ) * ((int32_t)dig_P6);
      var2 = var2 + ((var1 * ((int32_t)dig_P5)) << 1);
      var2 = (var2 >> 2) + (((int32_t)dig_P4) << 16);
      var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) * var1) >> 1)) >> 18;
      var1 = ((((32768 + var1)) * ((int32_t)dig_P1)) >> 15);
      if (var1 == 0) {
        //return 0; // avoid exception caused by division by zero
      }
      else
      {
        pressure = (((uint32_t)(((int32_t)1048576) - adc_p) - (var2 >> 12))) * 3125;
        if (pressure < 0x80000000) {
          pressure = (pressure << 1) / ((uint32_t)var1);
        } else {
          pressure = (pressure / (uint32_t)var1) * 2;
        }
        var1 = (((int32_t)dig_P9) * ((int32_t)(((pressure >> 3) * (pressure >> 3)) >> 13))) >> 12;
        var2 = (((int32_t)(pressure >> 2)) * ((int32_t)dig_P8)) >> 13;
        pressure = (uint32_t)((int32_t)pressure + ((var1 + var2 + dig_P7) >> 4));
      }
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

mBME bme;*/
