bool loadConfig() {
  // Открываем файл для чтения
  File configFile = LittleFS.open(config_filename, "r");
  if (!configFile) {
    // если файл не найден
    Serial.println("Failed to open config file");
    //  Создаем файл запcав в него даные по умолчанию
    saveConfig();
    configFile.close();
    return false;
  }
  // Проверяем размер файла, будем использовать файл размером меньше 1024 байта
  size_t size = configFile.size();
  if (size > 4096) {
    Serial.println("Config file size is too large");
    configFile.close();
    return false;
  }
  // загружаем файл конфигурации в глобальную переменную
  jsonConfig = configFile.readString();
  configFile.close();
  // Резервируем памяь для json обекта буфер может рости по мере необходимти предпочтительно для ESP8266
  JsonDocument docConfig;
  //  вызовите парсер JSON через экземпляр docConfig
  //  строку возьмем из глобальной переменной String jsonConfig
 //JsonObject root = docConfig.parseObject(jsonConfig);
  //deserializeJson(docConfig, jsonConfig);
  // Теперь можно получить значения из root
  _ssidAP = docConfig["ssidAPName"].as<String>(); // Так получаем строку
  _passwordAP = docConfig["ssidAPPassword"].as<String>();
  time_zone = docConfig["timezone"];               // Так получаем число
  SSDP_Name = docConfig["SSDPName"].as<String>();
  _ssid = docConfig["ssidName"].as<String>();
  _password = docConfig["ssidPassword"].as<String>();
  _ntp = docConfig["ntp"].as<String>();
  for (byte i = 0; i < 8; i++)
  { //так получаем число и строку в массиве
    Pinout_name[i] = docConfig["Load" + String(i)][0].as<String>();
    Pinout[i] = docConfig["Load" + String(i)][1];
    //Alarm_on[i] = root["pin"+ String(i)][2].as<String>();
    //Alarm_off[i] = root["pin"+ String(i)][3].as<String>();
    //alarm_state_on[i] = root["pin"+ String(i)][4].as<String>();
    //alarm_state_off[i] = root["pin"+ String(i)][5].as<String>();
  }
  return true;
}

// Запись данных в файл config.json
bool saveConfig() {
  // Резервируем память для json обекта буфер может рости по мере необходимти предпочтительно для ESP8266
  JsonDocument docConfig;
  deserializeJson(docConfig, jsonConfig);
  
  //  вызовите парсер JSON через экземпляр docConfig
  //JsonObject& json = docConfig.parseObject(jsonConfig);
  
  // Заполняем поля json
  docConfig["SSDPName"] = SSDP_Name;
  docConfig["ssidAPName"] = _ssidAP;
  docConfig["ssidAPPassword"] = _passwordAP;
  docConfig["ssidName"] = _ssid;
  docConfig["ssidPassword"] = _password;
  docConfig["timezone"] = time_zone;
  docConfig["ntp"] = _ntp;

  for (byte i = 0; i < 8; i++) {
    JsonArray loads = docConfig["Load" + String(i)].to<JsonArray>();
    loads.add(Pinout_name[i]);
    loads.add(Pinout[i]);
    //  pins.add(Alarm_on[i]);
    //  pins.add(Alarm_off[i]);
    //  pins.add(alarm_state_on[i]);
    //  pins.add(alarm_state_off[i]);
  }
  // Помещаем созданный json в глобальную переменную json.printTo(jsonConfig);
  //docConfig.printTo(jsonConfig);
  serializeJson(docConfig, jsonConfig);
  // Открываем файл для записи
  File configFile = LittleFS.open(config_filename, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    configFile.close();
    return false;
  }
  // Записываем строку json в файл
  //json.printTo(configFile);
  serializeJson(docConfig, configFile);
  configFile.close();
  return true;
}


// Prints the content of a file to the Serial
void printFile(const char* filename) {
  // Open file for reading
  File file = LittleFS.open(filename);
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}

void test_json(const char* filename) {
  Serial.println(F("Loading configuration..."));
  loadConfig();
  printFile(config_filename);
}
