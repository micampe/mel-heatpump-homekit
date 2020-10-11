#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <MQTTClient.h>
#include <Ticker.h>
#include <Wire.h>

#include "env_sensor.h"
#include "debug.h"
#include "accessory.h"

// FIXME: this should be configurable
#define MQTT_SERVER "192.168.1.249"
#define SAMPLE_INTERVAL 30

// FIXME: name should be configurable
const char *sensorName;

static Adafruit_BME280 bme;

static DHT_Unified dht(D4, DHT22);
static DHT_Unified::Temperature dhtTemperature = dht.temperature();
static DHT_Unified::Humidity dhtHumidity = dht.humidity();

static Adafruit_Sensor *temperatureSensor = nullptr;
static Adafruit_Sensor *humiditySensor = nullptr;

static Ticker ticker;

static WiFiClient net;
static MQTTClient mqtt;

static char *temperatureTopic;
static char *humidityTopic;
static char *dewPointTopic;

static void _updateSensorReading() {
    sensors_event_t temperatureEvent;
    temperatureSensor->getEvent(&temperatureEvent);
    sensors_event_t humidityEvent;
    humiditySensor->getEvent(&humidityEvent);

    float temperature = temperatureEvent.temperature;
    float humidity = humidityEvent.relative_humidity;
    float dewPoint = temperature - ((100 - humidity) / 5);

    // MIE_LOG(" ⮕ HK %.1fºC %.1f%% RH %.1fºC DP", temperature, humidity, dewPoint);

    // looks like I need this workaround?
    while (!mqtt.connect(sensorName)) {
        delay(100);
    }
    mqtt.publish(temperatureTopic, String(temperature));
    mqtt.publish(humidityTopic, String(humidity));
    mqtt.publish(dewPointTopic, String(dewPoint));

    _set_characteristic_float(&ch_dehumidifier_relative_humidity, humidity, true);
    _set_characteristic_float(&ch_dew_point, dewPoint, true);
    if (ch_thermostat_current_temperature.value.float_value < 0.001) {
        // set the thermostat temperature from the sensor only if it doesn't have its own reading
        _set_characteristic_float(&ch_thermostat_current_temperature, temperature, true);
    }
}

void initEnvironmentReporting(const char* ssid) {
    sensorName = ssid;

    asprintf(&temperatureTopic, "home/sensors/%s/temperature", sensorName);
    asprintf(&humidityTopic, "home/sensors/%s/humidity", sensorName);
    asprintf(&dewPointTopic, "home/sensors/%s/dewPoint", sensorName);

    sensors_event_t temperatureEvent;
    sensors_event_t humidityEvent;

    if (bme.begin(BME280_ADDRESS_ALTERNATE)) {
        bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                Adafruit_BME280::SAMPLING_X1,
                Adafruit_BME280::SAMPLING_NONE,
                Adafruit_BME280::SAMPLING_X1,
                Adafruit_BME280::FILTER_OFF,
                Adafruit_BME280::STANDBY_MS_1000);
        // FIXME: this should be configurable
        bme.setTemperatureCompensation(-5);

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
        delay(1000);

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
        MIE_LOG("Starting environment sensor reporting");

        temperatureSensor->printSensorDetails();
        humiditySensor->printSensorDetails();

        Serial.print("Connecting to MQTT broker...");
        mqtt.begin(MQTT_SERVER, net);
        delay(500);
        while (!mqtt.connect(sensorName)) {
            Serial.print(".");
            delay(500);
        }
        Serial.println();

        _updateSensorReading();
        ticker.attach_scheduled(SAMPLE_INTERVAL, _updateSensorReading);
    } else {
        Serial.println(F("No temperature and humidity sensors found"));
        MIE_LOG("No temperature and humidity sensors found");
    }
}
