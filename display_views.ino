void main_view(){
  if (millis() % 2000 < 30) {
    printVBat(true);
    ResiveSymbol(false);
  }
  if (e220ttl.available() > 1) {
#ifdef ENABLE_RSSI
    ResponseContainer rc = e220ttl.receiveMessageRSSI();
#else
    ResponseContainer rc = e220ttl.receiveMessage();
#endif
    // Is something goes wrong print error
    if (rc.status.code != 1) {
      Serial.println(rc.status.getResponseDescription());
    } else {
      if (rc.data[0, 1] ==  2) {
        rc.data.remove(0, 4);
      }
      //Serial.println(rc.data.substring(0,3));
      
      lastRssi = rc.rssi; //последний rssi
      if (rc.data.substring(0, 4) == "TIME") {//получаем время
        Serial.println(rc.data.substring(4));
        display.setTextSize(1);
        display.setCursor(0, 12);
        display.print(rc.data.substring(4));

      }
      if (rc.data.substring(0, 3) == "DHT") {
        display.clearDisplay();
        ResiveSymbol(true);
        display.setTextSize(2);
        // Print the data received
        Serial.println(rc.status.getResponseDescription());
        Serial.println(rc.data);
        drawHeadLine();
        display.setCursor(0, 24);
        display.print(rc.data.substring(3 , rc.data.indexOf("/") - 1));
        display.write(0xF7); display.print("c");
        String Humid = rc.data.substring(rc.data.indexOf("/") + 1, rc.data.indexOf("/") + 3);
        display.println(" " + Humid + "%");
        printVBat(true);
      }

      //rssi = rc.rssi;
#ifdef ENABLE_RSSI
      display.setTextSize(1);
      Serial.print("RSSI: "); Serial.println(lastRssi, DEC);
      display.setCursor(0, 0);
      display.write(0x1F);
      display.setCursor(8, 0);
      
      display.setTextColor(SSD1306_BLACK);
      for (byte t = 0; t < 3; t++) display.write(0xDA);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(8, 0);
      display.print(rc.rssi, DEC);
      display.display();
      //display.clearDisplay();
      //display.setCursor(0, 36);
#endif
    }
  }

  //test_send();
  }

void menu(byte c_str){ //cursor string
  display.clearDisplay();
  display.setTextSize(1);
  
  const int NUM_ITEMS = 8; // number of items in the list and also the number of screenshots and screenshots with QR codes (other screens)
  const int MAX_ITEM_LENGTH = 20; // maximum characters for the item name

  char menu_items [NUM_ITEMS] [MAX_ITEM_LENGTH] = {  // array with item names
    { "3D Cube" }, 
    { "Battery" }, 
    { "Dashboard" }, 
    { "Fireworks" }, 
    { "GPS Speed" }, 
    { "Big Knob" },   
    { "Park Sensor" }, 
    { "Turbo Gauge" }
  };
    for(byte i=0; i<4; i++) {
      display.setCursor(2, 16*i);
      display.print(menu_items[i+c_str]);
  }
  display.display();
}
