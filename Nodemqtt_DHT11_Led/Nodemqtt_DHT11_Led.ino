#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <DHT.h> 

#define DHTTYPE DHT11 
#define DHTPIN   D3
#define LED D5

//Informações do WiFi
#define WIFISSID "XXXXXX"
#define WIDIPASS "XXXXXX"


//Informações do Servidor MQTT
const char* mqttserver = "XXX.XXX.XXX.XXX";
int mqttserverport = XXXX;
const char* mqttuser = "XXXXXX";
const char* mqttpass = "XXXXXX";

//Variáveis
WiFiClient wifiClient;
PubSubClient ConexaoMQTT(wifiClient);
int UltimoValor = 0;
int TimeCounter = 0;

const char* MQTT_LED_STATE_TOPIC = "nodemcu/led/status";

//Inicializando o objeto dht do tipo DHT passando como parâmetro o pino (DHTPIN) 
//e o tipo do sensor (DHTTYPE)
DHT dht(DHTPIN, DHTTYPE); 

float temperatura; //variável para armazenar a temperatura
float umidade; //Variável para armazenar a umidade

void reconnect() {
  ConexaoMQTT.setServer(mqttserver, mqttserverport);
  ConexaoMQTT.setCallback(mqttCallback);
  while (!ConexaoMQTT.connected()) {
    Serial.println("Conectando ao Broker MQTT");
    ConexaoMQTT.connect("esp8266",mqttuser,mqttpass);
    delay(3000);
  }
  ConexaoMQTT.subscribe(MQTT_LED_STATE_TOPIC);
  Serial.println("MQTT conectado");
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.println("Conectando a Rede: "); //Imprime na serial a mensagem
  Serial.println(WIFISSID); //Imprime na serial o nome da Rede Wi-Fi
  WiFi.begin(WIFISSID, WIDIPASS);
  delay(5000);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado! IP address: ");
  Serial.println(WiFi.localIP());

  if (!ConexaoMQTT.connected()) {
    reconnect();
  }
  dht.begin(); //Inicializa o sensor DHT11

  pinMode(LED, OUTPUT); 

  //Configura Sensores através do MQTT Discovery do Home Assistant
  String sensorUmidade = "{\"expire_after\": \"600\", \"unique_id\":\"umidade_nodemcu_1\", \"device_class\":\"humidity\",\"unit_of_measurement\":\"%\", \"name\": \"Umidade_NodeMCU_1\", \"state_topic\": \"nodemcu/umidade/state\", \"device\": {\"identifiers\": [\"NodeMCU_0001\"], \"name\": \"NodeMCU MQTT\"}}";
  ConexaoMQTT.publish("homeassistant/sensor/nodemcu/umidade/config", sensorUmidade.c_str(), true);
  delay(500);
  String sensorTemperatura = "{\"expire_after\": \"600\", \"unique_id\":\"temperatura_nodemcu_1\", \"device_class\":\"temperature\",\"unit_of_measurement\":\"°C\", \"name\": \"Temperatura_NodeMCU_1\", \"state_topic\": \"nodemcu/temperatura/state\", \"device\": {\"identifiers\": [\"NodeMCU_0001\"], \"name\": \"NodeMCU MQTT\"}}";
  ConexaoMQTT.publish("homeassistant/sensor/nodemcu/temperatura/config", sensorTemperatura.c_str(), true);
  delay(500);

  // Cria um subscriber para receber comandos do Home Assistant
  ConexaoMQTT.subscribe(MQTT_LED_STATE_TOPIC);

}

void loop() {
  if (!ConexaoMQTT.connected()) {
    reconnect();
  }

  temperatura = dht.readTemperature();  //Realiza a leitura da temperatura
  umidade = dht.readHumidity(); //Realiza a leitura da umidade
  if (isnan(temperatura) || isnan(umidade)){
    Serial.println("Erro lendo umidade/temperatura");
  }else{
    Serial.print("Temperatura: ");
    Serial.print(temperatura); //Imprime no monitor serial o valor da temperatura lida
    Serial.println(" ºC");
    Serial.print("Umidade: ");
    Serial.print(umidade); //Imprime no monitor serial o valor da umidade lida
    Serial.println(" %");
    String umidade_str((float)umidade);
    ConexaoMQTT.publish("nodemcu/umidade/state", umidade_str.c_str());
    String temperatura_str((float)temperatura);
    ConexaoMQTT.publish("nodemcu/temperatura/state", temperatura_str.c_str());
  }
  ConexaoMQTT.loop();

  delay(500);
}


// Callback para ligar e desligar o LED
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived on Topic:");
  Serial.println(topic);
  // concat the payload into a string

  payload[length] = '\0';
  String message((char *)payload);

  Serial.print("Message: ");
  Serial.println(message);

  if (String(MQTT_LED_STATE_TOPIC).equals(topic)){
    if (message == "ON"){
      digitalWrite(LED, HIGH);
    }
    if (message == "OFF"){
      digitalWrite(LED, LOW);
    }
  }
}