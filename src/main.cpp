#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "sensor.h"
#include "communication.h"
#include <esp_now.h>
#include <Hashtable.h>
// Функция хеширования для MAC-адресов
struct MacHash
{
  uint32_t operator()(const uint8_t *mac) const
  {
    uint32_t hash = 5381; // Базовое значение
    for (int i = 0; i < 6; i++)
    {
      hash = ((hash << 5) + hash) + mac[i]; // hash * 33 + mac[i]
    }
    return hash;
  }
};

// Определяем хеш-таблицу для MAC-адресов
Hashtable<const uint8_t *, uint8_t, MacHash> macToIdx;
uint8_t macAP[6];

// Глобальные переменные
Sensor *sensor;
bool isAPMode = false;
SensorData receivedData[10];
WebServer server(80);

// Функция регистрации устройств
void registerDevice(const uint8_t *mac)
{
  static uint8_t nextID = 1;
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

  // Добавляем MAC-адрес в хеш-таблицу
  macToIdx.put(mac, assignedID);
}

// Callback для приема данных через ESP-NOW
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  if (len == sizeof(SensorData))
  {
    SensorData data;
    memcpy(&data, incomingData, sizeof(SensorData));

    // Проверяем, зарегистрирован ли MAC-адрес
    uint8_t *value = macToIdx.get(mac);
    if (value == nullptr)
    {
      // Если MAC-адрес не зарегистрирован, регистрируем устройство
      registerDevice(mac);
    }
    else
    {
      // Если MAC-адрес зарегистрирован, сохраняем данные
      data.id = *value;
      data.time = micros();
      receivedData[*value] = data;
      Serial.printf("Data received from device ID %d\n", *value);
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
      data.id = 0; // Главное устройство имеет ID = 0
      data.dtime = micros();
      data.time = micros();
      receivedData[0] = data;
    }
    else
    {
      sendData(data); // Отправка данных через ESP-NOW
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Задержка 10 мс
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
          if (millis() - receivedData[i].time <= 200) { // Проверяем актуальность данных
              response += "\"d" + String(i) + "\":{";
              response += "\"ax\":" + String(receivedData[i].accel_x) + ",";
              response += "\"ay\":" + String(receivedData[i].accel_y) + ",";
              response += "\"az\":" + String(receivedData[i].accel_z) + ",";
              response += "\"gx\":" + String(receivedData[i].gyro_x) + ",";
              response += "\"gy\":" + String(receivedData[i].gyro_y) + ",";
              response += "\"gz\":" + String(receivedData[i].gyro_z) + ",";
              response += "\"dt\":" + String(receivedData[i].dtime) + ",";
              response += "\"t\":" + String(receivedData[i].time);
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
  status_t status = sensor->begin();
  if (status != IMU_SUCCESS)
  {
    Serial.println("Failed to initialize sensor");
    Serial.println(status);
    while (1)
    {
    }
  }

  // Настройка Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin("SENSORIO", "12345678");
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
    WiFi.softAP("SENSORIO", "12345678");
    isAPMode = true;
    WiFi.softAPmacAddress(macAP);
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