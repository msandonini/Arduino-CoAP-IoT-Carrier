#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#include "CarrierManager.h"

#include "CarrierGfxDrawFunctions.h"


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

unsigned long CarrierManager::setSensorsUpdateTimeout(unsigned long timeout) {
    CarrierManager::SENSORS_UPDATE_TIMEOUT_MS = timeout;
}

// ---------------
// CONSTRUCTORS & DESTRUCTORS
// ---------------

CarrierManager::CarrierManager() {
}

CarrierManager::~CarrierManager() {
    CarrierManager::CASE = 0;
    CarrierManager::PIR = 0;
    CarrierManager::SENSORS_UPDATE_TIMEOUT_MS = 1000;
}


// ---------------
// PUBLIC METHODS
// ---------------

void CarrierManager::begin() {
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

    this->carrier.begin();

    this->closeRelays();
    
    this->gfxInit();
    this->buttonsInit();
    this->ledsInit();
    this->sensorsInit();

    this->sensorsUpdate();
    this->lastLoopFunction = -1;
    this->selectedFunction = 0;
    this->lastSensorsUpdateDrawn = false;
    this->lastSensorsUpdateMs = millis();
    this->gfxUpdate();
}
void CarrierManager::loop() {
    if (millis() > this->lastSensorsUpdateMs + CarrierManager::SENSORS_UPDATE_TIMEOUT_MS) {
        this->sensorsUpdate();
        this->lastSensorsUpdateDrawn = false;
        this->lastSensorsUpdateMs = millis();
    }
    this->buttonsUpdate();
    this->ledsUpdate();
    if (!this->lastSensorsUpdateDrawn || this->lastLoopFunction != this->selectedFunction) {
        this->gfxUpdate();
        this->lastSensorsUpdateDrawn = true;
    }
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

void CarrierManager::setMessage(String msg){
    this->message = msg;
}

String CarrierManager::getMessage() {
    return this->message;
}

// ---------------
// PRIVATE STATIC ATTRIBUTES
// ---------------

int CarrierManager::CASE = 0;
int CarrierManager::PIR = 0;
unsigned long CarrierManager::SENSORS_UPDATE_TIMEOUT_MS = 1000;

// GFX

void CarrierManager::gfxInit() {
    this->carrier.display.setRotation(0);
}


void CarrierManager::gfxUpdate() {
    cleanDisplay(this->carrier.display);
    switch (this->selectedFunction) {
        case 0:

            // TEMPERATURE
            drawThermometerIcon(this->carrier.display, 0x07E0, 45, 50);

            drawMessage(this->carrier.display, &FreeSans18pt7b, 100, 95, 0xFFFF, String((int) this->environment.temperature) + " C");

            // HUMIDITY
            drawDropletIcon(this->carrier.display, 0x07FF, 40, 130);

            drawMessage(this->carrier.display, &FreeSans18pt7b, 100, 175, 0xFFFF, String((int) this->environment.humidity) + " %");
        
            break;
        
        case 1:
            // ACCELEROMETER

            drawMovementIcon(this->carrier.display, 0x001F, 30, 50);

            drawMessage(this->carrier.display, &FreeSans9pt7b, 100, 70, 0xFFFF, "X: " + String(this->imu.accelerometer.x));
            drawMessage(this->carrier.display, &FreeSans9pt7b, 100, 90, 0xFFFF, "Y: " + String(this->imu.accelerometer.y));
            drawMessage(this->carrier.display, &FreeSans9pt7b, 100, 110, 0xFFFF, "Z: " + String(this->imu.accelerometer.z));
            
            // GYROSCOPE
            
            drawRotationIcon(this->carrier.display, 0xF800, 30, 130);

            drawMessage(this->carrier.display, &FreeSans9pt7b, 100, 150, 0xFFFF, "X: " + String(this->imu.gyroscope.x));
            drawMessage(this->carrier.display, &FreeSans9pt7b, 100, 170, 0xFFFF, "Y: " + String(this->imu.gyroscope.y));
            drawMessage(this->carrier.display, &FreeSans9pt7b, 100, 190, 0xFFFF, "Z: " + String(this->imu.gyroscope.z));

            break;
        case 2:
            // PRESSURE

            drawPressureIcon(this->carrier.display, 0xFFE0, 30, 85);

            drawMessage(this->carrier.display, &FreeSans12pt7b, 100, 120, 0xFFFF, String(this->pressure.pressure) + " kPa");

            break;
        case 4:
            // MESSAGE
            drawMessage(this->carrier.display, &FreeSans12pt7b, 20, 120, 0xFFFF, this->message);
            break;
    }
}

// BUTTONS

void CarrierManager::buttonsInit() {
}

void CarrierManager::buttonsUpdate() {
    this->carrier.Buttons.update();

    this->lastLoopFunction = this->selectedFunction;

    if (this->carrier.Buttons.onTouchDown(TOUCH0)) {
        //this->carrier.Buzzer.beep();
        this->selectedFunction = 0;
    } else if (this->carrier.Buttons.onTouchDown(TOUCH1)) {
        //this->carrier.Buzzer.beep();
        this->selectedFunction = 1;
    } else if (this->carrier.Buttons.onTouchDown(TOUCH2)) {
        //this->carrier.Buzzer.beep();
        this->selectedFunction = 2;
    } else if (this->carrier.Buttons.onTouchDown(TOUCH3)) {
        //this->carrier.Buzzer.beep();
        this->selectedFunction = 3;
    } else if (this->carrier.Buttons.onTouchDown(TOUCH4)) {
        //this->carrier.Buzzer.beep();
        this->selectedFunction = 4;
    }
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
    this->carrier.leds.clear();
    this->carrier.leds.setPixelColor(0, 0, 0, 0);
    this->carrier.leds.setPixelColor(1, 0, 0, 0);
    this->carrier.leds.setPixelColor(2, 0, 0, 0);
    this->carrier.leds.setPixelColor(3, 0, 0, 0);
    this->carrier.leds.setPixelColor(4, 0, 0, 0);
    this->carrier.leds.setPixelColor(this->selectedFunction, 0, 0, 255);
    this->carrier.leds.show();
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


// DRAW METHODS
void drawMovementIcon(Adafruit_ST7789& display) {

}

void drawRotationIcon(Adafruit_ST7789& display) {
    
}