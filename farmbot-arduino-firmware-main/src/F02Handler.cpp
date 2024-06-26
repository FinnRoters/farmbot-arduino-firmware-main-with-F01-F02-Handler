/*
 * F02Handler.cpp
 *
 *  Created on: 13 june 2024
 *      Author: Finn Roters
 */

#include "F02Handler.h"

static F02Handler *instance;  // defines a static pointer to an "F02Handler" instance (ensures that there is only one instance of the class).

unsigned int F02Handler::pulsePerSecond;
unsigned int F02Handler::pulse;

// Check whether instance points to nullptr (i.e. no valid memory area) (if yes, a new instance of F02Handler is created).
F02Handler *F02Handler::getInstance()
{
  if (!instance)
  {
    instance = new F02Handler();
  };
  return instance;
};

// Constructor of the F42Handler class (empty, because no special initialisations are necessary).
F02Handler::F02Handler()
{
}


int F02Handler::execute(Command *command)
{
  unsigned int valvePin = 10;               // Pin for the water valve.
  unsigned int sensorPin = 2;              // Pin 2 (Arduino Mega 2560) Interrupt capable
  float calibrationFactor = 7.5;          // According to the data sheet: 450 pulses / L -> Calibration Factor: 450 / 60 = 7.5
  pulsePerSecond = 0;
  pulse = 0;
  float flowRate = 0.0;
  long flowMl = 0;
  long totalMl = 0;
  unsigned long oldTime = 0;
  pinMode(sensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(sensorPin), F02Handler::pulseCount, RISING);

  Serial.println("F02 command has started");
  // Open the water valve  
  PinControl::getInstance()->writeValue(valvePin, 1, 0); // Set the pin of the water valve to high (0 = digital).
  
  Serial.print("command->getN(): ");
  Serial.println(command->getN());  // Output of the value of command->getN()

  while(totalMl < command->getN()){
    if((millis() - oldTime) > 1000) {
      detachInterrupt(sensorPin);   // Deactivate interrupt for the calculations.
      Serial.print("Pulse : ");
      Serial.println(pulse);
      flowRate = ((1000.0 / (millis() - oldTime))* pulsePerSecond) / calibrationFactor;  
      flowMl = (flowRate/60) * 1000;     // Millilitre per second
      totalMl += flowMl;            // each second add the measured millilitre
      Serial.print("Total Ml : ");
      Serial.println(totalMl);
      pulsePerSecond = 0;
      oldTime = millis();
      attachInterrupt(sensorPin, F02Handler::pulseCount, RISING);
    }
  }
  // Close the water valve
  PinControl::getInstance()->writeValue(valvePin, 0, 0); // Set the pin of the water valve to low (0 = digital).
  Serial.println("F02 is done");
  return 0;
}

void F02Handler::pulseCount() {
  // Increment the pulse counter
  // Serial.println("INTERRUPT");
  pulsePerSecond++;
  pulse++;
}
