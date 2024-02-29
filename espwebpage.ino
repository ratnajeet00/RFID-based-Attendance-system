#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

const char* ssid = "MyWifi";
const char* password = "Vikas@16";
const char* apiUrl = "https://192.168.1.3:3000/attendance"; // Replace with your server address

SoftwareSerial mySerial(D2, D3); // RX=D2, TX=D3

void setup() {
  Serial.begin(2400);
  mySerial.begin(2400);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if (mySerial.available() > 0) {
    String uid = mySerial.readStringUntil('\n');

    // Print the raw RFID UID
    Serial.println("Raw RFID UID: " + uid);

    // URL encode the RFID UID using custom function
    String encodedUid = urlEncode(uid);

    // Print the URL-encoded UID
    Serial.println("Encoded RFID UID: " + encodedUid);

    // Make HTTP request to get the name from the server
    String name = getNameFromServer(encodedUid);

    // Send the name back to the Arduino
    mySerial.println(name);
    Serial.println("Name: " + name);
  }

  delay(5000);
}

String getNameFromServer(String encodedUid) {
  WiFiClient wifiClient;

  HTTPClient http;
  http.begin(wifiClient, apiUrl);
  http.addHeader("Content-Type", "application/json");

  // Decide whether to capture or check based on your conditions
  String action = "check"; // Change this based on your logic

  // Use the URL-encoded UID and action in the JSON payload
  String jsonPayload = "{\"uid\":\"" + encodedUid + "\",\"action\":\"" + action + "\"}";

  Serial.println("Sent JSON Payload: " + jsonPayload);

  // Make the HTTP POST request
  int httpCode = http.POST(jsonPayload);

  // Print the HTTP response code
  Serial.print("HTTP response code: ");
  Serial.println(httpCode);

  String name = "";

  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    Serial.println("Server response: " + response);

    // Parse the JSON response to extract the name
    name = parseJsonResponse(response);
  } else {
    Serial.println("HTTP request failed");
    Serial.println(http.getString());  // Print server response even if the request fails
  }

  http.end();
  return name;
}

String parseJsonResponse(String jsonResponse) {
  String name = "";

  // Parse JSON using ArduinoJson library
  DynamicJsonDocument doc(2048);  // Adjust the capacity based on your JSON structure
  deserializeJson(doc, jsonResponse);

  // Extract the name from the JSON
  name = doc["name"].as<String>();

  return name;
}

// Function to URL encode a string
String urlEncode(String str) {
  String encodedString = "";

  char c;
  char code0;
  char code1;

  for (unsigned int i = 0; i < str.length(); i++) {
    c = str.charAt(i);

    if (c == ' ') {
      encodedString += '+';
    } else if ((c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9')) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
    yield();
  }

  return encodedString;
}
