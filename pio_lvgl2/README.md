# pio_lvgl2

## 背景
- `pio_p4_test` で Arduino_GFX を使った DSI デモが動作することを確認。
- 同じ環境に LVGL (v8 系) を組み合わせるサンプルとして `pio_lvgl2` を追加。
- PSRAM にダブルバッファを確保し、最小限の UI（ラベル・ボタン・スライダー）を描画。

## セットアップ手順
リポジトリルート（`/Users/ring2/Documents/src/cyd7`）で以下を実行してください。

```bash
# 依存ライブラリの展開
pio pkg install -d pio_lvgl2

# Arduino_GFX に ESP32-P4 用パッチを適用
cd pio_lvgl2/.pio/libdeps/esp32-p4-evboard/GFX\ Library\ for\ Arduino
patch -p1 < ../../../../patches/arduino_gfx_p4.patch

# ルートに戻り、ビルド→書き込み→モニタ
cd ../../../../../
pio run -d pio_lvgl2 -t upload && pio device monitor -d pio_lvgl2
```

## 期待される挙動
- 起動後、LVGL ベースのラベル・ボタン・スライダが表示される。
- バックライト GPIO23 をオンにし、PSRAM 上のダブルバッファで描画する構成。
