
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

//ESP32 2.0.9

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30        /* Time ESP32 will go to sleep (in seconds) */

#define AUX_PIN 11

#define BUTTON_PIN_BITMASK 0x800 //2^11 in 16 bit


// With FIXED SENDER configuration
// #define DESTINATION_ADDL 3

#include "esp_sleep.h"
#include "driver/gpio.h"
#include <esp_clk.h>

// With FIXED RECEIVER configuration
#define DESTINATION_ADDL 2

// If you want use RSSI uncomment //#define ENABLE_RSSI true
// and use relative configuration with RSSI enabled
#define ENABLE_RSSI true

#include "Arduino.h"
#include "LoRa_E220.h"
#include <Arduino.h>
#include <Wire.h>

#include <U8g2lib.h>

#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>

#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <HardwareSerial.h>

#include "driver/temp_sensor.h"

#include <ESP32Time.h>

//#include <base64.h>
#include <Base64.h>

void initTempSensor() {
  temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
  temp_sensor.dac_offset = TSENS_DAC_L2;  // TSENS_DAC_L2 is default; L4(-40°C ~ 20°C), L2(-10°C ~ 80°C), L1(20°C ~ 100°C), L0(50°C ~ 125°C)
  temp_sensor_set_config(temp_sensor);
  temp_sensor_start();
}

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

HardwareSerial MySerial(1);

#define WIRE Wire

#define BUTTON_UP 1
#define BUTTON_OK 2
#define BUTTON_DOWN 4
#define BUTTON_BACK 6

#define LED_PIN  15

//#define FORMAT_SPIFFS_IF_FAILED true
#define FORMAT_LITTLEFS_IF_FAILED true

bool button[] = {0, 0, 0, 0};
bool stateButton[] = {0, 0, 0, 0};

int lastRssi = 0;

bool print_logf_status = true;
bool led_msg = true;
bool send_dht = false;

unsigned long icon_timestamp = 0;
unsigned long sleep_timestump = millis();

uint8_t lora_link = 3;
uint16_t lora_symb[4] = {0xe21a, 0xe219, 0xe218, 0xe217};

volatile bool interruptExecuted = false;


String time_substring;
String old_time_substring;

float host_temp = 0;
float host_humid = 0;

uint8_t current_selection = 0;

uint8_t off_display_sec = 30;

uint8_t relay[8] {B00000000};

RTC_DATA_ATTR int bootCount = 0;

//json test
struct Config {
  int id;
  bool wifi_boot;
  char hostname[64];
  int port;
};

struct Info {
  unsigned long msgtime;
  float temp;
  float humid;
  //float power;
  int rssi;
  byte xSwitch;
  bool save = false;
};

struct Info r_info;

const char* filename = "/config.txt";  // <- SD library uses 8.3 filenames
Config config;                         // <- global configuration object


// Определяем переменные wifi
String _ssid     = ""; // Для хранения SSID
String _password = ""; // Для хранения пароля сети
String _ssidAP = "ESP32LogServer";   // SSID AP точки доступа
String _passwordAP = "12345678"; // пароль точки доступа

// Create AsyncWebServer object on port 80
static AsyncWebServer server(80);

//AsyncWebServer server(80, LittleFS, "myServer");

unsigned int time_zone = 4;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600*time_zone, 60000);

ESP32Time rtc(3600*time_zone);

// Log file configuration
const char* LOG_FILE_PATH = "/log.txt";

bool Wifi_boot = false;
bool WIFI_AP_on = false;

unsigned long epochTime;

int old_rssi = 0;

//struct tm *ptm;


//LiquidCrystal_PCF8574 lcd(0x27);

//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SSD1306_128X64_ALT0_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // same as the NONAME variant, but may solve the "every 2nd line skipped" problem
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* reset=*/ 8);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 16, /* data=*/ 17, /* reset=*/ U8X8_PIN_NONE);   // ESP32 Thing, pure SW emulated I2C
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 16, /* data=*/ 17);   // ESP32 Thing, HW I2C with pin remapping

//Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &WIRE);

int rssi;

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

  Serial.begin(115200);
  delay(500);

  initTempSensor();
  Serial.println("Start 1 TempSensor");

  // Startup all pins and UART
  e220ttl.begin();
  Serial.println("Start 2 LoRa module");


  u8g2.begin(/* menu_select_pin= */ 2, /* menu_next_pin= */ 4, /* menu_prev_pin= */ 1, /* menu_home_pin= */ 6);
  u8g2.enableUTF8Print();
  Serial.println("Start 3-SSD1306 Display and botton module");


  if (bootCount != 1) {
    ResponseStructContainer c;
    c = e220ttl.getConfiguration();
    // It's important get configuration pointer before all other operation
    Configuration configuration = *(Configuration*) c.data;
    Serial.println(c.status.getResponseDescription());
    Serial.println(c.status.code);

    printParameters(configuration);
    c.close();

    //display.display();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
      Serial.println("LittleFS Mount Failed");
      return;
    } else {
      u8g2.drawStr(0, 32, "LittleFS Mnted");
      listDir(LittleFS, "/", 0);
    }

    bootCount++;
    u8g2.setCursor(0, 16);
    u8g2.print(" Boot number: " + String(bootCount));
    u8g2.sendBuffer();
    delay(1000);
  }
  //symbolTest();
  //setCpuFrequencyMhz(80);

  //bottons
  pinMode(BUTTON_UP, INPUT);
  pinMode(BUTTON_OK, INPUT);
  pinMode(BUTTON_DOWN, INPUT);
  pinMode(BUTTON_BACK, INPUT);

  pinMode(GPIO_NUM_10, INPUT);

  analogReadResolution(12);

  pinMode(LED_PIN, OUTPUT);
  e220ttl.setMode(MODE_0_NORMAL);

  epochTime=millis()/1000;
  if(Wifi_boot){
    WIFIinit();
  }

}

void loop() {

  
  if(interruptExecuted) {
    Serial.println("WakeUp Callback, AUX pin go LOW and start receive message!");
    Serial.flush();
    //attachInterrupt(digitalPinToInterrupt(AUX_PIN), wakeUp, FALLING);
    interruptExecuted = false;
    e220ttl.setMode(MODE_0_NORMAL);
    u8g2.setPowerSave(0);
  }
  
  buttonsActive();
  main_view();

  if (send_dht) {
    if (millis() % 3000 < 200) {
      testDhtMessage();
    }
  }

  if (Serial.available()) {

    String input = Serial.readString();
    ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, input);
    Serial.println(rs.getResponseDescription());
  }

  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_11, 1);
  //esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  //esp_deep_sleep_start();
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //
  if (millis() > sleep_timestump + 30*1000){
    //sleep_timestump = millis();
    u8g2.setPowerSave(1);
  }

  
}

void buttonsActive() {
  button[0] = !digitalRead(BUTTON_UP);
  button[1] = !digitalRead(BUTTON_OK);
  button[2] = !digitalRead(BUTTON_DOWN);
  button[3] = !digitalRead(BUTTON_BACK);


  for (byte i = 0; i < 4; i++) {
    if (stateButton[i] != button[i]) {
      stateButton[i] = button[i];
      if (button[i] == 1) {
        Serial.printf("Botton %d\n" , i);
        if ( i == 0 ) {
          sleep_timestump = millis();
          u8g2.setPowerSave(0);
        }
        else if ( i == 1 ) {
          //sendLoraCommand("DHT");
          u8g2.setPowerSave(0);
          sleep_timestump = millis();
          main_menu();
        }
        else if ( i == 2 ) {
          sleep_timestump = millis();
          u8g2.setPowerSave(0);
        }
        //        }
        else if ( i == 3 ) {
          sleep_timestump = millis();
          u8g2.setPowerSave(0);
          power_menu();
        }

      } //else drawCircles(i, 0);
    }
  }
}

float getIncludeTemperature() {
  float result = 0;
  temp_sensor_read_celsius(&result);
  u8g2.print(" " + String(result) + "C ");
  return result;
}

void sendLoraCommand(String cmd) {
  Serial.println("Send " + cmd);
  e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "1" + cmd + "1");
}

//void lightSleep() {
//    esp_sleep_enable_timer_wakeup();
//    esp_light_sleep_start();
//}
