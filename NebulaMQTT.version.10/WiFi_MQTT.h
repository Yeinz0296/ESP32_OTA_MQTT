#define WIFI_SSID "t05"
#define WIFI_PASSWORD "p0llin8ti0n"

#define MQTT_HOST "188.166.222.166"
#define MQTT_USERNAME "nebulamqtt"
#define MQTT_PASSWORD "nebulat05"
#define MQTT_PREFIX_TOPIC "T05/"  //TOPIC
#define MQTT_MAC_TOPIC "246F28B49A5C"

//WAJIB
#define MQTT_STATUS_INITAL_TOPIC "status/initial"  //PUB
#define MQTT_STATUS_ONLINE_TOPIC "/status/online"  //PUB
#define MQTT_PAYMENT_TOPIC "/payment/cash"         //PUB

#define MQTT_PAYMENT_REMOTE_TOPIC "/payment/credit"  //SUB
#define MQTT_SETTING_TOPIC "/setting"                //SUB

//OPTION
#define MQTT_UPDATE_TOPIC "/update"                      //PUB
#define MQTT_STATUS_TOPIC "/status"                      //PUB
#define MQTT_UPDATE_AVAILABLE_TOPIC "/update/available"  //SUB
#define MQTT_UPDATE_LINK_TOPIC "/update/link"            //SUB
#define MQTT_STATUS_RESTART_TOPIC "/status/restart"      //SUB
;
WiFiClient net;
MQTTClient mqtt(1024);
Preferences preferences;

void connectToWiFi() {
  preferences.begin("credentials", true);
  String ssid = preferences.getString("ssid", ssid);
  String password = preferences.getString("password", password);

  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(" connected!");
}

void messageReceived(String& topic, String& payload) {


  String TOPIC = String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC);

  //WAJIB
  if (topic == (String(TOPIC) + String(MQTT_PAYMENT_REMOTE_TOPIC))) {
    // Deserialize the JSON document
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    T05Pay = doc["outputPulse"];
    Serial.println(T05Pay);
    lastMillis = millis();
  }

  if (topic == (String(TOPIC) + String(MQTT_SETTING_TOPIC))) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    outPulseCounterRate = doc["pulseRate"];
    outPulseValue = doc["outputPulseValue"];
  }

  //OPTIONAL
  if (topic == (String(TOPIC) + String(MQTT_UPDATE_AVAILABLE_TOPIC)) && payload == "true") {
    update_available = true;
  }

  if (topic == (String(TOPIC) + String(MQTT_UPDATE_AVAILABLE_TOPIC)) && payload == "false") {
    update_available = false;
  }

  if (update_available == true && topic == (String(TOPIC) + String(MQTT_UPDATE_LINK_TOPIC))) {
    update_link = payload;
  }

  if (topic == (String(TOPIC) + String(MQTT_STATUS_RESTART_TOPIC)) && payload == "true") {
    Serial.println("Restarting");
    ESP.restart();
  }
}

void connectToMqttBroker() {
  preferences.begin("credentials", true);
  String ssid = preferences.getString("ssid", ssid);
  String password = preferences.getString("password", password);
  //preferences.end();

  Serial.print("Connecting to '" + String(MQTT_HOST) + "' ...");

  mqtt.begin(MQTT_HOST, net);
  mqtt.onMessage(messageReceived);

  String uniqueString = String(ssid) + "-" + String(random(1, 98)) + String(random(99, 999));
  char uniqueClientID[uniqueString.length() + 1];

  uniqueString.toCharArray(uniqueClientID, uniqueString.length() + 1);

  while (!mqtt.connect(uniqueClientID, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(" connected!");

  MQTT_payment_remote = mqtt.subscribe(String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_PAYMENT_REMOTE_TOPIC));
  MQTT_setting = mqtt.subscribe(String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_SETTING_TOPIC));
  MQTT_update_available = mqtt.subscribe(String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_UPDATE_AVAILABLE_TOPIC));
  MQTT_update_link = mqtt.subscribe(String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_UPDATE_LINK_TOPIC));
  MQTT_status_restart = mqtt.subscribe(String(MQTT_PREFIX_TOPIC) + String(MQTT_MAC_TOPIC) + String(MQTT_STATUS_RESTART_TOPIC));
}
