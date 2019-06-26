/*------------------------------------------------------------------------------
  05/24/2019
  Author: Schygulla
  Platform: ESP8266
  Language: C++/Arduino/javascript
  File: webserver_websockets.ino
  ------------------------------------------------------------------------------
  Description:
  Code for demonstrating how to transfer data between a web server
  and a web client in real-time using websockets.

  ------------------------------------------------------------------------------*/

#include <ESP8266WiFi.h>                    //https://github.com/esp8266/Arduino
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Average.h>
#include <ArduinoJson.h>
#include <TimeLib.h>                        //https://github.com/PaulStoffregen/Time
#include <WiFiUdp.h>
#include <Timezone.h>                       //https://github.com/JChristensen/Timezone
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"
SdFat sd;

// create a serial stream
ArduinoOutStream cout(Serial);
const char *divider = F("//------------------------------------------------------------------------------");

//---------------------------------------------------------------
/** WiFi Settings **/
const char *ssid = "UPC3651551";
const char *password = "MEHHTNQA";
const char *ESP_HOST_NAME = "test.ino";// + ESP.getFlashChipId();

// Data wire is plugged into pin 5 on the ESP8266
#define ONE_WIRE_BUS D1
// CS wire is plugged into pin 4 on the ESP8266
#define SD_SPI_CS D2
// OnBoard LED is plugged into pin 2 on the ESP8266
#define OnBoard_LED_Pin D4
// OnBoard FLASH BUTTON is plugged into pin 0 on the ESP8266
#define OnBoard_FLASH_BUTTON D3
// 18B20_Resolution
#define Resolution_18B20 11

// Reserve space for 10 entries in the average bucket.
// Change the type between < and > to change the entire way the library works.
Average<float> Tmpave0(10);
Average<float> Tmpave1(10);

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

//Blink settings
String ms = "500";
String blinks = "1";
String ledstatus = "Led off";

long lastMils = 0;
String temp;
float lastTemp = 0;
float Temperatur1;
//---------------------------------------------------------------
//Led Gamma Correction
const uint16_t tblgamma[] PROGMEM = {
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    1,    1,    1,    1,    1,    1,    1,    1,    2,    2,    2,    2,    2,    3,    3,
  3,    3,    4,    4,    4,    5,    5,    5,    6,    6,    7,    7,    7,    8,    8,    9,
  10,   10,   11,   11,   12,   13,   13,   14,   15,   15,   16,   17,   18,   19,   20,   20,
  21,   22,   23,   24,   25,   26,   27,   29,   30,   31,   32,   33,   35,   36,   37,   38,
  40,   41,   43,   44,   46,   47,   49,   50,   52,   54,   55,   57,   59,   61,   63,   64,
  66,   68,   70,   72,   74,   77,   79,   81,   83,   85,   88,   90,   92,   95,   97,  100,
  102,  105,  107,  110,  113,  115,  118,  121,  124,  127,  130,  133,  136,  139,  142,  145,
  149,  152,  155,  158,  162,  165,  169,  172,  176,  180,  183,  187,  191,  195,  199,  203,
  207,  211,  215,  219,  223,  227,  232,  236,  240,  245,  249,  254,  258,  263,  268,  273,
  277,  282,  287,  292,  297,  302,  308,  313,  318,  323,  329,  334,  340,  345,  351,  357,
  362,  368,  374,  380,  386,  392,  398,  404,  410,  417,  423,  429,  436,  442,  449,  455,
  462,  469,  476,  483,  490,  497,  504,  511,  518,  525,  533,  540,  548,  555,  563,  571,
  578,  586,  594,  602,  610,  618,  626,  634,  643,  651,  660,  668,  677,  685,  694,  703,
  712,  721,  730,  739,  748,  757,  766,  776,  785,  795,  804,  814,  824,  833,  843,  853,
  863,  873,  884,  894,  904,  915,  925,  936,  946,  957,  968,  979,  990, 1001, 1012, 1023
};

//---------------------------------------------------------------
/** Config Settings **/
struct Config {
  uint16_t brightness;
  char hostname[64];
  int port;
};

const char *configfilename = "/config.txt";  // <- SD library uses 8.3 filenames
Config config;                                // <- global configuration object

#include "webSocketEvent.h"
#include "getContentType.h"
#include "handleFileRead.h"
#include "time_start.h"
#include "configuration.h"
#include "filemg.h"

ADC_MODE(ADC_VCC);//ESP needs to reconfigure the ADC at startup in order for this feature to be available.

//---------------------------------------------------------------
void setup() {
  // Initialize serial port
  Serial.begin(115200);

  // Wait for USB Serial
  while (!Serial) {
    SysCall::yield();
  }

  // Print ESP8266 Startup Parameter
  cout << divider << endl;
  cout << F("Reset reason: ") << ESP.getResetReason().c_str() << endl;
  cout << F("Sdk version: ") << ESP.getSdkVersion() << endl;
  cout << F("Core Version: ") << ESP.getCoreVersion().c_str() << endl;
  cout << F("Boot Version: ") << ESP.getBootVersion() << endl;
  cout << F("Boot Mode: ") << ESP.getBootMode() << endl;
  cout << F("CPU Frequency: ") << ESP.getCpuFreqMHz() << F("MHz") << endl;
  cout << F("Chip ID: ") << ESP.getFlashChipId() << endl;
  Serial.printf("Boot Version: %u\n", ESP.getBootVersion());
  Serial.printf("Boot Mode: %u\n", ESP.getBootMode());
  Serial.printf("CPU Frequency: %u MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Vcc: %f Volt\n", ESP.getVcc() / 1000);
  cout << ESP.getVcc() << endl;
  cout << ESP.getResetInfo().c_str() << endl;

  //---------------------------------------------------------------
  /** Start the SD CARD **/
  cout << divider << endl;
  cout << F("mounting SD CARD") << endl;
  if (!sd.begin(SD_SPI_CS, SPI_FULL_SPEED)) {//, SD_SCK_MHZ(50), SPI_HALF_SPEED
    cout << F("An Error has occurred while mounting SD CARD") << endl;
    return;
  } else {
    cout << F("SD CARD ready!") << endl;
  }

  pinMode(OnBoard_LED_Pin, OUTPUT);
  pinMode(ONE_WIRE_BUS, INPUT);
  pinMode(OnBoard_FLASH_BUTTON, INPUT);

  sensors.begin();
  sensors.setResolution(Resolution_18B20);

  //---------------------------------------------------------------
  /** Start WiFi **/
  WiFi.mode(WIFI_STA);
  IPAddress ip(192, 168, 0, 100);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(192, 168, 0, 1);
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.begin(ssid, password);
  WiFi.hostname(ESP_HOST_NAME);//WiFi.sethostname(ESP_HOST_NAME);
  cout << divider << endl;
  cout << F("Connecting WiFi") << endl;
  while (WiFi.status() != WL_CONNECTED) {
    cout << F(".");
    delay(500);
  }
  cout << endl;
  cout << F("Connected to ") << WiFi.SSID().c_str() << " Use IP address: " << WiFi.localIP().toString().c_str() << endl; // Report which SSID and IP is in use
  
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    timer0_detachInterrupt();
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  //  Serial.println("Ready");
  //  Serial.print("IP address: ");
  //  Serial.println(WiFi.localIP());

  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.begin();                           // Actually start the server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  cout << F("//---------------------------------------------------------------") << endl;
  cout << F("requestTemperatures...") << endl;
  long savenow = millis();
  sensors.requestTemperatures(); // Send the command to get temperatures
  sensors.getTempCByIndex(0);
  sensors.getTempCByIndex(1);
  sensors.getTempCByIndex(2);
  sensors.getTempCByIndex(3);
  sensors.getTempCByIndex(4);
  sensors.getTempCByIndex(5);
  sensors.getTempCByIndex(6);
  long duration = millis() - savenow;
  cout << F("duration:") << duration << endl;
  savenow = millis();
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println(String(sensors.getTempCByIndex(0)));
  Serial.println(String(sensors.getTempCByIndex(1)));
  Serial.println(String(sensors.getTempCByIndex(2)));
  Serial.println(String(sensors.getTempCByIndex(3)));
  Serial.println(String(sensors.getTempCByIndex(4)));
  Serial.println(String(sensors.getTempCByIndex(5)));
  Serial.println(String(sensors.getTempCByIndex(6)));
  duration = millis() - savenow;
  cout << F("duration:") << duration << endl;
//  Serial.println(String(sensors.getTempCByIndex(0)));
//  webSocket.broadcastTXT(String(Temperatur1));

  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(sensors_ISR);
  timer0_write(ESP.getCycleCount() + 80000000L); // 80MHz == 1sec
  interrupts();

  // Should load default config if run for the first time
  Serial.println(F("//---------------------------------------------------------------"));
  Serial.println(F("Loading configuration..."));
  loadConfiguration(configfilename, config);

  // Create configuration file
  Serial.println(F("\n//---------------------------------------------------------------"));
  Serial.println(F("Saving configuration..."));
  saveConfiguration(configfilename, config);

  // Dump config file
  Serial.println(F("\n//---------------------------------------------------------------"));
  Serial.println(F("Print config file..."));
  printFile(configfilename);

  // Dump Directory content
  Serial.println(F("\n//---------------------------------------------------------------"));
  Serial.println("Print Directory content...");
  sd.ls(LS_R | LS_DATE | LS_SIZE);

  Udp.begin(localPort);
  Serial.println(F("//---------------------------------------------------------------"));
  Serial.println(F("Synchronizing Time over UDP"));
  Serial.print(F("UDP Local port: "));
  Serial.println(Udp.localPort());
  setSyncProvider(getNtpTime);
  setSyncInterval(3600);//Anzahl der Sekunden zwischen dem erneuten Synchronisieren. 86400 = 1 Tag

  Serial.println(F("//---------------------------------------------------------------"));
  Serial.println(F("Preparing sensor.log File"));
  long saveow = now();
  int a = countline("sensor.log");
  long dur = now() - saveow;
  cout << "duration:" << dur << endl;

  //  int b = movelines("sensor.log", "sensor.old", 3600);
  cout << "count Lines:" << a << endl;
  //  cout << "move Lines:" << b << endl;
      sd.remove("sensor.log");
      sd.remove("new.log");

  cout << "\nDone!\n";
  //int sht = shiftLines("/sensor.log", "/sensor.old", 10800);
  //Serial.println("Lines shifted: " + String(sht));
  //countines("/sensor.log");
}
time_t prevDisplay = 0; // when the digital clock was displayed

void loop() {
  ArduinoOTA.handle();
  webSocket.loop();
  server.handleClient();

  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      lokaleZeit();
      //      digitalClockPrintSerial();
    }
  }

  String DataToSend = "";
  if (Serial.available() > 0) {
    String rxConsole = Serial.readString();
    rxConsole.trim();
    DataToSend += "rxConsole=" + rxConsole + "?";
  }

  long millisnow = now();
  if (millisnow > lastMils) {
    String savenow = String(now());

    DataToSend += "epoch=" + savenow + "?";
    DataToSend += "Temp1=" + String(Tmpave0.mean()) + "?";
    DataToSend += "Temp2=" + String(Tmpave1.mean()) + "?";
    DataToSend += "Sw1=" + String(digitalRead(OnBoard_FLASH_BUTTON));// + "?";

    webSocket.broadcastTXT(DataToSend);

    saveData("/sensor.log", "1", savenow, String(Tmpave0.mean()));
    saveData("/sensor.log", "2", savenow, String(Tmpave1.mean()));
    saveData("/sensor.log", "3", savenow, String(digitalRead(OnBoard_FLASH_BUTTON)));
    //Serial.println(DataToSend);
    lastMils = millisnow;
  }
  yield();
}

void sensors_ISR (void) {
  sensors.requestTemperatures(); // Send the command to get temperatures
  //  Temperatur1 = sensors.getTempCByIndex(0);
  Tmpave0.push(sensors.getTempCByIndex(0));
  Tmpave1.push(sensors.getTempCByIndex(1));
  timer0_write(ESP.getCycleCount() + 80000000L); // 80MHz == 1sec
}
