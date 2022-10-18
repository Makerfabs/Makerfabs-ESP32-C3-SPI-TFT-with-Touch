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

int list_1[50] = {0};
int list_1_length = 20;
int list_2[50] = {0};
int list_2_length = 20;
int interval = 2000;

void setup(void)
{
    Serial.begin(115200);

    tft_init();
    sensor_init();

    draw_line_chart_window("TVOC Line Chart ( Unit : ppb ) ", (String) "Interval : " + interval + " ms", list_1_length, 0, 1200, TFT_WHITE);
    draw_line_chart_window2("ECO2 Line Chart ( Unit : ppm ) ", (String) "Interval : " + interval + " ms", list_2_length, 0, 5000, TFT_WHITE);
    delay(2000);
}

void loop()
{
    draw_line_chart(list_1, list_1_length, 0, 1200, TFT_WHITE);
    draw_line_chart2(list_2, list_2_length, 0, 5000, TFT_WHITE);
    delay(interval);

    int tvoc = 0;
    int eco2 = 0;
    sensor_read(&tvoc, &eco2);

    draw_line_chart(list_1, list_1_length, 0, 1200, TFT_BLACK);
    draw_line_chart2(list_2, list_2_length, 0, 5000, TFT_BLACK);
    add_list(list_1, list_1_length, tvoc);
    add_list(list_2, list_2_length, eco2);
}

// Sensor
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

void sensor_read(int *tvoc, int *eco2)
{

    float humidity = sht31.readHumidity();
    float temperature = sht31.readTemperature();

    Serial.print("humidity=");
    Serial.println(humidity);
    Serial.print("temperature=");
    Serial.println(temperature);

    sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

    if (!sgp.IAQmeasure())
    {
        Serial.println("Measurement failed");
        return;
    }
    Serial.print("TVOC ");
    Serial.print(*tvoc = sgp.TVOC);
    Serial.print(" ppb\t");
    Serial.print("eCO2 ");
    Serial.print(*eco2 = sgp.eCO2);
    Serial.println(" ppm");
}
// TFT

void tft_init()
{
    lcd.init();
    lcd.fillScreen(TFT_BLACK);

    lcd.setTextSize(2);
    lcd.setTextColor(TFT_WHITE);

    // lcd.fillRect(80, 160, 160, 160, TFT_BLACK);
    // lcd.setCursor(10, 10);
    // lcd.println("Mabee Co2");
    // lcd.setTextSize(2);
}

void draw_line_chart_window(String text1, String text2, int length, int low, int high, int color)
{
    // draw rect and unit
    // lcd.drawRect(20, 20, 280, 200, color);
    lcd.drawLine(30, 20, 30, 220, color);
    lcd.drawLine(30, 220, 300, 220, color);

    lcd.drawLine(20, 20, 30, 20, color);
    lcd.drawLine(20, 120, 30, 120, color);

    lcd.setTextColor(color);
    lcd.setTextSize(1);

    lcd.setCursor(0, 10);
    lcd.println(high);

    lcd.setCursor(80, 10);
    lcd.println(text1);

    lcd.setCursor(0, 110);
    lcd.println((high + low) / 2);

    lcd.setCursor(0, 210);
    lcd.println(low);

    lcd.setCursor(80, 230);
    lcd.println(text2);

    int x_start = 32;
    int x_unit = 280 / (length - 1);
    for (int i = 0; i < length; i++)
    {
        int x = x_start + x_unit * i;
        if (i != 0 && i != length - 1)
            lcd.drawLine(x, 220, x, 225, color);
    }
}

void draw_line_chart(int *list, int length, int low, int high, int color)
{
    // list to position
    int pos[50][2] = {0};
    int detail = 50;
    int x_start = 32;
    int y_start = 218;
    int x_unit = 280 / (length - 1);
    int y_unit = -200 / (detail - 1);
    for (int i = 0; i < length; i++)
    {
        pos[i][0] = x_start + i * x_unit;
        int y = map(*(list + i), low, high, 0, detail);
        if (y > detail)
            y = detail;
        pos[i][1] = y_start + y_unit * y;
    }

    // draw line chart
    for (int i = 0; i < length - 1; i++)
    {
        lcd.drawLine(pos[i][0], pos[i][1], pos[i + 1][0], pos[i + 1][1], color);
    }
}

void draw_line_chart_window2(String text1, String text2, int length, int low, int high, int color)
{
    // draw rect and unit
    // lcd.drawRect(20, 20, 280, 200, color);
    lcd.drawLine(30, 260, 30, 460, color);
    lcd.drawLine(30, 460, 300, 460, color);

    lcd.drawLine(20, 260, 30, 260, color);
    lcd.drawLine(20, 360, 30, 360, color);

    lcd.setTextColor(color);
    lcd.setTextSize(1);

    lcd.setCursor(0, 250);
    lcd.println(high);

    lcd.setCursor(80, 250);
    lcd.println(text1);

    lcd.setCursor(0, 350);
    lcd.println((high + low) / 2);

    lcd.setCursor(0, 450);
    lcd.println(low);

    lcd.setCursor(80, 470);
    lcd.println(text2);

    int x_start = 32;
    int x_unit = 280 / (length - 1);
    for (int i = 0; i < length; i++)
    {
        int x = x_start + x_unit * i;
        if (i != 0 && i != length - 1)
            lcd.drawLine(x, 460, x, 465, color);
    }
}

void draw_line_chart2(int *list, int length, int low, int high, int color)
{
    // list to position
    int pos[50][2] = {0};
    int detail = 50;
    int x_start = 32;
    int y_start = 458;
    int x_unit = 280 / (length - 1);
    int y_unit = -200 / (detail - 1);
    for (int i = 0; i < length; i++)
    {
        pos[i][0] = x_start + i * x_unit;
        int y = map(*(list + i), low, high, 0, detail);
        if (y > detail)
            y = detail;
        pos[i][1] = y_start + y_unit * y;
    }

    // draw line chart
    for (int i = 0; i < length - 1; i++)
    {
        lcd.drawLine(pos[i][0], pos[i][1], pos[i + 1][0], pos[i + 1][1], color);
    }
}

// Data
void add_list(int *list, int length, int num)
{
    for (int i = length - 2; i >= 0; i--)
    {
        *(list + i + 1) = *(list + i);
    }
    *list = num;
}