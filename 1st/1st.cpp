#include "bitmap_manager.hpp"

using namespace std;

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
 * @fn ヒストグラムの最大値を求める
 * @detail gnuplotのy軸の範囲を決定
 */
int count_max(int *count){
    auto temp_max = 0;

    for (int i = 0; i < 256; i++) {
        if (temp_max < count[i]) {
            temp_max = count[i];
        }
    }

    return temp_max;
}

/**
 * @fn gnuplotを用いてヒストグラムを生成
 */
void showHistgram(BitmapManager *bmp, int *count, string img_name) {
    FILE *gp;
    int i;

    //! ヒストグラムの最大値 
    auto max = count_max(count);

    // gnuplot 起動・設定
    gp = popen("gnuplot -persist", "w");  // パイプを開き、gnuplotの立ち上げ
    //fprintf(gp, "set multiplot\n");  // マルチプロットモード
    fprintf(gp, "set terminal png\n");
    fprintf(gp, "set out \"./histgram/%s_histgram.png\"\n", img_name.c_str());
    fprintf(gp, "set xrange [0:255]\n");  // 範囲の指定 x[0 255]
    fprintf(gp, "set yrange [0:%d]\n", max + 500);  // 範囲の指定 y[0 ヒストグラムの最大値+1000]
    fprintf(gp, "set xlabel \"Pixel Values\"\n");  // ラベル表示
    fprintf(gp, "set ylabel \"Frequency\"\n");

    // プロット
    fprintf(gp, "plot '-' with lines linetype 1\n");
    for (i = 0; i < 255; ++i) {
        fprintf(gp, "%d\t%d\n", i, count[i]);
    }
    fprintf(gp, "e\n");

    // gnuplot 終了処理
    // fprintf(gp, "set nomultiplot\n"); // マルチプロットモード終了
    fprintf(gp, "exit\n"); // gnuplotの終了
    fflush(gp);
    pclose(gp); // パイプを閉じる
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


int main(int argc, char *argv[]) {

    if (argc != 2){
        cerr << "Usage ./prog filename(without .bmp)" << endl;
        return -1;
    }

    //! ファイル名
    string src_filename = "src/" + string(argv[1]) + ".bmp";
    string gray_filename = "dst/" + string(argv[1]) + "_gray.bmp";
    string binarization_filename = "dst/" + string(argv[1]) + "_binarization.bmp";

    // Bitmap
    BitmapManager bmp;
    // ヒストグラム用カウンタ
    int count[256] = {0};

    // 画像読み込み
    bmp.loadData(src_filename);
    bmp.displayHeader();

    // グレースケール化
    color2Grayscale(&bmp, count);
    bmp.writeData(gray_filename);

    // ヒストグラム表示
    showHistgram(&bmp, count, string(argv[1]));

    // 判別分析法の利用
    applyBinarization(&bmp, count);
    bmp.writeData(binarization_filename);

    return 0;
}
