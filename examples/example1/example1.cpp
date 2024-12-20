#include <Arduino.h>
#include "DS18Manager.h"

// Pin connected to the DS18B20 sensors
#define ONE_WIRE_BUS D5

Olon::DS18Manager temperatureManager;

unsigned long lastReadingTime = 0;
const unsigned long readingInterval = 10000;  // 10 seconds interval

// Callback function that will be called when temperature data is ready or a timeout occurs
void onTemperatureRead(bool success) {
  // Check if any sensors are detected before reporting a success
  uint8_t sensorCount = temperatureManager.getSensorCount();

  if (success && sensorCount > 0) {
    Serial.println("Temperature reading successful!");

    Serial.print("Number of sensors: ");
    Serial.println(sensorCount);

    // Loop through all sensors and print their address and temperature
    for (uint8_t i = 0; i < sensorCount; i++) {
      DeviceAddress sensorAddress;
      if (temperatureManager.getAddress(i, sensorAddress)) {
        Serial.printf("Sensor %d [%s]: %.2f\n", i, temperatureManager.convertDeviceAddressToString(sensorAddress), temperatureManager.getTemperatureByIndex(i));
      } else {
        Serial.printf("Failed to get address for sensor %d", i);
      }
    }
  } else {
    // Either a timeout occurred or no sensors are connected
    if (sensorCount == 0) {
      Serial.println("No sensors detected!");
    } else {
      Serial.println("Temperature reading timed out!");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize the temperature manager
  temperatureManager.begin(ONE_WIRE_BUS);

  // Set the global resolution
  temperatureManager.setGlobalResolution(12);

  // Set a timeout of 2 seconds
  temperatureManager.setTimeout(2000);

  // Register a callback function for when the data is ready or timeout occurs
  temperatureManager.onComplete(onTemperatureRead);

  // Start temperature readings immediately
  temperatureManager.requestTemperatures();
}

void loop() {
  // Periodically check if temperature data is ready or timeout has occurred
  temperatureManager.loop();

  // Check if 10 seconds have passed since the last reading
  unsigned long currentMillis = millis();
  if (currentMillis - lastReadingTime >= readingInterval) {
    lastReadingTime = currentMillis;

    // Request new temperature readings
    temperatureManager.requestTemperatures();
    Serial.println("Requested temperature readings...");
  }
}
