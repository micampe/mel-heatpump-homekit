#include <BME280I2C.h>
#include <Ticker.h>
#include <Wire.h>
#include <EnvironmentCalculations.h>

#include "humidity.h"
#include "debug.h"
#include "accessory.h"

BME280I2C bme;
Ticker humidityTicker;

void _updateHumidity() {
    float temperature, humidity, pressure;
    bme.read(pressure, temperature, humidity);
    float dewPoint = EnvironmentCalculations::DewPoint(temperature, humidity);
    MIE_LOG(" ⮕ HK temp %.1fºC hum %.1f%% dew %.1fºC press %.1fPa", temperature, humidity, dewPoint, pressure);

    _set_characteristic_float(&ch_dehumidifier_relative_humidity, humidity, true);
    _set_characteristic_float(&ch_dew_point, dewPoint, true);
    // _set_characteristic_float(&ch_thermostat_current_temperature, temperature, true);
}

void initHumidityReporting() {
    Wire.begin();
    bme.begin();
    if (bme.chipModel() == BME280::ChipModel_BME280) {
        MIE_LOG("Found BME280 sensor");
        _updateHumidity();
        humidityTicker.attach_scheduled(600, _updateHumidity);
    } else {
        MIE_LOG("BME280 sensor not found");
    }
}
