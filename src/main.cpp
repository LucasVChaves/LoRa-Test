#include <Arduino.h>
#include "LoRa_E220.h"

/* LORA PINS
*  O Modulo E220 tambem tem os pinos AUX, M0 E M1
*  AUX eh utilizado para debugging do modulo
*  M0 e M1 para escolha do modo:
*     - 0 | 0 Modo normal
*     - 0 | 1 Wake On Radio Transmit com suporte para Wake-On-Air
*     - 1 | 0 Wake On Radio Transmit
*     - 1 | 1 Wake On Radio Receive
*/
#define LORA_RX_PIN 16 // RX ESP -> TX LORA
#define LORA_TX_PIN 17 // TX ESP -> RX LORA
#define LORA_AUX_PIN 18
#define LORA_M0_PIN 19
#define LORA_M1_PIN 21

#define LED_BUILTIN 2

LoRa_E220 e22ttl(&Serial2, LORA_AUX_PIN, LORA_M0_PIN, LORA_M1_PIN);

// Debug LED
unsigned long led_on_time = 0;
const unsigned long led_duration = 500; // 500 ms
bool led_state = false;

struct ReceivedMsg {
    char data[100];
    int rssi;
    bool valid;
};

/*
 * Funcao para receber mensagens LoRa
 * Retorna uma estrutura com os dados recebidos e informações do sinal
 */
ReceivedMsg receive_lora_msg() {
    ReceivedMsg msg;
    msg.valid = false;

    ResponseContainer rc = e22ttl.receiveMessage();
    if (rc.status.code == 1) {
        String received_data = rc.data;
        received_data.toCharArray(msg.data, sizeof(msg.data));
        msg.rssi = rc.rssi;
        msg.valid = true;

        Serial.println("Mensagem LoRa Recebida");
    } else {
        Serial.println("Erro ao receber mensagem LoRa");
        Serial.print("Erro: ");
        Serial.println(rc.status.getResponseDescription());
    }

    return msg;
}

/* Manda uma mensagem qualquer como resposta para a rede LoRa
*  Uso opcional
*/
void send_response() {
    String resp = "ACK: Mensagem recebida pelo ESP\n"; // Msg de acknowledgment

    ResponseStatus rs = e22ttl.sendMessage(resp);
    if (rs.code != 1) {
        Serial.print("Erro ao enviar mensagem de resposta:");
        Serial.println(rs.getResponseDescription());
    }
}

/* Funcao de teste de envio de mensagens
*  Pode colocar em qualquer lugar pra testar o envio e o canal
*/
void send_test_msg() {
    String test_msg = "TESTE ESP32 -  " + String(millis());
    ResponseStatus rs = e22ttl.sendMessage(test_msg);
    if (rs.code == 1) {
        Serial.println("Mensagem de teste enviada");
    } else {
        Serial.print("Erro ao enviar mensagem de teste: ");
        Serial.println(rs.getResponseDescription());
    }
}

/* Info a mensagem recebida
*/
void display_received_msg(const ReceivedMsg& msg) {
    Serial.println("\n--- MENSAGEM RECEBIDA ---");
    Serial.print("Dados: ");
    Serial.println(msg.data);
    Serial.print("RSSI: ");
    Serial.print(msg.rssi);
    Serial.println(" dBm");
    Serial.print("Timestamp: ");
    Serial.print(millis());
    Serial.println(" ms");
}

/* flip flop do led de debug
*/
void turn_led() {
    if (led_state && (millis() - led_on_time >= led_duration)) {
        digitalWrite(LED_BUILTIN, LOW);
        led_state = false;
    } else {
        digitalWrite(LED_BUILTIN, HIGH);
        led_state = true;
    }
}

void setup() {
    // Serial de debug
    Serial.begin(115200);
    delay(200); // Coloquei esse delay pq tava imprimindo antes de iniciar o Serial e bugando tudo
    Serial.println("=== ESP32 LoRa Transceiver ===");

    pinMode(LORA_M0_PIN, OUTPUT);
    pinMode(LORA_M1_PIN, OUTPUT);

    // OS ENDPOINTS SERÃO TRANSMIT E OS GATEWAYS RECEIVER POR AGORA
    // Na protoboard o TRANSMIT é o com termômetro e o RECEIVER o sem
    // WOR TRANSMIT MODE
    digitalWrite(LORA_M0_PIN, HIGH);
    digitalWrite(LORA_M1_PIN, HIGH);
    // WOR RECEIVE MODE
    //digitalWrite(LORA_M0_PIN, HIGH);
    //digitalWrite(LORA_M1_PIN, LOW);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Serial UART com o E220
    Serial2.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
    Serial.println("Inicializando LoRa");
    e22ttl.begin();

    // Delay pra estabilizacao recomendado pela ebyte
    delay(1000); 

    Serial.println("Configuracao do Modulo:");
    ResponseStructContainer config_container;
    config_container = e22ttl.getConfiguration();
    if (config_container.status.code == 1) {
        Configuration config = *(Configuration *) config_container.data;
        Serial.print("Channel: "); 
        Serial.println(config.CHAN, HEX);
        Serial.print("Addr High: "); 
        Serial.println(config.ADDH, HEX);
        Serial.print("Addr Low: "); 
        Serial.println(config.ADDL, HEX);

        // Aqui eh possivel mudar canal e enderecos sem precisar usar aquele programa chines estranho
        config_container.close();
    } else {
        Serial.println("Erro: Nao foi possivel ler a configuracao do modulo");
        Serial.println(config_container.status.getResponseDescription());
    }
}

void loop() {
    send_test_msg();

    if (e22ttl.available() > 1) {
        ReceivedMsg msg = receive_lora_msg();
        if (msg.valid) {
            turn_led();
            display_received_msg(msg);

            // Opcional
            //send_response();
        }
    }

    turn_led();
    delay(20);
}