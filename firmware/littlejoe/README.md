# LittleJoe - TinyUSB MIDI Monitor

## 概要

XIAO SAMD21 を使用した TinyUSB MIDI モニタリングデバイス。Mihashi から USB MIDI データを受信し、UART 経由で ASCII 形式として UartMonitor に出力。

## ハードウェア

- **基板**: Seeed XIAO SAMD21
- **CPU**: SAMD21G18A (ARM Cortex-M0+ @ 48MHz)
- **メモリ**: 32KB SRAM、256KB Flash
- **USB**: USB-C Device mode

## 接続

```
[Mihashi] ←USB A/C→ [LittleJoe] ←UART→ [UartMonitor]
```

### ピン配置

- **USB-C**: Mihashi との TinyUSB MIDI 通信
- **D6 (PA04)**: UART TX → UartMonitor RX
- **D7 (PA05)**: UART RX ← UartMonitor TX（将来用）
- **GND/3V3**: 電源・グランド

## セットアップ

### Arduino IDE 設定

1. **ボードマネージャ追加**
   ```
   https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
   ```

2. **ボード選択**
   ```
   ツール → ボード → Seeed SAMD Boards → Seeeduino XIAO
   ```

### 必要ライブラリ

```
ライブラリマネージャでインストール:
- TinyUSB (Adafruit TinyUSB Library)
- MIDI Library (FortySevenEffects)
- ArduinoJson (Benoit Blanchon)
```

## 機能

### MIDI メッセージ対応

- **Note On/Off**: チャンネル、ノート番号、ベロシティ
- **Control Change**: チャンネル、コントローラ番号、値
- **Program Change**: チャンネル、プログラム番号
- **Pitch Bend**: チャンネル、ベンド値
- **System Exclusive**: データ配列（最大50バイト表示）

### 出力フォーマット

JSON形式でタイムスタンプ付き出力:

```json
{
  "type": "note_on",
  "channel": 1,
  "note": 60,
  "velocity": 127,
  "timestamp": 1234567
}
```

## 使用方法

1. **コンパイル・アップロード**
   ```
   Arduino IDE → マイコンボードに書き込む
   ```

2. **接続確認**
   ```
   - Mihashi → LittleJoe: USB A/C ケーブル
   - LittleJoe → UartMonitor: UART配線
   ```

3. **動作確認**
   ```
   UartMonitor のシリアル出力でJSON形式のMIDIデータを確認
   ```

## トラブルシューティング

### TinyUSB が認識されない
- ライブラリバージョン確認
- ボード設定の再確認
- USB ケーブルの品質確認

### UART 出力が不安定
- ボーレート確認（115200）
- 配線確認（TX ↔ RX）
- GND 接続確認

## 開発ノート

- Serial1 使用で UART 出力（UartMonitor 用）
- TinyUSB_Device_Task() で USB 処理維持
- JSON 出力で構造化データ送信
- SysEx は50バイト制限で truncated フラグ
- millis() でタイムスタンプ付与

---

**開発環境**: Arduino IDE 2.x  
**対象ハードウェア**: Seeed XIAO SAMD21  
**出力先**: UartMonitor (UART)