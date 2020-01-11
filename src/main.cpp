#ifndef UNIT_TEST

#include <Arduino.h>

#endif

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

/*********************************************************************/

const char *ssid = "ALBANESE";
const char *password = "0101010101";
ESP8266WebServer server(80);

/*********************************************************************/

const int red = 5;
const int green = 4;
const int blue = 0;

int color[] = {0, 0, 0};
int relay[] = {14, 12, 13, 15};
bool status_relay[] = {false, false, false, false};

/*********************************************************************/

/**
 * Size of relay array
 * @return number of element in array
 */
int get_relay_size() {
    return sizeof(relay) / sizeof(relay[0]);
}

/**
 * Initialization of relay pins
 */
void init_relay(int relay_size) {
    for (int i = 0; i < relay_size; i++) {
        pinMode(relay[i], OUTPUT);
        digitalWrite(relay[i], HIGH);
    }
}

/*********************************************************************/

/**
 * Check if the relay pin received is correct
 * @param num relay pin
 * @return "true" if is correct | "false" if is incorrect
 */
bool is_valid_relay_pin(int num) {
    if (num >= 0 && num <= get_relay_size() - 1) {
        return true;
    }
    return false;
}

/*********************************************************************/

/**
 * Check if the value received for the color is correct
 * @param value color value
 * @return "true" if is correct | "false" if is incorrect
 */
bool is_valid_color_value(int value) {
    if (value >= 0 && value <= 255) {
        return true;
    }
    return false;
}

/*********************************************************************/

/**
 * Turn on a specific relay
 * @param pin Pin of relay pin
 */
void turn_on(int pin) {
    digitalWrite(pin, LOW);
}

/**
 * Turn off a specific relay
 * @param pin Pin of relay pin
 */
void turn_off(int pin) {
    digitalWrite(pin, HIGH);
}

/*********************************************************************/

/**
 * Size of color array
 * @return number of element in array
 */
int get_color_size() {
    return sizeof(color) / sizeof(color[0]);
}

/**
 * Set all colors to 0
 */
void reset_color() {
    analogWrite(red, 0);
    analogWrite(green, 0);
    analogWrite(blue, 0);

    for (int i = 0; i < get_color_size(); i++) {
        color[i] = 0;
    }
}

/**
 * Initialization of rgb pins
 */
void init_rgb() {
    pinMode(red, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(blue, OUTPUT);

    reset_color();
}

/*********************************************************************/

/**
 * Set the color of the leds
 * @param red_input red value
 * @param green_input green value
 * @param blue_input blue value
 */
void set_color(int red_input, int green_input, int blue_input) {
    analogWrite(red, red_input);
    analogWrite(green, green_input);
    analogWrite(blue, blue_input);

    color[0] = red_input;
    color[1] = green_input;
    color[2] = blue_input;
}

/*********************************************************************/

/**
 * Turn on/off a specific relay by parameters
 */
void relay_args() {
    String message = "";
    int num;

    if (server.arg(0) == "on") {
        num = server.arg(1).toInt() - 1;

        if (is_valid_relay_pin(num)) {
            turn_on(relay[num]);
            status_relay[num] = true;

            message = "On = " + server.arg(1);

            server.send(200, "text/plain", message);
        } else {
            message = "Erro: num of relay";

            server.send(400, "text/plain", message);
        }
    } else if (server.arg(0) == "off") {
        num = server.arg(1).toInt() - 1;

        if (is_valid_relay_pin(num)) {
            turn_off(relay[num]);
            status_relay[num] = false;

            message = "Off = " + server.arg(1);

            server.send(200, "text/plain", message);
        } else {
            message = "Erro: num of relay";

            server.send(400, "text/plain", message);
        }
    } else {
        message = "Error: incorrect action";

        server.send(400, "text/plain", message);
    }
}

/*********************************************************************/

/**
 * Send the status of relays
 */
void relay_status() {
    String status;
    DynamicJsonDocument doc(1024);

    for (int i = 0; i < get_relay_size(); i++) {
        int num = i + 1;
        String name = "in" + String(num);

        doc[name] = status_relay[i];
    }

    serializeJson(doc, status);

    server.send(200, "text/json", status);
}

/*********************************************************************/

/**
 * Receives the value of the colors and sets them
 */
void rgb_args() {
    String message = "";

    int red_input = server.arg(0).toInt();
    int green_input = server.arg(1).toInt();
    int blue_input = server.arg(2).toInt();

    if (!is_valid_color_value(red_input)) {
        red_input = 0;
    }

    if (!is_valid_color_value(green_input)) {
        green_input = 0;
    }

    if (!is_valid_color_value(blue_input)) {
        blue_input = 0;
    }

    set_color(red_input, green_input, blue_input);

    message = "ok";

    server.send(200, "text/plain", message);
}

/*********************************************************************/

/**
 * Send the status of colors
 */
void rgb_status() {
    String status;
    DynamicJsonDocument doc(1024);

    doc["red"] = color[0];
    doc["green"] = color[1];
    doc["blue"] = color[2];

    serializeJson(doc, status);

    server.send(200, "text/json", status);
}

/*********************************************************************/

void setup() {
    init_relay(get_relay_size());
    init_rgb();

    WiFi.hostname("NodeMCU-LedStrip");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    // /smart?action=on&in=1
    server.on("/rgb/relay", relay_args);
    server.on("/rgb/relay/status", relay_status);

    // /rgb?red=255&green=255&blue=255
    server.on("/rgb", rgb_args);
    server.on("/rgb/status", rgb_status);

    server.begin();
}

void loop() {
    server.handleClient();
}