void test_send() {
  if (millis() % 5000 < 30) {
    //display.clearDisplay();
    //display.setCursor(0,0);
    if (millis() / 5000 % 7 == 1) {
      display.clearDisplay();
      drawHeadLine();
      display.setCursor(0, 9);
    }
    //uint32_t Vbatt = 0;//powerinfo
    //for(int i = 0; i < 16; i++) {
    //Vbatt = Vbatt + analogReadMilliVolts(A0); // ADC with correction
    //}
    //float Vbattf = 2 * analogReadMilliVolts(A0) / 1000.0;
    display.print("Test " + String(millis() / 5000, DEC));





    e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "\nTest " + String(millis() / 5000, DEC) +  \
                             "  " + String(getIncludeTemperature()) + "C");

    //int analogValue = analogRead(GPIO_NUM_10);

    // print out the values you read:
    //Serial.printf("ADC analog value = %d\n", analogValue);

    float tVBat = realVBat();
    Serial.println(tVBat);
    display.print(tVBat);
    display.println("V");
    display.display();
  }
}

void printParameters(struct Configuration configuration) {
  Serial.println("----------------------------------------");

  Serial.print(F("HEAD : "));  Serial.print(configuration.COMMAND, HEX); Serial.print(" "); Serial.print(configuration.STARTING_ADDRESS, HEX); Serial.print(" "); Serial.println(configuration.LENGHT, HEX);
  Serial.println(F(" "));
  Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, HEX);
  Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, HEX);
  Serial.println(F(" "));
  Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
  Serial.println(F(" "));
  Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN); Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
  Serial.print(F("SpeedUARTDatte     : "));  Serial.print(configuration.SPED.uartBaudRate, BIN); Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRateDescription());
  Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN); Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRateDescription());
  Serial.println(F(" "));
  Serial.print(F("OptionSubPacketSett: "));  Serial.print(configuration.OPTION.subPacketSetting, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getSubPacketSetting());
  Serial.print(F("OptionTranPower    : "));  Serial.print(configuration.OPTION.transmissionPower, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());
  Serial.print(F("OptionRSSIAmbientNo: "));  Serial.print(configuration.OPTION.RSSIAmbientNoise, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getRSSIAmbientNoiseEnable());
  Serial.println(F(" "));
  Serial.print(F("TransModeWORPeriod : "));  Serial.print(configuration.TRANSMISSION_MODE.WORPeriod, BIN); Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getWORPeriodByParamsDescription());
  Serial.print(F("TransModeEnableLBT : "));  Serial.print(configuration.TRANSMISSION_MODE.enableLBT, BIN); Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getLBTEnableByteDescription());
  Serial.print(F("TransModeEnableRSSI: "));  Serial.print(configuration.TRANSMISSION_MODE.enableRSSI, BIN); Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getRSSIEnableByteDescription());
  Serial.print(F("TransModeFixedTrans: "));  Serial.print(configuration.TRANSMISSION_MODE.fixedTransmission, BIN); Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getFixedTransmissionDescription());

  Serial.println("----------------------------------------");

}

void symbolTest(){
  display.clearDisplay();
  for(byte c=0; c<126; c++){
      int y = (c*7/128)*8;
      int x = 0 + c*7 - 16*y ;
      
      Serial.println(String(x) + ":" + String(y));
      
      display.drawChar( x ,y, c, SSD1306_WHITE, SSD1306_BLACK,1);
      display.display();
      delay(5);
  }
}
