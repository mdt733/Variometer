void loop()
{
  t = millis();
  buttonState[0] = (PIND & (1 << 2)) >> 2; //digitalRead(2); //reverse pin 2 (low vs high)
  buttonState[1] = 1 - (PIND & (1 << 3)) >> 3;
  buttonState[2] = 1 - (PIND & (1 << 4)) >> 4;

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
        if (t - sw_time[i] >= 3000)
        { PORTD = 0; //kill everything.
          toneAC(400);
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

  t = millis();
  dt = t - timers[3];
  if (dt >= 40) //process Barometer
  { timers[3] = t;
    p = ms5.readPressure(true);
    altitude = ms5.simple_altitude(p);
    //reg.lr_Sample(altitude * 100);
  }

  t = millis();
  dt = t - timers[4];
  if (dt >= 40) //process
  { timers[4] = t;
    temper = ms5.readTemperature(true);
    kalAlt.update(altitude, (zaccel - gravity), millis());
    //reg.lr_CalculateAverage();
    //reg.lr_CalculateSlope();
    //sl = 0.9 * sl + 0.1 * (-0.01 * reg.gSlope);
  }

  t = millis();
  dt = t - timers[5];
  if (dt >= 20)
  { timers[5] = t;

    myGLCD.clrScr();
    switch (page)
    {
      case 1:
        myGLCD.setFont(b);
        if (alt1)
        {
          myGLCD.printNumI(altitude, LEFT, 0, 4, '0');

        }
        else
        {
          myGLCD.printNumI(altitude - alt2, LEFT, 0, 4, '0');

        }

        //myGLCD.printNumF(-3.4f, 1, RIGHT, 16);

        if ((kalAlt.getVelocity() >= 10) || (kalAlt.getVelocity() <= -10))
        {
          myGLCD.printNumF(abs(kalAlt.getVelocity()), 1, LEFT, 16);
        }
        else
        {
          if (kalAlt.getVelocity() > 0)
          {
            myGLCD.print("/", LEFT, 16);
            myGLCD.printNumF(kalAlt.getVelocity(), 1, 12, 16);
          }
          else
          {
            myGLCD.printNumF(kalAlt.getVelocity(), 1, LEFT, 16, '.', 4, '-');
          }
        }

        myGLCD.setFont(SmallFont);
        myGLCD.print("12:34am  00:23", LEFT, 34);
        myGLCD.printNumI(temper, 50, 0);
        myGLCD.print("~", 50 + 12, 0);

        myGLCD.setFont(TinyFont);
        myGLCD.printNumF(batteryVoltage, 1, 72, 2);
        //myGLCD.printNumF(PVVoltage, 1, RIGHT, 6);

        myGLCD.print("PAGE  |  VOL  | ALT 1", LEFT, 43);

        myGLCD.drawLine(49, 0, 49, 42); //right side of big numbers
        myGLCD.drawLine(0, 42, 84, 42); //above button labels
        myGLCD.drawLine(0, 32, 84, 32); //below big buttons
        myGLCD.drawLine(49, 8, 84, 8); //below temperature
        myGLCD.drawLine(69, 0, 69, 8); //between temp and voltage

        if (sw_state[0] == 4) //short button press
        {
          sw_state[0] = 0;
          page = 2;
        }

        if (sw_state[0] == 5) sw_state[0] = 0;
        if (sw_state[1] == 4) sw_state[1] = 0;
        if (sw_state[1] == 5) sw_state[1] = 0;

        if (sw_state[2] == 4) //toggle altitude view
        {
          alt1 = ~alt1;
          sw_state[2] = 0;
        }

        if (sw_state[2] == 5)
        {
          if (alt1)
          { //go to qnh page
            page = 3;
          }
          else
          { //zero alt 2
            alt2 = altitude;
          }
          sw_state[2] = 0;
        }


        break;
      case 2:
        myGLCD.setFont(SmallFont);
        myGLCD.printNumF(temper, 2, LEFT, 0);
        myGLCD.printNumI(p, LEFT, 8);
        myGLCD.printNumF(PVVoltage, 3, LEFT, 16);
        myGLCD.print("12:34am", RIGHT, 0);
        myGLCD.print("17/Nov", RIGHT, 8);
        myGLCD.printNumF(batteryVoltage, 3, RIGHT, 16);
        myGLCD.drawLine(0, 42, 84, 42); //above button labels

        if (sw_state[0] == 4) {
          sw_state[0] = 0;
        }
        if (sw_state[0] == 5)
        {
          sw_state[0] = 0;
          page = 1;
        }

        if (sw_state[1] == 4) sw_state[1] = 0;
        if (sw_state[1] == 5) sw_state[1] = 0;
        if (sw_state[2] == 4) sw_state[2] = 0;
        if (sw_state[2] == 5) sw_state[2] = 0;

        break;
      case 3:
        myGLCD.setFont(SmallFont);
        myGLCD.print("QNH:", LEFT, 0);
        myGLCD.printNumI(ms5.QNH, RIGHT, 0);
        myGLCD.print("Alt:", LEFT, 10);
        myGLCD.printNumF(altitude, 1, RIGHT, 10);
        myGLCD.print("Pres", LEFT, 20);
        myGLCD.printNumI(p, RIGHT, 20);

        //pres
        //temp


        myGLCD.setFont(TinyFont);
        myGLCD.print(" OKAY |   -1  |   +1 ", LEFT, 43);
        myGLCD.drawLine(0, 42, 84, 42); //above button labels

        if (sw_state[0] == 4) {
          sw_state[0] = 0;
        }
        if (sw_state[0] == 5)
        {
          sw_state[0] = 0;
          page = 1;
        }
        if (sw_state[1] == 4)
        {
          ms5.newQNH(ms5.QNH - 100);
          sw_state[1] = 0;
        }
        if (sw_state[1] == 5) sw_state[1] = 0;
        if (sw_state[2] == 4)
        {
          ms5.newQNH(ms5.QNH + 100);
          sw_state[2] = 0;
        }
        if (sw_state[2] == 5) sw_state[2] = 0;
        break;
    }
    myGLCD.update();
  }
}
