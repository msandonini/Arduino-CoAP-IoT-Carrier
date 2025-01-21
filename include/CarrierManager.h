#pragma once


#include <Arduino.h>

#include <Arduino_MKRIoTCarrier.h>


class CarrierManager {
public:
    struct HTS221_EnvironmentSensors {
        float temperature;
        float humidity;

        bool enabled;
    };

    struct LPS22HB_PressureSensor {
        float pressure;

        bool enabled;
    };

    struct LSM6DS3_GyroscopeSensor {
        float x, y, z;

        bool enabled;
    };

    struct LSM6DS3_AccelerometerSensor {
        float x, y, z;

        bool enabled;
    };

    struct LSM6DS3_IMUSensor {
        LSM6DS3_GyroscopeSensor gyroscope;
        LSM6DS3_AccelerometerSensor accelerometer;
    };

    struct APDS9960_RGBSensor {
        int r, g, b;

        bool enabled;
    };

    struct APDS9960_GestureSensor {
        int gesture;

        bool enabled;
    };

    struct APDS9960_LightSensor {
        APDS9960_RGBSensor rgb;
        APDS9960_GestureSensor gesture;
    };

    static int setCase(bool useCase);
    static int setPIR(bool usePIR);
    static unsigned long setSensorsUpdateTimeout(unsigned long timeout);


    CarrierManager();
    ~CarrierManager();


    void begin();
    void loop();

    HTS221_EnvironmentSensors getEnvironmentSensor();
    LPS22HB_PressureSensor getPressureSensor();
    LSM6DS3_IMUSensor getIMUSensor();
    APDS9960_LightSensor getLightSensor();
    
    Adafruit_ST7789& getDisplay();


    void enableEnvironmentSensorUpdates(bool enable = true);
    void enablePressureSensorUpdates(bool enable = true);
    void enableGyroscopeSensorUpdates(bool enable = true);
    void enableAccelerometerSensorUpdates(bool enable = true);
    void enableRGBSensorUpdates(bool enable = true);
    void enableGestureSensorUpdates(bool enable = true);

    void setMessage(String msg);
    String getMessage();
private:
    static int CASE;     // 0 = false, >0 = true, <0 = already started, cannot change
    static int PIR;      // 0 = false, >0 = true, <0 = already started, cannot change
    static unsigned long SENSORS_UPDATE_TIMEOUT_MS;


    MKRIoTCarrier carrier;

    HTS221_EnvironmentSensors environment;
    LPS22HB_PressureSensor pressure;
    LSM6DS3_IMUSensor imu;
    APDS9960_LightSensor light;
    String message;

    int lastLoopFunction;
    int selectedFunction;

    long lastSensorsUpdateMs;
    bool lastSensorsUpdateDrawn;


    void gfxInit();
    void gfxUpdate();

    void buttonsInit();
    void buttonsUpdate();

    void ledsInit();
    void ledsUpdate();

    void sensorsInit();
    void sensorsUpdate();

    void closeRelays();
};
