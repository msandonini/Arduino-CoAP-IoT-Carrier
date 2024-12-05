#include <Arduino.h>

#include <WiFiNINA.h>
#include <coap-simple.h>
#include <ArduinoJson.h>

#include "CarrierManager.h"
#include "arduino_secrets.h"


#define SERIAL_BAUD_RATE 9600

#define SETUP_DELAY_MS 5000

#define LOOP_CARRIER_UPDATE_MS 10000

#define WIFI_DELAY_FIRMWARE_NOT_UPDATED 5000
#define WIFI_RETRY_LOOPS_LIMIT 5
#define WIFI_RETRY_DELAY_MS 1000
#define UDP_COAP_PORT 5683

#define COAP_TEMP_RESOURCE_NAME "temperature"
#define COAP_HMDT_RESOURCE_NAME "humidity"

#define CORE_TEMP_TITLE "temperature-actuator"
#define CORE_TEMP_RT "iot.mkriotcarrier.sensor.env.temperature"
#define CORE_TEMP_IF "core.s"
#define CORE_TEMP_CT COAP_APPLICATION_JSON

#define CORE_HMDT_TITLE "humidity-actuator"
#define CORE_HMDT_RT "iot.mkriotcarrier.sensor.env.humidity"
#define CORE_HMDT_IF "core.s"
#define CORE_HMDT_CT COAP_APPLICATION_JSON

#define SENML_BN "mkriotcarrier:rack:env"
#define SENML_BVER 1.0

#define SENML_N_TEMPERATURE "temperature"
#define SENML_U_TEMPERATURE "Cel"

#define SENML_N_HUMIDITY "humidity"
#define SENML_U_HUMIDITY "%RH"

#define CORE_IF "core.s"


CarrierManager carrier;

WiFiUDP udp;
Coap coap(udp);

unsigned long startTime;


void callback_wkc(CoapPacket &packet, IPAddress ip, int port);
void callback_temp(CoapPacket &packet, IPAddress ip, int port);
void callback_hmdt(CoapPacket &packet, IPAddress ip, int port);


void setup() {
    delay(SETUP_DELAY_MS);

    carrier.enableEnvironmentSensorUpdates();
    //carrier.enableAccelerometerSensorUpdates();
    //carrier.enableGyroscopeSensorUpdates();

    carrier.begin();

    String firmware = WiFi.firmwareVersion();
    if (firmware < WIFI_FIRMWARE_LATEST_VERSION) {
        carrier.setTopText("Update WiFi firmware");
        delay(WIFI_DELAY_FIRMWARE_NOT_UPDATED);
    }

    carrier.setTopText("Connecting...");

    WiFi.begin(SECRET_SSID, SECRET_PASS);

    udp.begin(UDP_COAP_PORT);
    
    // coap.server(callback_wkc, COAP_DISCOVERY_RESOURCE_NAME); // Does not work, message gets too long
    coap.server(callback_temp, COAP_TEMP_RESOURCE_NAME);
    coap.server(callback_hmdt, COAP_HMDT_RESOURCE_NAME);

    coap.start();
    
    if (WiFi.status() == WL_CONNECTED) {
        carrier.setTopText(WiFi.localIP().toString());
    } else {
        carrier.setTopText("Not connected");
    }
}

int wifiOldStatus;

void loop() {
    carrier.loop();
    
    if (WiFi.status() == WL_CONNECTED) {
        coap.loop();
    } else {
        WiFi.begin(SECRET_SSID, SECRET_PASS);
    }

    if (WiFi.status() != wifiOldStatus) {
        wifiOldStatus = WiFi.status();

        carrier.setTopText(wifiOldStatus == WL_CONNECTED ? WiFi.localIP().toString() : "Not connected");
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
