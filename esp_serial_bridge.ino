

#include <ArduinoJson.h>

/*
   ESP8266 MQTT Wifi Client to Serial Bridge with NTP
   Author: rkubera https://github.com/rkubera/
   License: MIT
*/

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <String.h>
#include <time.h>                       // time() ctime()
#include <sys/time.h>  

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
                 // struct timeval
//#include <coredecls.h>                  // settimeofday_cb()
IPAddress ip;  

//NTP
#define TZ              0       // (utc+) TZ in hours
#define DST_MN          0      // use 60mn for summer time in some countries
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)
timeval cbtime;      // time set in callback
bool cbtime_set = false;


//WIFIsettings
String ssid = "oooooio";
String  password = "pmgana921";
#ifndef STASSID
#define STASSID "oooooio"
#define STAPSK  "pmgana921"
#endif

bool wificonnected = false;

//MQTT Client
WiFiClient espGateway;
PubSubClient client(espGateway);

String mqtt_server = "192.168.8.150";
int mqtt_port = 1883;
String mqtt_user = "mqtt";
String mqtt_pass = STAPSK;
String mqtt_allSubscriptions = "home/MQTTGateway/#";



// String mqtt_server ="192.168.8.107";
// int mqtt_port = 1883;
// String mqtt_user = "";
// String mqtt_pass = "";
// String mqtt_allSubscriptions = "";



//BUFFER
#define bufferSize 1024
uint8_t myBuffer[bufferSize];
int bufIdx = 0;

//MQTT payload
long lastMsg = 0;
char msg[bufferSize];
long int value = 0;

void time_is_set_cb(void) {
  gettimeofday(&cbtime, NULL);
  cbtime_set = true;
  if (ssid != "" && WiFi.status() == WL_CONNECTED) {
    cbtime_set = true;
  }
  else {
    cbtime_set = false;
  }
}



void mqtt_cb(char* topic, byte* payload, unsigned int length) {
  CRC32_reset();
  for (size_t i = 0; i < strlen(topic); i++) {
    CRC32_update(topic[i]);
  }

  CRC32_update(' ');

  for (size_t i = 0; i < length; i++) {
    CRC32_update(payload[i]);
  }
  uint32_t checksum = CRC32_finalize();

  Serial.print(checksum);
  Serial.print(" ");
  Serial.print(topic);
  Serial.print (" ");
  Serial.write (payload, length);
  Serial.println();
  if ((char)payload[0] == '1') {
    //digitalWrite(LED_, LOW);   // Turn the LED on (Note that LOW is the voltage level
  } else {
    //digitalWrite(LED_, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

void reSubscribe() {
  int start = 0;
  int lineIdx;
  String sub;
  do {
    lineIdx = mqtt_allSubscriptions.indexOf('\n', start);
    if (lineIdx > -1) {
      sub = mqtt_allSubscriptions.substring(start, lineIdx);
      start = lineIdx + 1;
      sub.trim();
    }
    else {
      sub = "";
    }
    if (sub.length() > 0) {
      client.subscribe(sub.c_str());
    }
  }
  while (lineIdx > -1);
}
bool reconnect() {
  // Create a random client ID
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  delay(100);
  if (mqtt_user != "") {
    if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_pass.c_str())) {
      sendCommand("mqtt connected", 0);
      return true;
    }
  }
  else {
    if (client.connect(clientId.c_str())) {
      sendCommand("mqtt connected", 0);
      return true;
    }
  }
  return false;
}

void setup() {
  delay(1000);
  //ESP.eraseConfig();
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize the BUILTIN_LED pin as an output

  //Serial
  Serial.begin(9600);

  Serial.println();
  delay(200);
  sendCommand("ready", 0);


  /// OTA
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);

    // Wait for a second
  
    delay(2000);
   
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  // ArduinoOTA.setPassword("admin");
//
//  ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
 ArduinoOTA.begin();
  ArduinoOTA.setHostname("AVR-Termostat-DUE");
  /// OTA

  WiFi.mode(WIFI_OFF);
  delay(120);
  //WIFI
  WiFi.mode(WIFI_STA);


  client.setCallback(mqtt_cb);
  Serial.println("Wifi Mode STA");
  //NTP
  //settimeofday_cb(time_is_set_cb);
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");


    



  

}

void loop() {



  if (WiFi.status() == WL_CONNECTED) {


  }

  if (ssid != "" && WiFi.status() != WL_CONNECTED) {

    wificonnected = false;
    WiFi.begin(ssid.c_str(), password.c_str());
    for (int i = 0; i < 1000; i++) {
      if ( WiFi.status() != WL_CONNECTED ) {
        delay(10);
        commandLoop();
        
      }
      else {
        sendCommand("wifi connected", 0);
        wificonnected = true;

    delay(1000);
   
 ip = WiFi.localIP();
    Serial.println(ip);

  
   
        

        if (mqtt_server != "") {
          if (reconnect()) {
            reSubscribe();
          }
        }
        break;
      }
    }
  }
  else if (ssid != "") {
    if (wificonnected == true) {
      if (!client.connected() && mqtt_server != "") {
        if (reconnect()) {
          reSubscribe();
        }
      }
      else {
        client.loop();
      }
    }
  }
  ArduinoOTA.handle();
  commandLoop();
  yield();
}
