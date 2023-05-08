bool performUpdate(Stream &updateSource, size_t updateSize) {
  
  Serial.println("Performing Update!");
  mqtt.publish(String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_STATUS_TOPIC), "Performing Update!");
  if (Update.begin(updateSize)) {
    size_t written = Update.writeStream(updateSource);
    if (written == updateSize) {
      Serial.println("Written : " + String(written) + " successfully");
      mqtt.publish(String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_STATUS_TOPIC), "successfully");
    } else {
      Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      mqtt.publish(String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_STATUS_TOPIC), "Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
    }
    if (Update.end()) {
      Serial.println("OTA done!");
      if (Update.isFinished()) {
        Serial.println("Update successfully completed. Rebooting.");
        mqtt.publish(String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_STATUS_TOPIC), "Update successfully completed. Rebooting.");
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
