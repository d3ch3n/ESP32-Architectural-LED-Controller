#include <Arduino.h>

void setup()
{
    Serial.begin(115200);

    Serial.println();
    Serial.println("--------------------------");
    Serial.println("Ripado v0.0.1");
    Serial.println("Inicializando...");
    Serial.println("--------------------------");
}

void loop()
{
    Serial.printf("Tempo ligado: %lu ms\n", millis());
    delay(1000);
}