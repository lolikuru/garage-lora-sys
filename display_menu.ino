void log_menu() { //cursor string
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_t_symbols);
  const char *debug_list =
    "Send test temp\n"
    "Delete json\n"
    "Print json config\n"
    "Print LoRa CFG\n"
    "LED MSG\n"
    "Wifi log AP\n"
    "Log size view\n"
    "Print log\n"
    "Delete log\n"
    "FS List\n"
    "Back to main\n"
    "EXIT";

  static uint8_t debug_selection = 1;
  uint8_t current_selection = u8g2.userInterfaceSelectionList(
                                "TEST_menu",
                                debug_selection,
                                debug_list);
  if (current_selection > 0) {
    debug_selection = current_selection;
  }
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
    deleteFile(LittleFS, config_filename);
    current_selection = 0;
  }

  else if ( current_selection == 3 ) {
    readFile(LittleFS, config_filename);
    current_selection = 0;
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
   uint8_t sure = u8g2.userInterfaceMessage(
      "LED MSG change",
      "now: ",
      onOff(led_msg),
      " Change \n Cancel ");
    if (sure == 1) {
      led_msg = !led_msg;
      saveConfig();
    }
  }

  else if ( current_selection == 6 ) {
    uint8_t choice = u8g2.userInterfaceMessage(
                       "Wifi log AP",
                       "now:",
                       onOff(Wifi_boot),
                       " ok \n cancel \n Retry ");
    if (choice == 1) {
      Wifi_boot = !Wifi_boot;
      saveConfig();
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
      saveConfig();
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
    //String t = String(u8x8_GetStringLineStart(power_item - 1, power_list));
    //Serial.println(t);
    if (sure == 1) {
      deep_sleep();
    }
  }
}

void settings_menu() {
  const char *settings_list = "CPU Frequency\n"
                              "Allways on disp\n"
                              "Sleep on time\n" //Просыпаться по UART
                              "Procent battery";
  static uint8_t settings_selection = 1;
  uint8_t current_selection = u8g2.userInterfaceSelectionList(
                                "Settings",
                                settings_selection,
                                settings_list);
  if (current_selection > 0) {
    settings_selection = current_selection;
  }
  if ( current_selection == 1 ) {
    const char *cpu_list = "80 MHz\n160 MHz\n240 MHz";
    uint8_t cpu_sel = u8g2.userInterfaceSelectionList("CPU Frequency", 1, cpu_list);
    if (cpu_sel == 1) setCpuFrequencyMhz(80);
    else if (cpu_sel == 2) setCpuFrequencyMhz(160);
    else if (cpu_sel == 3) setCpuFrequencyMhz(240);
  }
  else if ( current_selection == 2 ) {
    uint8_t sure = u8g2.userInterfaceMessage(
                     "if AOD is off",
                     "after n s sleep:",
                     onOff(allways_on_disp),
                     " ok \n cancel");
    if (sure == 1) {
      allways_on_disp = !allways_on_disp;
      saveConfig();
    }
  }
  else if ( current_selection == 3 ) {
    u8g2.userInterfaceInputValue("Seconds before sleep", "delay= ", &off_display_sec, 30, 180, 3, " sec");
    saveConfig();
  }
  else if ( current_selection == 4 ) {
    uint8_t sure = u8g2.userInterfaceMessage(
                     "voltage - off",
                     "procent - on",
                     onOff(procent_battery),
                     " ok \n cancel");
    if (sure == 1) {
      procent_battery = !procent_battery;
      saveConfig();
    }
  }
}
