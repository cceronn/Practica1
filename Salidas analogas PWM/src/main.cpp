
#include <Arduino.h>
#include <ESP32Servo.h>
#include <math.h>

// ==================== PINES ====================
const int LDR_IZQUIERDO = 35;
const int LDR_DERECHO   = 34;
const int SERVO_PIN     = 25;
const int LED_PIN       = 26;

// ==================== CONFIGURACION ====================
const float VCC_MV = 3300.0;
const float RESISTENCIA_FIJA = 10000.0;

// Ajustar experimentalmente segun la iluminacion
const int ZONA_MUERTA = 180;
const int PASO_SERVO = 2;

const int ANGULO_MIN = 0;
const int ANGULO_MAX = 180;
const int ANGULO_INICIAL = 90;

const unsigned long INTERVALO = 100;
const int NUM_MUESTRAS = 10;

// ==================== OBJETOS ====================
Servo servo;

int anguloActual = ANGULO_INICIAL;
unsigned long tiempoAnterior = 0;

// Promedia varias lecturas para reducir ruido
int leerLDR(int pin) {
    long suma = 0;

    for (int i = 0; i < NUM_MUESTRAS; i++) {
        suma += analogRead(pin);
        delay(2);
    }

    return suma / NUM_MUESTRAS;
}

// Calcula la resistencia del LDR.
// Circuito: 3V3 -> LDR -> GPIO -> 10k -> GND
float calcularResistencia(int pin) {
    float voltaje_mV = analogReadMilliVolts(pin);

    if (voltaje_mV <= 1.0) {
        return INFINITY;
    }

    if (voltaje_mV >= VCC_MV) {
        return 0.0;
    }

    return RESISTENCIA_FIJA *
           (VCC_MV / voltaje_mV - 1.0);
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);

    analogReadResolution(12);
    analogSetPinAttenuation(LDR_IZQUIERDO, ADC_11db);
    analogSetPinAttenuation(LDR_DERECHO, ADC_11db);

    // Configuracion del PWM para el servomotor
    servo.setPeriodHertz(50);
    servo.attach(SERVO_PIN, 500, 2400);

    servo.write(ANGULO_INICIAL);
    delay(500);

    Serial.println("Seguidor de luz - ESP32");
    Serial.println("ADC_I,ADC_D,R_I_ohm,R_D_ohm,Angulo");
}

void loop() {
    if (millis() - tiempoAnterior < INTERVALO) {
        return;
    }

    tiempoAnterior = millis();

    // Lecturas analogicas
    int lecturaIzquierda = leerLDR(LDR_IZQUIERDO);
    int lecturaDerecha = leerLDR(LDR_DERECHO);

    // Resistencias estimadas
    float resistenciaIzquierda =
        calcularResistencia(LDR_IZQUIERDO);

    float resistenciaDerecha =
        calcularResistencia(LDR_DERECHO);

    // Diferencia de iluminacion.
    // Con este divisor, una lectura ADC mayor
    // normalmente significa mas luz.
    int diferencia = lecturaIzquierda - lecturaDerecha;

    // Control incremental con zona muerta
    if (diferencia > ZONA_MUERTA) {
        anguloActual += PASO_SERVO;
    } else if (diferencia < -ZONA_MUERTA) {
        anguloActual -= PASO_SERVO;
    }

    anguloActual = constrain(
        anguloActual, ANGULO_MIN, ANGULO_MAX
    );

    servo.write(anguloActual);

    // LED opcional: encendido si hay diferencia notable
    digitalWrite(
        LED_PIN,
        abs(diferencia) > ZONA_MUERTA ? HIGH : LOW
    );

    // Monitor serial: una fila por lectura
    Serial.print(lecturaIzquierda);
    Serial.print(",");

    Serial.print(lecturaDerecha);
    Serial.print(",");

    if (isinf(resistenciaIzquierda)) {
        Serial.print("INF");
    } else {
        Serial.print(resistenciaIzquierda, 0);
    }

    Serial.print(",");

    if (isinf(resistenciaDerecha)) {
        Serial.print("INF");
    } else {
        Serial.print(resistenciaDerecha, 0);
    }

    Serial.print(",");
    Serial.println(anguloActual);
}