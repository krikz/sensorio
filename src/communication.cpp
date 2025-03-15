#include "communication.h"
#include <esp_now.h>
#include <WiFi.h>

// Широковещательный адрес для отправки данных
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Флаг для отслеживания состояния ESP-NOW
bool espNowInitialized = false;

// Инициализация ESP-NOW
bool setupESPNOW()
{
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return false;
    }

    // Регистрация callback для приема данных
    esp_now_register_recv_cb(onDataRecv);

    // Добавление широковещательного адреса для отправки данных
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return false;
    }

    espNowInitialized = true;
    Serial.println("ESP-NOW initialized successfully");
    return true;
}

// Отправка данных через ESP-NOW
void sendData(const SensorData &data)
{
    if (!espNowInitialized)
    {
        Serial.println("ESP-NOW is not initialized");
        return;
    }

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&data, sizeof(data));
    if (result == ESP_OK)
    {
        Serial.println("Data sent successfully");
    }
    else
    {
        Serial.println("Error sending data");
    }
}