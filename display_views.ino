const char * onOff(bool input) {
  const char* msg = "OFF";
  if (input) {
    msg = "ON";
  }
  return msg;
}

void main_view() {

  u8g2.clearBuffer();
  printVBat(false);
  u8g2.setCursor(0, 24);
  u8g2.print(rtc.getTime("%d/%b/%Y %H:%M:%S"));
  u8g2.setFont(u8g2_font_10x20_tf );
  u8g2.setCursor(0, 42);
  //String DHT_substring = String(host_temp).substring(0, 4) + "ºC " + String(host_humid).substring(0, 4) + "%";
  u8g2.print(String(host_temp).substring(0, 4) + "ºC " + String(host_humid).substring(0, 4) + "%");
  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.print(" ");

  if (print_logf_status) {
    u8g2.setFont(u8g2_font_6x12_t_symbols);
    u8g2.setCursor(32, 62);
    u8g2.print("log size:");
    u8g2.print(get_log_size(LittleFS));
    u8g2.print("K");
  }

  //  if (Wifi_boot) {
  //    //epochTime = timeClient.getEpochTime();
  //    //tm *ptm = gmtime((time_t *)&epochTime);//time update
  //    u8g2.setCursor(38, 12);
  //    u8g2.print(epochTime);
  //    //u8g2.printf("%02d:%02d:%02d\n", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
  //  }

#ifdef ENABLE_RSSI
  if (lastRssi > 0) {
    Serial.println(lastRssi);
    lora_link = 0;
    //if (led_msg) digitalWrite(LED_PIN, HIGH);
    old_rssi = lastRssi;
    u8g2.setFont(u8g2_font_siji_t_6x10);
    if (r_info.save) {
      Serial.print("RSSI: "); Serial.println(lastRssi, DEC);
      u8g2.drawGlyph(0, 12, lora_symb[lora_link]);
      u8g2.setDrawColor(1);
    }
  }
  u8g2.setCursor(12, 12);
  u8g2.print(old_rssi, DEC);
//    u8g2.setCursor(12, 12);
//    u8g2.print(old_rssi, DEC);

#endif
    u8g2.setFont(u8g2_font_siji_t_6x10);
    u8g2.drawGlyph(0, 12, lora_symb[lora_link]);
    if (millis() > icon_timestamp + 10000 ) { //update 1 time in 10 sec
      icon_timestamp = millis();
      if ( lora_link < 3 ) lora_link++;
    }

    u8g2.sendBuffer();
//    if (r_info.save) {
//      String log_str = rtc.getTime("%d/%b/%Y %H:%M:%S") + "|" + DHT_substring + "|" + lastRssi + "|" + String(realVBat()) + "\n";
//      Serial.println(log_str);
//      appendFile(LittleFS, "/log.txt", log_str.c_str());
//      r_info.save = false;
//    }
    lastRssi = 0;
  }

  void main_menu() {
    //cursor string
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_t_symbols);
    const char *main_list =
      "ON/OFF 8ch switch\n"
      "Hub settings\n"
      "Temp/Humid Trigger\n"
      "Time Trigger\n"
      "Client Msg Event\n"
      "6\n"
      "7\n"
      "Settings\n"
      "TEST_menu\n"
      "EXIT";


    uint8_t current_main_selection = u8g2.userInterfaceSelectionList(
                                       "Main menu",
                                       current_main_selection,
                                       main_list);
    if ( current_main_selection == 1 ) {

    }
    else if ( current_main_selection == 2 ) {

    }
    else if ( current_main_selection == 3) {

    }
    else if ( current_main_selection == 4 ) {

    }
    else if ( current_main_selection == 5 ) {

    }
    else if ( current_main_selection == 6 ) {

    }
    else if ( current_main_selection == 7 ) {

    }
    else if ( current_main_selection == 8 ) {
      settings_menu();
    }
    else if ( current_main_selection == 9 ) {
      log_menu();
    }

    else if ( current_main_selection == 10 ) {
      current_main_selection = 0;
      u8g2.clearBuffer();
      //  if ( current_selection == 0 ) {
      //    menu_page = 0;
      //  }

    }
  }

  void log_menu() { //cursor string
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_t_symbols);
    const char *debug_list =
      "Send test temp\n"
      "Json delete\n"
      "Test json config\n"
      "Print LoRa CFG\n"
      "LED MSG\n"
      "Wifi log AP\n"
      "Log size view\n"
      "Print log\n"
      "Delete log\n"
      "FS List\n"
      "Back to main\n"
      "EXIT";

    uint8_t current_selection = u8g2.userInterfaceSelectionList(
                                  "TEST_menu",
                                  current_selection,
                                  debug_list);
    Serial.println(current_selection);
    if ( current_selection == 1 ) {
      uint8_t sure = u8g2.userInterfaceMessage(
                       "Send lora temp",
                       "every 5 sec",
                       onOff(send_dht),
                       " ok \n cansel");
      if ( sure == 1 ) send_dht = !send_dht;
    }

    if ( current_selection == 2 ) {
      deleteFile(LittleFS, filename);
      current_selection == 0;
    }

    else if ( current_selection == 3 ) {
      test_json();
      current_selection == 0;
    }

    else if ( current_selection == 4 ) {
      u8g2.userInterfaceMessage(
        "Print LoRa CFG",
        "from uart port",
        "",
        " ok ");
      ResponseStructContainer c;
      c = e220ttl.getConfiguration();
      // It's important get configuration pointer before all other operation
      Configuration configuration = *(Configuration*) c.data;
      Serial.println(c.status.getResponseDescription());
      Serial.println(c.status.code);
      printParameters(configuration);
      c.close();
    }

    else if ( current_selection == 5 ) {
      u8g2.userInterfaceMessage(
        "LED MSG change",
        "now: ",
        onOff(led_msg),
        " ok ");
      led_msg = !led_msg;
    }

    else if ( current_selection == 6 ) {
      uint8_t choice = u8g2.userInterfaceMessage(
                         "Wifi log AP",
                         "now:",
                         onOff(Wifi_boot),
                         " ok \n cancel \n Retry ");
      if (choice == 1) {
        Wifi_boot = !Wifi_boot;
        if (Wifi_boot) {
          WIFIinit();
        } else {
          WiFi.disconnect(true);
          WiFi.mode(WIFI_OFF);
        }
      } else if (choice == 3) {
        if (Wifi_boot) {
          WIFIinit();
        }
      }
    }

    else if ( current_selection == 7 ) {
      const char* logf_on_str = "OFF";
      if (print_logf_status) {
        logf_on_str = "ON";
      }
      uint8_t choice = u8g2.userInterfaceMessage(
                         "Log size view",
                         "now:",
                         onOff(print_logf_status),
                         " change \n cancel ");
      if ( choice == 1 ) {
        print_logf_status = !print_logf_status;
      }
    }
    else if ( current_selection == 8 ) {
      u8g2.userInterfaceMessage(
        "Print log",
        "from uart port",
        "",
        " ok ");
      readFile(LittleFS, "/log.txt");
    }
    else if ( current_selection == 9 ) {
      u8g2.userInterfaceMessage(
        "Delete log",
        "from FS",
        "",
        " ok ");
      deleteFile(LittleFS, "/log.txt");
    }

    else if ( current_selection == 10 ) {
      u8g2.userInterfaceMessage(
        "littlefs List",
        "from uart port",
        "",
        " ok ");
      listDir(LittleFS, "/", 0);
    }
    else if ( current_selection == 11) {
      //menu_page = 1;
      main_menu();
    }

    else if ( current_selection == 12 ) {
      current_selection = 0;
      u8g2.clearBuffer();
    }
  }

  void power_menu() {
    const char *power_list = "Back\n"
                             "Sleep\n"
                             "Power off";
    uint8_t power_item = u8g2.userInterfaceSelectionList(
                           "Power menu",       // Заголовок меню
                           1,                 // Начальная позиция выделения
                           power_list  // Пункты меню
                         );
    if (power_item == 2) {
      uint8_t sure = u8g2.userInterfaceMessage("Selected:", u8x8_GetStringLineStart(power_item - 1, power_list ), "", " Ok \n Cancel ");
      if (sure == 1) {
        //esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, LOW);
        //esp_sleep_enable_uart_wakeup(1);
        light_sleep(true);
      }
    }
    else if (power_item == 3) {
      uint8_t sure = u8g2.userInterfaceMessage("Selected:", u8x8_GetStringLineStart(power_item - 1, power_list ), "", " Ok \n Cancel ");
      String t = String(u8x8_GetStringLineStart(power_item - 1, power_list));
      Serial.println(t);
      if (sure == 1) {
        deep_sleep();
      }
    }
  }

  void settings_menu() {
    const char *settings_list = "CPU Frequency\n"
                                "allways on disp\n"
                                "Sleep on time\n"; //Просыпаться по UART
    uint8_t current_selection = u8g2.userInterfaceSelectionList(
                                  "TEST_menu",
                                  current_selection,
                                  settings_list);
    if ( current_selection == 1 ) {

    }
    else if ( current_selection == 2 ) {
      uint8_t sure = u8g2.userInterfaceMessage(
                       "if AOD is off",
                       "after 60s sleep:",
                       onOff(allways_on_disp),
                       " ok \n cancel");
      if (sure == 1) {
        allways_on_disp = !allways_on_disp;
      }
    }
    else if ( current_selection == 3 ) {

    }
  }
