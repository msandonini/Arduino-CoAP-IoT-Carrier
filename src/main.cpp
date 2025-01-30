#include <Arduino.h>

#include <WiFiNINA.h>
#include <coap-simple.h>
#include <ArduinoJson.h>

#include "CarrierManager.h"
#include "arduino_secrets.h"


#define SERIAL_BAUD_RATE 9600

#define SETUP_DELAY_MS 5000

#define LOOP_CARRIER_UPDATE_MS 10000

#define WIFI_DELAY_FIRMWARE_NOT_UPDATED 500
#define WIFI_RETRY_LOOPS_LIMIT 5
#define WIFI_RETRY_DELAY_MS 1000
#define WIFI_RETRY_LOOP_TIMEOUT_MS 60000
#define UDP_COAP_PORT 5683

#define COAP_TEMP_RESOURCE_NAME "temperature"
#define COAP_HMDT_RESOURCE_NAME "humidity"
#define COAP_PRSS_RESOURCE_NAME "pressure"
#define COAP_ACCL_RESOURCE_NAME "accelerometer"
#define COAP_GYRO_RESOURCE_NAME "gyroscope"

#define CORE_TEMP_TITLE "temperature-sensor"
#define CORE_TEMP_RT "iot.mkriotcarrier.sensor.env.temperature"
#define CORE_TEMP_IF "core.s"
#define CORE_TEMP_CT COAP_APPLICATION_JSON

#define CORE_HMDT_TITLE "humidity-sensor"
#define CORE_HMDT_RT "iot.mkriotcarrier.sensor.env.humidity"
#define CORE_HMDT_IF "core.s"
#define CORE_HMDT_CT COAP_APPLICATION_JSON

#define CORE_PRSS_TITLE "pressure-sensor"
#define CORE_PRSS_RT "iot.mkriotcarrier.sensor.pressure"
#define CORE_PRSS_IF "core.s"
#define CORE_PRSS_CT COAP_APPLICATION_JSON

#define CORE_ACCL_TITLE "accel-sensor"
#define CORE_ACCL_RT "iot.mkriotcarrier.sensor.accelerometer"
#define CORE_ACCL_IF "core.s"
#define CORE_ACCL_CT COAP_APPLICATION_JSON

#define CORE_GYRO_TITLE "gyro-sensor"
#define CORE_GYRO_RT "iot.mkriotcarrier.sensor.gyroscope"
#define CORE_GYRO_IF "core.s"
#define CORE_GYRO_CT COAP_APPLICATION_JSON

#define SENML_BN "mkriotcarrier:rack:env"
#define SENML_BVER 1.0

#define SENML_N_TEMPERATURE "temperature"
#define SENML_U_TEMPERATURE "Cel"

#define SENML_N_HUMIDITY "humidity"
#define SENML_U_HUMIDITY "%RH"

#define SENML_N_PRESSURE "pressure"
#define SENML_U_PRESSURE "Pa"

#define SENML_N_ACCELEROMETER_X "accel:x"
#define SENML_N_ACCELEROMETER_Y "accel:y"
#define SENML_N_ACCELEROMETER_Z "accel:z"

#define SENML_N_GYROSCOPE_X "gyro:x"
#define SENML_N_GYROSCOPE_Y "gyro:y"
#define SENML_N_GYROSCOPE_Z "gyro:z"

#define CORE_IF "core.s"


CarrierManager carrier;

WiFiUDP udp;
Coap coap(udp);


int wifiOldStatus;
unsigned long wifiStatusUpdateTime;


// void callback_wkc(CoapPacket &packet, IPAddress ip, int port);
void callback_temp(CoapPacket &packet, IPAddress ip, int port);
void callback_hmdt(CoapPacket &packet, IPAddress ip, int port);
void callback_accl(CoapPacket &packet, IPAddress ip, int port);
void callback_gyro(CoapPacket &packet, IPAddress ip, int port);
void callback_prss(CoapPacket &packet, IPAddress ip, int port);


void setup() {
    delay(SETUP_DELAY_MS);

    carrier.enableEnvironmentSensorUpdates();
    carrier.enableAccelerometerSensorUpdates();
    carrier.enableGyroscopeSensorUpdates();
    carrier.enablePressureSensorUpdates();
    carrier.setSensorsUpdateTimeout(LOOP_CARRIER_UPDATE_MS);
    carrier.setCase(false);

    carrier.begin();

    carrier.setMessage("Connecting...");

    WiFi.begin(SECRET_SSID, SECRET_PASS);

    udp.begin(UDP_COAP_PORT);
    
    // coap.server(callback_wkc, COAP_DISCOVERY_RESOURCE_NAME); // Does not work, message gets too long
    coap.server(callback_temp, COAP_TEMP_RESOURCE_NAME);
    coap.server(callback_hmdt, COAP_HMDT_RESOURCE_NAME);
    coap.server(callback_prss, COAP_PRSS_RESOURCE_NAME);
    coap.server(callback_accl, COAP_ACCL_RESOURCE_NAME);
    coap.server(callback_gyro, COAP_GYRO_RESOURCE_NAME);

    coap.start();
    
    if (WiFi.status() == WL_CONNECTED) {
        carrier.setMessage(WiFi.localIP().toString());
    }

    wifiStatusUpdateTime = millis();
}

void loop() {
    carrier.loop();
    
    if (WiFi.status() == WL_CONNECTED) {
        coap.loop();
    } else if ((WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_CONNECTION_LOST || WiFi.status() == WL_CONNECT_FAILED) &&
                millis() > wifiStatusUpdateTime + WIFI_RETRY_LOOP_TIMEOUT_MS) {
        WiFi.begin(SECRET_SSID, SECRET_PASS);
        wifiStatusUpdateTime = millis();
    }

    if (WiFi.status() != wifiOldStatus) {
        wifiOldStatus = WiFi.status();

        carrier.setMessage(wifiOldStatus == WL_CONNECTED ? WiFi.localIP().toString() + " : " + UDP_COAP_PORT : "Connecting...");
    }
}


/*
void callback_wkc(CoapPacket &packet, IPAddress ip, int port) {
    String temperatureLinkFormat = "</" + String(COAP_TEMP_RESOURCE_NAME) + 
        ">;ct=\"" + CORE_TEMP_CT + 
        "\";if=\"" + CORE_TEMP_IF + 
        "\";rt=\"" + CORE_TEMP_RT +
        "\";title=\"" + CORE_TEMP_TITLE + "\"";
    
    String humidityLinkFormat = "</" + String(COAP_HMDT_RESOURCE_NAME) + 
        ">;ct=\"" + CORE_HMDT_CT + 
        "\";if=\"" + CORE_HMDT_IF + 
        "\";rt=\"" + CORE_HMDT_RT +
        "\";title=\"" + CORE_HMDT_TITLE + "\"";

    String message = temperatureLinkFormat + "," + humidityLinkFormat;

    coap.sendResponse(ip, port, packet.messageid,
        message.c_str(), message.length(),
        COAP_RESPONSE_CODE(COAP_CONTENT), COAP_CONTENT_TYPE(CORE_DISCOVERY_CT),
        packet.token, packet.tokenlen);
}
*/

void callback_temp(CoapPacket &packet, IPAddress ip, int port) {
    JsonDocument doc;
    
    JsonObject jsonBase = doc.add<JsonObject>();
    jsonBase["bn"] = SENML_BN;
    jsonBase["bt"] = millis();
    jsonBase["bver"] = SENML_BVER;

    JsonObject jsonTemp = doc.add<JsonObject>();
    jsonTemp["n"] = SENML_N_TEMPERATURE;
    jsonTemp["v"] = carrier.getEnvironmentSensor().temperature;
    jsonTemp["u"] = SENML_U_TEMPERATURE;

    doc.shrinkToFit();

    String message;
    serializeJson(doc, message);

    coap.sendResponse(ip, port, packet.messageid, 
        message.c_str(), message.length(), 
        COAP_RESPONSE_CODE(COAP_CONTENT), COAP_CONTENT_TYPE(CORE_TEMP_CT), 
        packet.token, packet.tokenlen);

    //coap.sendResponse(ip, port, packet.messageid, message.c_str());
}

void callback_hmdt(CoapPacket &packet, IPAddress ip, int port) {
    JsonDocument doc;

    JsonObject jsonBase = doc.add<JsonObject>();
    jsonBase["bn"] = SENML_BN;
    jsonBase["bt"] = millis();
    jsonBase["bver"] = SENML_BVER;

    JsonObject jsonHmdt = doc.add<JsonObject>();
    jsonHmdt["n"] = SENML_N_HUMIDITY;
    jsonHmdt["v"] = carrier.getEnvironmentSensor().humidity;
    jsonHmdt["u"] = SENML_U_HUMIDITY;

    doc.shrinkToFit();

    String message;
    serializeJson(doc, message);

    coap.sendResponse(ip, port, packet.messageid, 
        message.c_str(), message.length(), 
        COAP_RESPONSE_CODE(COAP_CONTENT), COAP_CONTENT_TYPE(CORE_HMDT_CT), 
        packet.token, packet.tokenlen);
}

void callback_accl(CoapPacket &packet, IPAddress ip, int port) {
    JsonDocument doc;

    JsonObject jsonBase = doc.add<JsonObject>();
    jsonBase["bn"] = SENML_BN;
    jsonBase["bt"] = millis();
    jsonBase["bver"] = SENML_BVER;

    JsonObject jsonAcclX = doc.add<JsonObject>();
    jsonAcclX["n"] = SENML_N_GYROSCOPE_X;
    jsonAcclX["v"] = carrier.getIMUSensor().gyroscope.x;

    JsonObject jsonAcclY = doc.add<JsonObject>();
    jsonAcclY["n"] = SENML_N_GYROSCOPE_Y;
    jsonAcclY["v"] = carrier.getIMUSensor().gyroscope.y;

    JsonObject jsonAcclZ = doc.add<JsonObject>();
    jsonAcclZ["n"] = SENML_N_GYROSCOPE_Z;
    jsonAcclZ["v"] = carrier.getIMUSensor().gyroscope.z;

    doc.shrinkToFit();

    String message;
    serializeJson(doc, message);

    coap.sendResponse(ip, port, packet.messageid, 
        message.c_str(), message.length(), 
        COAP_RESPONSE_CODE(COAP_CONTENT), COAP_CONTENT_TYPE(CORE_ACCL_CT), 
        packet.token, packet.tokenlen);
}

void callback_gyro(CoapPacket &packet, IPAddress ip, int port) {
    JsonDocument doc;

    JsonObject jsonBase = doc.add<JsonObject>();
    jsonBase["bn"] = SENML_BN;
    jsonBase["bt"] = millis();
    jsonBase["bver"] = SENML_BVER;

    JsonObject jsonGyroX = doc.add<JsonObject>();
    jsonGyroX["n"] = SENML_N_GYROSCOPE_X;
    jsonGyroX["v"] = carrier.getIMUSensor().gyroscope.x;

    JsonObject jsonGyroY = doc.add<JsonObject>();
    jsonGyroY["n"] = SENML_N_GYROSCOPE_Y;
    jsonGyroY["v"] = carrier.getIMUSensor().gyroscope.y;

    JsonObject jsonGyroZ = doc.add<JsonObject>();
    jsonGyroZ["n"] = SENML_N_GYROSCOPE_Z;
    jsonGyroZ["v"] = carrier.getIMUSensor().gyroscope.z;

    doc.shrinkToFit();

    String message;
    serializeJson(doc, message);

    coap.sendResponse(ip, port, packet.messageid, 
        message.c_str(), message.length(), 
        COAP_RESPONSE_CODE(COAP_CONTENT), COAP_CONTENT_TYPE(CORE_GYRO_CT), 
        packet.token, packet.tokenlen);
}

void callback_prss(CoapPacket &packet, IPAddress ip, int port) {
    JsonDocument doc;

    JsonObject jsonBase = doc.add<JsonObject>();
    jsonBase["bn"] = SENML_BN;
    jsonBase["bt"] = millis();
    jsonBase["bver"] = SENML_BVER;

    JsonObject jsonPrss = doc.add<JsonObject>();
    jsonPrss["n"] = SENML_N_PRESSURE;
    jsonPrss["v"] = carrier.getEnvironmentSensor().humidity;
    jsonPrss["u"] = SENML_U_PRESSURE;

    doc.shrinkToFit();

    String message;
    serializeJson(doc, message);

    coap.sendResponse(ip, port, packet.messageid, 
        message.c_str(), message.length(), 
        COAP_RESPONSE_CODE(COAP_CONTENT), COAP_CONTENT_TYPE(CORE_PRSS_CT), 
        packet.token, packet.tokenlen);
}
