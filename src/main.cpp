#include <Adafruit_BMP280.h>
#include<Wire.h>

Adafruit_BMP280 bmp;

unsigned status;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  status = bmp.begin(0x76); // Aqui funcionou com esse endereço
  if(status!=1) status = bmp.begin(0x77); // Pode ser que o sensor inicialize nesse endereço
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Temperatura = ");
  Serial.println(bmp.readTemperature());
  delay(500);
}