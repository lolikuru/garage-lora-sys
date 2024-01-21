/*
   EBYTE LoRa E220
   Send a string message to a fixed point ADDH ADDL CHAN

   You must configure 2 device: one as SENDER (with FIXED SENDER config) and uncomment the relative
   define with the correct DESTINATION_ADDL, and one as RECEIVER (with FIXED RECEIVER config)
   and uncomment the relative define with the correct DESTINATION_ADDL.

   Write a string on serial monitor or reset to resend default value.

   Pai attention e220 support RSSI, if you want use that functionality you must enable RSSI on configuration
   configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED;

   and uncomment #define ENABLE_RSSI true in this sketch

   You must uncommend the correct constructor.

   by Renzo Mischianti <https://www.mischianti.org>

   https://www.mischianti.org

   E220		  ----- WeMos D1 mini	----- esp32			----- Arduino Nano 33 IoT	----- Arduino MKR	----- Raspberry Pi Pico   ----- stm32               ----- ArduinoUNO
   M0         ----- D7 (or GND) 	----- 19 (or GND) 	----- 4 (or GND) 			----- 2 (or GND) 	----- 10 (or GND)	      ----- PB0 (or GND)        ----- 7 Volt div (or GND)
   M1         ----- D6 (or GND) 	----- 21 (or GND) 	----- 6 (or GND) 			----- 4 (or GND) 	----- 11 (or GND)	      ----- PB10 (or GND)       ----- 6 Volt div (or GND)
   TX         ----- D3 (PullUP)		----- TX2 (PullUP)	----- TX1 (PullUP)			----- 14 (PullUP)	----- 8 (PullUP)	      ----- PA2 TX2 (PullUP)    ----- 4 (PullUP)
   RX         ----- D4 (PullUP)		----- RX2 (PullUP)	----- RX1 (PullUP)			----- 13 (PullUP)	----- 9 (PullUP)	      ----- PA3 RX2 (PullUP)    ----- 5 Volt div (PullUP)
   AUX        ----- D5 (PullUP)		----- 18  (PullUP)	----- 2  (PullUP)			----- 0  (PullUP)	----- 2  (PullUP)	      ----- PA0  (PullUP)       ----- 3 (PullUP)
   VCC        ----- 3.3v/5v			----- 3.3v/5v		----- 3.3v/5v				----- 3.3v/5v		----- 3.3v/5v		      ----- 3.3v/5v             ----- 3.3v/5v
   GND        ----- GND				----- GND			----- GND					----- GND			----- GND			      ----- GND                 ----- GND

*/

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */


// With FIXED SENDER configuration
// #define DESTINATION_ADDL 3

// With FIXED RECEIVER configuration
#define DESTINATION_ADDL 2



// If you want use RSSI uncomment //#define ENABLE_RSSI true
// and use relative configuration with RSSI enabled
#define ENABLE_RSSI true

#include "Arduino.h"
#include "LoRa_E220.h"
#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <HardwareSerial.h>

#include "driver/temp_sensor.h"

void initTempSensor() {
  temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
  temp_sensor.dac_offset = TSENS_DAC_L2;  // TSENS_DAC_L2 is default; L4(-40°C ~ 20°C), L2(-10°C ~ 80°C), L1(20°C ~ 100°C), L0(50°C ~ 125°C)
  temp_sensor_set_config(temp_sensor);
  temp_sensor_start();
}


RTC_DATA_ATTR int bootCount = 0;

/*
  Method to print the reason by which ESP32
  has been awaken from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

//void setup() {
//  Serial.begin(115200);
//  initTempSensor();
//}
//
//void loop() {
//  Serial.print("Temperature: ");
//  float result = 0;
//  temp_sensor_read_celsius(&result);
//  Serial.print(result);
//  Serial.println(" °C");
//  delay(5000);
//}

HardwareSerial MySerial(1);

#define WIRE Wire

#define BUTTON_UP 1
#define BUTTON_OK 2
#define BUTTON_DOWN 4
#define BUTTON_BACK 6

#define LED_PIN  15

bool button[] = {0, 0, 0, 0};
bool stateButton[] = {0, 0, 0, 0};



//LiquidCrystal_PCF8574 lcd(0x27);
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &WIRE);

int rssi;


// 2 custom characters

byte dotOff[] = { 0b00000, 0b01110, 0b10001, 0b10001,
                  0b10001, 0b01110, 0b00000, 0b00000
                };
byte dotOn[] = { 0b00000, 0b01110, 0b11111, 0b11111,
                 0b11111, 0b01110, 0b00000, 0b00000
               };




// ---------- esp8266 pins --------------
//LoRa_E220 e220ttl(RX, TX, AUX, M0, M1);  // Arduino RX <-- e220 TX, Arduino TX --> e220 RX
//LoRa_E220 e220ttl(2, 16, 14, 13, 12); // Arduino RX <-- e220 TX, Arduino TX --> e220 RX AUX M0 M1
//LoRa_E220 e220ttl(2, 16); // Config without connect AUX and M0 M1

//SoftwareSerial mySerial(D2, D3); // Arduino RX <-- e220 TX, Arduino TX --> e220 RX
//LoRa_E220 e220ttl(&mySerial, D5, D7, D6); // AUX M0 M1
// -------------------------------------

// ---------- Arduino pins --------------
//LoRa_E220 e220ttl(4, 5, 3, 7, 6); // Arduino RX <-- e220 TX, Arduino TX --> e220 RX AUX M0 M1
//LoRa_E220 e220ttl(4, 5); // Config without connect AUX and M0 M1

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(4, 5); // Arduino RX <-- e220 TX, Arduino TX --> e220 RX
//LoRa_E220 e220ttl(&mySerial, 3, 7, 6); // AUX M0 M1
// -------------------------------------

// ------------- Arduino Nano 33 IoT -------------
// LoRa_E220 e220ttl(&Serial1, 2, 4, 6); //  RX AUX M0 M1
// -------------------------------------------------

// ------------- Arduino MKR WiFi 1010 -------------
// LoRa_E220 e220ttl(&Serial1, 0, 2, 4); //  RX AUX M0 M1
// -------------------------------------------------

// ---------- esp32 pins --------------
// LoRa_E220 e220ttl(&Serial2, 15, 21, 19); //  RX AUX M0 M1

//LoRa_E220 e220ttl(&Serial2, 1, 2, 4, 6, 8, UART_BPS_RATE_9600); //  esp32 RX <-- e220 TX, esp32 TX --> e220 RX AUX M0 M1
LoRa_E220 e220ttl(37, 39, &MySerial, 11, 3, 5, UART_BPS_RATE_9600, SERIAL_8N1);
// -------------------------------------

// ---------- Raspberry PI Pico pins --------------
// LoRa_E220 e220ttl(&Serial2, 2, 10, 11); //  RX AUX M0 M1
// -------------------------------------

// ---------------- STM32 --------------------
//HardwareSerial Serial2(USART2);   // PA3  (RX)  PA2  (TX)
//LoRa_E220 e220ttl(&Serial2, PA0, PB0, PB10); //  RX AUX M0 M1
// -------------------------------------------------
void printParameters(struct Configuration configuration);

void setup() {
  int error;

  Serial.begin(9600);
  delay(500);
  initTempSensor();

  // Startup all pins and UART
  e220ttl.begin();

  //Wire.begin();
  //Wire.beginTransmission(0x27);
  //error = Wire.endTransmission();
  //if (error == 0) {
  //Serial.println(": LCD found.");
  //lcd.begin(20, 4);  // initialize the lcd
  //lcd.createChar(1, dotOff);
  //lcd.createChar(2, dotOn);
  //} else {
  //  Serial.println(": LCD not found.");
  //}  // if

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  ResponseStructContainer c;
  c = e220ttl.getConfiguration();
  // It's important get configuration pointer before all other operation
  Configuration configuration = *(Configuration*) c.data;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);

  printParameters(configuration);
  c.close();

  Serial.println("Hi, I'm going to send message!");

  // Send message
  ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "Hello, world?");
  // Check If there is some problem of succesfully send
  Serial.println(rs.getResponseDescription());
  display.display();


  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  testdrawline();
  display.clearDisplay();
  display.print("display connected!");
  display.display();
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //setCpuFrequencyMhz(80);

  //bottons
  pinMode(BUTTON_UP, INPUT);
  pinMode(BUTTON_OK, INPUT);
  pinMode(BUTTON_DOWN, INPUT);
  pinMode(BUTTON_BACK, INPUT);

  pinMode(GPIO_NUM_10, INPUT);

  analogReadResolution(12);

  pinMode(LED_PIN, OUTPUT);

}

void loop() {
  buttonTest();


  // If something available
  //lcd.setBacklight(0);
  if (e220ttl.available() > 1) {
    //lcd.setBacklight(255);
    //lcd.clear();
    // read the String message
#ifdef ENABLE_RSSI
    ResponseContainer rc = e220ttl.receiveMessageRSSI();
#else
    ResponseContainer rc = e220ttl.receiveMessage();
#endif
    // Is something goes wrong print error
    if (rc.status.code != 1) {
      Serial.println(rc.status.getResponseDescription());
    } else {
      display.clearDisplay();
      // Print the data received
      Serial.println(rc.status.getResponseDescription());
      Serial.println(rc.data);
      //String MyStr = MyStr.remove(0, myindex);
      display.setCursor(0, 0);
      display.println(rc.status.getResponseDescription());
      //display.setCursor(0, 3);
      display.println(rc.data);

      //rssi = rc.rssi;
#ifdef ENABLE_RSSI
      Serial.print("RSSI: "); Serial.println(rc.rssi, DEC);
      //display.setCursor(0, 4);
      display.print("RSSI: "); display.print(rc.rssi, DEC);
      display.display();
      delay(100);
      display.clearDisplay();
      delay(100);
      //e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23,  "OK!");
#endif
    }
  }

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
    float result = 0;
    temp_sensor_read_celsius(&result);
    display.print(" " + String(result) + "C ");




    e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "\nTest " + String(millis() / 5000, DEC) +  \
                             "  " + String(result) + "C");

    //int analogValue = analogRead(GPIO_NUM_10);

    // print out the values you read:
    //Serial.printf("ADC analog value = %d\n", analogValue);

    float tVBat = realVBat();
    Serial.println(tVBat);
    display.print(tVBat);
    display.println("V");
    display.display();


  }

  if (Serial.available()) {

    String input = Serial.readString();
    ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, input);
    Serial.println(rs.getResponseDescription());
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


float realVBat() {//only 100/100komh GPIO_NUM_10
  uint32_t Vbatt = 0;
  for (int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(GPIO_NUM_10); // ADC with correction
  }
  float Vbattf = Vbatt / 16 ;
  float realVbat = (Vbattf * 3.59 + 480) / 1000;
  return realVbat;
}

void buttonTest(){
  button[0] = !digitalRead(BUTTON_UP);
  button[1] = !digitalRead(BUTTON_OK);
  button[2] = !digitalRead(BUTTON_DOWN);
  button[3] = !digitalRead(BUTTON_BACK);


  for (byte i = 0; i < 4; i++) {
    if (stateButton[i] != button[i]) {
      stateButton[i] = button[i];
      if (button[i] == 1) {
        Serial.printf("Botton %d\n" , i);
        drawCircle(i);
      }
    }
  }
}
