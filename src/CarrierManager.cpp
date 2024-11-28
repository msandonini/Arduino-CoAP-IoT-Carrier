#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#include "CarrierManager.h"

// ---------------
// STATIC METHODS
// ---------------

int CarrierManager::setCase(bool useCase) {
    if (CarrierManager::CASE > -1) {
        CarrierManager::CASE = (useCase ? 1 : 0);
    }

    return CarrierManager::CASE;
}

int CarrierManager::setPIR(bool usePIR) {
    if (CarrierManager::PIR > -1) {
        CarrierManager::PIR = (usePIR ? 1 : 0);
    }

    return CarrierManager::PIR;
}


// ---------------
// CONSTRUCTORS & DESTRUCTORS
// ---------------

CarrierManager::CarrierManager() {
    if (CarrierManager::CASE > 0) {
        this->carrier.withCase();
    } else {
        this->carrier.noCase();
    }

    CarrierManager::CASE = -1;

    if (CarrierManager::PIR > 0) {
        pinMode((this->carrier.getBoardRevision() == 1 ? A5 : A0), INPUT);
    }

    CarrierManager::PIR = -1;
}

CarrierManager::~CarrierManager() {
    CarrierManager::CASE = 0;
    CarrierManager::PIR = 0;
}


// ---------------
// PUBLIC METHODS
// ---------------

void CarrierManager::begin() {
    this->carrier.begin();

    this->closeRelays();
    
    this->gfxInit();
    this->ledsInit();
    this->buttonsInit();
    this->sensorsInit();
}
void CarrierManager::loop() {
    this->sensorsUpdate();
    this->buttonsUpdate();
    this->ledsUpdate();
    this->gfxUpdate();
}

CarrierManager::HTS221_EnvironmentSensors CarrierManager::getEnvironmentSensor() {
    return this->environment;
}
CarrierManager::LPS22HB_PressureSensor CarrierManager::getPressureSensor() {
    return this->pressure;
}
CarrierManager::LSM6DS3_IMUSensor CarrierManager::getIMUSensor() {
    return this->imu;
}
CarrierManager::APDS9960_LightSensor CarrierManager::getLightSensor() {
    return this->light;
}

void CarrierManager::setTopText(String text) {
    this->carrier.display.setFont(&FreeSans9pt7b);
    this->carrier.display.fillRect(0, 0, 250, 55, 0x0000); // Display clear
    this->carrier.display.setCursor(55, 45);
    this->carrier.display.setTextColor(0xFFFF);
    this->carrier.display.print(text.c_str());
}

void CarrierManager::enableEnvironmentSensorUpdates(bool enable){
    this->environment.enabled = enable;
}
void CarrierManager::enablePressureSensorUpdates(bool enable){
    this->pressure.enabled = enable;
}
void CarrierManager::enableGyroscopeSensorUpdates(bool enable){
    this->imu.gyroscope.enabled = enable;
}
void CarrierManager::enableAccelerometerSensorUpdates(bool enable){
    this->imu.accelerometer.enabled = enable;
}
void CarrierManager::enableRGBSensorUpdates(bool enable){
    this->light.rgb.enabled = enable;
}
void CarrierManager::enableGestureSensorUpdates(bool enable){
    this->light.gesture.enabled = enable;
}

// ---------------
// PRIVATE STATIC ATTRIBUTES
// ---------------

int CarrierManager::CASE = 0;
int CarrierManager::PIR = 0;

// GFX

void CarrierManager::gfxInit() {
    this->carrier.display.setRotation(0);
}

void CarrierManager::gfxUpdate() {
    this->carrier.display.setFont(&FreeSans18pt7b);
    
    // TEMPERATURE ICON
    this->carrier.display.fillCircle(60, 70, 10, 0x07E0);
    this->carrier.display.fillRect(50, 70, 21, 28, 0x07E0);
    this->carrier.display.fillCircle(60, 105, 15, 0x07E0);
    this->carrier.display.fillCircle(60, 70, 7, 0x0000);
    this->carrier.display.fillRect(53, 73, 15, 25, 0x0000);
    this->carrier.display.fillCircle(60, 105, 12, 0x0000);

    // DROPLET ICON
    this->carrier.display.fillTriangle(42, 170, 60, 140, 78, 170, 0x07FF);
    this->carrier.display.fillCircle(60, 180, 20, 0x07FF);
    this->carrier.display.fillTriangle(45, 170, 60, 145, 75, 170, 0x0000);
    this->carrier.display.fillCircle(60, 180, 17, 0x0000);

    // TEMPERATURE TEXT
    this->carrier.display.fillRect(90, 60, 150, 60, 0x0000); // Display clear
    this->carrier.display.setCursor(95, 105);
    this->carrier.display.setTextColor(0xFFFF);
    this->carrier.display.print(String(this->environment.temperature) + " C");

    // HUMIDITY TEXT
    this->carrier.display.fillRect(90, 140, 150, 60, 0x0000); // Display clear
    this->carrier.display.setCursor(95, 185);
    this->carrier.display.setTextColor(0xFFFF);
    this->carrier.display.print(String(this->environment.humidity) + " %");
}

// BUTTONS

void CarrierManager::buttonsInit() {
    //this->carrier.Buttons.begin();
}

void CarrierManager::buttonsUpdate() {
    this->carrier.Buttons.update();

    this->buttonsState.btn0 = this->carrier.Buttons.getTouch(TOUCH0);
    this->buttonsState.btn1 = this->carrier.Buttons.getTouch(TOUCH1);
    this->buttonsState.btn2 = this->carrier.Buttons.getTouch(TOUCH2);
    this->buttonsState.btn3 = this->carrier.Buttons.getTouch(TOUCH3);
    this->buttonsState.btn4 = this->carrier.Buttons.getTouch(TOUCH4);
}

// LEDS

void CarrierManager::ledsInit() {
    this->carrier.leds.setBrightness(10);
    this->carrier.leds.setPixelColor(0, 255, 0, 0);
    this->carrier.leds.setPixelColor(1, 255, 255, 0);
    this->carrier.leds.setPixelColor(2, 0, 255, 0);
    this->carrier.leds.setPixelColor(3, 0, 255, 255);
    this->carrier.leds.setPixelColor(4, 0, 0, 255);
    this->carrier.leds.show();
}

void CarrierManager::ledsUpdate() {
}

// MEASUREMENTS

void CarrierManager::sensorsInit() {
    this->carrier.Env.begin();
    this->carrier.Pressure.begin();
    this->carrier.IMUmodule.begin();
    this->carrier.Light.begin();
}

void CarrierManager::sensorsUpdate() {
    if (this->environment.enabled) {
        this->environment.temperature = this->carrier.Env.readTemperature();
        this->environment.humidity = this->carrier.Env.readHumidity();
    }

    if (this->pressure.enabled) {
        this->pressure.pressure = this->carrier.Pressure.readPressure();
    }

    if (this->imu.accelerometer.enabled && this->carrier.IMUmodule.accelerationAvailable()) {
        this->carrier.IMUmodule.readAcceleration(this->imu.accelerometer.x, this->imu.accelerometer.y, this->imu.accelerometer.z);
    }
    if (this->imu.gyroscope.enabled && this->carrier.IMUmodule.gyroscopeAvailable()) {
        this->carrier.IMUmodule.readGyroscope(this->imu.gyroscope.x, this->imu.gyroscope.y, this->imu.gyroscope.z);
    }
    
    if (this->light.rgb.enabled && this->carrier.Light.colorAvailable()) {
        this->carrier.Light.readColor(this->light.rgb.r, this->light.rgb.g, this->light.rgb.b);
    }
    if (this->light.gesture.enabled && this->carrier.Light.gestureAvailable()) {
        this->light.gesture.gesture = this->carrier.Light.readGesture();
    }
}

// RELAYS

void CarrierManager::closeRelays() {
    this->carrier.Relay1.close();
    this->carrier.Relay2.close();
}