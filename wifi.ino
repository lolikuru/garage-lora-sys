void WIFIinit() {
  u8g2.clearBuffer();
  u8g2.setCursor(0, 12);
  u8g2.setFont(u8g2_font_6x12_t_symbols);
  // Попытка подключения к точке доступа
  WiFi.mode(WIFI_STA);
  u8g2.println("Wi-Fi connect");
  u8g2.sendBuffer();
  byte tries = 11;
  WiFi.begin(ssidName.c_str(), ssidPassword.c_str());
  // Делаем проверку подключения до тех пор пока счетчик tries
  // не станет равен нулю или не получим подключение
  u8g2.setCursor(78, 12);
  while (--tries && WiFi.status() != WL_CONNECTED)
  {
    u8g2.print(".");
    u8g2.sendBuffer();
    delay(1000);
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    // Если не удалось подключиться запускаем в режиме AP
    u8g2.setCursor(0, 24);
    u8g2.println("WiFi up AP");
    StartAPMode();
    delay(1000);
  }
  else {
    // Иначе удалось подключиться отправляем сообщение
    // о подключении и выводим адрес IP
    u8g2.setCursor(0, 24);
    u8g2.println("WiFi connected");
    u8g2.setCursor(0, 36);
    u8g2.print("IP addr: ");
    u8g2.print(WiFi.localIP());
    u8g2.setCursor(0, 48);
    u8g2.print("Time zone: ");
    u8g2.print(timezone);
    u8g2.setCursor(0, 60);
    u8g2.print("SSID: ");
    u8g2.print(ssidName.c_str());
    u8g2.sendBuffer();
    timeClient.begin();
    timeClient.update();
    delay(1000);

    unsigned long syncedEpoch = timeClient.getEpochTime();
    epochTime = syncedEpoch;
    struct tm *ptm = gmtime((time_t *)&syncedEpoch);
    rtc.setTime(syncedEpoch);

    Serial.printf("Now time is: %02d:%02d:%02d\n", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    delay(5000);
    //WiFi.disconnect(true);
    //WiFi.mode(WIFI_OFF);
    FS_Browser_init();
  }
}

bool StartAPMode() {
  IPAddress apIP(192, 168, 4, 1);
  // Отключаем WIFI
  WiFi.disconnect();
  // Меняем режим на режим точки доступа
  WiFi.mode(WIFI_AP);
  // Задаем настройки сети
  WIFI_AP_on = true;
  //Выключаем авторизацию в AP режиме
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  // Включаем WIFI в режиме точки доступа с именем и паролем
  // хронящихся в переменных _ssidAP _passwordAP
  WiFi.softAP(ssidAPName.c_str(), ssidAPPassword.c_str());

    // Route for root / web page
//  server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
//    if (LittleFS.exists(LOG_FILE_PATH)) {
//      request->send(LittleFS, LOG_FILE_PATH, "text/plain");
//    } else {
//      request->send(404, "text/plain", "Log file not found");
//    }
//  });
  FS_Browser_init();
  //server.begin();
  
  return true;
}
