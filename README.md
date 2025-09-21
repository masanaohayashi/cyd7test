# JC1060P470 Arduino GFX デモ

## プロジェクト概要
- ESP32-P4 を搭載した Guition JC1060P470 LCD デバイスを Arduino 環境で動作検証するためのサンプルです。
- [こちらのブログ記事](https://gijin77.blog.jp/archives/43856982.html) で公開されている Arduino スケッチを元に、PlatformIO でもビルド・実行できるよう移植しています。
- 同作者によるタッチ対応の解説（[ESP32-P4 JC1060P470 タッチパネル調査](https://gijin77.blog.jp/archives/43861539.html)）では、I²C 接続の GT911 コントローラを利用してタッチ座標を取得する方法が紹介されています。タッチを有効化したい場合は同記事を参考にしてください。
- [Arduino_GFX_Library](https://github.com/moononournation/Arduino_GFX) を利用し、DSI 接続パネルを初期化してベンチマークおよびロゴ描画を行います。
- 現在は Arduino IDE で動作確認済みですが、VS Code での開発に向けたセットアップ手順もまとめています。

## ハードウェア前提
- Guition JC1060P470（10.1 インチ / 1024×600 DSI パネル、バックライト PIN: GPIO23）
- ESP32-P4（または ESP32-C6 モジュールを含む JC1060P470 ボード付属 MCU）
- USB-C ケーブル（デバッグ・書き込み用）
- オンボード PSRAM（フレームバッファや LVGL バッファ用として活用可能）

## ソフトウェア要件
- Arduino IDE 2.x もしくは Arduino CLI 0.35 以上
- Espressif Systems ESP32 ボードパッケージ（`esp32` 3.x 系を推奨。P4 サポートが含まれるバージョンを使用してください）
- ライブラリ
  - Arduino_GFX_Library 1.4.11 以上（DSI パネル対応版）
  - 依存ライブラリは Arduino IDE / CLI が自動で解決します

## リポジトリ構成
- `README.md` : プロジェクト概要とセットアップ手順
- `pio_p4_test/platformio.ini` : PlatformIO 設定（ESP32-P4 ボード定義・ビルドフラグなど）
- `pio_p4_test/src/main.cpp` : Arduino_GFX を用いたメインスケッチ
- `pio_p4_test/include/img_logo.h` : 480×320 の RGB565 ロゴデータ
- `pio_p4_test/patches/arduino_gfx_p4.patch` : GFX ライブラリ用 ESP32-P4 パッチ
- `pio_lvgl/` : LVGL + Arduino_GFX を用いた PlatformIO プロジェクト（画面描画のみ）

## Arduino IDE でのビルド手順
1. Arduino IDE を起動し、「ボードマネージャー」で Espressif の `esp32` パッケージをインストール（P4 対応版を選択）。
2. 「ライブラリマネージャー」で `Arduino GFX Library` をインストールまたは最新化。
3. IDE で `JC1060P470_Arduino_GFX_Demo/JC1060P470_Arduino_GFX_Demo.ino` を開く。
4. 「ツール > ボード」で JC1060P470 に搭載された ESP32-P4 ボード定義を選択。
5. USB 接続後、「シリアルポート」を該当デバイスに設定し、書き込み実行。

## VS Code でのセットアップ
### 1. 必要な拡張機能
- `Arduino` (by Microsoft)
- `C/C++` (by Microsoft)

### 2. 環境設定
1. Arduino IDE または Arduino CLI をインストールし、`arduino-cli config init` で基本設定を行います。
2. VS Code の設定 (`settings.json`) で `"arduino.path"` または `"arduino.commandPath"` に Arduino CLI / IDE のパスを指定。
3. コマンドパレットから `Arduino: Initialize` を実行し、ワークスペースルートに `.vscode/arduino.json` を生成します。
   ```json
   {
     "sketch": "JC1060P470_Arduino_GFX_Demo/JC1060P470_Arduino_GFX_Demo.ino",
     "board": "esp32:esp32:esp32p4devkit",        // 利用するボード名に置き換えてください
     "configuration": "PartitionScheme=default"
   }
   ```
   *ボード ID はインストール済みの ESP32 ボードパッケージに合わせて変更してください。
4. 同じく `.vscode/c_cpp_properties.json` を生成し、`includePath` に Arduino コアとライブラリが追加されていることを確認。

### 3. ビルド & 書き込み
- ステータスバー左下のボード選択から ESP32-P4 を選び、ポートも設定します。
- `Arduino: Verify` でコンパイル、`Arduino: Upload` で書き込み。
- シリアルモニタは `Arduino: Open Serial Monitor` から利用できます（115200bps 推奨）。
- Upload Mode を `USB-OTG CDC (TinyUSB)` に設定すると高速な書き込みが可能です。

### PlatformIO を使う場合
- 現時点では ESP32-P4 のサポートが未整備の可能性があります。PlatformIO にボード定義が追加されるまでは Arduino 拡張を用いる構成が安全です。

### PlatformIO（pio_p4_test）での動作手順
1. **PlatformIO CLI を用意**（未インストールなら `pip install platformio` などで導入）。
2. **依存パッケージを取得**
   ```bash
   cd <このリポジトリのクローン先>
   pio pkg install -d pio_p4_test
   ```
3. **Arduino_GFX へのパッチ適用**（ESP32-P4 対応の欠落を補います）
   ```bash
   cd pio_p4_test/.pio/libdeps/esp32-p4-evboard/GFX\ Library\ for\ Arduino
   patch -p1 < ../../patches/arduino_gfx_p4.patch
   ```
4. **ビルド**（リポジトリルートに戻って実行）
   ```bash
   cd ../../../../../
   pio run -d pio_p4_test
   ```
5. **書き込み & シリアルモニタ**
   ```bash
   pio run -d pio_p4_test -t upload
   pio device monitor -d pio_p4_test
   ```
   *`monitor_port` は `platformio.ini` で `/dev/cu.usbserial-110` に設定済み。別ポートの場合は適宜変更してください。*
   まとめて実行したい場合は以下のようにすると便利です。
   ```bash
   pio run -d pio_p4_test -t upload && pio device monitor -d pio_p4_test
   ```

> **注意**: `pio_p4_test/.pio/libdeps/...` を削除・再生成するとライブラリが元に戻るため、再度パッチを適用してください。パッチファイルは `pio_p4_test/patches/arduino_gfx_p4.patch` にあります。

### PlatformIO（pio_lvgl）で LVGL サンプルを動かす
`pio_lvgl` でも同様に Arduino_GFX のパッチ適用が必要です。

1. 依存を取得
   ```bash
   cd <このリポジトリのクローン先>
   pio pkg install -d pio_lvgl
   ```
2. Arduino_GFX へのパッチ適用
   ```bash
   cd pio_lvgl/.pio/libdeps/esp32-p4-evboard/GFX\ Library\ for\ Arduino
   patch -p1 < ../../../../patches/arduino_gfx_p4.patch
   ```
3. ビルド → 書き込み → モニタ
   ```bash
   cd ../../../../../
   pio run -d pio_lvgl -t upload && pio device monitor -d pio_lvgl
   ```
   *`pio_lvgl` のサンプルは LVGL 8 系ライブラリで稼働し、PSRAM 上のダブルバッファを使用します。*

## トラブルシュート
- **画面が真っ黒**: バックライト PIN (`GFX_BL = 23`) が `HIGH` で駆動されているか確認。
- **ビルド失敗 (DSI 関連)**: Arduino_GFX_Library を最新化し、`jd9165_init_operations` など DSI 初期化コマンドがライブラリに含まれているか確認。
- **ポートが表示されない**: ドライバ (CP210x / CH9102 など) を OS に導入。

## 今後の TODO
- VS Code 用のタスク (`tasks.json`) 追加で書き込みの自動化
- PlatformIO への移行検証（ボードマップ追加）
- ベンチマーク結果の記録と比較
