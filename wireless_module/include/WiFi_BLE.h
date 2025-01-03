#ifndef _WIFI_BLE_H
#define _WIFI_BLE_H

#define DEBUG 1

#include <sstream>

#include "WiFi.h"
#include "HTTPClient.h"

#include "cJSON.h"
#include "cmd_Parse.h"

#include <BLEDevice.h>
#include <BLE2902.h>

#define bleServer "IoT_module"
#define STA_SSID "NinoC137"
#define STA_PASS "zyx666...+"

class WiFiData{
public:
  struct WiFiIndex
  {
    int idx;
    char* SSID;
    char* PassWord;
    char* devType = (char*)"esp32";
    IPAddress ipv4;
    std::string MacAddress;
    std::string devID;
  };

  std::string serverip = "iot.lyhctech.com";
  int serverport = 6588;

  WiFiIndex WiFi_store[3];
};

class HeartBeatPacket{  //心跳包
public:
  long keepAliveTime = 100;
  long keepLiveCnt;
};

class ProjectDataPacket{
public:
  bool blestatus;
  bool wifistatus;
  std::string time;

  int ble_rssi = 0; //强度在 -100 ～ 0 之间

  int worktime; //单位为秒
  int runTime;  //单位为秒

  std::string device_ID;
};

class MyServerCallbacks : public BLEServerCallbacks // 创建连接和断开调用类
{
public:
  void onConnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param); // 开始连接函数
  void onDisconnect(BLEServer *pServer); // 断开连接函数
};

class MyCallbacks : public BLECharacteristicCallbacks
{
public:
  void onWrite(BLECharacteristic *pCharacteristic);
};

extern WiFiData WiFi_Data;
extern WiFiClient client;
// extern HTTPClient http;
extern String readTCP;

extern HeartBeatPacket HeartBeat;
extern ProjectDataPacket ProjectData;

extern int my_timezone;
extern long gmtOffset_sec;
extern struct tm timeinfo;

extern std::string value;
extern int cJsonParseEnd;

extern BLEUUID ServiceUUID;
extern BLECharacteristic RX_Characteristics;
extern BLEDescriptor RX_Descriptor;
extern BLECharacteristic TX_Characteristics;
extern BLEDescriptor TX_Descriptor;

extern BLEAddress connectedDeviceAddress;

void WiFi_BLE_setUp();  //在WiFi开启之后调用此函数
void BLEHandler();
void WiFiHandler();

void updateLocalTime();

void callAPI();

#endif // !_WIFI_BLE_H