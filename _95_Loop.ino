
void loop()
{
  t = millis();
  buttonState[0] = (PIND & (1 << 2)) >> 2; //digitalRead(2); //reverse pin 2 (low vs high)

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
        if (t - sw_time[i] >= 5000) //power hold;
        {
          PORTD = 0; //kill everything.
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
  if (dt >= 50) //process
  { timers[4] = t;
    temper = ms5.readTemperature(true);
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
        myGLCD.setFont(MediumNumbers);
        myGLCD.printNumF(altitude, 1, RIGHT, 0);
        myGLCD.printNumF(-3.4f, 1, RIGHT, 16);
        //myGLCD.printNumF(sl, 1, RIGHT, 16);

        //myGLCD.setFont(SmallFont);
        //myGLCD.print("12:34", LEFT, 16);
        //myGLCD.print("2.3m/s", LEFT, 24);
        myGLCD.setFont(TinyFont);
        myGLCD.printNumF(batteryVoltage, 2, RIGHT, 36);
        //myGLCD.print("v|SNK0|V012", 18, 36);
        myGLCD.print("PAGE  |  VOL  |  ALT1", LEFT, 42);
       

        if (sw_state[0] == 4) //short button press
        {
          sw_state[0] = 0;
          page = 2;
        }

        if (sw_state[0] == 5)
        {
          sw_state[0] = 0;
        }


        break;
      case 2:
        myGLCD.setFont(SmallFont);
        myGLCD.print("page 2", CENTER, 20);


        if (sw_state[0] == 4)
        {
          sw_state[0] = 0;
        }
        if (sw_state[0] == 5)
        {
          sw_state[0] = 0;
          page = 1;
        }
        break;
    }

    myGLCD.update();
  }
}
