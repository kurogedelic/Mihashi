# LittleJoe 開発ガイド（XIAO SAMD21 TinyUSB MIDI Monitor）

## 概要

**LittleJoe** は Seeed XIAO SAMD21 を使用した **TinyUSB MIDI モニタリングデバイス** です。Mihashi から受信した USB MIDI データを TinyUSB MIDI ライブラリでリアルタイム解析し、UART 経由で ASCII 形式として UartMonitor に出力します。UartMonitor は USB Serial でデータを GhostPC に送信し、リアルタイム監視を実現します。

---

## ハードウェア仕様

### 基板: Seeed XIAO SAMD21

- **CPU**: SAMD21G18A (ARM Cortex-M0+ @ 48MHz)
- **メモリ**: 32KB SRAM、256KB Flash
- **USB**: USB-C（Device mode、TinyUSB対応）
- **GPIO**: 11ピン（うち8ピンがアナログ対応）
- **サイズ**: 20×17.5mm（超小型）
- **電源**: 3.3V/5V両対応

### 接続構成

```
[Mihashi] ←USB A/C→ [LittleJoe] ←UART→ [UartMonitor] ←USB C→ [GhostPC]
```

**ピン割り当て**:
- USB-C: Mihashi との TinyUSB MIDI 通信
- D6 (PA04): UART TX → UartMonitor RX
- D7 (PA05): UART RX → UartMonitor TX（将来用）  
- GND/3V3: 電源・グランド → UartMonitor

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

### 必要ライブラリ

```arduino
// Arduino IDE → ライブラリマネージャでインストール
#include <TinyUSB.h>           // TinyUSB MIDI対応
#include <MIDI.h>              // MIDI通信用
#include <ArduinoJson.h>       // JSON出力用（オプション）
```

---

## ソフトウェア実装

### メイン機能構成

```arduino
#include <TinyUSB.h>
#include <MIDI.h>

// TinyUSB MIDI インスタンス作成
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

void setup() {
    // UART通信初期化（UartMonitor用）
    Serial1.begin(115200);
    
    // TinyUSB MIDI初期化
    TinyUSB_Device_Init(0);
    
    // MIDI設定
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandleControlChange(handleControlChange);
    
    Serial1.println("LittleJoe TinyUSB MIDI Monitor Ready");
}

void loop() {
    // TinyUSB処理
    TinyUSB_Device_Task();
    
    // MIDI メッセージ処理
    if (MIDI.read()) {
        // MIDIデータを受信・処理
        processMidiMessage();
    }
    
    delay(1);
}
```

### MIDI モニタリング実装

```arduino
// Note On イベント処理
void handleNoteOn(byte channel, byte note, byte velocity) {
    StaticJsonDocument<200> json;
    json["type"] = "note_on";
    json["channel"] = channel;
    json["note"] = note;
    json["velocity"] = velocity;
    json["timestamp"] = millis();
    
    serializeJson(json, Serial1);
    Serial1.println();
}

// Note Off イベント処理  
void handleNoteOff(byte channel, byte note, byte velocity) {
    StaticJsonDocument<200> json;
    json["type"] = "note_off";
    json["channel"] = channel;
    json["note"] = note;
    json["velocity"] = velocity;
    json["timestamp"] = millis();
    
    serializeJson(json, Serial1);
    Serial1.println();
}

// Control Change イベント処理
void handleControlChange(byte channel, byte cc, byte value) {
    StaticJsonDocument<200> json;
    json["type"] = "control_change";
    json["channel"] = channel;
    json["controller"] = cc;
    json["value"] = value;
    json["timestamp"] = millis();
    
    serializeJson(json, Serial1);
    Serial1.println();
}
```

### USB MIDI パケット解析

```arduino
// USB MIDI パケット構造
struct USBMIDIPacket {
    uint8_t header;     // Cable Number + Code Index
    uint8_t midi[3];    // MIDI data bytes
};

void parseUSBMIDIPacket(const USBMIDIPacket& packet) {
    uint8_t codeIndex = packet.header & 0x0F;
    uint8_t cableNumber = (packet.header >> 4) & 0x0F;
    
    StaticJsonDocument<300> json;
    json["type"] = "usb_midi_packet";
    json["cable"] = cableNumber;
    json["code_index"] = codeIndex;
    
    JsonArray data = json.createNestedArray("data");
    for (int i = 0; i < 3; i++) {
        data.add(packet.midi[i]);
    }
    
    json["timestamp"] = millis();
    serializeJson(json, Serial1);
    Serial1.println();
}
```

---

## デバッグ設定

### picoprobe 接続

**SWD 接続:**
```
LittleJoe D6 (PA04) → picoprobe GP2 (SWCLK)
LittleJoe D7 (PA05) → picoprobe GP3 (SWDIO)
LittleJoe GND       → picoprobe GND
LittleJoe 3V3       → picoprobe 3V3
```

**UART デバッグ:**
```
LittleJoe D10 (PA18) → picoprobe GP4 (UART RX)
LittleJoe GND        → picoprobe GND
```

### OpenOCD 設定

```bash
# GhostPC での OpenOCD 起動
openocd -f interface/cmsis-dap.cfg -f target/at91samdXX.cfg

# 別ターミナルで GDB 接続
arm-none-eabi-gdb firmware/littlejoe/LittleJoe.elf
(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) continue
```

---

## データ出力フォーマット

### JSON 出力例

```json
{
  "type": "note_on",
  "channel": 1,
  "note": 60,
  "velocity": 127,
  "timestamp": 1234567
}

{
  "type": "usb_midi_packet", 
  "cable": 0,
  "code_index": 9,
  "data": [144, 60, 127],
  "timestamp": 1234568
}
```

### ASCII 出力モード（オプション）

```arduino
// シンプルなASCII出力
void outputASCII(byte channel, byte note, byte velocity) {
    Serial.print("NOTE_ON CH:");
    Serial.print(channel);
    Serial.print(" NOTE:");
    Serial.print(note);
    Serial.print(" VEL:");
    Serial.println(velocity);
}
```

---

## パフォーマンス最適化

### メモリ使用量最適化

```arduino
// 静的バッファ使用（動的確保回避）
#define MIDI_BUFFER_SIZE 64
uint8_t midiBuffer[MIDI_BUFFER_SIZE];
uint8_t bufferIndex = 0;

// JSON出力最適化
void optimizedOutput(const char* type, byte data1, byte data2, byte data3) {
    Serial.print("{\"type\":\"");
    Serial.print(type);
    Serial.print("\",\"data\":[");
    Serial.print(data1);
    Serial.print(",");
    Serial.print(data2);
    Serial.print(",");
    Serial.print(data3);
    Serial.print("],\"ts\":");
    Serial.print(millis());
    Serial.println("}");
}
```

### 処理速度最適化

```arduino
// 高速処理用設定
void optimizePerformance() {
    // CPU クロック最適化（48MHz固定）
    while (GCLK->STATUS.bit.SYNCBUSY);
    
    // USB転送最適化
    USBDevice.setSpeed(USB_SPEED_FULL);
    
    // シリアル出力バッファ最適化
    Serial.setTimeout(1);
}
```

---

## 実装フェーズ

### Phase 1: 基本機能
- [x] Arduino IDE環境構築
- [x] XIAO SAMD21 動作確認
- [ ] 基本MIDI受信機能
- [ ] picoprobe SWD接続

### Phase 2: モニタリング機能
- [ ] リアルタイムMIDI解析
- [ ] JSON形式出力実装
- [ ] USB MIDI パケット解析

### Phase 3: デバッグ統合
- [ ] OpenOCD デバッグ環境
- [ ] リアルタイムログ出力
- [ ] パフォーマンス監視

---

## トラブルシューティング

### よくある問題

1. **Arduino IDE でボードが認識されない**
   ```bash
   # ブートローダーモード進入
   XIAO の RST ボタンを素早く2回押す
   ```

2. **MIDI データが受信できない**
   - USB ケーブルの品質確認
   - Mihashi との接続確認
   - MIDI ライブラリ設定確認

3. **SWD デバッグが接続できない**
   - picoprobe ファームウェア確認
   - 配線チェック（特にGND接続）
   - OpenOCD バージョン確認

### デバッグコマンド

```arduino
// デバッグ情報出力
void debugInfo() {
    Serial.println("=== LittleJoe Debug Info ===");
    Serial.print("Free RAM: ");
    Serial.println(freeMemory());
    Serial.print("CPU Freq: ");
    Serial.println(SystemCoreClock);
    Serial.print("USB Status: ");
    Serial.println(USBDevice.connected() ? "Connected" : "Disconnected");
}
```

---

## 拡張機能

### MIDI ルーティング
- 特定チャンネルのフィルタリング
- MIDI エフェクト処理
- 複数出力対応

### 表示機能
- 小型OLED ディスプレイ追加
- LED インジケータ
- ボタン操作対応

### ログ機能
- SD カードログ記録
- WiFi経由でのデータ送信
- リアルタイム波形表示

---

**最終更新**: 2025-06-24  
**対象ハードウェア**: Seeed XIAO SAMD21  
**開発環境**: Arduino IDE 2.x + TinyUSB MIDI  
**出力先**: UartMonitor (XIAO SAMD21)