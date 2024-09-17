// Programa: Controle de LED
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
 
// Pela numeração do NodeMCU:
#define SAIDAD5   D5
// Pela numeração do GPIO:
//#define SAIDAD5   14

const char* ssid = "XXXXXXXXXXXXXX";
const char* password = "XXXXXXX";
ESP8266WebServer server(80);
 
void setup() {
  pinMode(SAIDAD5, OUTPUT); //DEFINE O PINO COMO SAÍDA
  
  Serial.begin(115200);
  SPIFFS.begin();
 
  Serial.println("");
  Serial.println("Conectando a Rede: "); //Imprime na serial a mensagem
  Serial.println(ssid); //Imprime na serial o nome da Rede Wi-Fi

  WiFi.begin(ssid, password); //Inicialização da comunicação Wi-Fi
  // Esperando conectar no Wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(ssid);
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
 
  // aguarda por requisições na URL raiz "/"
  server.on("/", trataLed);
  
  server.begin();
}

 // https://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
void trataLed() {
  // se for maior que 1 então não faz nada...
  String message = "";
  for (int i = 0; i < server.args(); i++) {
    message += "Arg nº" + (String)i + " –> ";
    message += server.argName(i) + ": ";
    message += server.arg(i) + "\n";
  } 
  Serial.print(message); 
  
  if (server.args() == 1){
    if (server.argName(0) == "led"){
      if (server.arg(0) == "ON"){
        digitalWrite(SAIDAD5,true);
        server.send(200, "text/html", "");
      }else if (server.arg(0) == "OFF"){
        digitalWrite(SAIDAD5,false);
        server.send(200, "text/html", "");
      }else{
        server.send(400, "text/html", "");
      }
    }else{
      server.send(400, "text/html", "");
    }
  }else{
    server.send(404, "text/html", "");
  }
}

void loop() {
  server.handleClient();
}

