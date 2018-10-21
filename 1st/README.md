# 先端画像処理・ロボティクス特論 第一回 課題

### 学番・氏名
AL15069 小野義基

### 共同作業者
- AF15043 坂内力
- AF15045 佐藤康平

## 元画像と出力画像 (一例)
- 元画像: ![img](./src/img.bmp)
- グレースケール画像: ![img2](./dst/img_gray.bmp)
- 2値化画像: ![img3](./dst/img_binarization.bmp)


## ソースコード等フォルダの構成
```
ソースコード等/
　├ 1st.cpp    `グレースケール、ヒストグラム、2値化画像などの処理`
　├ bitmap_manager.cpp    `bmp画像の読み書きなどを管理するクラス`
　├ bitmap_manager.hpp    `bitmap_manager用のヘッダ`
　│
　├ src/
　│　├ img.bmp    `元画像1`
　│　├ img2.bmp    `元画像2`
　│　└ img3.bmp    `元画像3`
　│
　├ dst/
　│　├ img_gray.bmp    `グレースケール画像`
　│　├ img_binarization.bmp    `2値化画像`
　│　... img2, img3も同様
　│
　├ histgram/
　│　└ img_hist.pdf    `ヒストグラム`
　│
　└ Makefile    `Makeファイル`
```

## 使い方

### 準備
- srcフォルダにbmpファイルをおいてください。

### コンパイル方法
``` sh
make
./1st bitmap_filename
```
`bitmap_filename` は `src` ディレクトリに置いた画像の名前です

ex) `img`, `img2`, `img3`

### 出力
- `dst/` -> 処理画像
- コンソール(`gnuplot`) -> グラフ
    - gnuplotからPDF出力をしたグラフを `histgram` ディレクトリへおいてあります。

### 注意
- グラフは `gnuplot` で生成しています。
- トップダウン方式のbmpファイルは読み込めません。
