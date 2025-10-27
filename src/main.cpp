// #include <Arduino.h>
// #include <SPI.h>
// #include <LoRa.h>
// #include <Ticker.h>

// // --- Pines LoRa RA-02 ---
// #define LORA_SCK 4
// #define LORA_MISO 5
// #define LORA_MOSI 6
// #define LORA_CS 7
// #define LORA_RST -1
// #define LORA_DIO0 3

// // --- Pines LEDs ---
// #define LED_ROJO 8
// #define LED_AMARILLO 9
// #define LED_VERDE 10

// // --- Frecuencia LoRa ---
// #define LORA_FREQ 433E6

// // --- Intervalos ---
// #define LORA_CHECK_INTERVAL 10000
// unsigned long Ultimo_Check_Lora = 0;

// bool lora_Conectado = false;
// volatile bool Lora_Mensaje_Recibido = false;

// // --- Instancia de ticker para LED verde ---
// Ticker tickerVerde;

// // --- Interrupción de LoRa ---
// void IRAM_ATTR onLoraDio0()
// {
//   Lora_Mensaje_Recibido = true;
// }

// // --- Leer registro REG_VERSION del SX127x por SPI ---
// uint8_t readLoRaVersionSPI()
// {
//   uint8_t version;
//   digitalWrite(LORA_CS, LOW);
//   SPI.transfer(0x42 & 0x7F);
//   version = SPI.transfer(0x00);
//   digitalWrite(LORA_CS, HIGH);
//   return version;
// }

// // --- Inicializar LoRa ---
// bool iniciar_Lora()
// {
//   // Inicializa SPI y pines LoRa
//   SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
//   LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

//   // Intenta iniciar LoRa en la frecuencia especificada
//   if (!LoRa.begin(LORA_FREQ))
//   {
//     return false;
//   }

//   // Verifica versión del chip LoRa
//   uint8_t version = readLoRaVersionSPI();
//   if (version != 0x12 && version != 0x13)
//   {
//     // Serial.printf("Chip LoRa no detectado (REG_VERSION=0x%02X)\n", version);
//     LoRa.end();
//     return false;
//   }

//   // Configura interrupción en DIO0
//   pinMode(LORA_DIO0, INPUT);
//   attachInterrupt(digitalPinToInterrupt(LORA_DIO0), onLoraDio0, RISING);

//   // Configuración de parámetros LoRa
//   LoRa.setSpreadingFactor(7);     // SF: Factor de propagación
//   LoRa.setSignalBandwidth(125E3); // BW: Ancho de banda
//   LoRa.setCodingRate4(5);         // CR: Tasa de corrección de errores
//   LoRa.setSyncWord(0x12);         // Palabra de sincronización (debe coincidir con transmisor)
//   LoRa.enableCrc();               // Habilita verificación CRC

//   Serial.println("LoRa iniciado correctamente");
//   return true;
// }

// // --- Reintento de conexión ---
// void reconnectLora()
// {
//   lora_Conectado = false;
//   digitalWrite(LED_VERDE, LOW);
//   LoRa.end();
//   delay(100);

//   while (!lora_Conectado)
//   {
//     if (iniciar_Lora())
//     {
//       lora_Conectado = true;
//       digitalWrite(LED_VERDE, HIGH);
//     }
//     else
//     {
//       digitalWrite(LED_ROJO, HIGH);
//       delay(100);
//       digitalWrite(LED_ROJO, LOW);
//       delay(200);
//     }
//   }
// }

// // --- Secuencia de parpadeo del LED verde ---
// void heartbeatSequence()
// {
//   static int step = 0;
//   switch (step)
//   {
//   case 0:
//     digitalWrite(LED_VERDE, HIGH);
//     tickerVerde.once_ms(55, heartbeatSequence);
//     break;
//   case 1:
//     digitalWrite(LED_VERDE, LOW);
//     tickerVerde.once_ms(100, heartbeatSequence);
//     break;
//   case 2:
//     digitalWrite(LED_VERDE, HIGH);
//     tickerVerde.once_ms(55, heartbeatSequence);
//     break;
//   case 3:
//     digitalWrite(LED_VERDE, LOW);
//     tickerVerde.once_ms(2500, heartbeatSequence);
//     break;
//   }
//   step = (step + 1) % 4;
// }

// void setup()
// {
//   Serial.begin(115200);
//   delay(500);
//   Serial.println("Iniciando receptor LoRa ESP32-C3...");

//   pinMode(LED_ROJO, OUTPUT);
//   pinMode(LED_AMARILLO, OUTPUT);
//   pinMode(LED_VERDE, OUTPUT);
//   digitalWrite(LED_ROJO, LOW);
//   digitalWrite(LED_AMARILLO, LOW);
//   digitalWrite(LED_VERDE, LOW);
//   lora_Conectado = iniciar_Lora();
//   tickerVerde.once_ms(100, heartbeatSequence);
//   Ultimo_Check_Lora = millis();
// }

// void loop()
// {
//   // Verificación periódica del módulo
//   if (millis() - Ultimo_Check_Lora >= LORA_CHECK_INTERVAL)
//   {
//     Ultimo_Check_Lora = millis();
//     uint8_t version = readLoRaVersionSPI();
//     if (version != 0x12 && version != 0x13)
//     {
//       reconnectLora();
//     }
//   }
//   if (!lora_Conectado)
//   {
//     return;
//   }

//   // --- Procesar mensajes recibidos ---
//   int packetSize = LoRa.parsePacket();
//   if (packetSize > 0)
//   {
//     String msg;
//     while (LoRa.available())
//     {
//       msg += (char)LoRa.read();
//     }
//     int rssi = LoRa.packetRssi();
//     msg.trim();
//     msg.toLowerCase();

//     if (msg == "@")
//     {
//       digitalWrite(LED_ROJO, HIGH);
//       delay(2000);
//       digitalWrite(LED_ROJO, LOW);
//     }
//     else if (msg == "read")
//     {
//       digitalWrite(LED_AMARILLO, HIGH);
//       delay(2000);
//       digitalWrite(LED_AMARILLO, LOW);
//     }
//   }

//   delay(10);
// }

// #include <Arduino.h>

// // Pines (según tu Conexiones EndPoint.pdf)
// #define RO_PIN    0   // RX desde MAX485 -> Receiver Output -> ESP32 RX (no usado para envío, pero declarado)
// #define DI_PIN    1   // TX hacia MAX485 -> Driver Input  -> ESP32 TX
// #define DE_RE_PIN 2   // Control DE+RE del MAX485 (HIGH=TX, LOW=RX)

// // LEDs (opcionales)
// #define LED_VERDE 10

// // Intervalo de envío (ms)
// const unsigned long INTERVALO_ENVIO = 5000;
// unsigned long ultimoEnvio = 0;

// void setup() {
//   // Serial USB para debug
//   Serial.begin(115200);
//   delay(100);
//   Serial.println();
//   Serial.println("Emisor RS485 (UART en pines específicos) - Inicio");

//   // Configurar pin DE/RE
//   pinMode(DE_RE_PIN, OUTPUT);
//   digitalWrite(DE_RE_PIN, LOW); // por defecto en recepción

//   // LED indicador de envío
//   pinMode(LED_VERDE, OUTPUT);
//   digitalWrite(LED_VERDE, LOW);

//   // Inicializar Serial1 en los pines RX, TX indicados:
//   // Serial1.begin(baud, config, rxPin, txPin);
//   // rxPin = RO_PIN (GPIO0), txPin = DI_PIN (GPIO1)
//   Serial1.begin(9600, SERIAL_8N1, RO_PIN, DI_PIN);
//   delay(10);

//   Serial.println("Serial1 inicializado: RX=GPIO0, TX=GPIO1, Baud=9600");
//   Serial.println("Listo para enviar 'ID_01' cada 5 segundos.");
// }

// void enviarMensajeRS485(const char *msg) {
//   // Poner el MAX485 en modo TRANSMISION
//   digitalWrite(DE_RE_PIN, HIGH);
//   // Pequeño retardo para estabilizar la línea antes de enviar
//   delay(2);

//   // Enviar por el UART mapeado a los pines físicos
//   Serial1.print(msg);
//   // Asegurarse de que todos los bytes se hayan enviado por el FIFO/UART
//   Serial1.flush();

//   // Pequeño retraso para garantizar que el transceiver haya transmitido la trama completa
//   delay(2);
//   // Volver a modo RECEPCION
//   digitalWrite(DE_RE_PIN, LOW);
// }

// void loop() {
//   unsigned long ahora = millis();

//   if (ahora - ultimoEnvio >= INTERVALO_ENVIO) {
//     ultimoEnvio = ahora;

//     // Mensaje a enviar (incluye '\n' para que el receptor pueda hacer readStringUntil('\n'))
//     const char *mensaje = "ID_01\n";

//     Serial.print("Enviando por RS485: ");
//     Serial.print(mensaje);

//     // Indicar envío con LED
//     digitalWrite(LED_VERDE, HIGH);

//     enviarMensajeRS485(mensaje);

//     // Apagar indicador
//     digitalWrite(LED_VERDE, LOW);

//     Serial.println(" -> Enviado y vuelto a RX");
//   }

//   // aquí podés hacer otras tareas sin bloquear
//   delay(10);
// }

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Ticker.h>
// #include <EspSoftwareSerial.h>

// --- Pines LoRa RA-02 ---
#define LORA_SCK 4
#define LORA_MISO 5
#define LORA_MOSI 6
#define LORA_CS 7
#define LORA_RST -1
#define LORA_DIO0 3

// --- Pines LEDs ---
#define LED_ROJO 8
#define LED_AMARILLO 9
#define LED_VERDE 10

// --- Pines RS485 (según tu PDF del endpoint) ---
#define DE_RE 2 // Dirección RS485 (GPIO2)
#define DI 1    // TX hacia MAX485
#define RO 0    // RX desde MAX485

// --- Frecuencia LoRa ---
#define LORA_FREQ 433E6

// --- Intervalos ---
#define LORA_CHECK_INTERVAL 10000
#define RS485_TIMEOUT 4000 // Tiempo máximo para esperar respuesta
unsigned long Ultimo_Check_Lora = 0;

bool lora_Conectado = false;
volatile bool Lora_Mensaje_Recibido = false;

// --- Objeto RS485 ---
HardwareSerial rs485(0); // UART0

// --- Ticker para LED verde ---
Ticker tickerVerde;

// --- Interrupción de LoRa ---
void IRAM_ATTR onLoraDio0()
{
  Lora_Mensaje_Recibido = true;
}

// --- Leer registro REG_VERSION del SX127x ---
uint8_t readLoRaVersionSPI()
{
  uint8_t version;
  digitalWrite(LORA_CS, LOW);
  SPI.transfer(0x42 & 0x7F);
  version = SPI.transfer(0x00);
  digitalWrite(LORA_CS, HIGH);
  return version;
}

// --- Inicializar LoRa ---
bool iniciar_Lora()
{
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ))
    return false;

  uint8_t version = readLoRaVersionSPI();
  if (version != 0x12 && version != 0x13)
  {
    LoRa.end();
    return false;
  }

  pinMode(LORA_DIO0, INPUT);
  attachInterrupt(digitalPinToInterrupt(LORA_DIO0), onLoraDio0, RISING);

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x12);
  LoRa.enableCrc();

  Serial.println("LoRa iniciado correctamente");
  return true;
}

// --- Reconexión LoRa ---
void reconnectLora()
{
  lora_Conectado = false;
  digitalWrite(LED_VERDE, LOW);
  LoRa.end();
  delay(100);

  while (!lora_Conectado)
  {
    if (iniciar_Lora())
    {
      lora_Conectado = true;
      digitalWrite(LED_VERDE, HIGH);
    }
    else
    {
      digitalWrite(LED_ROJO, HIGH);
      delay(100);
      digitalWrite(LED_ROJO, LOW);
      delay(200);
    }
  }
}

// --- LED verde heartbeat ---
void heartbeatSequence()
{
  static int step = 0;
  switch (step)
  {
  case 0:
    digitalWrite(LED_VERDE, HIGH);
    tickerVerde.once_ms(55, heartbeatSequence);
    break;
  case 1:
    digitalWrite(LED_VERDE, LOW);
    tickerVerde.once_ms(100, heartbeatSequence);
    break;
  case 2:
    digitalWrite(LED_VERDE, HIGH);
    tickerVerde.once_ms(55, heartbeatSequence);
    break;
  case 3:
    digitalWrite(LED_VERDE, LOW);
    tickerVerde.once_ms(2500, heartbeatSequence);
    break;
  }
  step = (step + 1) % 4;
}

// --- Enviar ID_01 y esperar respuesta ---
void solicitarLecturaRS485()
{
  Serial.println("→ Enviando comando RS485: ID_01");
  digitalWrite(DE_RE, HIGH);    // Modo transmisión
  digitalWrite(LED_ROJO, HIGH); // LED rojo ON mientras transmite
  delay(10);

  rs485.println("ID_01");
  rs485.flush();

  delay(10);
  digitalWrite(DE_RE, LOW); // Modo recepción
  digitalWrite(LED_ROJO, LOW);

  // --- Esperar respuesta ---
  unsigned long start = millis();
  String respuesta = "";
  while (millis() - start < RS485_TIMEOUT)
  {
    if (rs485.available())
    {
      digitalWrite(LED_AMARILLO, HIGH);
      char c = rs485.read();
      Serial.print(c); // debug
      respuesta += c;
      if (c == '&') // fin del mensaje
        break;
    }
  }

  if (respuesta.length() > 0)
  {
    digitalWrite(LED_AMARILLO, HIGH);
    Serial.print("← Respuesta RS485: ");
    Serial.println(respuesta);
    delay(300);
    digitalWrite(LED_AMARILLO, LOW);
  }
  else
  {
    Serial.println("No se recibió respuesta del sensor RS485.");
  }
}

void setup()
{
  Serial.begin(9600);
  delay(500);
  // LEDs
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  digitalWrite(LED_ROJO, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_VERDE, LOW);

  // RS485
  pinMode(DE_RE, OUTPUT);
  digitalWrite(DE_RE, LOW); // recepción por defecto
  rs485.begin(9600, SERIAL_8N1, RO, DI);
  delay(500);

  // LoRa
  lora_Conectado = iniciar_Lora();
  tickerVerde.once_ms(100, heartbeatSequence);
  Ultimo_Check_Lora = millis();
}

void loop()
{
  // --- Verificación del LoRa ---
  if (millis() - Ultimo_Check_Lora >= LORA_CHECK_INTERVAL)
  {
    Ultimo_Check_Lora = millis();
    uint8_t version = readLoRaVersionSPI();
    if (version != 0x12 && version != 0x13)
      reconnectLora();
  }

  if (!lora_Conectado)
    return;

  // --- Cada 10 segundos pedir lectura ---
  static unsigned long tLast = 0;
  if (millis() - tLast > 5000)
  {
    tLast = millis();
    solicitarLecturaRS485();
  }

  delay(10);
}
