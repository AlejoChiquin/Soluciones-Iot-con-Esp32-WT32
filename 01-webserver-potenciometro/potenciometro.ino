#include <ETH.h>
#include <WebServer.h>

WebServer server(80);

const int pinLed = 4;   // IO04 (Tu LED/Calefactor)
const int pinPot = 36;  // IO36 (Potenciómetro)

float temperatura = 0.0;
int setPoint = 25;      // Setpoint por defecto

void setup() {
  Serial.begin(115200);
  
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, LOW);

  ETH.begin();

  Serial.print("Conectando a la red por Ethernet...");
  while (ETH.localIP()[0] == 0) {
    delay(500);
    Serial.print(".");
  }
  
  // Rutas del Servidor
  server.on("/", handleRoot);
  server.on("/SETPOINT", handleSetpoint);
  server.on("/LEER_DATOS", handleLeerDatos); // Ruta oculta que usa JavaScript para actualizar la info

  server.begin();
  
  Serial.println("\n¡Servidor Dinámico Listo!");
  Serial.print("Escribe esta IP: http://");
  Serial.println(ETH.localIP());
}

void loop() {
  // 1. Leer potenciómetro (El ESP32 tiene resolución de 12 bits: 0 a 4095)
  int lecturaRaw = analogRead(pinPot);
  
  // 2. Convertir lectura a rango de -10C a 95C usando floats para los decimales
  temperatura = -10.0 + ((float)lecturaRaw / 4095.0) * (95.0 - (-10.0));

  // 3. Lógica de Control (Termostato automático usando el Setpoint)
  if (temperatura < setPoint) {
    digitalWrite(pinLed, HIGH); // Enciende si hace frío
  } else {
    digitalWrite(pinLed, LOW);  // Apaga si ya pasó el Setpoint
  }

  server.handleClient();
  delay(50); // Ajuste de estabilidad
}

// --- CONTROL DE PÁGINAS WEB ---

// Envía la estructura principal de la página (HTML + JavaScript)
void handleRoot() {
  String html = "<!DOCTYPE HTML><html><head>";
  html += "<title>Termostato WT32-ETH01</title><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  
  // --- JAVASCRIPT PARA ACTUALIZAR EN TIEMPO REAL ---
  html += "<script>";
  html += "setInterval(function() {";
  html += "  fetch('/LEER_DATOS').then(response => response.json()).then(data => {";
  html += "    document.getElementById('temp').innerText = data.temp + ' °C';";
  html += "    var ledStatus = document.getElementById('led');";
  html += "    if(data.led == 1) { ledStatus.innerText = 'ENCENDIDO'; ledStatus.style.color = 'green'; }";
  html += "    else { ledStatus.innerText = 'APAGADO'; ledStatus.style.color = 'red'; }";
  html += "  });";
  html += "}, 1000);"; // Se ejecuta automáticamente cada 1000ms (1 segundo)
  html += "</script>";
  
  html += "</head>";
  html += "<body style='font-family: Arial; text-align: center; margin-top: 50px; background-color: #f4f4f4;'>";
  html += "<h2>Sistema de Control de Temperatura Inteligente</h2>";
  html += "<hr style='width:80%;'>";
  
  // Contenedores dinámicos (JavaScript cambiará lo que está adentro)
  html += "<h3>Temperatura Actual: <span id='temp' style='color:blue; font-size:24px;'>Cargando...</span></h3>";
  html += "<h3>Estado del Calefactor (LED Pin 4): <span id='led' style='font-size:24px;'>Cargando...</span></h3>";
  
  html += "<hr style='width:50%; margin: 30px auto;'>";
  
  // Sección estática del Setpoint
  html += "<h3>Setpoint Configurado: <span style='color:orange;'>" + String(setPoint) + " °C</span></h3>";
  html += "<form action='/SETPOINT' method='get'>";
  html += "Modificar Setpoint: <input type='number' name='valor' min='-10' max='95' required>";
  html += "<input type='submit' value='Guardar'>";
  html += "</form>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Envía exclusivamente los datos crudos (en formato JSON) que el JavaScript necesita leer cada segundo
void handleLeerDatos() {
  int estadoLed = digitalRead(pinLed);
  // Construye una cadena JSON simple: {"temp": 23.5, "led": 1}
  String json = "{\"temp\":" + String(temperatura, 1) + ",\"led\":" + String(estadoLed) + "}";
  server.send(200, "application/json", json);
}

// Procesa el cambio de Setpoint enviado desde el formulario
void handleSetpoint() {
  if (server.hasArg("valor")) {
    setPoint = server.arg("valor").toInt();
    Serial.print("Nuevo Setpoint fijado en: ");
    Serial.println(setPoint);
  }
  server.sendHeader("Location", "/");
  server.send(303);
}
