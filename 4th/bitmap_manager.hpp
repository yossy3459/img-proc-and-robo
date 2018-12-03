#ifndef BITMAP_MANAGER_HPP
#define BITMAP_MANAGER_HPP

#include <iostream>
#include <cstring>
#include <cmath>
#include <vector>

//! @def BMPのファイルヘッダー、情報ヘッダーのサイズ
#define FILE_HEADER_SIZE 14
#define INFO_HEADER_SIZE 40

/**
 * @brief ファイルヘッダー定義
 */
typedef struct FileHeader {
    uint8_t origData[FILE_HEADER_SIZE];
    std::string type;  // タイプ
    int size;  // サイズ
} FileHeader;

/**
 * @brief 情報ヘッダー定義
 */
typedef struct InfoHeader {
    uint8_t origData[INFO_HEADER_SIZE];
    int infoHeaderSize;  // ヘッダーサイズ
    int width;  // 幅
    int height;  // 高さ
    int colorParPixel;  // 1ピクセルあたりの色数
    int dataSize;  // サイズ
} InfoHeader;

/**
 * @brief カラー構造体定義
 */
typedef struct Color {
    int r;
    int g;
    int b;
} Color;

/**
 * @brief 画素の位置を定義
 */
typedef struct ColorPosition {
    int r;
    int g;
    int b;
} ColorPosition;

int bit2Integer(uint8_t, uint8_t, uint8_t, uint8_t);

/**
 * @brief ビットマップ処理クラス
 * @details 各ヘッダーと画素データをまとめたクラス。ファイルの読み書き、情報確認、画素への読み書きを行う
 */
class BitmapManager {
    // フィールド定義
    FILE *file; 
    uint8_t *image;
    FileHeader fileHeader;
    InfoHeader infoHeader;

    // 画像形式がトップダウンかどうか
    bool is_topdown = false;

public:
    // コンストラクタ
    BitmapManager() {
        file = nullptr;
        image = nullptr;
    }

    // デストラクタ
    ~BitmapManager() {
        fclose(file);
        delete[] image;
    }

    // メソッド定義
    void loadData(std::string filename);  // データ読み込み
    void writeData(std::string filename);  // データ書き込み
    void displayHeader();  // 画像情報を標準出力へ出力
    int getHeight();  // 画像の高さの大きさを取得
    int getWidth();  // 画像の幅の大きさを取得
    Color getColor(int row, int col);  // 指定した画素の色を取得
    void setColor(int row, int col, int r, int g, int b);  // 指定した画素へ色を設定

    // 2ndより メソッド追加
    FileHeader getFileHeader();
    void setFileHeader(FileHeader);
    InfoHeader getInfoHeader();
    void setInfoHeader(InfoHeader);
    void copy(BitmapManager &);

private:
    void readFileHeader();
    void readInfoHeader();
    void readImageData();
};

#endif // BITMAP_MANAGER_HPP