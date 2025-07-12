/*************************************************************

  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6IPJ2oHec"
#define BLYNK_TEMPLATE_NAME "Air conditioner Controller"
#define BLYNK_AUTH_TOKEN "lgzolWzOdnkf53audkCGysWFSWk1u91V"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 2
#define DHTTYPE DHT22
#define DEBUG 1 // Set to 1 to enable debug prints, 0 to disable

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "CIT";
char pass[] = "";

int ledPin = 14;

BlynkTimer timer;

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  int value = param.asInt();
  digitalWrite(ledPin, value);
  // Update state
  Blynk.virtualWrite(V1, value);
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, millis() / 1000);
}
float getTemperature()
{
  float temperature = dht.readTemperature();
  if (isnan(temperature))
  {
    Serial.println("Khong doc duoc DHT!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Loi cam bien!");
    return -1;
  }
  if (DEBUG)
  {
    Serial.print("Nhiet do: ");
    Serial.print(temperature);
    Serial.println(" C");
  }
  return temperature;
}
float getHumidity()
{
  float humidity = dht.readHumidity();
  if (isnan(humidity))
  {
    Serial.println("Khong doc duoc DHT!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Loi cam bien!");
    return -1;
  }
  if (DEBUG)
  {
    Serial.print("Do am: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
  return humidity;
}
void turnOn()
{
  digitalWrite(ledPin, HIGH);
  Blynk.virtualWrite(V1, 1);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("May da bat!");
}
void sendSensor()
{
  float humidity = getHumidity();
  float temperature = getTemperature();
  if (humidity == -1 || temperature == -1)
  {
    Serial.println("Khong doc duoc DHT!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Loi cam bien!");
    return;
  }
  Blynk.virtualWrite(V6, temperature);
  Blynk.virtualWrite(V7, humidity);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Do am: ");
  lcd.print(humidity, 1);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Nhiet do: ");
  lcd.print(temperature, 1);
  lcd.print("C");
}
void setup()
{
  // Debug console
  Serial.begin(9600);
  dht.begin();
  pinMode(ledPin, OUTPUT);
  Wire.begin(4, 5);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // You can also specify server:
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);
  timer.setInterval(4000L, sendSensor);
}

void loop()
{
  Blynk.run();
  timer.run();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}
