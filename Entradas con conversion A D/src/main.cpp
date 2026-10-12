
#include <Arduino.h>

// ==================== PINES ====================

const int LDR_IZQ = 35;
const int LDR_DER = 34;

const int LED_IZQ = 25;
const int LED_DER = 26;

// ==================== CONSTANTES ====================

const float VCC_MV = 3300.0;
const float RESISTENCIA_FIJA = 10000.0;  // 10 kOhm

// Diferencia minima para considerar que predomina un lado.
// Ajustable segun las condiciones de iluminacion.
const int UMBRAL = 150;

// Intervalo entre mediciones
const unsigned long INTERVALO_MS = 250;

// ==================== VARIABLES ====================

int lecturaIzq = 0;
int lecturaDer = 0;

float voltajeIzq = 0.0;
float voltajeDer = 0.0;

float resistenciaIzq = 0.0;
float resistenciaDer = 0.0;

// ==================== FUNCIONES ====================

// Promedia varias lecturas para reducir fluctuaciones.
int leerLDR(int pin) {
    long suma = 0;
    
    for (int i = 0; i < 10; i++) {
        suma += analogRead(pin);
        delay(2);
    }

    return suma / 10;
}

// Convierte el voltaje medido a resistencia del LDR.
// Circuito: 3V3 -> LDR -> GPIO -> resistencia 10k -> GND.
float calcularResistencia(float voltaje_mV) {
    if (voltaje_mV <= 1.0) {
        return INFINITY;
    }

    if (voltaje_mV >= VCC_MV) {
        return 0.0;
    }

    return RESISTENCIA_FIJA *
           (VCC_MV / voltaje_mV - 1.0);
}

// ==================== CONFIGURACION ====================

void setup() {
    Serial.begin(115200);

    pinMode(LED_IZQ, OUTPUT);
    pinMode(LED_DER, OUTPUT);

    digitalWrite(LED_IZQ, LOW);
    digitalWrite(LED_DER, LOW);

    analogReadResolution(12);

    analogSetPinAttenuation(LDR_IZQ, ADC_11db);
    analogSetPinAttenuation(LDR_DER, ADC_11db);

    delay(500);

    Serial.println();
    Serial.println("Monitor de dos sensores LDR");
    Serial.println("Izquierdo: GPIO 35 | LED: GPIO 25");
    Serial.println("Derecho:   GPIO 34 | LED: GPIO 26");
    Serial.println("----------------------------------------");
}

// ==================== BUCLE PRINCIPAL ====================

void loop() {
    // 1. Leer los dos sensores.
    lecturaIzq = leerLDR(LDR_IZQ);
    lecturaDer = leerLDR(LDR_DER);

    // 2. Obtener voltajes en milivoltios.
    voltajeIzq = analogReadMilliVolts(LDR_IZQ);
    voltajeDer = analogReadMilliVolts(LDR_DER);

    // 3. Calcular la resistencia de cada LDR.
    resistenciaIzq = calcularResistencia(voltajeIzq);
    resistenciaDer = calcularResistencia(voltajeDer);

    // 4. Determinar el lado con mayor iluminacion.
    String direccion;

    if (lecturaIzq > lecturaDer + UMBRAL) {
        direccion = "IZQUIERDA";

        digitalWrite(LED_IZQ, HIGH);
        digitalWrite(LED_DER, LOW);

    } else if (lecturaDer > lecturaIzq + UMBRAL) {
        direccion = "DERECHA";

        digitalWrite(LED_IZQ, LOW);
        digitalWrite(LED_DER, HIGH);

    } else {
        direccion = "CENTRO / LUZ SIMILAR";

        digitalWrite(LED_IZQ, LOW);
        digitalWrite(LED_DER, LOW);
    }

    // 5. Mostrar resultados en el monitor serial.
    Serial.println("========================================");

    Serial.print("ADC izquierdo: ");
    Serial.println(lecturaIzq);

    Serial.print("ADC derecho:   ");
    Serial.println(lecturaDer);

    Serial.print("Voltaje izquierdo: ");
    Serial.print(voltajeIzq / 1000.0, 3);
    Serial.println(" V");

    Serial.print("Voltaje derecho:   ");
    Serial.print(voltajeDer / 1000.0, 3);
    Serial.println(" V");

    Serial.print("Resistencia LDR izquierdo: ");
    Serial.print(resistenciaIzq / 1000.0, 2);
    Serial.println(" kOhm");

    Serial.print("Resistencia LDR derecho:   ");
    Serial.print(resistenciaDer / 1000.0, 2);
    Serial.println(" kOhm");

    Serial.print("Posicion del foco: ");
    Serial.println(direccion);

    Serial.println();

    delay(INTERVALO_MS);
}
