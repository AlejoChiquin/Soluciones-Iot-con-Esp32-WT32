#include <ETH.h>
#include <WebServer.h>
#include <ModbusTCP.h> 

// --- CONFIGURACIÓN DE RED ---
IPAddress plcIP(192, 168, 88, 11); // !!! PON AQUÍ LA IP REAL DE TU PLC !!!

WebServer server(80);
ModbusTCP mb; // Objeto para el control Modbus TCP

// --- PINES ---
const int pinLed = 4; // IO04 para tu LED / Calefactor

// --- VARIABLES DEL SISTEMA ---
float temperaturaPLC = 0.0;
float setPointPLC = 25.0;      // Cambiado a float para soportar decimales en la web
unsigned long ultimoModbus = 0;

// Variables temporales para almacenar las lecturas crudas en formato entero del PLC
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

  // Inicializar Ethernet nativo de la WT32-ETH01
  ETH.begin();

  Serial.print("Conectando a la red por Ethernet...");
  while (ETH.localIP()[0] == 0) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n¡Conectado!");
  Serial.print("IP del ESP32: ");
  Serial.println(ETH.localIP());

  // Inicializar Modbus TCP en modo Cliente (Master)
  mb.client();

  // Configurar las rutas del Servidor Web
  server.on("/", handleRoot);
  server.on("/SETPOINT", handleSetpoint);
  server.on("/LEER_DATOS", handleLeerDatos);

  server.begin();
  Serial.println("Servidor Web iniciado.");
}

void loop() {
  server.handleClient();
  
  // Leer datos del PLC cada 1 segundo para mantener la red fluida
  if (millis() - ultimoModbus > 1000) {
    ultimoModbus = millis();
    
    if (mb.isConnected(plcIP)) {
      
      // 1. Leer Temperatura Actual del PLC (Registro 9179)
      mb.readHreg(plcIP, 9179, &valorCrudoTemp, 1);
      // Convierte el entero del PLC a decimal (ej: 235 -> 23.5 °C)
      temperaturaPLC = (float)valorCrudoTemp / 10.0; 

      // 2. Leer Setpoint Actual del PLC (Registro 16383)
      mb.readHreg(plcIP, 16383, &valorCrudoSP, 1);
      // Convierte el entero del PLC a decimal (ej: 180 -> 18.0 °C)
      setPointPLC = (float)valorCrudoSP / 10.0; 

      // 3. Lógica de Control local del LED (Termostato)
      if (temperaturaPLC < setPointPLC) {
        digitalWrite(pinLed, HIGH); // Enciende si la temperatura es menor al setpoint
      } else {
        digitalWrite(pinLed, LOW);  // Apaga si ya alcanzó o superó el setpoint
      }
      
    } else {
      // Si se desconectó del PLC, intenta enlazar de nuevo
      mb.connect(plcIP);
      Serial.println("Intentando conectar al PLC vía Modbus TCP...");
    }
  }
  
  mb.task(); // Mantiene vivos los procesos internos de Modbus TCP
  delay(10);
}

// --- DESARROLLO DE FUNCIONES DEL SERVIDOR WEB ---

void handleRoot() {
  String html = "<!DOCTYPE HTML><html><head>";
  html += "<title>Puente Web-Modbus</title><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  
  // AJAX: Actualiza las etiquetas cada segundo desde la web sin parpadeos
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
  html += "<body style='font-family: Arial; text-align: center; margin-top: 50px; background-color: #f4f4f4;'>";
  html += "<h2>Control Industrial WT32-ETH01 a PLC</h2>";
  html += "<hr style='width:80%;'>";
  
  html += "<h3>Temp. Actual (Modbus 9179): <span id='temp' style='color:blue; font-size:24px;'>Leyendo PLC...</span></h3>";
  html += "<h3>Estado Calefactor (LED Pin 4): <span id='led' style='font-size:24px;'>Leyendo...</span></h3>";
  html += "<h3>Setpoint en PLC (Modbus 16383): <span id='sp_actual' style='color:orange; font-size:24px;'>Leyendo PLC...</span></h3>";
  
  html += "<hr style='width:50%; margin: 30px auto;'>";
  
  // El formulario ahora acepta pasos de 0.1 gracias a step='0.1'
  html += "<form action='/SETPOINT' method='get'>";
  html += "Cambiar Setpoint en PLC: <input type='number' name='valor' min='-10' max='95' step='0.1' required>";
  html += "<input type='submit' value='Enviar a PLC'>";
  html += "</form>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Emite la lectura en formato JSON para que JavaScript la procese de fondo
void handleLeerDatos() {
  int estadoLed = digitalRead(pinLed);
  String json = "{\"temp\":" + String(temperaturaPLC, 1) + ",\"sp\":" + String(setPointPLC, 1) + ",\"led\":" + String(estadoLed) + "}";
  server.send(200, "application/json", json);
}

// Recibe el valor flotante de la web, lo escala para el PLC y lo inyecta por Modbus
void handleSetpoint() {
  if (server.hasArg("valor")) {
    float valorWeb = server.arg("valor").toFloat(); // Lee el decimal de la web (ej: 28.4)
    int valorParaPLC = (int)(valorWeb * 10.0);      // Lo escala a entero para el PLC (ej: 284)
    
    if (mb.isConnected(plcIP)) {
      // Escribir el nuevo valor entero escalado en el PLC (Función 06 Modbus)
      mb.writeHreg(plcIP, 16383, valorParaPLC);
      Serial.print("Modbus: Enviado nuevo Setpoint escalado al PLC -> ");
      Serial.println(valorParaPLC);
    } else {
      Serial.println("Error: PLC desconectado. No se pudo enviar el Setpoint.");
    }
  }
  // Te regresa automáticamente a la pantalla principal
  server.sendHeader("Location", "/");
  server.send(303);
}
