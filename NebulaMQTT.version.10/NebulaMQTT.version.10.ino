#define DEVICENAME "NebulaMQTT TEST DEVICE"
#define VERSIONMAJOR 1
#define VERSIONMINOR 0
#define VERSIONPATCH 0

#include <MQTT.h>
#include <WiFi.h>
#include <Update.h>
#include <nvs_flash.h>
#include <WebServer.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "variables.h"
#include "WiFi_MQTT.h"
#include "functionsOnly.h"
#include "OTA_Update.h"
#include "initial_setup.h"

TaskHandle_t wirelessTask;

void wireless_setup(void *pvParameters) {
  if (digitalRead(buttonSetting)) {
    Serial.print("ButtonSetting: ");
    Serial.println(digitalRead(buttonSetting));
    settingsMode = true;
    initialSetup();
  }

  if (digitalRead(buttonReset)) {
    Serial.print("ButtonReset: ");
    Serial.println(digitalRead(buttonReset));
    settingsReset = true;
    nvs_flash_erase();
    nvs_flash_init();
  }

  if (!settingsMode) {
    Serial.println("SINI");
    connectToWiFi();
    connectToMqttBroker();
  }

  if (WiFi.status() == WL_CONNECTED && mqtt.connected()) {
    InitialNebulaMQTT();
  }

  for (;;) {
    server.handleClient();
    mqtt.loop();
    delay(10);  // <- fixes some issues with WiFi stability

    if (!settingsMode) {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wifi Xok 1");
        connectToWiFi();
      }

      if (!mqtt.connected()) {
        Serial.println("MQTT Xok 1");
        connectToMqttBroker();
      }
    }

    if (settingsMode && wifiCredential) {
      Serial.println(settingsMode);
      Serial.println(wifiCredential);
      Serial.println("Wifi Xok 2");
      connectToWiFi();
      Serial.println("MQTT Xok 2");
      connectToMqttBroker();
      InitialNebulaMQTT();
      ESP.restart();
      //updateFirmware(updateFirmwareInitialLink.c_str());
    }

    if ((update_available || ask_request) && WiFi.status() && mqtt.connected()) {
      if (update_available && update_link.length() > 0) {
        updateFirmware(update_link.c_str());
        Serial.println("Link: " + update_link);
      }
    }



    if (WiFi.status() && mqtt.connected()) {
      //If there transcation, it will send to T05/MACADDRESS/payment/cash
      if (total) {
        if (millis() - lastMillis > 4000) {
          lastMillis = millis();

          outPulseCounter = total;
          Serial.println("OUTPulse: " + String(outPulseCounter));
          outputPulse(outPulseCounter, outPulseCounterRate);

          String dataInJson = "{";
          dataInJson += "\"MAC\": \"" + String(WiFi.macAddress()) + "\",";
          dataInJson += "\"cash\":" + String(cash / outPulseValue) + ",";
          dataInJson += "\"type\":" + String(cash ? "\"bill\"" : "\"coin\"") + "";
          dataInJson += "}";

          Serial.println("Data to Publish: " + dataInJson);
          Serial.println("Length of Data: " + String(dataInJson.length()));
          Serial.println("Publish to: " + String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_PAYMENT_TOPIC));

          mqtt.publish(String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_PAYMENT_TOPIC), dataInJson);
          outPulseCounter = 0;
          cash = 0;
          T05Pay = 0;
          total = 0;
        }
      }

      //Sent online status to T05/MACADDRESS/status/online every 10 seconds
      if (millis() - lastMillis > 10000) {
        lastMillis = millis();

        String dataInJson = "{";
        dataInJson += "\"MAC\": \"" + String(WiFi.macAddress()) + "\"";
        dataInJson += "}";

        Serial.println("Data to Publish: " + dataInJson);
        Serial.println("Length of Data: " + String(dataInJson.length()));
        Serial.println("Publish to: " + String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_STATUS_ONLINE_TOPIC));

        mqtt.publish(String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_STATUS_ONLINE_TOPIC), dataInJson);
      }
    }
  }
}


void setup() {
  Serial.begin(115200);

  Serial.println(String(DEVICENAME) + " Version: " + String(VERSION));

  pinMode(pulse1, INPUT);
  pinMode(buttonSetting, INPUT);
  pinMode(buttonReset, INPUT);
  pinMode(outPulse, OUTPUT);


  xTaskCreatePinnedToCore(wireless_setup, "wireless", 10000, NULL, 1, &wirelessTask, 0);
}

void loop() {

  checkPulse(pulse1);

  total = +cash / outPulseValue + T05Pay / 100;

  // Serial.println("CASH: " + String(cash));
  // Serial.println("T05Pay: " + String(T05Pay));
  Serial.println("Total: " + String(total));

  // if (millis() - lastMillis > 5000) {
  //   if (total) {
  //   }
  // }

  delay(10);
}
