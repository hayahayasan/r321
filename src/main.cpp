#include <M5Unified.h>
#include <Wire.h>
#include<SD.h>
#include <cctype>

#include <SdFat.h>
#include <USB.h>  
#include <FS.h>
#include <vector>    // std::vector を使用するために必要
#include <algorithm>
#include <map>      // std::mapを使用するため
#include <set>
#include <SPIFFS.h>
#include "shares.h"
#include <SPI.h> 
#include <sstream>
#include <algorithm>
// SdFatライブラリを使用

// SdFatオブジェクトを定義。M5Stack環境では、通常SPIバスが自動的に設定されます。

#pragma region <henssu>
int SCROLL_INTERVAL_FRAMES = 1;
int SCROLL_SPEED_PIXELS = 3;
int frameright;
int holdpositpointt;
String tttt = "hello";


String RESERVED_NAMES[] = {
    "CON", "PRN", "AUX", "NUL",
    "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
    "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
};
String optiontxt[6];
int frameleft;
bool btna;
int TABLE_IDd = 0;
MettDataMap dataToSaveE;
int holdimanopage;
bool beginizeSD = false;
const size_t BUFFER_SIZE = 4096;
const char* TABLE_NAME_PATTERN = "TABLE_NAME:";
// 識別子 "NEW_DATA_SET_PATTERN" の定義
const char* NEW_DATA_SET_PATTERN = "--- NEW DATA_SET ---";
const size_t PATTERN_LEN = strlen("TABLE_NAME:");
const size_t PATTERN_LEN1 = strlen("TABLE_NAME:");
const size_t PATTERN_LEN2 = strlen("--- NEW DATA_SET ---");
const int METT_CHUNK_SIZE = 1024;
bool btnc;
unsigned long lastTextScrollTime;
void  updatePointer2();
unsigned long TEXT_SCROLL_INTERVAL_MS;

int scrollPos;
//後で実装　ファイルのゴミ箱移動オプション
//ファイルコピペ時に、ペースト後「カットしますか」を出す
#pragma region <hensu>
String mainprintex = "M5Core3 LAN Activationer";
String sita = "";
String ggmode = "";
String sitagar[] = {"Net Status","Wifi","FLASHBrowser","SDBrowser","Configs","Options","SD Eject/Format","User Management","Log","Help/About"};
static bool sd_card_initialized = false; // SDカードが初期化されているか

// --- コピー操作キャンセルフラグ ---
volatile bool cancelCopyOperation = false;

int mainmode = 0;
int maindex = 0;
String maereposit = "";
int sizex = 2;
bool serious_error_flash = false;
int resercounter;
int address = 0;
int imano_page = 0;
int holdpositpointmax = 0;
int maxpage = 0;
int maxLinesPerPage = 0;
int maxLinesPerPageSave = 0;
int maxLinesPerPage2 = 0;
int maxLinesPerPage3 = 0;
int File_goukeifont = 3;
bool sderror = false;
int positpoint;
int holdpositpoint;
int positpointmax;
int positpointmaxg;
bool filebrat = false;
String g_copySourcePath; // コピー元パス (ファイルまたはフォルダ)
String g_destinationDirPath; // ペースト先ディレクトリのパス
bool g_isSourceFolder; // コピー元がフォルダであるかどうかのフラグ
int holdpositpointd = 0;
int holdimanopaged = 0;
int holdpositpointmaxd = 0;
String Tex2;
int pagemoveflag = 0;
String Filelist[100];

String directlist[100];
String ForDlist[100];
String DirecX="";
String lastArrowKeyInput = "NULL"; // 前のフレームで受け取った矢印キー入力（"UP", "DOWN", "LEFT", "RIGHT", "NULL"）
int arrowKeyHoldCounter = 0;       // 現在の矢印キーが押され続けているフレーム数
int arrowKeyRepeatCounter = 0;     // 連続実行が最後に発生してからのフレーム数
bool isArrowKeyRepeating = false;  // キーが現在連続実行状態であるか
int nullCount = 0; 
int lastDrawnCursorScreenX = -9999; 
int lastDrawnCursorScreenY = -9999;


String Textex = "!"; // 最下部にスクロール表示するテキスト
String Textex2 = "";
int scrollOffset = 0; // スクロールテキストの描画オフセット
int scrollFrameCounter = 0; // スクロールフレームカウンター

bool firstScrollLoop = false;
int LONG_PRESS_THRESHOLD_FRAMES = 50; // 長押しと判断するまでのフレーム数 (変更: 500 -> 50)
int REPEAT_INTERVAL_FRAMES = 30;      // 連続実行の間隔フレーム数 (変更: 400 -> 30)
int NULL_RESET_THRESHOLD = 3; 
int entryenter = 0; 
String copymotroot;
int positpointmain1;
bool copymotdir;
bool modordir;
static int frameCounter = 0;

struct SdEntryInfo {
  String name;
  bool isDirectory;
};


const int CURSOR_BLINK_INTERVAL = 10; // カーソル点滅のフレーム間隔
const int MAX_STRING_LENGTH = 65535; // SuperTに格納可能な最大文字数


#pragma endregion <henssu>
#pragma region <hensu2>

bool isStart = true;

bool redrawRequired = true; // 再描画が必要かどうかのフラグ
int lastValidIndex = 0;     // 最後に有効な項目のインデックス
// 追加するグローバル変数
String JJ = "Test JJ Text"; // JJの初期値
String currentPosDisplayText = ""; // 最下部に表示されるCurrentPosのテキスト
// 再描画最適化のための変数
String lastDrawnJj = ""; 
String lastDrawnCurrentPosText = "";

String AllName[100];
// 点滅関連のグローバル変数
unsigned long lastBlinkToggleTime = 0;
bool showAngleBrackets = true; // true: <X> を表示, false: X を表示 (<>なし)

// メインモード変数（テスト用に7に初期化）



#pragma endregion
#pragma region <funcdef>



#pragma endregion

//配列のNULL代入も作る
/**
 * @brief テーブル名が有効な形式であるか検証します。
 * * @param tableName 検証するテーブル名
 * @return bool 有効な場合はtrue
 */




String trimString(const String& s) {
    if (s.length() == 0) {
        return "";
    }
    const char* str = s.c_str();
    size_t first = 0;
    while (first < s.length() && (str[first] == ' ' || str[first] == '\t' || str[first] == '\n' || str[first] == '\r')) {
        first++;
    }
    if (first == s.length()) {
        return "";
    }
    size_t last = s.length() - 1;
    while (last > first && (str[last] == ' ' || str[last] == '\t' || str[last] == '\n' || str[last] == '\r')) {
        last--;
    }
    return s.substring(first, last - first + 1);
}







// M5Stackのセットアップ関数は削除されました。
// 通常、M5Stackの初期化はsetup()関数で行う必要がありますが、
// このコードが既存のプロジェクトの一部であり、
// 別の場所でM5Stackの初期化が行われている場合、この変更で問題ありません。
#pragma endregion <text input>
// 毎フレーム呼び出されるメインのテキスト入力処理関数
void textluck() {
    // M5.begin() など、M5Stackの初期化がここ以外で行われていることを前提とします。
    // I2C通信の初期化もここ以外で行われていることを前提とします。
    // 例: Wire.begin(); Wire.setClock(400000);
    M5.Lcd.setTextSize(3); // フォントサイズを3に設定
    M5.Lcd.setTextColor(WHITE, BLACK); // テキスト色を白、背景色を黒に設定

    String inputChar = wirecheck(); // wirecheck()を直接呼び出す

    int oldCursorIndex = cursorIndex; // カーソル位置の変更を検出するために保存

    // --- キー入力処理（長押し挙動なし） ---
    // "NULL"、"error"、"nokey"、"whattf" 以外の信号を有効なキー入力と見なす
    if (inputChar != "NULL" && inputChar != "error" && inputChar != "nokey" && inputChar != "whattf") {
        needsRedraw = true; // 有効な入力があれば再描画が必要
        if(inputChar == "TAB"){
          entryenter = 1;
          
          return;
        }
        if(inputChar == "ESC"){
          entryenter = 2;
          
          return;
        }
        if (inputChar == "ENT") {
            if (SuperT.length() < MAX_STRING_LENGTH) {
                SuperT = SuperT.substring(0, cursorIndex) + "\n" + SuperT.substring(cursorIndex);
                cursorIndex++;
            }
        } else if (inputChar == "BACK") {
            if (cursorIndex > 0) {
                SuperT = SuperT.substring(0, cursorIndex - 1) + SuperT.substring(cursorIndex);
                cursorIndex--;
            }
        } else if (inputChar == "SPACE") {
            if (SuperT.length() < MAX_STRING_LENGTH) {
                SuperT = SuperT.substring(0, cursorIndex) + " " + SuperT.substring(cursorIndex);
                cursorIndex++;
            }
        } else if (inputChar == "UP" || inputChar == "DOWN" || inputChar == "LEFT" || inputChar == "RIGHT") {
            // 矢印キーの場合、performArrowKeyActionを直接呼び出す
            performArrowKeyAction(inputChar);
        } else { // 通常の文字
            if (SuperT.length() < MAX_STRING_LENGTH) {
                SuperT = SuperT.substring(0, cursorIndex) + inputChar + SuperT.substring(cursorIndex);
                cursorIndex += inputChar.length();
            }
        }
    }
    
    // カーソルインデックスが範囲内にあることを確認
    if (cursorIndex < 0) cursorIndex = 0;
    if (cursorIndex > SuperT.length()) cursorIndex = SuperT.length();

    // カーソル位置のピクセル座標を計算し、スクロールオフセットを調整
    // adjustScroll()内でneedsRedrawが更新される可能性がある
    CursorPosInfo currentCursorInfo = calculateCursorPixelPos(cursorIndex, SuperT);
    cursorPixelX = currentCursorInfo.pixelX;
    cursorPixelY = currentCursorInfo.pixelY;
    adjustScroll(); // adjustScroll()がneedsRedrawをtrueに設定する可能性あり

    // カーソルインデックスが変更された場合、またはテキスト内容が変更された場合も再描画が必要
    if (cursorIndex != oldCursorIndex || (inputChar != "NULL" && inputChar != "error" && inputChar != "nokey" && inputChar != "whattf")) {
        needsRedraw = true;
    }

    // 再描画が必要な場合のみ画面をクリアし、テキストを描画
    if (needsRedraw) {
        // 通常のテキスト入力領域をクリア
        M5.Lcd.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height() - getFontHeight(), BLACK); 

        int currentDrawX = -offsetX;
        int currentDrawY = -offsetY;
        int charWidth = getCharWidth();
        int fontHeight = getFontHeight();
        int textInputAreaHeight = M5.Lcd.height() - getFontHeight(); // 通常のテキスト入力領域の高さ

        for (int i = 0; i < SuperT.length(); ++i) {
            char c = SuperT.charAt(i);

            if (c == '\n') {
                currentDrawX = -offsetX; // 新しい行の開始X座標はオフセットを考慮
                currentDrawY += fontHeight;
            } else {
                // 画面の水平方向と垂直方向の範囲内にある場合のみ描画
                // スクロールテキスト領域にかからないように描画範囲を制限
                if (currentDrawX + charWidth > 0 && currentDrawX < M5.Lcd.width() &&
                    currentDrawY + fontHeight > 0 && currentDrawY < textInputAreaHeight) { // 修正: textInputAreaHeightを使用
                    M5.Lcd.drawChar(c, currentDrawX, currentDrawY);
                }
                currentDrawX += charWidth;
            }
        }
        needsRedraw = false; // 描画が完了したのでフラグをリセット
    }

    // カーソルを描画 (常に表示)
    // 現在のカーソルのスクリーン座標を計算
    int currentCursorScreenX = cursorPixelX - offsetX;
    int currentCursorScreenY = cursorPixelY - offsetY;
    int textInputAreaHeight = M5.Lcd.height() - getFontHeight(); // 通常のテキスト入力領域の高さ

    // カーソルが通常のテキスト入力領域の範囲内にある場合のみ描画
    if (currentCursorScreenX >= 0 && currentCursorScreenX < M5.Lcd.width() &&
        currentCursorScreenY >= 0 && currentCursorScreenY < textInputAreaHeight) { // 修正: textInputAreaHeightを使用
        M5.Lcd.drawChar('|', currentCursorScreenX, currentCursorScreenY);
    }
    
    // 最後に描画したカーソル位置を更新
    lastDrawnCursorScreenX = currentCursorScreenX;
    lastDrawnCursorScreenY = currentCursorScreenY;

    // --- 最下部のスクロールテキスト描画 ---
    int scrollLineY = M5.Lcd.height() - getFontHeight(); // スクロールテキストのY座標

    // スクロールテキスト領域をクリア
    M5.Lcd.fillRect(0, scrollLineY, M5.Lcd.width(), getFontHeight(), BLACK);
    
    // スクロールロジック
    scrollFrameCounter++;
    if (scrollFrameCounter >= SCROLL_INTERVAL_FRAMES) {
        if (firstScrollLoop) {
            // 初回ループ時、テキストの左端から開始（scrollOffset は 0 または Textex.length()に応じて調整）
            // テキストが画面幅より短い場合はスクロール不要
            if (M5.Lcd.textWidth(Textex) > M5.Lcd.width()) {
                scrollOffset -= SCROLL_SPEED_PIXELS;
            } else {
                scrollOffset = 0; // 画面内に収まる場合は左端に固定
            }

            // テキストが完全に画面外に出たら、二回目以降のループへ
            if (scrollOffset < -M5.Lcd.textWidth(Textex)) {
                scrollOffset = M5.Lcd.width(); // 画面右端から再開
                firstScrollLoop = false;
            }
        } else {
            // 二回目以降のループ、右端から開始
            scrollOffset -= SCROLL_SPEED_PIXELS; // 左にスクロール
            if (scrollOffset < -M5.Lcd.textWidth(Textex)) {
                scrollOffset = M5.Lcd.width(); // 画面右端から再開
            }
        }
        scrollFrameCounter = 0;
    }

    // Textexが空でなければ描画
    if (Textex.length() > 0) {
        // スクロールオフセットを考慮して描画
        M5.Lcd.drawString(Textex, scrollOffset, scrollLineY);
    }
}



//#endregion
bool dexx = false;

//オプション:ファイルソート順番、ファイル拡張子デフォルト、書き込み方式





#pragma endregion

#pragma region <flashmonitor>

void listFlashContents(const String& path) {
  Serial.println("\n==============================================");
  Serial.printf("★★★ 内蔵フラッシュ閲覧: パス \"%s\" ★★★\n", path.c_str());
  Serial.println("==============================================");

  if (!SPIFFS.begin(false)) {
    Serial.println("エラー: SPIFFSの初期化に失敗しました。");
    return;
  }

  // 1. パスの正規化 (SPIFFSパスは必ず "/" から始まる)
  String normalizedPath = path;
  if (!normalizedPath.startsWith("/")) {
    normalizedPath = "/" + normalizedPath;
  }
  
  // フラッシュ全体の情報表示
  Serial.printf("全体サイズ: %d KB | 使用容量: %d KB\n", 
                 SPIFFS.totalBytes() / 1024, 
                 SPIFFS.usedBytes() / 1024);
  Serial.println("----------------------------------------------");

  // 2. 指定されたディレクトリのオープン
  File dir = SPIFFS.open(normalizedPath); 
  if (!dir) {
    Serial.printf("エラー: 指定されたディレクトリ '%s' のオープンに失敗しました。パスを確認してください。\n", normalizedPath.c_str());
    return;
  }
  
  // open() が成功しても、それがファイルである可能性があるため、isDirectory()で確認
  if (!dir.isDirectory()) {
    Serial.printf("エラー: '%s' はディレクトリではなくファイルです。\n", normalizedPath.c_str());
    dir.close();
    return;
  }

  // リストを格納する文字列
  String folderList = "--- フォルダ ---\n";
  String fileList = "--- ファイル ---\n";
  
  // 3. ディレクトリ内のすべてのエントリを走査
  while (File file = dir.openNextFile()) {
    
    // File.isDirectory() でフォルダかファイルを判定
    if (file.isDirectory()) {
      // フォルダの場合、名前を表示
      folderList += " [DIR] " + String(file.name()) + "\n";
    } else {
      // ファイルの場合、名前とサイズを表示
      fileList += " [FILE] " + String(file.name()) + " (" + String(file.size()) + " バイト)\n";
    }
    
    file.close(); // 開いたファイルを閉じる
  }

  // 4. 結果の表示
  Serial.println(folderList);
  Serial.println(fileList);
  
  dir.close(); // ディレクトリへの参照を閉じる
  
  Serial.println("==============================================");
}







#pragma endregion
//#region Text 1
void textexx() {
  if(mainmode == 1){
    return;
  }
  // 使いたいフォントサイズを指定（例: 2）
  int textSize = 2;

  // mainprintexの行数カウント（\nの数 + 1）
  int lineCount = 1;
  for (int i = 0; i < mainprintex.length(); i++) {
    if (mainprintex.charAt(i) == '\n') {
      lineCount++;
    }
  }

  // 1行の高さ = 8px × textSize
  int lineHeight = 8 * textSize;

  // mainprintexの表示領域高さ（px）
  int reservedHeight = lineCount * lineHeight;

  // 画面の下の部分、reservedHeight分はクリアしないようにする
  M5.Lcd.fillRect(0, reservedHeight, M5.Lcd.width(), M5.Lcd.height() - reservedHeight, BLACK);

  // mainprintexを上部に描画
  M5.Lcd.setTextSize(textSize);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print(mainprintex);

  // sita2を表示する領域はmainprintex領域の下から開始
  // sita2の表示用y座標
  int sitaStartY = reservedHeight;

  for (int i = 0; i < sita.length(); i++) {
    String sita2 = sita.substring(0, i + 1);

    // sita2の領域だけ一旦クリア
    M5.Lcd.fillRect(0, sitaStartY, M5.Lcd.width(), lineHeight, BLACK);

    // sita2描画
    M5.Lcd.setCursor(0, sitaStartY);
    M5.Lcd.print(sita2 );

    delay(50);
  }
}


#pragma region <potlist>




/**
 * @brief Initializes the SD card and creates a file at the specified path if it does not exist.
 * @param ss The full file path to initialize.
 */
bool initializeSDCard(String ss) {
    Serial.println("\n--- SD Card Initializing ---");
    if (!SD.begin()) {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.setTextSize(2);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("Error: SD Card init failed.");
        Serial.println("Error: SD Card init failed.");
        beginizeSD = false;
        return false;
    }
    beginizeSD = true;
    Serial.println("SD Card init success.");

    if (!SD.exists(ss)) {
        File file = SD.open(ss.c_str(), FILE_WRITE);
        if (file) {
            Serial.printf("Info: New file created: %s\n", ss.c_str());
            file.close();
            return true;
        } else {
            Serial.printf("Error: Failed to create file: %s\n", ss.c_str());
            return false;
        }
    } else {
        Serial.printf("Info: File already exists: %s\n", ss.c_str());
        return true;
    }
}
// --- ポインターの変動と画面更新を行う関数 ---
void updatePointerAndDisplay(int ril) {
    bool pointerChanged = false;

    if (ril == 1) { // BtnAが押された場合
        currentPos--;
        if (currentPos < 0) {
            currentPos = numMenuItems - 1;
        }
        pointerChanged = true;
    } else if (ril == 2) { // BtnCが押された場合
        currentPos++;
        if (currentPos >= numMenuItems) {
            currentPos = 0;
        }
        pointerChanged = true;
    }

    if (millis() - lastBlinkToggleTime >= 50) {
        showAngleBrackets = !showAngleBrackets;
        lastBlinkToggleTime = millis();
        pointerChanged = true;
    }

    if (ril != 0) {
        Serial.printf("[DEBUG] updatePointerAndDisplay: Button pressed. ril: %d, New currentPos: %d, Item: '%s'\n", ril, currentPos, potlist[currentPos].c_str());
    }

    if (pointerChanged) {
        M5.Lcd.setTextSize(3); // 描画前に必ずテキストサイズを設定
        drawTopText(showAngleBrackets);
        drawCenterText();
        drawBottomText();
    }
    
    if (ril != 0) { 
        lastBlinkToggleTime = millis(); 
        showAngleBrackets = true;
    }
}




#pragma endregion





# pragma region <Metload>

// Struct to hold variable information extracted from a metadata file


// Map type for saving/loading metadata
// Key: variable name, Value: value string


// Global variables
const int itemsPerPage = 8; // Number of items to display on a page
std::vector<String> allTableNames;



/**
 * @brief Creates or appends a metadata file at the specified path and saves variables from a map.
 * @param fs The SD card filesystem object.
 * @param fullFilePath The full path of the file to save.
 * @param tableName The name of the table to save within the file.
 * @param data The reference to the MettDataMap to save.
 * @param isError Reference to a boolean that will be set to true if an error occurs, false otherwise.
 */
void saveMettFile(fs::FS &fs, const String& fullFilePath, const String& tableName, const MettDataMap& data, bool& isError) {
    isError = false;

    auto getDirFromPath = [](const String& fullPath) -> String {
        int lastSlash = fullPath.lastIndexOf('/');
        if (lastSlash > 0) {
            return fullPath.substring(0, lastSlash);
        }
        return "/"; 
    };

    auto createDirIfNotExists = [&](const String& dirname) -> bool {
        // SD::exists と fs.mkdir が利用可能であることを前提とする
        if (SD.exists(dirname.c_str())) {
            return true;
        }
        Serial.printf("Debug: Creating directory: %s\n", dirname.c_str());
        if (fs.mkdir(dirname.c_str())) {
            return true;
        } else {
            Serial.printf("Error: Failed to create directory: %s\n", dirname.c_str());
            return false;
        }
    };

    if (!fullFilePath.startsWith("/") || fullFilePath.lastIndexOf('.') == -1 || fullFilePath.substring(fullFilePath.lastIndexOf('.')) != ".mett" || containsInvalidTableNameChars(tableName)) {
        Serial.println("Error: Basic save validation failed.");
        isError = true;
        return;
    }

    String dirPath = getDirFromPath(fullFilePath);
    if (!createDirIfNotExists(dirPath)) {
        isError = true;
        return;
    }

    // ファイルを上書きモードでオープン (FILE_WRITE)
    // 既存のデータをロードしてマージするロジックはここにはありません。
    File file = fs.open(fullFilePath.c_str(), FILE_WRITE);
    if (!file) {
        Serial.printf("Error: Failed to open file for writing (overwrite mode): %s\n", fullFilePath.c_str());
        isError = true;
        return;
    }

    size_t bytesWritten;
    
    // 🌟 修正: NEW DATA SET ヘッダーを TABLE_ID に変更し、TABLE_NAMEを追記
    if (file.println(TABLE_IDd) == 0) { isError = true; }
    String tableNameLine = "TABLE_NAME:" + tableName + "\n";
    if (file.print(tableNameLine) == 0) { isError = true; }

    // 変数データの書き込み
    for(const auto& pair : data) {
        String varName = pair.first;
        if (containsInvalidVariableNameChars(varName)) {
            Serial.printf("Warning: Variable name '%s' contains invalid chars. Skipping.\n", varName.c_str());
            isError = true; 
            continue;
        }
        
        String valueStr = pair.second;
        String dataType = inferDataType(valueStr);
        String lineToWrite = String(varName.c_str()) + ":" + dataType.c_str() + ":" + valueStr.c_str() + "\n";
        
        if (file.print(lineToWrite) == 0) {
            Serial.printf("Error: Failed to write variable '%s'.\n", varName.c_str());
            isError = true; 
        }
    }
    
    // 最終的な空行
    if (file.println() == 0) { isError = true; }

    file.close();
    if (!isError) {
        // ログを上書き保存に変更
        Serial.printf("Info: File saved successfully (OVERWRITTEN): %s (Table: %s)\n", fullFilePath.c_str(), tableName.c_str());
    }
}
/**
 * @brief Mettファイル内の古いテーブル名を、一時ファイル方式でメモリ安全に新しい名前に置き換えます。
 * * 元のファイルを全量SRAMに読み込む代わりに、一時ファイルを作成し、行単位で読み書きをストリーミングします。
 * これにより、SRAM上限を超える巨大なファイルでも安定してテーブル名を書き換えることが可能です。
 * * @param fs SDカードまたはSPIFFSなどのファイルシステムオブジェクト
 * @param fullFilePath リネーム対象のファイルパス
 * @param oldTableName 古いテーブル名
 * @param newTableName 新しいテーブル名
 * @param isError 処理中にエラーが発生したかを示すフラグ (出力)
 * @return bool 処理の試行が成功したかどうか (ファイルが見つからない、書き込みエラーなど)
 */



void loadMettFile(fs::FS &fs, const String& fullFilePath, const String& targetTableName, bool& success, bool& isEmpty, std::vector<MettVariableInfo>& variables) {
    variables.clear();
    success = false;
    isEmpty = true; 

    if (!fullFilePath.startsWith("/") || fullFilePath.lastIndexOf('.') == -1 || fullFilePath.substring(fullFilePath.lastIndexOf('.')) != ".mett") {
        Serial.printf("Error (SD Load): Invalid file path or extension: %s\n", fullFilePath.c_str());
        return;
    }
    
    File file = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!file) {
        Serial.printf("Error (SD Load): Failed to open file for reading: %s\n", fullFilePath.c_str());
        return;
    }

    if (file.size() == 0) {
        file.close();
        // size 0 の場合は success=false, isEmpty=true のままでリターン
        return;
    }

    // IMPORTANT CHANGE: ファイルサイズが0より大きい場合でも、まだ有効なテーブルが
    // 読み込まれていないため、isEmpty は true のまま維持する。
    
    Serial.printf("Info (SD Load): Loading file (Chunked Process): %s (Target Table(s): %s)\n", fullFilePath.c_str(), targetTableName.isEmpty() ? "All" : targetTableName.c_str());

    // --- ターゲットテーブル名解析 ---
    std::vector<String> targetTableList;
    if (!targetTableName.isEmpty()) {
        String tempTableName = targetTableName;
        int commaIndex;
        while ((commaIndex = tempTableName.indexOf(',')) != -1) {
            String singleTarget = trimString(tempTableName.substring(0, commaIndex));
            if (!singleTarget.isEmpty()) targetTableList.push_back(singleTarget);
            tempTableName = tempTableName.substring(commaIndex + 1);
        }
        String singleTarget = trimString(tempTableName);
        if (!singleTarget.isEmpty()) targetTableList.push_back(singleTarget);
    }
    
    String currentTableNameInFile = "";
    bool shouldLoadCurrentTable = targetTableList.empty(); 

    uint8_t buffer[METT_CHUNK_SIZE]; 
    String remainder = "";
    size_t bytesRead;

    // --- チャンク単位のファイル読み込みと行処理 ---
    while ((bytesRead = file.read(buffer, METT_CHUNK_SIZE))) {
        String chunk((char*)buffer, bytesRead);
        String combinedData = remainder + chunk;
        remainder.clear();
        int lastNewlinePos = combinedData.lastIndexOf('\n'); 

        if (lastNewlinePos != -1) {
            String linesToProcess = combinedData.substring(0, lastNewlinePos);
            remainder = combinedData.substring(lastNewlinePos + 1);

            int currentPos = 0;
            while (currentPos < linesToProcess.length()) {
                int nextNewline = linesToProcess.indexOf('\n', currentPos); 
                String line;
                if (nextNewline == -1) {
                    line = linesToProcess.substring(currentPos);
                    currentPos = linesToProcess.length();
                } else {
                    line = linesToProcess.substring(currentPos, nextNewline); 
                    currentPos = nextNewline + 1;
                }
                
                line = trimString(line);

                if (line.startsWith("#") || line.isEmpty()) continue;
                
                // ブロックマーカーのチェック (テーブルブロックの終了/開始)
                if (line.startsWith(String(TABLE_IDd))) {
                    currentTableNameInFile = "";
                    shouldLoadCurrentTable = targetTableList.empty();
                    continue;
                }
                
                // TABLE_NAME の検出
                if (line.startsWith("TABLE_NAME:")) {
                    int colonIndex = line.indexOf(':');
                    currentTableNameInFile = (colonIndex != -1) ? trimString(line.substring(colonIndex + 1)) : "";
                    
                    if (containsInvalidTableNameChars(currentTableNameInFile)) {
                        Serial.printf("Warning: Table name '%s' contains invalid chars. (Skipping table variables)\n", currentTableNameInFile.c_str());
                        currentTableNameInFile = "";
                        shouldLoadCurrentTable = false;
                        continue;
                    }
                    
                    // ターゲットテーブルかどうかの判定
                    shouldLoadCurrentTable = targetTableList.empty() || (std::find(targetTableList.begin(), targetTableList.end(), currentTableNameInFile) != targetTableList.end());
                    continue;
                }

                if (!shouldLoadCurrentTable || currentTableNameInFile.isEmpty()) continue;
                
                // 変数行のパース (VAR_NAME:DATA_TYPE:VALUE_STRING)
                int firstColonIndex = line.indexOf(':');
                int secondColonIndex = line.indexOf(':', firstColonIndex + 1);
                
                if (firstColonIndex == -1 || secondColonIndex == -1) {
                    Serial.printf("Warning: Invalid mett line format: %s\n", line.c_str());
                    continue;
                }
                
                MettVariableInfo varInfo;
                varInfo.variableName = trimString(line.substring(0, firstColonIndex));
                varInfo.dataType = trimString(line.substring(firstColonIndex + 1, secondColonIndex));
                varInfo.valueString = trimString(line.substring(secondColonIndex + 1));
                varInfo.tableName = currentTableNameInFile;
                
                if (containsInvalidVariableNameChars(varInfo.variableName)) {
                    Serial.printf("Warning: Variable name '%s' contains invalid chars. Skipping.\n", varInfo.variableName.c_str());
                    continue;
                }
                variables.push_back(varInfo);
            }
        } else {
            remainder = combinedData;
        }
    }

    // ファイル終端処理: remainder に残った最後の行を処理
    if (!remainder.isEmpty()) {
        String line = trimString(remainder);
        
        if (!line.isEmpty() && !line.startsWith("#") && shouldLoadCurrentTable) {
             int firstColonIndex = line.indexOf(':');
             int secondColonIndex = line.indexOf(':', firstColonIndex + 1);
             if (firstColonIndex != -1 && secondColonIndex != -1) {
                 MettVariableInfo varInfo;
                 varInfo.variableName = trimString(line.substring(0, firstColonIndex));
                 varInfo.dataType = trimString(line.substring(firstColonIndex + 1, secondColonIndex));
                 varInfo.valueString = trimString(line.substring(secondColonIndex + 1));
                 varInfo.tableName = currentTableNameInFile;

                 if (!containsInvalidVariableNameChars(varInfo.variableName)) {
                     variables.push_back(varInfo);
                 } else {
                     Serial.printf("Warning: Variable name '%s' contains invalid chars (End of File).\n", varInfo.variableName.c_str());
                 }
             } else {
                 Serial.printf("Warning: Invalid mett line format (End of File): %s\n", line.c_str());
             }
        }
    }

    file.close();
    
    // --- 最終ステータス設定 ---
    success = true; 
    
    // ユーザーの要望に従い、ファイルは開けたが有効なデータ（テーブル）がなかった場合は
    // isEmptyをtrueのままにする。
    isEmpty = variables.empty(); 
    
    Serial.printf("Info (SD Load): Mett file loaded successfully (isEmpty: %s, Loaded Variables: %d)\n", isEmpty ? "true" : "false", variables.size());
}

#pragma endregion

bool loadmett(){
    // SDカード上の全`.mett`ファイルをスキャンしてデータを抽出
    Serial.println("\n--- 全.mettファイルのデータ抽出 ---");
    std::vector<FileMettData> allExtractedData = scanAndExtractMettData(SD, "/data");
    printFileM(allExtractedData);
    
    std::vector<MettVariableInfo> loadedVariables;
    printTable("/save/save1.mett", "TestOpt1", loadedVariables);
    
    // 特定のファイルをロードしてデータを抽出
    Serial.println("\n--- 特定のファイルのデータロード ---");
    bool loadSuccess = false;
    // loadMettFileのisEmptyは「ファイルが見つからなかった場合にtrue」となります。
    bool fileIsMissing = false; 
    
    // 1. 最初のロード試行
    loadMettFile(SD, "/save/save1.mett", "TestOpt1", loadSuccess, fileIsMissing, loadedVariables);

    // loadSuccess が true ならファイルは存在し、ロードに成功した
    if (loadSuccess) {
        // ロード成功後の処理
        
        // 1.1. ファイルが存在するが、データセット (TestOpt1) の変数が一つもない場合、初期化とみなす
        if (loadedVariables.empty()) { 
             // ログの表現をより正確に修正
             Serial.println("Info: ファイルは存在しますが、データセット (TestOpt1) に変数がありません。初期データを保存します。");
             
             // 初期データを構築
             MettDataMap dataToSave;
             dataToSave["file_ext"] = "txt";
             dataToSave["stringtype"] = "unicode";
             dataToSave["sorttype"] = "nameasc";
             dataToSave["onlinetype"] = "only pass";
             std::vector<int> sensorInts = {1, 2, 3, -132};
             dataToSave["sensorInts_test"] = joinVectorToString(sensorInts);
             Serial.printf("Info: Saving IntArray: %s\n", dataToSave["sensorInts_test"].c_str());
             
             bool isError = false; // フラグ名をisErrorに変更
             saveMettFile(SD, "/save/save1.mett", "TestOpt1", dataToSave, isError); 

             // 修正: isError が true なら失敗、false なら成功
             if(!isError){ 
                 Serial.println("Info: 初期データを保存しました。");
                 // 初期値で optiontxt を更新
                 optiontxt[0] = "txt";
                 optiontxt[1] = "unicode";
                 optiontxt[2] = "nameasc";
                 optiontxt[3] = "only pass";
                 return true;
             }else{
                 Serial.println("Error: 初期データの保存に失敗しました。");
                 return false;
             }
        }
        
        // 1.2. データが存在し、ロードに成功した場合 (既存のデータを反映)
        
        Serial.println("Info: ロード成功。既存のデータを処理します。");

        // ロードしたデータから値を取得し、optiontxtに反映
        // getVariableString を使用
        optiontxt[0] = getVariableString(loadedVariables, "file_ext");
        optiontxt[1] = getVariableString(loadedVariables, "stringtype");
        optiontxt[2] = getVariableString(loadedVariables, "sorttype");
        optiontxt[3] = getVariableString(loadedVariables, "onlinetype");
        std::vector<int> sensorInts = getVariableIntArray(loadedVariables, "sensorInts_test");
        
        Serial.printf("Info: Loaded file_ext: %s\n", optiontxt[0].c_str());
        Serial.printf("Info: Loaded sensorInts_test: %s\n", joinVectorToString(sensorInts).c_str());
        
        return true;

    } else {
        // ロード失敗 (fileIsMissing == true) = ファイルが存在しない
        Serial.println("Error: ファイルのロードに失敗しました (ファイルが存在しない可能性があります)。初期データを保存します。");
        
        // 初期データを構築
        MettDataMap dataToSave;
        dataToSave["file_ext"] = "txt";
        dataToSave["stringtype"] = "unicode";
        dataToSave["sorttype"] = "nameasc";
        dataToSave["onlinetype"] = "only pass";
        std::vector<int> sensorInts = {1, 2, 3, -132};
        dataToSave["sensorInts_test"] = joinVectorToString(sensorInts);
        Serial.printf("Info: Saving IntArray: %s\n", dataToSave["sensorInts_test"].c_str());
        
        bool isError = false; // フラグ名をisErrorに変更
        // ファイルが存在しない場合のみ saveMettFile を実行
        saveMettFile(SD, "/save/save1.mett", "TestOpt1", dataToSave, isError); 
        
        // 修正: isError が true なら失敗、false なら成功
        if(!isError){
            Serial.println("Info: 初期データを保存しました。");
            // 初期値で optiontxt を更新
            optiontxt[0] = "txt";
            optiontxt[1] = "unicode";
            optiontxt[2] = "nameasc";
            optiontxt[3] = "only pass";
            return true;
        }else{
            Serial.println("Error: 初期データの保存に失敗しました。");
            return false;
        }
    }
}

void shokaipointer2(int pageNum, String filePath  ) {
  M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.println("Loading...");
    
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    frameright = 0;
    positpoint = 0;
    M5.Lcd.setTextSize(3);
    frameleft = 0;
    // Get all table names from a single file
    bool tt = false;
    allTableNames = getAllTableNamesInFile(SD, filePath,tt);

    if (allTableNames.empty()) {
        M5.Lcd.fillScreen(BLACK);
        Serial.println("No tables found.");
        maxLinesPerPage = -1;
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextFont(3);
      M5.Lcd.println("No Tables!\n Press B");
        return;
    }

    int totalItems = allTableNames.size();
    int totalPages = (totalItems + itemsPerPage - 1) / itemsPerPage;
    
      
    
      if (pageNum < 0 || pageNum >= totalPages) {
         M5.Lcd.fillScreen(BLACK);
        Serial.println("Invalid page.");
        M5.Lcd.setCursor(0, M5.Lcd.height() - 20);
        M5.Lcd.setTextSize(2);
        M5.Lcd.printf("Page: %d/%d", 1, totalPages);
        return;
    }

    // Calculate positpointmax based on the new logic
    int remainingItems = totalItems % itemsPerPage;
    if (pageNum == totalPages - 1) {
        if (remainingItems == 0) {
            positpointmax = itemsPerPage ;
            positpointmaxg = itemsPerPage;
        } else {
            positpointmax = remainingItems ;
            positpointmaxg = itemsPerPage;
        }
    } else {
        positpointmax = itemsPerPage ;
    }
    Serial.printf("Debug: positpointmax = %d\n", positpointmax); // Debugging line
    // Use positpointmax for the loop
    int start = pageNum * itemsPerPage;
    int end = start + positpointmax;
    if(totalItems % itemsPerPage == 0){
      maxLinesPerPage = totalItems / itemsPerPage;
    }else{
      maxLinesPerPage = (totalItems / itemsPerPage) + 1;
    }
    
    Serial.println("start" + String(start) + "end" + String(end) + "patge" + String(imano_page));
    M5.Lcd.setCursor(0, 0);
    for(int i = 0; i < 100; i++){
      AllName[i] = "";
    }
    int ii = 0;
    M5.Lcd.fillScreen(BLACK);
    for (int i = start; i < end; ++i) {
        M5.Lcd.println("  " + allTableNames[i]);
        AllName[ii] = allTableNames[i];
        ii++;
    }
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, M5.Lcd.height() - 20);
    M5.Lcd.printf("Page: %d/%d", pageNum + 1, totalPages);
    }
    

void optkobun(){
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println(String("  FlashBU Loc:...") 
            + "\n  tabletype:" + dataToSaveE["table_opt2"] +
             "\n  tag:...\n\n table options!");
          positpoint = 0;
          maxpage = -1;
          imano_page = 0;
          positpointmax = 3;
          mainmode = 16;
          return;
}

void opt1_kaimei(){
  SuperT = dataToSaveE["table_opt1"];
  Textex = "Enter file location.";
  while(true){
    textluck();
    delay(1);
    if(entryenter == 1){
      entryenter = 0;
      if(isValidTableName(SuperT,AllName,101)){
        dataToSaveE["table_opt1"] = SuperT;
        bool sus = false;
        saveMettFile(SD, DirecX + ggmode, AllName[positpoint], dataToSaveE, sus);
        if(sus){
          kanketu("Set Success!",500);
        }else{
          kanketu("Set Failed!",500);
        }
        optkobun();

        return;
      }else{
        Textex = "Invalid Name!";
      }
    }else if(entryenter == 2){
      entryenter = 0;
      M5.Lcd.fillScreen(BLACK);
              optkobun();
          return;
    }
  }
}



void setup() {
  
  TEXT_SCROLL_INTERVAL_MS = 40; 
  auto cfg = M5.config();
  Serial.begin(115200);
  lastTextScrollTime = 0;
  SCROLL_SPEED_PIXELS = 4;
  M5.begin();
  frameleft = 1;
  frameright = 1;
 
  scrollPos = M5.Lcd.width();
  Serial.println("M5Stack initialized");

    Wire.begin(); 
  Wire.setClock(400000);
  
  // SDカードの初期化を試みます
  // 起動時にSDカードが存在しない場合でも、sdcmode()が繰り返し初期化を試みます。
  btna = false;
  btnc = false;

  

  // 文字のサイズと色を設定（小さめで表示）
  M5.Lcd.setTextSize(sizex);
  M5.Lcd.setTextColor(WHITE, BLACK); // 白文字、黒背景
  M5.Lcd.fillScreen(BLACK); // 画面全体を黒でクリア
  // 左上すれすれ (0,0) に表示
  M5.Lcd.setCursor(0, 0);
  sita = tttt;
  textexx();
 
  wirecheck();
  mainmode = 0;
   

  
  // USB接続/切断コールバックを設定


  

}
//後でファイル名作成時の拡張子オプションロード追加
//ログ機能の追加．ログ追加後，メニューから一発でテーブル編集に飛ぶ機能，つまりファイルのお気に入り指定の追加
void loop() {
  M5.update(); // ボタン状態を更新
 delay(1);//serial.println暴走対策,Allname[positpoint]はテーブル名
if(mainmode == 16){
    updatePointer2();
    if(pagemoveflag == 2){
      pagemoveflag = 0;
      return;
    }else if(pagemoveflag == 1){
      pagemoveflag = 0;
      return;
    }else if((pagemoveflag == 5) ){
      pagemoveflag = 0;
      positpoint = holdpositpoint;
      imano_page = holdimanopage;
      mainmode = 13;
      M5.Lcd.fillScreen(BLACK);
      positpointmax = 5;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;

      return;
    
    }else if(M5.BtnB.wasPressed()){
        if(positpoint == 0){
          M5.Lcd.fillScreen(BLACK);
          SCROLL_INTERVAL_FRAMES = 1;
        SCROLL_SPEED_PIXELS = 3;
        firstScrollLoop = true;
          opt1_kaimei();
          M5.Lcd.fillScreen(BLACK);
          optkobun();
          return;
        }else if(positpoint == 1){
          M5.Lcd.fillScreen(BLACK);
          String opt22[4] = {"normal","readonly","oncewrite"};//oncewriteは空白セルまたはデフォルトに1回しか書き込めない
          selectOption(opt22,4,"select option!","read options!");
          M5.Lcd.fillScreen(BLACK);
          optkobun();
          return;
        }
    }
}

 else if(mainmode == 15){
  delay(1);
    textluck();
    if(entryenter == 2){//back
      entryenter = 0;
      positpoint = holdpositpoint;
      imano_page = holdimanopage;
      positpointmax = holdpositpointmax;
      mainmode = 12;
      M5.Lcd.fillScreen(BLACK);
      positpointmax = 5;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      return;
    }else if(entryenter == 1){//enter
      entryenter = 0;
      if(isValidTableName(SuperT,AllName,101)){
        MettDataMap dataToSave;
        bool loadSuccess = false;
        Serial.println("fs" + String(holdpositpointt));
        if(holdpositpointt == 1){//create
          

        saveMettFile(SD, DirecX + ggmode, SuperT, dataToSave, loadSuccess);
        }else if (holdpositpointt == 2){//rename
          renameTableInMettFile(SD, DirecX + ggmode, AllName[holdpositpoint], SuperT, loadSuccess);
        }
        
        

        if(loadSuccess){
          Textex = "Save/Rename Error!";
        }else{
          kanketu("Create Success!",500);
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0,0);
          M5.Lcd.setTextSize(3);
          M5.Lcd.println("Loading...");
        positpoint = holdpositpoint;
        imano_page = holdimanopage;
          mainmode = 12;
          positpoint = holdpositpoint;
          imano_page = holdimanopage;
          holdpositpointmax = holdpositpointmax;
          shokaipointer2(holdimanopage,DirecX + ggmode);
          maxpage = maxLinesPerPage;
          return;
        }
      }else{
        Textex = "Invalid Name!";
      }
    }
}

//テーブルオプション一覧
//フラッシュファイル空のバックアップ元変更String
//読み込み方式(通常，readonly,appendonly)
//形式(int,String,boolean,float,double,Char,date)
else if(mainmode == 14){
  updatePointer2();
  if(pagemoveflag == 2){
      pagemoveflag = 0;
      return;
    }else if(pagemoveflag == 1){
      pagemoveflag = 0;
      return;
    }else if((pagemoveflag == 5) ){
      pagemoveflag = 0;
      positpoint = holdpositpoint;
      imano_page = holdimanopage;
      mainmode = 12;
      M5.Lcd.fillScreen(BLACK);
      positpointmax = 5;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;

      return;
    
    }else if(M5.BtnB.wasPressed()){
      
      if(positpoint == 3){//Delete

          if(positpointmax == 0){
            kanketu("No Tables!", 500);
            imano_page = holdimanopage;
      mainmode = 12;
      M5.Lcd.fillScreen(BLACK);
      
      imano_page = 0;
      positpoint = 0;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      return;
          }
          bool tt = areusure();
          if(tt){
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0,0);
            M5.Lcd.println("Deleting...");
            if(deleteTableInFile(SD, DirecX + ggmode, AllName[holdpositpoint])){
              kanketu("Delete Success!",500);
              imano_page = holdimanopage;
      mainmode = 12;
      M5.Lcd.fillScreen(BLACK);
      
      imano_page = 0;
      positpoint = 0;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      return;

            }else{
              kanketu("Delete Failed!",500);
              positpoint = holdpositpoint;
      imano_page = holdimanopage;
      mainmode = 12;
      M5.Lcd.fillScreen(BLACK);
      
      positpoint = holdpositpoint;
      imano_page = holdimanopage;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      return;
            }

          }  else{
            positpoint = holdpositpoint;
      imano_page = holdimanopage;
      mainmode = 13;
      M5.Lcd.fillScreen(BLACK);
      positpointmax = 5;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      return;
          }

      }    
      else if(positpoint == 1 || positpoint == 2){//Create or Rename
        bool tt = areusure();
        holdpositpointt = positpoint;
        if(tt){
          M5.Lcd.fillScreen(BLACK);
          entryenter = false;
        SuperT="";
        SCROLL_INTERVAL_FRAMES = 1;
        SCROLL_SPEED_PIXELS = 3;
        firstScrollLoop = true;
        
        cursorIndex = 0;
        Textex = "If you wanna end,press tab key. No return key!";
          mainmode = 15;
          return;

        }else{
          positpoint = holdpositpoint;
      imano_page = holdimanopage;
      mainmode = 13;
      M5.Lcd.fillScreen(BLACK);
      positpointmax = 5;
      Textex = "Save/Rename Error!";
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      return;
        }
      }else if(positpoint == 4){//options
        M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.setTextSize(3);
      holdpositpoint = positpoint;
      holdimanopage = imano_page;

      bool loadSuccess = false;
    bool fileIsEmpty = false;
    std::vector<MettVariableInfo> loadedVariables;
    M5.Lcd.fillScreen(BLACK);   
    M5.Lcd.println("loading...");
    loadMettFile(SD, DirecX + ggmode, AllName[positpoint], loadSuccess, fileIsEmpty, loadedVariables);
       if(loadSuccess){
        
        dataToSaveE = copyVectorToMap(loadedVariables);
        bool jj = false;

        if(datt("table_opt1","/")){
          jj = true;
        }
        if(datt("table_opt2","normal")){
          jj = true;
        }
        if(datt("table_opt3","")){
          jj = true;
        }
        Serial.println("DD!" + dataToSaveE["table_opt1"]);
        if(jj){
          saveMettFile(SD, DirecX + ggmode, AllName[positpoint], dataToSaveE, loadSuccess);
          if(loadSuccess){
            kanketu("Option Saved!",200);
          }else{
           kanketu("Option Save Failed!",200);
          }
        }
          
        
        optkobun();
          return;


        }
       else{
          kanketu("Option Load Failed!",500);
          positpoint = holdpositpoint;
      imano_page = holdimanopage;
      mainmode = 13;
      M5.Lcd.fillScreen(BLACK);
      positpointmax = 5;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      return;
       }
      }
      
    }
}
else if(mainmode == 13){
  if(maxLinesPerPage != -1){
        updatePointer2();

    if(pagemoveflag == 1){
      pagemoveflag = 0;
      imano_page = 0;
      positpoint = 0;
      Serial.println("fefe1!");
      shokaipointer2(imano_page,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      return;
    }else if(pagemoveflag == 2){
      pagemoveflag = 0;
      imano_page = imano_page + 1;
      positpoint = 0;
      Serial.println("fefe2!");
      shokaipointer2(imano_page,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      return;
    }else if(pagemoveflag == 3){
      pagemoveflag = 0;
      imano_page = imano_page - 1;
      
      positpoint = positpointmaxg - 1;
      shokaipointer2(imano_page,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      return;
    }else if (pagemoveflag == 4){
      pagemoveflag = 0;
      imano_page = 0;
      positpoint = holdpositpoint;
      imano_page = holdimanopage;
      mainmode = 1;
      M5.Lcd.fillScreen(BLACK);
      // SDカードコンテンツの初期表示
      shokaipointer();
      return;
    }else if(M5.BtnB.wasPressed()){
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.setTextSize(3);
      holdpositpoint = positpoint;
      holdimanopage = imano_page;
      M5.Lcd.println("  Open\n  Create\n  Rename\n  Delete\n  TableOptions\n  Back\n  Export to FLASH" );
      positpoint = 0;
      positpointmax = 7;
      maxpage = -1;
      mainmode = 14;
      return;

    }
  }else{
    if(M5.BtnB.wasPressed()){
      bool tt = areubunki("Create Table","Back");
      if(tt){
        M5.Lcd.fillScreen(BLACK);
          entryenter = false;
          positpoint = 1;
        SuperT="";
        SCROLL_INTERVAL_FRAMES = 1;
        SCROLL_SPEED_PIXELS = 3;
        firstScrollLoop = true;
        
        cursorIndex = 0;
        Textex = "If you wanna end,press tab key. No return key!";
          mainmode = 15;
          return;
      }else{
        M5.Lcd.fillScreen(BLACK);
        mainmode = 1;
        positpoint = 0;
        holdpositpoint = 0;
        
        imano_page = 0;
        frameright  = 1;
        frameleft = 1;
        shokaipointer();
        return;
      }
    }
  }

}
 else if(mainmode == 12){
    if(M5.BtnA.wasPressed()){
      M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = 0;
        mainmode = 1;
        positpointmax = holdpositpointmaxd;
        imano_page = holdimanopaged;
        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
    }
    if(ggmode.endsWith(".mett") && M5.BtnB.wasPressed()){
      M5.Lcd.fillScreen(BLACK);
      if(!checkSDCardOnly){
        kanketu("SD card not found!",500);
        M5.Lcd.fillScreen(BLACK);
        mainmode = 1;
        positpoint = 0;
        holdpositpoint = 0;
        
        imano_page = 0;
        frameright  = 1;
        frameleft = 1;
        shokaipointer();
        return;
      }


      mainmode = 13;
      M5.Lcd.setCursor(0,0);
      M5.Lcd.setTextSize(3);
      M5.Lcd.println("Loading...");
      Serial.println("fe" + DirecX + ggmode);
      shokaipointer2(0,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      Serial.println("sus" + String(maxpage));
      return;
    }
 }
 #pragma region <optmodee>//0=拡張子 1=文字コード 2=ソート 3=オンラインタイプ
 


#pragma endregion  

else if (mainmode == 0) { // メニューモードの場合

    String key = wirecheck(); // wirecheck()は常に呼び出される
    //これを入れないとmainmode変数認識が遅れやすい
    // ボタンAが押された場合の処理 (メニューを上に移動)

    if (M5.BtnA.wasPressed() ) {
      if (maindex == 0) {
       maindex = sizeof(sitagar) / sizeof(sitagar[0]);
      } else {
        maindex--;
      }
      sita = sitagar[maindex]; // 選択されたオプションを更新
      textexx(); // メニュー画面を再描画
    }

    // ボタンBが押された場合の処理 (アクション実行、例: SDリスト表示へ移行)
    if (M5.BtnB.wasPressed()) {
      if (maindex == 3) { // 例: maindex 3 がSDリスト表示のオプションの場合
        M5.Lcd.fillScreen(BLACK); // 画面をクリア
        mainmode = 1; // モードをSDリスト表示モードに切り替え
        imano_page = 0;
        DirecX = "/";
        copymotroot = "";
        shokaipointer();
        return;//mainmode0フラグ誤作動対策
      } else  if(maindex == 4){
        if (!SD.begin(GPIO_NUM_4, SPI, 20000000)) {//SDカード入ってない
          serious_errorsd = true;
          kanketu("No SD Card!",500);
          mainmode = 0;
          beginizeSD = false;
          M5.Lcd.setCursor(0, 0);
        sita = "hello";
        textexx();
        positpoint = 0;
        holdpositpoint = 0;
        imano_page = 0;
          

          return;
        }
        beginizeSD = true;
        M5.Lcd.fillScreen(BLACK); // 画面をクリア
        bool tt = initializeSDCardAndCreateFile("/save/save2.mett");
        tt = tt * initializeSDCardAndCreateFile("/save/save1.mett");
        tt = tt * initializeSDCardAndCreateFile("/save/save3.mett");
        if(!tt){
          kanketu("SD File Create Failed!",600);
          mainmode = 0;

          M5.Lcd.setCursor(0, 0);
        sita = "hello";
        textexx();
        positpoint = 0;
        holdpositpoint = 0;
        imano_page = 0;
          

          return;
        }

        bool ss = loadmett();
        if(!ss){
          kanketu("load Error!",500);
          mainmode = 0;

          M5.Lcd.setCursor(0, 0);
        sita = "hello";
        textexx();
        positpoint = 0;
        holdpositpoint = 0;
        imano_page = 0;
          

          return;
        }
        else{
          mainmode = 7; // モードをSDリスト表示モードに切り替え
          
        }
        
        
        
        return;//mainmode0フラグ誤作動対策
      }else if(maindex == 6){//SDイジェクトフォーマット
        M5.Lcd.fillScreen(BLACK); // 画面をクリア
        M5.Lcd.setTextSize(3);
       M5.Lcd.setTextColor(WHITE, BLACK); // 白文字、黒背景
       M5.Lcd.setCursor(0, 0);
        mainmode = -1;
        maxpage = -1;
        positpoint = 0;
        positpointmax = 2;
        pagemoveflag = 0;
        imano_page = 0;
        M5.Lcd.println("  SD Eject\n  SD Format");
        return;
      }
      // ボタンBが押された場合、sitaを"button1"に設定し、textexx()を呼び出す
      // これはメニューモードでのみ行われる
      M5.Lcd.fillScreen(BLACK); // 画面をクリア
      M5.Lcd.setCursor(0, 0);
      sita = "button1"; // 一時的に表示テキストを変更
      textexx(); // メニュー画面を再描画
    }

    // ボタンCが押された場合の処理 (メニューを下に移動)
    if (M5.BtnC.wasPressed()) {
      if (maindex < sizeof(sitagar) / sizeof(sitagar[0])) { // 最後のオプション (インデックス4) まで移動
        maindex++;
      } else {
        maindex = 0; // 最初のオプションへループ
      }
      sita = sitagar[maindex]; // 選択されたオプションを更新
      textexx(); // メニュー画面を再描画
    }
  }else if(mainmode == -1){


    updatePointer2();
      if(pagemoveflag == 2){
      pagemoveflag = 0;
      return;
    }else if(pagemoveflag == 1){
      pagemoveflag = 0;
      return;
    }else if((pagemoveflag == 5) ){
     M5.Lcd.setCursor(0, 0);
     M5.Lcd.fillScreen(BLACK);
        sita = tttt;
        textexx();
        positpoint = 0;
        holdpositpoint = 0;
        imano_page = 0;
        mainmode = 0;
        return;

     
    
    }else if(M5.BtnB.wasPressed()){
      if(positpoint == 0){
        
          
          M5.Lcd.setCursor(0,0);
          M5.Lcd.fillScreen(BLACK);
          SD.end();
          M5.Lcd.println("Ejected!\n Press B to \nCheck SD Property");
          while(true){
            delay(1);
            M5.update();
            if(M5.BtnB.wasPressed()){
              break;
            }
          }
          
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.println("Loading...");
          M5.Lcd.setTextSize(1);
          M5.Lcd.setCursor(0, 0);
          initializeSDCard();
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.println("SD Data:\n  Capacity:" + getSDCardRawCapacity() + "\n  Format:" + getSDCardType() + "\nMisc:" + getSDCardCIDInfo());
          
          while(true){
            delay(1);
            M5.update();
            if(M5.BtnB.wasPressed()){
              break;
            }
          }
          
          M5.Lcd.setTextSize(3);
        sita = tttt;
        releaseSDBusForOtherUse();
        M5.Lcd.fillScreen(BLACK);
        textexx();
        positpoint = 0;
        holdpositpoint = 0;
        imano_page = 0;
        mainmode = 0;
        return;
          
        
      }else if(positpoint == 1){
        bool ss = areusure();
        if(ss){
          M5.Lcd.fillScreen(BLACK);
          
       
        
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("Formating! \nDo Not Touch SD");
            SD.end();
        initializeSDCard();
        bool ss = formatSDCardFull();
        if(ss){
          kanketu("Format Completed!",500);
        }else{
          kanketu("Format Failed!",500);
        }
        M5.Lcd.setTextSize(3);
        sita = tttt;
        releaseSDBusForOtherUse();
        M5.Lcd.fillScreen(BLACK);
        textexx();
        positpoint = 0;
        holdpositpoint = 0;
        imano_page = 0;
        mainmode = 0;
        return;
      

        }else{
           M5.Lcd.setTextSize(3);
        sita = tttt;
        M5.Lcd.fillScreen(BLACK);
        textexx();
        positpoint = 0;
        holdpositpoint = 0;
        imano_page = 0;
        mainmode = 0;
        return;
        }
      }


    }
  }else{
    mainkansu_optsd();
  }

}






