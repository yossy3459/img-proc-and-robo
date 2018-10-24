#include "bitmap_manager.hpp"

using namespace std;

#define PREWITT 0
#define SOBEL 1

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
 * @fn エッジフィルターを適用
 * @param src 元画像
 * @param dst 結果画像
 * @param mode (prewitt or sobel)
 */
void applyEdgeFilter(BitmapManager *src, BitmapManager *dst, int mode){
    // エラー処理 (モードが適切かどうかを判定)
    if ((mode != PREWITT) && (mode != SOBEL)) {
        cerr << "applyEgdeFilter: mode error" << endl;
        return;
    }

    //! 各画素の計算用(x, y, total)
    int gx = 0, gy = 0, g = 0;

    //! 横方向 prewitt 重み
    int prewitt_wx[3][3] = {-1, 0, 1,
                      -1, 0, 1,
                      -1, 0, 1};
    //! 縦方向 prewitt 重み
    int prewitt_wy[3][3] = {-1, -1, -1,
                       0,  0,  0,
                       1,  1,  1};
    //! 横方向 sobel 重み
    int sobel_wx[3][3] = {-1, 0, 1,
                    -2, 0, 2,
                    -1, 0, 1};
    //! 横方向 sobel 重み
    int sobel_wy[3][3] = {-1, -2, -1,
                     0,  0,  0,
                     1,  2,  1};


    for (int row = 1; row < src->getHeight()-1; row++) {
        for (int col = 1; col < src->getWidth()-1; col++) {

            // 画素値取得、周囲の画素を取り込む、wx, wyをかけることで重みもつける
            for (int innerRow = -1; innerRow <= 1; innerRow++) {
                for (int innerCol = -1; innerCol <= 1; innerCol++) {
                    if (mode == PREWITT) {
                        gx += src->getColor(row+innerRow, col+innerCol).r * prewitt_wx[innerRow+1][innerCol+1];
                        gy += src->getColor(row+innerRow, col+innerCol).r * prewitt_wy[innerRow+1][innerCol+1];
                    }
                    if (mode == SOBEL) {
                        gx += src->getColor(row+innerRow, col+innerCol).r * sobel_wx[innerRow+1][innerCol+1];
                        gy += src->getColor(row+innerRow, col+innerCol).r * sobel_wy[innerRow+1][innerCol+1];

                    }
                }
            }

            // 二乗の和の平方根を出す
            g = sqrt(gx*gx + gy*gy);

            // 255を超えたとき、255で抑制
            if (g > 255) g = 255;

            // for debug
            //cout << ave << endl;

            // set
            dst->setColor(row, col, g, g, g);

            // 変数リセット
            gx = gy = g = 0;
        }
    }
    // for debug
    cout << "Completed: EdgeFilter ";
    if (mode == PREWITT)
        cout << "(mode: prewitt)" << endl;
    if (mode == SOBEL)
        cout << "(mode: sobel)" << endl;

    return;
}

/**
 * @fn ラプラシアンフィルターを適用
 * @param src 元画像
 * @param dst 結果画像
 */
void applyLaplacianFilter(BitmapManager *src, BitmapManager *dst){
    //! 各画素の計算用(和、平均値)
    int tempElement = 0;

    int w[3][3] = {1,  1, 1,
             1, -8, 1,
             1,  1, 1};


    for (int row = 1; row < src->getHeight()-1; row++) {
        for (int col = 1; col < src->getWidth()-1; col++) {

            // 画素値取得、周囲の画素を取り込む、wx, wyをかけることで重みもつける
            for (int innerRow = -1; innerRow <= 1; innerRow++) {
                for (int innerCol = -1; innerCol <= 1; innerCol++) {
                    tempElement += src->getColor(row+innerRow, col+innerCol).r * w[innerRow+1][innerCol+1];
                }
            }

            // 各種抑制
            if (tempElement > 255)  tempElement = 255;
            if (tempElement < 0)    tempElement = 0;

            // for debug
            //cout << ave << endl;

            // set
            dst->setColor(row, col, tempElement, tempElement, tempElement);

            // 変数リセット
            tempElement = 0;
        }
    }
    // for debug
    cout << "Completed: LaplacianFilter" << endl;

    return;
}

int main(int argc, char *argv[]) {

    if (argc != 2){
        cerr << "Usage ./prog filename(without .bmp)" << endl;
        return -1;
    }

    //! ファイル名
    string src_filename = "src/" + string(argv[1]) + ".bmp";
    string gray_filename = "dst/" + string(argv[1]) + "_gray.bmp";
    string prewittFilter_filename = "dst/" + string(argv[1]) + "_prewittFilter.bmp";
    string sobelFilter_filename = "dst/" + string(argv[1]) + "_sobelFilter.bmp";
    string laplacianFilter_filename = "dst/" + string(argv[1]) + "_laplacianFilter.bmp";

    // Bitmap
    BitmapManager src, dstPrewitt, dstSobel, dstLaplacian;

    //! for color2Grayscale
    int count[256] = {0};

    // 画像読み込み
    src.loadData(src_filename);
    src.displayHeader();

    // グレースケール化
    color2Grayscale(&src, count);
    src.writeData(gray_filename);

    // 画像をコピー
    dstPrewitt.copy(src);
    dstSobel.copy(src);
    dstLaplacian.copy(src);

    // prewittフィルタ適用
    applyEdgeFilter(&src, &dstPrewitt, PREWITT);
    dstPrewitt.writeData(prewittFilter_filename);

    // sobelフィルタ適用
    applyEdgeFilter(&src, &dstSobel, SOBEL);
    dstSobel.writeData(sobelFilter_filename);

    // Laplacianフィルタ適用
    applyLaplacianFilter(&src, &dstLaplacian);
    dstLaplacian.writeData(laplacianFilter_filename);

    return 0;
}
