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


// Replace with your network credentials
const char* ssid =      "insert wifi ssid here";           //insert wifi ssid here
const char* password =  "insert wifi password here";       //insert wifi password here

// IFTTT URLs here
String heatOn     = "insert releavent ifttt url here"; //Leave string empty if no URL available. URL looks like http://maker.ifttt.com/trigger/custom_ifttt_trgr_name/with/key/full_keyhere
String heatOff    = "insert releavent ifttt url here"; //Leave string empty if no URL available. URL looks like http://maker.ifttt.com/trigger/custom_ifttt_trgr_name/with/key/full_keyhere
String coldOn     = "insert releavent ifttt url here"; //Leave string empty if no URL available. URL looks like http://maker.ifttt.com/trigger/custom_ifttt_trgr_name/with/key/full_keyhere
String coldOff    = "insert releavent ifttt url here"; //Leave string empty if no URL available. URL looks like http://maker.ifttt.com/trigger/custom_ifttt_trgr_name/with/key/full_keyhere
String humiOn     = "insert releavent ifttt url here"; //Leave string empty if no URL available. URL looks like http://maker.ifttt.com/trigger/custom_ifttt_trgr_name/with/key/full_keyhere
String humiOff    = "insert releavent ifttt url here"; //Leave string empty if no URL available. URL looks like http://maker.ifttt.com/trigger/custom_ifttt_trgr_name/with/key/full_keyhere
String serverPath = "";

// ac, heat and humidity control flags
ac_control    = 0 //1-Enable, 0-Disable control
heat_control  = 0 //1-Enable, 0-Disable control
humi_control  = 0 //1-Enable, 0-Disable control

// Heat, AC , Humidity Thresholds here [on_threshold, off_threshold]
heat_temp      = [16, 18]   //in degC
cold_temp      = [18, 16]   //in degC
humi_threshold = [80, 90]   //in percentage

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
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var)
{
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  return String();
}

// Ardurino Initialization
void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println("Connecting to WiFi");
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, HIGH);    // turn the LED off by making the voltage LOW

  dht.begin();
  
  // Connect to Wi-Fi
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LEDpin, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(LEDpin, LOW);    // turn the LED off by making the voltage LOW
    Serial.println(".");
  }

  digitalWrite(LEDpin, HIGH);    // turn the LED off by making the voltage LOW

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

  // Start server
  server.begin();
}

// Code that updates the webpage information, and IFTTT server
void loop()
{  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) 
  {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;

    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    // float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    
    if (isnan(newT)) 
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else 
    {
      dht_t = newT;
      Serial.println(dht_t);
      
      // for heat control
      if(heat_control == 1)
      {
        if (dht_t < heat_temp[0])
        {
         serverPath = heatOn;
        }
        if (dht_t > heat_temp[1])
        {
         serverPath = heatOff;
        }
      }

      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
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

      // for AC control
      if(ac_control == 1)
      {
        if (dht_t > cold_temp[0])
        {
         serverPath = coldOn;
        }
        if (dht_t < cold_temp[1])
        {
         serverPath = coldOff;
        }
      }

      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
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
        }
        if (dht_h > humi_threshold[1])
        {
         serverPath = humiOff;
        }
      }
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
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


  }
}
