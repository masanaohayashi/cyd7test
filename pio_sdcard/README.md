# pio_sdcard

## Overview
- ESP32-P4 + JC1060P470 の TF スロットを使い、SD_MMC (microSD) への書き込み・読み出し・削除をテストするサンプルです。
- LVGL 上に 3 つのボタンとログ用ラベルを配置し、それぞれ `Write File` / `Read File` / `Delete File` の操作をトリガーします。
- タッチ入力は TouchLib (GT911) を利用しており、ボタン操作結果は画面下部のメッセージに英語で表示されます。

## How To Run
リポジトリルート (`/Users/ring2/Documents/src/cyd7`) で以下を実行してください。

```bash
pio pkg install -d pio_sdcard && \
  (cd pio_sdcard/.pio/libdeps/esp32-p4-evboard/GFX\ Library\ for\ Arduino && patch -p1 < ../../../../patches/arduino_gfx_p4.patch) && \
  pio run -d pio_sdcard -t upload && \
  pio device monitor -d pio_sdcard
```

- 既に依存ライブラリ展開・パッチ適用済みの場合は、該当部分を省略してもかまいません。
- シリアルモニタは 115200bps で初期化しています。

## UI Behavior
1. **Write File**: `/lvgl_sd_demo.txt` に `Hello from SD demo at <millis>` を追記し、結果を画面とシリアルに表示。
2. **Read File**: ファイル内容を読み込み、最大 512 文字を画面メッセージとして表示。
3. **Delete File**: ファイルが存在すれば削除し、結果をメッセージ更新。

## Notes
- microSD は FAT32 でフォーマットし、TF スロットへ挿入してください。
- `SD_MMC.begin()` は 1-bit モードで初期化しています。マウントに失敗するとメッセージが `SD mount failed` になります。
- TouchLib のキャリブレーションは `touch_helper.*` に実装済みです。タップ位置がずれる場合は補正値を調整してください。
