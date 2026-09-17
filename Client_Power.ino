static void prepareSleepWakeupPins() {
  rtc_gpio_pullup_en(GPIO_NUM_2);
  rtc_gpio_pulldown_dis(GPIO_NUM_2);
  rtc_gpio_pullup_en(GPIO_NUM_11);
  rtc_gpio_pulldown_dis(GPIO_NUM_11);
}

void deep_sleep() {
  prepareSleepWakeupPins();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 0);
  u8g2.setPowerSave(1);
  e220ttl.setMode(MODE_2_POWER_SAVING);
  Serial.flush();
  esp_deep_sleep_start();
}

void light_sleep(bool on_display) {
  prepareSleepWakeupPins();
  // ESP32 Arduino 2.0.9 has no ESP_EXT1_WAKEUP_ANY_LOW. For light sleep,
  // GPIO wakeup fires when ANY of these pins goes LOW (OK button or LoRa AUX).
  gpio_wakeup_enable(GPIO_NUM_2, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable(GPIO_NUM_11, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  u8g2.setPowerSave(1);
  Serial.flush();
  esp_light_sleep_start();

  gpio_wakeup_disable(GPIO_NUM_2);
  gpio_wakeup_disable(GPIO_NUM_11);

  sleep_timestump = millis();
  e220ttl.setMode(MODE_0_NORMAL);

  bool wake_display = on_display;
  if (digitalRead(GPIO_NUM_11) == LOW) {
    interruptExecuted = true;
  }
  if (digitalRead(GPIO_NUM_2) == LOW) {
    wake_display = true;
  }
  if (wake_display) {
    display_on = true;
    u8g2.setPowerSave(0);
  }
}
