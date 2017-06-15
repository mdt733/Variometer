unsigned long timers[6];
unsigned long t, dt, i;

float baroAlt, baroVel;
float altitude;
float zaccel;
float accVel;
float filVel;

float temper;
float alti;

void loop()
{
  digitalWrite(7, digitalRead(4));
  t = millis();
  dt = t - timers[0];
  if (dt >= 100)
  { timers[0] = t;
    readVoltages();
    if (batteryVoltage <= 3.3f) digitalWrite(7, LOW); //low voltage cut off
  }

  t = millis();
  dt = t - timers[1];
  if (dt >= 10) //process gyroscope. (100hz), and buttons
  { timers[1] = t;
    mpu.read();
    imu.MadgwickGyro(mpu.scaled_Gx, mpu.scaled_Gy, mpu.scaled_Gz, 0.001f * dt);
  }

  t = millis();
  dt = t - timers[2];
  if (dt >= 25) //process accelerometer
  { timers[2] = t;
    imu.MahonyAcc(mpu.scaled_Ax, mpu.scaled_Ay, mpu.scaled_Az, 0.001f * dt);
    zaccel = imu.imu_GravityCompensatedAccel(mpu.scaled_Ax, mpu.scaled_Ay, mpu.scaled_Az);
    gravity = 0.99 * gravity + 0.01 * zaccel;
    accVel += (zaccel - gravity) * dt * 0.001;
    //accVel = 0.9 * accVel + 0.1 * baroVel;
  }

  t = millis();
  dt = t - timers[3];
  if (dt >= 40) //process Barometer
  { timers[3] = t;
  p = ms5.readPressure(true);
  
    //bme.read();
    //altitude = bme.simple_altitude(bme.pressure);
    reg.lr_Sample(p*10);
  }

  t = millis();
  dt = t - timers[4];
  if (dt >= 50) //process
  { timers[4] = t;
    temper = ms5.readTemperature(true);
    reg.lr_CalculateAverage();
    //reg.lr_CalculateSlope();
    //baroAlt = 0.01f * reg.gZAverage;
    //baroVel = 0.01f * reg.gSlope;
  }

  t = millis();
  dt = t - timers[5];
  if (dt >= 20)
  { timers[5] = t;
    /*Serial.print(""); Serial.print(batteryVoltage, 2);
    Serial.print(","); Serial.print(baroAlt);
    Serial.print(","); Serial.print(baroVel);
    Serial.print(","); Serial.print(zaccel - gravity);
    Serial.print(","); Serial.print(accVel);


    Serial.print(",-1,0,1");
    Serial.println();
*/
    myGLCD.clrScr();
    myGLCD.setFont(TinyFont);
   // myGLCD.print("volt:", LEFT, 0);
    myGLCD.print("s:", LEFT, 0);
    myGLCD.printNumI(digitalRead(2),8,0);
    myGLCD.printNumI(digitalRead(3),12,0);
    myGLCD.printNumI(digitalRead(4),16,0);
    myGLCD.printNumI(millis()/100,0,6);
   // myGLCD.printNumF(temper,2,0,12);
    
    
    myGLCD.setFont(SmallFont);
    myGLCD.printNumF(batteryVoltage,2,RIGHT,0);
    myGLCD.printNumF(temper,2,RIGHT,8);
    myGLCD.printNumI(p,RIGHT,16);
    myGLCD.printNumF(ms5.simple_altitude(reg.gZAverage*0.1f),1,RIGHT,24);
    
    myGLCD.update();

    i = (int) map_float(batteryVoltage, 3.2, 4.2, 0, 255);
    colourBar(i);
  }
}
