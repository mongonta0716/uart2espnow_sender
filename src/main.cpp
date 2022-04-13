// uart2espnow sender
// UARTから受け取ったデータをESP-NOWで同報する
// Copyright (c) 2022 Takao Akaki

#include <Arduino.h>
#include <M5Unified.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

esp_now_peer_info_t esp_ap;
const uint8_t *peer_addr = esp_ap.peer_addr;
const esp_now_peer_info_t *peer = &esp_ap;

// ESP-NOWで使用するWiFiチャンネルを固定（※ルーターの設定も必要な場合があります。）
// 送信側と受信側でチャンネルを同じにしないといけません。
#define WIFI_DEFAULT_CHANNEL 1
#define WIFI_SECONDORY_CHANNEL 2

#define UART_BUFFER_MAX 1000
char uart_data[1000] = "\0";
uint16_t last_data_length = 0;
uint8_t data_send_count = 0;

char *json_buffer;

#define MAX_CLIENT 2   // 接続先のESP32の台数 
uint8_t mac[][6] = {
  { 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa },  // 接続先1のMACアドレス
  { 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb },  // 接続先2のMACアドレス
// { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC }, // 接続先の端末分だけ配列を用意する
};

void dataSend(int client_number, char* buffer, int data_length) {
  for (int j=0; j<6; j++) {
    esp_ap.peer_addr[j] = (uint8_t)mac[client_number][j];
  }
  if (esp_now_add_peer(peer) != ESP_OK){
    M5.Display.println("Failed to add peer");
  } else {
    //M5.Display.println("Success Peer");
    esp_now_send(peer_addr, (uint8_t *)buffer, data_length);
  }
  esp_now_del_peer(peer_addr);
}

void dataSend(const uint8_t *mac_addr, char* buffer, int data_length) {
  for (int j=0; j<6; j++) {
    esp_ap.peer_addr[j] = mac_addr[j];
  }
  if (esp_now_add_peer(peer) != ESP_OK){
    M5.Display.println("Failed to add peer");
  } else {
    //M5.Display.println("Success Peer");
    esp_now_send(peer_addr, (uint8_t *)buffer, data_length);
  }
  esp_now_del_peer(peer_addr);
}


void onRecvData(const uint8_t *mac_addr, const uint8_t *data, int len) {
  //M5.Display.println("onRecvData");
  // ログを画面表示したい場合は排他をかけないと失敗する場合あり
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
      mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  int data_length = 0;
  if (last_data_length > 240) {
    for (int i=0; i<240; i++) {
      json_buffer[i] = uart_data[i + 240 * data_send_count];
    }
    json_buffer[240] = '\0';
    strcat(json_buffer, "_NXT_");
    last_data_length = last_data_length - 240;
    data_length = 245;
    data_send_count++;
  } else {
    for (int i=0; i<last_data_length; i++) {
      json_buffer[i] = uart_data[i + 240 * data_send_count];
    }
    json_buffer[last_data_length] = '\0';
    strcat(json_buffer, "_END_");
    data_length = last_data_length + 5;
    last_data_length = 0;
    data_send_count = 0;
  }
  dataSend(mac_addr, json_buffer, data_length);
}


void UART_RX_IRQ() {
  M5.Display.println("UART Recv");
  if (Serial.available()) {
    int data_length = Serial.readBytesUntil('\n', uart_data, UART_BUFFER_MAX);
    M5.Display.printf("UART_DataLen:%d\n", data_length);
    if (data_length > 240) {
      for (int i=0; i<240; i++) {
        json_buffer[i] = uart_data[i];
      }
      json_buffer[240] = '\0';
      strcat(json_buffer, "_NXT_");
      last_data_length = data_length - 240;
      data_length = 245;
      data_send_count++;
    } else {
      for (int i=0; i<data_length; i++) {
        json_buffer[i] = uart_data[i];
      }
      json_buffer[data_length] = '\0';
      strcat(json_buffer, "_END_"); //("\n","_END_");
      data_length += 5;
      data_send_count = 0;
    }
    M5.Display.printf("json:%s\n", json_buffer);
    for (int i=0; i< MAX_CLIENT; i++) {
      dataSend(i, json_buffer, data_length);
    }
  }
}



void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.init();
  M5.Display.setRotation(3);
  M5.Display.setTextSize(2);
  M5.Display.println("UART Test");
  M5.Display.setTextScroll(true);
  M5.Display.setScrollRect(0, 15, M5.Display.width(), M5.Display.height() - 15);
  Serial.onReceive(UART_RX_IRQ);
  json_buffer = (char *)malloc(size_t(245));
  WiFi.mode(WIFI_STA);
  ESP_ERROR_CHECK(esp_wifi_set_channel(WIFI_DEFAULT_CHANNEL, WIFI_SECOND_CHAN_ABOVE));
  if (esp_now_init() == 0) {
    Serial.println("esp now init");
  } else {
    Serial.println("esp now init failed");
    M5.Display.println("esp now init failed");
  }
  esp_now_register_recv_cb(onRecvData);
}

void loop() {
  M5.update();
  if (M5.BtnA.wasClicked()) {
    ESP.restart();
  }
}