#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>    

ESP8266WebServer server(80);

void setup() {
    Serial.begin(9600);

    WiFiManager wifiManager;
    // Uncomment the next line to reset saved WiFi credentials
    // wifiManager.resetSettings();

    // Attempt to connect to WiFi network. If unsuccessful, open configuration portal
    wifiManager.autoConnect("DeskLampAP");

    Serial.println("Connected to WiFi");
    Serial.println("IP Address: ");
    Serial.println(WiFi.localIP());

    // Setup web server
    server.on("/", handleRoot);
    server.on("/toggle_power", []() { sendCommand('t'); server.send(200, "text/plain", ""); });
    server.on("/set_brightness", []() {
        int brightness = server.arg("value").toInt();
        Serial.write('b');
        Serial.print(brightness);
        server.send(200, "text/plain", "");
    });
    server.on("/select_effect", []() {
        int effect = server.arg("effect").toInt();
        Serial.write('e');
        Serial.print(effect);
        server.send(200, "text/plain", "");
    });
    server.on("/toggle_autoplay", []() { sendCommand('a'); server.send(200, "text/plain", ""); });
    server.on("/toggle_white_mode", []() { sendCommand('w'); server.send(200, "text/plain", ""); });

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}

void sendCommand(char command) {
    Serial.write(command);
}

void handleRoot() {
    String html = 
    "<!DOCTYPE html><html><head>"
    "<title>Lamp Control</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/milligram/1.4.1/milligram.min.css'>"
    "<style>"
    "body { font-family: 'Roboto', 'Arial', sans-serif; background-color: #f4f4f4; }"
    "header { text-align: center; margin: 15px 0; }"
    ".container { padding: 0 15px; }"
    "input[type='range'] { width: 100%; }"
    "</style>"
    "</head><body>"
    "<header>"
    "<h1>Desk Lamp Controller</h1>"
    "</header>"
    "<div class='container'>"
    "<button class='button button-outline' onclick='sendCommand(\"/toggle_power\")'>Toggle Power</button>"
    "<button class='button button-outline' onclick='sendCommand(\"/toggle_autoplay\")'>Toggle Autoplay</button>"
    "<button class='button button-outline' onclick='sendCommand(\"/toggle_white_mode\")'>White Mode</button>"
    "<label for='effectSelect'>Select Effect:</label>"
    "<select id='effectSelect' onchange='selectEffect(this.value)'>"
    "<option value='0'>Lighter</option>"
    "<option value='1'>Light Bugs</option>"
    "<option value='2'>Colors</option>"
    "<option value='3'>Rainbow</option>"
    "<option value='4'>Sparkles</option>"
    "<option value='5'>Fire</option>"
    "<option value='6'>Vinigret</option>"
    "</select>"
    "<label for='brightnessSlider'>Brightness:</label>"
    "<input type='range' id='brightnessSlider' min='0' max='255' value='250' onchange='sendBrightness(this.value)'>"
    "</div>"
    "<script>"
    "function sendCommand(path) {"
    "  var xhttp = new XMLHttpRequest();"
    "  xhttp.open('GET', path, true);"
    "  xhttp.send();"
    "}"
    "function sendBrightness(value) {"
    "  sendCommand('/set_brightness?value=' + value);"
    "}"
    "function selectEffect(effect) {"
    "  sendCommand('/select_effect?effect=' + effect);"
    "}"
    "</script>"
    "</body></html>";

    server.send(200, "text/html", html);
}
