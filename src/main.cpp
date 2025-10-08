#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_bt.h>

#define LED_ROJO 8
#define LED_AMARILLO 9
#define LED_VERDE 10

void setup()
{
  // Inicializar comunicación serial para debug
  Serial.begin(9600);
  WiFi.mode(WIFI_OFF);

  // Desactivar WiFi completamente
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  esp_wifi_deinit();

  // Desactivar Bluetooth
  btStop();
  esp_bt_controller_deinit();
  esp_bt_mem_release(ESP_BT_MODE_BTDM);

  // Configurar pines como salida
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);

  // Apagar ambos LEDs al inicio
  digitalWrite(LED_ROJO, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_VERDE, LOW);

  Serial.println("LEDs inicializados");
}
void loop()
{
  // Encender LED verde
  Serial.println("LED Verde ON");
  digitalWrite(LED_VERDE, HIGH);
  delay(200);

  // Apagar LED verde
  Serial.println("LED Verde OFF");
  digitalWrite(LED_VERDE, LOW);
  delay(100);

  // Encender LED amarillo
  Serial.println("LED Amarillo ON");
  digitalWrite(LED_AMARILLO, HIGH);
  delay(200);

  // Apagar LED amarillo
  Serial.println("LED Amarillo OFF");
  digitalWrite(LED_AMARILLO, LOW);
  delay(100);

  // Encender Led Rojo
  Serial.println("LED Rojo ON");
  digitalWrite(LED_ROJO, HIGH);
  delay(200);

  // Apagar LED rojo
  Serial.println("LED Rojo OFF");
  digitalWrite(LED_ROJO, LOW);
  delay(100);
}
