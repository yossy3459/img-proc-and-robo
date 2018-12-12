#define _USE_MATH_DEFINES
#include "bitmap_manager.hpp"
#include <algorithm>

using namespace std;

#define DILATION_MAX 3
#define EROSION_MAX 3


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

void dilation(BitmapManager *img) {
    BitmapManager dst;
    dst.copy(*img);

    for (int row = 1; row < img->getHeight()-1; row++){
        for (int col = 1; col < img->getWidth()-1; col++){

            for (int innerRow = -1; innerRow <= 1; innerRow++) {
                for (int innerCol = -1; innerCol <= 1; innerCol++) {

                    if (img->getColor(row + innerRow, col + innerCol).r == 255)
                        dst.setColor(row, col, 255, 255, 255);

                }
            }

        }
    }

    img->copy(dst);
}

void erosion(BitmapManager *img) {
    BitmapManager dst;
    dst.copy(*img);

    for (int row = 1; row < img->getHeight()-1; row++){
        for (int col = 1; col < img->getWidth()-1; col++){

            for (int innerRow = -1; innerRow <= 1; innerRow++) {
                for (int innerCol = -1; innerCol <= 1; innerCol++) {

                    if (img->getColor(row + innerRow, col + innerCol).r == 0)
                        dst.setColor(row, col, 0, 0, 0);

                }
            }

        }
    }

    img->copy(dst);
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
    string dilation_filename = "dst/" + string(argv[1]) + "_dilation.bmp";
    string erosion_filename = "dst/" + string(argv[1]) + "_erosion.bmp";

    // Bitmap
    BitmapManager src, gray, binarization, out_dilation, out_erosion;

    //! for color2Grayscale
    int count[256] = {0};

    // 画像読み込み
    src.loadData(src_filename);
    src.displayHeader();

    // グレースケール化
    gray.copy(src);
    color2Grayscale(&gray, count);
    gray.writeData(gray_filename);

    // 1. 2値化
    binarization.copy(gray);
    applyBinarization(&binarization, count);
    binarization.writeData(binarization_filename);

    // 初期化
    out_dilation.copy(binarization);
    out_erosion.copy(binarization);

    // 処理
    // 1. Dilation
    for (int i=0; i<DILATION_MAX; i++) {
        dilation(&out_dilation);
    }
    out_dilation.writeData(dilation_filename);

    // 2. Erosion
    for (int i=0; i<EROSION_MAX; i++) {
        erosion(&out_erosion);
    }
    out_erosion.writeData(erosion_filename);

    return 0;
}
