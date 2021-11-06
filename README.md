# smart_room
Smart room control using ESP8266, DHT11, and smart switches
project derived from https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/

1. Install [Ardurino IDE](https://www.arduino.cc/en/software)
2. Follow this [guide](https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/) to install the board drivers and runa hello world LED code, to see if everything is working as expected.
3. Connect DHT11 sensor as follows
	1. 	Vdd to 3V pin left of USB
	2. 	GND to G pin left of USB
	3. 	Signal pin to D1 left of USB
5. Open the [dht11_server.ino](https://github.com/wcklyaditya/smart_room/blob/main/dht11_server.ino) file in Ardurino IDE. Update the following Parameters
	1. Replace with your network credentials
		1. const char* ssid =      "insert wifi ssid here";
		2. const char* password =  "insert wifi password here";
	2. // IFTTT URLs here
		1. String heatOn     = "insert releavent ifttt url here"; //Only "" if not using
		2. String heatOff    = "insert releavent ifttt url here"; //Only "" if not using
		3. String coldOn     = "insert releavent ifttt url here"; //Only "" if not using
		4. String coldOff    = "insert releavent ifttt url here"; //Only "" if not using
		5. String humiOn     = "insert releavent ifttt url here"; //Only "" if not using
		6. String humiOff    = "insert releavent ifttt url here"; //Only "" if not using
		7. String serverPath = "";
	3. ac, heat and humidity control flags
		1. ac_control    = 0 //1-Enable, 0-Disable control
		2. heat_control  = 0 //1-Enable, 0-Disable control
		3. humi_control  = 0 //1-Enable, 0-Disable control
	4. Heat, AC , Humidity Thresholds here [on_threshold, off_threshold]
		1. heat_temp      = [16, 18]   //in degC
		2. cold_temp      = [18, 16]   //in degC
		3. humi_threshold = [80, 90]   //in percentage
