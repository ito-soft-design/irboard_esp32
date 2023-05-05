# irBoard Library for ESP32

irBoard Library for ESP32はiOSアプリirBoardからESP32デバイスに接続し、デバイスの操作やモニター、設定などができるようにするためのArduino IDE向けのライブラリーです。

irBoardはiPodやiPhoneを[PLC(Programable logic controller)](https://ja.wikipedia.org/wiki/プログラマブルロジックコントローラ)向けの、タッチパネルディスプレイとして利用できるiOSアプリケーションです。

[irBoard](https://irboard.itosoft.com)

このライブラリーを使うことでiPadやiPhoneからデバイスの操作やモニター、設定などができる様になります。

## チュートリアル

ライブラリーのインポートやirBoardと接続するまでをQiitの記事に書いていますのでご覧ください。

[irBoardからM5StickCなどに接続できるライブラリーを作りました - Qiita](https://qiita.com/katsuyoshi/items/5c850cd14e8e2ff47b4f)

## 使い方

1. irboard.hをインクルードします
2. Irboardのインスタンスを生成します
3. addAP()で接続するWiFiのssidとpasswordを設定します。  
  WiFiMultiを利用してますのでaddAP()を繰り返し複数のアクセスポイントを登録できます。
4. setVerbose()にtrueを渡すと接続時に割り当てられたIPアドレスをTerminalに出力します。
5. begin()を呼び出しirBoardとの接続を開始します。
6. setShortValue(,)を呼び出しSD19を1にしirBoardからの書き込みを許可します。


```
#include <irboard.h>                    // 1

Irboard irboard = Irboard();            // 2

void setup() {
    .
    .
    irboard.addAP(ssid, password);      // 3
    irboard.setVerbose(true);           // 4
    irboard.begin();                    // 5
    irboard.setShortValue("SD19", 1);   // 6
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
|D00 - D127|short|データメモリ(16bit)|
|SD00 - SD32|short|irBoardとのインタラクションに使用されます|

デバイスのサイズば[irboard.h](https://github.com/ito-soft-design/irboard_esp32/blob/master/src/irboard.h#L37)で定義しています。  
コンパイルオプションで再定義するか直接このファイル変えることで変更できます。

PLCではないので、この様なデバイスが実際にあるわけではなく、イメージとしてはデバイス名をキーとしたHashがあってそこに読み書きしている様な感じです。

7. update()でirBoardとの送受信処理を行いデバイスの値を更新します。
8. isChanged()で値に変化があったか確認でき、変化がある時に画面などを更新する様にできます。
9. 値を読み込む場合はデバイスの型に合わせてboolValue()やshortValue()、intValue()、floatValue()にデバイス名を指定して呼び出します。intValueとfloatValueは2ワード分使用します。D0を指定した場合はD0と次のD1を繋げて32bitとして扱います。
10. 書き込みの場合はsetBoolValue(,)やsetShortValue(,)、setIntValue(,)、setFloatValue(,)を使います。

```
bool x0;
short d0;

void loop() {
    irboard.update();                           // 7
    if (irboard.isChanged()) {                  // 8
        x0 = irboard.boolValue("X0");           // 9
        d0 = irboard.shortValue("D0");          // 9

        // LCDの更新などします
    }
    short temprature = getTemperatur(); // 温度取得関数があるとして
    irboard.setShortValue("D1", temprature);        // 10
    irboard.setBoolValue("Y0", M5.Btn.isPressed()); // 10
}

```

## irBoardについて

irBoardについては公式サイトやドキュメンをご覧ください。

[irBoard](https://irboard.itosoft.com)

[ドキュメント](https://irboard.itosoft.com/document/ja/document.html)


### TODO

- textの扱い

### ライセンス

[MIT](https://github.com/ito-soft-design/irboard_esp32/blob/master/LICENSE.txt)
