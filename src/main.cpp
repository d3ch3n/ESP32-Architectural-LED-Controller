#include <Arduino.h>

#include "Core/Application.h"

void setup()
{
    g_application.begin();
}

void loop()
{
    g_application.update();
}