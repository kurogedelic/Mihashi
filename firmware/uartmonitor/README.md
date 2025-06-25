# UartMonitor - UART to USB Serial Bridge

## 概要

XIAO SAMD21 を使用した UART-USB Serial ブリッジデバイス。LittleJoe から UART 経由で受信した ASCII 形式の MIDI データを USB Serial で GhostPC に転送。

## ハードウェア

- **基板**: Seeed XIAO SAMD21
- **CPU**: SAMD21G18A (ARM Cortex-M0+ @ 48MHz)
- **メモリ**: 32KB SRAM、256KB Flash
- **USB**: USB-C Device mode (USB Serial)

## 接続

```
[LittleJoe] ←UART→ [UartMonitor] ←USB C→ [GhostPC]
```

### ピン配置

- **USB-C**: GhostPC との USB Serial 通信
- **D6 (PA04)**: UART RX ← LittleJoe TX
- **D7 (PA05)**: UART TX → LittleJoe RX（将来用双方向通信）
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
- ArduinoJson (Benoit Blanchon)
```

## 機能

### 基本機能

- **UART → USB Serial ブリッジ**: リアルタイム転送
- **JSON 解析・検証**: データ整合性チェック
- **メッセージフィルタリング**: 不要なデータ除外
- **タイムスタンプ付与**: ブリッジ通過時刻記録
- **統計情報表示**: 処理状況監視

### フィルタリング機能

設定可能なメッセージタイプ:
- Note On/Off メッセージ
- Control Change
- Program Change  
- Pitch Bend
- System Exclusive

### 出力フォーマット

タイムスタンプ付きでデータ転送:

```
[1234567] {"type":"note_on","channel":1,"note":60,"velocity":127,"timestamp":1234560}
[1234580] {"type":"control_change","channel":1,"controller":7,"value":100,"timestamp":1234575}
```

## GhostPC での受信

### Arduino IDE シリアルモニター

```
ツール → シリアルモニター → 115200 baud
```

### Linux コマンドライン

```bash
# screen使用
screen /dev/ttyACM0 115200

# cat使用
cat /dev/ttyACM0

# ログ保存
cat /dev/ttyACM0 | tee midi_monitor.log
```

### Python スクリプト

```python
import serial
import json
import time

ser = serial.Serial('/dev/ttyACM0', 115200)
while True:
    if ser.in_waiting:
        line = ser.readline().decode('utf-8').strip()
        print(f'{time.strftime("%H:%M:%S")} {line}')
```

## 設定変更

### フィルタリング設定

コード内の boolean 変数で制御:

```cpp
bool enableNoteMessages = true;     // Note On/Off
bool enableControlChange = true;    // Control Change
bool enableProgramChange = true;    // Program Change
bool enablePitchBend = true;        // Pitch Bend
bool enableSysEx = true;           // System Exclusive
bool enableTimestamp = true;        // タイムスタンプ
bool enableStatistics = false;     // 統計情報（10秒毎）
```

### パフォーマンス設定

```cpp
#define UART_BAUD 115200      // UART通信速度
#define USB_BAUD 115200       // USB Serial通信速度  
#define BUFFER_SIZE 512       // 受信バッファサイズ
```

## トラブルシューティング

### USB Serial が認識されない

```bash
# デバイス確認
ls /dev/ttyACM*
lsusb | grep -i seeed

# 権限確認
sudo chmod 666 /dev/ttyACM0
```

### UART 通信エラー

- **ボーレート確認**: 115200 baud 設定
- **配線確認**: TX ↔ RX、GND 接続
- **電源確認**: 3.3V 供給状況

### バッファオーバーフロー

```
[ERROR] Buffer overflow, resetting
```
- BUFFER_SIZE を増加
- 処理速度を向上
- データ量を削減

## パフォーマンス最適化

### 高速処理版

buffer 管理とリングバッファ実装で高速化:

```cpp
#define RING_BUFFER_SIZE 1024
char ringBuffer[RING_BUFFER_SIZE];
volatile int head = 0, tail = 0;
```

### メモリ最適化

```cpp
// 静的メモリ使用
static char staticBuffer[256];

// JSON サイズ制限
StaticJsonDocument<200> compactDoc;
```

## 拡張機能

### 双方向通信

将来的に GhostPC → LittleJoe の制御コマンド送信:

```cpp
// USB Serial → UART 転送
while (Serial.available()) {
    char c = Serial.read();
    Serial1.write(c);
}
```

### 高度な監視

- WiFi モジュール追加でネットワーク監視
- SD カード追加でローカルログ保存
- OLED ディスプレイ追加で状態表示

## デバッグ機能

### 統計情報表示

```
=== UartMonitor Statistics ===
Messages processed: 1523
Bytes received: 89456
Uptime: 3600 seconds
Free memory: 24576
==============================
```

### エラー検出

- JSON パースエラー自動検出
- メモリ使用量監視
- 通信エラー検出・報告

---

**開発環境**: Arduino IDE 2.x  
**対象ハードウェア**: Seeed XIAO SAMD21  
**入力元**: LittleJoe (UART)  
**出力先**: GhostPC (USB Serial)