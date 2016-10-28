/*
 *  This sketch sends a message to a TCP server
 *
 */
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti WiFiMulti;

SoftwareSerial mySerial(D2,D3);

const uint16_t port = 16103;
//const uint16_t port = 6096;
//const char * host = "192.168.20.179"; // ip or dns
const char * host = "192.168.1.1";
IPAddress ip(192,168,1,20);

WiFiClient client;

String tk;
String lastAv;
String lastAh;
int flag2 = 1;
unsigned long lastSent = 0;
int dem;
int mssid;
void setup() {
    Serial.begin(9600);
    mySerial.begin(9600);
    delay(10);

    //WiFiMulti.addAP("dfm", "minhvuong2010");
    WiFiMulti.addAP("SATECODAQ", "dfm1610!");

    Serial.println();
    Serial.println();
    Serial.print("Wait for WiFi... ");
    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    mssid=0;
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Wait for data...!");
    delay(500);
}


void loop() {
    // Use WiFiClient class to create TCP connections
    if (mySerial.available() > 0) {
       delay(10);
       mssid++;
       if (mssid == 256) mssid = 1;
       tk=ESP.getChipId();
       tk+=" 1 ";
       tk+=String(mssid);
       tk+=" ";
       tk+=mySerial.readStringUntil('\n');
      Serial.print("connecting to ");
      Serial.println(host);
      if (!client.connect(host, port)) {
          Serial.println("connection failed");
          Serial.println("wait 5 sec...");
          delay(5000);
          return;
      }
      else {    
        Serial.println(tk);
        client.println(tk);
        Serial.println("closing connection");
        client.stop();
        delay(10);
        Serial.println("Wait for data....!");
      }
    } else { 
      delay(100);
    }
}

