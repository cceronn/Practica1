
#include <Arduino.h>

const int PIN_PULSOS = 18;
const unsigned long ANTIRREBOTE_US = 150000;

volatile unsigned long contador = 0;
volatile unsigned long ultimoPulso = 0;

void IRAM_ATTR contarPulso() {
    unsigned long ahora = micros();

    if (ahora - ultimoPulso >= ANTIRREBOTE_US) {
        contador++;
        ultimoPulso = ahora;
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(PIN_PULSOS, INPUT);

    attachInterrupt(
        digitalPinToInterrupt(PIN_PULSOS),
        contarPulso,
        FALLING
    );

    Serial.println("Contador de pulsos iniciado");
    Serial.println("Presiona el pulsador para contar.");
}

void loop() {
    static unsigned long anterior = 0;
    unsigned long actual;

    noInterrupts();
    actual = contador;
    interrupts();

    if (actual != anterior) {
        Serial.print("Pulsos recibidos: ");
        Serial.println(actual);
        anterior = actual;
    }

    delay(10);
}
