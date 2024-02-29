#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

int lcdAddress = 0x27;
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);

MFRC522 mfrc522(10, 9); // SDA, RST
SoftwareSerial mySerial(2, 3); // RX=D2, TX=D3

void setup() {
  Wire.begin();
  mySerial.begin(2400);
  Serial.begin(2400);
  while (!Serial);

  lcd.begin(lcdColumns, lcdRows);
  lcd.backlight();

  Serial.println("\nI2C Scanner and RFID Reader");

  placeacard();
  SPI.begin();
  mfrc522.PCD_Init();
}

void placeacard() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place a card...");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Card UID:");
    lcd.setCursor(0, 1);
    String uid = getCardUID();
    lcd.print(uid);

    Serial.print("Card UID: ");
    Serial.println(uid);

    mySerial.println(uid);

    Serial.println("Waiting for ESP8266 response...");

    // Wait for ESP8266 response (name) - wait until serial data is available
    while (!mySerial.available()) {
      // Do nothing while waiting for data
    }

    // Check for incoming data from ESP8266
    String name = mySerial.readStringUntil('\n');
    lcd.clear();
    displayOnLCD(name);
    Serial.println("Received Name from ESP8266: " + name);
  }

  placeacard();
  delay(500);
}


String getCardUID() {
  String cardUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    cardUID += String(mfrc522.uid.uidByte[i], HEX);
  }
  return cardUID;
}
void displayOnLCD(String name) {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if (name.length() <= lcdColumns) {
    // If the name fits on one line, display it directly
    lcd.print("Name: " + name);
  } else {
    // If the name is longer than the LCD width, scroll it
    for (int i = 0; i < name.length() - lcdColumns + 1; i++) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Name: " + name.substring(i, i + lcdColumns));
      delay(500);  // Adjust the delay as needed for scrolling speed
    }
  }
  delay(3000);  // Display the full name for a moment (adjust as needed)
}

