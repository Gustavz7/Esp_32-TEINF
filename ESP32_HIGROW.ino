#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <Ticker.h>

//LEDs
#define led_G 0
#define led_Y 2
#define led_R 15

//MQ-2 Analog Pin
#define gas_sensor 34

//Sensor capacitivo
#define SOILCAPIN 32

// WIFI config
#define ledWifi 23
Ticker tic_WifiLed;
char ssid[] = "POCO X3 Pro Guz";
char password[] = "123pormii";
byte estado = 0;
byte conectado = 0;
byte cont = 0;
byte max_intentos = 50;

// DHT Sensor Pin&Config
#define DHTIN 22
#define DHTTYPE DHT11
DHT dht(DHTIN, DHTTYPE);
String Humedad = "hum";
String Temperatura = "temp";

// numero aleatorio
float numero_aleatorio = 0;

//requisito pubsubclient
WiFiClient espClient;
PubSubClient client(espClient);

//conexion MQTT PubSubClient
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
const char* mqtt_server = "industrial.api.ubidots.com";
const int port = 1883;
#define mi_token ""

//Almacen de la respuesta en void callback();
String topico = "";
String content = "";

//topicos en los que publicar
const char btn_r[] = "/v1.6/devices/higrow-esp32/btn_r";
const char btn_a[] = "/v1.6/devices/higrow-esp32/btn_a";
const char btn_v[] = "/v1.6/devices/higrow-esp32/btn_v";
const char hum[] = "/v1.6/devices/higrow-esp32/hum";
const char temp[] = "/v1.6/devices/higrow-esp32/temp";
const char random_numbers[] = "/v1.6/devices/higrow-esp32/random_numbers";
const char gas[] = "/v1.6/devices/higrow-esp32/gas";
const char luz[] = "/v1.6/devices/higrow-esp32/luz";
const char joystick[] = "/v1.6/devices/higrow-esp32/joystick";
const char sensor_capacitivo[] = "/v1.6/devices/higrow-esp32/sensor_capacitivo";

//funcion para usar con ticker.h, estatus led wifi
void wifi_led_status() {
  estado = digitalRead(ledWifi);
  digitalWrite(ledWifi, !estado);
}

// Conexión y configuracion WIFI
void wifiSetup () {
  WiFi.begin(ssid, password);
  Serial.print("Conectando a: ");
  Serial.println(ssid);
  Serial.print("macAdress: ");
  Serial.println(WiFi.macAddress());
  //Cuenta hasta 50 si no se puede conectar lo cancela
  while (WiFi.status() != WL_CONNECTED and cont < max_intentos) {
    delay(500);
    cont++;
    Serial.print(".");
  };
  Serial.println("");
  if (cont < max_intentos) {  //Si se conectó al wifi especificado
    Serial.println(" -----------------Conexion Exitosa -----------------");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    tic_WifiLed.detach();
    tic_WifiLed.attach(1, wifi_led_status);
  }
  else { //Si no logró conectarse
    Serial.println(" ----------------- Conexion Fallida -----------------");
    tic_WifiLed.attach(4, wifi_led_status);
    cont = 0;
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  topico = topic;
  content = "";
  for (int i = 0; i < length; i++) {
    content.concat((char)payload[i]);
  };
  Serial.println();
  Serial.print("CALLBACK DETECTADO");
  /*Serial.print("Actualizacion en el topico: ");Serial.println(topico);
  Serial.print("Detalles:"); Serial.println(content);*/
}

void reconnect() {
  //Funcion de conexion WIFI
  wifiSetup();

  //Comienzo de la conexion mqtt
  while (!client.connected()) {
    Serial.println("Intentando conexión MQTT ...");

    // Create a random client ID
    String clientId = "gustavo-";
    clientId += String(random(0xffff), HEX);
    const char username[] = mi_token; //token de ubidots
    const char pass[] = "1234"; //pass random

    // Si la conexion a ubidots mqtt es exitosa
    if (client.connect(clientId.c_str(), username, pass)) {
      Serial.println("*** conexion MQTT Exitosa ***");
      
      client.subscribe(random_numbers);
      /*client.subscribe("/v1.6/devices/higrow-esp32/btn_r");
      client.subscribe("/v1.6/devices/higrow-esp32/btn_a");
      client.subscribe("/v1.6/devices/higrow-esp32/btn_v");*/

    } else {
      Serial.print("Conexion a MQTT Fallida, Err_Code= ");
      Serial.print(client.state());
      Serial.println(" Intentando nuevamente en 5s");
      delay(5000);
      wifiSetup();
    };
  }
}

void setup() {
  tic_WifiLed.attach(0.1, wifi_led_status);
  Serial.begin(9600);

  //PubSubClient Setup
  client.setServer(mqtt_server, port);
  client.setCallback(callback);

  //Iniciar Pines
  pinMode(ledWifi, OUTPUT);
  pinMode(led_R, OUTPUT);
  pinMode(led_Y, OUTPUT);
  pinMode(led_G, OUTPUT);

  //Sensor DHT
  dht.begin();

  //MQ-2
  //Innesario
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  /* Lectura desde sensores*/
  float h = dht.readHumidity();  //Humedad DHT
  float t = dht.readTemperature(); //Temperatura DHT
  int hum_suelo = analogRead(SOILCAPIN); //Sensor Capacitivo
  numero_aleatorio = random(0, 100); //Num aleatorio
  float gas_read = analogRead(gas_sensor); //Gas

  /*Conversion a valores CHAR*/
  //(valor, length entero, length decimal, guardar)
  char hum_value[8]; //Humedad DHT
  dtostrf(h, 3, 2, hum_value);
  char temp_value[8]; // Temperatura DHT
  dtostrf(t, 3, 2, temp_value);
  String humedad_srt = String(hum_suelo); //humedad (sensor capacitivo)
  float humedad_flo = humedad_srt.toFloat();
  char humedad_value[8];
  dtostrf(humedad_flo, 3, 2, humedad_value);
  char random_value[8]; //random numero
  dtostrf(numero_aleatorio, 3, 2, random_value);
  char gas_value[8]; //Gas
  dtostrf(gas_read, 3, 2, gas_value);

  digitalWrite(led_R, LOW);
  digitalWrite(led_Y, LOW);
  digitalWrite(led_G, LOW);
  
  // LEDs desde ubidots
  Serial.println("_____________________________________________________");
  Serial.println(topico);
  Serial.println(content);
  if (topico == "/v1.6/devices/higrow-esp32/btn_r") {
    digitalWrite(led_R, HIGH);
  } else {
    digitalWrite(led_R, LOW);
  };
  Serial.println("_____________________________________________________");
  /*Serial.print("Humedad DHT: "); Serial.println(hum_value);
  Serial.print("Temperatura DHT: "); Serial.println(temp_value);
  Serial.print("Capacitivo: "); Serial.println(humedad_value);
  Serial.print("Random Number: "); Serial.println(random_value);
  Serial.print("Gas: "); Serial.println(gas_value);
*/
  //client.publish(hum, hum_value);
  //client.publish(temp, temp_value);
  //client.publish(sensor_capacitivo, humedad_value);
  client.publish(random_numbers, random_value);

  delay(6000);
}
