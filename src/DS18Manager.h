#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <functional>  // Needed for std::function

namespace Olon {

class DS18Manager {
  private:
    using OnCompleteCallback = std::function<void(bool success)>;
    OneWire oneWire;
    DallasTemperature sensors;
    uint8_t oneWirePin;
    uint8_t resolution;
    unsigned long timeout;  // Timeout duration in milliseconds
    unsigned long lastRequestTime;
    bool requestInProgress;
    bool timeoutEnabled;
    OnCompleteCallback onCompleteCallback = nullptr;

  public:
    DS18Manager() : oneWire(0), sensors(&oneWire) {
      resolution = 12;  // Default to 12-bit resolution
      timeout = 1000;   // Default timeout of 1 second
      requestInProgress = false;
      timeoutEnabled = true;
      onCompleteCallback = nullptr;
    }

    // Initialize the DS18B20 on the specified 1-wire pin
    void begin(uint8_t pin1wire) {
      oneWirePin = pin1wire;
      oneWire = OneWire(oneWirePin);
      sensors.setOneWire(&oneWire);
      sensors.begin();
      setGlobalResolution(12);  // Set default resolution

      // Ensure non-blocking temperature conversion
      sensors.setWaitForConversion(false);
    }

    // Get the number of sensors connected
    uint8_t getSensorCount() {
      // sensors.begin();
      return sensors.getDeviceCount();
    }

    // Set the timeout for data reading
    void setTimeout(unsigned long duration) {
      timeout = duration;
    }

    // Enable or disable timeout mechanism
    void enableTimeout(bool enable) {
      timeoutEnabled = enable;
    }

    // Get the global resolution of DS18B20 sensors
    uint8_t getGlobalResolution() {
      return resolution;
    }

    // Set the global resolution for all sensors.
    // The resolution of the temperature sensor is user-configurable to 9, 10, 11, or 12 bits,
    // resolution | increments | sample duration
    //  9         | 0.5°C      | 93.75ms
    // 10         | 0.25°C     | 187.5ms
    // 11         | 0.125°C    | 375ms
    // 12         | 0.0625°C   | 750ms
    bool setGlobalResolution(uint8_t newResolution) {
      if (newResolution < 9 || newResolution > 12) {
        return false;  // Invalid resolution
      }
      resolution = newResolution;
      sensors.setResolution(resolution);
      return true;
    }

    // Set the callback function to be called when data is read or timeout occurs
    void onComplete(OnCompleteCallback callback) {
      onCompleteCallback = callback;
    }

    // Start a temperature reading request for all sensors
    void requestTemperatures() {
      sensors.requestTemperatures();  // Request temperatures (non-blocking)
      lastRequestTime = millis();
      requestInProgress = true;
    }

    // Function to be called periodically to check if data is read or timed out
    void loop() {
      if (requestInProgress) {
        unsigned long currentTime = millis();
        if (sensors.isConversionComplete()) {
          requestInProgress = false;
          if (onCompleteCallback) {
            onCompleteCallback(true);  // Data successfully read
          }
        } else if (timeoutEnabled && (currentTime - lastRequestTime >= timeout)) {
          requestInProgress = false;
          if (onCompleteCallback) {
            onCompleteCallback(false);  // Timeout occurred
          }
        }
      }
    }

    // Get the temperature from a specific sensor by index
    float getTemperatureByIndex(uint8_t index) {
      if (index < sensors.getDeviceCount()) {
        return sensors.getTempCByIndex(index);
      }
      return DEVICE_DISCONNECTED_C;  // Return if sensor is out of bounds
    }

    // Get the address of a sensor by index (useful for identifying sensors)
    bool getAddress(uint8_t index, DeviceAddress deviceAddress) {
      return sensors.getAddress(deviceAddress, index);
    }

    // Print the address of a sensor
    void printAddress(DeviceAddress deviceAddress) {
      for (uint8_t i = 0; i < 8; i++) {
        if (deviceAddress[i] < 16) Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
      }
    }

    // Convert the device address to a string and return it
    const char* getAddressAsString(uint8_t index) {
      static char addrStr[17];  // 16 characters + null terminator
      DeviceAddress deviceAddress;

      // Ensure the address is valid
      if (!getAddress(index, deviceAddress)) {
        return nullptr;  // Return null if the address is invalid
      }

      // Convert the address to a string
      for (uint8_t i = 0; i < 8; i++) {
        sprintf(&addrStr[i * 2], "%02X", deviceAddress[i]);
      }

      addrStr[16] = '\0';  // Null-terminate the string
      return addrStr;
    }

    const char* convertDeviceAddressToString(const DeviceAddress address) {
        static char str[17]; // 8 bytes * 2 characters per byte + 1 for null terminator
        for (uint8_t i = 0; i < 8; i++) {
            sprintf(&str[i * 2], "%02X", address[i]);
        }
        str[16] = '\0'; // Add the null terminator
        return str;
    }

};

}; // namespace Olon