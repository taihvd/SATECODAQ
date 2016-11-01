/*
 *  This sketch sends a message to a TCP server
 *
 */
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

extern "C" {
#include "user_interface.h"
}

#define MAX_SRV_CLIENTS 5


const char* ssid     = "SATECODAQ";
const char* password = "dfm1610!";

SoftwareSerial mySerial(D2,D3);

const uint16_t port = 16103;
//const uint16_t port = 6096;
//const char * host = "192.168.20.179"; // ip or dns
const char * host = "192.168.1.1";
const long long interval=8000;
unsigned long previousMillis=0;

WiFiClient client;
WiFiServer server(16103);
WiFiClient serverClients[MAX_SRV_CLIENTS];

byte mac[6];
String tk,tks;
String lastAv;
String lastAh;
int flag2 = 1;
unsigned long lastSent = 0;
int dem;
int mssid;
int uuck;

IPAddress ip(192, 168, 1, 94);    
IPAddress gateway(192,168,1,1); // this is the gateway ip of the router as i see it when login to the router panel
IPAddress subnet(255,255,255,0); 


void initVariant() { uint8_t mac[6] = {0x72, 0x18, 0xc8, 0x1b, 0x2, 0x93}; wifi_set_macaddr(STATION_IF, &mac[0]); }

void handleServer() {
  if (server.hasClient()) {    // connecting the client to the server 
    int connected = 0;
    for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()) {
        if (serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        Serial.print("New client: "); Serial.println(i);
        continue;
      }
      connected ++;
    }
    Serial.print("Connected clients = ");
    Serial.println(connected);
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  for (int i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (serverClients[i] && serverClients[i].connected())
    {
      if (serverClients[i].available())
      {
        Serial.print("Recieved Message from: ");
        Serial.print(serverClients[i].remoteIP());
        Serial.print("\t");
        String message = "";
        while (serverClients[i].available())
        {
          char tempchar = serverClients[i].read();
          message += tempchar;
        }
        
        Serial.println(message);
        if (message.indexOf("terminate") >= 0) {        
          ESP.restart();
        }
        
      }
    }
  }
}
void setup() {
    initVariant();
    Serial.begin(9600);
    mySerial.begin(9600);
    delay(10);
    //WiFiMulti.addAP("dfm", "minhvuong2010");
    WiFi.config(ip, gateway, subnet);
    WiFi.begin(ssid, password);

    Serial.println();
    Serial.println();
    Serial.print("Wait for WiFi... ");

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    mssid=0;
    server.begin();
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Wait for data...!");

    delay(500);
}


void loop() {
   handleServer();
    // Use WiFiClient class to create TCP connections
    if (mySerial.available() > 0) {
       delay(10);
       mssid++;
       if (mssid == 256) mssid = 1;
       tk=ESP.getChipId();
       tk+=" 1 ";
       tk+=String(mssid);
       tk+=" ";
       tks=tk;
       tk+=mySerial.readStringUntil('\n');
       tks+=mySerial.readStringUntil('\n');
       tk+=tks;
       unsigned long currentMillis = millis();
         if (currentMillis - previousMillis >= interval)
         {
            previousMillis = currentMillis;
            Serial.println(tk);
            Serial.print("connecting to ");
            Serial.println(host);
            if (!client.connect(host, port)) {
                Serial.println("connection failed");
                Serial.println("wait 5 sec...");
                delay(200);
                return;
            } else {    
              client.println(tk);
              //client.println(tks);
              Serial.println("closing connection");
              client.stop();
              delay(10);
              Serial.println("Wait for data....!");
            }
         }
      } else { 
        delay(100);
      }
}

