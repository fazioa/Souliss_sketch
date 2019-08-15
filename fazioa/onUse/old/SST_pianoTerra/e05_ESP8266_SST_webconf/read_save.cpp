#include <Arduino.h>
#include "constants.h"
#include "preferences.h"
#include "read_save.h"
#include "FS.h"
#include <ArduinoJson.h>

void save_spiffs_prefs(int json_iDisplayBright, int json_bClock, int json_timeZone, int json_DayLightSavingTime, int json_bCrono, int json_bCronoLearn, int json_bSystem, int json_bLayout1, int json_bLayout2) {
  SPIFFS.begin();
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["Luminosita"] = json_iDisplayBright;
  root["Orologio"] = json_bClock;
  root["Tzone"] = json_timeZone;
  root["DayLightSavingTime"] = json_DayLightSavingTime;
  root["Crono"] = json_bCrono;
  root["CronoLearn"] = json_bCronoLearn;
  root["Dispositivo"] = json_bSystem;
  root["Layout1"] = json_bLayout1;
  root["Layout2"] = json_bLayout2;
  //Serial.print("Ecco i dati in json: ");
  //root.printTo(Serial);
  char buffer[256];
  root.printTo(buffer, sizeof(buffer));
  //Serial.println();

  // open file for writing
  File sst_spiffs = SPIFFS.open("/sst_settings.json", "w");
  if (!sst_spiffs) {
    #ifdef DEBUG_DEV
      Serial.println("sst_settings.json open failed");
    #endif
  }
  //qui salvo il buffer su file
  sst_spiffs.println(buffer);
  #ifdef DEBUG_DEV
    Serial.print("Salvo in SPIFFS il buffer con i settings :"); Serial.println(buffer);
  #endif
  delay(1);
  //chiudo il file
  sst_spiffs.close();
}

int read_spiffs_prefs(const char*  valuedaleggere) {
  File  sst_spiffs_inlettura = SPIFFS.open("/sst_settings.json", "r");
  if (!sst_spiffs_inlettura) {
    #ifdef DEBUG_DEV
      Serial.println("sst_settings.json open failed");
    #endif
    return 0;
  }
  String risultato = sst_spiffs_inlettura.readStringUntil('\n');
  //Serial.print("Ho letto dal file : ");Serial.println(risultato);
  char json[200];
  risultato.toCharArray(json, 200);
  //Serial.print("Ecco l'array json convertito: ");Serial.println(json);
  StaticJsonBuffer<200> jsonBuffer_inlettura;
  JsonObject& root_inlettura = jsonBuffer_inlettura.parseObject(json);
  if (!root_inlettura.success()) {
    #ifdef DEBUG_DEV
      Serial.println("parseObject() failed");
    #endif
    return 0;
  }
  //leggo il valore e lo parso:
  int risultatoparsed = root_inlettura[valuedaleggere];
  #ifdef DEBUG_DEV
    Serial.print("Spiffs Json parsed value of "); Serial.print(valuedaleggere); Serial.print(" :");Serial.println(risultatoparsed);
  #endif
  sst_spiffs_inlettura.close();
  return risultatoparsed;
}


void spiffs_Reset() {
  #ifdef DEBUG_DEV
    Serial.println("Reset SST");
    Serial.println("SPIFFS Formatting... ");
  #endif
  if (SPIFFS.format()) {
    #ifdef DEBUG_DEV
      Serial.println("OK");
    #endif
  } else {
    #ifdef DEBUG_DEV
      Serial.println("Fail");
    #endif
  }
}



