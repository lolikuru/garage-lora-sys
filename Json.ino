bool saveConfig() {
  JsonDocument docConfig;

  // Сетевые настройки
  docConfig["SSDPName"] = SSDPName;
  docConfig["ssidAPName"] = ssidAPName;
  docConfig["ssidAPPassword"] = ssidAPPassword;
  docConfig["ssidName"] = ssidName;
  docConfig["ssidPassword"] = ssidPassword;

  // Массив нагрузок (вместо разрозненных Load0, Load1 ...)
  JsonArray loadsArray = docConfig["loads"].to<JsonArray>();
  for (byte i = 0; i < 8; i++) {
    JsonObject item = loadsArray.add<JsonObject>();
    item["name"] = Pinout_name[i];
    item["pin"] = Pinout[i];
  }

  // Настройки клиента в виде именованного объекта
  char clientKey[16];
  snprintf(clientKey, sizeof(clientKey), "Client%d", client_id);

  JsonObject clientObj = docConfig[clientKey].to<JsonObject>();
  clientObj["name"]              = SSDPName;
  clientObj["timezone"]          = timezone;
  clientObj["ntp"]               = ntp;
  clientObj["off_display_sec"]   = off_display_sec;
  clientObj["allways_on_disp"]   = allways_on_disp;
  clientObj["led_msg"]           = led_msg;
  clientObj["print_logf_status"] = print_logf_status;
  clientObj["procent_battery"]   = procent_battery;
  clientObj["wifi_boot"]         = Wifi_boot;
  clientObj["frequency"]         = Frequency;

  // Запись в файл
  File configFile = LittleFS.open(config_filename, "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing"));
    return false;
  }

  size_t bytesWritten = serializeJson(docConfig, configFile);
  configFile.close();

  if (bytesWritten == 0) {
    Serial.println(F("Failed to write data to file"));
    return false;
  }

  readFile(LittleFS, config_filename);
  return true;
}

// Чтение данных из файла config.json
bool loadConfig() {
  File configFile = LittleFS.open(config_filename, "r");
  if (!configFile) {
    Serial.println(F("Failed to open config file, saving defaults..."));
    saveConfig();
    return false;
  }

  if (configFile.size() > 4096) {
    Serial.println(F("Config file size is too large"));
    configFile.close();
    return false;
  }

  JsonDocument docConfig;
  DeserializationError error = deserializeJson(docConfig, configFile);
  configFile.close();

  if (error) {
    Serial.println(F("Failed to parse JSON, keeping defaults"));
    return false;
  }

  // Считываем базовые настройки
  if (!docConfig["SSDPName"].isNull()) {
    SSDPName = docConfig["SSDPName"].as<String>();
  }
  if (!docConfig["ssidAPName"].isNull()) {
    ssidAPName = docConfig["ssidAPName"].as<String>();
  }
  if (!docConfig["ssidAPPassword"].isNull()) {
    ssidAPPassword = docConfig["ssidAPPassword"].as<String>();
  }
  if (!docConfig["ssidName"].isNull()) {
    ssidName = docConfig["ssidName"].as<String>();
  }
  if (!docConfig["ssidPassword"].isNull()) {
    ssidPassword = docConfig["ssidPassword"].as<String>();
  }

  // Считываем массив нагрузок
  JsonArray loadsArray = docConfig["loads"].as<JsonArray>();
  if (!loadsArray.isNull()) {
    byte index = 0;
    for (JsonObject load : loadsArray) {
      if (index >= 8) break;
      Pinout_name[index] = load["name"].as<String>();
      Pinout[index]      = load["pin"].as<int>();
      index++;
    }
  }

  // Считываем настройки текущего клиента
  char clientKey[16];
  snprintf(clientKey, sizeof(clientKey), "Client%d", client_id);
  JsonObject clientObj = docConfig[clientKey].as<JsonObject>();

  if (!clientObj.isNull()) {
    if (!clientObj["name"].isNull())              SSDPName = clientObj["name"].as<String>();
    if (!clientObj["timezone"].isNull())          timezone = clientObj["timezone"].as<int>();
    if (!clientObj["ntp"].isNull())               ntp = clientObj["ntp"].as<String>();
    if (!clientObj["off_display_sec"].isNull())   off_display_sec = clientObj["off_display_sec"].as<int>();
    if (!clientObj["allways_on_disp"].isNull())   allways_on_disp = clientObj["allways_on_disp"].as<bool>();
    if (!clientObj["led_msg"].isNull())           led_msg = clientObj["led_msg"].as<bool>();
    if (!clientObj["print_logf_status"].isNull()) print_logf_status = clientObj["print_logf_status"].as<bool>();
    if (!clientObj["procent_battery"].isNull())   procent_battery = clientObj["procent_battery"].as<bool>();
    if (!clientObj["wifi_boot"].isNull())         Wifi_boot = clientObj["wifi_boot"].as<bool>();
    if (!clientObj["frequency"].isNull())         Frequency = clientObj["frequency"].as<int>();

    strncpy(ntpServer, ntp.c_str(), sizeof(ntpServer) - 1);
    ntpServer[sizeof(ntpServer) - 1] = '\0';
  }

  return true;
}