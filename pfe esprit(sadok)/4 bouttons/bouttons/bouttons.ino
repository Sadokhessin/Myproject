#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>
#include <ThingSpeak.h>

// Configurations WiFi
const char* ssid = "Mot de passe";
const char* password = "123123123";

// Configuration ThingSpeak
const char* myWriteAPIKey = "JE4ZR6XIPAPJCIBG";
unsigned long myChannelNumber = 2610671;

// Boutons
int buttonPin1 = D3;
int buttonPin2 = D4;
int buttonPin3 = D5;
int buttonPin4 = D6;

int buttonState1;
int lastButtonState1 = HIGH;

int buttonState2;
int lastButtonState2 = HIGH;

int buttonState3;
int lastButtonState3 = HIGH;

int buttonState4;
int lastButtonState4 = HIGH;

unsigned long startTime1 = 0;
unsigned long elapsedTime1 = 0;
bool counting1 = false;

unsigned long startTime2 = 0;
unsigned long elapsedTime2 = 0;
bool counting2 = false;

unsigned long startTime3 = 0;
unsigned long elapsedTime3 = 0;
bool counting3 = false;

unsigned long startTime4 = 0;
unsigned long elapsedTime4 = 0;
bool counting4 = false;

WiFiClient client;
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    Serial.begin(9600);
    delay(10);

    // Initialisation de l'afficheur LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Connecting...");

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        lcd.setCursor(0, 1);
        lcd.print(".");
    }
    Serial.println("Connected to WiFi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connected!");

    ThingSpeak.begin(client);

    // Configuration des boutons
    pinMode(buttonPin1, INPUT_PULLUP);
    pinMode(buttonPin2, INPUT_PULLUP);
    pinMode(buttonPin3, INPUT_PULLUP);
    pinMode(buttonPin4, INPUT_PULLUP);
}

void loop() {
    // Gestion du bouton 1 (Panne mécanique)
    buttonState1 = digitalRead(buttonPin1);
    if (buttonState1 == LOW && lastButtonState1 == HIGH) {
        if (counting1) {
            // Stoppe le compteur et envoie le temps cumulé
            elapsedTime1 += millis() - startTime1;
            sendTimeToThingSpeak(elapsedTime1, 1);
            counting1 = false;
        } else {
            // Démarre le compteur
            startTime1 = millis();
            elapsedTime1 = 0; // Reset elapsed time
            counting1 = true;
        }
    }
    lastButtonState1 = buttonState1;

    // Gestion du bouton 2 (Panne électrique)
    buttonState2 = digitalRead(buttonPin2);
    if (buttonState2 == LOW && lastButtonState2 == HIGH) {
        if (counting2) {
            // Stoppe le compteur et envoie le temps cumulé
            elapsedTime2 += millis() - startTime2;
            sendTimeToThingSpeak(elapsedTime2, 2);
            counting2 = false;
        } else {
            // Démarre le compteur
            startTime2 = millis();
            elapsedTime2 = 0; // Reset elapsed time
            counting2 = true;
        }
    }
    lastButtonState2 = buttonState2;

    // Gestion du bouton 3 (Panne qualité)
    buttonState3 = digitalRead(buttonPin3);
    if (buttonState3 == LOW && lastButtonState3 == HIGH) {
        if (counting3) {
            // Stoppe le compteur et envoie le temps cumulé
            elapsedTime3 += millis() - startTime3;
            sendTimeToThingSpeak(elapsedTime3, 3);
            counting3 = false;
        } else {
            // Démarre le compteur
            startTime3 = millis();
            elapsedTime3 = 0; // Reset elapsed time
            counting3 = true;
        }
    }
    lastButtonState3 = buttonState3;

    // Gestion du bouton 4 (Réglage machine)
    buttonState4 = digitalRead(buttonPin4);
    if (buttonState4 == LOW && lastButtonState4 == HIGH) {
        if (counting4) {
            // Stoppe le compteur et envoie le temps cumulé
            elapsedTime4 += millis() - startTime4;
            sendTimeToThingSpeak(elapsedTime4, 4);
            counting4 = false;
        } else {
            // Démarre le compteur
            startTime4 = millis();
            elapsedTime4 = 0; // Reset elapsed time
            counting4 = true;
        }
    }
    lastButtonState4 = buttonState4;

    // Mise à jour de l'affichage LCD
    updateLCD();

    delay(100); // Réduire le délai pour une détection plus rapide des boutons
}

void updateLCD() {
    lcd.clear();
    if (counting1) {
        unsigned long currentTime1 = millis() - startTime1;
        lcd.setCursor(0, 0);
        lcd.print("Btn1 Time: ");
        lcd.print((elapsedTime1 + currentTime1) / 1000); // Convertir en secondes
        lcd.setCursor(0, 1);
        lcd.print("Panne Mec.");
    } else if (counting2) {
        unsigned long currentTime2 = millis() - startTime2;
        lcd.setCursor(0, 0);
        lcd.print("Btn2 Time: ");
        lcd.print((elapsedTime2 + currentTime2) / 1000); // Convertir en secondes
        lcd.setCursor(0, 1);
        lcd.print("Panne Elec.");
    } else if (counting3) {
        unsigned long currentTime3 = millis() - startTime3;
        lcd.setCursor(0, 0);
        lcd.print("Btn3 Time: ");
        lcd.print((elapsedTime3 + currentTime3) / 1000); // Convertir en secondes
        lcd.setCursor(0, 1);
        lcd.print("Panne Qual.");
    } else if (counting4) {
        unsigned long currentTime4 = millis() - startTime4;
        lcd.setCursor(0, 0);
        lcd.print("Btn4 Time: ");
        lcd.print((elapsedTime4 + currentTime4) / 1000); // Convertir en secondes
        lcd.setCursor(0, 1);
        lcd.print("Reglage Mach.");
    } else {
        lcd.setCursor(0, 0);
        lcd.print("Aucune Panne");
        lcd.setCursor(0, 1);
        lcd.print("Detectee");
    }
}

void sendTimeToThingSpeak(unsigned long time, int field) {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("Sending time ");
        Serial.print(time / 1000); // Convertir en secondes
        Serial.print(" to ThingSpeak (Field ");
        Serial.print(field);
        Serial.println(")...");

        int responseCode = ThingSpeak.writeField(myChannelNumber, field, (long)(time / 1000), myWriteAPIKey);
        if (responseCode == 200) {
            Serial.println("Data sent to ThingSpeak successfully");
        } else {
            Serial.println("Problem sending data to ThingSpeak. HTTP error code: " + String(responseCode));
        }
    } else {
        Serial.println("WiFi not connected");
    }
}
//code des 4 butons