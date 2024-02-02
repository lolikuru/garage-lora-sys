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
  ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "NEW CONNECTION");
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
  delay(1000);
  //symbolTest();
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
  e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "1DHT1");

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
      if (rc.data[0, 1] ==  2) {
        rc.data.remove(0, 4);
      }
      //Serial.println(rc.data.substring(0,3));
      if(rc.data.substring(0,3) == "DHT"){
        display.clearDisplay();
        display.setTextSize(2); 
        // Print the data received
        Serial.println(rc.status.getResponseDescription());
        Serial.println(rc.data);
        //drawHeadLine();
        //String MyStr = MyStr.remove(0, myindex);
        display.setCursor(0, 11);
        //display.println(rc.status.getResponseDescription());
        //display.setCursor(0, 3);
        //display.println(rc.data);
        display.print(rc.data.substring(3 , rc.data.indexOf("/")-1));
        display.write(0xF7);display.print("c");
        String Humid = rc.data.substring(rc.data.indexOf("/") + 1, rc.data.indexOf("/") + 3);
        display.println(" " + Humid + "%");
        printVBat(false);
        
      }
      

      //rssi = rc.rssi;
#ifdef ENABLE_RSSI
      display.setTextSize(1);
      Serial.print("RSSI: "); Serial.println(rc.rssi, DEC);
      display.setCursor(0, 0);
      display.write(0x1F); 
      display.setCursor(8, 0);
      display.print(rc.rssi, DEC);
      display.display();
      delay(100);
      //display.clearDisplay();
      //display.setCursor(0, 36);
      delay(100);
#endif
    }
  }

  //test_send();

  if (Serial.available()) {

    String input = Serial.readString();
    ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, input);
    Serial.println(rs.getResponseDescription());
  }

  if (millis() % 2000 < 30) {
    printVBat(false);
  }
}

void buttonTest() {
  button[0] = !digitalRead(BUTTON_UP);
  button[1] = !digitalRead(BUTTON_OK);
  button[2] = !digitalRead(BUTTON_DOWN);
  button[3] = !digitalRead(BUTTON_BACK);


  for (byte i = 0; i < 4; i++) {
    if (stateButton[i] != button[i]) {
      stateButton[i] = button[i];
      if (button[i] == 1) {
        Serial.printf("Botton %d\n" , i);
        drawCircles(i,1);
        if ( i == 1 ) {
          e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "1DHT1");
        }
        
      }else drawCircles(i,0);
    }
  }
}

float getIncludeTemperature(){
    float result = 0;
    temp_sensor_read_celsius(&result);
    display.print(" " + String(result) + "C ");
    return result;
}
