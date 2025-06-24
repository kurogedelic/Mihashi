# LittleJoe 通信規格 (XIAO RP2040 UART Converter)

## 概要

LittleJoe (微型XIAO RP2040)はUSB MIDIデバイスを受け取り、MIDIパケットをASCIIテキストとしてUART端末に出力する。
これは主計算側の"MidiBridgePico2"が受け取り、解析、再構成するための不透明化チャネルとして活用される。

---

## 通信方式

### インターフェース

* **UART**
* 通信速度: `115200 baud`
* パラリティ: 8N1 (8bit, No parity, 1 stop bit)

### データ形式

* 1 ラインに1パケット
* ASCII 16進数の2文字表現
* 3 バイト または 4 バイト (SysExなど) に対応
* 区切りはスペース or `\n`

### 例:

```
90 3C 7F
80 3C 00
F0 7E 7F 09 F7
```

---

## 文字列フォーマット

* ASCIIテキストにより、単箇所で直接人間が読める
* パケット内容は、MIDIバイトを表す数値の群 (0x00〜0xFF)
* 1行が1パケットに相当

---

## ストリームの概要 (LittleJoe側)

1. USB MIDIで受信 (TinyUSB + MIDI class)
2. USB MIDIパケットを入力
3. UART端末に、"XX XX XX"\n として出力
4. (SysExはバッファに続けて表示)

---

## MidiBridgePico2側の読み取り

* UARTからの1行を読み込み
* ASCIIを解析し、MIDIバイト列に変換
* 正しいMIDIパケットとして再構成
* SysExなどの複雑なメッセージはストリームでの分割に注意

---

## 拡張性

* `LittleJoe` 側でのUSB MIDI IN/OUTの対応
* ASCIIでなくbinary UARTでの出力に切り替えも可
* MIDIプロトコルを用いた再生テスト用っぽくもできる

---

## 表示名称

* **LittleJoe** = UART MIDI Text Converter (先端のXIAO)
* **MidiBridgePico2** = RP2350A USB MIDI Host側本体

設置完了後は、ケーブル1本で「USB MIDI → UART ASCII」のインターフェースが成立する。