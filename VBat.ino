float realVBat() {//only 100/100komh GPIO_NUM_10
  uint32_t Vbatt = 0;
  for (int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(GPIO_NUM_10); // ADC with correction
  }
  float Vbattf = Vbatt / 16 ;
  float realVbat = (Vbattf * 3.59 + 480) / 1000;
  return realVbat;
}

void printVBat(bool procent){
      display.setTextSize(1);
      display.setCursor(60, 0);
      display.setTextColor(SSD1306_BLACK);
      for (byte t=0; t<5; t++) display.write(0xDA);
      display.setCursor(60, 0);
      display.setTextColor(SSD1306_WHITE);
      if (procent == false){
        display.print(realVBat());
        display.print("V");
      } else {
          int mP = map(realVBat(), 3.20, 4.18, 0, 100);
          display.print(mP);
          display.print("V");
      }
      display.display();
}
