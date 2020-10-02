/*
  Purple Air Arduino Reader

  Hardware requirements:

  Arduino MKR 1010
  Arduino MKR RGB Sheild
  
*/

#include <ArduinoGraphics.h> // Arduino_MKRRGB depends on ArduinoGraphics
#include <Arduino_MKRRGB.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <Arduino_JSON.h>

// Air quality level PM2.5 μg/m3  (see https://blissair.com/what-is-pm-2-5.htm)
int max_level[] = {12, 35, 55, 150, 250, 500};
String labels[] = {"Good", "Moderate", "USG", "Unhealthy", "V Unhealthy", "Hazardous"};
int color[][3] = {{0, 255, 0}, {255, 255, 0}, {255, 128, 0}, {255, 0, 0}, {128, 0, 128}, {128, 0, 0}};

long timer = 0;

// please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password
String sensor_id = SECRET_SENSOR_ID;

int status = WL_IDLE_STATUS;
char server[] = "www.purpleair.com";
WiFiClient wifi;
HttpClient client = HttpClient(wifi, server, 80);

void setup() {
  
  Serial.begin(9600);
  
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);  
    status = WiFi.begin(ssid, pass);
  }

  MATRIX.begin();
  MATRIX.brightness(10);
  MATRIX.textScrollSpeed(50);
}

void loop() {
  getAirQuality();
}

void getAirQuality() {
  Serial.println("Requesting data from PurpleAir");
  client.get("/json?show=" + sensor_id);
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  
  int level = 5;

  if (statusCode == 200) {
    JSONVar myObject = JSON.parse(response);
    int PM2_5 = atoi(myObject["results"][0]["PM2_5Value"]);

    // Find air quality level
    for (int i = 0; i < 6; i++) {
      if (PM2_5 <= max_level[i]) {
        level = i;
        break;
      }
    }

    Serial.println(PM2_5);
    Serial.println(labels[level]);

    timer = millis();
    
    MATRIX.beginText(MATRIX.width() - 1, 0, color[level][0], color[level][1], color[level][2]); // X, Y, then R, G, B
    MATRIX.print(PM2_5);
    MATRIX.print("  ");
    MATRIX.println(labels[level]);
    MATRIX.endText(SCROLL_LEFT);
    
    MATRIX.beginDraw();
    MATRIX.stroke(color[level][0], color[level][1], color[level][2]);
    MATRIX.rect(0, 0, MATRIX.width(),  MATRIX.height());
    MATRIX.endDraw();

  } 
   delay(10000);
}
