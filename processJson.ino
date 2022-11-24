#include <ArduinoJson.h>

void processJson (String inputJs) {
  String outJSON;

  /////   DECODE INCOMING HTTP JSON MESSAGE

  Serial.print(inputJs);
  DynamicJsonDocument doca(3072);

  DeserializationError error = deserializeJson(doca, inputJs);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  if (doca["cod"] = "200") {

    /////   SERIALIZE NEW MESSAGE

    DynamicJsonDocument docs(384);

    docs["cod"] = "forecast";

    //String jsonString = JSON.stringify(doca["city"]["sunrise"].as<long>());
    //docs["sunrise"]         =       doca["city"]["sunrise"].as<long>();
    //docs["sunrise"] = jsonString;

    docs["sunrise"]         =     doca["city"]["sunrise"];
    docs["sunset"]          =     doca["city"]["sunset"];
    docs["temp"]            =     doca["list"][0]["main"]["temp"];
    docs["feels_like"]      =     doca["list"][0]["main"]["feels_like"];
    docs["temp_min"]        =     doca["list"][0]["main"]["temp_min"];
    docs["temp_max"]        =     doca["list"][0]["main"]["temp_max"];
    docs["humidity"]        =     doca["list"][0]["main"]["humidity"];
    docs["pressure"]        =     doca["list"][0]["main"]["pressure"];
    docs["visibility"]      =     doca["list"][0]["visibility"];
    docs["clouds_all"]      =     doca["list"][0]["clouds"]["all"];
    docs["wind_speed"]      =     doca["list"][0]["wind"]["speed"];         //2.94;
    docs["rain"]            =     doca["list"][0]["rain"]["main"];          //="Rain";
    docs["dt"]              =     doca["list"][0]["dt_txt"];

    //serializeJsonPretty(docs, Serial);

    JsonObject rain = docs.createNestedObject("rain");
    
    rain["id"]              =     doca["list"][0]["rain"]["id"];          //=500;
    rain["3h"]              =     doca["list"][0]["rain"]["3h"];          //=0.76;
    rain["icon"]            =     doca["list"][0]["rain"]["icon"];        //"10d";
    rain["description"]     =     doca["list"][0]["rain"]["description"]; //"light rain";


    serializeJson(docs, outJSON);
    //Serial.print(outJSON);
    sendCommand(outJSON, 1);
  } else {

    Serial.print("Error getting JSON forecast");


  }


}
