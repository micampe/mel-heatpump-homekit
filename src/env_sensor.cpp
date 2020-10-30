#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Ticker.h>
#include <Wire.h>

#include "accessory.h"
#include "debug.h"
#include "env_sensor.h"
#include "heatpump_client.h"
#include "mqtt.h"
#include "web.h"

#define SAMPLE_INTERVAL 10

static Adafruit_BME280 bme;

static DHT_Unified dht(D4, DHT22);
static DHT_Unified::Temperature dhtTemperature = dht.temperature();
static DHT_Unified::Humidity dhtHumidity = dht.humidity();

static Adafruit_Sensor *temperatureSensor = nullptr;
static Adafruit_Sensor *humiditySensor = nullptr;

static Ticker ticker;

char env_sensor_status[30] = {0};

static double dew_point(double t, double r) {
    // Magnus-Tetens approximation
    double a = 17.27;
    double b = 237.7;
    double alpha = ((a * t) / (b + t)) + log(r / 100);
    return (b * alpha) / (a - alpha);
}

static void env_sensor_update() {
    sensors_event_t temperatureEvent;
    temperatureSensor->getEvent(&temperatureEvent);
    sensors_event_t humidityEvent;
    humiditySensor->getEvent(&humidityEvent);

    float temperature = temperatureEvent.temperature;
    float humidity = humidityEvent.relative_humidity;
    float dewPoint = dew_point(temperature, humidity);

    sensor_t sensor;
    temperatureSensor->getSensor(&sensor);
    snprintf(env_sensor_status, sizeof(env_sensor_status), "%s %.2fºC %.2f%%RH", sensor.name, temperature, humidity);

    if (mqtt_connect()) {
        char str[6];
        if (strlen(settings.mqtt_temp) && std::isnormal(temperature)) {
            snprintf(str, sizeof(str), "%.2f", temperature);
            mqtt.publish(settings.mqtt_temp, str);
        }
        if (strlen(settings.mqtt_humidity) && std::isnormal(humidity)) {
            snprintf(str, sizeof(str), "%.2f", humidity);
            mqtt.publish(settings.mqtt_humidity, str);
        }
        if (strlen(settings.mqtt_dew_point) && std::isnormal(dewPoint)) {
            snprintf(str, sizeof(str), "%.2f", dewPoint);
            mqtt.publish(settings.mqtt_dew_point, str);
        }
    }

    accessory_set_float(&ch_dehumidifier_relative_humidity, humidity, true);
    accessory_set_float(&ch_dew_point, dewPoint, true);
    if (!heatpump.isConnected()) {
        accessory_set_float(&ch_thermostat_current_temperature, temperature, true);
    }
}

void env_sensor_init() {
    sensors_event_t temperatureEvent;
    sensors_event_t humidityEvent;

    if (bme.begin(BME280_ADDRESS_ALTERNATE)) {
        bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                Adafruit_BME280::SAMPLING_X1,
                Adafruit_BME280::SAMPLING_NONE,
                Adafruit_BME280::SAMPLING_X1,
                Adafruit_BME280::FILTER_OFF,
                Adafruit_BME280::STANDBY_MS_1000);

        temperatureSensor = bme.getTemperatureSensor();
        humiditySensor = bme.getHumiditySensor();

        temperatureSensor->getEvent(&temperatureEvent);
        humiditySensor->getEvent(&humidityEvent);

        if (!isnan(temperatureEvent.temperature) && !isnan(humidityEvent.relative_humidity)) {
            strlcpy(env_sensor_status, "BME280", sizeof(env_sensor_status));
            MIE_LOG("Found BME280 sensor: %.1fºC %.1f%% RH",
                    temperatureEvent.temperature,
                    humidityEvent.relative_humidity);
        } else {
            temperatureSensor = nullptr;
            humiditySensor = nullptr;
        }
    } else {
        dht.begin();
        sensor_t sensor;
        dhtTemperature.getSensor(&sensor);
        delay(sensor.min_delay / 1000);

        dhtTemperature.getEvent(&temperatureEvent);
        dhtHumidity.getEvent(&humidityEvent);

        if (!isnan(temperatureEvent.temperature) && !isnan(humidityEvent.relative_humidity)) {
            strlcpy(env_sensor_status, "DHT22", sizeof(env_sensor_status));
            MIE_LOG("Found DHT22 sensor: %.1fºC %.1f%% RH",
                    temperatureEvent.temperature,
                    humidityEvent.relative_humidity);

            temperatureSensor = &dhtTemperature;
            humiditySensor = &dhtHumidity;
        }
    }

    if (temperatureSensor && humiditySensor) {
        ticker.attach_scheduled(SAMPLE_INTERVAL, env_sensor_update);
    } else {
        MIE_LOG("No temperature and humidity sensors found");
    }
}
