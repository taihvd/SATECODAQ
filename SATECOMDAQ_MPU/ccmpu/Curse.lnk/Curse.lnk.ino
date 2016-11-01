#include <Wire.h>
#include <Kalman.h> 
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define BMEMV 1

#define BMEMH 5

#define RESTRICT_PITCH 

#define btnRIGHT  0

#define btnUP     1

#define btnDOWN   2

#define btnLEFT   3

#define btnSELECT 4

#define btnNONE   5

#define rxPin 12

#define txPin 11

Kalman kalmanX; 
Kalman kalmanY;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);
String a[5]={"SET BALANCE"};
int midArray[9];
int lcdMode=0;

const long interval = 500; 

unsigned long previousMillis = 0; 


double avRoll,avPitch,avDistance,lastV,lastH;
double balanceV;
double balanceH;
double newV = 0;
double newH = 0;

long long countAverage;
//cote E0;//don vi 000000.0cm
int pos=15;
int count = 0;
int angle=223;
long avc = 100;
long nDelay = 1;
int keyIn  = 0;
int line = 0;
int stage = 0;
int lcdKey = 0;
int selectPos = 0;
int flagchange = 0,flagUpdate = 0,flagBalance = 0;
int trigger,sugar;
int t;

String transData;

double accX, accY, accZ;
double gyroX, gyroY, gyroZ;
int16_t tempRaw;

double gyroXangle, gyroYangle; 
double compAngleX, compAngleY; 
double kalAngleX, kalAngleY; 

uint32_t timer;
uint8_t i2cData[14]; 

void setup() {
  Serial.begin(9600);
   mySerial.begin(9600);
  lcd.begin(16,2);
  flagUpdate = 3;
  avRoll = 0;
  avPitch = 0;
  countAverage = 0;
//  Serial.println(readValue(BMEMV));
//  Serial.println(readValue(BMEMH));
    balanceV=readValue(BMEMV);
   balanceH=readValue(BMEMH);
   
  //Wifi connect.
  //WiFiMulti.addAP("SATECODAQ", "!dfm1610");
    //setup wire
  Wire.begin();
#if ARDUINO >= 157
  Wire.setClock(400000UL); // Set I2C frequency to 400kHz
#else
  TWBR = ((F_CPU / 400000UL) - 16) / 2; // Set I2C frequency to 400kHz
#endif

  i2cData[0] = 7; // Set the sample rate to 1000Hz - 8kHz/(7+1) = 1000Hz
  i2cData[1] = 0x00; // Disable FSYNC and set 260 Hz Acc filtering, 256 Hz Gyro filtering, 8 KHz sampling
  i2cData[2] = 0x00; // Set Gyro Full Scale Range to ±250deg/s
  i2cData[3] = 0x00; // Set Accelerometer Full Scale Range to ±2g
  while (i2cWrite(0x19, i2cData, 4, false)); // Write to all four registers at once
  while (i2cWrite(0x6B, 0x01, true)); // PLL with X axis gyroscope reference and disable sleep mode

  while (i2cRead(0x75, i2cData, 1));
  if (i2cData[0] != 0x68) { // Read "WHO_AM_I" register
    Serial.print(F("Error reading sensor"));
    while (1);
  }

  delay(100); // Wait for sensor to stabilize

  /* Set kalman and gyro starting angle */
  while (i2cRead(0x3B, i2cData, 6));
  accX = (i2cData[0] << 8) | i2cData[1];
  accY = (i2cData[2] << 8) | i2cData[3];   
  accZ = (i2cData[4] << 8) | i2cData[5];

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // It is then converted from radians to degrees
#ifdef RESTRICT_PITCH // Eq. 25 and 26
  double roll  = atan2(accY, accZ) * RAD_TO_DEG;
  double pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
#else // Eq. 28 and 29
  double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  double pitch = atan2(-accX, accZ) * RAD_TO_DEG;
#endif

  kalmanX.setAngle(roll); // Set starting angle
  kalmanY.setAngle(pitch);
  gyroXangle = roll;
  gyroYangle = pitch;
  compAngleX = roll;
  compAngleY = pitch;

  timer = micros();
  sugar=1;
}
void caculateRP() {
  //lcd.clear();
  //balanceV=0;
  while (i2cRead(0x3B, i2cData, 14));
  accX = ((i2cData[0] << 8) | i2cData[1]);
  accY = ((i2cData[2] << 8) | i2cData[3]);
  accZ = ((i2cData[4] << 8) | i2cData[5]);
  tempRaw = (i2cData[6] << 8) | i2cData[7];
  gyroX = (i2cData[8] << 8) | i2cData[9];
  gyroY = (i2cData[10] << 8) | i2cData[11];
  gyroZ = (i2cData[12] << 8) | i2cData[13];

  double dt = (double)(micros() - timer) / 1000000; // Calculate delta time
  timer = micros();

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // It is then converted from radians to degrees
#ifdef RESTRICT_PITCH // Eq. 25 and 26
  double roll  = atan2(accY, accZ) * RAD_TO_DEG;
  double pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
  double yaw   = atan2(accY, accZ) * RAD_TO_DEG;
#else // Eq. 28 and 29
  double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  double pitch = atan2(-accX, accZ) * RAD_TO_DEG;
  double yaw   = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
#endif

  double gyroXrate = gyroX / 131.0; // Convert to deg/s
  double gyroYrate = gyroY / 131.0; // Convert to deg/s

#ifdef RESTRICT_PITCH
  // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
  if ((roll < -90 && kalAngleX > 90) || (roll > 90 && kalAngleX < -90)) {
    kalmanX.setAngle(roll);
    compAngleX = roll;
    kalAngleX = roll;
    gyroXangle = roll;
  } else
    kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculate the angle using a Kalman filter

  if (abs(kalAngleX) > 90)
    gyroYrate = -gyroYrate; // Invert rate, so it fits the restriced accelerometer reading
  kalAngleY = kalmanY.getAngle(pitch, gyroYrate, dt);
#else
  // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
  if ((pitch < -90 && kalAngleY > 90) || (pitch > 90 && kalAngleY < -90)) {
    kalmanY.setAngle(pitch);
    compAngleY = pitch;
    kalAngleY = pitch;
    gyroYangle = pitch;
  } else
    kalAngleY = kalmanY.getAngle(pitch, gyroYrate, dt); // Calculate the angle using a Kalman filter

  if (abs(kalAngleY) > 90)
    gyroXrate = -gyroXrate; // Invert rate, so it fits the restriced accelerometer reading
  kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculate the angle using a Kalman filter
#endif

  gyroXangle += gyroXrate * dt; // Calculate gyro angle without any filter
  gyroYangle += gyroYrate * dt;
  //gyroXangle += kalmanX.getRate() * dt; // Calculate gyro angle using the unbiased rate
  //gyroYangle += kalmanY.getRate() * dt;

  compAngleX = 0.93 * (compAngleX + gyroXrate * dt) + 0.07 * roll; // Calculate the angle using a Complimentary filter
  compAngleY = 0.93 * (compAngleY + gyroYrate * dt) + 0.07 * pitch;

  // Reset the gyro angle when it has drifted too much
  if (gyroXangle < -180 || gyroXangle > 180)
    gyroXangle = kalAngleX;
  if (gyroYangle < -180 || gyroYangle > 180)
    gyroYangle = kalAngleY;
  
  if (roll-lastH > 4 || roll-lastH < -4 || pitch-lastV > 4 || pitch-lastV < -4 )
  {
    flagchange = 1;
  }
  sugar--;
  if (roll<0) { 
    roll=roll*97.78/100.0;
    pitch-=roll*0.65/100.0;            
  } else {
    roll=roll*96.7/100.0;;
    pitch+=roll*1.7/100.0;
  }
  if (pitch < 0) {
    pitch=pitch*108.8/100;;
  } else {
    pitch=pitch*100.5/100;
  }
  avRoll += roll;
  avPitch += pitch;
  countAverage++;
//  Serial.print(pitch );
//  Serial.print(" ");
//  Serial.println(roll);
  if (t == 100) nDelay=100;
  //lcd.clear();
  //Serial.println(avc);
  //Serial.println(countAverage);
  if (sugar == 0) {
    trigger=avc;
    sugar=trigger;
    lcd.clear();
    lcd.setCursor(0,0);
    newH = avRoll/countAverage - balanceH;  
    transData="av "+String(newH,1);
      mySerial.println(transData);
      Serial.println(transData);
    
    String strNewH = String(newH,1);
    if (newH>=0.0)
      strNewH = String(" ") + strNewH;
    transData="Aplomb: "+strNewH;
    //"E voile"
    //"H voile"
    lcd.print(transData);  
    lcd.write(angle);
    if (t<100)
    t=countAverage/30;
    lcd.setCursor(0,1);
    newV = avPitch/countAverage - balanceV;
    String strNewV = String(newV,1);
    
       
    transData="ah " + strNewV ;  
      mySerial.println(transData);
      Serial.println(transData);
    
     if (newV>=0.0)
      strNewV = String(" ") + strNewV;  
    transData="Niveau: "+strNewV  ;
    
    lcd.print(transData);
    lcd.write(angle);
    flagBalance=1;
    if (flagchange > 0 )
    {
    t=0;
    lastH=roll;
    lastV=pitch;
    countAverage=0;
    avRoll=0;
    avPitch=0;
    trigger=avc;
    sugar=avc;
    nDelay=1;
    flagchange = 0;
    }
  }  
   Serial.println(t);
  if (t >= 100)
  {
    if (flagchange > 0)
    {
    t=0;
    lastH=roll;
    lastV=pitch;
    countAverage=0;
    avRoll=0;
    avPitch=0;
    trigger=avc;
    sugar=avc;
    nDelay=1;
    flagchange=0;
    }
    return;
  }
  delay(nDelay);
}
//day' gia tri nhan tt. set Eo H0 L_e0-2a
//H =
void lcdBink(String showDown) {
unsigned long currentMillis = millis();
      
      if (currentMillis-previousMillis >= interval)
      {
        previousMillis = currentMillis;
        if (count == 1) {
          lcd.setCursor(0,1);
          lcd.print("     Select!"); 
          count=0;
        }
        else {
          lcd.setCursor(0,1);
          lcd.print("               "); 
          count=1;
          }
      }  
}
void LCDnoBlink()
{
  lcd.noBlink();
}
void LCDBlink()
{
  unsigned long currentMillis = millis();
  int d=0;
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (d==0)
    {
     d++;
     lcd.blink();
    }
    else 
    {
      d=0;
      lcd.noBlink();
    }
  }
}
int read_LCD_buttons(){              

    keyIn = analogRead(0);     
    delay(5); 
    int k = (analogRead(0) - keyIn); 
    if (5 < abs(k)) return btnNONE; 

    if (keyIn > 1000) return btnNONE; 

    if (keyIn < 50)   return btnRIGHT;  

    if (keyIn < 250)  return btnUP; 

    if (keyIn < 450)  return btnDOWN; 

    if (keyIn < 650)  return btnLEFT; 

    if (keyIn < 850)  return btnSELECT;  

    return btnNONE;

}
int button_press()
{   
    stage=line;  
    line=read_LCD_buttons();
    if (line==btnNONE) return stage;
}
void loop() {
  
  if (lcdMode == 0)
  {
    if (flagUpdate == 0)
    {
      flagUpdate = 1;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("  Calculer...!");
      delay(10);
      t=0;
      lastH=0;
      lastV=0;
      countAverage=0;
      avRoll=0;
      avPitch=0;
      trigger=avc;
      sugar=avc;
      nDelay=1;
    }
    if (flagUpdate == 3)
    {
      flagUpdate = 1;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("   SATECODAQ");
      lcd.setCursor(0,1);
      lcd.print("  Bonjour....!");
      delay(1000);
    }
    caculateRP();
    lcdKey = button_press();
    if (lcdKey == btnSELECT) {
      lcdMode = 1;
      flagUpdate=0;
    }
    selectPos = 0;
  }
  if (lcdMode == 1)
  {
    if (flagUpdate == 0) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("FIXE BALANCE");
      lcd.setCursor(0,1);
      lcd.print("OK");
      lcd.setCursor(9,1);
      lcd.print("ANNULER");
      flagUpdate=1;
    }
    lcdKey = button_press();
    switch(lcdKey) {
    case btnLEFT: 
        if (flagBalance == 1) {
        if (newV < 300.0 || newV >-300.0)
        {
          balanceV += newV;
          writeValue(1,balanceV);
        }
         if (newH < 300.0 || newH >-300.0)
        {
          balanceH += newH;
          writeValue(5,balanceH);
        }
        sugar = avc;
        trigger = avc;
        }
        flagUpdate=0;
        lcdMode=0; 
        LCDnoBlink();
        flagBalance = 0;
        
        break;
    case btnRIGHT:  
        flagUpdate=0;
        lcdMode=0; 
        LCDnoBlink();
        break;
    case btnSELECT: 
        if (flagBalance == 1) {
        if (newV < 300.0 || newV >-300.0)
        {
          balanceV += newV;
          writeValue(1,balanceV);
        }
         if (newH < 300.0 || newH >-300.0)
        {
          balanceH += newH;
          writeValue(5,balanceH);
        }
        sugar = avc;
        trigger = avc;
        }
        flagUpdate=0;
        lcdMode=0; 
        LCDnoBlink();
        flagBalance = 0;
 
        break;
    }  
  }
}

