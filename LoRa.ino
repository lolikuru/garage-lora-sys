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
      // Prefix like "2.." from a numbered peer; comma-operator rc.data[0, 1] was a bug
      if (rc.data.length() >= 3 && rc.data.charAt(0) == '2') {
        rc.data.remove(0, 3);
      }
      if (rc.data.substring(0, 1) == "I") {
        rc.data.remove(0, 1);
        Serial.println(rc.status.getResponseDescription());
        if (led_msg) digitalWrite(LED_PIN, HIGH);

        time_substring = rc.data.substring(0, rc.data.indexOf("/"));
        r_info.msgtime = strtol(time_substring.c_str(), NULL, 10);
        rc.data.remove(0, rc.data.indexOf("/") + 1);

        float new_temp = rc.data.substring(0, rc.data.indexOf("/")).toFloat();
        rc.data.remove(0, rc.data.indexOf("/") + 1);
        float new_humid = rc.data.substring(0, rc.data.indexOf("/")).toFloat();
        rc.data.remove(0, rc.data.indexOf("/") + 1);

        r_info.save = (new_temp != host_temp) || (new_humid != host_humid);
        host_temp = new_temp;
        host_humid = new_humid;
        r_info.temp = host_temp;
        r_info.humid = host_humid;
        r_info.rssi = lastRssi;

        rtc.setTime(strtol(time_substring.c_str(), NULL, 10));

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

void IRAM_ATTR wakeUp() {
  interruptExecuted = true;
}

//void testTimeMessage() {
//  Serial.println("Send_RTC_Messsage");
//  RtcDateTime now = Rtc.GetDateTime();
//  e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "/TIME" + printDateTime(now));
//  Serial.println(printDateTime(now));
//}
