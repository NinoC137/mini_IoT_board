#include <Arduino.h>
#include "WiFi_BLE.h"
#include "common.h"

#include "multi_button.h"
#include "FastLED.h"

void LEDTaskThread(void *argument);
TaskHandle_t LEDTaskHandle;

void RGBTaskThread(void *argument);
TaskHandle_t RGBTaskHandle;

void IoTTaskThread(void *argument);
TaskHandle_t IoTTaskHandle;

void KEYTaskThread(void *argument);
TaskHandle_t KEYTaskHandle;

CRGB leds[1];

void setup()
{
  Serial.begin(115200);  
  Serial.println("------IoT System Beginning------\r\n");

  xTaskCreatePinnedToCore(LEDTaskThread, "LEDTask", 1024 * 4, NULL, 1, &LEDTaskHandle, 0);

  xTaskCreatePinnedToCore(RGBTaskThread, "RGBTask", 1024 * 4, NULL, 1, &RGBTaskHandle, 0);

  xTaskCreatePinnedToCore(KEYTaskThread, "KEYTask", 1024 * 4, NULL, 1, &KEYTaskHandle, 0);

  xTaskCreatePinnedToCore(IoTTaskThread, "IoTTask", 1024 * 6, NULL, 2, &IoTTaskHandle, 1);
}

void loop()
{
  delay(5);
}

void IoTTaskThread(void *argument){ 
  WiFi_BLE_setUp();

  for(;;){
    BLEHandler();
    // WiFiHandler();
    updateLocalTime();
    
    vTaskDelay(5);
  }
}

void RGBTaskThread(void *argument){
  FastLED.addLeds<WS2812, RGB_IO, RGB>(leds, 1);
  leds[0] = CRGB(255,0,0);
  FastLED.setBrightness(128);  //配置为50%最大亮度
  
  for(;;){
    if(ProjectData.blestatus == true){
      leds[0] = CRGB(255,0,0);
      esp_ble_gap_read_rssi(*connectedDeviceAddress.getNative()); //获取连接设备的rssi信号强度

      int rssi_brightness = 120 + ProjectData.ble_rssi * 2;
      if(rssi_brightness <= 10) { rssi_brightness = 10; }
      // Serial.printf("brightness : %d\r\n", rssi_brightness);

      FastLED.setBrightness((uint8_t)rssi_brightness);  //根据信号强度来决定亮度
    }else{
      leds[0] = CRGB(0,255,0);
      FastLED.setBrightness(64);
    }

    FastLED.show();
    vTaskDelay(5);
  }
}

void LEDTaskThread(void *argument){
  uint8_t led_counter = 0;
  uint8_t led_mode = 0;

  pinMode(SYSTEM_LED, OUTPUT);
  digitalWrite(SYSTEM_LED, 0);
  delay(100);
  digitalWrite(SYSTEM_LED, 1);
  delay(100);
  digitalWrite(SYSTEM_LED, 0);

  for(;;){
    led_counter++;

    if(led_counter >= 100){

      led_counter = 0;

      if(led_mode == 0){
        led_mode = 1;
        digitalWrite(SYSTEM_LED, 0);
      }else{
        led_mode = 0;
        digitalWrite(SYSTEM_LED, 1);
      }

    }

    vTaskDelay(5);
  }
}

void KEYTaskThread(void *argument){
  button_init(&KEY1, read_KEY1_GPIO, 0);
  button_attach(&KEY1, LONG_PRESS_START, KEY1_PRESS_Handler);
  button_start(&KEY1);
  for(;;) {
    button_ticks();
    vTaskDelay(5);
  }
}