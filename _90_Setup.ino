float gravity;
uint32_t p;
char page;

unsigned long timers[6];
unsigned long t, dt, i;

float baroAlt, baroVel;
float altitude;
float zaccel;
float accVel;
float filVel;

float temper;
float alti;

//char page;


char buttonState[3];
char sw_state[3];
unsigned long sw_time[3];
unsigned long toneStop;
float sl;


void setup() {
  DDRD |= (1 << 7); //pin 7 to output
  DDRD &= ~(1 << 2); //2, 3, 4 to input;
  DDRD &= ~(1 << 3);
  DDRD &= ~(1 << 4);

  PORTD |= (1 << 7); //pin 7 to high
  PORTD |= (1 << 3) | (1 << 4); //pull up on pins 3,4

  toneAC(800);
  delay(100);
  toneAC(0);

  Serial.begin(115200);
  pixels.begin();
  Wire.begin();
  mpu.init();

  myGLCD.InitLCD();

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

  while (!ms5.begin(MS5611_ULTRA_HIGH_RES, 102000)) delay(100);

  p = ms5.readPressure(true);
  ms5.newQNH(p + 2000);

  altitude = ms5.simple_altitude(p);
 // reg.lr_Init((long)altitude * 100,5);

  kalAlt.init(altitude, 0, 0.2, 0.3, millis());

  page = 1;

}
