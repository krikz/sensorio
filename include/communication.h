#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>
#include "sensor.h"

// Инициализация ESP-NOW
bool setupESPNOW();

// Отправка данных через ESP-NOW
void sendData(const SensorData& data);

// Callback для приема данных через ESP-NOW
extern void onDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len);

#endif