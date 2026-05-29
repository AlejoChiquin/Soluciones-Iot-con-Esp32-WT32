import tkinter as tk
from tkinter import ttk
import paho.mqtt.client as mqtt

# --- CONFIGURACIÓN MQTT ---
MQTT_BROKER = "localhost" # Como corre en la misma Raspberry, es localhost
TOPIC_TEMPERATURA = "esp32/temperatura"
TOPIC_LED = "esp32/led"
TOPIC_SETPOINT = "raspberry/setpoint"

# --- FUNCIONES MQTT ---
def on_connect(client, userdata, flags, rc):
    print("Conectado al Broker MQTT con código: " + str(rc))
    # Nos suscribimos a los datos que envía el ESP32
    client.subscribe(TOPIC_TEMPERATURA)
    client.subscribe(TOPIC_LED)

def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    # Si llega la temperatura, actualizamos la interfaz
    if msg.topic == TOPIC_TEMPERATURA:
        lbl_temp_val.config(text=f"{payload} °C")
    # Si llega el estado del LED
    elif msg.topic == TOPIC_LED:
        if payload == "1" or payload.upper() == "ENCENDIDO":
            lbl_led_val.config(text="ENCENDIDO", foreground="green")
        else:
            lbl_led_val.config(text="APAGADO", foreground="red")

def enviar_setpoint():
    nuevo_sp = entry_sp.get()
    if nuevo_sp:
        # Enviamos el nuevo Setpoint al ESP32
        client.publish(TOPIC_SETPOINT, nuevo_sp)
        print(f"Setpoint enviado: {nuevo_sp}")

# --- CONFIGURACIÓN DE LA INTERFAZ GRÁFICA (TKINTER) ---
root = tk.Tk()
root.title("Panel IoT - AlejoChq")
root.geometry("400x350")
root.configure(bg="#1e1e1e") # Estilo oscuro criminal

style = ttk.Style()
style.theme_use('clam')

# Título
lbl_titulo = tk.Label(root, text="CONTROL DE PLANTA ESP32", font=("Arial", 16, "bold"), bg="#1e1e1e", fg="white")
lbl_titulo.pack(pady=15)

# Contenedor de Datos
frame_datos = tk.Frame(root, bg="#2d2d2d", bd=2, relief="groove")
frame_datos.pack(pady=10, fill="x", padx=20)

# Etiqueta Temperatura
lbl_temp = tk.Label(frame_datos, text="Temp. Potenciómetro:", font=("Arial", 12), bg="#2d2d2d", fg="white")
lbl_temp.grid(row=0, column=0, padx=10, pady=10, sticky="w")
lbl_temp_val = tk.Label(frame_datos, text="--- °C", font=("Arial", 14, "bold"), bg="#2d2d2d", fg="#00adb5")
lbl_temp_val.grid(row=0, column=1, padx=10, pady=10)

# Etiqueta LED
lbl_led = tk.Label(frame_datos, text="Estado del LED:", font=("Arial", 12), bg="#2d2d2d", fg="white")
lbl_led.grid(row=1, column=0, padx=10, pady=10, sticky="w")
lbl_led_val = tk.Label(frame_datos, text="DESCONECTADO", font=("Arial", 14, "bold"), bg="#2d2d2d", fg="orange")
lbl_led_val.grid(row=1, column=1, padx=10, pady=10)

# Contenedor de Control (Setpoint)
frame_control = tk.Frame(root, bg="#1e1e1e")
frame_control.pack(pady=20)

lbl_sp = tk.Label(frame_control, text="Nuevo Setpoint:", font=("Arial", 11), bg="#1e1e1e", fg="white")
lbl_sp.pack(side="left", padx=5)

entry_sp = ttk.Entry(frame_control, width=8, font=("Arial", 12))
entry_sp.pack(side="left", padx=5)

btn_enviar = ttk.Button(frame_control, text="Enviar SP", command=enviar_setpoint)
btn_enviar.pack(side="left", padx=5)

# --- INICIAR MQTT EN SEGUNDO PLANO ---
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

try:
    client.connect(MQTT_BROKER, 1883, 60)
    client.loop_start() # Arranca el MQTT sin congelar la ventana
except Exception as e:
    print(f"No se pudo conectar al broker: {e}")

# Arrancar la interfaz gráfica
root.mainloop()