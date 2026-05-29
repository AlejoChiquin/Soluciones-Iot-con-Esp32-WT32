# Módulo 04: Sistema de Control y Telemetría IoT (ESP32 + Raspberry Pi)

Este módulo implementa una arquitectura Cliente-Servidor para el monitoreo y control industrial asíncrono utilizando el protocolo MQTT. El ESP32 actúa como nodo sensor/actuador de campo y la Raspberry Pi funciona como el servidor central (Broker) y panel de control (HMI).

---

## 🛠️ 1. Configuración de la Raspberry Pi

### Instalación del Servidor MQTT (Mosquitto)
Para convertir la Raspberry Pi en el Broker central de mensajería, ejecuta los siguientes comandos en la terminal:

```bash
sudo apt update
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto
sudo systemctl start mosquitto



Apertura de Permisos de Red (Paso Crítico)
Por defecto, Mosquitto bloquea conexiones externas. Para permitir que el ESP32 se conecte, edita el archivo de configuración:

sudo nano /etc/mosquitto/mosquitto.conf

Ve al final del archivo y agrega estas dos líneas:

listener 1883
allow_anonymous true

Guarda con Ctrl+O, Enter y sal con Ctrl+X.

Reinicia el servidor para aplicar los cambios:

sudo systemctl restart mosquitto

Entorno de Python para la Interfaz Gráfica
Instala la librería necesaria para que Python se comunique por MQTT:

pip3 install paho-mqtt

Para arrancar el panel de control visual, ejecuta:

python3 interfaz_iot.py


2. Configuración del ESP32
El firmware del ESP32 se encarga de la lógica de control local y el envío de telemetría:

Librería requerida: PubSubClient (por Nick O'Leary) instalada desde el gestor de librerías del IDE de Arduino.

Procesamiento de variables: Lee un potenciómetro en el pin 36 (ADC 12 bits) y mapea de forma matemática la lectura a un rango dinámico real de 25.0 °C a 80.0 °C.

Lógica de Termostato: Realiza un control ON/OFF asíncrono sobre el LED integrado. Si la temperatura supera el Setpoint enviado desde Python, el LED se enciende; de lo contrario, se apaga.

Eficiencia de red: Solo publica el estado del LED cuando detecta un cambio real, evitando saturar el tráfico del broker.

3. Tópicos MQTT UtilizadosTópicoDirecciónDescripciónesp32/temperaturaESP32 ➡️ Raspberry PiEnvía la lectura del potenciómetro filtrada a 1 decimal.esp32/ledESP32 ➡️ Raspberry PiNotifica el estado actual del LED (ENCENDIDO/APAGADO).raspberry/setpointRaspberry Pi ➡️ ESP32Envía el valor límite de control configurado en la interfaz.