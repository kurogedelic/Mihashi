# Mihashi 技術仕様書（RP2350A MIDI USB Host）

## 概要

**Mihashi** は Waveshare RP2350A USB PIO HOST を用いた **双方向 MIDI USB ホストブリッジ** 装置である。USB MIDI デバイスの接続・制御を行い、そのデータを LittleJoe（開発用モニター）に転送する。

---

## ハードウェア仕様

### 基板: Waveshare RP2350A USB PIO HOST

- **CPU**: デュアルコア Arm Cortex-M33 @ 150MHz
- **メモリ**: 520KB SRAM、2MB Flash
- **PIO**: 12個のプログラマブルI/Oステートマシン（3ブロック）
- **USB Host**: Type-A ポート（GPIO12/13使用）
- **プログラミング**: Type-C ポート
- **電源**: 5V 1A以上推奨

### 接続構成

```
[GhostPC] ←USB MIDI→ [Mihashi] ←USB A/C→ [LittleJoe]
```

**GPIO割り当て**:
- GPIO12/13: USB D+/D-（22Ω直列抵抗付き）
- GPIO4: UART TX（LittleJoe通信用・予備）
- SWDピン: デバッグ用（必要時）

---

## ソフトウェア仕様

### 開発環境

- **SDK**: Pico SDK + Pico-PIO-USB
- **言語**: C/C++
- **ビルドシステム**: CMake
- **デバッガ**: OpenOCD + gdb-multiarch

### アーキテクチャ

#### 3層アーキテクチャ
1. **USB Host Layer**: PIOベースUSB Host実装
2. **MIDI Processing Layer**: USB MIDI ↔ 内部形式変換
3. **Output Layer**: LittleJoeへのデータ転送

#### 処理フロー
```c
// メイン処理ループ
void midi_bridge_process() {
    // USB MIDI受信処理
    uint8_t usb_packet[4];
    while (tuh_midi_packet_read(dev_addr, usb_packet) > 0) {
        process_usb_midi_packet(usb_packet);
        forward_to_littlejoe(usb_packet);
    }
    
    // USB Hostタスク維持
    tuh_task();
}
```

### USB MIDI実装詳細

#### USB MIDIパケット構造
```c
typedef struct {
    uint8_t header;     // Cable Number (4bit) + Code Index (4bit)
    uint8_t midi_0;     // MIDI status/data byte
    uint8_t midi_1;     // MIDI data byte 1
    uint8_t midi_2;     // MIDI data byte 2
} usb_midi_packet_t;
```

#### PIO USB Host設定
- **周波数要件**: 120MHz または 240MHz（120MHzの倍数）
- **ステートマシン**: 3個使用（TX×1、RX×2）
- **リソース**: 約15KB ROM/RAM消費
- **対応速度**: Low-Speed (1.5Mbps)、Full-Speed (12Mbps)

---

## 通信プロトコル

### LittleJoeとの通信

**接続方式**: USB A（Mihashi）→ USB C（LittleJoe）

**データ形式**: 
- USB MIDI パケットをそのまま転送
- LittleJoe側でMIDI解析・モニタリング

**転送レート**: USB 2.0 Full-Speed (12Mbps)

---

## パフォーマンス仕様

### レイテンシ目標
- **USB → LittleJoe**: < 1ms
- **MIDI処理**: < 100μs/パケット
- **SysExサポート**: 最大64KB/メッセージ

### リソース使用量
- **RAM**: < 100KB（バッファ含む）
- **Flash**: < 200KB
- **CPU使用率**: < 30%（通常動作時）

---

## エラーハンドリング

### USB接続エラー
```c
// USB デバイス接続監視
void tuh_mount_cb(uint8_t dev_addr) {
    printf("MIDI Device connected: %d\n", dev_addr);
    midi_device_addr = dev_addr;
}

void tuh_umount_cb(uint8_t dev_addr) {
    printf("MIDI Device disconnected: %d\n", dev_addr);
    midi_device_addr = 0;
}
```

### 復旧メカニズム
1. **ハードウェアWatchdog**: 1秒タイムアウト
2. **USB再接続**: 自動検出・再初期化
3. **バッファオーバーフロー**: 循環バッファで対応

---

## 開発・デバッグ

### ビルド手順
```bash
cd firmware/mihashi
mkdir build && cd build
cmake ..
make -j4
```

### デバッグ接続
- **プログラミング**: Type-C → GhostPC
- **SWDデバッグ**: 必要時にpicoprobe接続可能
- **シリアル出力**: UART経由でログ出力

---

## 実装フェーズ

### Phase 1: 基本機能
- [x] USB Host初期化
- [x] MIDI デバイス認識
- [ ] 基本MIDI転送機能

### Phase 2: 最適化
- [ ] レイテンシ最適化
- [ ] エラーハンドリング強化
- [ ] SysEx完全サポート

### Phase 3: 自律開発統合
- [ ] Claude Code連携
- [ ] 自動テスト機能
- [ ] パフォーマンス監視

---

## 設定パラメータ

### システム設定
```c
#define MIDI_BRIDGE_VERSION "1.0.0"
#define USB_MIDI_BUFFER_SIZE 1024
#define CPU_FREQ_HZ 240000000  // 240MHz固定
#define WATCHDOG_TIMEOUT_MS 1000
```

### USB設定
```c
#define USB_HOST_GPIO_DP 12
#define USB_HOST_GPIO_DM 13
#define USB_MIDI_CLASS 0x01
#define USB_MIDI_SUBCLASS 0x03
```

---

## トラブルシューティング

### よくある問題

1. **USB認識しない**
   - 電源供給確認（5V 1A以上）
   - ケーブル品質確認
   - GPIO12/13接続確認

2. **レイテンシが高い**
   - CPU周波数確認（240MHz設定）
   - USB転送頻度調整
   - バッファサイズ最適化

3. **デバイス切断が多い**
   - 電源ノイズ確認
   - USB Host実装確認
   - Watchdog設定調整

---

## 今後の拡張計画

### ハードウェア拡張
- 複数USB MIDI デバイス対応
- MIDI DIN端子追加検討
- 電源管理機能強化

### ソフトウェア拡張  
- 双方向MIDI対応（LittleJoe → Mihashi）
- MIDIルーティング機能
- Web設定インターフェース

---

**最終更新**: 2025-06-24  
**対象ハードウェア**: Waveshare RP2350A USB PIO HOST  
**対象ファームウェア**: Mihashi v1.0.0