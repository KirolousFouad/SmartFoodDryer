#include <Arduino.h>
#include "Application.h"
#include "WeightSensor.h"

WeightSensor weightSensor(6, 7);

Application app;

void setup()
{
    app.begin();

    delay(1000);

    weightSensor.begin();
}

void loop()
{
    app.update();

    weightSensor.update();

    delay(100);
}