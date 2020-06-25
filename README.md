# irBoard Library for ESP32

irBoard Library for ESP32はiOSアプリirBoardからESP32デバイスに接続し、デバイスの操作やモニター、設定などができるようにするためのArduino IDE向けのライブラリーです。

irBoardはiPodやiPhoneを[PLC(Programable logic controller)](https://ja.wikipedia.org/wiki/プログラマブルロジックコントローラ)向けの、タッチパネルディスプレイとして利用できるiOSアプリケーションです。

[irBoard](https://irboard.itosoft.com)

このライブラリーを使うことでiPadやiPhoneからデバイスの操作やモニター、設定などができる様になります。

## チュートリアル

ライブラリーのインポートやirBoardと接続するまでをQiitの記事に書いていますのでご覧ください。

[irBoardからM5StickCなどに接続できるライブラリーを作りました - Qiita](https://qiita.com/katsuyoshi/items/5c850cd14e8e2ff47b4f)

## 使い方

1. irboar.hをインクルードします
2. Irboardのインスタンスを生成します
3. addAP()で接続するWiFiのssidとpasswordを設定します。  
  WiFiMultiを利用してますのでaddAP()を繰り返し複数のアクセスポイントを登録できます。
4. setVerbose()にtrueを渡すと接続時に割り当てられたIPアドレスをTerminalに出力します。
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

irBoardはPLCと接続し操作やモニターなどができるようになっています。
PLCの代わりにPCやRaspberry PIなどを仮想的なPLCとして扱えるRuby gem Ladder Driveがあり、irBoardはこれとも接続できる様になっています。

[GitHub - ito-soft-design/ladder_drive](https://github.com/ito-soft-design/ladder_drive)

irBoard Library for ESP32ではLadder Driveの仮想的なPLCのデバイスを持っているとみなせる様に作っています。

このライブラリーで扱えるPLCデバイスは次の通りです。

|デバイス(範囲)|型|用途|
|:--|:-:|:--|
|X00 - X31|bool|スイッチなどの入力デバイス|
|Y00 - Y31|bool|ランプなどの出力デバイス|
|M00 - M31|bool|補助コイル|
|H00 - H31|bool|保持コイル。本来は電源が切れても記憶するのですが記憶されません|
|D00 - D127|short|データメモリ|
|SD00 - SD32|short|irBoardとのインタラクションに使用されます|

デバイスのサイズば[irboard.h](https://github.com/ito-soft-design/irboard_esp32/blob/master/src/irboard.h#L36)で定義していますのでここを変えれば変更できます。

PLCではないので、この様なデバイスが実際にあるわけではなく、イメージとしてはデバイス名をキーとしたHashがあってそこに読み書きしている様な感じです。

6. update()でirBoardとの送受信処理を行いデバイスの値を更新します。
7. isChanged()で値に変化があったか確認でき、変化がある時に画面などを更新する様にできます。
8. 値を読み込む場合はデバイスの型に合わせてboolValue()やshortValue()にデバイス名を指定して呼び出します。
9. 書き込みの場合はsetBoolValue(,)やsetShortValue(,)を使います。

```
bool x0;
short d0;

void loop() {
    irboard.update();                       // 6
    if (irboard.isChanged()) {              // 7
        x0 = irboard.boolValue("X0");           // 8
        d0 = irboard.shortValue("D0");          // 8

        // LCDの更新などします
    }
    short temprature = getTemperatur(); // 温度取得関数があるとして
    irboard.setShortValue("D1", temprature);        // 9
    irboard.setBoolValue("Y0", M5.Btn.isPressed()); // 9
}

```

## irBoardについて

irBoardについては公式サイトやドキュメンをご覧ください。

[irBoard](https://irboard.itosoft.com)

[ドキュメント](https://irboard.itosoft.com/document/ja/document.html)


### TODO

- float値の扱い
- textの扱い
- M5Stackのスケッチ例追加

### ライセンス

[MIT](https://github.com/ito-soft-design/irboard_esp32/blob/master/LICENSE.txt)
