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

bool loraConnected = false;
volatile bool loraMessageReceived = false;

// --- Instancia de ticker para LED verde ---
Ticker tickerVerde;

// --- Interrupción de LoRa ---
void IRAM_ATTR onLoraDio0()
{
  loraMessageReceived = true;
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
  Serial.println("Inicializando LoRa...");

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ))
  {
    Serial.println("Error al iniciar LoRa");
    return false;
  }

  uint8_t version = readLoRaVersionSPI();
  if (version != 0x12 && version != 0x13)
  {
    Serial.printf("Chip LoRa no detectado (REG_VERSION=0x%02X)\n", version);
    LoRa.end();
    return false;
  }

  Serial.println("LoRa detectado y configurado correctamente");

  pinMode(LORA_DIO0, INPUT);
  attachInterrupt(digitalPinToInterrupt(LORA_DIO0), onLoraDio0, RISING);

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x12);

  return true;
}

// --- Reintento de conexión ---
void reconnectLora()
{
  Serial.println("Intentando reconectar LoRa...");
  loraConnected = false;
  digitalWrite(LED_VERDE, LOW);

  for (int i = 0; i < 3; i++)
  {
    digitalWrite(LED_AMARILLO, HIGH);
    delay(100);
    digitalWrite(LED_AMARILLO, LOW);
    delay(100);
  }

  LoRa.end();
  delay(100);

  while (!loraConnected)
  {
    if (iniciar_Lora())
    {
      loraConnected = true;
      digitalWrite(LED_VERDE, HIGH);
      Serial.println("Reconectado correctamente");
    }
    else
    {
      Serial.println("Fallo al reconectar, reintentando...");
      digitalWrite(LED_ROJO, HIGH);
      delay(300);
      digitalWrite(LED_ROJO, LOW);
      delay(700);
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
    tickerVerde.once_ms(55, heartbeatSequence); // ON 50 ms
    break;
  case 1:
    digitalWrite(LED_VERDE, LOW);
    tickerVerde.once_ms(100, heartbeatSequence); // OFF 80 ms
    break;
  case 2:
    digitalWrite(LED_VERDE, HIGH);
    tickerVerde.once_ms(55, heartbeatSequence); // ON 50 ms
    break;
  case 3:
    digitalWrite(LED_VERDE, LOW);
    tickerVerde.once_ms(2500, heartbeatSequence); // OFF 3 s
    break;
  }
  step = (step + 1) % 4;
}


void setup()
{
  Serial.begin(115200);
  delay(1000);
  //Serial.println("Iniciando receptor LoRa ESP32-C3");

  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  digitalWrite(LED_ROJO, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_VERDE, LOW);

  loraConnected = iniciar_Lora();

  if (loraConnected)
  {
    digitalWrite(LED_VERDE, HIGH);
    Serial.println("Esperando mensajes LoRa...");
  }
  else
  {
    Serial.println("No se detectó módulo LoRa al inicio");
  }

  tickerVerde.once_ms(100, heartbeatSequence);
  Ultimo_Check_Lora = millis();
}

// --- Loop principal ---
void loop()
{
  if (millis() - Ultimo_Check_Lora >= LORA_CHECK_INTERVAL)
  {
    Ultimo_Check_Lora = millis();

    uint8_t version = readLoRaVersionSPI();

    if (version != 0x12 && version != 0x13)
    {
      if (loraConnected)
      {
        Serial.println("Módulo LoRa desconectado");
        reconnectLora();
      }
    }
    else if (!loraConnected)
    {
      Serial.println("Módulo LoRa detectado nuevamente");
      reconnectLora();
    }
  }

  if (!loraConnected)
    return;

  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    String msg;
    while (LoRa.available())
      msg += (char)LoRa.read();

    int rssi = LoRa.packetRssi();
    Serial.printf("Mensaje: %s | RSSI: %d\n", msg.c_str(), rssi);

    msg.trim();
    msg.toLowerCase();

    if (msg == "read")
    {
      Serial.println("Comando 'read' → LED rojo");
      digitalWrite(LED_ROJO, HIGH);
      delay(2000);
      digitalWrite(LED_ROJO, LOW);
    }
    else
    {
      Serial.println("Comando no reconocido");
      digitalWrite(LED_AMARILLO, HIGH);
      delay(300);
      digitalWrite(LED_AMARILLO, LOW);
    }
  }

  delay(10);
}
