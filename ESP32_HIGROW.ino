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


void setup() {
  tic_WifiLed.attach(0.1, wifi_led_status);
  Serial.begin(9600);

  // Conexión WIFI
  WiFi.begin(ssid, password);
  Serial.print("Conectando a la red WiFi: ");
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
  //Iniciar Pines
  pinMode(ledWifi, OUTPUT);
  pinMode(led_R, OUTPUT);
  pinMode(led_A, OUTPUT);
  pinMode(led_V, OUTPUT);

  //Sensor DHT
  dht.begin();
}

void loop() {
  /*Sensor Capacitivo*/
  unsigned int hum_suelo = analogRead(SOILCAPIN);
  Serial.print("Sensor Capacitivo: ");
  Serial.println(hum_suelo);

  /*Humedad y temperatura DHT*/
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print("Humedad: ");
  Serial.println(h);
  Serial.print("temperatura: ");
  Serial.println(t);

  /*Num aleatorio*/
  numero_aleatorio = random(-500, 500);
  Serial.print("Numero Random: ");
  Serial.println(numero_aleatorio);
  delay(500);

  digitalWrite(led_R, HIGH);
  digitalWrite(led_A, HIGH);
  digitalWrite(led_V, HIGH);

  delay(500);

  digitalWrite(led_R, LOW);
  digitalWrite(led_A, LOW);
  digitalWrite(led_V, LOW);

}

//Wifi LED indicador
void wifi_led_status() {
  estado = digitalRead(ledWifi);
  digitalWrite(ledWifi, !estado);
}
