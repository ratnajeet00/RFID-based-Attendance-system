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

    // Wait for ESP8266 response (name) - adjust delay as needed
    delay(5000);

    // Check for incoming data from ESP8266
    while (mySerial.available() > 0) {
      String name = mySerial.readStringUntil('\n');
      displayOnLCD(name);
      Serial.println("Received Name from ESP8266: " + name);
      delay(5000);
    }

    placeacard();
  }

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
  lcd.print("Name: " + name);

  // If the name is longer than the LCD width, scroll it
 if (name.length() > lcdColumns) {
    for (int i = 0; i < name.length() - lcdColumns + 1; i++) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Name: " + name.substring(i, i + lcdColumns));
        delay(1000);
        
        delay(1000); // Adjust the delay as needed for scrolling speed
    }
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Present marked");

}
