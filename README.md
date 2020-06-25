# irBoard Library for ESP32

irBoard Library for ESP32はiOSアプリirBoardからESP32デバイスに接続し、デバイスの操作やモニター、設定などができるようにするためのArduino IDE向けのライブラリーです。

irBoardはiPodやiPhoneを[PLC(Programable logic controller)](https://ja.wikipedia.org/wiki/プログラマブルロジックコントローラ)向けの、タッチパネルディスプレイとして利用できるiOSアプリケーションです。

[irBoard](https://irboard.itosoft.com)

このライブラリーを使うことでiPadやiPhoneからデバイスの操作やモニター、設定などができる様になります。

## チュートリアル

ライブラリーのインポートやirBoardと接続するまでをQiitの記事に書いていますのでご覧ください。

https://qiita.com/katsuyoshi/items/5c850cd14e8e2ff47b4f


## 使い方

1. irboar.hをインクルードします
2. Irboardのインスタンスを生成します
3. addAP()で接続するWiFiのssidとpasswordを設定します。  
  WiFiMultiを利用してますのでaddAP()を繰り返し複数のWiFiを登録できます。
4. setVerbose()にtrueを渡すと接続時にTerminalに割り当てられたIPアドレスを出力します。
5. begin()を呼び出しirBoardとの接続を開始します。


```
#include <irboard.h>                // 1

Irboard irboard = Irboard();        // 2

void setup() {
    .
    .
    irboard.addAP(ssid, password);  // 3
    irboard.setVerbose(true);       // 4
    irboard.begin();                // 5
}
```

irBoardはPLCと接続します。
Ruby gem Ladder Driveを用いると、PCやRaspberry PIなどを仮想的なPLCとして扱える様になります。

[GitHub - ito-soft-design/ladder_drive](https://github.com/ito-soft-design/ladder_drive)

irBoard Library for ESP32ではESP32デバイスがLadder Driveの仮想的なPLCとみなせる様に作っています。

このライブラリーで使用できるデバイスは次の通りです。

|デバイス範囲|型|用途|
|:--|:-:|:--|
|X00 - X31|bool|入力デバイス|
|Y00 - Y31|bool|出力デバイス|
|M00 - M31|bool|補助コイル|
|H00 - H31|bool|保持コイル。本来は電源が切れても記憶するのですが記憶されません|
|D00 - D127|short|データメモリ。本来は電源が切れても記憶するのですが記憶されません|
|SD00 - SD32|short|irBoardとのインタラクションに使用されます|

デバイスのサイズば[irboard.h](https://github.com/ito-soft-design/irboard_esp32/blob/master/src/irboard.h#L36)で定義していますのでここを変えれば変更できます。


この様なデバイスが実際にあるわけではなく、イメージとしてはデバイス名をキーとしたHashがあってそこに読み書きしている様な感じです。

値を読み込む場合はデバイスの型に合わせてboolValue()やshortValue()にデバイス名を指定して呼び出します。(6.)

書き込みの場合はsetBoolValue(,)やsetShortValue(,)を使います。(7.)

```
bool x0;
short d0;

void loop() {
    irboard.update();
    if (irboard.isChanged()) {
        x0 = irboard.boolValue("X0");           // 6
        d0 = irboard.shortValue("D0");          // 6

        // LCDの更新などします
    }
    irboard.setBoolValue("Y0", M5.Btn.isPressed()); // 7
    short temprature = getTemperatur(); // 温度取得関数があるとして
    irboard.setShortValue("D1", temprature);        // 7
}

```


### ライセンス

MIT

### TODO

- float値の扱い
- textの扱い
- M5Stackのスケッチ例追加
