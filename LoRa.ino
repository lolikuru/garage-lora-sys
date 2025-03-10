void UpdateLoraInfoStruct() {
  
  if (e220ttl.available() > 1) {
    if (led_msg)digitalWrite(LED_PIN, HIGH);
    //String old_time_substring;
    //u8g2.clearBuffer();
#ifdef ENABLE_RSSI
    ResponseContainer rc = e220ttl.receiveMessageRSSI();
    lastRssi = rc.rssi; //последний rssi
#else
    ResponseContainer rc = e220ttl.receiveMessage();
#endif

    if (rc.status.code != 1) {
      Serial.println(rc.status.getResponseDescription());
    } else {
      if (rc.data[0, 1] == 2) {
        rc.data.remove(0, 3);
      }
      if (rc.data.substring(0, 1) == "I") {
        rc.data.remove(0, 1);
        Serial.println(rc.status.getResponseDescription());
        if (led_msg) digitalWrite(LED_PIN, HIGH);

        time_substring = rc.data.substring(0, rc.data.indexOf("/"));
        r_info.msgtime = strtol(time_substring.c_str(), NULL, 10);
        rc.data.remove(0, rc.data.indexOf("/") + 1);
        //Serial.println(time_substring);

        r_info.save = true;

        if(host_temp == rc.data.substring(0, rc.data.indexOf("/")).toFloat()) r_info.save = false;
        else host_temp = rc.data.substring(0, rc.data.indexOf("/")).toFloat();
        r_info.temp = host_temp;
        rc.data.remove(0, rc.data.indexOf("/") + 1);

        if (host_humid == rc.data.substring(0, rc.data.indexOf("/")).toFloat()) r_info.save = false;
        else host_humid = rc.data.substring(0, rc.data.indexOf("/")).toFloat();
        r_info.humid = host_humid;
        rc.data.remove(0, rc.data.indexOf("/") + 1);

        r_info.rssi = lastRssi;

        rtc.setTime(strtol(time_substring.c_str(), NULL, 10) - 3600 * time_zone);

        //        Serial.println("left: " + rc.data);
        //        Serial.println("host_temp: " + String(host_temp));
        //        Serial.println("host_humid: " + String(host_humid));
        //Serial.printf("%u %2.1f %2.1f %i\n", r_info.msgtime, r_info.temp, r_info.humid, r_info.rssi);

      } else {
        r_info.save = false;
      }
      if (r_info.save) {
        String DHT_substring = String(host_temp).substring(0, 4) + "ºC " + String(host_humid).substring(0, 4) + "%";
        String log_str = rtc.getTime("%d/%b/%Y %H:%M:%S") + "|" + DHT_substring + "|" + lastRssi + "|" + String(realVBat()) + "\n";
        Serial.println(log_str);
        appendFile(LittleFS, "/log.txt", log_str.c_str());
        r_info.save = false;
      }
    }
  }
  if(led_msg)digitalWrite(LED_PIN, LOW);
}

void wakeUp() {
  interruptExecuted = true;
  //detachInterrupt(digitalPinToInterrupt(AUX_PIN));
}

//void testTimeMessage() {
//  Serial.println("Send_RTC_Messsage");
//  RtcDateTime now = Rtc.GetDateTime();
//  e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "/TIME" + printDateTime(now));
//  Serial.println(printDateTime(now));
//}
