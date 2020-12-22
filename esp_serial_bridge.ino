/*
 * ESP8266 MQTT Wifi Client to Serial Bridge with NTP
 * Author: rkubera https://github.com/rkubera/
 * License: MIT
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <String.h>
#include <ESP8266HTTPClient.h>
#include <String.h>
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
//#include <coredecls.h>                  // settimeofday_cb()

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

bool wificonnected = false;

//MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

String mqtt_server ="192.168.8.105";
int mqtt_port = 1883;
String mqtt_user = "pi";
String mqtt_pass = "root";
String mqtt_allSubscriptions = "czujniki";


// String mqtt_server ="192.168.8.107";
// int mqtt_port = 1883;
// String mqtt_user = "";
// String mqtt_pass = "";
// String mqtt_allSubscriptions = "";



//BUFFER
#define bufferSize 1024
uint8_t myBuffer[bufferSize];
int bufIdx=0;

//MQTT payload
long lastMsg = 0;
char msg[bufferSize];
long int value = 0;

void time_is_set_cb(void) {
  gettimeofday(&cbtime, NULL);
  cbtime_set = true;
  if (ssid!="" && WiFi.status() == WL_CONNECTED) {
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
    if (lineIdx>-1) {
      sub = mqtt_allSubscriptions.substring(start, lineIdx);
      start = lineIdx+1;
      sub.trim();
    }
    else {
      sub = "";
    }
    if (sub.length()>0) {
      client.subscribe(sub.c_str());
    }
  }
  while (lineIdx>-1);
}
bool reconnect() {
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    delay(100);
    if (mqtt_user!="") {
      if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_pass.c_str())) {
        sendCommand("mqtt connected",0);
        return true;
      }
    }
    else {
      if (client.connect(clientId.c_str())) {
        sendCommand("mqtt connected",0);
        return true;
      }
    }
    return false;
}

void setup() {
  delay(1000);
  //ESP.eraseConfig();
     // Initialize the BUILTIN_LED pin as an output

  //Serial
  Serial.begin(9600);
  
  Serial.println();
  delay(200);
  sendCommand("ready",0);




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

if(WiFi.status() == WL_CONNECTED){
          
}

  if (ssid!="" && WiFi.status() != WL_CONNECTED) {
    
    wificonnected=false;
    WiFi.begin(ssid.c_str(), password.c_str());
    for (int i = 0; i < 1000; i++) {
      if ( WiFi.status() != WL_CONNECTED ) {
        delay(10);
        commandLoop();
      }
      else {
        sendCommand("wifi connected",0);
        wificonnected=true;
        if (mqtt_server!="") {
          if (reconnect()) {
             reSubscribe();
          }
        }
        break;
      }
    }
  }
  else if (ssid!="") {
    if (wificonnected==true) {
      if (!client.connected() && mqtt_server!="") {
        if (reconnect()) {
           reSubscribe();
        }
      }
      else {
        client.loop();
      }
    }
  }
  commandLoop();
  yield();
}
