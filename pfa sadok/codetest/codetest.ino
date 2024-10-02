// Définir le numéro de la broche à laquelle la LED est connectée
const int ledPin = 13; // Changez ce numéro selon votre câblage

// Durées pour les différents effets
const int slowBlink = 1000;   // 1 seconde
const int fastBlink = 200;     // 200 millisecondes
const int pulseDelay = 15;     // Délai pour l'effet de pulsation

void setup() {
  // Initialiser la broche de la LED comme une sortie
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // 1. Clignotement lent
  blink(slowBlink);

  // 2. Clignotement rapide
  blink(fastBlink);

  // 3. Effet de pulsation
  pulseEffect();
}

// Fonction pour effectuer un clignotement
void blink(int delayTime) {
  digitalWrite(ledPin, HIGH); // Allumer la LED
  delay(delayTime);            // Attendre
  digitalWrite(ledPin, LOW);  // Éteindre la LED
  delay(delayTime);            // Attendre
}

// Fonction pour l'effet de pulsation
void pulseEffect() {
  // Augmenter la luminosité
  for (int brightness = 0; brightness <= 255; brightness++) {
    analogWrite(ledPin, brightness); // Ajuste la luminosité
    delay(pulseDelay);                // Délai pour la pulsation
  }

  // Diminuer la luminosité
  for (int brightness = 255; brightness >= 0; brightness--) {
    analogWrite(ledPin, brightness); // Ajuste la luminosité
    delay(pulseDelay);                // Délai pour la pulsation
  }
}
