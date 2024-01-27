#include <Arduino.h>
#include <Preferences.h>
#include "../inc/PinDefinitionsAndMore.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <IRremote.hpp>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "DNSServer.h"
#include "ESPmDNS.h"
#include "../inc/globalDefines.h"
#include "../inc/tasks.h"
#include "../inc/webServer.h"
#include "../inc/wifi.h"


char msg[256];
const char* const AP_SSID = "Buttmote";
const char* const AP_PASS = NULL; 
String wifiSSID = "SECRET";
String wifiPass = "SECRET";
String bulbIP = "192.168.137.93";

bool remoteEnabled = true;
float bufferMinutes;
bool sitting = false;
float minutesSat = 0;
float minutesStood = 0;
int lightState = 3;
bool irBool;
bool connected = false;

WiFiUDP udp;
Preferences prefs;
DNSServer dnsServer;
static AsyncWebServer server(80);

const int udpPort = 38899;
int brightness, brightness1, brightness2, brightness3;

bool receivedFlag;
bool lightOn = false;

struct storedIRDataStruct {
  IRData receivedIRData;
  uint8_t rawCode[RAW_BUFFER_LENGTH];
  uint8_t rawCodeLength;
} sStoredIRData, dataFromStorage;

// Semaphores
SemaphoreHandle_t udpSemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t irSendBinarySemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t irReceiveBinarySemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t lightStateOffSemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t lightStateOnSemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t lightStateStandDimming = xSemaphoreCreateBinary();

void IrSendTask(void * pvParameters){
  IrSender.begin();
  while(1){
    if(xSemaphoreTake(irSendBinarySemaphore, portMAX_DELAY) == pdTRUE){

      //Flush serial register to ensure code is sent correctly
      Serial.flush();
      if (sStoredIRData.receivedIRData.protocol == UNKNOWN) {
        IrSender.sendRaw(sStoredIRData.rawCode, sStoredIRData.rawCodeLength, 38);
      } else {
        IrSender.write(&sStoredIRData.receivedIRData);
      }
    }
    vTaskDelay(IR_SEND_TASK_DELAY);
  }
}

void IrReceiveTask(void * pvParameters){
  while(1){
    if(xSemaphoreTake(irReceiveBinarySemaphore, portMAX_DELAY) == pdTRUE){
      WiFi.disconnect(true);
      server.end();
      irBool = true;
      IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

      while(receivedFlag == false){
        if(IrReceiver.decode()){
          WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
          server.begin();
          irBool = false;
          receivedFlag = true;
          sStoredIRData.receivedIRData = IrReceiver.decodedIRData;
          uint32_t dataToStorage = sStoredIRData.receivedIRData.decodedRawData;

          //Filter non valid data and continue receiving
          if(IrReceiver.decodedIRData.decodedRawData == 0){
            receivedFlag = false;
          }
          if (receivedFlag){
            prefs.putBytes("data", &sStoredIRData, sizeof(sStoredIRData));
          }
          IrReceiver.resume();
        }
        vTaskDelay(1);
      }
    }
    IrReceiver.stop();
    vTaskDelay(IR_RECEIVE_TASK_DELAY);
  }
}

void IrReceiveButtonTask(void * pvParameters){
  while(1){
    if(!digitalRead(IR_RECEIVE_BUTTON_PIN)){
      receivedFlag = false;
      xSemaphoreGive(irReceiveBinarySemaphore);
      vTaskDelay(500);
    }
    vTaskDelay(IR_RECEIVE_TRIGGER_TASK_DELAY);
  }
}

void TaskLightOn(void *pvParameters){
  Serial.println("TASK CREATED: LightOn\n");
  for (;;){
    if(xSemaphoreTake(lightStateOnSemaphore, portMAX_DELAY) == pdTRUE){
      if(lightState == 1){
        brightness = brightness1;
      } else if(lightState == 2){
        brightness = brightness2;
      } else if(lightState == 3){
        brightness = brightness3;
      }
      if(brightness == 0){
        snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", 10, OFF);
      } else {
        snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", brightness, ON);
      }

      xSemaphoreGive(udpSemaphore);
    }
    vTaskDelay(TASK_DELAY);
  }
}

void TaskLightOff(void *pvParameters){
  Serial.println("TASK CREATED: LightOff\n");
  for (;;){
    if(xSemaphoreTake(lightStateOffSemaphore, portMAX_DELAY) == pdTRUE){
      snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", brightness, OFF);
      xSemaphoreGive(udpSemaphore);
    }
    vTaskDelay(TASK_DELAY);
  }
}

void TaskLightBrightnessDOWN(void *pvParameters){
  Serial.println("TASK CREATED: BrightnessDOWN\n");
  for(;;){
    if(!digitalRead(BTN_BRIGHTNESS_DOWN)){
      if(lightState == 1){
        if(brightness1 >= 10){
          brightness1 -= 10;
          brightness = brightness1;
          snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", brightness, ON);
        } else {
          snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", 10, OFF);
        }
      } else if(lightState == 2){
        if(brightness2 >= 10){
          brightness2 -= 10;
          brightness = brightness2;
          snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", brightness, ON);
        }else {
          snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", 10, OFF);
        }
      } else if(lightState == 3){
        if(brightness3 >= 10){
          brightness3 -= 10;
          brightness = brightness3;
          snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", brightness, ON);
        }else {
          snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", 10, OFF);
        }
      }
      prefs.putBytes("brightness1", &brightness1, sizeof(brightness1));
      prefs.putBytes("brightness2", &brightness2, sizeof(brightness2));
      prefs.putBytes("brightness3", &brightness3, sizeof(brightness3));
      Serial.println(brightness);
      Serial.println(msg);
      xSemaphoreGive(udpSemaphore);
    }
    vTaskDelay(TASK_DELAY);
  }
}

void TaskLightBrightnessUP(void *pvParameters){
  Serial.println("TASK CREATED: brightnessUP\n");
  for(;;){
    if(!digitalRead(BTN_BRIGHTNESS_UP)){
      if(lightState == 1){
        if(brightness1 <= 90){
          brightness1 += 10;
          brightness = brightness1;
          snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", brightness, ON);
        }
      } else if(lightState == 2){
        if(brightness2 <= 90){
          brightness2 += 10;
          brightness = brightness2;
          snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", brightness, ON);
        }
      } else if(lightState == 3){
        if(brightness3 <= 90){
          brightness3 += 10;
          brightness = brightness3;
          snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", brightness, ON);
        }
      }
      prefs.putBytes("brightness1", &brightness1, sizeof(brightness1));
      prefs.putBytes("brightness2", &brightness2, sizeof(brightness2));
      prefs.putBytes("brightness3", &brightness3, sizeof(brightness3));
      xSemaphoreGive(udpSemaphore);
    }
    vTaskDelay(TASK_DELAY);
  }
}

void TaskStandUpDimming(void *pvParameters){
  Serial.println("TASK CREATED: StandUpDimming\n");
  for(;;){
    if(xSemaphoreTake(lightStateStandDimming, portMAX_DELAY) == pdTRUE){
      snprintf(msg, sizeof(msg), "{\"id\": 1, \"method\": \"setPilot\", \"params\": {\"dimming\": %d, \"state\": %s}}", DIMMING_VALUE, ON);
      xSemaphoreGive(udpSemaphore);
    }
    vTaskDelay(TASK_DELAY);
  }
}

void ControlLedsTask(void * pvParameters){
  // Turn on green LED to indicate power on
  digitalWrite(GREEN_LED_PIN, HIGH);
  while(1) {
    if(irBool) {
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, HIGH);
    }
    // Turn on white LWD if the Wifi is connected
    // Turn on red LED if the Wifi isn't connected
    else if (WiFi.status() == WL_CONNECTED) {
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, HIGH);
      digitalWrite(BLUE_LED_PIN, HIGH);
    } else {
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, LOW);
    }
    vTaskDelay(TASK_DELAY);
  }
}

void sittingLogicTask(void * pvParameters){
  bool sittingTest = false;
  bool timerAlreadyTriggered = true;
  bool timerNotYetTriggered = false;
  long int timeWhenStartedStanding;

  while(1){
    if(timerNotYetTriggered && !digitalRead(BUTT_SENSOR_PIN) && !sittingTest){
      xSemaphoreGive(lightStateOffSemaphore);
      Serial.println("Sitting");
      sittingTest = true; 
    }
    if(!timerNotYetTriggered && !digitalRead(BUTT_SENSOR_PIN) && !sittingTest){
      lightState = 1;
      Serial.println("Sitting"); 
      sittingTest = true;
      timerAlreadyTriggered = false; 
      xSemaphoreGive(lightStateOnSemaphore);

      xSemaphoreGive(irSendBinarySemaphore);
    }
    // Check if user stood up
    if (digitalRead(BUTT_SENSOR_PIN) && sittingTest){
      lightState = 2;
      Serial.println("Standing");
      sittingTest = false;
      timerNotYetTriggered = true;
      timeWhenStartedStanding = xTaskGetTickCount();
      xSemaphoreGive(lightStateOnSemaphore);
    } // Check if buffer time has elapsed since standing
    else if (!sittingTest && !timerAlreadyTriggered && digitalRead(BUTT_SENSOR_PIN) && ((xTaskGetTickCount() - timeWhenStartedStanding) > (bufferMinutes * 60000))){
      lightState = 3;
      Serial.print("Timer triggered. Time: ");
      Serial.println(xTaskGetTickCount() - timeWhenStartedStanding);
      timerAlreadyTriggered = true;
      timerNotYetTriggered = false;
      xSemaphoreGive(lightStateOnSemaphore);
      xSemaphoreGive(irSendBinarySemaphore);
    }
    sitting = sittingTest;
    vTaskDelay(TASK_DELAY);
  }
}

void TaskUDP(void *pvParameters){
  Serial.println("TASK CREATED: UDP\n");
  for(;;){
    if(xSemaphoreTake(udpSemaphore, portMAX_DELAY) == pdTRUE){
      if(connected){
        udp.beginPacket(bulbIP.c_str(), udpPort);
        udp.printf(msg);
        udp.endPacket();
        Serial.println("UDP packet sent\n");
      }
    }
    vTaskDelay(TASK_DELAY);
  }
}
