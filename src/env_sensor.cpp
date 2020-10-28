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

static double apparent_temperature(double t, double r) {
    // Rothfusz approximation
    double c1 = -8.78469475556;
    double c2 = 1.61139411;
    double c3 = 2.33854883889;
    double c4 = -0.14611605;
    double c5 = -0.012308094;
    double c6 = -0.0164248277778;
    double c7 = 0.002211732;
    double c8 = 0.00072546;
    double c9 = -0.000003582;
    double t2 = t * t;
    double r2 = r * r;

    return c1 + c2 * t + c3 * r + c4 * t * r + c5 * t2 + c6 * r2 + c7 * t2 * r + c8 * t * r2 + c9 * t2 * r2;
}

static void env_sensor_update() {
    sensors_event_t temperatureEvent;
    temperatureSensor->getEvent(&temperatureEvent);
    sensors_event_t humidityEvent;
    humiditySensor->getEvent(&humidityEvent);

    float temperature = temperatureEvent.temperature;
    float humidity = humidityEvent.relative_humidity;
    float dewPoint = dew_point(temperature, humidity);
    float appTemp = apparent_temperature(temperature, humidity);

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
        if (strlen(settings.mqtt_app_temp) && std::isnormal(appTemp)) {
            snprintf(str, sizeof(str), "%.2f", appTemp);
            mqtt.publish(settings.mqtt_app_temp, str);
        }
    }

    accessory_set_float(&ch_dehumidifier_relative_humidity, humidity, true);
    accessory_set_float(&ch_dew_point, dewPoint, true);
    accessory_set_float(&ch_apparent_temp, appTemp, true);
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
