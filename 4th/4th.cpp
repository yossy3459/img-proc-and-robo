#define _USE_MATH_DEFINES
#include "bitmap_manager.hpp"
#include <algorithm>

using namespace std;

/**
 * 画素値に対するラベルを管理するクラス
 */
class Label {
    int width;
    int height;
    int *data;

public:
    /**
     * @fn 画像のサイズを保存し、画素に対応する領域を確保する
     * @param width 画像の幅
     * @param height 画素の高さ
     */
    void setSize(int width, int height){
        this->width = width;
        this->height = height;
        data = new int[width * height];
    }

    /**
     * @fn 画素に対するラベルを保存
     * @param row 画素の行
     * @param col 画素の列
     * @param tempData 保存するデータ
     */
    void setData(int row, int col, int tempData){
        this->data[row * width + col] = tempData;
    }

    /**
     * @fn 保存されているラベルを取り出す
     * @param row 画素の行
     * @param col 画素の列
     * @return 角度値
     */
    int getData(int row, int col){
        return data[row * width + col];
    }
};

/*
 * 長方形を管理する構造体
 */
struct Rectangle {
    int index;    // ラベル番号(修正前)
    int top;
    int bottom;
    int left;
    int right;
};


/**
 * @fn カラー画像をグレイスケール画像へ変換
 * @param bmp ビットマップマネージャー
 * @count ヒストグラム用の配列 [0 256)
 */
void color2Grayscale(BitmapManager *bmp, int *count) {
    //! 各色に乗ずる係数
    const float c_r = 0.3;
    const float c_g = 0.59;
    const float c_b = 0.11;

    //! 各画素の色を保存する変数
    Color color;
    //! カラー画素から生成したグレイスケールの値
    int grayValue;

    for (int row = 0; row < bmp->getHeight(); row++){
        for (int col = 0; col < bmp->getWidth(); col++){
            // カラー取得
            color = bmp->getColor(row, col);

            // 変換式によってグレースケールへ変換
            grayValue = (int)(c_r * color.r + c_g * color.g + c_b * color.b);

            // 求めたグレースケール値をセット
            bmp->setColor(row, col, grayValue, grayValue, grayValue);

            // ヒストグラム用にグレースケール値をカウント
            count[grayValue]++;
        }
    }
}

/**
 * @fn 判別分析法を用いて画像を2値化
 */
void applyBinarization(BitmapManager *bmp, int *hist) {

    // 判別分析法
    //! しきい値
    int threshold = 0;
    //! pixelNum1 * pixelNum2 * (ave1 - ave2)^2 の最大値
    double max = 0.0;

    for (int i = 0; i < 256; ++i){
        int pixelNum1 = 0;  //! クラス1の画素数
        int pixelNum2 = 0;  //! クラス2の画素数
        long sum1 = 0;  //! クラス1の平均を出すための合計値
        long sum2 = 0;  //! クラス2の平均を出すための合計値
        double ave1 = 0.0;  //! クラス1の平均
        double ave2 = 0.0;  //! クラス2の平均

        // iまで クラス1へ格納
        for (int j = 0; j <= i; ++j){
            pixelNum1 += hist[j];
            sum1 += j * hist[j];
        }

        // i以降 クラス2へ格納
        for (int j = i + 1; j < 256; ++j){
            pixelNum2 += hist[j];
            sum2 += j * hist[j];
        }

        // 平均導出
        if (pixelNum1)
            ave1 = (double)sum1 / pixelNum1;

        if (pixelNum2)
            ave2 = (double)sum2 / pixelNum2;

        // 分散導出
        double tmp = ((double)pixelNum1 * pixelNum2 * (ave1 - ave2) * (ave1 - ave2));

        // 最大値更新
        if (tmp > max){
            max = tmp;
            threshold = i;
        }
    }

    cout << "threshold: " << threshold << endl;

    /* tの値を使って2値化 */
    for (int row = 0; row < bmp->getHeight(); row++){
        for (int col = 0; col < bmp->getWidth(); col++){
            // しきい値を下回れば0、上回れば255で書き込み
            if (bmp->getColor(row, col).r < threshold)
                bmp->setColor(row, col, 0, 0, 0);
            else
                bmp->setColor(row, col, 255, 255, 255);
        }
    }
}

/**
 * @fn ラベル化を適用
 * @param img ２値画像
 * @param lut ラベル番号処理用のルックアップテーブル
 * @param label ラベルをデータとした二次元配列
 */
void applyClassification(BitmapManager *img, vector<int> lut, Label label) {

    int candidate = 0, tempColor = 0;
    bool existNonZero = false;
    vector<int> surround;

    lut.push_back(0);

    // label初期化
    for (int row = 0; row < img->getHeight(); row++){
        for (int col = 0; col < img->getWidth(); col++){
            label.setData(row, col, 0);
        }
    }

    for (int row = 1; row < img->getHeight()-1; row++){
        for (int col = 1; col < img->getWidth()-1; col++){

            // 注目画素が白のとき
            if (img->getColor(row, col).r == 255){
                surround.clear();
                // 左下、下、右下、左の画素を取得
                surround.push_back(label.getData(row-1, col-1));
                surround.push_back(label.getData(row-1, col));
                surround.push_back(label.getData(row-1, col+1));
                surround.push_back(label.getData(row, col-1));

                // その画素を探索
                for (auto itr = surround.begin(); itr != surround.end(); ++itr) {
                    // 0以外の数字があるかどうかを確認
                    if (*itr != 0) {
                        existNonZero = true;
                    }
                }

                // 0しかないとき
                if (existNonZero == false) {
                    // 注目画素へlutの次の値を代入
                    label.setData(row, col, lut.size());
                    lut.push_back(lut.size());
                    continue;
                }

                // 0以外もあるとき
                if (existNonZero == true) {
                    // candidateとその周辺画素を比較
                    for (auto itr = surround.begin(); itr != surround.end(); ++itr) {
                        if (*itr != 0){
                            // candidateが0のときは無条件で代入
                            if (candidate == 0){
                                candidate = *itr;
                            }
                            // candidateが0以外のとき、周辺画素のほうが小さければ、candidateに代入
                            else if (candidate > *itr) {
                                candidate = *itr;
                            }
                        }
                    }
                    // 注目画素にセット
                    label.setData(row, col, candidate);

                    // lutの更新
                    for (auto itr = surround.begin(); itr != surround.end(); ++itr) {
                        if (*itr != 0 && *itr > candidate) {
                            lut[*itr] = candidate;

                            while (candidate != lut[candidate]) {
                                lut[*itr] = lut[candidate];
                                candidate = lut[candidate];
                            }
                        }
                    }
                }

                // 初期化
                candidate = 0;
                existNonZero = false;
            }
        }
    }

    int tempLabelData;

    // lutに従って、candidateを更新
    for (int row = 1; row < img->getHeight()-1; row++){
        for (int col = 1; col < img->getWidth()-1; col++){
            tempLabelData = label.getData(row, col);
            label.setData(row, col, lut[tempLabelData]);
        }
    }
}

/**
 * ラベルの上下左右の情報を元に、長方形を画像に書き込む
 * @param img カラー画像
 * @param label ラベル情報
 */
void displayClassification(BitmapManager *img, Label label) {
    vector<Rectangle> rect;
    Rectangle tempRect;
    int currentLabel;
    bool alreadyExist = false;

    for (int row = 1; row < img->getHeight()-1; row++){
        for (int col = 1; col < img->getWidth()-1; col++){
            
            currentLabel = label.getData(row, col);

            // ラベル情報が0でないとき
            if (currentLabel != 0) {

                // 全矩形を探索、indexが同一のラベルがあれば、上下左右を取り出す。大小比較して更新。
                for (auto itr = rect.begin(); itr != rect.end(); ++itr){
                    if ((*itr).index == currentLabel) {
                        alreadyExist = true;
                        if (itr->top > row)  itr->top = row;
                        if (itr->bottom < row) itr->bottom = row;
                        if (itr->left > col) itr->left = col;
                        if (itr->right < col) itr->right = col;
                    }
                }
                
                // まだ矩形情報がない時、ラベルから矩形情報を生成してpush_back
                if (alreadyExist == false) {
                    tempRect.top = tempRect.bottom = row;
                    tempRect.left = tempRect.right = col;
                    tempRect.index = label.getData(row, col);
                    rect.push_back(tempRect);
                }

                alreadyExist = false;
            }

        }
    }

    // 画像に書き込み
    for (auto itr = rect.begin(); itr != rect.end(); ++itr) {

        // ラベル情報を標準出力へ出力
        cout << distance(rect.begin(), itr) << " (top, bottom, left, right) = (" << itr->top << ", " << itr->bottom << ", " << itr->left << ", " << itr->right << ")" << endl;

        // 矩形の周囲を赤色で塗る
        for (int row = itr->top-1; row <= itr->bottom+1; row++) {
            img->setColor(row, itr->left-1, 255, 0, 0);
            img->setColor(row, itr->right+1, 255, 0, 0);
        }

        for (int col = itr->left-1; col <= itr->right+1; col++){
            img->setColor(itr->top-1, col, 255, 0, 0);
            img->setColor(itr->bottom+1, col, 255, 0, 0);
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc != 2){
        cerr << "Usage ./prog filename(without .bmp)" << endl;
        return -1;
    }

    //! ファイル名
    string src_filename = "src/" + string(argv[1]) + ".bmp";
    string gray_filename = "dst/" + string(argv[1]) + "_gray.bmp";
    string binarization_filename = "dst/" + string(argv[1]) + "_binarization.bmp";
    string classification_filename = "dst/" + string(argv[1]) + "_classification.bmp";


    // Bitmap
    BitmapManager src, gray, binarization, imgClassification;

    //! for color2Grayscale
    int count[256] = {0};

    // 画像読み込み
    src.loadData(src_filename);
    src.displayHeader();

    // LUT for applyClassification
    vector<int> lut;
    Label label;
    label.setSize(src.getWidth(), src.getHeight());

    // グレースケール化
    gray.copy(src);
    color2Grayscale(&gray, count);
    gray.writeData(gray_filename);

    // 処理
    // 1. 2値化
    binarization.copy(gray);
    applyBinarization(&binarization, count);
    binarization.writeData(binarization_filename);
    // 2. ラベリング
    imgClassification.copy(binarization);
    applyClassification(&imgClassification, lut, label);
    // 3. ラベリングの枠をカラー画像に表示
    displayClassification(&src, label);
    src.writeData(classification_filename);

    return 0;
}
