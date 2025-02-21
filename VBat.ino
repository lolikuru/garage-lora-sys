float realVBat() {//only 100/100komh GPIO_NUM_10
  uint32_t Vbatt = 0;
  for (int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(GPIO_NUM_10); // ADC with correction
  }
  float Vbattf = Vbatt / 16 ;
  //Serial.println(Vbattf);
  float realVbat = (Vbattf * 3.59 + 480) / 1000;
  return realVbat;
}

void printVBat(bool procent) {
  u8g2.setFont(u8g2_font_siji_t_6x10);
  u8g2.setCursor(96, 12);
  u8g2.setDrawColor(1);
  //for (byte t = 0; t < 4; t++) u8g2.print("\0xDA");
  //u8g2.setCursor(x, y);
  //u8g2.setDrawColor(0);
  if (procent == false) {
    u8g2.print(realVBat());
    u8g2.print("V");
  } else {
    int mP = map(realVBat() * 100, 320, 418, 0, 100);
    if ( mP >= 0 && mP < 100) {
      u8g2.print(mP);
      u8g2.print("%");
    } else if (mP > 100) {
      u8g2.print("100%");
    }
    else {
      u8g2.drawGlyph(110, 12, 0xe242);
      u8g2.drawGlyph(120, 12, 0xe0b3);
    }
    
  }
  //drawBatImage(x - 6, y);
  //display.display();
}
