float gravity;

void setup() {
  Serial.begin(115200);
  pixels.begin();
  Wire.begin();
  mpu.init();

  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);

  for (int i = 0; i < 50; i++)
  {
    mpu.read();
    imu.MahonyAcc(mpu.Ax_Raw, mpu.Ay_Raw, mpu.Az_Raw, 0.5f);
    delay(5);
  }

  gravity = imu.imu_GravityCompensatedAccel(mpu.scaled_Ax, mpu.scaled_Ay, mpu.scaled_Az);
  for (int i = 0; i < 25; i++)
  {
    mpu.read();
    imu.MahonyAcc(mpu.Ax_Raw, mpu.Ay_Raw, mpu.Az_Raw, 0.1f);
    gravity = 0.95 * gravity + 0.05 * imu.imu_GravityCompensatedAccel(mpu.scaled_Ax, mpu.scaled_Ay, mpu.scaled_Az);
    delay(5);
  }

  while(!ms5.begin(MS5611_ULTRA_HIGH_RES,1020.0f)) delay(100);

  //reg.lr_Init((long)bme.simple_altitude(bme.pressure) * 100, 20);

  pinMode(7, OUTPUT); //power enable
  pinMode(2, INPUT); //switch 1, power.
  pinMode(3, INPUT_PULLUP); //switch 2
  pinMode(4, INPUT_PULLUP); //switch 3

  digitalWrite(7, HIGH);
  toneAC(800);
  delay(100);
  toneAC(0);
}
