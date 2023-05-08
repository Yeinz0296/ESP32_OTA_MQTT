// void checkPulse(int pulsePin) {
//   unsigned long IN_Pulse = pulseInLong(pulsePin, LOW);
//   if ((IN_Pulse > minwidth_low) && (IN_Pulse < maxwidth_low)) {
//     Serial.println("PULSE: " + String(IN_Pulse));
//     cash++;
//     lastMillis = millis();
//   }
// }
void checkPulse(int pulsePin) {
  unsigned long IN_Pulse = digitalRead(pulsePin);
  // Serial.println("BUTTON: " + String(IN_Pulse));


  if (IN_Pulse == 0) {
    Serial.println("PULSE: " + String(IN_Pulse));
    cash++;
    lastMillis = millis();
  }
}

void outputPulse(int outPulseCounter, int outPulseRate) {
  // outPulseCounter = outPulseCounter / outPulseValue;
  for (int i = 0; i < outPulseCounter; i++) {
    digitalWrite(outPulse, HIGH);
    delay(outPulseRate);
    digitalWrite(outPulse, LOW);
    delay(outPulseRate);
  }
}

void InitialNebulaMQTT() {
  String dataInJson = "{";
  dataInJson += "\"MAC\": \"" + String(WiFi.macAddress()) + "\",";
  dataInJson += "\"lastKnowIP\": \"" + String(WiFi.localIP().toString()) + "\",";
  dataInJson += "\"name\":\"" + String(DEVICENAME) + "\",";
  dataInJson += "\"version\":\"" + String(VERSION) + "\",";
  dataInJson += "\"update\":" + String(update_available ? "true" : "false");
  dataInJson += "}";

  Serial.println("Data to Publish: " + dataInJson);
  Serial.println("Length of Data: " + String(dataInJson.length()));
  Serial.println("Publish to: " + String(MQTT_PREFIX_TOPIC) + String(MQTT_STATUS_INITAL_TOPIC));

  mqtt.publish(String(MQTT_PREFIX_TOPIC) + String(MQTT_STATUS_INITAL_TOPIC), dataInJson);
}
