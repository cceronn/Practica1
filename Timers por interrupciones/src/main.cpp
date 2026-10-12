
#include <Arduino.h>

const int PIN_BOTON = 18;
const unsigned long ANTIRREBOTE_US = 200000;

// Variables compartidas con la interrupción
volatile unsigned long ultimoPulsoUs = 0;
volatile unsigned long instantePulsoUs = 0;
volatile bool nuevoPulso = false;

// Variables del programa principal
unsigned long primerPulsoUs = 0;
bool esperandoSegundoPulso = false;

// Rutina de servicio de interrupción
void IRAM_ATTR interrupcionBoton() {
    unsigned long ahora = micros();

    // Antirrebote por software: 200 ms
    if (ahora - ultimoPulsoUs >= ANTIRREBOTE_US) {
        ultimoPulsoUs = ahora;
        instantePulsoUs = ahora;
        nuevoPulso = true;
    }
}

void setup() {
    Serial.begin(115200);

    // La resistencia externa de 10 kΩ mantiene el GPIO en LOW
    pinMode(PIN_BOTON, INPUT);

    // El pulsador lleva GPIO 18 a HIGH al presionarlo
    attachInterrupt(
        digitalPinToInterrupt(PIN_BOTON),
        interrupcionBoton,
        RISING
    );

    Serial.println();
    Serial.println("Medicion de tiempo entre pulsaciones");
    Serial.println("ESP32 listo. Pulsa el boton dos veces.");
}

void loop() {
    unsigned long pulsoActualUs = 0;
    bool hayPulso = false;

    // Acceso seguro a las variables compartidas
    noInterrupts();

    if (nuevoPulso) {
        pulsoActualUs = instantePulsoUs;
        nuevoPulso = false;
        hayPulso = true;
    }

    interrupts();

    if (hayPulso) {
        if (!esperandoSegundoPulso) {
            primerPulsoUs = pulsoActualUs;
            esperandoSegundoPulso = true;

            Serial.println("Primera pulsacion registrada.");
        } else {
            unsigned long diferenciaUs =
                pulsoActualUs - primerPulsoUs;

            Serial.print("Tiempo entre pulsaciones: ");
            Serial.print(diferenciaUs / 1000.0, 3);
            Serial.println(" ms");

            esperandoSegundoPulso = false;
        }
    }
}