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

#define SENML_BN "mkriotcarrier:rack:env"
#define SENML_BVER 1.0
#define SENML_N_TEMPERATURE "temperature"
#define SENML_U_TEMPERATURE "Cel"
#define SENML_N_HUMIDITY "humidity"
#define SENML_U_HUMIDITY "%RH"

//TODO: Implement CoRE Link Format
#define CORE_RT "environment-sensor"
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

    carrier.begin();

    String firmware = WiFi.firmwareVersion();
    if (firmware < WIFI_FIRMWARE_LATEST_VERSION) {
        carrier.setTopText("Update WiFi firmware");
        delay(WIFI_DELAY_FIRMWARE_NOT_UPDATED);
    }

    //TODO: Move connection to loop() with if statement (WiFi.status() != WL_CONNECTED) and remove retry loop
    carrier.setTopText("Connecting...");
    
    for (int i = 0; WiFi.begin(SECRET_SSID, SECRET_PASS) != WL_CONNECTED && i < WIFI_RETRY_LOOPS_LIMIT; i ++) {
        delay(WIFI_RETRY_DELAY_MS);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        carrier.setTopText(WiFi.localIP().toString());

        udp.begin(UDP_COAP_PORT);
        
        coap.server(callback_temp, COAP_TEMP_RESOURCE_NAME);
        coap.server(callback_hmdt, COAP_HMDT_RESOURCE_NAME);

        coap.start();
    } else {
        carrier.setTopText("Not connected");
    }

    carrier.loop();
    startTime = millis();
}

void loop() {
    if (millis() > startTime + LOOP_CARRIER_UPDATE_MS) {
        carrier.loop();
        startTime = millis();
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        coap.loop();
    }
}


// TODO: Implement
void callback_wkc(CoapPacket &packet, IPAddress ip, int port) {

}

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
        COAP_RESPONSE_CODE(COAP_CONTENT), COAP_CONTENT_TYPE(COAP_APPLICATION_JSON), 
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
        COAP_RESPONSE_CODE(COAP_CONTENT), COAP_CONTENT_TYPE(COAP_APPLICATION_JSON), 
        packet.token, packet.tokenlen);
}
