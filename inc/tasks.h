#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>

void IrSendTask(void *pvParameters);
void IrReceiveTask(void *pvParameters);
void IrReceiveButtonTask(void *pvParameters);

void TaskLightOn(void *pvParameters);
void TaskLightOff(void *pvParameters);
void TaskLightBrightnessUP(void *pvParameters);
void TaskLightBrightnessDOWN(void *pvParameters);
void TaskStandUpDimming(void *pvParameters);

void sittingLogicTask(void * pvParameters);
void ControlLedsTask(void *pvParameters);

void TaskUDP(void *pvParameters);

#endif