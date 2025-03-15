#include "sensor.h"
#include <Wire.h>

status_t LSM6DS3Sensor::begin() {
    return imu.begin(); // Инициализация через I2C
}

SensorData LSM6DS3Sensor::read() {
    SensorData data;
    data.accel_x = imu.readFloatAccelX();
    data.accel_y = imu.readFloatAccelY();
    data.accel_z = imu.readFloatAccelZ();
    data.gyro_x = imu.readFloatAccelX();
    data.gyro_y = imu.readFloatAccelY();
    data.gyro_z = imu.readFloatAccelZ();
    data.time = millis();
    return data;
}

SensorData MockSensor::read() {
    static float mockValue = 0.0;
    mockValue += 0.1;

    SensorData data;
    data.accel_x = mockValue;
    data.accel_y = mockValue + 0.1;
    data.accel_z = mockValue + 0.2;
    data.gyro_x = mockValue + 0.3;
    data.gyro_y = mockValue + 0.4;
    data.gyro_z = mockValue + 0.5;
    return data;
}