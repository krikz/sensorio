#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <SparkFunLSM6DS3.h>

typedef struct
{
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    uint8_t id;
    ulong time;
} SensorData;

class Sensor
{
public:
    virtual status_t begin() = 0;      // Инициализация датчика
    virtual SensorData read() = 0; // Чтение данных
};

// Реальный датчик LSM6DS3
class LSM6DS3Sensor : public Sensor
{
public:
    status_t begin() override;
    SensorData read() override;

private:
    LSM6DS3 imu = LSM6DS3(I2C_MODE, 0x6B);
};

// Тестовый датчик (mock)
class MockSensor : public Sensor
{
public:
    status_t begin() override { return IMU_SUCCESS; }
    SensorData read() override;
};

#endif