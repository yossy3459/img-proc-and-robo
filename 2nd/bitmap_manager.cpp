#include "bitmap_manager.hpp"

using namespace std;

/*
* @fn 4ビット情報をInt整数値に変換
* @param b1 最下位ビット
* @param b2 次のビット
* @param b3 次のビット
* @param b4 最上位ビット
* @return 変換された整数値
*/
int bit2Integer(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
    return b1 +
            b2 * 256 +
            b3 * 256 * 256 +
            b4 * 256 * 256 * 256;
}

/**
 * @fn ビットマップデータをヘッダー・データに分けて読み込み
 * @param filenameファイル名
 */
void BitmapManager::loadData(string filename) {
    if (file != NULL){
        fclose(file);
    }

    // ファイルオープン
    file = fopen(filename.c_str(), "rb");

    if (file == NULL) {
        cout << "Error: can't open " << filename << "." << endl;
        return;
    }

    // 各種ロード用関数読み込み
    readFileHeader();
    readInfoHeader();
    readImageData();
}

/**
 * @fn ファイルヘッダーをロードする
 */
void BitmapManager::readFileHeader() {
    //! データ格納関数
    uint8_t data[FILE_HEADER_SIZE];

    //! 読み込んだ文字数、freadで読み込み
    size_t count = fread(data, sizeof(uint8_t), FILE_HEADER_SIZE, file);

    // ストア用にオリジナルデータ保存
    memcpy(fileHeader.origData, data, sizeof data);

    fileHeader.type = "";
    fileHeader.type += data[0];
    fileHeader.type += data[1];
    fileHeader.size = bit2Integer(data[2], data[3], data[4], data[5]);
}

/**
 * @fn ファイルヘッダーをロードする
 */
void BitmapManager::readInfoHeader() {
    uint8_t data[INFO_HEADER_SIZE];
    size_t count = fread(data, sizeof(uint8_t), INFO_HEADER_SIZE, file);

    // ストア用にオリジナルデータ保存
    memcpy(infoHeader.origData, data, sizeof data);

    infoHeader.infoHeaderSize = bit2Integer(data[0], data[1], data[2], data[3]);
    infoHeader.width = bit2Integer(data[4], data[5], data[6], data[7]);
    infoHeader.height = bit2Integer(data[8], data[9], data[10], data[11]);
    infoHeader.colorParPixel = bit2Integer(data[14], data[15], 0, 0);

    //! ヘッダ上でのデータサイズ
    // データサイズは非圧縮の場合0があり得るため、分岐させて各年
    auto tempDataSize = bit2Integer(data[20], data[21], data[22], data[23]);
    infoHeader.dataSize = tempDataSize == 0 ? fileHeader.size - 54 : tempDataSize;

    //　高さがマイナスの場合、トップダウンであるためフラグを立てる
    if (infoHeader.height < 0)  is_topdown = true;
}

/**
 * @fn 画像データを読みこむ
 */
void BitmapManager::readImageData() {
    // すでにimageがあったら削除
    if (image != NULL) {
        delete[] image;
    }

    //! ヘッダーで定義されているデータサイズ
    int imageSize = infoHeader.dataSize;

    // インスタンス変数のポインタへ領域確保
    image = new uint8_t[imageSize];

    //! 読み込んだデータ数、freadで読み込み
    int count = fread(image, sizeof(uint8_t), imageSize, file);

    // 読み込んだデータ数とヘッダーでのデータサイズが一致しないとき、エラー処理
    if (count != imageSize) {
        cout << "Error: ヘッダーの画像サイズと実際の画像サイズが矛盾" << endl;
        cout << "count:  " << count << endl;
        cout << "header: " << imageSize << endl;
        return;
    }
}

/**
 * @fn ビットマップデータのファイル書き出し
 * @param filename ファイルの名前
 */
void BitmapManager::writeData(string filename){
    FILE *out = fopen(filename.c_str(), "wb");

    if (out == NULL)
    cout << "Error: 書き出し先のファイルを開けません。" << endl;

    // オリジナルデータをもとにヘッダー書き出し
    fwrite(fileHeader.origData, sizeof(uint8_t), FILE_HEADER_SIZE, out);
    fwrite(infoHeader.origData, sizeof(uint8_t), INFO_HEADER_SIZE, out);

    // データ書き出し
    fwrite(image, sizeof(uint8_t), infoHeader.dataSize, out);

    fclose(out);
}

/**
 * @fn ヘッダー情報を標準出力に表示
 */
void BitmapManager::displayHeader() {
    cout << "FileType:   " << fileHeader.type << endl;
    cout << "FileSize:   " << fileHeader.size << endl;
    cout << "InfoSize:   " << infoHeader.infoHeaderSize << endl;
    cout << "ImgWidth:   " << infoHeader.width << endl;
    cout << "ImgHeight:  " << infoHeader.height << endl;
    cout << "NumofColor: " << infoHeader.colorParPixel << endl;
    cout << "ImgSize:    " << infoHeader.dataSize << endl;
}

/**
 * @fn 取り出す画素の位置を色ごとに指定
 * @param infoHeader 情報ヘッダー
 * @param row 行
 * @param col 列
 * @return 各色の格納されている位置
 */
ColorPosition getColorPosition(InfoHeader infoHeader, int row, int col) {
    // 範囲外かどうかを確認
    if (row < 0 || row >= infoHeader.height) {
        cout << "Error: getColor(): rowが範囲外" << endl;
        return {0, 0, 0};
    }
    if (col < 0 || col >= infoHeader.width) {
        cout << "Error: getColor(): colが範囲外" << endl;
        return {0, 0, 0};
    }

    // 参考: http://coconut.sys.eng.shizuoka.ac.jp/bmp/
    int width = 3 * infoHeader.width;
    while (width % 4)  ++width;

    ColorPosition colorPos;

    colorPos.b = row * width + 3 * col;
    colorPos.g = colorPos.b + 1;
    colorPos.r = colorPos.b + 2;

    return colorPos;
}

/**
 * @fn width getter
 * @return width
 */
int BitmapManager::getWidth() {
    return infoHeader.width;
}

/**
 * @fn height getter
 * @return width
 */
int BitmapManager::getHeight() {
    return infoHeader.height;
}

/**
 * @fn 指定されたピクセルの色を取得
 * @param row 行
 * @param col 列
 * @return 色 (r, g, b)
 */
Color BitmapManager::getColor(int row, int col){
    //! 指定した行と列のデータ上の位置を取得
    ColorPosition pos = getColorPosition(infoHeader, row, col);

    // 色取得
    Color color;
    color.r = image[pos.r];
    color.g = image[pos.g];
    color.b = image[pos.b];

    return color;
}

/**
 * @fn 指定されたピクセルに色を指定
 * @param row 行
 * @param col 列
 * @param r 赤
 * @param g 緑
 * @param b 青
 */
void BitmapManager::setColor(int row, int col, int r, int g, int b) {
    //! 指定した行と列のデータ上の位置を取得
    ColorPosition pos = getColorPosition(infoHeader, row, col);

    // 色セット
    image[pos.r] = r;
    image[pos.g] = g;
    image[pos.b] = b;
}

FileHeader BitmapManager::getFileHeader(){
    return fileHeader;
}

void BitmapManager::setFileHeader(FileHeader FileHeader){
    this->fileHeader = fileHeader;
}

InfoHeader BitmapManager::getInfoHeader(){
    return infoHeader;
}

void BitmapManager::setInfoHeader(InfoHeader InfoHeader){
    this->infoHeader = infoHeader;
}

// 参考: http://program.station.ez-net.jp/special/handbook/cpp/class/copy.asp
void BitmapManager::copy(BitmapManager &src){
    // ヘッダーコピー
    fileHeader = src.getFileHeader();
    infoHeader = src.getInfoHeader();

    // すでにimageがあったら削除
    if (image != NULL) {
        delete[] image;
    }

    //! ヘッダーで定義されているデータサイズ
    int imageSize = infoHeader.dataSize;

    // インスタンス変数のポインタへ領域確保
    image = new uint8_t[imageSize];

    // コピー
    memcpy(image, src.image, sizeof(uint8_t) * imageSize);
}