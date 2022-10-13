#include <SPI.h>
#include <Wire.h>

#include "Adafruit_SGP30.h"
#include "Adafruit_SHT31.h"
#include "C3_SPI_9488.h"

#define I2C_SDA_PIN 2
#define I2C_SCL_PIN 3

Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_SGP30 sgp;
LGFX lcd;

float h = 0.0;
float t = 0.0;

int TVOC = 0;
int eCO2 = 0;
int H2 = 0;
int Ethanol = 0;

void setup(void)
{
    Serial.begin(115200);

    lcd.init();
    lcd.fillScreen(TFT_BLACK);

    lcd.setTextSize(4);
    lcd.setTextColor(TFT_WHITE);

    // lcd.fillRect(80, 160, 160, 160, TFT_BLACK);
    lcd.setCursor(10, 10);
    lcd.println("Mabee Co2");
    lcd.setTextSize(2);

    int x_pos = 10;
    int y = 1;

    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("Humidity");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("Temperature");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("TVOC");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("eCO2");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("Raw H2");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("Raw Ethanol");

    x_pos = 280;
    y = 1;
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("%");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("C");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("ppb");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("ppm");

    sensor_init();

    delay(2000);
}

int counter = 0;
void loop()
{

    float humidity = sht31.readHumidity();
    float temperature = sht31.readTemperature();

    if (!isnan(temperature))
    { // check if 'is not a number'
        Serial.print("Temp *C = ");
        Serial.println(temperature);
        t = temperature;
    }
    else
    {
        Serial.println("Failed to read temperature");
    }

    if (!isnan(humidity))
    { // check if 'is not a number'
        Serial.print("Hum. % = ");
        Serial.println(humidity);
        h = humidity;
    }
    else
    {
        Serial.println("Failed to read humidity");
    }

    // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
    // float temperature = 22.1; // [°C]
    // float humidity = 45.2; // [%RH]
    sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

    if (!sgp.IAQmeasure())
    {
        Serial.println("Measurement failed");
        return;
    }
    Serial.print("TVOC ");
    Serial.print(TVOC = sgp.TVOC);
    Serial.print(" ppb\t");
    Serial.print("eCO2 ");
    Serial.print(eCO2 = sgp.eCO2);
    Serial.println(" ppm");

    if (!sgp.IAQmeasureRaw())
    {
        Serial.println("Raw Measurement failed");
        return;
    }
    Serial.print("Raw H2 ");
    Serial.print(H2 = sgp.rawH2);
    Serial.print(" \t");
    Serial.print("Raw Ethanol ");
    Serial.print(Ethanol = sgp.rawEthanol);
    Serial.println("");

    sensor_display();

    delay(1000);

    counter++;
    if (counter == 30)
    {
        counter = 0;
        uint16_t TVOC_base, eCO2_base;

        if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base))
        {
            Serial.println("Failed to get baseline readings");
            return;
        }
        Serial.print("****Baseline values: eCO2: 0x");
        Serial.print(eCO2_base, HEX);
        Serial.print(" & TVOC: 0x");
        Serial.println(TVOC_base, HEX);
    }
}

/* return absolute humidity [mg/m^3] with approximation formula
 * @param temperature [°C]
 * @param humidity [%RH]
 */
uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                                                                // [mg/m^3]
    return absoluteHumidityScaled;
}

void sensor_init()
{
    Serial.println("Check Sensor");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(100000UL);

    if (!sht31.begin(0x44))
    {
        Serial.println("SHT31 not found.");
    }
    delay(100);

    if (!sgp.begin())
    {
        Serial.println("SGP30 not found.");
    }
}

void sensor_display()
{
    int x_pos = 160;
    int y = 1;

    lcd.fillRect(160, 60, 120, 160, TFT_BLACK);
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println(h);
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println(t);
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println(TVOC);
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println(eCO2);
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println(H2);
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println(Ethanol);
    lcd.setCursor(x_pos, 60 + 20 * y++);
}