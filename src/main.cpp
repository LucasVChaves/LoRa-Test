#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <Arduino.h>

#define LORA_RX_PIN 16
#define LORA_TX_PIN 17

HardwareSerial loraSerial(2); // UART2
Adafruit_BMP280 bmp;

unsigned status;

void lora_send_default_msg() {
  const unsigned long max_time = 5000; // 5 seg
  static unsigned long last_send = 0;
  if (millis() - last_send > max_time) {
    String msg = "Hello " + String(millis());
    loraSerial.print(msg);
    Serial.println("Enviado via LoRa" + msg);
    last_send = millis();
  }
}

void lora_receive_default_msg() {
  String received = loraSerial.readString();
  Serial.print("Recebido:");
  Serial.println(received);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  status = bmp.begin(0x76); // Aqui funcionou com esse endereço
  if(status!=1) status = bmp.begin(0x77); // Pode ser que o sensor inicialize nesse endereço

  loraSerial.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
  Serial.println("LoRa E220 inicializado");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Temperatura = ");
  Serial.println(bmp.readTemperature());
  delay(500);

  lora_send_default_msg();
  lora_receive_default_msg();
  delay(500);
}