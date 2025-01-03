#include "cmd_Parse.h"
#include "common.h"

void cmd1(cJSON *root)
{
    cJSON *cmd_mode = cJSON_GetObjectItem(root, "mode");
    if (cmd_mode == NULL)
    {
        TX_Characteristics.setValue("json string error!!");
        TX_Characteristics.notify();
        return;
    }

    RGB_Mode = cmd_mode->valueint;

    cJSON *tx_root = cJSON_CreateObject();
    cJSON_AddItemToObject(tx_root, "res", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(tx_root, "cmd", cJSON_CreateNumber(1));
    cJSON_AddItemToObject(tx_root, "mode", cJSON_CreateNumber(RGB_Mode));
    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);
}

void cmd2(cJSON *root)
{
    cJSON *cmd_ssid = cJSON_GetObjectItem(root, "ssid");
    cJSON *cmd_psw = cJSON_GetObjectItem(root, "psw");
    if (cmd_ssid == NULL || cmd_psw == NULL)
    {
        TX_Characteristics.setValue("json string error!!");
        TX_Characteristics.notify();
        return;
    }
    // 将数据存储至全局变量
    WiFi_Data.WiFi_store[0].SSID = strdup(cmd_ssid->valuestring);
    WiFi_Data.WiFi_store[0].PassWord = strdup(cmd_psw->valuestring);

    cJSON *tx_root = cJSON_CreateObject();
    cJSON_AddItemToObject(tx_root, "res", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(tx_root, "cmd", cJSON_CreateNumber(2));
    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);

    WiFi.begin(WiFi_Data.WiFi_store[0].SSID, WiFi_Data.WiFi_store[0].PassWord);
}

void cmd3(cJSON *root) // 读取设备保存的WiFi(一个)
{
    cJSON *tx_root = cJSON_CreateObject();
    cJSON *tx_wifi_array = cJSON_CreateArray();
    // 开始生成json串

    cJSON_AddItemToObject(tx_root, "res", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(tx_root, "cmd", cJSON_CreateNumber(3));

    cJSON_AddItemToObject(tx_root, "ssid", cJSON_CreateString(WiFi_Data.WiFi_store[0].SSID));
    cJSON_AddItemToObject(tx_root, "psw", cJSON_CreateString(WiFi_Data.WiFi_store[0].PassWord));

    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);
}

void cmd4(cJSON *root)
{
    cJSON *cmd_R = cJSON_GetObjectItem(root, "R");
    cJSON *cmd_G = cJSON_GetObjectItem(root, "G");
    cJSON *cmd_B = cJSON_GetObjectItem(root, "B");
    if (cmd_R == NULL || cmd_G == NULL || cmd_B == NULL)
    {
        TX_Characteristics.setValue("json string error!!");
        TX_Characteristics.notify();
        return;
    }

    RGB_Mode = 3;
    R = cmd_R->valueint;
    G = cmd_G->valueint;
    B = cmd_B->valueint;

    cJSON *tx_root = cJSON_CreateObject();
    cJSON_AddItemToObject(tx_root, "res", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(tx_root, "cmd", cJSON_CreateNumber(4));
    cJSON_AddItemToObject(tx_root, "R", cJSON_CreateNumber(R));
    cJSON_AddItemToObject(tx_root, "G", cJSON_CreateNumber(G));
    cJSON_AddItemToObject(tx_root, "B", cJSON_CreateNumber(B));
    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);
}

void cmd5(cJSON *root)
{
    testAPI(); 
}

void cmd6(cJSON *root) // 设置时区
{
    cJSON *cmd_timezone = cJSON_GetObjectItem(root, "timezone");
    if (cmd_timezone == NULL)
    {
        TX_Characteristics.setValue("json string error!!");
        TX_Characteristics.notify();
        return;
    }
    my_timezone = cmd_timezone->valueint;
    gmtOffset_sec = my_timezone * 3600;
    updateLocalTime();

    cJSON *tx_root = cJSON_CreateObject();
    cJSON_AddItemToObject(tx_root, "res", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(tx_root, "cmd", cJSON_CreateNumber(6));
    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);
}

void cmd7(cJSON *root) // 查询时区
{
    cJSON *tx_root = cJSON_CreateObject();
    cJSON_AddItemToObject(tx_root, "res", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(tx_root, "cmd", cJSON_CreateNumber(7));
    cJSON_AddItemToObject(tx_root, "timezone", cJSON_CreateNumber(my_timezone));
    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);
}

void cmd8(cJSON *root) // 查询设备信息
{
    cJSON *tx_root = cJSON_CreateObject();

    WiFi_Data.WiFi_store[0].ipv4 = WiFi.localIP();

    cJSON_AddItemToObject(tx_root, "res", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(tx_root, "cmd", cJSON_CreateNumber(8));
    cJSON_AddItemToObject(tx_root, "staMAC", cJSON_CreateString(WiFi_Data.WiFi_store[0].MacAddress.c_str()));
    cJSON_AddItemToObject(tx_root, "staIP", cJSON_CreateString(WiFi_Data.WiFi_store[0].ipv4.toString().c_str()));
    cJSON_AddItemToObject(tx_root, "devID", cJSON_CreateString(WiFi_Data.WiFi_store[0].devID.c_str()));
    cJSON_AddItemToObject(tx_root, "devType", cJSON_CreateString(WiFi_Data.WiFi_store[0].devType));
    cJSON_AddItemToObject(tx_root, "current_time", cJSON_CreateString(ProjectData.time.c_str()));
    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);
}

void cmd12(cJSON *root) // 服务器设置
{
    cJSON *cmd_serverip = cJSON_GetObjectItem(root, "serverip");
    cJSON *cmd_serverport = cJSON_GetObjectItem(root, "serverport");

    if (cmd_serverip == NULL || cmd_serverport == NULL)
    {
        TX_Characteristics.setValue("json string error!!");
        TX_Characteristics.notify();
        return;
    }

    WiFi_Data.serverip = cmd_serverip->valuestring;
    WiFi_Data.serverport = cmd_serverport->valueint;

    // std::stringstream urlStream;
    // urlStream << "http://" << WiFi_Data.serverip << ":" << WiFi_Data.serverport;

    // http.begin(urlStream.str().c_str()); // 连接服务器对应域名

    cJSON *tx_root = cJSON_CreateObject();
    cJSON_AddItemToObject(tx_root, "res", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(tx_root, "cmd", cJSON_CreateNumber(12));
    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);
}

void cmd13(cJSON *root) // 读取服务器设置
{
    cJSON *tx_root = cJSON_CreateObject();
    cJSON_AddItemToObject(tx_root, "res", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(tx_root, "cmd", cJSON_CreateNumber(13));
    cJSON_AddItemToObject(tx_root, "serverip", cJSON_CreateString(WiFi_Data.serverip.c_str()));
    cJSON_AddItemToObject(tx_root, "serverport", cJSON_CreateNumber(WiFi_Data.serverport));
    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);
}

void cmd14(cJSON *root) // 心跳包设置
{
    cJSON *cmd_keepalivetime = cJSON_GetObjectItem(root, "keepalivetime");
    cJSON *cmd_keepalivecnt = cJSON_GetObjectItem(root, "keepalivecnt");

    if (cmd_keepalivetime == NULL || cmd_keepalivecnt == NULL)
    {
        TX_Characteristics.setValue("json string error!!");
        TX_Characteristics.notify();
        return;
    }

    HeartBeat.keepAliveTime = cmd_keepalivetime->valueint;
    HeartBeat.keepLiveCnt = cmd_keepalivecnt->valueint;

    cJSON *tx_root = cJSON_CreateObject();
    cJSON_AddItemToObject(tx_root, "res", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(tx_root, "cmd", cJSON_CreateNumber(14));
    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);
}

void cmd15(cJSON *root) // 读取心跳包设置
{
    cJSON *tx_root = cJSON_CreateObject();
    cJSON_AddItemToObject(tx_root, "res", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(tx_root, "cmd", cJSON_CreateNumber(15));
    cJSON_AddItemToObject(tx_root, "keepalivetime", cJSON_CreateNumber(HeartBeat.keepAliveTime));
    cJSON_AddItemToObject(tx_root, "keepalivecnt", cJSON_CreateNumber(HeartBeat.keepLiveCnt));
    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);
}

void cmd17() // 通过蓝牙向宿主机发送事件日志
{
    cJSON *tx_root = cJSON_CreateObject();
    // cJSON_AddItemToObject(tx_root, "device_id", cJSON_CreateString(ProjectData.device_ID.c_str()));
    cJSON_AddItemToObject(tx_root, "device_id", cJSON_CreateString(WiFi_Data.WiFi_store[0].devID.c_str()));
    cJSON_AddItemToObject(tx_root, "blestatus", cJSON_CreateNumber(ProjectData.blestatus));
    cJSON_AddItemToObject(tx_root, "wifistatus", cJSON_CreateNumber(ProjectData.wifistatus));
    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);
}

void cmd19(cJSON *root) // 初始化设备信息
{
    cJSON *cmd_devName = cJSON_GetObjectItem(root, "devName");
    if (cmd_devName == NULL)
    {
        TX_Characteristics.setValue("json string error!!");
        TX_Characteristics.notify();
        return;
    }
    ProjectData.device_ID = cmd_devName->valuestring;

    cJSON *tx_root = cJSON_CreateObject();
    cJSON_AddItemToObject(tx_root, "res", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(tx_root, "cmd", cJSON_CreateNumber(19));
    char *json_string = cJSON_Print(tx_root);
    // 生成完毕, 准备发送
    TX_Characteristics.setValue(json_string);
    TX_Characteristics.notify();
#if DEBUG
    Serial.printf("%s\r\n", json_string);
#endif // DEBUG
    cJSON_Delete(tx_root);
    free(json_string);
}