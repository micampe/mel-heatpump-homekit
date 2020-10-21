#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Ticker.h>
#include <Wire.h>

#include "env_sensor.h"
#include "debug.h"
#include "accessory.h"
#include "heatpump_client.h"
#include "web.h"

#define SAMPLE_INTERVAL 300

// FIXME: name should be configurable
const char *sensorName;

static Adafruit_BME280 bme;

static DHT_Unified dht(D4, DHT22);
static DHT_Unified::Temperature dhtTemperature = dht.temperature();
static DHT_Unified::Humidity dhtHumidity = dht.humidity();

static Adafruit_Sensor *temperatureSensor = nullptr;
static Adafruit_Sensor *humiditySensor = nullptr;

static Ticker ticker;

WiFiClient net;
MQTTClient mqtt;
char env_sensor_status[30] = {0};

static bool mqttConnect() {
    const int timeout = 3; // 30 ticks = 3000ms
    int tick = 0;
    while (!mqtt.connect(sensorName) && ++tick < timeout) {
        delay(10);
    }

    if (tick < timeout) {
        return true;
    } else {
        // FIXME: report this in the status page
        MIE_LOG("MQTT connection failed");
        return false;
    }
}

bool mqttIsConfigured() {
    return strlen(settings.mqtt_server) 
            && strlen(settings.mqtt_temp)
            && strlen(settings.mqtt_humidity);
}

static void _updateSensorReading() {
    sensors_event_t temperatureEvent;
    temperatureSensor->getEvent(&temperatureEvent);
    sensors_event_t humidityEvent;
    humiditySensor->getEvent(&humidityEvent);

    float temperature = temperatureEvent.temperature;
    float humidity = humidityEvent.relative_humidity;
    float dewPoint = temperature - ((100 - humidity) / 5);

    sensor_t sensor;
    temperatureSensor->getSensor(&sensor);
    snprintf(env_sensor_status, sizeof(env_sensor_status), "%s %.1fºC %.1f%%RH", sensor.name, temperature, humidity);

    if (mqttIsConfigured() && mqttConnect()) {
        char str[6];
        snprintf(str, 6, "%.1f", temperature);
        mqtt.publish(settings.mqtt_temp, str);
        snprintf(str, 6, "%.1f", humidity);
        mqtt.publish(settings.mqtt_humidity, str);
        snprintf(str, 6, "%.1f", dewPoint);
        mqtt.publish(settings.mqtt_dew_point, str);
    }

    _set_characteristic_float(&ch_dehumidifier_relative_humidity, humidity, true);
    _set_characteristic_float(&ch_dew_point, dewPoint, true);
    if (!heatpump.isConnected()) {
        _set_characteristic_float(&ch_thermostat_current_temperature, temperature, true);
    }
}

void initEnvironmentReporting(const char* ssid) {
    sensorName = ssid;

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
            Serial.printf("Found BME280 sensor: %.1fºC %.1f%% RH\n",
                    temperatureEvent.temperature,
                    humidityEvent.relative_humidity);
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
            Serial.printf("Found DHT22 sensor: %.1fºC %.1f%% RH\n",
                    temperatureEvent.temperature,
                    humidityEvent.relative_humidity);
            MIE_LOG("Found DHT22 sensor: %.1fºC %.1f%% RH",
                    temperatureEvent.temperature,
                    humidityEvent.relative_humidity);

            temperatureSensor = &dhtTemperature;
            humiditySensor = &dhtHumidity;
        }
    }

    if (temperatureSensor && humiditySensor) {
        temperatureSensor->printSensorDetails();
        humiditySensor->printSensorDetails();

        if (mqttIsConfigured()) {
            Serial.println("Connecting to MQTT broker...");
            MIE_LOG("Connecting to MQTT broker...");
            mqtt.begin(settings.mqtt_server, settings.mqtt_port, net);
            mqttConnect();
        } else {
            Serial.println("MQTT reporting not configured");
            MIE_LOG("MQTT reporting not configured");
        }

        _updateSensorReading();
        ticker.attach_scheduled(SAMPLE_INTERVAL, _updateSensorReading);
    } else {
        Serial.println(F("No temperature and humidity sensors found"));
        MIE_LOG("No temperature and humidity sensors found");
    }
}
