// Importa as Bibliotecas Necessarias
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Configurações do Wi-Fi
const char* ssid = "DIDE_Erleide";              
const char* password = "dide@pmp";         

//Configura MQTT
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
  dht.begin();
  
  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

// Em caso de erro, reconecta ao MQTT
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Leitura dos dados do sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Verificação de erro na leitura do DHT11
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Falha ao ler do sensor DHT");
    return;
  }

  // Publicando os dados via MQTT
  char tempStr[8];
  char humStr[8];
  dtostrf(temperature, 1, 2, tempStr);
  dtostrf(humidity, 1, 2, humStr);

  // Publicando e imprimindo no Serial Monitor
  if (client.publish(topicTemperature, tempStr)) {
    Serial.print("Temperatura publicada com sucesso: ");
    Serial.println(tempStr);
  } else {
    Serial.println("Falha ao publicar temperatura.");
  }

  if (client.publish(topicHumidity, humStr)) {
    Serial.print("Umidade publicada com sucesso: ");
    Serial.println(humStr);
  } else {
    Serial.println("Falha ao publicar umidade.");
  }

  // Intervalo entre as leituras
  delay(10000);
}