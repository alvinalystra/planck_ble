# Planck Rev. 6 Bluetooh Support

BLUETOOTH_ENABLE しても対応しているコードが古いもので、Rev. 6 だと全然動かなくて、そもそも ARM だと動かないみたいだし、QMK のドキュメントの Serial Driver とか UART Driver とか並記されててよくわからないし、そのとおりにしても動かないようなので、いろいろためして動いたので、記録しておきます。

## 準備するもの

### Hardware

- [OLKB Planck Rev. 6 Keyboard](https://drop.com/buy/planck-mechanical-keyboard)
- [Bluefruit Feather nRF52 Bluefruit LE - nRF52832](https://www.adafruit.com/product/3406)
- Li-Po Battery (Option)

今なら Adafruit Feather nRF52840 Express の方がいいかも。

### Software

- QMK Firmware 0.12.35
- Arduino 1.8.15
- Adafruit nRF52 0.21.0

Adafruit nRF52 はこれをいろいろ試した時は 0.22.1 が最新だったんですけれども、それだとコンパイルが通らなかったので、0.21.0 を使ってます。これを書いている2021年7月4日現在では 0.24.0 が最新みたいです。更新早っ。

## 制限事項

- 6KRO だけしかサポートしてません。

## 準備

Planck Rev. 6 と Bluefruit Feather nRF52 Bluefruit LE - nRF52832 は以下の結線をします。

Planck - Bluefruit
Vcc - Vcc
GND - GND
SCL - RXD

## QMK の変更

BLUETOOTH_ENABLE のコードを参考にして以下を変更しました。

自分のkeymapのところで以下を変更します。
rules.mk
次の行を追加
```
TMK_COMMON_DEFS += -DBLUETOOTH2_ENABLE
```

halconf.h 新しく作る
```
#pragma once
#define HAL_USE_SERIAL TRUE
#include_next <halconf.h>
```

mcuconf.h 新しく作る
```
#include_next "mcuconf.h"
#undef STM32_SERIAL_USE_USART1
#define STM32_SERIAL_USE_USART1 TRUE
```

keymap.c
以下を追加
```
void serial_write(uint8_t c) {
  sdPut(&SD1, c);
}
```
keyoard_post_init_userの中に以下を追加
```
  palSetLineMode(B6, PAL_MODE_ALTERNATE(7) | PAL_STM32_OTYPE_OPENDRAIN);
  palSetLineMode(B7, PAL_MODE_ALTERNATE(7) | PAL_STM32_OTYPE_OPENDRAIN);
  static SerialConfig serialConfig = {115200, 0, 0, 0};
  sdStart(&SD1, &serialConfig);
```

tmk_core/common/host.c に以下を追加
```
#ifdef BLUETOOTH2_ENABLE
extern void serial_write(uint8_t c);
#endif
```
host_keyboard_send の最後に以下を追加
```
#ifdef BLUETOOTH2_ENABLE
    serial_write(0xFD);
    serial_write(0x09);
    serial_write(0x01);
    serial_write(report->mods);
    serial_write(report->reserved);
    for (uint8_t i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        serial_write(report->keys[i]);
    }
#endif
```
host_mouse_send の最後に以下を追加
```
#ifdef BLUETOOTH2_ENABLE
    serial_write(0xFD);
    serial_write(0x00);
    serial_write(0x03);
    serial_write(report->buttons);
    serial_write(report->x);
    serial_write(report->y);
    serial_write(report->v);  // should try sending the wheel v here
    serial_write(report->h);  // should try sending the wheel h here
    serial_write(0x00);
#endif
```
host_consumer_send の最後に以下を追加
```
#ifdef BLUETOOTH2_ENABLE
    static uint16_t last_report = 0;
    if (report == last_report) return;
    last_report = report;
    serial_write(0xFD);
    serial_write(0x03);
    serial_write(0x03);
    serial_write(report & 0xFF);
    serial_write((report >> 8) & 0xFF);
#endif
```
consumer 以外は BLUETOOTH_ENABLE で送っていたものに合わせてますが意味はないです。受け取る側の Arduino とあっていれば大丈夫です。

## Bluefruit

Planck_ble.ino を書き込みます。

## 使い方
Bluefruit に LiPo バッテリーをつなぐか、モバイルバッテリーをつなげば動きます。

Mac に接続することしか考えてませんが、Windows などにもつながると思います。

Cmd-1〜4で4箇所に接続できます。

Bluefruit が青い点滅をしている時に Mac から Setting の Bluetooth の Connect で接続します。

Logicool の Flow を擬似的にサポートしています。Flow を Ctrl を押している時のみ有効にしておけば、Alt-Ctrl を押し続けていれば 1台目と2台目の間で切り替えます。
Alt-Ctrl を500ms以上押し続けている場合に切り替えているだけですが、それなりに使えます。
