#include "bitmap_manager.hpp"

using namespace std;

/**
 * @fn クイックソート
 * @param first イテレータのはじめ
 * @param last イテレータの終わり
 */
template< class T >
void quick_sort(T first, T last) {
    if (last - first <= 1) { return; }
    T i = first, j = last - 1;
    for (T pivot = first;; ++i, --j) {
        while (*i < *pivot)  ++i;
        while (*pivot < *j)  --j;
        if (i >= j)  break;
        iter_swap(i, j);
    }
    quick_sort(first, i);
    quick_sort(j + 1, last);
}

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
 * @fn 3x3 平均フィルタを適用
 * @param src 元画像
 * @count dst 結果画像
 */
void applyAvarageFilter(BitmapManager *src, BitmapManager *dst){
    //! 各画素の計算用(和、平均値)
    int sum=0, ave=0;

    // 上下左右1画素ずつ除いてループ
    for (int row = 1; row < src->getHeight()-1; row++) {
        for (int col = 1; col < src->getWidth()-1; col++) {

            // 画素値取得、周囲の画素を取り込んで総和を取る
            for (int innerRow = -1; innerRow <= 1; innerRow++) {
                for (int innerCol = -1; innerCol <= 1; innerCol++) {
                    sum += src->getColor(row+innerRow, col+innerCol).r;
                }
            }

            // 平均を出す (3x3 = 9)
            ave = sum / 9;

            // for debug
            //cout << ave << endl;

            // set
            dst->setColor(row, col, ave, ave, ave);

            // 変数リセット
            sum = ave = 0;
        }
    }
    // for debug
    cout << "Completed: avarageFilter" << endl;
}

/**
 * @fn 3x3 ガウシアンフィルタを適用
 * @param src 元画像
 * @count dst 結果画像
 */
void applyGaussianFilter(BitmapManager *src, BitmapManager *dst){
    //! 各画素の計算用(和、平均値)
    int sum=0, ave=0;

    //! 各画素にかけるフィルターの重み
    int w[3][3] = {1, 2, 1,
                   2, 4, 2,
                   1, 2, 1};

    for (int row = 1; row < src->getHeight()-1; row++) {
        for (int col = 1; col < src->getWidth()-1; col++) {

            // 画素値取得、周囲の画素を取り込む、wをかけることで重みもつける
            for (int innerRow = -1; innerRow <= 1; innerRow++) {
                for (int innerCol = -1; innerCol <= 1; innerCol++) {
                    sum += src->getColor(row+innerRow, col+innerCol).r * w[innerRow+1][innerCol+1];
                }
            }

            // 平均を出す
            ave = sum / 16;

            // for debug
            //cout << ave << endl;

            // set
            dst->setColor(row, col, ave, ave, ave);

            // 変数リセット
            sum = ave = 0;
        }
    }
    // for debug
    cout << "Completed: gaussianFilter" << endl;
}

/**
 * @fn メディアンフィルタ
 * @param src 元画像
 * @count dst 結果画像
 */
void applyMedianFilter(BitmapManager *src, BitmapManager *dst){
    //! 
    vector<int> elementArray;

    //!
    int tempMid = 0;

    for (int row = 1; row < src->getHeight()-1; row++) {
        for (int col = 1; col < src->getWidth()-1; col++) {

            // 画素値取得、周囲の画素を取り込む
            for (int innerRow = -1; innerRow <= 1; innerRow++) {
                for (int innerCol = -1; innerCol <= 1; innerCol++) {
                    elementArray.push_back(src->getColor(row+innerRow, col+innerCol).r);
                }
            }

            // ソートを行う
            quick_sort(elementArray.begin(), elementArray.end());

            // 中央値を取り出す
            tempMid = elementArray[(elementArray.size() - 1) / 2];

            // for debug
            //cout << ave << endl;

            // set
            dst->setColor(row, col, tempMid, tempMid, tempMid);

            // 変数リセット
            elementArray.clear();
            tempMid = 0;
        }
    }
    // for debug
    cout << "Completed: medianFilter" << endl;
}



int main(int argc, char *argv[]) {

    if (argc != 2){
        cerr << "Usage ./prog filename(without .bmp)" << endl;
        return -1;
    }

    //! ファイル名
    string src_filename = "src/" + string(argv[1]) + ".bmp";
    string gray_filename = "dst/" + string(argv[1]) + "_gray.bmp";
    string avarageFilter_filename = "dst/" + string(argv[1]) + "_avarageFilter.bmp";
    string gaussianFilter_filename = "dst/" + string(argv[1]) + "_gaussianFilter.bmp";
    string medianFilter_filename = "dst/" + string(argv[1]) + "_medianFilter.bmp";

    // Bitmap
    BitmapManager src, dstAve, dstGauss, dstMedian;
    // ヒストグラム用カウンタ
    int count[256] = {0};

    // 画像読み込み
    src.loadData(src_filename);
    src.displayHeader();

    // グレースケール化
    color2Grayscale(&src, count);
    src.writeData(gray_filename);

    // 画像をコピー
    dstAve.copy(src);
    dstGauss.copy(src);
    dstMedian.copy(src);

    // 平均フィルタ適用
    applyAvarageFilter(&src, &dstAve);
    dstAve.writeData(avarageFilter_filename);

    // ガウシアンフィルタ適用
    applyGaussianFilter(&src, &dstGauss);
    dstGauss.writeData(gaussianFilter_filename);

    // メディアンフィルタ適用
    applyMedianFilter(&src, &dstMedian);
    dstMedian.writeData(medianFilter_filename);

    return 0;
}
