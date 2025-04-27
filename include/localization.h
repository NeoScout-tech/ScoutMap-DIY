#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <WString.h>
#include <ArduinoJson.h>
#include <pgmspace.h>

class Localization {
private:
  String currentLang = "en"; // По умолчанию английский
  StaticJsonDocument<1024> langDoc; // Уменьшено с 1536 до 1024

  static const char langJson[] PROGMEM;

public:
  void init();
  void setLanguage(String lang);
  String getString(String key);
  String getString(String key, String arg1);
  String getString(String key, int arg1);
  String getString(String key, String arg1, String arg2);
  String getString(String key, String arg1, int arg2);
  String getString(String key, int arg1, int arg2);
  String getString(String key, int arg1, String arg2, int arg3);
};

extern Localization loc; // Объявление глобального объекта

#endif