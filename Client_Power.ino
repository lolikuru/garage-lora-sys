void deep_sleep() {
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, LOW);
  u8g2.setPowerSave(1); //
  e220ttl.setMode(MODE_2_POWER_SAVING); //lora off
  esp_deep_sleep_start();
  //sleep
  e220ttl.setMode(MODE_0_NORMAL);
  u8g2.setPowerSave(0);
}

void light_sleep() {
  uint64_t wakeup_pin_mask = (1ULL << GPIO_NUM_2) | (1ULL << GPIO_NUM_11);
  esp_sleep_enable_ext1_wakeup(wakeup_pin_mask, ESP_EXT1_WAKEUP_ALL_LOW);
  u8g2.setPowerSave(1);
  e220ttl.setMode(MODE_2_WOR_RECEIVER);
  esp_light_sleep_start();
  //sleep
  e220ttl.setMode(MODE_0_NORMAL);
  u8g2.setPowerSave(0);
}
