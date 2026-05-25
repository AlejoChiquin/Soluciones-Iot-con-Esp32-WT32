# Soluciones-Iot-con-Esp32-WT32
Ecosistema de soluciones industriales con ESP32 (WT32-ETH01). Servidor Web interactivo vía Wi-Fi/Ethernet, comunicación Modbus TCP, MQTT y procesamiento de señales y contadores.


# Ecosistema de Automatización IoT con ESP32 (WT32-ETH01)

Este repositorio es una bitácora de ingeniería y un banco de soluciones diseñado para transformar la placa **WT32-ETH01** en un puente de comunicación versátil para entornos industriales e IoT (Industria 4.0). 

El objetivo principal es centralizar diferentes arquitecturas de software capaces de interactuar con hardware industrial (PLCs), servidores en la nube y sensores, aprovechando al máximo la conectividad híbrida de este chip.

---

## 🛠️ ¿Qué se ha logrado hasta ahora? (Hitos Completados)

Actualmente, el repositorio cuenta con soluciones estables y probadas en banco de trabajo para los siguientes requerimientos:

* **Conectividad Servidor Web Híbrida:** Desarrollo de interfaces web interactivas y dinámicas (utilizando AJAX para actualización en tiempo real sin recarga de página). Funcionan con éxito tanto de forma cableada (**Ethernet Nativo**) como inalámbrica (**Wi-Fi**).
* **Integración Modbus TCP/IP:** Implementación completa del protocolo en modo Cliente (Master) para conectarse a PLCs.
* **Gestión y Escalado de Variables:** Procesamiento matemático de datos en formato decimal (ej. conversión de datos crudos de temperatura/setpoint de formato entero `180` a flotante `18.0°C`) bidireccional entre la interfaz web y los registros del PLC.
* **Control de Lazo Cerrado:** Lógica local de termostato para el accionamiento automatizado de salidas digitales (LED/Actuador) comparando lecturas dinámicas frente a Setpoints modificables desde la web.

---

## 🎯 Próximos Objetivos (En Desarrollo y Futuras Soluciones)

El proyecto está en constante expansión. Las siguientes etapas buscan integrar más herramientas críticas del entorno industrial:

* **Monitoreo de Variables Ambientales:** Añadir la lectura, escalado y visualización de Humedad Relativa (HR) junto con la temperatura actual.
* **Conectividad IoT Avanzada (MQTT):** Implementar el protocolo MQTT (mediante bróker Mosquitto) para la telemetría y envío de datos históricos hacia plataformas en la nube o servidores locales (Node-RED, Home Assistant, etc.).
* **Instrumentación y Conteo:** Desarrollar algoritmos optimizados para el manejo de entradas rápidas dedicadas a contadores de pulsos industriales (medición de flujo, producción, vueltas de motor).
* **Lectura de Señales Físicas:** Integrar el procesamiento de señales analógicas y digitales físicas protegidas del entorno real hacia el servidor web.

---

## ⚙️ Tecnologías Clave
* **Hardware:** WT32-ETH01 (Módulo ESP32 con chip LAN8720 para Ethernet).
* **Entorno:** IDE de Arduino.
* **Protocolos:** HTTP, TCP/IP, Modbus TCP, MQTT.
