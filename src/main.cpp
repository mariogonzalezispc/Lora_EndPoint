#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Ticker.h>

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

// --- Frecuencia LoRa ---
#define LORA_FREQ 433E6

// --- Intervalos ---
#define LORA_CHECK_INTERVAL 10000
unsigned long Ultimo_Check_Lora = 0;

bool lora_Conectado = false;
volatile bool Lora_Mensaje_Recibido = false;

// --- Instancia de ticker para LED verde ---
Ticker tickerVerde;

// --- Interrupción de LoRa ---
void IRAM_ATTR onLoraDio0()
{
  Lora_Mensaje_Recibido = true;
}

// --- Leer registro REG_VERSION del SX127x por SPI ---
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
  // Inicializa SPI y pines LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  // Intenta iniciar LoRa en la frecuencia especificada
  if (!LoRa.begin(LORA_FREQ))
  {
    return false;
  }

  // Verifica versión del chip LoRa
  uint8_t version = readLoRaVersionSPI();
  if (version != 0x12 && version != 0x13)
  {
    // Serial.printf("Chip LoRa no detectado (REG_VERSION=0x%02X)\n", version);
    LoRa.end();
    return false;
  }

  // Configura interrupción en DIO0
  pinMode(LORA_DIO0, INPUT);
  attachInterrupt(digitalPinToInterrupt(LORA_DIO0), onLoraDio0, RISING);

  // Configuración de parámetros LoRa
  LoRa.setSpreadingFactor(7);     // SF: Factor de propagación
  LoRa.setSignalBandwidth(125E3); // BW: Ancho de banda
  LoRa.setCodingRate4(5);         // CR: Tasa de corrección de errores
  LoRa.setSyncWord(0x12);         // Palabra de sincronización (debe coincidir con transmisor)
  LoRa.enableCrc();               // Habilita verificación CRC

  Serial.println("LoRa iniciado correctamente");
  return true;
}

// --- Reintento de conexión ---
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

// --- Secuencia de parpadeo del LED verde ---
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

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("Iniciando receptor LoRa ESP32-C3...");

  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  digitalWrite(LED_ROJO, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_VERDE, LOW);
  lora_Conectado = iniciar_Lora();
  tickerVerde.once_ms(100, heartbeatSequence);
  Ultimo_Check_Lora = millis();
}

void loop()
{
  // Verificación periódica del módulo
  if (millis() - Ultimo_Check_Lora >= LORA_CHECK_INTERVAL)
  {
    Ultimo_Check_Lora = millis();
    uint8_t version = readLoRaVersionSPI();
    if (version != 0x12 && version != 0x13)
    {
      reconnectLora();
    }
  }
  if (!lora_Conectado)
  {
    return;
  }

  // --- Procesar mensajes recibidos ---
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0)
  {
    String msg;
    while (LoRa.available())
    {
      msg += (char)LoRa.read();
    }
    int rssi = LoRa.packetRssi();
    msg.trim();
    msg.toLowerCase();

    if (msg == "@")
    {
      digitalWrite(LED_ROJO, HIGH);
      delay(2000);
      digitalWrite(LED_ROJO, LOW);
    }
    else if (msg == "read")
    {
      digitalWrite(LED_AMARILLO, HIGH);
      delay(2000);
      digitalWrite(LED_AMARILLO, LOW);
    }
  }

  delay(10);
}
