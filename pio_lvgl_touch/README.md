# pio_lvgl_touch

## What This Project Does
- ESP32-P4 + JC1060P470 を対象に、LVGL と TouchLib (GT911) を組み合わせたタッチ UI デモです。
- 3×3 のグリッドボタンを表示し、タップしたボタン番号を画面下部に `Button N pressed` として表示します。
- PSRAM 上にダブルバッファを確保し、バックライト GPIO23 を制御する構成は `pio_lvgl2` から継承しています。

## How It Was Built
- `pio_lvgl2` を複製してベースを作成し、タッチ周りを `touch_helper.*` として再構成しました。
- TouchLib の GT911 実装を利用し、生データ範囲 (X:0-800, Y:0-500) を画面解像度へマッピングしています。
- LVGL のグリッドレイアウトを使って 3×3 ボタンを生成し、ユーザーデータでボタン番号をイベントハンドラへ渡します。

## Tricky Points / Notes
- Arduino_GFX は標準状態では ESP32-P4 の RGB パネルに未対応なので、`patches/arduino_gfx_p4.patch` をライブラリ展開後に適用する必要があります（ライブラリ再取得時は再適用してください）。
- GT911 の生データは表示座標と一致しないため、`touch_helper.cpp` で範囲補正を入れないとタップ位置が右下へずれます。
- LVGL ボタンのイベントは環境によって `LV_EVENT_SHORT_CLICKED` になることがあるため、`button_event_cb` では `CLICKED` と `SHORT_CLICKED` を両方処理しています。

## How To Run
リポジトリルート (`/Users/ring2/Documents/src/cyd7`) で以下をまとめて実行します。

```bash
pio pkg install -d pio_lvgl_touch && \
  (cd pio_lvgl_touch/.pio/libdeps/esp32-p4-evboard/GFX\ Library\ for\ Arduino && patch -p1 < ../../../../patches/arduino_gfx_p4.patch) && \
  pio run -d pio_lvgl_touch -t upload && \
  pio device monitor -d pio_lvgl_touch
```

- すでにライブラリが展開済みでパッチ適用も済んでいる場合は、先頭の 2 つの処理を省略しても構いません。
- 実行後、モニタに起動ログが出力され、ディスプレイ上でボタンをタップすると下部メッセージが更新されます。
