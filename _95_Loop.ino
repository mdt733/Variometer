void loop()
{
  t = millis();
  buttonState[0] = (PIND & (1 << 2)) >> 2; //digitalRead(2); //reverse pin 2 (low vs high)
  buttonState[1] = (~(PIND & (1 << 3)) >> 3) + 2;
  buttonState[2] = (~(PIND & (1 << 4)) >> 4) + 2; //shh

  for (int i = 0; i < 3; i++)
  {
    switch (sw_state[i])
    {
      case 0: //clear, no flags
        if (buttonState[i] == 1) //pressed in
        {
          sw_state[i] = 1;  //press time
          sw_time[i] = t;
        }
        break;

      case 1: // high. pre debounce
        if (buttonState[i] == 0) sw_state[i] = 0; //nevermind, too short a press
        if (t - sw_time[i] >= 5) //held in, make a noise, and go to state 2;
        {
          toneAC(1000);
          toneStop = t + 50;
          sw_state[i] = 2; //pressed
        }
        break;

      case 2: // high, post debounce
        if (buttonState[i] == 0) sw_state[i] = 4; //short press flag;
        if (t - sw_time[i] >= 600) //long hold, make a noise, and go to state 3;
        {
          //start higher tone
          toneAC(2000);
          toneStop = t + 50;
          sw_state[i] = 3; //pressed
        }
        break;

      case 3: //long hold
        if (buttonState[i] == 0) sw_state[i] = 5; //long press flag;
        if (i == 0)
        {
          if (t - sw_time[i] >= 3000)
          { PORTD = 0; //kill everything.
            toneAC(400);
          }
        }
        break;
    }
  }

  if (toneStop != 0)
  {
    if (millis() >= toneStop)
    {
      toneStop = 0;
      toneAC(0);
    }
  }

  t = millis();
  dt = t - timers[0];
  if (dt >= 500)
  { timers[0] = t;
    readVoltages();
    readPVVoltage();
    if (batteryVoltage <= 3.3f) PORTD = 0; //low voltage cut off
  }

#ifdef MPU
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
  }
#endif

  t = millis();
  dt = t - timers[3];
  if (dt >= 40) //process Barometer
  { timers[3] = t;
    pressure = ms5.readPressure(true);
    reg.lr_Sample(ms5.simple_altitude(pressure) * 100);
  }

  t = millis();
  dt = t - timers[4];
  if (dt >= 40) //process
  { timers[4] = t;
    temperature = ms5.readTemperature(true);
    reg.lr_CalculateSlope();
    reg.lr_CalculateAverage();
    altitude = reg.gZAverage*0.01f;
    zVelocity = reg.gSlope*0.01f;
  }

  t = millis();
  dt = t - timers[5];
  if (dt >= 50)
  { timers[5] = t;
    myGLCD.clrScr();
    switch (page)
    {
      //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% PAGE 1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
      case 1:
        myGLCD.setFont(b);

        if (alt1 == 1) myGLCD.printNumI(altitude, LEFT, 0, 4, '0');
        else          myGLCD.printNumI(altitude - alt2, LEFT, 0, 4, '0');

        if ((zVelocity >= 10) || (zVelocity <= -10))
        {
          myGLCD.printNumF(abs(zVelocity), 1, LEFT, 16);
        }
        else
        {
          if (zVelocity > 0)
          {
            myGLCD.print("/", LEFT, 16);
            myGLCD.printNumF(zVelocity, 1, 12, 16);
          }
          else
          {
            myGLCD.printNumF(zVelocity, 1, LEFT, 16, '.', 4, '-');
          }
        }

        myGLCD.setFont(SmallFont);
        myGLCD.print("12:34am  00:23", LEFT, 34);
        myGLCD.printNumI(temperature, 50, 0);
        myGLCD.print("~", 50 + 12, 0);

        myGLCD.setFont(TinyFont);
        myGLCD.printNumF(batteryVoltage, 1, 72, 2);
        myGLCD.printNumI(sw_state[2], RIGHT, 12);


        myGLCD.print("PAGE  |  VOL  | ALT", LEFT, 43);
        myGLCD.printNumI(2 - alt1, RIGHT, 43);

        myGLCD.drawLine(49, 0, 49, 42); //right side of big numbers
        myGLCD.drawLine(0, 42, 84, 42); //above button labels
        myGLCD.drawLine(0, 32, 84, 32); //below big buttons
        myGLCD.drawLine(49, 8, 84, 8); //below temperature
        myGLCD.drawLine(69, 0, 69, 8); //between temp and voltage

        if (sw_state[0] == 4) page = 2;
        if (sw_state[2] == 4) alt1 = 1 - alt1;
        
        if (sw_state[2] == 5)
        {
          if (alt1 == 1)
          { //go to qnh page
            page = 3;
          }
          else
          { //zero alt 2
            alt2 = altitude;
          }
        }
        break;
      //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% PAGE 2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
      case 2:
        myGLCD.setFont(SmallFont);
        myGLCD.printNumF(temperature, 2, LEFT, 0);
        myGLCD.printNumI(pressure, LEFT, 8);
        myGLCD.printNumF(PVVoltage, 2, LEFT, 16);
        myGLCD.print("12:34a", RIGHT, 0);
        myGLCD.print("17/Nov", RIGHT, 8);
        myGLCD.printNumF(batteryVoltage, 2, RIGHT, 16);
        myGLCD.drawLine(0, 42, 84, 42); //above button labels

        if (sw_state[0] == 4) page = 1;
        if (sw_state[0] == 5) page = 1;

        break;

      //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% PAGE 3 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
      case 3:
        myGLCD.setFont(SmallFont);
        myGLCD.print("QNH:", LEFT, 0);
        myGLCD.printNumI(ms5.QNH, RIGHT, 0);
        myGLCD.print("Alt:", LEFT, 10);
        myGLCD.printNumF(altitude, 1, RIGHT, 10);
        myGLCD.print("Pres", LEFT, 20);
        myGLCD.printNumI(pressure, RIGHT, 20);
        myGLCD.print("ALT2", LEFT, 30);
        myGLCD.printNumF(alt2, 1, RIGHT, 30);

        myGLCD.setFont(TinyFont);
        myGLCD.print(" OKAY |   -1  |   +1 ", LEFT, 43);
        myGLCD.drawLine(0, 42, 84, 42); //above button labels

        if (sw_state[0] == 4) page = 1;
        if (sw_state[0] == 5)
        {
          page = 1;
          EEPROMWritelong(1, ms5.QNH);
        }

        if (sw_state[1] == 4) ms5.newQNH(ms5.QNH - 10);
        if (sw_state[2] == 4) ms5.newQNH(ms5.QNH + 10);
        
        if (sw_state[1] == 3) ms5.newQNH(3 + ms5.QNH - (t - sw_time[i]) / 1000);
        if (sw_state[2] == 3) ms5.newQNH(-3 + ms5.QNH + (t - sw_time[i]) / 1000);
        
        break;
    }

    if (sw_state[0] == 4) sw_state[0] = 0;
    if (sw_state[0] == 5) sw_state[0] = 0;
    if (sw_state[1] == 4) sw_state[1] = 0;
    if (sw_state[1] == 5) sw_state[1] = 0;
    if (sw_state[2] == 4) sw_state[2] = 0;
    if (sw_state[2] == 5) sw_state[2] = 0;


    myGLCD.update();
  }
}
