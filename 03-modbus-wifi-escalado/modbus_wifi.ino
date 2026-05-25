#include <WiFi.h>          // <-- Cambiamos <ETH.h> por <WiFi.h>
#include <WebServer.h>
#include <ModbusTCP.h> 

// --- CONFIGURACIÓN DEL WI-FI ---
const char* ssid = "Automatroni CA";     // !!! PON AQUÍ EL NOMBRE DE TU RED !!!
const char* password = "7654321%";  // !!! PON AQUÍ LA CLAVE DE TU RED !!!

// --- CONFIGURACIÓN DE RED MODBUS ---
IPAddress plcIP(192, 168, 88, 11); // IP de tu PLC

WebServer server(80);
ModbusTCP mb; 

// --- PINES ---
const int pinLed = 4; // IO04 para tu LED

// --- VARIABLES DEL SISTEMA ---
float temperaturaPLC = 0.0;
float setPointPLC = 25.0;      
unsigned long ultimoModbus = 0;

uint16_t valorCrudoTemp = 0;
uint16_t valorCrudoSP = 0;

// --- DECLARACIÓN DE FUNCIONES WEB ---
void handleRoot();
void handleSetpoint();
void handleLeerDatos();

void setup() {
  Serial.begin(115200);
  
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, LOW);

  // Inicializar Wi-Fi en modo Estación (conectarse a tu router)
  Serial.print("Conectando al Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Esperar a que se conecte a la red inalámbrica
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n¡Wi-Fi Conectado exitosamente!");
  Serial.print("Escribe esta IP en tu navegador: http://");
  Serial.println(WiFi.localIP()); // Esta es la nueva IP que te dará el Wi-Fi

  // Inicializar Modbus TCP en modo Cliente
  mb.client();

  // Configurar las rutas del Servidor Web (Se quedan exactamente igual)
  server.on("/", handleRoot);
  server.on("/SETPOINT", handleSetpoint);
  server.on("/LEER_DATOS", handleLeerDatos);

  server.begin();
  Serial.println("Servidor Web iniciado por Wi-Fi.");
}

void loop() {
  server.handleClient();
  
  // Revisar datos Modbus cada 1 segundo
  if (millis() - ultimoModbus > 1000) {
    ultimoModbus = millis();
    
    if (mb.isConnected(plcIP)) {
      // 1. Leer Temperatura Actual del PLC (9179)
      mb.readHreg(plcIP, 9179, &valorCrudoTemp, 1);
      temperaturaPLC = (float)valorCrudoTemp / 10.0; 

      // 2. Leer Setpoint Actual del PLC (16383)
      mb.readHreg(plcIP, 16383, &valorCrudoSP, 1);
      setPointPLC = (float)valorCrudoSP / 10.0; 

      // 3. Lógica del Termostato
      if (temperaturaPLC < setPointPLC) {
        digitalWrite(pinLed, HIGH);
      } else {
        digitalWrite(pinLed, LOW);
      }
    } else {
      mb.connect(plcIP);
      Serial.println("Buscando PLC en la red inalámbrica...");
    }
  }
  
  mb.task(); 
  delay(10);
}

// --- LAS FUNCIONES DEL SERVIDOR WEB SE QUEDAN EXACTAMENTE IGUAL ---

void handleRoot() {
  String html = "<!DOCTYPE HTML><html><head>";
  html += "<title>Puente Wi-Fi-Modbus</title><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  
  html += "<script>";
  html += "setInterval(function() {";
  html += "  fetch('/LEER_DATOS').then(response => response.json()).then(data => {";
  html += "    document.getElementById('temp').innerText = data.temp.toFixed(1) + ' °C';";
  html += "    document.getElementById('sp_actual').innerText = data.sp.toFixed(1) + ' °C';";
  html += "    var ledStatus = document.getElementById('led');";
  html += "    if(data.led == 1) { ledStatus.innerText = 'ENCENDIDO'; ledStatus.style.color = 'green'; }";
  html += "    else { ledStatus.innerText = 'APAGADO'; ledStatus.style.color = 'red'; }";
  html += "  });";
  html += "}, 1000);";
  html += "</script>";
  
  html += "</head>";
  html += "<body style='font-family: Arial; text-align: center; margin-top: 50px; background-color: #eef2f7;'>";
  html += "<h2>Control Industrial WT32-ETH01 por Wi-Fi</h2>";
  html += "<hr style='width:80%;'>";
  
  html += "<h3>Temp. Actual (Modbus 9179): <span id='temp' style='color:blue; font-size:24px;'>Leyendo PLC...</span></h3>";
  html += "<h3>Estado Calefactor (LED Pin 4): <span id='led' style='font-size:24px;'>Leyendo...</span></h3>";
  html += "<h3>Setpoint en PLC (Modbus 16383): <span id='sp_actual' style='color:orange; font-size:24px;'>Leyendo PLC...</span></h3>";
  
  html += "<hr style='width:50%; margin: 30px auto;'>";
  
  html += "<form action='/SETPOINT' method='get'>";
  html += "Cambiar Setpoint en PLC: <input type='number' name='valor' min='-10' max='95' step='0.1' required>";
  html += "<input type='submit' value='Enviar a PLC'>";
  html += "</form>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleLeerDatos() {
  int estadoLed = digitalRead(pinLed);
  String json = "{\"temp\":" + String(temperaturaPLC, 1) + ",\"sp\":" + String(setPointPLC, 1) + ",\"led\":" + String(estadoLed) + "}";
  server.send(200, "application/json", json);
}

void handleSetpoint() {
  if (server.hasArg("valor")) {
    float valorWeb = server.arg("valor").toFloat();
    int valorParaPLC = (int)(valorWeb * 10.0);
    
    if (mb.isConnected(plcIP)) {
      mb.writeHreg(plcIP, 16383, valorParaPLC);
      Serial.print("Modbus Wi-Fi: Enviado nuevo Setpoint -> ");
      Serial.println(valorParaPLC);
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}
