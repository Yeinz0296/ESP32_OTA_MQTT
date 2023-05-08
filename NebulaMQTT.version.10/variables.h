const int pulse1 = 18;
const int buttonSetting = 5;
const int buttonReset = 4;

const int outPulse = 23;

const int pulse_width_high = 50000;  //In microsecond = 300ms
const int pulse_width_low = 50000;    //In microsecond = 50ms
const int pulse_rate = 2;             //For 2% error rate

const float minwidth_high = pulse_width_high - (pulse_width_high * pulse_rate / 100);
const float maxwidth_high = pulse_width_high + (pulse_width_high * pulse_rate / 100);

const float minwidth_low = pulse_width_low - (pulse_width_low * pulse_rate / 100);
const float maxwidth_low = pulse_width_low + (pulse_width_low * pulse_rate / 100);

unsigned int cash;
unsigned int T05Pay;
unsigned int total;
unsigned int outPulseCounter;
unsigned int outPulseValue = 5;
unsigned int outPulseCounterRate = 100;

unsigned long lastMillis = 0;

bool update_available = false;
bool ask_request = false;
bool settingsMode = false;
bool settingsReset = false;
bool wifiCredential = false;
bool MQTT_payment_remote = false;
bool MQTT_setting = false;
bool MQTT_update_available = false;
bool MQTT_update_link = false;
bool MQTT_status_restart = false;

String update_link;
String updateFirmwareInitialLink = "https://github.com/Yeinz0296/ESP32_OTA_MQTT/blob/main/NebulaMQTT.version.10.ino.esp32.bin?raw=true";
String VERSION = String(VERSIONMAJOR) + "." + String(VERSIONMINOR) + "." + String(VERSIONPATCH);
