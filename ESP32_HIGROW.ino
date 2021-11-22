#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <Ticker.h>

#define led_R 19
#define led_A 18
#define led_V 5

#define SOILCAPIN 32

// WIFI
#define ledWifi 21
Ticker tic_WifiLed;
char ssid[] = "POCO X3 Pro Guz";
char password[] = "123pormii";
byte estado = 0;
byte conectado = 0;
byte cont = 0;
byte max_intentos = 50;

// DHT
#define DHTIN 22
#define DHTTYPE DHT11
DHT dht(DHTIN, DHTTYPE);
String Humedad = "hum";
String Temperatura = "temp";

long numero_aleatorio = 0;

WiFiClient espClient;
PubSubClient client(espClient);

//MQTT PubSubClient
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
const char* mqtt_server = "industrial.api.ubidots.com";
const int port = 1883;

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

void wifi_led_status() {
  estado = digitalRead(ledWifi);
  digitalWrite(ledWifi, !estado);
}
void wifiSetup () {
  // Conexión WIFI
  WiFi.begin(ssid, password);
  Serial.print("Conectando a: ");
  Serial.println(ssid);
  Serial.print("macAdress: ");
  Serial.println(WiFi.macAddress());
  while (WiFi.status() != WL_CONNECTED and cont < max_intentos) { //Cuenta hasta 50 si no se puede conectar lo cancela
    delay(500);
    cont++;
    Serial.print(".");
  };
  Serial.println("");
  if (cont < max_intentos) {  //Si se conectó
    Serial.println("********************************************");
    Serial.println("Conexion Exitosa");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    tic_WifiLed.detach();
    tic_WifiLed.attach(1, wifi_led_status);
    Serial.println("********************************************");
  }
  else { //Si no se conectó
    Serial.println("------------------------------------");
    Serial.println("No se puedo conectar a la Red Wifi");
    tic_WifiLed.attach(4, wifi_led_status);

    Serial.println("------------------------------------");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Intentando conexión MQTT ...");

    // Create a random client ID
    String clientId = "gustavo-";
    clientId += String(random(0xffff), HEX);
    const char username[] = "BBFF-ULPIAkNrl6wJK8brY3CgHcD0sw1nD4"; //token de ubidots
    const char pass[] = "1234"; //pass random

    // Si la conexion es exitosa
    if (client.connect(clientId.c_str(), username, pass)) {
      Serial.println("connected");
      //client.subscribe(hum);
      //client.subscribe(temp);
      client.subscribe(sensor_capacitivo);
      client.subscribe(random_numbers);
    } else {
      wifiSetup();
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
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
  pinMode(led_A, OUTPUT);
  pinMode(led_V, OUTPUT);

  //Sensor DHT
  dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //Sensor Capacitivo
  int hum_suelo = analogRead(SOILCAPIN);
  /*Serial.print("Sensor Capacitivo: ");
  Serial.println(hum_suelo);*/

  //Humedad y temperatura DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  /*
  Serial.print("Humedad: ");
  Serial.println(h);
  Serial.print("temperatura: ");
  Serial.println(t);
*/
  //Num aleatorio
  int numero_aleatorio = random(0, 100);

  //(valor, length entero, length decimal, guardar)
  char hum_value[8];
  dtostrf(h, 3, 2, hum_value);
  char temp_value[8];
  dtostrf(t, 3, 2, temp_value);

  /*char humedad_value[8]= {hum_suelo};
  char random_value[8]= {numero_aleatorio};*/

  String humedad_srt = String(hum_suelo);
  float humedad_flo = humedad_srt.toFloat();
  char humedad_value[8];
  dtostrf(humedad_flo, 3, 2, humedad_value);

  String random_srt = String(numero_aleatorio);
  float random_flo = random_srt.toFloat();
  char random_value[8];
  dtostrf(random_flo, 3, 2, random_value);
  
  /*itoa(hum_suelo, humedad_value, 10);
  char humedad_value = hum_suelo;
  char random_value = numero_aleatorio;*/

  //client.publish(hum, hum_value);
  //client.publish(temp, temp_value);
  Serial.print("analogRead: ");
  Serial.println(hum_suelo);
  Serial.print("Read converted: ");
  Serial.println(humedad_value);
  client.publish(sensor_capacitivo, humedad_value);
  //client.publish(random_numbers, random_value);

  delay(3500);
}
/*
  const char btn_r[]
  const char btn_a[]
  const char btn_v[]
  const char hum[]
  const char temp[]
  const char random_numbers[]
  const char gas[]
  const char luz[]
  const char joystick[]
  const char sensor_capacitivo[]
*/
