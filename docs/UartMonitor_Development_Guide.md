# UartMonitor 開発ガイド（XIAO SAMD21 UART→USB Serial Bridge）

## 概要

**UartMonitor** は Seeed XIAO SAMD21 を使用した **UART-USB Serial ブリッジデバイス** です。LittleJoe から UART 経由で受信した ASCII 形式の MIDI データを USB Serial で GhostPC に転送し、リアルタイム監視を可能にします。

---

## ハードウェア仕様

### 基板: Seeed XIAO SAMD21

- **CPU**: SAMD21G18A (ARM Cortex-M0+ @ 48MHz)
- **メモリ**: 32KB SRAM、256KB Flash
- **USB**: USB-C（Device mode、USB Serial対応）
- **GPIO**: 11ピン（うち8ピンがアナログ対応）
- **サイズ**: 20×17.5mm（超小型）
- **電源**: 3.3V/5V両対応

### 接続構成

```
[LittleJoe] ←UART→ [UartMonitor] ←USB C→ [GhostPC]
```

**ピン割り当て**:
- USB-C: GhostPC との USB Serial 通信
- D6 (PA04): UART RX ← LittleJoe TX
- D7 (PA05): UART TX → LittleJoe RX（将来用）  
- GND/3V3: 電源・グランド ← LittleJoe

---

## 開発環境セットアップ

### Arduino IDE設定

1. **ボードマネージャ設定**
   ```
   Arduino IDE → 環境設定 → 追加ボードマネージャURL:
   https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
   ```

2. **Seeed SAMD ボードインストール**
   ```
   ツール → ボード → ボードマネージャ → "Seeed SAMD Boards" をインストール
   ```

3. **ボード選択**
   ```
   ツール → ボード → Seeed SAMD Boards → Seeeduino XIAO
   ```

---

## ソフトウェア実装

### メイン機能構成

```arduino
void setup() {
    // USB Serial通信初期化（GhostPC用）
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    // UART通信初期化（LittleJoe用）
    Serial1.begin(115200);
    
    Serial.println("UartMonitor USB-Serial Bridge Ready");
}

void loop() {
    // UART → USB Serial 転送
    while (Serial1.available()) {
        char c = Serial1.read();
        Serial.write(c);
    }
    
    // USB Serial → UART 転送（将来用）
    while (Serial.available()) {
        char c = Serial.read();
        Serial1.write(c);
    }
}
```

### 高機能版実装

```arduino
#include <ArduinoJson.h>

// バッファ管理
#define BUFFER_SIZE 256
char uartBuffer[BUFFER_SIZE];
int bufferIndex = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial1.begin(115200);
    
    Serial.println("UartMonitor Enhanced Bridge Ready");
    Serial.println("Format: JSON MIDI Monitor Data");
}

void loop() {
    // UART から1行読み取り
    while (Serial1.available()) {
        char c = Serial1.read();
        
        if (c == '\\n') {
            // 行完了 - USB Serial に送信
            uartBuffer[bufferIndex] = '\\0';
            processLine(uartBuffer);
            bufferIndex = 0;
        } else if (bufferIndex < BUFFER_SIZE - 1) {
            uartBuffer[bufferIndex++] = c;
        }
    }
    
    delay(1);
}

void processLine(const char* line) {
    // タイムスタンプ付きで転送
    Serial.print("[");
    Serial.print(millis());
    Serial.print("] ");
    Serial.println(line);
    
    // JSON検証（オプション）
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, line);
    
    if (error) {
        Serial.print("JSON Parse Error: ");
        Serial.println(error.c_str());
    }
}
```

### フィルタリング機能

```arduino
// MIDI メッセージタイプフィルタ
bool enableNoteMessages = true;
bool enableControlChange = true;
bool enableSysEx = false;

void processFilteredLine(const char* line) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, line);
    
    if (!error) {
        const char* type = doc["type"];
        
        // フィルタリング
        if ((strcmp(type, "note_on") == 0 || strcmp(type, "note_off") == 0) && !enableNoteMessages) {
            return;
        }
        if (strcmp(type, "control_change") == 0 && !enableControlChange) {
            return;
        }
        if (strcmp(type, "sysex") == 0 && !enableSysEx) {
            return;
        }
    }
    
    // フィルタ通過 - 転送
    Serial.println(line);
}
```

---

## GhostPC での受信確認

### シリアルモニター使用

```bash
# Arduino IDE シリアルモニター
ツール → シリアルモニター → 115200 baud

# Linux/Ubuntu での確認
sudo apt install screen
screen /dev/ttyACM0 115200

# Python での受信
python3 -c "
import serial
import time

ser = serial.Serial('/dev/ttyACM0', 115200)
while True:
    if ser.in_waiting:
        line = ser.readline().decode('utf-8').strip()
        print(f'{time.strftime(\"%H:%M:%S\")} {line}')
"
```

### ログファイル保存

```bash
# リアルタイムログ保存
screen /dev/ttyACM0 115200 | tee midi_monitor.log

# Python での詳細ログ
python3 << 'EOF'
import serial
import json
import datetime

ser = serial.Serial('/dev/ttyACM0', 115200)
log_file = open('midi_detailed.log', 'w')

while True:
    if ser.in_waiting:
        line = ser.readline().decode('utf-8').strip()
        timestamp = datetime.datetime.now().isoformat()
        
        log_entry = {
            'timestamp': timestamp,
            'raw_data': line
        }
        
        try:
            midi_data = json.loads(line)
            log_entry['midi'] = midi_data
        except:
            log_entry['parse_error'] = True
        
        log_file.write(json.dumps(log_entry) + '\\n')
        log_file.flush()
        print(f"{timestamp}: {line}")
EOF
```

---

## パフォーマンス最適化

### バッファリング最適化

```arduino
// 高速バッファリング
#define UART_BUFFER_SIZE 512
#define USB_BUFFER_SIZE 1024

char uartRingBuffer[UART_BUFFER_SIZE];
char usbBuffer[USB_BUFFER_SIZE];
volatile int uartHead = 0, uartTail = 0;
int usbIndex = 0;

void setup() {
    Serial.begin(115200);
    Serial1.begin(115200);
    
    // UART割り込み有効化
    Serial1.setTimeout(1);
}

void loop() {
    // バッファ処理
    processUartBuffer();
    flushUsbBuffer();
    delay(1);
}

void processUartBuffer() {
    while (Serial1.available() && uartHead != (uartTail + 1) % UART_BUFFER_SIZE) {
        uartRingBuffer[uartHead] = Serial1.read();
        uartHead = (uartHead + 1) % UART_BUFFER_SIZE;
    }
    
    // リングバッファから読み取り
    while (uartTail != uartHead) {
        char c = uartRingBuffer[uartTail];
        uartTail = (uartTail + 1) % UART_BUFFER_SIZE;
        
        if (c == '\\n') {
            flushUsbBuffer();
        } else if (usbIndex < USB_BUFFER_SIZE - 1) {
            usbBuffer[usbIndex++] = c;
        }
    }
}

void flushUsbBuffer() {
    if (usbIndex > 0) {
        usbBuffer[usbIndex] = '\\0';
        Serial.println(usbBuffer);
        usbIndex = 0;
    }
}
```

---

## トラブルシューティング

### よくある問題

1. **USB Serial が認識されない**
   ```bash
   # デバイス確認
   ls /dev/ttyACM*
   lsusb | grep -i seeed
   
   # ドライバ確認
   dmesg | tail
   ```

2. **UART通信が不安定**
   - ボーレート確認（115200 baud）
   - 配線確認（TX ↔ RX、GND接続）
   - 電源電圧確認（3.3V）

3. **データ取りこぼし**
   - バッファサイズ増加
   - 処理間隔短縮
   - フロー制御追加

### デバッグ機能

```arduino
// デバッグモード
#define DEBUG_MODE 1

void debugPrint(const char* message) {
    #if DEBUG_MODE
    Serial.print("[DEBUG] ");
    Serial.println(message);
    #endif
}

void debugStats() {
    #if DEBUG_MODE
    Serial.print("Free Memory: ");
    Serial.println(freeMemory());
    Serial.print("Buffer Usage: ");
    Serial.print(usbIndex);
    Serial.print("/");
    Serial.println(USB_BUFFER_SIZE);
    #endif
}
```

---

## 拡張機能

### Web インターフェース（将来）
- WiFi モジュール追加でブラウザ監視
- リアルタイム MIDI 可視化
- ログダウンロード機能

### OLED ディスプレイ
- 小型 OLED で MIDI 状態表示
- 統計情報の表示
- エラー状態の視覚的通知

---

**最終更新**: 2025-06-24  
**対象ハードウェア**: Seeed XIAO SAMD21  
**開発環境**: Arduino IDE 2.x  
**入力元**: LittleJoe (XIAO SAMD21)  
**出力先**: GhostPC (USB Serial)