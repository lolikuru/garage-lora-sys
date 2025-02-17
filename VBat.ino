float realVBat() {//only 100/100komh GPIO_NUM_10
  uint32_t Vbatt = 0;
  for (int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(GPIO_NUM_10); // ADC with correction
  }
  float Vbattf = Vbatt / 16 ;
  float realVbat = (Vbattf * 3.59 + 480) / 1000;
  return realVbat;
}

void printVBat(bool procent) {
  int x = 100;
  int y = 0;
  display.setTextSize(1);
  display.setCursor(x, y);
  display.setTextColor(SSD1306_BLACK);
  for (byte t = 0; t < 4; t++) display.write(0xDA);
  display.setCursor(x, y);
  display.setTextColor(SSD1306_WHITE);
  if (procent == false) {
    display.print(realVBat());
    //display.print("V");
  } else {
    int mP = map(realVBat() * 100, 320, 418, 0, 100);
    if ( mP >= 0 ) {
      display.print(mP);
      display.print("%");
    } else if (mP > 100) {
      display.print("100%");
    }
    else display.print("ERR");
  }
  drawBatImage(x - 6, y);
  //display.display();
}
