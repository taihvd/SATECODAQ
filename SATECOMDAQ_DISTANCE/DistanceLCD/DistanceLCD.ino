#include <Wire.h>
#include <Kalman.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#define Trig1 11
#define Echo1 3

#define relayPin 2

#define BMEMV 0

#define BMEMH 4

#define RESTRICT_PITCH

#define btnRIGHT  0

#define btnUP     1

#define btnDOWN   2

#define btnLEFT   3

#define btnSELECT 4

#define btnNONE   5

#define rxPin 13

#define txPin 12

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);

String a[3] = { "Mise en E0","Mise en A", "Mise en B"};

int midArray[9];
int lcdMode = 0;

const long interval = 500;
const long intervalRelay = 10000;
const long delayLaser = 500;
unsigned long previousMillis = 0;
unsigned long previousMillisForLaser = 0;
double avDistance, avDistanceSecond;

int countAverage;

String transData1,transData2, transDataSecond,lastLaser;

int pos = 12;
int count = 0;
long coteA = 0;
long coteB = 0;
long E0 = 0;
long nDelay = 10;
int j;
int keyIn  = 0;
int line = 0;
int stage = 0;
int lcdKey = 0;
int selectPos = 0;
int flagUpdate = 0,flaglaser = 0;
int trigger, sugar;
int costsum;
int slpt;
float notePads[110];
float numAverage = 100;
float total;
float baudrate=0.2;
float suppoxr = 0;
float mistotal,request;
void setup() 
{

  Serial.begin(9600);
  mySerial.begin(9600);
  lcd.begin(16, 2);
  flagUpdate = 3;
  coteA = EEPROMReadlong(1);
  coteB = EEPROMReadlong(5);
  E0    = EEPROMReadlong(9);
  lastLaser="0";
  mistotal = 0;
  request = 0;
  pinMode(Echo1, INPUT);
  pinMode(Trig1, OUTPUT);
  pinMode(relayPin , OUTPUT);
  delay(200);
  digitalWrite(relayPin,HIGH);
}

void lcdBink(String showDown) {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    if (count == 1) {
      lcd.setCursor(0, 1);
      lcd.print("  Choisissiz!");
      count = 0;
    }
    else {
      lcd.setCursor(0, 1);
      lcd.print("               ");
      count = 1;
    }
  }
}
void Startup(int theleg)
{
  unsigned long currentMillisForLaser = millis();
  //Serial.println(currentMillisForLaser);
  //Serial.println(previousMillisForLaser);
  //Serial.println(flaglaser);
  if (flaglaser == 1) {
     if (currentMillisForLaser - previousMillisForLaser >= intervalRelay)
    {
      //Serial.println("NO");
      previousMillisForLaser = currentMillisForLaser;
      flaglaser=0;
      digitalWrite(relayPin,LOW);
      delay(200);
      digitalWrite(relayPin,HIGH);
    } 
  }
  else 
  {
      if (currentMillisForLaser - previousMillisForLaser >= delayLaser)
    {
      //Serial.println("YES");
      previousMillisForLaser = currentMillisForLaser;
      flaglaser=1;
      digitalWrite(relayPin,LOW);
      delay(200);
      digitalWrite(relayPin,HIGH);
    }  
  }
  
  //low la bat high la tat ngac nhien chua :v.
}
void LCDnoBlink()
{
  lcd.noBlink();
}
void EEPROMWritelong(int address, long value)   {
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);

}

long EEPROMReadlong(long address)
{
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
void LCDBlink()
{
  unsigned long currentMillis = millis();
  int d = 0;
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (d == 0)
    {
      d++;
      lcd.blink();
    }
    else
    {
      d = 0;
      lcd.noBlink();
    }
  }
}
int read_LCD_buttons() {

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
  stage = line;
  line = read_LCD_buttons();
  if (line == btnNONE) return stage;
}

void calculate()
{
  digitalWrite(Trig1, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig1, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig1, LOW);
  float ds = pulseIn(Echo1, HIGH);

  ds /= 58;

  countAverage++;
  notePads[countAverage]=ds;

  if (countAverage == numAverage) {
    //sort
    for (int x=0;x<countAverage-1;x++)
      for (int y=x+1;y<countAverage;y++)
        if (notePads[x] < notePads[y])
        {
          notePads[x]+=notePads[y];
          notePads[y]=notePads[x]-notePads[y];
          notePads[x]=notePads[x]-notePads[y];
        }
       // for (int x=0;x<numAverage;x++)
     //Serial.println(notePads[x]);
    j=5;
    countAverage = 0;
    total=notePads[j];
    slpt=1;
    baudrate=0.2;
    
    for (int x=j+1;x<100;x++)
      if (abs(notePads[x]-notePads[j])<=baudrate)
      {
        total+=notePads[x]; 
        slpt++;
      }
    else break;
    for (int x=j-1;x>=0;x--)
      if (abs(notePads[x]-notePads[j])<=baudrate)
      {
        total+=notePads[x]; 
        slpt++;
      }
    else break;

    total/=slpt;
    total+=4.4;
    
//    Serial.println("request ");
//    Serial.println(request);
//    Serial.println("mistotal ");
//    Serial.println(mistotal);
//    Serial.println("slpt ");
    //Serial.println(slpt);
    if (abs(mistotal-total) > (request))
    {
      request=total/100;
      mistotal=total;
      
      suppoxr=E0;
      total+=(suppoxr/10);
      suppoxr=coteA;
      total-=(suppoxr/5);
      
      transData1 = "E voile: "+ String(total,1) + "cm";
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(transData1);
      
      transData1 = "dh " + String(total,1) ;
    
       
      
      //Serial.println(total);
    }    
    Serial.println(transData1);
    
    if (mySerial.available ()>0) {
      lastLaser=mySerial.readString();
      }
      transData2 = "dv " + lastLaser ;
        Serial.println(transData2);
  }
 
 delay(10);
}
void loop() {
  Startup(relayPin);
  if (lcdMode == 0)
  {
    if (flagUpdate == 0)
    {
      flagUpdate = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  Calculer...!");
      delay(10);
    }
    if (flagUpdate == 3)
    {
      flagUpdate = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   SATECODAQ");
      lcd.setCursor(0, 1);
      lcd.print("  Bonjour....!");
      delay(800);
    }
    calculate();
    lcdKey = button_press();
    if (lcdKey == btnSELECT) {
      lcdMode = 1;
      flagUpdate = 0;
    }
    selectPos = 0;
  }
  if (lcdMode == 1)
  {
    if (flagUpdate == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(selectPos + 1);
      lcd.print(".");
      lcd.print(a[selectPos]);

      flagUpdate = 1;
    }
    lcdKey = button_press();
    if (lcdKey == btnLEFT || lcdKey == btnUP) {
      selectPos--;
      if (selectPos < 0) selectPos = 2;
      flagUpdate = 0;
    }
    if (lcdKey == btnRIGHT || lcdKey == btnDOWN) {
      selectPos++;
      if (selectPos > 2) selectPos = 0;
      flagUpdate = 0;
    }
    if (lcdKey == btnSELECT) {
      //lcdMode = 0;
      lcdMode += selectPos + 1;
      flagUpdate = 0;
    }
    lcdBink("");
  }
  if (lcdMode == 3)
  {
    if (flagUpdate == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Cote A:");
      lcd.setCursor(4, 1);
      flagUpdate = 1;
      long x = 100000000;
      for (int i = 0; i < 9; i++)
      {
        midArray[i] = coteA / (x) % 10;
        lcd.print(midArray[i]);
        x /= 10;
      }
      pos = 12;
      lcd.print(" mm");
    }
    lcd.setCursor(pos, 1);
    
    LCDBlink();
    lcdKey = button_press();
    switch (lcdKey) {
      case btnLEFT: if (pos > 4) pos--; break;
      case btnRIGHT: if (pos < 12)pos++; break;
      case btnUP: if (midArray[pos - 4] < 9) midArray[pos - 4]++; else midArray[pos - 4] = 0; lcd.print(midArray[pos - 4]); break;
      case btnDOWN: if (midArray[pos - 4] > 0) midArray[pos - 4]--; else midArray[pos - 4] = 9; lcd.print(midArray[pos - 4]); break;
      case btnSELECT:
        coteA = 0;
        for (int i = 0; i < 9; i++) {
          coteA *= 10;
          coteA += midArray[i];
        }
        EEPROMWritelong(1, coteA);
        //Serial.println(coteA);
        countAverage = 0;
        flagUpdate = 0;
        lcdMode = 0;
        request = 0;
        mistotal = 0;
        LCDnoBlink();
        //return;
        break;
    }
  }

  if (lcdMode == 4)
  {
    if (flagUpdate == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Cote B:");
      lcd.setCursor(4, 1);
      flagUpdate = 1;
      long x = 100000000;
      for (int i = 0; i < 9; i++)
      {
        midArray[i] = coteB / (x) % 10;
        lcd.print(midArray[i]);
        x /= 10;
      }
      pos = 12;
      lcd.print(" mm" );
      }
      lcd.setCursor(pos, 1);
      
      LCDBlink();
      lcdKey = button_press();
      switch (lcdKey) {
        case btnLEFT: if (pos > 4) pos--; break;
        case btnRIGHT: if (pos < 12)pos++; break;
        case btnUP: if (midArray[pos - 4] < 9) midArray[pos - 4]++; else midArray[pos - 4] = 0; lcd.print(midArray[pos - 4]); break;
        case btnDOWN: if (midArray[pos - 4] > 0) midArray[pos - 4]--; else midArray[pos - 4] = 9; lcd.print(midArray[pos - 4]); break;
        case btnSELECT:
          coteB = 0;
          for (int i = 0; i < 9; i++) {
            coteB *= 10;
            coteB += midArray[i];
          }
          EEPROMWritelong(5, coteB);
          Serial.println(coteB);
          countAverage = 0;
          flagUpdate = 0;
          lcdMode = 0;
          request = 0;
          mistotal = 0;
          LCDnoBlink();
          //return;
          break;
      }
  }
    if (lcdMode == 2)
        {
          if (flagUpdate == 0) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Cote E0:");
            lcd.setCursor(4, 1);
            flagUpdate = 1;
            long x = 100000000;
            for (int i = 0; i < 9; i++)
            {
              midArray[i] = E0 / (x) % 10;
              lcd.print(midArray[i]);
              x /= 10;
            }
            pos = 12;
            lcd.print(" mm");
          }
          lcd.setCursor(pos, 1);
          LCDBlink();
          lcdKey = button_press();
          switch (lcdKey) {
            case btnLEFT: if (pos > 7) pos--; break;
            case btnRIGHT: if (pos < 15)pos++; break;
            case btnUP: if (midArray[pos - 4] < 9) midArray[pos - 4]++; else midArray[pos - 4] = 0; lcd.print(midArray[pos - 4]); break;
            case btnDOWN: if (midArray[pos - 4] > 0) midArray[pos - 4]--; else midArray[pos - 4] = 9; lcd.print(midArray[pos - 4]); break;
            case btnSELECT:
              E0 = 0;
              for (int i = 0; i < 9; i++) {
                E0 *= 10;
                E0 += midArray[i];
              }
              EEPROMWritelong(9, E0);
              Serial.println(E0);
              countAverage = 0;
              flagUpdate = 0;
              lcdMode = 0;
              request = 0;
              mistotal = 0;
              LCDnoBlink();
              //return;
              break;
          }
        }
}

