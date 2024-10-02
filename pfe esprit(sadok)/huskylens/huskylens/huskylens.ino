#include "HUSKYLENS.h"
#include "SoftwareSerial.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>

// Configurer vos identifiants WiFi
const char* ssid = "Mot de passe";
const char* password = "123123123";

// Clé API de votre channel ThingSpeak
const char* myWriteAPIKey = "JE4ZR6XIPAPJCIBG";
unsigned long myChannelNumber = 2610671; // Remplacez par votre numéro de canal

HUSKYLENS huskylens;
SoftwareSerial mySerial(14, 12); // RX, TX
WiFiClient client;

const int endStopPin = D3; // Pin du bouton de fin de course
int pieceCount = 0; // Compteur de pièces

void setup() {
    Serial.begin(115200);
    mySerial.begin(9600);

    pinMode(endStopPin, INPUT_PULLUP); // Configurer le bouton de fin de course en entrée avec pull-up

    // Initialiser la connexion WiFi
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Connected to WiFi");

    // Initialiser ThingSpeak
    ThingSpeak.begin(client);
    Serial.println("ThingSpeak initialized");

    // Initialiser HUSKYLENS
    Serial.println("Initializing HUSKYLENS...");
    while (!huskylens.begin(mySerial)) {
        Serial.println(F("Begin failed!"));
        Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
        Serial.println(F("2.Please recheck the connection."));
        delay(100);
    }
    Serial.println("HUSKYLENS started successfully");
}

void loop() {
    Serial.println("Requesting data from HUSKYLENS...");
    if (!huskylens.request()) {
        Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    } else if(!huskylens.isLearned()) {
        Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
    } else if(!huskylens.available()) {
        Serial.println(F("No block or arrow appears on the screen!"));
    } else {
        Serial.println("Data received from HUSKYLENS");
        while (huskylens.available()) {
            HUSKYLENSResult result = huskylens.read();
            Serial.print("Detected ID: ");
            Serial.println(result.ID);
            if (result.ID != 1 && digitalRead(endStopPin) == HIGH) {
                pieceCount++;
                Serial.print("Piece Count: ");
                Serial.println(pieceCount);
                sendPieceCountToThingSpeak(pieceCount);
                delay(500); // Petite pause pour éviter de multiples incrémentations dues à un appui prolongé sur le bouton
            }
            delay(1000); // Petite pause pour éviter de surcharger le port série
        }
    }
    delay(1000); // Petite pause pour éviter de surcharger le port série
}

void sendPieceCountToThingSpeak(int count) {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("Sending Piece Count ");
        Serial.print(count);
        Serial.println(" to ThingSpeak...");
        int responseCode = ThingSpeak.writeField(myChannelNumber, 4, count, myWriteAPIKey);
        if (responseCode == 200) {
            Serial.println("Data sent to ThingSpeak successfully");
        } else {
            Serial.println("Problem sending data to ThingSpeak. HTTP error code: " + String(responseCode));
        }
    } else {
        Serial.println("WiFi not connected");
    }
}