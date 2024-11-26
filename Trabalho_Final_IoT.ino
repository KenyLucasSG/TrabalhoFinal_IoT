#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Configurações do Wi-Fi
const char* ssid = "Error_404";              
const char* password = "KLSG9124";         

// Configurações do MQTT
const char* mqtt_server = "test.mosquitto.org"; 
const char* topicTemperature = "mestrado/iot/aluno/keny/temperatura"; 
const char* topicHumidity = "mestrado/iot/aluno/keny/umidade";       

// Configurações do sensor DHT11
#define DHTPIN D1  // Pino que conecta o DHT11
#define DHTTYPE DHT11 // Tipo do sensor
DHT dht(DHTPIN, DHTTYPE);

// Inicializa Wi-Fi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando...");

  // Inicializa o sensor DHT
  dht.begin();
  delay(1000); // Aguarda estabilização do sensor

  // Configura o cliente MQTT
  client.setServer(mqtt_server, 1883);
}

void connectToWiFi() {
  // Desativa o Modem-Sleep para conectar ao Wi-Fi
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  Serial.println("Modem-Sleep desativado.");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Conectando ao Wi-Fi: ");
  unsigned long startAttemptTime = millis();

  // Timeout de 10 segundos para conexão Wi-Fi
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi conectado");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha ao conectar ao Wi-Fi. Reiniciando...");
    ESP.restart();
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Conectado ao MQTT.");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente...");
      delay(2000);
    }
  }
}

void loop() {
  // Conecta ao Wi-Fi se não estiver conectado
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  // Conecta ao MQTT se não estiver conectado
  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();

  // Lê os dados do sensor DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Falha ao ler o sensor DHT.");
    delay(5000); // Tenta novamente após 5 segundos
    return;
  }

  char tempStr[8];
  char humStr[8];
  dtostrf(temperature, 1, 2, tempStr);
  dtostrf(humidity, 1, 2, humStr);

  // Publica os dados no MQTT
  if (client.publish(topicTemperature, tempStr)) {
    Serial.print("Temperatura publicada: ");
    Serial.println(tempStr);
  } else {
    Serial.println("Falha ao publicar temperatura.");
  }

  if (client.publish(topicHumidity, humStr)) {
    Serial.print("Umidade publicada: ");
    Serial.println(humStr);
  } else {
    Serial.println("Falha ao publicar umidade.");
  }

  // Habilita Modem-Sleep após a publicação
  WiFi.setSleepMode(WIFI_MODEM_SLEEP);
  Serial.println("Modem-Sleep habilitado para economia de energia.");

  // Aguarda antes de repetir o ciclo
  delay(20000);
}
