
char *ItoaPadded(int val, char *str, uint8_t bytes, uint8_t decimalpos)  {
  uint8_t neg = 0;
  if(val < 0) {
    neg = 1;
    val = -val;
  }

  str[bytes] = 0;
  for(;;) {
    if(bytes == decimalpos) {
      str[--bytes] = DECIMAL;
      decimalpos = 0;
    }
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    if(bytes == 0 || (decimalpos == 0 && val == 0))
      break;
  }

  if(neg && bytes > 0)
    str[--bytes] = '-';

  while(bytes != 0)
    str[--bytes] = ' ';

  return str;
}

char *FormatGPSCoord(int32_t val, char *str, uint8_t p, char pos, char neg) {
  if(val < 0) {
    pos = neg;
    val = -val;
  }

  uint8_t bytes = p+8;

  str[bytes] = 0;
  str[--bytes] = pos;
  for(;;) {
    if(bytes == p) {
      str[--bytes] = DECIMAL;
      continue;
    }
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    if(bytes < 3 && val == 0)
       break;
   }

   while(bytes != 0)
     str[--bytes] = ' ';

   return str;
}

// Take time in Seconds and format it as 'SMM:SS' or ' SM:SS',
// Return pointer to minute digit,
char *formatTime(uint16_t val, char *str) {
  int8_t bytes = 5;
  str[bytes] = 0;
  str[--bytes] = '0' + (val % 10);
  val = val / 10;
  str[--bytes] = '0' + (val % 6);
  val = val / 6;
  str[--bytes] = ':';
  do {
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
  } while(val != 0 && bytes != 0);

  while(bytes != 0)
     str[--bytes] = ' ';

  return str;
}

uint8_t FindNull(void)
{
  uint8_t xx;
  for(xx=0;screenBuffer[xx]!=0;xx++)
    ;
  return xx;
}

void displayTemperature(void)                           // WILL WORK ONLY WITH V1.2
{
  if (unitSystem)
    temperature = temperature*1.8+32;       //Fahrenheit conversion for imperial system.

  if(temperature > temperMAX)
    temperMAX = temperature;

  itoa(temperature,screenBuffer,10);
  uint8_t xx = FindNull();   // find the NULL
  screenBuffer[xx++]=temperatureUnitAdd[unitSystem];
  screenBuffer[xx]=0;                                   // Restore the NULL
  MAX7456_WriteString(screenBuffer,getPosition(temperaturePosition));
}

void displayMode(void)
{
  // Put sensor symbold (was displaySensors)
  screenBuffer[0] = (MwSensorPresent&ACCELEROMETER) ? 0xa0 : ' ';
  screenBuffer[1] = (MwSensorPresent&BAROMETER) ? 0xa2 : ' ';
  screenBuffer[2] = (MwSensorPresent&MAGNETOMETER) ? 0xa1 : ' ';
  screenBuffer[3] = (MwSensorPresent&GPSSENSOR) ? 0xa3 : ' ';

  if(MwSensorActive&STABLEMODE)
  {
    screenBuffer[4]=0xac;
    screenBuffer[5]=0xad;
  }
  else
  {
    screenBuffer[4]=0xae;
    screenBuffer[5]=0xaf;
  }
  screenBuffer[6]=' ';
  if(MwSensorActive&GPSHOMEMODE)
    screenBuffer[7]=0xff;
  else if(MwSensorActive&GPSHOLDMODE)
    screenBuffer[7]=0xef;
  else if(GPS_fix)
    screenBuffer[7]=0xdf;
  else
    screenBuffer[7]=' ';
  screenBuffer[8]=0;
  MAX7456_WriteString(screenBuffer,getPosition(sensorPosition));

  // Put ON indicator under sensor symbol
  screenBuffer[0] = (MwSensorActive&STABLEMODE) ? 0xBE : ' ';
  screenBuffer[1] = (MwSensorActive&BAROMODE) ? 0xBE : ' ';
  screenBuffer[2] = (MwSensorActive&MAGMODE) ? 0xBE : ' ';
  screenBuffer[3] = (MwSensorActive&(GPSHOMEMODE|GPSHOLDMODE)) ? 0xBE : ' ';
  screenBuffer[4] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(sensorPosition)+LINE);
}

void displayArmed(void)
{
  armed = (MwSensorActive&ARMEDMODE);
  if(armedTimer==0)
    MAX7456_WriteString_P(disarmed_text, getPosition(motorArmedPosition));
  else if((armedTimer>1) && (armedTimer<9) && (Blink10hz))
    MAX7456_WriteString_P(armed_text, getPosition(motorArmedPosition));
}

void displayHorizonPart(int X,int Y,int roll)
{
  // Roll Angle will be between -45 and 45 this is converted to 0-56 to fit with DisplayHorizonPart function
  X=X*(0.6)+28;
  if(X>56) X=56;
  if(X<0) X=0;
  // 7 row, 8 lines per row, mean 56 different case per segment, 2 segment now
  int xx=X/8;
  switch (xx)
  {
  case 0:
    screen[(roll*30)+100+Y]=0x10+(X);
    break;
  case 1:
    screen[(roll*30)+130+Y]=0x10+(X-8);
    break;
  case 2:
    screen[(roll*30)+160+Y]=0x10+(X-16);
    break;
  case 3:
    screen[(roll*30)+190+Y]=0x10+(X-24);
    break;
  case 4:
    screen[(roll*30)+220+Y]=0x10+(X-32);
    break;
  case 5:
    screen[(roll*30)+250+Y]=0x10+(X-40);
    break;
  case 6:
    screen[(roll*30)+280+Y]=0x10+(X-48);
    break;
  }
}

void displayHorizon(short rollAngle, short pitchAngle)
{
  rollAngle = rollAngle / 10;
  pitchAngle = pitchAngle /10;
  if(pitchAngle>25) pitchAngle=25;
  if(pitchAngle<-20) pitchAngle=-20;
  if(rollAngle>40) rollAngle=40;
  if(rollAngle<-40) rollAngle=-40;
  pitchAngle = pitchAngle /5;

  displayHorizonPart(rollAngle,0,pitchAngle );
  displayHorizonPart(rollAngle*0.75,1,pitchAngle );
  displayHorizonPart(rollAngle*0.5,2,pitchAngle );
  displayHorizonPart(rollAngle*0.25,3,pitchAngle );
  displayHorizonPart(0,4,pitchAngle );
  displayHorizonPart(-1*rollAngle*0.25,5,pitchAngle );
  displayHorizonPart(-1*rollAngle*0.5,6,pitchAngle );
  displayHorizonPart(-1*rollAngle*0.75,7,pitchAngle );
  displayHorizonPart(-1*rollAngle,8,pitchAngle );

#if defined(DISPLAY_HORIZON_BR)
  //Draw center screen
  screen[219-30]=0x03;
  screen[224-30-1]=0x1D;
  screen[224-30+1]=0x1D;
  screen[224-30]=0x01;
  screen[229-30]=0x02;
#if defined WITHDECORATION
  screen[128]=0xC7;
  screen[128+30]=0xC7;
  screen[128+60]=0xC7;
  screen[128+90]=0xC7;
  screen[128+120]=0xC7;
  screen[128+12]=0xC6;
  screen[128+12+30]=0xC6;
  screen[128+12+60]=0xC6;
  screen[128+12+90]=0xC6;
  screen[128+12+120]=0xC6;
#endif
#endif
}

void displayVoltage(void)
{
if (VIDVOLTAGE_VBAT){
  vidvoltage=MwVBat;
}

  if (MAINVOLTAGE_VBAT){
    voltage=MwVBat;
  }


  ItoaPadded(voltage, screenBuffer, 4, 3);
  screenBuffer[4] = voltageUnitAdd;
  screenBuffer[5] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(voltagePosition));

#if defined SHOWBATLEVELEVOLUTION
  if (voltage < 105) screenBuffer[0]=0x96;
  else if (voltage < 108) screenBuffer[0]=0x95;
  else if (voltage < 110) screenBuffer[0]=0x94;
  else if (voltage < 115) screenBuffer[0]=0x93;
  else if (voltage < 120) screenBuffer[0]=0x92;
  else if (voltage < 122) screenBuffer[0]=0x91;
  else screenBuffer[0]=0x90;                              // Max charge icon
#else
  screenBuffer[0]=0x97;
#endif
  screenBuffer[1]=0;
  MAX7456_WriteString(screenBuffer,getPosition(voltagePosition)-1);

if (VIDVOLTAGE){
  ItoaPadded(vidvoltage, screenBuffer, 4, 3);
  screenBuffer[4]=voltageUnitAdd;
  screenBuffer[5]=0;
  MAX7456_WriteString(screenBuffer,getPosition(vidvoltagePosition));

  screenBuffer[0]=0xbf;
  screenBuffer[1]=0;
  MAX7456_WriteString(screenBuffer,getPosition(vidvoltagePosition)-1);
}
}

void displayCurrentThrottle(void)
{                                                                           // CurentThrottlePosition is set in Config.h to line 11 above flyTimePosition

  if (MwRcData[THROTTLESTICK] > HighT) HighT = MwRcData[THROTTLESTICK] -5;
  if (MwRcData[THROTTLESTICK] < LowT) LowT = MwRcData[THROTTLESTICK];      // Calibrate high and low throttle settings  --defaults set in GlobalVariables.h 1100-1900
  screenBuffer[0]=0xC8;
  screenBuffer[1]=0;
  MAX7456_WriteString(screenBuffer,getPosition(CurrentThrottlePosition));
  if(!armed) {
    screenBuffer[0]=' ';
    screenBuffer[1]=' ';
    screenBuffer[2]='-';
    screenBuffer[3]='-';
    screenBuffer[4]=0;
    MAX7456_WriteString(screenBuffer,getPosition(CurrentThrottlePosition)+1);
  }
  else
  {
    int CurThrottle = map(MwRcData[THROTTLESTICK],LowT,HighT,0,100);
    ItoaPadded(CurThrottle,screenBuffer,3,0);
    screenBuffer[3]='%';
    screenBuffer[4]=0;
    MAX7456_WriteString(screenBuffer,getPosition(CurrentThrottlePosition)+1);
  }
}

void displayTime(void)
{
  screenBuffer[0] = flyTimeUnitAdd;
  formatTime(flySecond, screenBuffer+1);
  MAX7456_WriteString(screenBuffer,getPosition(flyTimePosition));

  screenBuffer[0] = onTimeUnitAdd;
  formatTime(onSecond, screenBuffer+1);
  MAX7456_WriteString(screenBuffer,getPosition(onTimePosition));
}

void displayAmperage(void)
{
  // Real Ampere is ampere / 10
  ItoaPadded(amperage, screenBuffer, 4, 3);     // 99.9 ampere max!
  screenBuffer[4]=amperageUnitAdd;
  screenBuffer[5]=0;
  MAX7456_WriteString(screenBuffer,getPosition(amperagePosition));
}

void displaypMeterSum(void)
{
#if defined (HARDSENSOR)
  pMeterSum = amperagesum;
#endif
  screenBuffer[0]=0xa4;
  int xx = pMeterSum / EST_PMSum;
  itoa(xx,screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(pMeterSumPosition));
}

void displayRSSI(void)
{
  screenBuffer[0]=rssiUnitAdd;
  // Calcul et affichage du Rssi
  itoa(rssi,screenBuffer+1,10);
  uint8_t xx = FindNull();
  screenBuffer[xx++]='%';
  screenBuffer[xx]=0;
  MAX7456_WriteString(screenBuffer,getPosition(rssiPosition));
}

void displayHeading(void)
{
  int16_t heading = MwHeading;
#if defined HEADING360
  if(heading < 0)
    heading += 360;
  ItoaPadded(heading,screenBuffer,3,0);
  screenBuffer[3]=MwHeadingUnitAdd;                 // Restore the NULL by the unit Symbols
  screenBuffer[4]=0;
#else
  ItoaPadded(heading,screenBuffer,4,0);
  screenBuffer[4]=MwHeadingUnitAdd;                 // Restore the NULL by the unit Symbols
  screenBuffer[5]=0;
#endif
  MAX7456_WriteString(screenBuffer,getPosition(MwHeadingPosition));
}

void displayHeadingGraph(void)
{
  int xx;
  xx = MwHeading * 4;
  xx = xx + 720 + 45;
  xx = xx / 90;

  screenBuffer[0] = headGraph[xx++];
  screenBuffer[1] = headGraph[xx++];
  screenBuffer[2] = headGraph[xx++];
  screenBuffer[3] = headGraph[xx++];
  screenBuffer[4] = headGraph[xx++];
  screenBuffer[5] = headGraph[xx++];
  screenBuffer[6] = headGraph[xx++];
  screenBuffer[7] = headGraph[xx++];
  screenBuffer[8] = headGraph[xx];
  screenBuffer[9] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(MwHeadingGraphPosition));
}

void displayIntro(void)
{

  MAX7456_WriteString_P((char*)pgm_read_word(&(introMessages[0])), RushduinoVersionPosition);

#if defined VideoSignalType_NTSC
  MAX7456_WriteString_P((char*)pgm_read_word(&(introMessages[1])), RushduinoVersionPosition+30);
#endif

#if defined VideoSignalType_PAL
  MAX7456_WriteString_P((char*)pgm_read_word(&(introMessages[2])), RushduinoVersionPosition+30);
#endif

  if(screenType==WIDE){
    MAX7456_WriteString_P((char*)pgm_read_word(&(introMessages[3])), RushduinoVersionPosition+60);
  }
  else{
    MAX7456_WriteString_P((char*)pgm_read_word(&(introMessages[4])), RushduinoVersionPosition+60);
  }

  MAX7456_WriteString_P(MultiWiiLogoL1Add, RushduinoVersionPosition+120);
  MAX7456_WriteString_P(MultiWiiLogoL2Add, RushduinoVersionPosition+120+LINE);
  MAX7456_WriteString_P(MultiWiiLogoL3Add, RushduinoVersionPosition+120+LINE+LINE);

  MAX7456_WriteString_P((char*)pgm_read_word(&(introMessages[5])), RushduinoVersionPosition+120+LINE+LINE+LINE);
  MAX7456_WriteString(itoa(MwVersion,screenBuffer,10),RushduinoVersionPosition+131+LINE+LINE+LINE);

  MAX7456_WriteString_P((char*)pgm_read_word(&(introMessages[6])), RushduinoVersionPosition+120+LINE+LINE+LINE+LINE+LINE);
  MAX7456_WriteString_P((char*)pgm_read_word(&(introMessages[7])), RushduinoVersionPosition+125+LINE+LINE+LINE+LINE+LINE+LINE);
  MAX7456_WriteString_P((char*)pgm_read_word(&(introMessages[8])), RushduinoVersionPosition+125+LINE+LINE+LINE+LINE+LINE+LINE+LINE);
}

void displayGPSPosition(void)
{
  if(!GPS_fix)
    return;

#if defined COORDINATES
  screenBuffer[0] = 0xCA;
  FormatGPSCoord(GPS_latitude,screenBuffer+1,3,'N','S');
  MAX7456_WriteString(screenBuffer,getPosition(MwGPSLatPosition));

  screenBuffer[0] = 0xCB;
  FormatGPSCoord(GPS_longitude,screenBuffer+1,4,'E','W');
  MAX7456_WriteString(screenBuffer,getPosition(MwGPSLonPosition));
#endif

  screenBuffer[0] = MwGPSAltPositionAdd[unitSystem];
  itoa(GPS_altitude,screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(MwGPSAltPosition));
}

void displayNumberOfSat(void)
{
  screenBuffer[0] = 0x1e;
  screenBuffer[1] = 0x1f;
  itoa(GPS_numSat,screenBuffer+2,10);
  MAX7456_WriteString(screenBuffer,getPosition(GPS_numSatPosition));
}

void displayGPS_speed(void)
{
  if(!GPS_fix)
    return;

  int xx;
  if(!unitSystem)
    xx = GPS_speed * 0.036;           // From MWii cm/sec to Km/h
  else
    xx = GPS_speed * 0.02236932;       // (0.036*0.62137)  From MWii cm/sec to mph

  if(xx > speedMAX)
    speedMAX = xx;

  screenBuffer[0]=speedUnitAdd[unitSystem];
  itoa(xx,screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(speedPosition));
}

void displayAltitude(void)
{
  int altitude;

  if(armed && allSec>5 && altitude > altitudeMAX)
    altitudeMAX = altitude;

  if(unitSystem)
    altitude = MwAltitude/100*3.2808;  // cm to feet
  else
    altitude = MwAltitude/100;         // cm to mt

  screenBuffer[0]=MwAltitudeAdd[unitSystem];
  itoa(altitude,screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(MwAltitudePosition));
}

void displayClimbRate(void)
{
  screenBuffer[0] = MwClimbRateAdd[unitSystem];
  int xx;
  if(unitSystem)
    xx = MwVario * 0.032808;       // cm/sec ----> ft/sec
  else
    xx = MwVario / 100;            // cm/sec ----> mt/sec
  itoa(xx,screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(MwClimbRatePosition));

  if(MwVario > 70)       screenBuffer[0]=0xB3;
  else if(MwVario > 50)  screenBuffer[0]=0xB2;
  else if(MwVario > 30)  screenBuffer[0]=0xB1;
  else if(MwVario > 20)  screenBuffer[0]=0xB0;
  else if(MwVario < -70) screenBuffer[0]=0xB4;
  else if(MwVario < -50) screenBuffer[0]=0xB5;
  else if(MwVario < -30) screenBuffer[0]=0xB6;
  else if(MwVario < -20) screenBuffer[0]=0xB7;
  else                   screenBuffer[0]=0xBC;
  screenBuffer[1]=0;

  int pos = getPosition(MwClimbRatePosition)-2;
  if(MwVario < -20)
    pos += LINE;
  MAX7456_WriteString(screenBuffer,pos);
}

void displayDistanceToHome(void)
{
  if(!GPS_fix)
    return;

  screenBuffer[0]=GPS_distanceToHomeAdd[unitSystem];
  screenBuffer[1]=0;
  MAX7456_WriteString(screenBuffer,getPosition(GPS_distanceToHomePosition));

  if(!unitSystem) GPS_distanceToHome = GPS_distanceToHome;                    // Mt
  if(unitSystem) GPS_distanceToHome = GPS_distanceToHome * 3.2808;            // mt to feet

  if(GPS_distanceToHome > distanceMAX) distanceMAX = GPS_distanceToHome;
  itoa(GPS_distanceToHome,screenBuffer,10);
  MAX7456_WriteString(screenBuffer,getPosition(GPS_distanceToHomePosition)+1);
}

void displayAngleToHome(void)
{
  if(!GPS_fix)
    return;
  if(GPS_distanceToHome <= 2 && Blink2hz)
    return;

  itoa(GPS_directionToHome,screenBuffer,10);
  uint8_t xx = FindNull();
  screenBuffer[xx++]=0xBD;
  screenBuffer[xx]=0;
  MAX7456_WriteString(screenBuffer,getPosition(GPS_angleToHomePosition));
}

void displayDirectionToHome(void)
{
  if(!GPS_fix)
    return;
  if(GPS_distanceToHome <= 2 && Blink2hz)
    return;

  int16_t d = MwHeading + 180 + 360 - GPS_directionToHome;
  d *= 4;
  d += 45;
  d = (d/90)%16;

  screenBuffer[0] = 0x60 + d;
  //screenBuffer[1] = 0x81 + d;
  screenBuffer[1]=0;                //2
  MAX7456_WriteString(screenBuffer,getPosition(GPS_directionToHomePosition));
}

void displayCursor(void)
{
int cursorpos;
static const char CURSOR[] PROGMEM = "*";

if(ROW==10){
  if(COL==3) cursorpos=SAVEP+16-1;    // page
  if(COL==1) cursorpos=SAVEP-1;       // exit
  if(COL==2) cursorpos=SAVEP+6-1;     // save/exit
  }
  if(ROW<10){  
    if(configPage==1){
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      if(COL==1) cursorpos=(ROW+2)*30+10;
      if(COL==2) cursorpos=(ROW+2)*30+10+6;  
      if(COL==3) cursorpos=(ROW+2)*30+10+6+6;
      }  
    if(configPage==2){
      COL=3;
      if (ROW==7) ROW=5;
      if (ROW==6) ROW=10;
      if (ROW==9) ROW=5;
      cursorpos=(ROW+2)*30+10+6+6;
      }      
    if(configPage==3){
      COL=3;
      if (ROW==1) ROW=2;
      if (ROW==9) ROW=6;
      if (ROW==7) ROW=10;
      cursorpos=(ROW+2)*30+10+6+6;
      }  
    if(configPage==4){
      COL=3;
      if (ROW==2) ROW=3;
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      if ((ROW==6)||(ROW==7)) cursorpos=(ROW+2)*30+10+6+6-2;  // Narrow/Imperial strings longer
      else cursorpos=(ROW+2)*30+10+6+6;
      }  
    if(configPage==5){
      COL=3;
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;   
      cursorpos=(ROW+2)*30+10+6+6;
      }
    if(configPage==6){
      ROW=10;
      }
  } 
  if(Blink10hz) MAX7456_WriteString_P(CURSOR,cursorpos);
}


void displayPIDConfigScreen(void)
{

  MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[0])), SAVEP);		//EXIT
  MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[1])), SAVEP+6);	//SaveExit
  MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[2])), SAVEP+16);	//<Page>

  if(configPage==1)
  {
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[3])), 38);
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[4])), ROLLT);
    MAX7456_WriteString(itoa(P8[0],screenBuffer,10),ROLLP);
    MAX7456_WriteString(itoa(I8[0],screenBuffer,10),ROLLI);
    MAX7456_WriteString(itoa(D8[0],screenBuffer,10),ROLLD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[5])), PITCHT);
    MAX7456_WriteString(itoa(P8[1],screenBuffer,10), PITCHP);
    MAX7456_WriteString(itoa(I8[1],screenBuffer,10), PITCHI);
    MAX7456_WriteString(itoa(D8[1],screenBuffer,10), PITCHD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[6])), YAWT);
    MAX7456_WriteString(itoa(P8[2],screenBuffer,10),YAWP);
    MAX7456_WriteString(itoa(I8[2],screenBuffer,10),YAWI);
    MAX7456_WriteString(itoa(D8[2],screenBuffer,10),YAWD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[7])), ALTT);
    MAX7456_WriteString(itoa(P8[3],screenBuffer,10),ALTP);
    MAX7456_WriteString(itoa(I8[3],screenBuffer,10),ALTI);
    MAX7456_WriteString(itoa(D8[3],screenBuffer,10),ALTD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[8])), VELT);
    MAX7456_WriteString(itoa(P8[4],screenBuffer,10),VELP);
    MAX7456_WriteString(itoa(I8[4],screenBuffer,10),VELI);
    MAX7456_WriteString(itoa(D8[4],screenBuffer,10),VELD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[9])), LEVT);
    MAX7456_WriteString(itoa(P8[7],screenBuffer,10),LEVP);
    MAX7456_WriteString(itoa(I8[7],screenBuffer,10),LEVI);
    MAX7456_WriteString(itoa(D8[7],screenBuffer,10),LEVD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[10])), MAGT);
    MAX7456_WriteString(itoa(P8[8],screenBuffer,10),MAGP);

    MAX7456_WriteString("P",71);
    MAX7456_WriteString("I",77);
    MAX7456_WriteString("D",83);
  }

  if(configPage==2)
  {
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[11])), 38);
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[12])), ROLLT);
    MAX7456_WriteString(itoa(rcRate8,screenBuffer,10),ROLLD);
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[13])), PITCHT);
    MAX7456_WriteString(itoa(rcExpo8,screenBuffer,10),PITCHD);
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[14])), YAWT);
    MAX7456_WriteString(itoa(rollPitchRate,screenBuffer,10),YAWD);
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[15])), ALTT);
    MAX7456_WriteString(itoa(yawRate,screenBuffer,10),ALTD);
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[16])), VELT);
    MAX7456_WriteString(itoa(dynThrPID,screenBuffer,10),VELD);
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[17])), LEVT);
    MAX7456_WriteString(itoa(cycleTime,screenBuffer,10),LEVD);
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[18])), MAGT);
    MAX7456_WriteString(itoa(I2CError,screenBuffer,10),MAGD);
  }

  if(configPage==3)
  {
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[19])), 35);
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[23])), PITCHT);
    if(enableVoltage){
      MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[21])), PITCHD);
    }
    else {
      MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[22])), PITCHD);
    }
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[24])), YAWT);
    MAX7456_WriteString(itoa(lowVoltage,screenBuffer,10),YAWD);
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[25])), ALTT);

    if(enableTemperature){
      MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[21])), ALTD);
    }
    else {
      MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[22])), ALTD);
    }
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[26])), VELT);
    MAX7456_WriteString(itoa(highTemperature,screenBuffer,10),VELD);
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[27])), LEVT);

    if(displayGPS){
      MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[21])), LEVD);
     }
     else {
      MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[22])), LEVD);
    }
  }

  if(configPage==4)
  {
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[31])), 39);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[32])), ROLLT);
    MAX7456_WriteString(itoa(rssiADC,screenBuffer,10),ROLLD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[33])), PITCHT);
    MAX7456_WriteString(itoa(rssi,screenBuffer,10),PITCHD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[34])), YAWT);
    if(rssiTimer>0) MAX7456_WriteString(itoa(rssiTimer,screenBuffer,10),YAWD-5);
    MAX7456_WriteString(itoa(rssiMin,screenBuffer,10),YAWD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[35])), ALTT);
    MAX7456_WriteString(itoa(rssiMax,screenBuffer,10),ALTD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[36])), VELT);
    if(enableRSSI){
      MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[21])), VELD);
    }
    else{
      MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[22])), VELD);
    }

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[37])), LEVT);
    if(unitSystem==METRIC){
      MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[38])), LEVD-2);
    }
    else {
      MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[39])), LEVD-2);
    }

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[40])), MAGT);
    if(screenType==NARROW){
      MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[41])), MAGD-2);
    }
    else {
      MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[42])), MAGD-1);
    }
  }

  if(configPage==5)
  {
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[43])), 37);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[44])), ROLLT);
    if(accCalibrationTimer>0)
      MAX7456_WriteString(itoa(accCalibrationTimer,screenBuffer,10),ROLLD);
    else
      MAX7456_WriteString("-",ROLLD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[45])), PITCHT);
    MAX7456_WriteString(itoa(MwAccSmooth[0],screenBuffer,10),PITCHD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[46])), YAWT);
    MAX7456_WriteString(itoa(MwAccSmooth[1],screenBuffer,10),YAWD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[47])), ALTT);
    MAX7456_WriteString(itoa(MwAccSmooth[2],screenBuffer,10),ALTD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[48])), VELT);
    if(magCalibrationTimer>0)
      MAX7456_WriteString(itoa(magCalibrationTimer,screenBuffer,10),VELD);
    else
      MAX7456_WriteString("-",VELD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[49])), LEVT);
    MAX7456_WriteString(itoa(MwHeading,screenBuffer,10),LEVD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[50])), MAGT);
    if(eepromWriteTimer>0)
      MAX7456_WriteString(itoa(eepromWriteTimer,screenBuffer,10),MAGD);
    else
      MAX7456_WriteString("-",MAGD);
  }

  if(configPage==6)
  {
    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[51])), 38);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[52])), ROLLT);
    MAX7456_WriteString(itoa(trip,screenBuffer,10),ROLLD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[53])), PITCHT);
    MAX7456_WriteString(itoa(distanceMAX,screenBuffer,10),PITCHD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[54])), YAWT);
    MAX7456_WriteString(itoa(altitudeMAX,screenBuffer,10),YAWD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[55])), ALTT);
    MAX7456_WriteString(itoa(speedMAX,screenBuffer,10),ALTD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[56])), VELT);

    formatTime(flyingSecond, screenBuffer);
    MAX7456_WriteString(screenBuffer,VELD-4);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[57])), LEVT);
    int xx= pMeterSum / EST_PMSum;
    MAX7456_WriteString(itoa(xx,screenBuffer,10),LEVD);

    MAX7456_WriteString_P((char*)pgm_read_word(&(configMsgs[58])), MAGT);
    MAX7456_WriteString(itoa(temperMAX,screenBuffer,10),MAGD);
  }
    displayCursor();  // NEB mod cursor display
}
