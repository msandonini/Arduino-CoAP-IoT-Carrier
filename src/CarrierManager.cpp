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

void CarrierManager::setTopText(String text) {
    this->carrier.display.setFont(&FreeSans9pt7b);
    this->carrier.display.fillRect(0, 0, 250, 46, 0x0000); // Display clear
    this->carrier.display.setCursor(55, 40);
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
unsigned long CarrierManager::SENSORS_UPDATE_TIMEOUT_MS = 1000;

// GFX

void CarrierManager::gfxInit() {
    this->carrier.display.setRotation(0);
}

void CarrierManager::gfxDrawEnvironment() {
    // Screen cleaning
    this->carrier.display.fillRect(30, 50, 61, 61, 0x0000);
    this->carrier.display.fillRect(30, 130, 61, 61, 0x0000);

    // Thermometer icon
    this->carrier.display.fillCircle(60, 60, 10, 0x07E0);
    this->carrier.display.fillRect(50, 60, 21, 28, 0x07E0);
    this->carrier.display.fillCircle(60, 95, 15, 0x07E0);
    this->carrier.display.fillCircle(60, 60, 7, 0x0000);
    this->carrier.display.fillRect(53, 63, 15, 25, 0x0000);
    this->carrier.display.fillCircle(60, 95, 12, 0x0000);

    // Droplet icon
    this->carrier.display.fillTriangle(42, 160, 60, 130, 78, 160, 0x07FF);
    this->carrier.display.fillCircle(60, 170, 20, 0x07FF);
    this->carrier.display.fillTriangle(45, 160, 60, 135, 75, 160, 0x0000);
    this->carrier.display.fillCircle(60, 170, 17, 0x0000);
}

void CarrierManager::gfxDrawIMU() {
    // Screen cleaning
    this->carrier.display.fillRect(30, 50, 61, 61, 0x0000);
    this->carrier.display.fillRect(30, 130, 61, 61, 0x0000);

    // Movement icon
    this->carrier.display.fillTriangle(50, 65, 60, 50, 70, 65, 0x001F);
    this->carrier.display.fillTriangle(55, 62, 60, 55, 65, 62, 0x0000);
    this->carrier.display.fillTriangle(30, 80, 45, 70, 45, 90, 0x001F);
    this->carrier.display.fillTriangle(35, 80, 42, 75, 42, 85, 0x0000);
    this->carrier.display.fillTriangle(50, 95, 70, 95, 60, 110, 0x001F);
    this->carrier.display.fillTriangle(55, 98, 65, 98, 60, 105, 0x0000);
    this->carrier.display.fillTriangle(75, 90, 75, 70, 90, 80, 0x001F);
    this->carrier.display.fillTriangle(78, 85, 78, 75, 85, 80, 0x0000);
    this->carrier.display.fillRect(45, 75, 30, 10, 0x001F);
    this->carrier.display.fillRect(55, 65, 10, 30, 0x001F);
    this->carrier.display.fillRect(46, 78, 29, 4, 0x0000);
    this->carrier.display.fillRect(58, 66, 4, 29, 0x0000);

    // Rotation icon
    this->carrier.display.fillCircle(65, 160, 25, 0xF800);
    this->carrier.display.fillCircle(55, 160, 25, 0xF800);
    this->carrier.display.fillCircle(65, 160, 22, 0x0000);
    this->carrier.display.fillCircle(55, 160, 22, 0x0000);
    this->carrier.display.fillCircle(65, 160, 18, 0xF800);
    this->carrier.display.fillCircle(55, 160, 18, 0xF800);
    this->carrier.display.fillCircle(65, 160, 15, 0x0000);
    this->carrier.display.fillCircle(55, 160, 15, 0x0000);
    this->carrier.display.fillTriangle(55, 160, 30, 130, 55, 130, 0x0000);
    this->carrier.display.fillTriangle(65, 160, 65, 190, 90, 190, 0x0000);
    this->carrier.display.fillRect(55, 130, 10, 60, 0x0000);
    this->carrier.display.fillTriangle(50, 140, 65, 130, 65, 150, 0xF800);
    this->carrier.display.fillTriangle(55, 190, 55, 170, 70, 180, 0xF800);
    this->carrier.display.fillTriangle(55, 140, 62, 135, 62, 145, 0x0000);
    this->carrier.display.fillTriangle(58, 185, 58, 175, 65, 180, 0x0000);
    this->carrier.display.fillCircle(60, 160, 5, 0xF800);
    this->carrier.display.fillCircle(60, 160, 2, 0x0000);
}

void CarrierManager::gfxUpdate() {
    
    //this->gfxDrawEnvironment();
    if (this->selectedFunction == 0) {
        if (this->lastLoopFunction != this->selectedFunction) {
            this->gfxDrawEnvironment();
        }

        this->carrier.display.setFont(&FreeSans18pt7b);
        // TEMPERATURE TEXT
        this->carrier.display.fillRect(95, 50, 150, 61, 0x0000); // Display clear
        this->carrier.display.setCursor(95, 95);
        this->carrier.display.setTextColor(0xFFFF);
        this->carrier.display.print(String(this->environment.temperature) + " C");

        // HUMIDITY TEXT
        this->carrier.display.fillRect(95, 130, 150, 61, 0x0000); // Display clear
        this->carrier.display.setCursor(95, 175);
        this->carrier.display.setTextColor(0xFFFF);
        this->carrier.display.print(String(this->environment.humidity) + " %");
    } else if (this->selectedFunction == 1) {
        if (this->lastLoopFunction != this->selectedFunction) {
            this->gfxDrawIMU();
        }
        this->carrier.display.setFont(&FreeSans9pt7b);

        // ACCELEROMETER
        this->carrier.display.fillRect(95, 50, 150, 61, 0x0000); // Display clear
        if (this->imu.accelerometer.enabled) {
            this->carrier.display.setTextColor(0xFFFF);
            this->carrier.display.setCursor(95, 70);
            this->carrier.display.print("X: " + String(this->imu.accelerometer.x));
            this->carrier.display.setCursor(95, 90);
            this->carrier.display.print("Y: " + String(this->imu.accelerometer.y));
            this->carrier.display.setCursor(95, 110);
            this->carrier.display.print("Z: " + String(this->imu.accelerometer.z));
        }

        // GYROSCOPE
        this->carrier.display.fillRect(95, 130, 150, 61, 0x0000); // Display clear
        if (this->imu.gyroscope.enabled) {
            this->carrier.display.setTextColor(0xFFFF);
            this->carrier.display.setCursor(95, 150);
            this->carrier.display.print("X: " + String(this->imu.gyroscope.x));
            this->carrier.display.setCursor(95, 170);
            this->carrier.display.print("Y: " + String(this->imu.gyroscope.y));
            this->carrier.display.setCursor(95, 190);
            this->carrier.display.print("Z: " + String(this->imu.gyroscope.z));
        }
    }
}

// BUTTONS

void CarrierManager::buttonsInit() {
}

void CarrierManager::buttonsUpdate() {
    this->carrier.Buttons.update();

    this->lastLoopFunction = this->selectedFunction;

    if (this->carrier.Buttons.onTouchDown(TOUCH0)) {
        this->carrier.Buzzer.beep();
        this->selectedFunction = 0;
    } else if (this->carrier.Buttons.onTouchDown(TOUCH1)) {
        this->carrier.Buzzer.beep();
        this->selectedFunction = 1;
    } else if (this->carrier.Buttons.onTouchDown(TOUCH2)) {
        this->carrier.Buzzer.beep();
        this->selectedFunction = 2;
    } else if (this->carrier.Buttons.onTouchDown(TOUCH3)) {
        this->carrier.Buzzer.beep();
        this->selectedFunction = 3;
    } else if (this->carrier.Buttons.onTouchDown(TOUCH4)) {
        this->carrier.Buzzer.beep();
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