
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
#include "driver/rtc_io.h"

// With FIXED RECEIVER configuration
#define DESTINATION_ADDL 2

// If you want use RSSI uncomment //#define ENABLE_RSSI true
// and use relative configuration with RSSI enabled
#define ENABLE_RSSI true

#include "Arduino.h"
#include "LoRa_E220.h"
#include <Wire.h>

#include <U8g2lib.h>

#include <ArduinoJson.h>        //Установить из менеджера библиотек.
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

void initTempSensor() {//метод внутренней температуры
  temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
  temp_sensor.dac_offset = TSENS_DAC_L2;  // TSENS_DAC_L2 is default; L4(-40°C ~ 20°C), L2(-10°C ~ 80°C), L1(20°C ~ 100°C), L0(50°C ~ 125°C)
  temp_sensor_set_config(temp_sensor);
  temp_sensor_start();
}

HardwareSerial MySerial(1);//Lora Serial

#define WIRE Wire

#define BUTTON_UP 1
#define BUTTON_OK 2
#define BUTTON_DOWN 4
#define BUTTON_BACK 6

#define LED_PIN  15
#define VBAT_PIN GPIO_NUM_10

// E220 UART: Arduino TX --> E220 RX, Arduino RX <-- E220 TX
// On ESP32-S2-WROOM/WROVER GPIO33-39 are often tied to in-package flash/PSRAM.
// If the board reboots/loops with flash errors, move UART to free GPIOs (e.g. 17/18).
#define LORA_TX_PIN 37
#define LORA_RX_PIN 39
#define LORA_AUX_PIN AUX_PIN
#define LORA_M0_PIN 3
#define LORA_M1_PIN 5
#define LORA_CHANNEL 23

#define DHT_SEND_INTERVAL_MS 5000UL

//#define FORMAT_SPIFFS_IF_FAILED true
#define FORMAT_LITTLEFS_IF_FAILED true

#define DBG_OUTPUT_PORT Serial

bool button[] = {0, 0, 0, 0};
bool stateButton[] = {0, 0, 0, 0};

int lastRssi = 0;

//bool print_logf_status = true;
//bool led_msg = true;
bool send_dht = false;
//bool allways_on_disp = false;
bool display_on = true;

//bool procent_battery = false;

unsigned long icon_timestamp = 0;
unsigned long sleep_timestump = millis();

uint8_t lora_link = 3;
uint16_t lora_symb[4] = {0xe21e, 0xe21d, 0xe21c, 0xe21b};

volatile bool interruptExecuted = false;


String time_substring;
String old_time_substring;

float host_temp = 0;
float host_humid = 0;

uint8_t current_selection = 0;

uint8_t relay[8] {B00000000};

RTC_DATA_ATTR int bootCount = 0;

//json test

uint8_t client_id = 1;
String SSDPName = "GarageClient";
String ssidAPName = "ESP32LogServer";
String ssidAPPassword = "12345678";
String ssidName = "";
String ssidPassword = "";
int timezone = 4;
String ntp = "pool.ntp.org";
uint8_t off_display_sec = 60;
bool allways_on_disp = false;
bool led_msg = true;
bool print_logf_status = true;
bool procent_battery = false;


struct Info {
  unsigned long msgtime;
  float temp;
  float humid;
  //float power;
  int rssi;
  uint8_t relay[8] {B00000000};
  bool save = false;
};

struct loadsConfig {
  int id;
  String name;
  bool state;
};

struct Info r_info;
String jsonConfig = "{}";

const char* config_filename = "/config.json";  // <- SD library uses 8.3 filenames
loadsConfig s_Relay;


// Определяем переменные wifi
String SSDP_Name = "Update"; // Имя SSDP
String _ssid     = ""; // Для хранения SSID
String _password = ""; // Для хранения пароля сети
String _ssidAP = "ESP32LogServer";   // SSID AP точки доступа
String _passwordAP = "12345678"; // пароль точки доступа

String Pinout_name[8] = {"Load1", "Load2", "Load3", "Load4", "Load5", "Load6", "Load7", "Load8"}; //Названия выводов
byte Pinout[8] = {1, 0, 0, 0, 0, 0, 0, 0}; //статус включения вывода(виртуального)

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

File fsUploadFile;

//AsyncWebServer server(80, LittleFS, "myServer");

//unsigned int time_zone = 4;
//String _ntp = "pool.ntp.org";

char ntpServer[64] = "pool.ntp.org";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, 3600 * 4, 60000);

ESP32Time rtc(0);

// Log file configuration
const char* LOG_FILE_PATH = "/log.txt";

bool Wifi_boot = false;
bool WIFI_AP_on = false;
bool littlefs_ok = false;

unsigned long epochTime;

int old_rssi = 0;




//LiquidCrystal_PCF8574 lcd(0x27);

//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SSD1306_128X64_ALT0_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // same as the NONAME variant, but may solve the "every 2nd line skipped" problem
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* reset=*/ 8);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);
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
LoRa_E220 e220ttl(LORA_TX_PIN, LORA_RX_PIN, &MySerial, LORA_AUX_PIN, LORA_M0_PIN, LORA_M1_PIN, UART_BPS_RATE_9600, SERIAL_8N1);
// -------------------------------------

// ---------- Raspberry PI Pico pins --------------
// LoRa_E220 e220ttl(&Serial2, 2, 10, 11); //  RX AUX M0 M1
// -------------------------------------

// ---------------- STM32 --------------------
//HardwareSerial Serial2(USART2);   // PA3  (RX)  PA2  (TX)
//LoRa_E220 e220ttl(&Serial2, PA0, PB0, PB10); //  RX AUX M0 M1
// -------------------------------------------------
void printParameters(struct Configuration configuration);
void IRAM_ATTR wakeUp();

void setup() {
  Serial.begin(115200);
  delay(200);

  // Display first: LoRa begin() can block forever waiting for AUX.
  u8g2.begin(/* menu_select_pin= */ BUTTON_OK, /* menu_next_pin= */ BUTTON_DOWN, /* menu_prev_pin= */ BUTTON_UP, /* menu_home_pin= */ BUTTON_BACK);
  u8g2.enableUTF8Print();
  u8g2.setPowerSave(0);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_t_symbols);
  u8g2.drawStr(0, 12, "Boot...");
  u8g2.sendBuffer();
  Serial.println("Start 3-SSD1306 Display and botton module");

  initTempSensor();
  Serial.println("Start 1 TempSensor");

  u8g2.drawStr(0, 24, "LoRa...");
  u8g2.sendBuffer();
  e220ttl.begin();
  Serial.println("Start 2 LoRa module");

  bool showSplash = (bootCount == 0);
  bootCount++;

  littlefs_ok = LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED);
  if (!littlefs_ok) {
    Serial.println("LittleFS Mount Failed");
    u8g2.drawStr(0, 36, "LittleFS Failed");
    u8g2.sendBuffer();
  } else {
    u8g2.drawStr(0, 36, "LittleFS Mnted");
    u8g2.setCursor(0, 48);
    u8g2.print("Boot number: ");
    u8g2.print(bootCount);
    u8g2.sendBuffer();
    if (showSplash) {
      ResponseStructContainer c = e220ttl.getConfiguration();
      if (c.data != NULL) {
        Configuration configuration = *(Configuration*) c.data;
        Serial.println(c.status.getResponseDescription());
        Serial.println(c.status.code);
        printParameters(configuration);
      }
      c.close();
      listDir(LittleFS, "/", 0);
      delay(1000);
    }
    loadConfig();
    strncpy(ntpServer, ntp.c_str(), sizeof(ntpServer) - 1);
    ntpServer[sizeof(ntpServer) - 1] = '\0';
    timeClient.setPoolServerName(ntpServer);
    timeClient.setTimeOffset(3600 * timezone);
    Serial.println("Start 4-Load Json config");
  }

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_OK, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_BACK, INPUT_PULLUP);
  pinMode(LORA_AUX_PIN, INPUT_PULLUP);
  pinMode(VBAT_PIN, INPUT);

  analogReadResolution(12);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  e220ttl.setMode(MODE_0_NORMAL);

  epochTime = millis() / 1000;
  sleep_timestump = millis();
  display_on = true;
  if (Wifi_boot) {
    WIFIinit();
  }
}

void loop() {

  
  if(interruptExecuted) {
    Serial.println("WakeUp Callback, AUX pin go LOW and start receive message!");
    Serial.flush();
    interruptExecuted = false;
    e220ttl.setMode(MODE_0_NORMAL);
    display_on = true;
    u8g2.setPowerSave(0);
    sleep_timestump = millis();
  }
  
  buttonsActive();
  UpdateLoraInfoStruct();
  if (display_on) {
    main_view();
  }

  if (send_dht) {
    static unsigned long last_dht_send = 0;
    if (millis() - last_dht_send >= DHT_SEND_INTERVAL_MS) {
      last_dht_send = millis();
      testDhtMessage();
    }
  }

  if (Serial.available()) {
    String input = Serial.readString();
    ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, LORA_CHANNEL, input);
    Serial.println(rs.getResponseDescription());
  }
  if (!allways_on_disp && !Wifi_boot && (millis() - sleep_timestump > (unsigned long)off_display_sec * 1000UL)) {
    display_on = false;
    light_sleep(false);
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
        bool was_on = display_on;
        sleep_timestump = millis();
        display_on = true;
        u8g2.setPowerSave(0);
        if ( i == 1 && was_on) {
          main_menu();
        }
        else if ( i == 3 && was_on) {
          power_menu();
        }

      } //else drawCircles(i, 0);
    }
  }
}

float getIncludeTemperature() {
  float result = 0;
  temp_sensor_read_celsius(&result);
  return result;
}

void sendLoraCommand(String cmd) {
  Serial.println("Send " + cmd);
  e220ttl.sendFixedMessage(0, DESTINATION_ADDL, LORA_CHANNEL, "1" + cmd + "1");
}

//void lightSleep() {
//    esp_sleep_enable_timer_wakeup();
//    esp_light_sleep_start();
//}
