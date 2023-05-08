const char* apSSID = "MyESP32AP";
const char* apPassword = "MyPassword";

WebServer server(80);

void handleRoot() {
  server.send(200, "text/html",
              "<html><body><h1>Enter your shop's Wi-Fi credentials</h1>"
              "<form method='post' action='/save'>"
              "<label for='ssid'>SSID:</label><br>"
              "<input type='text' id='ssid' name='ssid'><br>"
              "<label for='password'>Password:</label><br>"
              "<input type='password' id='password' name='password'><br><br>"
              "<label for='ssid'>Output Pulse Value:</label><br>"
              "<input type='text' id='outputPulse' name='outputPulse'><br>"
              "<label for='ssid'>Pulse Rate:</label><br>"
              "<input type='text' id='pulseRate' name='pulseRate'><br>"
              "<input type='submit' value='Submit'>"
              "</form></body></html>");
}

void handleSave() {
  preferences.begin("credentials", false);
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  String outPulseValueNew =server.arg("outputPulse");

  // Save the Wi-Fi credentials to preference

  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.putString("outPulseValueServer", outPulseValueNew);
  preferences.end();
  wifiCredential = true;
  server.send(200, "text/plain", "Wi-Fi credentials saved. Please reconnect to your shop's Wi-Fi network.");
}

void initialSetup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID, apPassword);

  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();

  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(apIP);
}
