# Módulo 05: Servidor Modbus TCP en ESP32 para Integración Industrial (PAS800L)

Este proyecto implementa un servidor industrial Modbus TCP/IP sobre Wi-Fi utilizando un ESP32. Está diseñado específicamente para interactuar como esclavo de pasarelas industriales (Gateways) como el Schneider Electric EcoStruxure Panel Server PAS800L.

---

## ⚙️ Configuración del Firmware

* **Librería Utilizada:** `<ModbusTCP.h>` junto a `<WebServer.h>`.
* **Puerto de Red:** Estándar internacional Modbus TCP (**Puerto 502**).
* **Procesamiento de Datos:** Realiza la lectura analógica de un potenciómetro en el pin 36 y escala la variable mediante un mapeo matemático a un rango cerrado de **25 a 80**.
* **Mapeo de Registros:** El valor de temperatura se almacena continuamente en el **Holding Register 100** (mapeado en sistemas Schneider como la dirección de registro `40100` o `40101`).

---

## 🔌 Parámetros de Integración para el PAS800L

Para jalar los datos desde la interfaz web del Panel Server PAS800L, se deben configurar los siguientes parámetros en la sección de dispositivos Ethernet:

| Parámetro | Valor Configurado |
| :--- | :--- |
| **Tipo de Conexión** | Modbus TCP/IP |
| **Dirección IP** | IP dinámica asignada por el Wi-Fi (Ver Monitor Serial) |
| **Puerto (Port)** | `502` |
| **Device ID / Unit ID** | `1` |
| **Tipo de Registro** | Holding Register (Function Code 03) |
| **Dirección del Dato** | `100` (o `40100` según formato) |
| **Tipo de Variable** | INT16 (Entero de 16 bits firmado) |