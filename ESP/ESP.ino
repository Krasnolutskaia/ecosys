#include <DHT.h>
#include <AccelStepper.h>  
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>  
#include <analogWrite.h>

#define SOIL_PIN 32
#define PUMP_PIN 16
#define LIGHT_PIN 25
#define DHT_PIN 27
#define DHT_TYPE DHT21

#define STEP 512
#define MOTORPIN1 23
#define MOTORPIN2 5
#define MOTORPIN3 13 
#define MOTORPIN4 12
#define HALFSTEP 4

#define POSITION 0
#define SENSOR_AMOUNT 4
#define SCAN_NUMBER 5
#define UNGLE_STEPS 90

AccelStepper stepper(HALFSTEP, MOTORPIN1, MOTORPIN3, MOTORPIN2, MOTORPIN4);

DHT dht(DHT_PIN, DHT_TYPE);

const char *ssid = "1";       // имя вашей wifi точки доступа
const char *password = "0123456789"; // пароль wifi
const char* serverName_from = "http://192.168.197.229:80/from_greenhouse";
const char* serverName_to = "http://192.168.197.229:80/get_data";
JSONVar doc;

float hum_air, temp;
int hum_soil, hum_soil_border, volume, angle, water;
int sunset = 0;
int sunrise = 0;
int curr_h = 0;
int day_durr = 0;
int rotate = 0;
int auto_rotate = 0;
int const_rotate = 0;

int delay_time = 1000;
unsigned long last_time;

int auto_water = 0;
unsigned long water_lasttime; 
int water_period = 30000;
int auto_light = 0;
int light = 0;

int index_min_sensor = 0;
int min_sensor = 5000;
int index_max_sensor = 0;
int max_sensor = 0;

int rotation_coordinate; 
bool f = false;
unsigned long cycle_lasttime; 
int cycle_period = 5000; 

int Data0[5];
int Data1[5];
int Data2[5];
int Data3[5];
int suma(int data[5]) {
  int s = 0;
  for (int i = 0; i < 5; i++) {
    s += data[i];
  }
  return s / 5;
}

int light_vals[4];
int light_pins[4] = {35, 34, 36, 39};

int light_border = 2700;

void setup() {
  Serial.begin(115200);
  dht.begin();

  stepper.setMaxSpeed(1000);

  last_time = millis();
  water_lasttime = millis();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) // подключение к точке
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void lights(){
  if (auto_light){
    int light_av = 0;
    for (int i = 0; i < 4; i ++){
      light_av += light_vals[i];
    }
    light_av = light_av / 4;
    if (light_av > light_border || day_durr * sunset * sunrise == 0 || sunset - sunrise >= day_durr || curr_h >= sunrise - (day_durr - (sunset - sunrise)) / 2 || curr_h <= sunset + (day_durr - (sunset - sunrise)) / 2){
      analogWrite(LIGHT_PIN, 255);
      light = 1;
    }
    else{
      analogWrite(LIGHT_PIN, 0);
      light = 0;
    }
  }
  else{
    if (light){
      analogWrite(LIGHT_PIN, 255);
    }
    else{
      analogWrite(LIGHT_PIN, 0);
    }
  }
}

void get_light(){
  for(int i = 0; i < 4; i++)
  {
    light_vals[i] = analogRead(light_pins[i]);
  }
}

int get_hum_soil(){
  return analogRead(SOIL_PIN);
}

void rotate_angle(int ang){
  int n = ang / 360;
  if (rotate == 1){
    ang = n * 2048 + map(ang - n * 360, -360, 360, -2048, 2048);
    Serial.println(ang);
    stepper.setSpeed(500);
    stepper.move(ang);
    rotate = 0;
  }
}

void rotating_function(){
  if (index_max_sensor < index_min_sensor){
    if (4 - index_min_sensor + index_max_sensor > index_min_sensor - index_max_sensor){rotation_coordinate = STEP;}
    else if (4 - index_min_sensor + index_max_sensor < index_min_sensor - index_max_sensor){rotation_coordinate = STEP * -1;}
    else {rotation_coordinate = STEP * 2;}
  }
  else {
    if (4 - index_max_sensor + index_min_sensor > index_max_sensor - index_min_sensor){rotation_coordinate = STEP * -1;}
    else if (4 - index_max_sensor + index_min_sensor < index_max_sensor - index_min_sensor){rotation_coordinate = STEP;}
    else {rotation_coordinate = STEP * 2;}
  }
  Serial.println(rotation_coordinate);
  max_sensor = 0;
  index_max_sensor = 0;

  min_sensor = 2000;
  index_min_sensor = 0;
}

void sensor_reading(){
    for (int i = 0; i < 5; i ++){
    Data0[i] = (int(analogRead(light_pins[0])));
    Data1[i] = (int(analogRead(light_pins[1])));
    Data2[i] = (int(analogRead(light_pins[2])));
    Data3[i] = (int(analogRead(light_pins[3])));
  }

  int ending_mass[4] = {suma(Data0), suma(Data1), suma(Data2), suma(Data3)};
  for (int i = 0; i < SENSOR_AMOUNT; i ++){
    if (ending_mass[i] > max_sensor){
      max_sensor = ending_mass[i];
      index_max_sensor = i;
    }
   }
    for (int i = 0; i < SENSOR_AMOUNT; i ++){
      if (ending_mass[i] < min_sensor){
        min_sensor = ending_mass[i];
        index_min_sensor = i;
    }
  }
  Serial.println(String(ending_mass[0]) + " " + String(ending_mass[1]) + " " + String(ending_mass[2]) + " " + String(ending_mass[3]) + " " + String(index_max_sensor) + " " + String(index_min_sensor));
}

void water_(){
  if (auto_water == 1){
    if (hum_soil > hum_soil_border and millis() - water_lasttime >= water_period){
      digitalWrite(PUMP_PIN, HIGH);
      delay(3000);
      digitalWrite(PUMP_PIN, LOW);
      water_lasttime = millis();
    }
  }
  else{
    if (water == 1){
      digitalWrite(PUMP_PIN, HIGH);
      delay(5000);
      digitalWrite(PUMP_PIN, LOW);
    }
  }

}


void send_data(){
  if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      http.begin(client, serverName_from);

      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String httpRequestData = "hum_air=" + String(dht.readHumidity())
                                + "&temp=" + String(dht.readTemperature())
                                + "&hum_soil=" + String(hum_soil)
                                + "&rotate=" + String(rotate)
                                + "&light=" + String(light);
      Serial.println(dht.readTemperature());
      
      for(int i = 0; i < 4; i++)
      {
        httpRequestData += "&light_val" + String(i) + "=" + String(light_vals[i]);
      }

      int httpResponseCode = http.POST(httpRequestData);

      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}


void get_data(){
    if(WiFi.status()== WL_CONNECTED){
              
      String server_data = httpGETRequest(serverName_to);

      doc = JSON.parse(server_data);

      if (JSON.typeof(doc) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      Serial.print("JSON object = ");
      Serial.println(doc);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    return;
}


String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, serverName);

  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  return payload;
}


void loop() 
{
  if (millis() - last_time > delay_time)
  {
    get_data();

    hum_soil = get_hum_soil();
    get_light();

    rotate = doc["rotate"];
    angle = doc["angle"];
    const_rotate = doc["const_rotate"];
    auto_rotate = doc["auto_rotate"];
    rotate_angle(angle);
      
    auto_light = doc["auto_light"];
    light = doc["light"];
    light_border = doc["light_border"];
    curr_h = doc["curr_h"];
    sunset = doc["sunset_h"];
    sunrise = doc["sunrise_h"];
    day_durr = doc["day_durr";]
    lights();

    water = doc["water"];
    volume = doc["volume"];
    auto_water = doc["auto_water"];
    hum_soil_border = doc["hum_soil_border"];
    water_();

    send_data();
    last_time = millis();
  }

  if (const_rotate == 1){
    stepper.setSpeed(400);
    stepper.runSpeed();
  }

  if (f){
    stepper.move(rotation_coordinate);
    stepper.setSpeed(400);
    f = false;
  }
  
  if (millis() - cycle_lasttime >= cycle_period and auto_rotate == 1){
    sensor_reading();
    rotating_function();
    f = true;
    cycle_lasttime = millis();
  }

  if (stepper.distanceToGo() != 0){
    stepper.runSpeedToPosition();
  }
  else{
    stepper.disableOutputs();
  }
}
