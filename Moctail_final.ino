#include <WiFi.h>
#include <WebServer.h>

// Access Point credentials
const char* ap_ssid = "CocktailESP32";
const char* ap_password = "makecocktails";

// Pins
const int pump1 = 16;
const int pump2 = 17;
const int pump3 = 18;
const int pump4 = 12;
const int pump5 = 13;
const int pump6 = 14;
const int mixer = 41;
const int solenoid = 40;

WebServer server(80);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("🍹 Starting ESP32-S3 Cocktail Machine...");

  // Start Access Point
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("📶 Access Point IP address: ");
  Serial.println(IP);

  // Set up pins
  int pins[] = {pump1, pump2, pump3, pump4, pump5, pump6, mixer, solenoid};
  for (int i = 0; i < 8; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], HIGH);  // OFF
  }

  // Web server routes
  server.on("/", handleRoot);
  server.on("/drink", handleDrink);
  server.on("/custom", handleCustom);
  server.begin();

  Serial.println("🌐 Web Server Started!");
}

void serveDrink(int pumps[], int durations[], int count, String drinkName) {
  Serial.println("🍹 Serving: " + drinkName);

  unsigned long startTime = millis();
  bool pumpOn[6] = {false};
  bool pumpOff[6] = {false};

  for (int i = 0; i < count; i++) {
    digitalWrite(pumps[i], LOW);
    pumpOn[i] = true;
  }

  while (true) {
    bool allDone = true;
    unsigned long now = millis();

    for (int i = 0; i < count; i++) {
      if (pumpOn[i] && !pumpOff[i] && now - startTime >= durations[i]) {
        digitalWrite(pumps[i], HIGH);
        pumpOff[i] = true;
      }
      if (pumpOn[i] && !pumpOff[i]) {
        allDone = false;
      }
    }

    if (allDone) break;
    delay(10);
    yield();
  }

  digitalWrite(mixer, LOW);
  delay(5000);
  digitalWrite(mixer, HIGH);
  delay(10);

  digitalWrite(solenoid, LOW);
  delay(16000);
  digitalWrite(solenoid, HIGH);

  Serial.println("✅ Drink Ready!");
  server.send(200, "text/plain", drinkName + " Ready!");
}

void handleDrink() {
  if (server.hasArg("drink")) {
    String drink = server.arg("drink");

    if (drink == "greenBeach") {
      int pumps[] = {pump1, pump2};
      int durations[] = {2000, 2000};
      serveDrink(pumps, durations, 2, "greenBeach");
    } else if (drink == "newYearSunrise") {
      int pumps[] = {pump3, pump4, pump5, pump6};
      int durations[] = {2000, 2000, 222, 666};
      serveDrink(pumps, durations, 4, "newYearSunrise");
    } else if (drink == "electricLemonade") {
      int pumps[] = {pump1, pump5};
      int durations[] = {2000, 222};
      serveDrink(pumps, durations, 2, "electricLemonade");
    } else if (drink == "shirleyTemple") {
      int pumps[] = {pump1, pump5};
      int durations[] = {2000, 500};
      serveDrink(pumps, durations, 2, "shirleyTemple");
    } else if (drink == "yellowHawaiian") {
      int pumps[] = {pump1, pump5, pump6};
      int durations[] = {2000, 222, 2000};
      serveDrink(pumps, durations, 3, "yellowHawaiian");
    } else if (drink == "alelulu") {
      int pumps[] = {pump1, pump2, pump5, pump6};
      int durations[] = {2000, 2000, 400, 400};
      serveDrink(pumps, durations, 4, "alelulu");
    } else {
      server.send(400, "text/plain", "❌ Invalid Drink");
    }
  } else {
    server.send(400, "text/plain", "❌ No Drink Selected");
  }
}

void handleCustom() {
  if (!server.hasArg("durations")) {
    server.send(400, "text/plain", "❌ Missing durations");
    return;
  }

  String durationsArg = server.arg("durations");
  Serial.println("🛠 Custom durations: " + durationsArg);

  const int pumpPins[6] = {pump1, pump2, pump3, pump4, pump5, pump6};
  int durations[6] = {0};
  bool pumpOn[6] = {false};
  bool pumpOff[6] = {false};

  int lastIndex = 0;
  for (int i = 0; i < 6; i++) {
    int commaIndex = durationsArg.indexOf(',', lastIndex);
    String part;
    if (commaIndex == -1) {
      part = durationsArg.substring(lastIndex);
    } else {
      part = durationsArg.substring(lastIndex, commaIndex);
      lastIndex = commaIndex + 1;
    }
    durations[i] = part.toInt();
    if (durations[i] > 0) {
      digitalWrite(pumpPins[i], LOW);
      pumpOn[i] = true;
    }
  }

  unsigned long startTime = millis();

  while (true) {
    bool allDone = true;
    unsigned long now = millis();

    for (int i = 0; i < 6; i++) {
      if (pumpOn[i] && !pumpOff[i] && now - startTime >= durations[i]) {
        digitalWrite(pumpPins[i], HIGH);
        pumpOff[i] = true;
      }
      if (pumpOn[i] && !pumpOff[i]) {
        allDone = false;
      }
    }

    if (allDone) break;
    delay(10);
    yield();
  }

  digitalWrite(mixer, LOW);
  delay(5000);
  digitalWrite(mixer, HIGH);
  delay(10);

  digitalWrite(solenoid, LOW);
  delay(16000);
  digitalWrite(solenoid, HIGH);

  Serial.println("✅ Custom drink complete.");
  server.send(200, "text/plain", "Custom Drink Served");
}

void handleRoot() {
  server.send(200, "text/plain", "ESP32 Cocktail Machine (AP Mode) is Online");
}

void loop() {
  server.handleClient();
}