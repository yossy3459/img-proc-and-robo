#define _USE_MATH_DEFINES
#include "bitmap_manager.hpp"

using namespace std;

#define PREWITT 0
#define SOBEL 1

#define T_UPPER 150
#define T_LOWER 50

/**
 * 画素値に対する角度を管理するクラス。Arctan周辺の処理で用いる
 */
class Angle{
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
     * @fn 画素に対する角度を保存
     * @param row 画素の行
     * @param col 画素の列
     * @param tempData 保存するデータ
     */
    void setData(int row, int col, int tempData){
        this->data[row * width + col] = tempData;
    }

    /**
     * @fn 保存されている角度を取り出す
     * @param row 画素の行
     * @param col 画素の列
     * @return 角度値
     */
    int getData(int row, int col){
        return data[row * width + col];
    }
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
 * @fn 5x5 ガウシアンフィルタを適用
 * @param src 元画像
 * @count dst 結果画像
 */
void applyGaussianFilter5x5(BitmapManager *src, BitmapManager *dst){
    //! 各画素の計算用(和、平均値)
    int sum=0, ave=0;

    //! 各画素にかけるフィルターの重み
    int w[5][5] = { 1,  4,  6,  4,  1,
                    4, 16, 24, 16,  4,
                    6, 24, 36, 24,  6,
                    4, 16, 24, 16,  4,
                    1,  4,  6,  4,  1};

    for (int row = 2; row < src->getHeight()-2; row++) {
        for (int col = 2; col < src->getWidth()-2; col++) {

            // 画素値取得、周囲の画素を取り込む、wをかけることで重みもつける
            for (int innerRow = -2; innerRow <= 2; innerRow++) {
                for (int innerCol = -2; innerCol <= 2; innerCol++) {
                    sum += src->getColor(row+innerRow, col+innerCol).r * w[innerRow+2][innerCol+2];
                }
            }

            // 平均を出す
            ave = sum / 256;

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
 * @fn Sobelフィルターを適用
 * @param src 元画像
 * @param dst 結果画像
 * @param dstAtan Arctanによる勾配方向の情報
 */
void applySobelFilter(BitmapManager *src, BitmapManager *dst, Angle angle){
    //! 各画素の計算用(x, y, total)
    int gx = 0, gy = 0, g = 0;
    auto theta = 0;

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
                    gx += src->getColor(row+innerRow, col+innerCol).r * sobel_wx[innerRow+1][innerCol+1];
                    gy += src->getColor(row+innerRow, col+innerCol).r * sobel_wy[innerRow+1][innerCol+1];
                }
            }

            // 二乗の和の平方根を出す
            g = sqrt(gx*gx + gy*gy);

            // 255を超えたとき、255で抑制
            if (g > 255) g = 255;

            // arctanについて処理、ラジアン値をthetaへ代入
            theta = atan2(gy, gx) * 180 / M_PI;

            // set
            dst->setColor(row, col, g, g, g);
            angle.setData(row, col, theta);
            // cout << "(" << row << ", " << col << "): " << theta << ", " << angle.getData(row, col) << endl;

            // 変数リセット
            gx = gy = g = theta = 0;
        }
    }
    // for debug
    cout << "Completed: SobelFilter" << endl;

    return;
}

/**
 * @fn 最大値抑制用処理
 * @param srcSobel ソーベルフィルタをかけた画像
 * @param srcAtan 方向値を保持した画像
 * @param dst 最大値抑制をかけたソーベルフィルタ画像
 */
void nonMaximumSuppression(BitmapManager* srcSobel, Angle angle, BitmapManager* dst) {

    //! 4方向の定義, 上下、左右、右上がり、左上がりを定義
    const int dir_0_180 = 0;
    const int dir_45_225 = 45;
    const int dir_90_270 = 90;
    const int dir_135_305 = 135;

    //! 度数法で表したarctanの結果 (-180 <= θ <= 180)
    auto degreeTheta = 0;

    // 方向近似
    for (int row = 1; row < srcSobel->getHeight()-1; row++) {
        for (int col = 1; col < srcSobel->getWidth()-1; col++) {
            // ラジアン値から度数を取り出す
            degreeTheta = angle.getData(row, col);

            //cout << degreeTheta << endl;
            // -180 ~ 180の値について、srcAtanを近似化
            if (-180.0 <= degreeTheta && degreeTheta < -157.5)       angle.setData(row, col, dir_0_180);
            else if (-157.5 <= degreeTheta && degreeTheta < -112.5)  angle.setData(row, col, dir_45_225);
            else if (-112.5 <= degreeTheta && degreeTheta < -67.5)   angle.setData(row, col, dir_90_270);
            else if (-67.5 <= degreeTheta && degreeTheta < -22.5)    angle.setData(row, col, dir_135_305);
            else if (-22.5 <= degreeTheta && degreeTheta < 22.5)     angle.setData(row, col, dir_0_180);
            else if (22.5 <= degreeTheta && degreeTheta < 67.5)      angle.setData(row, col, dir_45_225);
            else if (67.5 <= degreeTheta && degreeTheta < 112.5)     angle.setData(row, col, dir_90_270);
            else if (112.5 <= degreeTheta && degreeTheta < 157.5)    angle.setData(row, col, dir_135_305);
            else if (157.5 <= degreeTheta && degreeTheta <= 180.0)   angle.setData(row, col, dir_0_180);
            else {
                cerr << "Error: nonMaximumSuppression: Atan_direction_approximated" << endl;
                cerr << "(row, col) = (" << row << ", " << col << ")" << endl;
                cerr << "degreeTheta:" << degreeTheta << endl;
                return;
            }
            //cout << angle.getData(row, col) << endl;
        }
    }

    //! 注目画素
    int interestedPixel;

    // 勾配方向の隣接マスを確認し、極大値でなければ0を代入
    for (int row = 1; row < srcSobel->getHeight()-1; row++) {
        for (int col = 1; col < srcSobel->getWidth()-1; col++) {
            interestedPixel = angle.getData(row, col);

            switch (interestedPixel) {
                // 左右
                case dir_0_180:
                    if (srcSobel->getColor(row, col).r < srcSobel->getColor(row-1, col).r
                        || srcSobel->getColor(row, col).r < srcSobel->getColor(row+1, col).r)
                        dst->setColor(row, col, 0, 0, 0);
                    break;
                // 右上がり方向
                case dir_45_225:
                    if (srcSobel->getColor(row, col).r < srcSobel->getColor(row-1, col-1).r
                        || srcSobel->getColor(row, col).r < srcSobel->getColor(row+1, col+1).r)
                        dst->setColor(row, col, 0, 0, 0);
                    break;
                // 上下方向
                case dir_90_270:
                    if (srcSobel->getColor(row, col).r < srcSobel->getColor(row, col-1).r
                        || srcSobel->getColor(row, col).r < srcSobel->getColor(row, col+1).r)
                        dst->setColor(row, col, 0, 0, 0);
                    break;
                // 左上がり方向
                case dir_135_305:
                    if (srcSobel->getColor(row, col).r < srcSobel->getColor(row+1, col-1).r
                        || srcSobel->getColor(row, col).r < srcSobel->getColor(row-1, col+1).r)
                        dst->setColor(row, col, 0, 0, 0);
                    break;
                default:
                    cerr << "Error: nonMaximumSuppression: Suppression" << endl;
                    break;
            }
        }
    }

    cout << "Completed: nonMaximumSuppression" << endl;

}

/**
 * @fn 他のエッジに隣接しているかを確認する
 * @param src 画像
 * @param row 注目画素の行
 * @param col 注目画素の列
 * @param t_upper 上側のエッジしきい値
 * @return エッジとして採用してよいかどうか (t or f)
 */
bool checkAdoptedEdge(BitmapManager* src, int row, int col, int t_upper) {
    
    for (int innerRow = -1; innerRow <= 1; innerRow++) {
        for (int innerCol = -1; innerCol <= 1; innerCol++) {
            // 隣接8方向に上側のエッジしきい値を超えるものがあればtrueを返す
            if (src->getColor(row+innerRow, col+innerCol).r >= t_upper)
                return true;
        }
    }

    return false;
}

/**
 * @fn ヒステリシスのしきい値処理
 * @param src 最大値抑制をした画像
 * @param dst 出力画像
 * @param t_upper しきい値(上)
 * @param t_lower しきい値(下)
 */
void hysteresisThreshold(BitmapManager* src, BitmapManager* dst, int t_upper, int t_lower) {
    bool isAdoptedEdge = false;

    for (int row = 1; row < src->getHeight()-1; row++) {
        for (int col = 1; col < src->getWidth()-1; col++) {
            // 上側は輪郭として採用
            if (src->getColor(row, col).r >= t_upper) {
                dst->setColor(row, col, 255, 255, 255);
            }

            // 下側は不採用
            else if (src->getColor(row, col).r < t_lower) {
                dst->setColor(row, col, 0, 0, 0);
            }

            // 上側と下側の間なら、周囲に採用される (t_upperよりも大きい) 値があれば採用
            else {
                isAdoptedEdge = checkAdoptedEdge(src, row, col, t_upper);

                if (isAdoptedEdge == true)
                    dst->setColor(row, col, 255, 255, 255);
                if (isAdoptedEdge == false)
                    dst->setColor(row, col, 0, 0, 0);

                isAdoptedEdge = false;
            }
        }
    }

    cout << "Completed: hysteresisThreshold" << endl;
}



int main(int argc, char *argv[]) {

    if (argc != 2){
        cerr << "Usage ./prog filename(without .bmp)" << endl;
        return -1;
    }

    //! ファイル名
    string src_filename = "src/" + string(argv[1]) + ".bmp";
    string gauss_filename = "dst/" + string(argv[1]) + "_gauss.bmp";
    string sobel_filename = "dst/" + string(argv[1]) + "_sobel.bmp";
    string sup_filename = "dst/" + string(argv[1]) + "_sup.bmp";
    string canny_filename = "dst/" + string(argv[1]) + "_canny.bmp";

    // Bitmap
    BitmapManager src, imgGauss, imgSobel, imgSuppression, dst;

    //! for color2Grayscale
    int count[256] = {0};

    // 画像読み込み
    src.loadData(src_filename);
    src.displayHeader();

    // グレースケール化
    color2Grayscale(&src, count);
    // src.writeData(gray_filename);

    // atan用Angle宣言
    Angle angle;
    angle.setSize(src.getWidth(), src.getHeight());

    // 処理
    // 1. 5x5 ガウシアンフィルタ適用
    imgGauss.copy(src);
    applyGaussianFilter5x5(&src, &imgGauss);
    // 2. ソーベルフィルタ適用
    imgSobel.copy(imgGauss);
    applySobelFilter(&imgGauss, &imgSobel, angle);
    // 3. 最大値抑制
    imgSuppression.copy(imgSobel);
    nonMaximumSuppression(&imgSobel, angle, &imgSuppression);
    // 4. ヒステリシスのしきい値適用
    dst.copy(imgSuppression);
    hysteresisThreshold(&imgSuppression, &dst, T_UPPER, T_LOWER);

    // 中間画像をすべて出力
    imgGauss.writeData(gauss_filename);
    imgSobel.writeData(sobel_filename);
    imgSuppression.writeData(sup_filename);
    dst.writeData(canny_filename);

    return 0;
}
