
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <SPI.h>                     //
#include <Wire.h>
#include <Adafruit_GFX.h>            //oled
#include <Adafruit_SSD1306.h>        //oled
#include <DHT.h>                     // Including library for dht
#include <WiFiUdp.h>
#include <ThingsBoard.h>
#include <JsonListener.h>
#include <time.h>
#include "OpenWeatherMapForecast.h"

float is_rain; //show weather status
String realtime="";  //show time for time and date

// initiate the client
OpenWeatherMapForecast clientt;

// weather api
String OPEN_WEATHER_MAP_APP_ID = "fba194ec809cd62f889d17cfe92068e1";   //token
String OPEN_WEATHER_MAP_LOCATION_ID = "142358";                        //id for Babolsar
String OPEN_WEATHER_MAP_LANGUAGE = "en";                               //language
boolean IS_METRIC = true;                                              //centigrade
uint8_t MAX_FORECASTS = 3;                                             // 2 Days later


//time
const char* ntpServer = "europe.pool.ntp.org"; 
const long  gmtOffset_sec = 16200;   //Replace with your GMT offset (seconds)
const int   daylightOffset_sec = 0;  //Replace with your daylight offset (seconds)

//internet connection
const char* ESP_HOST_NAME = "esp-" + ESP.getFlashChipId(); //chip id
#define WIFI_AP "Zi"
#define WIFI_PASSWORD "12345678"
WiFiClient client;


//platform
#define TOKEN "teoxLA8CInMUF4uzPYkY"                       // platform token
char thingsboardServer[] = "platform.smbapp.ir";
WiFiClient wifiClient;
ThingsBoard tb(wifiClient);


//oled
#define SCREEN_WIDTH 128                               // OLED display width, in pixels
#define SCREEN_HEIGHT 64                               // OLED display height, in pixels
#define OLED_RESET -1                                  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//dht
#define DHTPIN D3 
DHT dht(DHTPIN, DHT11);


int status = WL_IDLE_STATUS;
unsigned long lastSend;

//pin
const int AirValue = 790;   //you need to replace this value with Value_1
const int WaterValue = 390;  //you need to replace this value with Value_2
const int moisturePin = A0;
int soilMoistureValue = 0;
int soilmoisturepercent = 0;
int relaypin = D5;

void setup()
{
  Serial.begin(115200);
  delay(10);
  InitWiFi();
  lastSend = 0;
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);                //initialize with the I2C addr 0x3C (128x64)
  display.clearDisplay();
  pinMode(relaypin, OUTPUT);
  Serial.println();
  Serial.println("\n\nNext Loop-Step: " + String(millis()) + ":");
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  dht.begin();
}

void loop()
{
   soilMoistureValue = analogRead(moisturePin);  //put Sensor insert into soil
   //Serial.println(soilMoistureValue);
   float h = dht.readHumidity();
   float t = dht.readTemperature();
   Serial.println(realtime.substring(11,16));
    //  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
    soilmoisturepercent = ( 100.00 - ( (analogRead(moisturePin) / 1023.00) * 100.00 ) );
    delay(1000);
    printLocalTime();
    if(soilmoisturepercent > 100)
{
  Serial.println("100 %");
  
  display.setCursor(0,0);  //oled display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print("Soil RH:");
  display.setTextSize(1);
  display.print("100");
  display.println(" %");
  display.setCursor(0,20);  //oled display
  display.setTextSize(2);
  display.print("Air RH:");
  display.setTextSize(1);
  display.print(h);
  display.println(" %");
  display.setCursor(0,40);  //oled display
  display.setTextSize(2);
  display.print("Temp:");
  display.setTextSize(1);
  display.print(t);
  display.println(" C");
  display.display();
  
  delay(1000);
  display.clearDisplay();
}
 
 
else if(soilmoisturepercent <0)
{
  Serial.println("0 %");
  
  display.setCursor(0,0);  //oled display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print("Soil RH:");
  display.setTextSize(1);
  display.print("0");
  display.println(" %");
  display.setCursor(0,20);  //oled display
  display.setTextSize(2);
  display.print("Air RH:");
  display.setTextSize(1);
  display.print(h);
  display.println(" %");
  display.setCursor(0,40);  //oled display
  display.setTextSize(2);
  display.print("Temp:");
  display.setTextSize(1);
  display.print(t);
  display.println(" C");
  display.display();
 
  delay(1000);
  display.clearDisplay();
}
 
 
else if(soilmoisturepercent >=0 && soilmoisturepercent <= 100)
{
  Serial.print(soilmoisturepercent);
  Serial.println("%");

  //  time 
     //  time 
  //timecc.update();
  display.setCursor(0,0);  //oled display
  Serial.println(realtime.substring(11,16));
  display.print("time:");
  display.setTextSize(1);
  display.print(realtime.substring(11,16));
  display.setTextSize(1);
  display.setCursor(0,15);  //oled display
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("moisture:");
  display.setTextSize(1);
  display.print(soilmoisturepercent);
  display.println(" %");
  display.setCursor(0,30);  //oled display
  display.setTextSize(1);
  display.print("Humidity:");
  display.setTextSize(1);
  display.print(h);
  display.println(" %");
  display.setCursor(0,45);  //oled display
  display.setTextSize(1);
  display.print("temp:");
  display.setTextSize(1);
  display.print(t);
  display.println(" C");
  display.display();
  display.clearDisplay();
  
}
 

if (realtime.substring(11,16) > "12:00" && realtime.substring(11,16) < "13:00"){
  digitalWrite(relaypin, LOW);
  Serial.println("pump is off");
}
else if (is_rain >=0)
{
  digitalWrite(relaypin, LOW); 
  Serial.println("pump is off");
}
else
if(soilmoisturepercent <= 20){
  digitalWrite(relaypin, HIGH); 
  Serial.println("pump is on");                  
 
  }
else if (soilmoisturepercent >20){
    digitalWrite(relaypin, LOW); 
    Serial.println("pump is off");
    delay(4000);
 }
    
  if ( !tb.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
    getAndSendMQ4Data();
    lastSend = millis();
  }

  tb.loop();
  
}

void getAndSendMQ4Data()
{
  
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    OpenWeatherMapForecastData data[MAX_FORECASTS];
    clientt.setMetric(IS_METRIC);
    clientt.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
    uint8_t allowedHours[] = {0,12};
    clientt.setAllowedHours(allowedHours, 2);
    uint8_t foundForecasts = clientt.updateForecastsById(data, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID, MAX_FORECASTS);
    Serial.printf("Found %d forecasts in this call\n", foundForecasts);
    Serial.println("------------------------------------");
    time_t time;
    for (uint8_t i = 0; i < foundForecasts; i++) {
        Serial.printf("---\nForecast number: %d\n", i);
        // {"dt":1527066000, uint32_t observationTime;
        time = data[i].observationTime;
        Serial.printf("observationTime: %d, full date: %s", data[i].observationTime, ctime(&time));
        // "main":{
        //   "temp":17.35, float temp;
        Serial.printf("temp: %f\n", data[i].temp);
        //   "feels_like": 16.99, float feelsLike;
        //Serial.printf("feels-like temp: %f\n", data[i].feelsLike);
        //   "temp_min":16.89, float tempMin;
        Serial.printf("tempMin: %f\n", data[i].tempMin);
        tb.sendTelemetryFloat("tempMin", data[i].tempMin);
        //   "temp_max":17.35, float tempMax;
        Serial.printf("tempMax: %f\n", data[i].tempMax);
        tb.sendTelemetryFloat("tempMax", data[i].tempMax);
        //   "pressure":970.8, float pressure;
        Serial.printf("pressure: %f\n", data[i].pressure);
        tb.sendTelemetryFloat("pressure", data[i].pressure);
        //   "sea_level":1030.62, float pressureSeaLevel;
        Serial.printf("pressureSeaLevel: %f\n", data[i].pressureSeaLevel);
        tb.sendTelemetryFloat("pressureSeaLevel", data[i].pressureSeaLevel);
        //   "grnd_level":970.8, float pressureGroundLevel;
        Serial.printf("pressureGroundLevel: %f\n", data[i].pressureGroundLevel);
        tb.sendTelemetryFloat("pressureGroundLevel", data[i].pressureGroundLevel);
        //   "humidity":97, uint8_t humidity;
        Serial.printf("humidity: %d\n", data[i].humidity);
        //   "temp_kf":0.46
        Serial.printf("main: %s\n", data[i].main.c_str());
        tb.sendTelemetryString("weather status", data[i].main.c_str());
        //   "description":"scattered clouds", String description;
        Serial.printf("description: %s\n", data[i].description.c_str());
        // }],"clouds":{"all":44}, uint8_t clouds;
        Serial.printf("clouds: %d\n", data[i].clouds);
        tb.sendTelemetryFloat("cloud status", data[i].clouds);
        // "wind":{
        //   "speed":1.77, float windSpeed;
        Serial.printf("windSpeed: %f\n", data[i].windSpeed);
        tb.sendTelemetryFloat("windSpeed", data[i].windSpeed);
        //   "deg":207.501 float windDeg;
        Serial.printf("windDeg: %f\n", data[i].windDeg);
         tb.sendTelemetryFloat("windSpeed", data[i].windSpeed);
        // rain: {3h: 0.055}, float rain;
        Serial.printf("rain: %f\n", data[i].rain);
        tb.sendTelemetryFloat("rain", data[i].rain);
        is_rain=data[i].rain;
        // },"sys":{"pod":"d"}
        // dt_txt: "2018-05-23 09:00:00"   String observationTimeText;
        Serial.printf("observationTimeText: %s\n", data[i].observationTimeText.c_str());
          /*if (data[i].rain >= 0)
        {
       digitalWrite(relaypin, LOW); 
       Serial.println("pump is off");
        }*/
}
    
    
    Serial.print("Humidity: ");
    Serial.println(h);
    Serial.print("Temperature: ");
    Serial.println(t);

    soilMoistureValue = analogRead(moisturePin);  //put Sensor insert into soil
    Serial.println(soilMoistureValue);

    //soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
    soilmoisturepercent = ( 100.00 - ( (analogRead(moisturePin) / 1023.00) * 100.00 ) );

  Serial.println("Sending to ThingsBoard..");

  tb.sendTelemetryFloat("Temperature", t);
  tb.sendTelemetryFloat("Humidity", h);
  tb.sendTelemetryFloat("soilmoisturepercent", soilmoisturepercent);
  Serial.println("Sent!");

  
//}
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}


void reconnect() {
  // Loop until we're reconnected
  while (!tb.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      // Code to connect to WiFi
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
      Serial.println("");
      Serial.println("WiFi connected!");
      Serial.println(WiFi.localIP());
      Serial.println();
    }

    // Code to connect to Thingsboard
    Serial.print("Connecting to ThingsBoard node ...");
    if ( tb.connect(thingsboardServer, TOKEN) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED]" );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}
void printLocalTime()
{
  time_t rawtime;
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  realtime = asctime(timeinfo);
  Serial.println(asctime(timeinfo));
  delay(1000);
}
