char page;

unsigned long timers[6];
unsigned long t, dt, i;


#ifdef MPU
float zaccel;
float gravity;
#endif

uint32_t pressure;
float altitude;
float temperature;
float zVelocity;

char buttonState[3];
char sw_state[3];
unsigned long sw_time[3];
unsigned long toneStop;

char alt1;
float alt2;

void setup() {
  DDRD |= (1 << 7); //pin 7 to output
  PORTD |= (1 << 7); //pin 7 to high

  DDRD &= ~(1 << 2); //2, 3, 4 to input;
  DDRD &= ~(1 << 3);
  DDRD &= ~(1 << 4);

  PORTD |= (1 << 3) | (1 << 4); //pull up on pins 3,4

  toneAC(800);
  delay(100);
  toneAC(0);

  Serial.begin(115200);

#ifdef PIXEL
  pixels.begin();
#endif

  myGLCD.InitLCD();

  Wire.begin();

#ifdef MPU
  mpu.init();


  for (int i = 0; i < 30; i++)
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
#endif

  while (!ms5.begin(MS5611_ULTRA_HIGH_RES, 102000)) delay(100);

  pressure = ms5.readPressure(true);
  ms5.newQNH(EEPROMReadlong(1));

  altitude = ms5.simple_altitude(pressure);
  reg.lr_Init((long)altitude * 100, 20);

  alt1 = 1;
  alt2 = altitude;

  //kalAlt.init(altitude, 0, 0.2, 0.5, millis());

  page = 1;

}
