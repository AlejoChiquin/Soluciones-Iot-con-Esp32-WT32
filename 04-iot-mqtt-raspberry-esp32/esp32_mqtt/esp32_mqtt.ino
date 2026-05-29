#include <WiFi.h>
#include <PubSubClient.h>

// --- CONFIGURACIÓN WI-FI ---
const char* ssid = "Autormatroni CA";
const char* password = "7654321%";

// --- CONFIGURACIÓN MQTT BROKER (Raspberry Pi) ---
const char* mqtt_server = "192.168.88.13";
const int mqtt_port = 1883;

// --- TOPICOS MQTT ---
const char* topic_temp = "esp32/temperatura";
const char* topic_led = "esp32/led";
const char* topic_setpoint = "raspberry/setpoint";

// --- PINES DE HARDWARE ---
const int pinPotenciometro = 36; // Pin VP (ADC1_CH0) típico en ESP32
const int pinLED = 2;            // LED integrado en la placa

// --- VARIABLES DE CONTROL ---
float temperaturaActual = 0.0;
float setpoint = 50.0;          // Setpoint inicial por defecto
unsigned long ultimoEnvio = 0;
const long intervalo = 1000;    // Envía datos cada 1 segundo (1000 ms)

WiFiClient espClient;
PubSubClient client(espClient);

// --- FUNCIÓN PARA CONECTAR AL WI-FI ---
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

  Serial.println("");
  Serial.println("¡Wi-Fi Conectado!");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

// --- FUNCIÓN DE RECEPCIÓN MQTT (Callback) ---
// Aquí llega lo que mandas desde la interfaz de Python (el Setpoint)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en el tópico [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String mensaje = "";
  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }
  Serial.println(mensaje);

  // Si el mensaje llega al tópico del Setpoint, actualizamos la variable
  if (String(topic) == topic_setpoint) {
    setpoint = mensaje.toFloat();
    Serial.print("Nuevo Setpoint configurado: ");
    Serial.println(setpoint);
  }
}

// --- FUNCIÓN PARA RECONECTAR AL BROKER MQTT ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT a la Raspberry...");
    // Intentamos conectar con un ID único
    if (client.connect("ESP32_PlantaClient")) {
      Serial.println("¡Conectado al Broker MQTT!");
      
      // Nos suscribimos al tópico del Setpoint que envía Python
      client.subscribe(topic_setpoint);
      
      // Publicamos el estado inicial del LED al conectar
      enviarEstadoLED();
    } else {
      Serial.print("Falló la conexión, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5 segundos...");
      delay(5000);
    }
  }
}

// --- FUNCIÓN AUXILIAR PARA ENVIAR EL ESTADO DEL LED ---
void enviarEstadoLED() {
  int estado = digitalRead(pinLED);
  String estadoStr = (estado == HIGH) ? "ENCENDIDO" : "APAGADO";
  client.publish(topic_led, estadoStr.c_str());
}

void setup() {
  Serial.begin(115200);
  pinMode(pinLED, OUTPUT);
  
  setup_wifi();
  
  // Configuración del servidor MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  // Asegurar que el MQTT permanezca conectado
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long ahora = millis();
  
  // Lógica de telemetría y control cada 1 segundo
  if (ahora - ultimoEnvio >= intervalo) {
    ultimoEnvio = ahora;

    // 1. Leer el potenciómetro (ADC de 0 a 4095)
    int lecturaRaw = analogRead(pinPotenciometro);
    
    // 2. Mapear la lectura al rango solicitado (25 a 80)
    // Usamos float para tener precisión decimal
    temperaturaActual = 25.0 + ((float)lecturaRaw / 4095.0) * (80.0 - 25.0);

    // 3. Publicar la temperatura actual hacia la interfaz de Python
    String tempStr = String(temperaturaActual, 1); // 1 decimal
    client.publish(topic_temp, tempStr.c_str());
    
    Serial.print("Temp Simulada: ");
    Serial.print(tempStr);
    Serial.print(" °C | Setpoint Actual: ");
    Serial.println(setpoint);

    // 4. Lógica de Termostato (Control ON/OFF del LED)
    int estadoAnteriorLED = digitalRead(pinLED);
    
    if (temperaturaActual >= setpoint) {
      digitalWrite(pinLED, HIGH); // Encender si supera o iguala el Setpoint
    } else {
      digitalWrite(pinLED, LOW);  // Apagar si está por debajo
    }

    // 5. Si el estado del LED cambió debido al termostato, se lo notificamos a Python de una
    if (digitalWrite != estadoAnteriorLED) {
      enviarEstadoLED();
    }
  }
}
