#define GYK  0.00053134110765f //gyro scale factor
#define ESHIFT 2

class mMPU
{
  public:

    int16_t mpuTmp;

    int16_t Ax_Raw, Ay_Raw, Az_Raw;
    int16_t Ax, Ay, Az; //averaged result

    int16_t Gx_Raw, Gy_Raw, Gz_Raw;
    int16_t Gx_Error, Gy_Error, Gz_Error;
    int16_t Gx, Gy, Gz; //Calibrated result
    float scaled_Gx, scaled_Gy, scaled_Gz;
    float scaled_Ax, scaled_Ay, scaled_Az;
    
    void read(void)    {
      Wire.beginTransmission(0x68);
      Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(0x68, 14, true); // request a total of 14 registers
      Ax = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
      Ay = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
      Az = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
      mpuTmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
      Gx_Raw = (Wire.read() << 8 | Wire.read()); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
      Gy_Raw = (Wire.read() << 8 | Wire.read()); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
      Gz_Raw = (Wire.read() << 8 | Wire.read()); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
      calibrateGyro();
      resolveGyro();
      resolveAccel();
    }

    void init(void)    {
      TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz
      Wire.beginTransmission(0x68);
      Wire.write(0x6B);  // PWR_MGMT_1 register
      Wire.write(0);     // set to zero (wakes up the MPU-6050)
      Wire.endTransmission(true);
      Wire.beginTransmission(0x68);
      Wire.write(0x1B);
      Wire.write(0b10 << 3); //FS_SEL, 00 = 250deg/s, 10 = 1000deg/s (1/32.8deg/s LSB)
      Wire.endTransmission(true);

      delay(10);
      read();
      zeroGyro();
      for (int i = 0; i < 10; i++)
      {
        delay(10);
        read();
        calibrateGyro();
      }
    }

    void zeroGyro()    {
      Gx_Error = Gx_Raw << ESHIFT; //calibrate gyroscope
      Gy_Error = Gy_Raw << ESHIFT;
      Gz_Error = Gz_Raw << ESHIFT;
    }

    void calibrateGyro()    {
      Gx_Error += ((Gx && 0xFF80) | ((!Gx + 1) && 0xFF80)) ? 0 : Gx; //calibrate if less then +|-128
      Gy_Error += ((Gy && 0xFF80) | ((!Gy + 1) && 0xFF80)) ? 0 : Gy;
      Gz_Error += ((Gz && 0xFF80) | ((!Gz + 1) && 0xFF80)) ? 0 : Gz;
    }

    void resolveGyro()
    {
      Gx = Gx_Raw - (Gx_Error >> ESHIFT);
      Gy = Gy_Raw - (Gy_Error >> ESHIFT);
      Gz = Gz_Raw - (Gz_Error >> ESHIFT);
      scaled_Gx = Gx * GYK;
      scaled_Gy = Gy * GYK;
      scaled_Gz = Gz * GYK;
    }

    void resolveAccel()
    {
      scaled_Ax = map_float(Ax, -7815, 8500, -5, 5);
      scaled_Ay = map_float(Ay, -8200, 8075, -5, 5);
      scaled_Az = map_float(Az, -9600, 7150, -5, 5);
    }
  private:
};

mMPU mpu;
