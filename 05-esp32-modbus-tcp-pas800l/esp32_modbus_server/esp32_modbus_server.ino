#include <WiFi.h>
#include <WebServer.h>
#include <ModbusTCP.h>

// --- CONFIGURACIÓN WI-FI ---
const char* ssid = "PAS800L_APCswitcheo1";
const char* password = "Pas800l*";

const int pinPotenciometro = 36;  // Pin del potenciómetro

// Usamos el nombre que tu librería sí reconoció
ModbusTCP modbusServer;

void setup() {
  Serial.begin(115200);

  // Conectar al Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n¡Wi-Fi Conectado!");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());

  // Iniciar el servidor Modbus TCP (por defecto abre el puerto 502)
  modbusServer.server();

  // En tu librería se usa 'addHreg' para registrar la dirección.
  // Registramos el Holding Register 100 de una vez.
  modbusServer.addHreg(100);

  Serial.println("Servidor Modbus TCP listo.");
}

void loop() {
  // Mantener el servidor Modbus escuchando al PAS800L
  modbusServer.task();

  // 1. Leer potenciómetro (0 a 4095)
  int lecturaRaw = analogRead(pinPotenciometro);

  // 2. Mapearlo de 25 a 80
  int temperaturaMapeada = map(lecturaRaw, 0, 4095, 25, 80);

  // 3. Guardar el valor en el Holding Register 100 usando 'Hreg'
  modbusServer.Hreg(100, temperaturaMapeada);

  // Monitor para ti en la PC
  static unsigned long ultimoPrint = 0;
  if (millis() - ultimoPrint > 1000) {
    ultimoPrint = millis();
    Serial.print("Enviando por Modbus (Reg 100): ");
    Serial.println(temperaturaMapeada);
  }
}