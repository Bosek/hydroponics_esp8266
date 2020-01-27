#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <MillisTimer.h>

#define GPIO0 0 //Pump
#define GPIO1 1 //LED
#define GPIO2 2 //Lights

ESP8266WebServer server(80);
HTTPClient httpClient;

int lightsTime = 60*1000;
MillisTimer lightsTimer = MillisTimer(lightsTime);
int pumpTime = 15*60*1000;
MillisTimer pumpTimer = MillisTimer(pumpTime);
int blinkTime = 1000;
MillisTimer blinkTimer = MillisTimer(blinkTime);

WiFiClient client;
bool sunlightNow = false;
bool pump = true;
bool blink = false;

void handleHttp() {
  server.send(200, "text/plain", sunlightNow ? "Sun is up." : "Sun is down.");
}

void cycleLights(MillisTimer &mt) {
  if (WiFi.isConnected()) {
    if(httpClient.begin(client, "http://hydroponics.drungor.cz/sunlightnow")) {
      if (httpClient.GET() == 200) {
        sunlightNow = httpClient.getString() == "true";
      }
      else {
        sunlightNow = false;
      }
      httpClient.end();
    }
  }
  else {
    sunlightNow = false;
  }
  digitalWrite(GPIO2, !sunlightNow);
}

void cyclePump(MillisTimer &mt) {
  pump = !pump;
  digitalWrite(GPIO0, pump);
}

void cycleBlink(MillisTimer &mt) {
  if (!WiFi.isConnected()) {
    digitalWrite(GPIO1, !blink);
    blink = !blink;
  }
  else {
    digitalWrite(GPIO1, LOW);
  }
}

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin("Bosekovi", "sarka6767");
  // WiFi.begin("CajovnaNaRynku", "Ovocnykoktejl1");

  pinMode(GPIO0, OUTPUT);
  digitalWrite(GPIO0, HIGH);
  pinMode(GPIO1, OUTPUT);
  digitalWrite(GPIO1, HIGH);
  pinMode(GPIO2, OUTPUT);
  digitalWrite(GPIO2, HIGH);

  server.onNotFound(handleHttp);
  server.begin();

  httpClient.setReuse(true);

  pumpTimer.expiredHandler(cyclePump);
  pumpTimer.start();

  lightsTimer.expiredHandler(cycleLights);
  lightsTimer.startFrom(lightsTime - 10*1000);

  blinkTimer.expiredHandler(cycleBlink);
  blinkTimer.start();
}

void loop() {
  blinkTimer.run();
  pumpTimer.run();
  lightsTimer.run();

  server.handleClient();
}