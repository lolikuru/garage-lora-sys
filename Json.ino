bool loadConfig() {
  // Открываем файл для чтения
  File configFile = LittleFS.open(config_filename, "r");
  if (!configFile) {
    // если файл не найден
    Serial.println("Failed to open config file");
    //  Создаем файл запиcав в него даные по умолчанию
    saveConfig();
    //configFile.close();
    return false;
  } 
  // Проверяем размер файла, будем использовать файл размером меньше 4096 байта
  size_t size = configFile.size();
  if (size > 4096) {
    Serial.println("Config file size is too large");
    configFile.close();
    return false;
  }
  // загружаем файл конфигурации в глобальную переменную
  // Резервируем памяь для json обекта буфер может рости по мере необходимти предпочтительно для ESP8266
  JsonDocument docConfig;
  DeserializationError error = deserializeJson(docConfig, configFile);//десериализуем конфиг
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  if (docConfig["SSDPName"].as<String>() != "null"){
    SSDPName = docConfig["SSDPName"].as<String>();
    ssidAPName = docConfig["ssidAPName"].as<String>(); // Так получаем строку
    ssidAPPassword = docConfig["ssidAPPassword"].as<String>();
    ssidName = docConfig["ssidName"].as<String>();
    ssidPassword = docConfig["ssidPassword"].as<String>();
    for (byte i = 0; i < 8; i++)
    { //так получаем число и строку в массиве
      Pinout_name[i] = docConfig["Load" + String(i)][0].as<String>();
      Pinout[i] = docConfig["Load" + String(i)][1];
    }
    SSDPName = docConfig["Client" + String(client_id)][0].as<String>();
    timezone = docConfig["Client" + String(client_id)][1];
    ntp = docConfig["Client" + String(client_id)][2].as<String>();
    off_display_sec = docConfig["Client" + String(client_id)][3];
    allways_on_disp = docConfig["Client" + String(client_id)][4];
    led_msg = docConfig["Client" + String(client_id)][5];
    print_logf_status = docConfig["Client" + String(client_id)][6];
    procent_battery = docConfig["Client" + String(client_id)][7];
    if (!docConfig["Client" + String(client_id)][8].isNull()) {
      Wifi_boot = docConfig["Client" + String(client_id)][8];
    }
    strncpy(ntpServer, ntp.c_str(), sizeof(ntpServer) - 1);
    ntpServer[sizeof(ntpServer) - 1] = '\0';
  } 
  configFile.close();
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
  docConfig["SSDPName"] = SSDPName;
  docConfig["ssidAPName"] = ssidAPName;
  docConfig["ssidAPPassword"] = ssidAPPassword;
  docConfig["ssidName"] = ssidName;
  docConfig["ssidPassword"] = ssidPassword;

  for (byte i = 0; i < 8; i++) {
    JsonArray loads = docConfig["Load" + String(i)].to<JsonArray>();
    loads.add(Pinout_name[i]);
    loads.add(Pinout[i]);
  }

  JsonArray SelfSettings = docConfig["Client" + String(client_id)].to<JsonArray>();
  SelfSettings.add(SSDPName); //0
  SelfSettings.add(timezone); //1
  SelfSettings.add(ntp); //2
  SelfSettings.add(off_display_sec); //3
  SelfSettings.add(allways_on_disp); //4
  SelfSettings.add(led_msg);//5
  SelfSettings.add(print_logf_status);//6
  SelfSettings.add(procent_battery);//7
  SelfSettings.add(Wifi_boot);//8
  
  serializeJson(docConfig, jsonConfig);
  // Открываем файл для записи
  File configFile = LittleFS.open(config_filename, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    configFile.close();
    return false;
  }
  // Записываем строку json в файл
  serializeJson(docConfig, configFile);
  configFile.close();
  readFile(LittleFS, config_filename);
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

//void test_json(const char* filename) {
//  Serial.println(F("Loading configuration..."));
//  loadConfig();
//  printFile(config_filename);
//}
