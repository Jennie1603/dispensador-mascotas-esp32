# Dispensador Automatizado de Alimento para Mascotas (ESP32 + MQTT + Dashboard Web)

Proyecto de **Entregable 3** que simula un dispensador inteligente de alimento para mascotas utilizando un microcontrolador **ESP32** en **Wokwi**, comunicación **MQTT** en tiempo real y un **dashboard web (`Dashboard.html`)** para visualizar la telemetría y el estado de la compuerta.

## Arquitectura y Funcionamiento

- **ESP32:** controla los sensores y actuadores, evalúa la lógica de dispensado y publica el estado en formato JSON por MQTT.
- **Sensor de movimiento PIR (`GPIO 4`):** detecta cuando una mascota se acerca al dispensador.
- **Sensor ultrasónico HC-SR04 (`Trig: GPIO 5`, `Echo: GPIO 18`):** mide la distancia en la tolva para determinar si hay alimento:
  - `< 60 cm` (y dentro del rango mínimo configurado): **CON COMIDA** (permite dispensar).
  - `>= 60 cm` (o sin lectura válida): **SIN COMIDA** (bloquea la apertura y activa alerta).
- **Servomotor (`GPIO 23`):** abre la compuerta a `90°` durante el dispensado y regresa a `0°` al cerrar.
- **Buzzer (`GPIO 19`):** indica el proceso de dispensado o emite una alerta sonora intermitente cuando se detecta una mascota sin comida en la tolva.
- **Dashboard Web (`Dashboard.html`):** se conecta vía WebSockets (`wss://broker.emqx.io:8084/mqtt`) al topic configurado para mostrar en vivo:
  - Estado de conexión
  - Distancia en la tolva (`cm`)
  - Estado de la tolva (`CON COMIDA` / `SIN COMIDA`)
  - Estado de la compuerta (`ABIERTA (90°)`, `CERRADA (0°)`, `BLOQUEADA (0°)`)
  - Registro de eventos en tiempo real

## Estructura del Proyecto

- `sketch.ino`: firmware del ESP32 con lógica de sensores, servomotor, buzzer y publicación MQTT.
- `libraries.txt`: dependencias requeridas en Wokwi (`ESP32Servo`, `PubSubClient`).
- `Dashboard.html`: interfaz web en tiempo real para monitoreo y demostración.

## Cómo Ejecutar la Demostración

1. Abre el proyecto en **Wokwi** y verifica que `libraries.txt` contenga `ESP32Servo` y `PubSubClient`.
2. Inicia la simulación en Wokwi y espera a que el Monitor Serie muestre conexión a Wi-Fi y a MQTT.
3. Abre `Dashboard.html` en tu navegador y confirma que el estado indique que está en línea y recibiendo datos.
4. Ajusta la distancia del sensor **HC-SR04** y activa el **PIR** para probar tanto la dispensación con comida como el bloqueo y alerta sin comida.