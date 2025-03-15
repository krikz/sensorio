#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "sensor.h"
#include "communication.h"
#include <esp_now.h>

// Глобальные переменные
Sensor *sensor;
bool isAPMode = false;
SensorData receivedData[10];
WebServer server(80);

// Функция регистрации устройств
void registerDevice(uint8_t *mac)
{
  static uint8_t nextID = 0;
  if (nextID >= 10)
  {
    Serial.println("Maximum number of devices reached");
    return;
  }

  uint8_t assignedID = nextID++;
  Serial.printf("Assigned ID %d to device with MAC: ", assignedID);
  for (int i = 0; i < 6; i++)
  {
    Serial.print(mac[i], HEX);
    if (i < 5)
      Serial.print(":");
  }
  Serial.println();

  esp_now_send(mac, &assignedID, sizeof(assignedID));
}

// Callback для приема данных через ESP-NOW
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  if (len == sizeof(SensorData))
  {
    SensorData data;
    memcpy(&data, incomingData, sizeof(SensorData));

    if (data.id == 0xFF)
    {
      registerDevice((uint8_t *)mac);
    }
    else if (data.id < 10)
    {
      receivedData[data.id] = data;
      Serial.printf("Data received from device ID %d\n", data.id);
    }
  }
}

// Task для чтения данных с датчика
void sensorTask(void *parameter)
{
  while (true)
  {
    SensorData data = sensor->read();
    if (isAPMode)
    {
      receivedData[0] = data;
    }
    else
    {
      data.id = 0xFF; // По умолчанию ID не назначен
      sendData(data); // Отправка данных через ESP-NOW
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Задержка 100 мс
  }
}

// Task для обработки HTTP-запросов
void httpTask(void *parameter)
{
  while (true)
  {
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(10)); // Задержка 10 мс
  }
}

// Настройка HTTP-сервера
void setupHTTPServer()
{
  server.on("/", []()
            {
        String response = "{";
        for (int i = 0; i < 10; i++) {
            if (receivedData[i].id != 0xFF) {
                response += "\"d_" + String(i) + "\":{";
                response += "\"a_x\":" + String(receivedData[i].accel_x) + ",";
                response += "\"a_y\":" + String(receivedData[i].accel_y) + ",";
                response += "\"a_z\":" + String(receivedData[i].accel_z) + ",";
                response += "\"g_x\":" + String(receivedData[i].gyro_x) + ",";
                response += "\"g_y\":" + String(receivedData[i].gyro_y) + ",";
                response += "\"g_z\":" + String(receivedData[i].gyro_z);
                response += "},";
            }
        }
        if (response.endsWith(",")) {
            response.remove(response.length() - 1);
        }
        response += "}";
        server.send(200, "application/json", response); });
  server.begin();
  Serial.println("HTTP server started");
}

void setup()
{
  Serial.begin(115200);
  delay(5000);
// Выбор датчика (реальный или тестовый)
#ifdef USE_MOCK_SENSOR
  sensor = new MockSensor();
#else
  sensor = new LSM6DS3Sensor();
#endif

  if (!sensor->begin())
  {
    Serial.println("Failed to initialize sensor");
    while (1)
    {
    }
  }

  // Настройка Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin("ESP32_AP", "12345678");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_AP", "12345678");
    isAPMode = true;
  }

  // Настройка ESP-NOW
  if (!setupESPNOW())
  {
    Serial.println("Failed to setup ESP-NOW");
    // return;
  }

  // Настройка HTTP-сервера
  if (isAPMode)
  {
    setupHTTPServer();
  }

  // Создание задач FreeRTOS
  xTaskCreate(sensorTask, "SensorTask", 2048, NULL, 1, NULL);
  if (isAPMode)
  {
    xTaskCreate(httpTask, "HTTPTask", 2048, NULL, 1, NULL);
  }
}

void loop()
{
  // Пустой цикл, так как все задачи выполняются в потоках FreeRTOS
}