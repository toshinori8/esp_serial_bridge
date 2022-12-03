/*
 * ESP8266 MQTT Wifi Client to Serial Bridge with NTP  // Added suport for http requests. 
 * Author: rkubera https://github.com/rkubera/
 * License: MIT
 */

#include <String.h>



void sendCommand(String myCommand, int raw)
{

  digitalWrite(LED_BUILTIN, LOW);
delay(50);
  digitalWrite(LED_BUILTIN, HIGH);
delay(100);
  digitalWrite(LED_BUILTIN, LOW);
delay(50);
  digitalWrite(LED_BUILTIN, HIGH);




  if (raw == 1)
  {
    Serial.print(myCommand);
  }
  else
  {
    CRC32_reset();
    CRC32_update('[');
    for (size_t i = 0; i < myCommand.length(); i++)
    {
      CRC32_update(myCommand[i]);
    }
    CRC32_update(']');
    uint32_t checksum = CRC32_finalize();

    Serial.print(checksum);
    Serial.print(" ");
    Serial.print("[");
    Serial.print(myCommand);
    Serial.println("]");
  }
}

void commandLoop()
{
  if (getSerialBuffer((char *)myBuffer, bufIdx))
  {
    parseLines((char *)myBuffer);
  }
}

void getCommand(String payload)
{
  int xtmpIdx;
  xtmpIdx = payload.indexOf('*');
  if (xtmpIdx > -1)
  {
    String payload_A = payload.substring(0, xtmpIdx);
    String payload_B = payload.substring(xtmpIdx + 1);
    if (payload_A == "url")
    {
      String Hpayload;
      HTTPClient http;
      http.begin(HTTPClient, payload_B); // Request URL
      int httpCode = http.GET();
      Serial.println("httpCode  = " + httpCode);
      if (httpCode > 0)
      { //Check the returning code
        Hpayload = http.getString(); //Get the request response payload
      }
      sendCommand(Hpayload, 1);
      http.end();
    }
  }else if (payload == "timestamp")
  {
    uint32_t mytimestamp = 0;
    mytimestamp = time(nullptr);
    String command = "timestamp ";
    if (cbtime_set == true)
    {
      mytimestamp = time(nullptr);
      if (mytimestamp < 1000000)
      {
        mytimestamp = 0;
      }
    }
    command = command + (String)mytimestamp;
    sendCommand(command, 0);
  }
  else if (payload == "echo")
  {
    sendCommand("echo", 0);
  }
  else if (payload == "ip")
  {
    IPAddress ip = WiFi.localIP();
    String command =
        String(ip[0]) + "." +
        String(ip[1]) + "." +
        String(ip[2]) + "." +
        String(ip[3]);
    sendCommand(command, 0);
  }
 
 
 /// GET FORECAST BY SERIAL CONSOLE
 
  else if (payload == "forecast_5h")
  {
    String Hpayload;
    HTTPClient http;

    http.begin(HTTPClient, "http://api.openweathermap.org/data/2.5/forecast?lat=49.8808919&lon=19.5607773&appid=f055d509de51700a688e61d5f8e3da76&units=metric&cnt=3"); //Specify request destination
    int httpCode = http.GET();

    //Serial.println("httpCode  = " + httpCode);

    if (httpCode > 0)
    { //Check the returning code

     Hpayload = http.getString(); //Get the request response payload
    }
          
 
      //Serial.print(Hpayload);

  
        
        processJson(Hpayload);
        //sendCommand(Hpayload, 1);
        http.end();
        
    
  }
  else if (payload == "startpair"){

              simpleEspConnection.startPairing(30);
              sendCommand("Pairing mode ON 30sec", 0);

  }
  else if (payload == "endpair"){

              simpleEspConnection.endPairing();
              sendCommand("Pairing mode OFF ", 0);

  }
   else if (payload == "endpair"){

              simpleEspConnection.endPairing();
              sendCommand("Pairing mode OFF ", 0);

  }
  else if (payload == "wifistatus")
  {
    if (wificonnected == true)
    {
      sendCommand("wifi connected", 0);
    }
    else
    {
      sendCommand("wifi not connected", 0);
    }
  }
  else if (payload == "mqttstatus")
  {
    if (client.connected())
    {
      sendCommand("mqtt connected", 0);
    }
    else
    {
      sendCommand("mqtt not connected", 0);
    }
  }
  else if (payload == "ssid")
  {
    sendCommand(ssid, 0);
  }
  else if (payload == "mqttserver")
  {
    sendCommand(mqtt_server, 0);
  }
  else if (payload == "mqttport")
  {
    sendCommand((String)mqtt_port, 0);
  }
  else if (payload == "mqttuser")
  {
    sendCommand(mqtt_user, 0);
  }
  else
    sendCommand("error", 0);
}

void publishCommand(String payload)
{
  if (!client.connected())
  {
    sendCommand("mqtt not connected", 0);
    return;
  }
  int tmpIdx;
  tmpIdx = payload.indexOf(' ');
  if (tmpIdx > -1)
  {
    client.publish(payload.substring(0, tmpIdx).c_str(), payload.substring(tmpIdx + 1).c_str());
    sendCommand("published", 0);
  }
  else
  {
    sendCommand("wrong publish command", 0);
  }
}

void publishretainedCommand(String payload)
{
  if (!client.connected())
  {
    sendCommand("mqtt not connected", 0);
    return;
  }
  int tmpIdx;
  tmpIdx = payload.indexOf(' ');
  if (tmpIdx > -1)
  {
    client.publish(payload.substring(0, tmpIdx).c_str(), payload.substring(tmpIdx + 1).c_str(), true);
    sendCommand("published", 0);
  }
  else
  {
    sendCommand("wrong publish command", 0);
  }
}
void connectCommand(String payload)
{
  int tmpIdx;
  tmpIdx = payload.indexOf(':');
  if (tmpIdx > -1)
  {
    //AP ssid and password
    ssid = payload.substring(0, tmpIdx);
    password = payload.substring(tmpIdx + 1);
  }
  else
  {
    //open AP, only ssid
    ssid = payload;
  }
  client.disconnect();
  WiFi.disconnect();
  wificonnected = false;
  sendCommand("connecting to wifi", 0);
}

void mqttUserPassCommand(String payload)
{
  int tmpIdx;
  tmpIdx = payload.indexOf(':');
  if (tmpIdx > -1)
  {
    //mqtt server and port
    mqtt_user = payload.substring(0, tmpIdx);
    mqtt_pass = payload.substring(tmpIdx + 1).toInt();
  }
  else
  {
    //mqtt only
    mqtt_user = payload;
  }
  client.disconnect();
  sendCommand("mqtt user and pass set", 0);
}

void mqttServerCommand(String payload)
{
  int tmpIdx;
  tmpIdx = payload.indexOf(':');
  if (tmpIdx > -1)
  {
    //mqtt server and port
    mqtt_server = payload.substring(0, tmpIdx);
    mqtt_port = payload.substring(tmpIdx + 1).toInt();
  }
  else
  {
    //mqtt only
    mqtt_server = payload;
  }
  client.disconnect();
  client.setServer(mqtt_server.c_str(), mqtt_port);
  sendCommand("connecting to mqtt server", 0);
}

void subscribeCommand(String subscription)
{
  if (!client.connected())
  {
    sendCommand("mqtt not connected", 0);
    return;
  }
  int start = 0;
  int lineIdx;
  String sub;
  bool found = false;
  do
  {
    lineIdx = mqtt_allSubscriptions.indexOf('\n', start);
    if (lineIdx > -1)
    {
      sub = mqtt_allSubscriptions.substring(start, lineIdx);
      start = lineIdx + 1;
      sub.trim();
    }
    else
    {
      sub = "";
    }
    if (sub.length() > 0)
    {
      if (sub == subscription)
      {
        found = true;
      }
    }
  } while (lineIdx > -1);
  if (found == false)
  {
    mqtt_allSubscriptions = mqtt_allSubscriptions + subscription + "\n";
    client.subscribe(subscription.c_str());
    sendCommand("subscription added", 0);
  }
  else
  {
    sendCommand("subscription exists", 0);
  }
}

void unsubscribeCommand(String subscription)
{
  if (!client.connected())
  {
    sendCommand("mqtt not connected", 0);
    return;
  }

  int start = 0;
  int lineIdx;
  String sub;
  String newAllSubscriptions = "";
  do
  {
    lineIdx = mqtt_allSubscriptions.indexOf('\n', start);
    if (lineIdx > -1)
    {
      sub = mqtt_allSubscriptions.substring(start, lineIdx);
      start = lineIdx + 1;
      sub.trim();
    }
    else
    {
      sub = "";
    }
    if (sub.length() > 0)
    {
      if (sub != subscription)
      {
        newAllSubscriptions = newAllSubscriptions + sub + "\n";
      }
    }
  } while (lineIdx > -1);
  mqtt_allSubscriptions = newAllSubscriptions;
  client.unsubscribe(subscription.c_str());
  sendCommand("subscription removed", 0);
}
