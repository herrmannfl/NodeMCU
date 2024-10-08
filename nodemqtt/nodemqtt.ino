/*Código baseado em: https://gist.github.com/lnpbr/84f9c0505032caf8c725ad9e2d9aa680 */
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

//Informações do WiFi
#define WIFISSID "XXXXXXXX"
#define WIDIPASS "XXXXXXXX"

//Informações do Servidor MQTT
const char* mqttserver = "XXXXX";
int mqttserverport = 0000;
const char* mqttuser = "XXXXX";
const char* mqttpass = "XXXXX";

//Variáveis
WiFiClient wifiClient;
PubSubClient ConexaoMQTT(wifiClient);
int UltimoValor = 0;
int TimeCounter = 0;

//Pinos
const int PINbotao = 5;

void reconnect() {
  ConexaoMQTT.setServer(mqttserver, mqttserverport);
  while (!ConexaoMQTT.connected()) {
    Serial.println("Conectando ao Broker MQTT");
    ConexaoMQTT.connect("esp8266",mqttuser,mqttpass);
    delay(3000);
  }
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
  
  pinMode(PINbotao, INPUT); 

  //Configura Sensores através do MQTT Discovery do Home Assistant
  String novosensor = "{\"expire_after\": \"600\", \"icon\": \"mdi:gesture-tap-button\", \"name\": \"Interruptor 1 ESP8266\", \"state_topic\": \"esp8266/interruptor1/state\"}";
  ConexaoMQTT.publish("homeassistant/binary_sensor/esp8266/interruptor1/config",novosensor.c_str(),false);

  //Primeira leitura
  int LeituraSwitchD1Pin = digitalRead(PINbotao);
  Serial.print("Primeira Leitura: ");
  Serial.println(LeituraSwitchD1Pin);
  if (LeituraSwitchD1Pin == 0) {
    UltimoValor = LeituraSwitchD1Pin;
    ConexaoMQTT.publish("esp8266/interruptor1/state","OFF");
  } else if (LeituraSwitchD1Pin == 1) {
    UltimoValor = LeituraSwitchD1Pin;
    ConexaoMQTT.publish("esp8266/interruptor1/state","ON");
  }
}

void loop() {
  if (!ConexaoMQTT.connected()) {
    reconnect();
  }
  
  int LeituraSwitchD1Pin = digitalRead(PINbotao);
  Serial.println(LeituraSwitchD1Pin);
  if (TimeCounter < 600) {
    TimeCounter++;
  } 
  // não foi utilizado um elseif para não precisar implementar outro IF para testar a variação do sinal no pino
  if (TimeCounter >= 600 || LeituraSwitchD1Pin != UltimoValor) {
    if (LeituraSwitchD1Pin == 0) {
      UltimoValor = LeituraSwitchD1Pin;
      ConexaoMQTT.publish("esp8266/interruptor1/state","OFF");
    } else if (LeituraSwitchD1Pin == 1) {
      UltimoValor = LeituraSwitchD1Pin;
      ConexaoMQTT.publish("esp8266/interruptor1/state","ON");
    }
    TimeCounter = 0;
  }
  delay(500);
}
