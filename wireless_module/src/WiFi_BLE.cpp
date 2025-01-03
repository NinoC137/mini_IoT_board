#include "WiFi_BLE.h"

WiFiClient client; // ESP32设置为客户端, TCP连接服务器
const IPAddress serverIP(192, 168, 1, 1);
uint16_t serverPort = 8888;

const char *apiUrl = "https://api.deepseek.com/chat/completions";
const char *apiKey = "sk-96a3f94798ed41e1a44b5b997a3c27f4";

String readTCP;

BLEUUID ServiceUUID("ab1ad444-6724-11e9-a923-1681be663d3e");                                                                                         // 服务的UUID
BLECharacteristic RX_Characteristics("ab1ad980-6724-11e9-a923-1681be663d3e", BLECharacteristic::PROPERTY_WRITE);                                     // 接收字符串的特征值
BLEDescriptor RX_Descriptor(BLEUUID((uint16_t)0x2901));                                                                                              // 接收字符串描述符
BLECharacteristic TX_Characteristics("ab1adb06-6724-11e9-a923-1681be663d3e", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY); // 发送字符串的特征值
BLEDescriptor TX_Descriptor(BLEUUID((uint16_t)0x2902));                                                                                              // 发送字符串描述符

BLEAddress connectedDeviceAddress = BLEAddress("00:00:00:00:00:00");

std::string value;
char *BLE_json_root;
char *WiFi_json_root;
int cJsonParseEnd;

// WiFi信息存储对象, 存储3个WiFi信息
WiFiData WiFi_Data;
// HTTP访问域名对象
HTTPClient http;
// 心跳包对象,存储心跳包在线时间与发送计数
HeartBeatPacket HeartBeat;
// 事件日志包, 存储各类信息
ProjectDataPacket ProjectData;

//-----------网络时间获取-----------//
const char *ntpServer = "pool.ntp.org"; // 网络时间服务器
// 时区设置函数，东八区（UTC/GMT+8:00）写成8*3600
int my_timezone = 8;
long gmtOffset_sec = my_timezone * 3600;
const int daylightOffset_sec = 0; // 夏令时填写3600，否则填0
struct tm timeinfo;

void MyServerCallbacks::onConnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param) // 开始连接函数
{
    ProjectData.blestatus = true;
    connectedDeviceAddress = BLEAddress(param->connect.remote_bda);
    Serial.printf("your ble address is :%s\r\n", connectedDeviceAddress.toString().c_str());
}
void MyServerCallbacks::onDisconnect(BLEServer *pServer) // 断开连接函数
{
    ProjectData.blestatus = false;
    pServer->getAdvertising()->start();
}
void MyCallbacks::onWrite(BLECharacteristic *pCharacteristic) // 写方法
{
    value = pCharacteristic->getValue(); // 接收值
    if (value.empty())
    {
    }
}

// RSSI 回调处理
void handle_rssi_event(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    if (event == ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT)
    {
        if (param->read_rssi_cmpl.status == ESP_BT_STATUS_SUCCESS)
        {
            ProjectData.ble_rssi = param->read_rssi_cmpl.rssi;
            // Serial.print("RSSI of connected device: ");
            // Serial.println(ProjectData.ble_rssi);
        }
        else
        {
            Serial.println("Failed to read RSSI.");
        }
    }
}

void WiFi_BLE_setUp()
{
    WiFi_Data.WiFi_store[0].SSID = (char *)STA_SSID;
    WiFi_Data.WiFi_store[0].PassWord = (char *)STA_PASS;

    WiFi.mode(WIFI_STA);
    WiFi.begin(WiFi_Data.WiFi_store[0].SSID, WiFi_Data.WiFi_store[0].PassWord);

    WiFi_Data.WiFi_store[0].ipv4 = WiFi.localIP();
    WiFi_Data.WiFi_store[0].MacAddress = WiFi.macAddress().c_str();
    WiFi_Data.WiFi_store[0].devID = WiFi_Data.WiFi_store[0].MacAddress;

    size_t pos = WiFi_Data.WiFi_store[0].devID.find(":");
    while (pos != std::string::npos)
    {
        WiFi_Data.WiFi_store[0].devID.erase(pos, 1);
        pos = WiFi_Data.WiFi_store[0].devID.find(":");
    }

    if (WiFi.isConnected() == true)
    {
        ProjectData.wifistatus = true;
    }
    else
    {
        ProjectData.wifistatus = false;
    }

    Serial.print("IPv4 address:");
    Serial.println(WiFi_Data.WiFi_store[0].ipv4);
    Serial.print("Mac:");
    Serial.printf("%s\r\n", WiFi_Data.WiFi_store[0].MacAddress.c_str());
    Serial.print("devID:");
    Serial.printf("%s\r\n", WiFi_Data.WiFi_store[0].devID.c_str());

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    BLEDevice::init(bleServer);                     // 初始化BLE客户端设备
    BLEServer *pServer = BLEDevice::createServer(); // BLEServer指针，创建Server
    BLEDevice::setMTU(256);
    BLEService *pService = pServer->createService(ServiceUUID); // BLEService指针，创建Service

    pServer->setCallbacks(new MyServerCallbacks()); // 设置连接和断开调用类

    // 注册 RSSI 回调
    esp_ble_gap_register_callback(handle_rssi_event);

    pService->addCharacteristic(&RX_Characteristics);
    RX_Descriptor.setValue("BLE Receive");
    RX_Characteristics.addDescriptor(new BLE2902());

    pService->addCharacteristic(&TX_Characteristics);
    TX_Descriptor.setValue("BLE Transmit");
    TX_Characteristics.addDescriptor(new BLE2902());

    RX_Characteristics.setCallbacks(new MyCallbacks()); // 设置回调函数
    TX_Characteristics.setCallbacks(new MyCallbacks()); // 设置回调函数

    RX_Characteristics.setValue("BLE RX Start"); // 发送信息
    TX_Characteristics.setValue("BLE TX Start"); // 发送信息

    pService->start();
    pServer->getAdvertising()->start();
}

void BLEHandler()
{
    if (value.length() > 0)
    {
        BLE_json_root = (char *)value.data();
        // Serial.printf("json string: %s\r\n value: %s\r\n", json_string, value.c_str());
        cJSON *root = cJSON_Parse(BLE_json_root);
        if (root == NULL)
        {
            Serial.printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        }
        cJSON *cmd = cJSON_GetObjectItem(root, "cmd");

        switch (cmd->valueint)
        {
        case 1:
            cmd1(root);
            break;
        case 2:
            cmd2(root);
            break;
        case 3:
            cmd3(root);
            break;
        case 4:
            cmd4(root);
            break;
        case 5:
            cmd5(root);
            break;
        case 6:
            cmd6(root);
            break;
        case 7:
            cmd7(root);
            break;
        case 8:
            cmd8(root);
            break;
        case 12:
            cmd12(root);
            break;
        case 13:
            cmd13(root);
            break;
        case 14:
            cmd14(root);
            break;
        case 15:
            cmd15(root);
            break;
        case 17:
            cmd17();
            break;
        default:
            Serial.printf("error cmd!\r\n");
            TX_Characteristics.setValue("received cmd: ERROR!");
            TX_Characteristics.notify();
            break;
        }

        cJSON_Delete(root);
        value.clear();
    }
}

void WiFiHandler()
{
    // WIFI连接服务器部分
    int httpCode = http.GET();
    Serial.printf("WiFi: Get http code: %d\r\n", httpCode);
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK) // HTTP请求无异常
        {
            WiFi_json_root = (char *)http.getString().c_str(); // 读取get到的json串
            Serial.println(WiFi_json_root);

            std::string post_Payload("ESP32 POST TEST");

            cJSON *root = cJSON_Parse(WiFi_json_root);
            if (root == NULL)
            {
                Serial.printf("Error before: [%s]\n", cJSON_GetErrorPtr());
            }
            cJSON *cmd = cJSON_GetObjectItem(root, "cmd");

            switch (cmd->valueint)
            {
            default:
                Serial.printf("error cmd!\r\n");
                http.POST(post_Payload.c_str());
                break;
            }

            cJSON_Delete(root);
        }
    }
}

void updateLocalTime()
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if (!getLocalTime(&timeinfo))
    {
        ProjectData.time = std::string("Failed to obtain time");
        return;
    }
    else
    {
        char time_str[64];
        sprintf(time_str, "%04d-%02d-%02d %02d:%02d:%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        ProjectData.time = std::string(time_str);
    }
}

void callAPI()  //stream流式对话
{
    // 检查Wi-Fi连接状态
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http_ai;

        // 开始HTTP请求
        http_ai.begin(apiUrl);
        http_ai.addHeader("Content-Type", "application/json");
        http_ai.addHeader("Authorization", "Bearer " + String(apiKey));

        // 创建请求体
        cJSON *jsonRequest = cJSON_CreateObject();
        cJSON_AddStringToObject(jsonRequest, "model", "deepseek-chat");

        // 创建messages数组
        cJSON *messagesArray = cJSON_AddArrayToObject(jsonRequest, "messages");

        // 添加system消息
        cJSON *systemMessage = cJSON_CreateObject();
        cJSON_AddStringToObject(systemMessage, "role", "system");
        cJSON_AddStringToObject(systemMessage, "content", "You are a helpful assistant");
        cJSON_AddItemToArray(messagesArray, systemMessage);

        // 添加user消息
        cJSON *userMessage = cJSON_CreateObject();
        cJSON_AddStringToObject(userMessage, "role", "user");
        cJSON_AddStringToObject(userMessage, "content", "你好!我刚刚通过api访问到了你,请为我重新介绍一下你自己!");
        cJSON_AddItemToArray(messagesArray, userMessage);

        // 启用流式传输
        cJSON_AddBoolToObject(jsonRequest, "stream", true);

        // 将cJSON对象转换为字符串
        char *requestBody = cJSON_Print(jsonRequest);
        Serial.println("请求体: " + String(requestBody));

        // 发送POST请求
        int httpResponseCode = http_ai.POST(requestBody);

        // 检查响应状态
        if (httpResponseCode > 0)
        {
            Serial.println("HTTP响应代码: " + String(httpResponseCode));

            // 获取流式响应
            WiFiClient *stream = http_ai.getStreamPtr();
            while (http_ai.connected())
            {
                if (stream->available())
                {
                    String line = stream->readStringUntil('\n');
                    line.trim(); // 去除换行符和空格

                    // 检查是否为有效数据行
                    if (line.startsWith("data: "))
                    {
                        String jsonStr = line.substring(6); // 去掉 "data: " 前缀
                        Serial.println("收到数据: " + jsonStr);

                        // 解析JSON数据
                        cJSON *jsonResponse = cJSON_Parse(jsonStr.c_str());
                        if (jsonResponse != NULL)
                        {
                            cJSON *choicesArray = cJSON_GetObjectItemCaseSensitive(jsonResponse, "choices");
                            if (cJSON_IsArray(choicesArray))
                            {
                                cJSON *firstChoice = cJSON_GetArrayItem(choicesArray, 0);
                                if (firstChoice != NULL)
                                {
                                    cJSON *delta = cJSON_GetObjectItemCaseSensitive(firstChoice, "delta");
                                    if (delta != NULL)
                                    {
                                        cJSON *content = cJSON_GetObjectItemCaseSensitive(delta, "content");
                                        if (cJSON_IsString(content) && (content->valuestring != NULL))
                                        {
                                            Serial.print(content->valuestring); // 打印流式内容
                                        }
                                    }
                                }
                            }
                            cJSON_Delete(jsonResponse);
                        }
                        else
                        {
                            Serial.println("解析JSON失败: " + jsonStr);
                        }
                    }
                }
                delay(10); // 避免CPU占用过高
            }
        }
        else
        {
            Serial.println("请求失败，错误代码: " + String(httpResponseCode));
        }

        // 释放内存
        cJSON_Delete(jsonRequest);
        free(requestBody);

        // 结束HTTP请求
        http_ai.end();
    }
    else
    {
        Serial.println("Wi-Fi未连接");
    }
}