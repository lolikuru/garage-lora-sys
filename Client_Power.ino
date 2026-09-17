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
  uint64_t wakeup_pin_mask = (1ULL << GPIO_NUM_2) | (1ULL << GPIO_NUM_11);
  prepareSleepWakeupPins();
  // ANY_LOW: wake on OK button or LoRa AUX. ALL_LOW required BOTH pins low at once.
  esp_sleep_enable_ext1_wakeup(wakeup_pin_mask, ESP_EXT1_WAKEUP_ANY_LOW);
  u8g2.setPowerSave(1);
  Serial.flush();
  esp_light_sleep_start();

  sleep_timestump = millis();
  e220ttl.setMode(MODE_0_NORMAL);

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  bool wake_display = on_display;
  if (cause == ESP_SLEEP_WAKEUP_EXT1) {
    uint64_t st = esp_sleep_get_ext1_wakeup_status();
    if (st & (1ULL << GPIO_NUM_11)) {
      interruptExecuted = true;
    }
    if (st & (1ULL << GPIO_NUM_2)) {
      wake_display = true;
    }
  }
  if (wake_display) {
    display_on = true;
    u8g2.setPowerSave(0);
  }
}
