/**************************************************************

     antsw2.ino - Antenna Switch software
     for the five port antenna switch

     Written 29 April 2020 by Gerald Swann
     during the height of the Covid-19 Stay at Home period.

     30 April 2020
     Added OTA capability

     8 May 2020
     Added 18B20 temperature sensor

     3 August 2020
     v 1.3 Added MQTT publication to feed an OLED display on my desk

     4 August 2020
     v 1.3b Changed MQTT publication text

     7 September 2020
     v 1.4 Changed config for 6 ports
     
 **************************************************************/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <PubSubClient.h>

//********************************
#define mqtt_user "xxxx"                //enter your MQTT username
#define mqtt_password "xxxx"            //enter your password
#define mqtt_client "client-antsw2"    // must be unique for each board/client
const char* mqtt_server = "192.168.2.6";
//********************************

WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = "NETGEARxx";
const char* password = "xxxxx";

const char* myVersion = "v1.4";

const char* WiFi_hostname = "antsw2";
const char* host = "antsw2";

const char* progname = "antsw2.ino";

ESP8266WebServer server(80);

const int led = LED_BUILTIN;
int led_pin = LED_BUILTIN;
int myLed = LED_BUILTIN;

int myAntenna = 0;

int cntSeconds;
unsigned long currentMillis;
unsigned long previousMillis;
const long oneSecond = 1000;

float tempF;
float tempF2;

boolean sensorEnabled = false;

unsigned int cntMR = 0;

// Data wire is plugged into pin D7 on the ESP8266 - GPIO 13
#define ONE_WIRE_BUS 13

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature DS18B20(&oneWire);


//***************************************************
// The hardware UARTs share the same baud rate generator !!
void sendATTiny(char *SerOut) {
  Serial1.begin(300);
  Serial1.println(SerOut);
  delay(500);
  Serial.begin(115200);
}

// to support software reset
void resetATTiny() {
  Serial1.begin(300);
  for (int ii = 0 ; ii < 3; ii++) {
    Serial1.print("r");
    delay(1000);
  }
  Serial.begin(115200);
}


void handleRoot() {
  // show the webserver is being accessed
  digitalWrite(myLed, 1);
  String message = "This is the antenna switch webserver \n";
  message += "Running " + String(progname) + " " + String(myVersion) + "\n"
             + "Supporting the Six Port Switch\n"
             + "Selected antenna is " + String(myAntenna) + "\n";

  if (!sensorEnabled) {
    message += "Temperature is disabled\n" ;
  } else {
    message += "Temperature is " + String(tempF2, 1) + " degrees F\n";
  }
  message += "Running for " + String(millis() / 1000) + " seconds\n";

  server.send(200, "text/plain", message);
  digitalWrite(myLed, 0);
}

void handleSensor() {

  sensorEnabled = true;

  // show the webserver is being accessed
  digitalWrite(myLed, 1);
  String message = "This is the antenna switch webserver \n";
  message += "Sensor enabled\n";

  server.send(200, "text/plain", message);
  digitalWrite(myLed, 0);
}

void handleAnt() {
  digitalWrite(led, 1);
  String message = "antenna control\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  message += "antenna " + server.arg(0);

  server.send(200, "text/plain", message);

  int antenna = server.arg(0).toInt();

  // local to global
  myAntenna = antenna;
//  client.publish("antdisp/cmd","oledcmd,clear");
  client.publish("antdisp/cmd","oled,1,1,Antenna ");
  client.publish("antdisp/cmd","oled,2,1,Switch  ");
  switch (antenna) {
    case 0:
      sendATTiny("0");
      client.publish("antdisp/cmd","oled,5,1,Ant disc");
//      client.publish("antdisp/cmd","oled,6,1,HIGH SWR");
      client.publish("antsw2/antenna", "Ant disc" );
      break;
    case 1:
      sendATTiny("1");
      client.publish("antdisp/cmd","oled,5,1,Ant1 40m");
      client.publish("antsw2/antenna", "Ant1 40m" );
      break;
    case 2:
      sendATTiny("2");
      client.publish("antdisp/cmd","oled,5,1,Ant2 30m");
      client.publish("antsw2/antenna", "Ant2 30m" );
      break;
    case 3:
      sendATTiny("3");
      client.publish("antdisp/cmd","oled,5,1,Ant3 20m");
      client.publish("antsw2/antenna", "Ant3 20m" );
      break;
    case 4:
      sendATTiny("4");
      client.publish("antdisp/cmd","oled,5,1,Ant4 15m");
      client.publish("antsw2/antenna", "Ant4 15m" );
      break;
    case 5:
      sendATTiny("5");
      client.publish("antdisp/cmd","oled,5,1,Ant5 10m");
      client.publish("antsw2/antenna", "Ant5 10m" );
      break;
    case 6:
      sendATTiny("6");
      client.publish("antdisp/cmd","oled,5,1,Ant6 Tun");
      client.publish("antsw2/antenna", "Ant6 Tun" );
      break;
  }

  digitalWrite(led, 0);
}


void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

//********************************
void reconnect() {
  int attempts = 0;
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_client, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      cntMR += 1;
      client.publish("antsw2/status", "up" );
      client.subscribe("antsw2/#");
    }
    else {
      ArduinoOTA.handle();
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      attempts += 1;
      // Wait 5 seconds before retrying
      delay(5000);
      if (attempts > 120){
         ESP.restart();
      }
      
    }
  }
} // end reconnect

void callback(char* topic, byte* payload, unsigned int length) {

  for (int ii = 0; ii < length; ii++) {
    payload[ii] = toupper(payload[ii]);
  }
  payload[length] = '\0';
  char *chrPayload = (char *) payload;
 
  if (strcmp(topic, "antsw2/switch") == 0)
  {

    if (strcmp(chrPayload, "PING") == 0) {
      client.publish("antsw2/hostname", WiFi_hostname);

      char mybytes[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}   ;
      sprintf(mybytes, "%u" , cntMR);
      client.publish("antsw2/mqttrec", mybytes);

      unsigned int myFree = ESP.getFreeHeap();
      char mybytes2[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}   ;
      sprintf(mybytes2, "%u" , myFree);
      client.publish("antsw2/memfree", mybytes2);

      int myRSSI = WiFi.RSSI();
      char mybytes3[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}   ;
      sprintf(mybytes3, "%d" , myRSSI);
      client.publish("antsw2/RSSI", mybytes3);

      IPAddress myip = WiFi.localIP();
      char ipchar[24];
      sprintf(ipchar, "%d.%d.%d.%d", myip[0], myip[1], myip[2], myip[3]) ;
      client.publish("antsw2", ipchar );

    }
  }
}  // end callback


void setup(void) {

  // provide ground to temperature sensor
  pinMode(D6, OUTPUT);
  digitalWrite(D6, 0);

  client.setServer(mqtt_server, 1883);

  pinMode(led, OUTPUT);
  digitalWrite(led, 1);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(WiFi_hostname);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(WiFi_hostname)) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/sensor", handleSensor);

  server.on("/ant", handleAnt);

  server.on("/inline", []() {
    String message = "this works as well \n";
    message += "line 2 is here";
    server.send(200, "text/plain", message);
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  setupOTA();
  Serial.println("OTA started");

  // IC Default 9 bit. If you have troubles consider upping it 12.
  // Ups the delay giving the IC more time to process the temperature measurement

  DS18B20.begin();
  getTemperature();
  Serial.println("DS18B20 started");

  for (int i = 0; i < 10; i++)
  {
    digitalWrite(myLed, HIGH);
    delay(100);
    digitalWrite(myLed, LOW);
    delay(100);
  }

  digitalWrite(myLed, HIGH);

  currentMillis = millis();

}

//*************************************************


void getTemperature() {
  float tempC;
  //  float tempF;

  if (!sensorEnabled) {
    return;
  }

  do {
    DS18B20.requestTemperatures();
    tempC = DS18B20.getTempCByIndex(0);
    tempF = DS18B20.getTempFByIndex(0);
    delay(100);
  } while (tempC == 85.0 || tempC == (-127.0));

  tempF2 = (tempF * 10) + 0.5 ;
  int inttempF = (int)tempF2;
  tempF2 = (float) inttempF / 10;
}



//*************************************************

void loop(void) {

  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  server.handleClient();

  currentMillis = millis();

  // every second
  if (currentMillis - previousMillis >= oneSecond) {
    cntSeconds += 1;
    ArduinoOTA.handle();
    server.handleClient();
    previousMillis = previousMillis + oneSecond;
  }
  ArduinoOTA.handle();
  server.handleClient();

  // 8 May 2020 - changed to every 120 seconds
  if ((cntSeconds % 120) == 0) {
    getTemperature();
  }

  // must be here or server will disconnect client 
  client.loop();

}


//*************************************************
void setupOTA() {

  ArduinoOTA.setHostname(WiFi_hostname);
  ArduinoOTA.onStart([]() {
    digitalWrite(myLed, LOW);
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    digitalWrite(myLed, !digitalRead(myLed));
    // Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end

    for (int i = 0; i < 10; i++)
    {
      digitalWrite(myLed, HIGH);
      delay(100);
      digitalWrite(myLed, LOW);
      delay(100);
    }

    digitalWrite(myLed, LOW);   // turn off blue LED

    // had to add on this computer to make OTA finish properly
    ESP.restart();
  });

  ArduinoOTA.onError([](ota_error_t error) {
    (void)error;
    ESP.restart();
  });

  // setup the OTA server
  ArduinoOTA.begin();

}
