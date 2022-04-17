# uart2espnow_sender
UARTで受け取ったデータをESP-NOWで同報するファームウェア

# 概要
M5StickCPlusやM5Atomに書き込んでPCやラズパイとUSBで接続し、テキストデータ(JSON)を送信するために作成しました。

UART（Serial）から受け取ったデータをESP-NOWで複数の端末に送ることができます。ESP-NOWの制限(上限250バイト)で240バイトまでのデータに収めてください。
※ 分割する機能も実装していますが２台以上送信しようとすると動きません。

[stackchan_test](https://github.com/mongonta0716/stackchan_test)と組み合わせて利用します。

# 設定箇所

## WiFi Channel

ESP-NOWの仕組み上WiFiのチャンネルを送信側、受信側で固定する必要があります。ルーターに接続した場合は自動で設定されてしまう場合もあるので場合によってはWiFiルーターの設定を行う必要もあります。

※ WiFiルーターを経由しない場合は、WIFI_DEFAULT_CHANNELの値が送信側、受信側で同じであれば大丈夫です。

```
#define WIFI_DEFAULT_CHANNEL 8
```

## 受信側のMacアドレスと接続台数の設定

あらかじめ、受信側のM5StackやESP32のMACアドレスを設定しておきます。複数同時に同じデータを送信することも可能です。

```
#define MAX_CLIENT 2   // 接続先のESP32の台数 
uint8_t mac[][6] = {
  { 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX },  // 接続先1のMACアドレス
  { 0xYY, 0xYY, 0xYY, 0xYY, 0xYY, 0xYY },  // 接続先2のMACアドレス
// { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC }, // 接続先の端末分だけ配列を用意する
};
```

# ビルド
PlatformIO用です。

ArduinoIDEで利用する場合はsrcフォルダとmain.cppの名前を揃えて変更してください。

## 例
src -> uart2espnow_sender

main.cpp -> uart2espnow_sender.ino

# 必要なライブラリ

## [M5Unified](https://github.com/m5stack/M5Unified)
※ arduino-esp32 2.0.3RC1の変更のため、M5Unified v0.0.6だとテキストスクロールが動きません。2022/04/13時点ではM5GFXとM5Unifiedのdevelopブランチを利用するとテキストスクロールが動きます。

## arduino-esp32は2.0.3RC1が必要

UARTの割り込みを使うため、2.0.3RC1の最新機能を利用しています。2.0.2以下では動きません。

# PC側のプログラム例

JSONデータをシリアルに書き込む例です。

```
import serial
import json

data = {"aqtalk": "konnichiwa.", "motion":0, "expression":0, "test11": 1, "test12": 1, "test13": 1 , "test14": 1, "tes5": 1} 
print(len(data))
ser = serial.Serial('/dev/ttyUSB0', 115200)
send_data = json.dumps(data)
print(len(send_data))
ser.write(send_data.encode('utf-8') + "\n".encode('utf-8'))

ser.close();
```

# Author

[Takao Akaki](https://github.com/mongonta0716)

# License
[MIT](https://github.com/mongonta0716/uart2espnow_sender/blob/main/LICENSE)

