void test_send() {
  if (millis() % 5000 < 30) {
    //display.clearDisplay();
    //display.setCursor(0,0);
    if (millis() / 5000 % 7 == 1) {
      u8g2.clearDisplay();
      //drawHeadLine();
      u8g2.setCursor(0, 9);
    }
    //uint32_t Vbatt = 0;//powerinfo
    //for(int i = 0; i < 16; i++) {
    //Vbatt = Vbatt + analogReadMilliVolts(A0); // ADC with correction
    //}
    //float Vbattf = 2 * analogReadMilliVolts(A0) / 1000.0;
    u8g2.print("Test " + String(millis() / 5000, DEC));





    e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "\nTest " + String(millis() / 5000, DEC) +  \
                             "  " + String(getIncludeTemperature()) + "C");

    //int analogValue = analogRead(GPIO_NUM_10);

    // print out the values you read:
    //Serial.printf("ADC analog value = %d\n", analogValue);

    float tVBat = realVBat();
    Serial.println(tVBat);
    u8g2.print(tVBat);
    u8g2.println("V");
    u8g2.display();
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

void symbolTest() {
  u8g2.clearBuffer();
  for (byte c = 0; c < 126; c++) {
    int y = (c * 7 / 128) * 8;
    int x = 0 + c * 7 - 16 * y ;

    //Serial.println(String(x) + ":" + String(y));
    u8g2.drawLine ( x , y, c, 0);
    //display.drawChar( x ,y, c, SSD1306_WHITE, SSD1306_BLACK,1);
    u8g2.sendBuffer();
    delay(5);
  }
}

void set_lora_configuration(struct Configuration configuration) {
  ResponseStatus rs = e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  Serial.println(rs.getResponseDescription());
  Serial.println(rs.code);

  ResponseStructContainer c;
  c = e220ttl.getConfiguration();
  // It's important get configuration pointer before all other operation
  configuration = *(Configuration*) c.data;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);

  printParameters(configuration);
  c.close();
}

void testDhtMessage() {
  float result = 0;
  temp_sensor_read_celsius(&result);
  float Humidity = 0; // Получаем показания влажности
  //ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, &message, sizeof(Message));
  Serial.println("Send_testMesssage");
  if(Wifi_boot){
    //epochTime = timeClient.getEpochTime();
    //tm *ptm = gmtime((time_t *)&epochTime);
    e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "/TIME" + String(epochTime));
  } else e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "/TIME" + String(millis()/1000));
  delay(150);
  e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "/DHT" + String(result) + "/0.00");
  delay(150);
}
