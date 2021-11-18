
/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/
*********/

// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

WiFiClient wifiClient;



// Replace with your network credentials
const char* ssid =      "Masonet";           //insert wifi ssid here
const char* password =  "19masonst-internet";       //insert wifi password here

// IFTTT URLs here
//String heatOn     = "insert releavent ifttt url here"; //Leave string empty if no URL available. URL looks like http://maker.ifttt.com/trigger/custom_ifttt_trgr_name/with/key/full_keyhere
//String heatOff    = "insert releavent ifttt url here"; //Leave string empty if no URL available. URL looks like http://maker.ifttt.com/trigger/custom_ifttt_trgr_name/with/key/full_keyhere
//String coldOn     = "insert releavent ifttt url here"; //Leave string empty if no URL available. URL looks like http://maker.ifttt.com/trigger/custom_ifttt_trgr_name/with/key/full_keyhere
//String coldOff    = "insert releavent ifttt url here"; //Leave string empty if no URL available. URL looks like http://maker.ifttt.com/trigger/custom_ifttt_trgr_name/with/key/full_keyhere
String humiOn     = "https://maker.ifttt.com/trigger/low_humidity/with/key/-1_uWR1Mrf-3urRoS25bN"; //Leave string empty if no URL available. URL looks like http://maker.ifttt.com/trigger/custom_ifttt_trgr_name/with/key/full_keyhere
String humiOff    = "https://maker.ifttt.com/trigger/high_humidity/with/key/-1_uWR1Mrf-3urRoS25bN"; //Leave string empty if no URL available. URL looks like http://maker.ifttt.com/trigger/custom_ifttt_trgr_name/with/key/full_keyhere
String serverPath = "";

// ac, heat and humidity control flags
//const int ac_control    = 0 //1-Enable, 0-Disable control
//const int heat_control  = 0 //1-Enable, 0-Disable control
const int humi_control  = 1; //1-Enable, 0-Disable control

// Heat, AC , Humidity Thresholds here [on_threshold, off_threshold]
//heat_temp      = [16, 18]   //in degC
//cold_temp      = [18, 16]   //in degC
float humi_threshold[2] = {70, 90};   //in percentage

#define LEDpin D4   //set led pin
#define DHTPIN D1     // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float dht_t = 0.0;
float dht_h = 0.0;
float worksornot = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 1000;  

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP8266 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
  <p>
  <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
  <span class="dht-labels">Temperature</span> 
  <span id="worksornot">%WORKSORNOT%</span>
  <sup class="units">!</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("worksornot").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/worksornot", true);
  xhttp.send();
}, 10000 ) ;

</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var)
{
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(dht_t);
  }
  else if(var == "HUMIDITY"){
    return String(dht_h);
  }
  else if(var == "WORKSORNOT"){
    return String(worksornot);
  }
  return String();
}

// Ardurino Initialization
void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println("Connecting to WiFi");

  dht.begin();
  
  // Connect to Wi-Fi
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");

  WiFi.hostname("dht11");
  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(dht_t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(dht_h).c_str());
  });
  server.on("/worksornot", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(worksornot).c_str());
  });
  

  // Start server
  server.begin();
}

// Code that updates the webpage information, and IFTTT server
void loop()
{  
  HTTPClient http;

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) 
  {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;

    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) 
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else 
    {
      dht_h = newH;
      Serial.println(dht_h);

      // for humidity control
      if(humi_control == 1)
      {
        if (dht_h < humi_threshold[0])
        {
         serverPath = humiOn;
         worksornot = humi_threshold[0];
        }
        if (dht_h > humi_threshold[1])
        {
         serverPath = humiOff;
         worksornot = humi_threshold[1];
        }
      }
      // Your Domain name with URL path or IP address with path
      http.begin(wifiClient, serverPath.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }

    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // if temperature read failed, don't change t value
    if (isnan(newT)) 
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else 
    {
      dht_t = newT;
      Serial.println(dht_t);
    }
      

  }
}
