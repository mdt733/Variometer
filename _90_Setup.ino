float gravity;
uint32_t p;

void setup() {
  DDRD |= (1<<7); //pin 7 to output
  DDRD &= ~(1<<2); //2, 3, 4 to input;
  DDRD &= ~(1<<3);
  DDRD &= ~(1<<4);

  PORTD |= (1<<7); //pin 7 to high
  PORTD |= (1<<3)|(1<<4); //pull up on pins 3,4

  toneAC(800);
  delay(100);
  toneAC(0);

  Serial.begin(115200);
  pixels.begin();
  Wire.begin();
  mpu.init();

  myGLCD.InitLCD();

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

  while (!ms5.begin(MS5611_ULTRA_HIGH_RES, 102000)) delay(100);

  p = ms5.readPressure(true);
  ms5.newQNH(p);
  reg.lr_Init(p*10,10);
  //reg.lr_Init((long)bme.simple_altitude(bme.pressure) * 100, 20);


}
