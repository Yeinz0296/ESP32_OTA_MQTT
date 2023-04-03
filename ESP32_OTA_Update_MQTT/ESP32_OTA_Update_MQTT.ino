#include <HTTPClient.h>
#include <MQTT.h>
#include <Update.h>
#include <WiFi.h>
//#include <WiFiClient.h>

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define MQTT_HOST "YOUR_BROKER_HOST"
#define MQTT_PREFIX_TOPIC "YOUR_PHONE_NUMBER/NebulaMQTT"
#define MQTT_PAYMENT_TOPIC "/payment"                    //PUB
#define MQTT_STATUS_TOPIC "/status"                      //PUB
#define MQTT_PULSES_TOPIC "/pulses"                      //SUB
#define MQTT_UPDATE_AVAILABLE_TOPIC "/update/available"  //SUB
#define MQTT_UPDATE_LINK_TOPIC "/update/link"            //SUB
#define MQTT_UPDATE_LINK_TOPIC "/status/restart"         //SUB
#define MQTT_USERNAME "YOUR_MQTT_USERNAME"
#define MQTT_PASSWORD "YOUR_MQTT_PASSWORD"

WiFiClient net;
MQTTClient mqtt(1024);

bool update_available = false;
String update_link;

TaskHandle_t wirelessTask;

unsigned long lastMillis = 0;

void connectToWiFi() {

  Serial.print("Connecting to Wi-Fi '" + String(WIFI_SSID) + "' ...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(" connected!");
}

void messageReceived(String &topic, String &payload) {

  if (topic == "T05/NebulaMQTT/update/available" && payload == "true") {
    update_available = true;
  }

  if (topic == "T05/NebulaMQTT/status/restart" && payload == "true") {
    Serial.println("Restarting");
    ESP.restart();
  }

  if (update_available == true && topic == "T05/NebulaMQTT/update/link") {
    update_link = payload;
  }

  if (topic == "T05/NebulaMQTT/update/available" && payload == "true") {
    update_available = true;
  }

  if (topic == "T05/NebulaMQTT/pulses") {
    Serial.println("Pulses: " + String(payload));
  }
}

void connectToMqttBroker() {

  Serial.print("Connecting to '" + String(MQTT_HOST) + "' ...");

  mqtt.begin(MQTT_HOST, net);
  mqtt.onMessage(messageReceived);

  String uniqueString = String(WIFI_SSID) + "-" + String(random(1, 98)) + String(random(99, 999));
  char uniqueClientID[uniqueString.length() + 1];

  uniqueString.toCharArray(uniqueClientID, uniqueString.length() + 1);

  while (!mqtt.connect(uniqueClientID)) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(" connected!");

  bool MQTT_update_available = mqtt.subscribe(String(MQTT_PREFIX_TOPIC) + String(MQTT_UPDATE_AVAILABLE_TOPIC));
  if (MQTT_update_available) {
    Serial.println("Subscribe to: " + String(MQTT_PREFIX_TOPIC) + String(MQTT_UPDATE_AVAILABLE_TOPIC));
  }

  bool MQTT_update_link = mqtt.subscribe(String(MQTT_PREFIX_TOPIC) + String(MQTT_UPDATE_LINK_TOPIC));
  if (MQTT_update_link) {
    Serial.println("Subscribe to: " + String(MQTT_PREFIX_TOPIC) + String(MQTT_UPDATE_LINK_TOPIC));
  }

  bool MQTT_pulses = mqtt.subscribe(String(MQTT_PREFIX_TOPIC) + String(MQTT_PULSES_TOPIC));
  if (MQTT_pulses) {
    Serial.println("Subscribe to: " + String(MQTT_PREFIX_TOPIC) + String(MQTT_PULSES_TOPIC));
  }
}

bool performUpdate(Stream &updateSource, size_t updateSize) {
  Serial.println("Performing Update!");
  if (Update.begin(updateSize)) {
    size_t written = Update.writeStream(updateSource);
    if (written == updateSize) {
      Serial.println("Written : " + String(written) + " successfully");
    } else {
      Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
    }
    if (Update.end()) {
      Serial.println("OTA done!");
      if (Update.isFinished()) {
        Serial.println("Update successfully completed. Rebooting.");
        return true;
      } else {
        Serial.println("Update not finished? Something went wrong!");
      }
    } else {
      Serial.println("Error Occurred. Error #: " + String(Update.getError()));
    }
  } else {
    Serial.println("Not enough space to begin OTA");
  }
  return false;
}

void updateFirmware(const char *firmwareUrl) {
  HTTPClient http;
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.begin(firmwareUrl);

  int httpCode = http.GET();
  if (httpCode == 200) {
    size_t contentLength = http.getSize();
    WiFiClient *stream = http.getStreamPtr();

    bool updated = performUpdate(*stream, contentLength);
    if (updated) {
      ESP.restart();
    }
  } else if (httpCode == 301) {
    String redirectedUrl = http.getLocation();
    updateFirmware(redirectedUrl.c_str());
  } else {
    Serial.println("HTTP error: " + String(httpCode));
    update_available = false;
  }

  http.end();
}

void wireless_setup(void *pvParameters) {
  connectToWiFi();        // <-- Function
  connectToMqttBroker();  // <-- Function

  if (WiFi.status() == WL_CONNECTED && mqtt.connected()) {
    String dataInJson = "{";
    dataInJson += "\"id\": \"" + String(WiFi.macAddress()) + "\",";
    dataInJson += "\"status\": \"Device Online\"";
    dataInJson += "}";

    Serial.println("Data to Publish: " + dataInJson);
    Serial.println("Length of Data: " + String(dataInJson.length()));
    Serial.println("Publish to: " + String(MQTT_PREFIX_TOPIC) + String(MQTT_STATUS_TOPIC));  // <- only for debug purpose, when production, better to remove this

    mqtt.publish(String(MQTT_PREFIX_TOPIC) + String(MQTT_STATUS_TOPIC), dataInJson);
  }

  for (;;) {
    mqtt.loop();
    delay(10);  // <- fixes some issues with WiFi stability

    if (WiFi.status() != WL_CONNECTED) {
      connectToWiFi();
    }

    if (!mqtt.connected()) {
      connectToMqttBroker();
    }

    if (WiFi.status() && mqtt.connected()) {
      if (millis() - lastMillis > 5000) {
        lastMillis = millis();

        String dataInJson = "{";
        dataInJson += "\"id\": \"" + String(WiFi.macAddress()) + "\",";
        dataInJson += "\"update\":" + String(update_available);
        dataInJson += "}";

        Serial.println("Data to Publish: " + dataInJson);
        Serial.println("Length of Data: " + String(dataInJson.length()));
        Serial.println("Publish to: " + String(MQTT_PREFIX_TOPIC) + String(MQTT_PAYMENT_TOPIC));
        mqtt.publish(String(MQTT_PREFIX_TOPIC) + String(MQTT_PAYMENT_TOPIC), dataInJson);
      }

      if (update_available && update_link.length() > 0) {
        updateFirmware(update_link.c_str());
        Serial.println("Link: " + update_link);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nNebulaMQTT");

  xTaskCreatePinnedToCore(wireless_setup, "wireless", 10000, NULL, 1, &wirelessTask, 0);

  pinMode(2, OUTPUT);
  
}

void loop() {
  digitalWrite(2, LOW);
}