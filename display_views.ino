const char * onOff(bool input) {
  const char* msg = "OFF";
  if (input) {
    msg = "ON";
  }
  return msg;
}

void main_view() {

  //timeClient.update();
  //  unsigned long epochTime = timeClient.getEpochTime();
  //  struct tm *ptm = gmtime((time_t *)&epochTime);//time update

  u8g2.clearBuffer();
  //bool save = get_lora_main_info();
  printVBat(procent_battery);
  u8g2.setCursor(0, 24);
  u8g2.print(rtc.getTime("%d/%b/%Y %H:%M:%S"));
  u8g2.setFont(u8g2_font_10x20_tf );
  u8g2.setCursor(0, 42);
  u8g2.print(String(host_temp).substring(0, 4) + "ºC " + String(host_humid).substring(0, 4) + "%");
  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.print(" ");

  if (print_logf_status) {
    u8g2.setFont(u8g2_font_6x12_t_symbols);
    u8g2.setCursor(32, 62);
    u8g2.print("log size:");
    u8g2.print(get_log_size());
    u8g2.print("K");
  }

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
    "Load Config\n"
    "7\n"
    "Settings\n"
    "TEST_menu\n"
    "EXIT";


  static uint8_t current_main_selection = 1;
  uint8_t sel = u8g2.userInterfaceSelectionList(
                                     "Main menu",
                                     current_main_selection,
                                     main_list);
  if (sel == 0) {
    return;
  }
  current_main_selection = sel;
  if ( current_main_selection == 1 ) {// ON/OFF 8ch switch
    String switch_list;
    for (uint8_t i = 0; i < 8; i++) {
      if (i) switch_list += "\n";
      switch_list += String(Pinout[i] ? "[*] " : "[ ] ") + Pinout_name[i];
    }
    uint8_t sw = u8g2.userInterfaceSelectionList("8ch switch", 1, switch_list.c_str());
    if (sw >= 1 && sw <= 8) {
      Pinout[sw - 1] = !Pinout[sw - 1];
      saveConfig();
      sendLoraCommand(String("L") + String(sw - 1) + (Pinout[sw - 1] ? "1" : "0"));
    }
  }
  else if ( current_main_selection == 2 ) {//Hub settings
    const char *hub_settings_list =
      "Backlight\n"
      "Send info timeout\n"
      "Wifi settings";

  }
  else if ( current_main_selection == 3) {
    

  }
  else if ( current_main_selection == 4 ) {

  }
  else if ( current_main_selection == 5 ) {

  }
  else if ( current_main_selection == 6 ) {//Load Config
    Serial.println("LOAD CFG");
    loadConfig();
    readFile(LittleFS, config_filename);
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
    current_main_selection = 1;
    u8g2.clearBuffer();
  }
}
