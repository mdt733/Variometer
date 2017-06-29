#ifdef MPU
class mIMU
{
    float twoKp = 1.0f, twoKi = 0.2f;
    float integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f;

    float invSqrt(float x)
    {
      float halfx = 0.5f * x;
      float y = x;
      long i = *(long*)&y;
      i = 0x5f3759df - (i >> 1);
      y = *(float*)&i;
      y = y * (1.5f - (halfx * y * y));
      return y;
    }
    
  public:
    float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;  // quaternion of sensor frame
    float Ex, Ey, Ez, Z_accel;

    void q_normalise()
    {
      float recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
      q0 *= recipNorm;
      q1 *= recipNorm;
      q2 *= recipNorm;
      q3 *= recipNorm;
    }

    MadgwickGyro(float gx, float gy, float gz, float dt) {
      //q += (q*quaternion(0,q_Error))*(dt*0.5);
      float halfdt = 0.5f * dt;
      q0 += (-q1 * gx - q2 * gy - q3 * gz) * (halfdt);
      q1 += (q0 * gx + q2 * gz - q3 * gy) * (halfdt);
      q2 += (q0 * gy - q1 * gz + q3 * gx) * (halfdt);
      q3 += (q0 * gz + q1 * gy - q2 * gx) * (halfdt);
    }

    void MahonyAcc(float ax, float ay, float az, float dt) {
      float recipNorm;
      float halfvx, halfvy, halfvz;
      float halfex, halfey, halfez;
      float qa, qb, qc;
      float gx, gy, gz;

      // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
      if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

        // Normalise accelerometer measurement
        recipNorm = invSqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // Estimated direction of gravity and vector perpendicular to magnetic flux(north)
        halfvx = q1 * q3 - q0 * q2;
        halfvy = q0 * q1 + q2 * q3;
        halfvz = q0 * q0 - 0.5f + q3 * q3;

        // Error is sum of cross product between estimated and measured direction of gravity
        halfex = (ay * halfvz - az * halfvy);
        halfey = (az * halfvx - ax * halfvz);
        halfez = (ax * halfvy - ay * halfvx);

        // Compute and apply integral feedback
        integralFBx += twoKi * halfex * dt; // integral error scaled by Ki
        integralFBy += twoKi * halfey * dt;
        integralFBz += twoKi * halfez * dt;
        gx = integralFBx; // apply integral feedback
        gy = integralFBy;
        gz = integralFBz;

        // Apply proportional feedback
        gx += twoKp * halfex;
        gy += twoKp * halfey;
        gz += twoKp * halfez;
      }
      MadgwickGyro(gx, gy, gz, dt); //apply to rotation quat
      q_normalise();
    }

    float euler_x(void) {
      return Ex =  57.29578f * atan2f(q0 * q1 + q2*q3, 0.5f - q1 * q1 - q2*q2);
    }
    float euler_y(void) {
      return Ey =  57.29578f * asinf(-2.0f * (q1 * q3 - q0*q2));
    }
    float euler_z(void) {
      return Ez =  57.29578f * atan2f(q1 * q2 + q0*q3, 0.5f - q2 * q2 - q3*q3);
    }

    void euler()
    {
      euler_x();
      euler_y();
      euler_z();
    }

    float imu_GravityCompensatedAccel(float ax, float ay, float az)
    {
      return Z_accel = 2.0f * (q1 * q3 - q0 * q2) * ax + 2.0f * (q0 * q1 + q2 * q3) * ay + (q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3) * az;
    }
};

mIMU imu;

#endif
