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
#define DEBUG 1
#define powerPin 14
#define ledFanPin 12 // Đèn dùng để thay thế quạt
#define reactPin 13
#define fanPin 15

bool isDeviceOn = false;
bool isFanOn = false;

float temperature = 0.0;
float humidity = 0.0;
float tempControl = 0.0;

// int powerPin = 14;
// int ledFanPin = 12;
// int reactPin = 13;
// int fanPin = 15;

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "CIT";
char pass[] = "";

BlynkTimer timer;

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  int value = param.asInt();
  // Update state
  if (value == 1)
  {
    turnOn();
  }
  else
  {
    turnOff();
  }
}
BLYNK_WRITE(V4)
{
  tempControl = param.asFloat();
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
void getTemperature()
{
  temperature = dht.readTemperature();
  if (DEBUG)
  {
    Serial.print("Nhiet do: ");
    Serial.print(temperature);
    Serial.println(" C");
  }
}
void getHumidity()
{
  humidity = dht.readHumidity();
  if (DEBUG)
  {
    Serial.print("Do am: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
}
void turnOn()
{
  digitalWrite(powerPin, HIGH);
  Blynk.virtualWrite(V1, 1);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("May dang bat!");
  isDeviceOn = true;
}
void turnOff()
{
  digitalWrite(powerPin, LOW);
  digitalWrite(ledFanPin, LOW);
  digitalWrite(reactPin, LOW);
  Blynk.virtualWrite(V1, 0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("May da tat!");
  isDeviceOn = false;
}

void printLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nhiet do: ");
  lcd.print(temperature, 1);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Do am: ");
  lcd.print(humidity, 1);
  lcd.print("%");
}
void sendSensor()
{
  getHumidity();
  getTemperature();
  if (isnan(humidity) || isnan(temperature))
  {
    if (DEBUG)
      Serial.println("Khong doc duoc DHT!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Loi cam bien!");
    return;
  }
  printLCD();
  Blynk.virtualWrite(V6, temperature);
  Blynk.virtualWrite(V7, humidity);
}
void controlTemperature()
{
  if (!isDeviceOn)
    return;

  sendSensor();

  if (isnan(temperature)) {
    if (DEBUG)
      Serial.println("Loi DHT!");
    digitalWrite(ledFanPin, LOW);
    digitalWrite(reactPin, LOW);
    return;
  }

  if (temperature > tempControl)
  {
    digitalWrite(ledFanPin, HIGH); // Bật đèn báo quạt
    digitalWrite(reactPin, LOW);   // Tắt đèn báo đạt nhiệt độ
    if (DEBUG) {
      Serial.println("Quat dang hoat dong de lam mat phong.");
      Serial.println(tempControl);
    }
  }
  else
  {
    digitalWrite(ledFanPin, LOW); // Tắt đèn báo quạt
    digitalWrite(reactPin, HIGH); // Bật đèn báo đạt nhiệt độ
    if (DEBUG) {
      Serial.println("Nhiet do da dat nguong mong muon.");
      Serial.println("Quat se quay o che do cham.");
      Serial.println(tempControl);
    }
  }
}


void adjustFanSpeed()
{
  if (!isDeviceOn)
  {
    analogWrite(fanPin, 0);
    digitalWrite(ledFanPin, LOW);
    return;
  }

  float diff = temperature - tempControl;

  if (diff < 0)
  {
    analogWrite(fanPin, 0);
    digitalWrite(ledFanPin, LOW);
    if (DEBUG)
      Serial.println("Temp below setpoint - Fan OFF");
  }
  else if (diff == 0)
  {
    analogWrite(fanPin, 200); // Quạt quay nhẹ giữ nhiệt độ
    digitalWrite(ledFanPin, HIGH);
    if (DEBUG)
      Serial.println("Temp at setpoint - Fan LOW");
  }
  else
  {
    int fanSpeed = map(diff * 10, 0, 50, 200, 1023); // PWM tăng theo độ lệch
    fanSpeed = constrain(fanSpeed, 200, 1023);
    analogWrite(fanPin, fanSpeed);
    digitalWrite(ledFanPin, HIGH);
    if (DEBUG)
    {
      Serial.print("Temp above setpoint - Fan Speed: ");
      Serial.println(fanSpeed);
    }
  }
}

void setup()
{
  // Debug console
  Serial.begin(9600);
  dht.begin();
  pinMode(powerPin, OUTPUT);
  pinMode(ledFanPin, OUTPUT);
  pinMode(reactPin, OUTPUT);
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
  timer.setInterval(5000L, controlTemperature);
}

void loop()
{
  Blynk.run();
  timer.run();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}
