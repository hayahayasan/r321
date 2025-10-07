#include <M5Unified.h>
#include <Wire.h>
#include<SD.h>
#include <USB.h> 

#include <FS.h>
#include <vector>    // std::vector を使用するために必要
#include <algorithm>
#include <map>      // std::mapを使用するため
#include <set>
#include <SDFat.h>
#include <SPI.h> 
#include "shares.h"
int NUM_RESERVED_NAMES;
SPIClass SDSPI; 
bool nosd;
bool serious_errorsd;
String karadirectname;
int goukei_page;
bool otroot;
String SuperT = ""; // 入力されたテキストを保持する文字列
int cursorIndex = 0; // SuperT内の現在のカーソル位置（文字インデックス）
int cursorPixelX = 0; // カーソルのX座標（ピクセル）
int cursorPixelY = 0; // カーソルのY座標（ピクセル）
int offsetX = 0; // テキスト描画の水平オフセット（スクロール用）
int offsetY = 0; // テキスト描画の垂直オフセット（スクロール用）
int scrollpx = 50;
bool needsRedraw = false;
SPIClass SDSPI; 
SdFs sd;
// SPI設定構造体を定義 (SdSpiConfig(CSピン, 最大速度, モード, SPIインスタンス))
// SD_SCK_MHZ(50)はSdFatのヘルパーマクロで、50MHzのクロックスピードを設定します。
const SdSpiConfig SD_SPI_CONFIG = SdSpiConfig(SD_CS_PIN, SD_SCK_MHZ(50), SPI_MODE0, &SDSPI);
const std::vector<String> reservedWords = {
    "CONFIG", "METT", "VERSION", "TABLE", "OPTION", "DEFAULT",
    "ENCRYPT", "NOTREAD", "REQUIRED", "NULL", "TRUE", "FALSE",
    "READ_ONLY", "ENCRYPTED", "TABLE_KEY", "USRDATA",
    "MAX_ROWS", "STORAGE", "INITIAL_DATA", "PRIMARY_KEY", 
    "SCHEMA_VERSION", "MIN_VERSION", "ACCESS", 
    "FORCE_LAST_MODIFIED", "FORCE_LAST_USER"
};



template String joinVectorToString<int>(const std::vector<int>&);
template String joinVectorToString<String>(const std::vector<String>&);

bool righttrue(){
  return frameright == 2 || frameright % scrollpx == 0;
}
bool lefttrue(){
  return frameleft == 2 || frameleft % scrollpx == 0;
}


// ポインターの位置を更新する関数
void updatePointer2() {
    delay(1);
    M5.Lcd.setTextSize(File_goukeifont);
    // 以前のポインター位置を記憶 (-1は初期状態を示す。これはstaticで一度だけ初期化される)
    static int prev_positpoint = -1;
    // 関数呼び出し時点のpositpoint（ボタン押下前のpositpoint）を保存
    int current_positpoint_on_entry = positpoint; 
    
    
    
    if(M5.BtnA.wasPressed()){
      btna = true;
    } else {
      btna = false;
    }
    
    if(M5.BtnC.wasPressed()){
      btnc = true;
    } else {
      btnc = false;
    }
    
    if(M5.BtnA.isPressed()){
      if(frameleft < 10000){
        frameleft++;
      }
    } else {
      frameleft = 1;
    }
    
    if(M5.BtnC.isPressed()){
      if(frameright < 10000){
        frameright++;
      }
    } else {
      frameright = 1;
    }

    //Serial.println("r" + String(btna) + "l" + String(btnc) + " " + positpoint + "  " + frameright + frameleft);
    pagemoveflag = 0;
    
    if(righttrue()){
      btna = false;
      btnc = true;  
      if(imano_page < maxpage - 1 && maxpage != -1){
        if(positpoint == positpointmax - 1){
          pagemoveflag = 2;
          return;
        }else{
          positpoint++;
        }
        
      }else{
        if(positpoint == positpointmax - 1){
          pagemoveflag = 1;
          return;
        }else{
          positpoint++;
        }
        
      }
    
    } else if(lefttrue() && positpoint == -1){
      btna = true;
      btnc = false;
      Serial.println("HHH" + String(positpoint));
      if(maxpage == -1)  {
        pagemoveflag = 5;
        Serial.println("pagemoved");
        return;
      }
      else if(imano_page >0){
        pagemoveflag = 3;
        return;
      }else if(imano_page == 0 && maxpage != -1){
        pagemoveflag = 4;
        return;
      }
    }  else if (lefttrue() && positpoint > -1) {
        positpoint--; // 上へ移動
        Serial.println("F" + String(DirecX) + "G" + String(positpoint));
        btna = true;
        btnc = false;
        
    }
    
    else if(!lefttrue() && !righttrue()) {
      btna = false;
      btnc = false;
    }
    
    // ページ移動フラグのロジック
    if (positpoint == positpointmax + 1 && imano_page < maxpage - 1 ) {
        if((imano_page == maxpage - 1 && mainmode == 1 && positpoint == maxLinesPerPage3 - 1)){
          return;
        } else {
          pagemoveflag = 1;
          return;
        }
    } else {
      pagemoveflag = 0;
    }

    
   
    
    // ポインターの位置が変更された場合、または初回描画時の処理
    if (prev_positpoint != positpoint) { 
        int pointer_char_width = M5.Lcd.textWidth(">");
        int font_height = M5.Lcd.fontHeight();

        int clear_zone_width = pointer_char_width + M5.Lcd.textWidth(" "); 
        
        // 以前のポインターを消去
        if (prev_positpoint != -1) {
            M5.Lcd.fillRect(0, prev_positpoint * font_height, clear_zone_width, font_height, BLACK);
        } else {
            M5.Lcd.fillRect(0, current_positpoint_on_entry * font_height, clear_zone_width, font_height, BLACK);
        }

        // 新しいポインターを描画
        M5.Lcd.setTextColor(YELLOW); // 黄色に設定
        M5.Lcd.setCursor(0, positpoint * font_height); // 新しい位置にカーソルを設定 (X=0)
        M5.Lcd.print(">"); // ポインターアイコンを描画
        M5.Lcd.setTextColor(WHITE); // 色を白に戻す

        // 現在の位置を次の描画のために記憶
        prev_positpoint = positpoint;
    }
}

bool checkSDCardOnly() {
    Serial.println("\n--- SDカードの初期化チェックを開始 ---");

    // SD.begin()の成功・失敗のみを確認
    if (!SD.begin()) {
        M5.Lcd.fillScreen(RED);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextSize(2);
        M5.Lcd.println("Error: SDカードの初期化に失敗しました。");
        Serial.println("Error: SDカードの初期化に失敗しました。カードが挿入されているか確認してください。");
        return false;
    }

    // 初期化成功
    M5.Lcd.fillScreen(GREEN);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("SDカードが正常に認識されました。");
    Serial.println("Info: SDカードが正常に認識されました。");

    Serial.println("--- SDカードの初期化チェックが完了しました ---");
    return true;
}

// 矢印キー長押し処理用のグローバル変数
// （重複定義を削除しました）

String formatBytes(uint64_t bytes) {
  const uint64_t KB = 1024ULL;
  const uint64_t MB = KB * 1024ULL;
  const uint64_t GB = MB * 1024ULL;
  const uint64_t TB = GB * 1024ULL;

  char output[50]; // 結果を格納するバッファ

  if (bytes >= TB) {
    double tbValue = (double)bytes / TB;
    // TBの場合、MB/KBの詳細は表示しないため、小数点以下は表示しない
    // 必要であればここでtbValueの表示形式を調整してください (例: 1.23 TB)
    snprintf(output, sizeof(output), "%.0f TB %.0f GB %.0f MB %.0f KB", 
             floor(tbValue), // TBの整数部分
             floor(fmod(bytes, TB) / GB), // TBを除いたGB部分
             floor(fmod(bytes, GB) / MB), // GBを除いたMB部分
             floor(fmod(bytes, MB) / KB)  // MBを除いたKB部分
            );
  } else if (bytes >= GB) {
    double gbValue = (double)bytes / GB;
    snprintf(output, sizeof(output), "%.0f GB %.0f MB %.0f KB", 
             floor(gbValue), // GBの整数部分
             floor(fmod(bytes, GB) / MB), // GBを除いたMB部分
             floor(fmod(bytes, MB) / KB)  // MBを除いたKB部分
            );
  } else if (bytes >= MB) {
    double mbValue = (double)bytes / MB;
    snprintf(output, sizeof(output), "%.0f MB %.0f KB", 
             floor(mbValue), // MBの整数部分
             floor(fmod(bytes, MB) / KB)  // MBを除いたKB部分
            );
  } else if (bytes >= KB) {
    double kbValue = (double)bytes / KB;
    // KBの場合のみ小数点以下第3位まで表示し、それ以降は切り捨て
    snprintf(output, sizeof(output), "%.3f KB", floor(kbValue * 1000) / 1000);
  } else {
    // 1KB未満の場合はバイト単位で表示（小数点はなし）
    snprintf(output, sizeof(output), "%.0f KB", 0.0); // 0KBと表示
  }

  // 容量が0バイトの場合、"0 KB"と表示するように調整
  if (bytes == 0) {
      return "0 KB";
  }

  return String(output);
}


bool endsWithTxtOrDbm(String filename) {
  // filename が ".txt" で終わるか、または ".dbm" で終わるかをチェック
  return filename.endsWith(".txt") || filename.endsWith(".dbm");
}
const char FORBIDDEN_CHARS[] = {'"', '<', '>', '|', '*', ':', '?', '\\', '/'};

// Windowsの予約名 (大文字小文字を区別しない)
// これらの名前は、単独でファイル名やディレクトリ名として使用できません。



/**
 * @brief 文字列がWindowsのファイル名またはディレクトリ名として共通の無効な文字を含んでいるかチェックします。
 * @param name チェックする文字列
 * @return 無効な文字が含まれていれば true、そうでなければ false
 */
bool containsForbiddenChars(String name) {
  for (int i = 0; i < sizeof(FORBIDDEN_CHARS); ++i) {
    if (name.indexOf(FORBIDDEN_CHARS[i]) != -1) {
      return true; // 禁止文字が含まれている
    }
  }
  return false;
}

/**
 * @brief 文字列がWindowsの予約名と一致するかチェックします (大文字小文字を区別しない)。
 * 通常、ファイル名やディレクトリ名のベース名部分に適用されます。
 * @param name チェックする文字列
 * @return 予約名と一致すれば true、そうでなければ false
 */
bool isReservedName(String name) {
  String upperName = name;
  upperName.toUpperCase(); // 大文字に変換して比較

  for (int i = 0; i < NUM_RESERVED_NAMES; ++i) {
    if (upperName.equals(RESERVED_NAMES[i])) {
      return true; // 予約名と一致
    }
  }
  return false;
}

/**
 * @brief 文字列がWindowsのディレクトリ名として有効かチェックします。
 * ルール:
 * - 禁止文字を含まない
 * - 1文字以上である
 * - 複数行でない
 * - 末尾がピリオド (.) でない
 * - 拡張子のような形式 (例: ".txt") で終わらない (ディレクトリ名には通常拡張子がないため)
 * - 予約名と一致しない
 * - 最大長 (255文字) を超えない
 * - 空白文字のみでない
 * @param textt チェックする文字列
 * @return 有効なディレクトリ名であれば true、そうでなければ false
 */
bool isValidWindowsDirName(String textt) {
  // 1. 空文字列でないこと
  if (textt.length() == 0) {
    return false;
  }

  // 新規追加: 2. 空白文字のみでないこと
  // String.trim() は Arduino の String クラスにはないため、手動でトリムしてチェックします。
  bool allWhitespace = true;
  for (int i = 0; i < textt.length(); ++i) {
    if (!isspace(textt.charAt(i))) {
      allWhitespace = false;
      break;
    }
  }
  if (allWhitespace) {
    return false;
  }

  // 3. 禁止文字を含まないこと
  if (containsForbiddenChars(textt)) {
    return false;
  }

  // 4. 複数行でないこと (改行文字を含まないこと)
  if (textt.indexOf('\n') != -1 || textt.indexOf('\r') != -1) {
    return false;
  }

// 5. 末尾がピリオド (.) でないこと
if (textt.endsWith(".")) {
  return false;
}

  // 6. 拡張子のような形式で終わらないこと (ディレクトリ名には通常拡張子がないため)
  //    例: "MyFolder.txt" のような形式を拒否
  int lastDotIndex = textt.lastIndexOf('.');
  if (lastDotIndex != -1 && lastDotIndex > 0) { // ドットがあり、かつ最初の文字ではない場合
    // ドット以降に文字があるか、かつその文字が拡張子のように見えるか
    if (lastDotIndex < textt.length() - 1) {
      // ドット以降の文字列がすべて英数字の場合、拡張子とみなして拒否
      bool isExtensionLike = true;
      for (int i = lastDotIndex + 1; i < textt.length(); ++i) {
        if (!isalnum(textt.charAt(i))) {
          isExtensionLike = false; // 英数字以外が含まれる場合は拡張子ではないと判断
          break;
        }
      }
      if (isExtensionLike) {
        return false;
      }
    }
  
  }

  // 7. 予約名と一致しないこと
  if (isReservedName(textt)) {
    return false;
  }

  // 8. 最大長 (Windowsの標準は255文字) を超えないこと
  //    ただし、ここでは一般的な入力の妥当性としてより短い制限を設けることも可能です。
  //    Windowsのファイルシステムでは個々の名前は255文字まで可能です。
  if (textt.length() > 255) { // Windowsの一般的なファイル/ディレクトリ名の最大長
    return false;
  }

  for(int ii = 0;ii < 100;ii++){
    if(Filelist[ii] == textt && ForDlist[ii] == "1"){
      return false;
    }
  }
  if(containsForbiddenChars(textt)){
      return false;
  }
  return true; // すべてのチェックを通過
}



/**
 * @brief 文字列がWindowsのファイル名として有効かチェックします。
 * ルール:
 * - 禁止文字を含まない
 * - 1文字以上である
 * - 複数行でない
 * - 末尾がピリオド (.) でない
 * - 末尾がスペースでない
 * - 拡張子 (例: ".txt") を持つこと (ドットが1つ以上あり、ドットが最初でも最後でもない)
 * - 予約名と一致しない (拡張子を除くベース名でチェック)
 * - 最大長 (255文字) を超えない
 * @param textt チェックする文字列
 * @return 有効なファイル名であれば true、そうでなければ false
 */
// Windowsのファイル名として有効かどうかをチェックする関数
bool isValidWindowsFileName(String textt) {
  // 1. 空文字列でないこと
  if (textt.length() == 0) {
    return false;
  }

  // 2. 禁止文字を含まないこと
  // (containsForbiddenChars関数は別途定義されていると仮定)
  if (containsForbiddenChars(textt)) {
    return false;
  }

  // 3. 複数行でないこと (改行文字を含まないこと)
  if (textt.indexOf('\n') != -1 || textt.indexOf('\r') != -1) {
    return false;
  }

  // 4. 末尾がピリオド (.) でないこと
  if (textt.endsWith(".")) {
    return false;
  }

  // 5. 末尾がスペースでないこと
  if (textt.endsWith(" ")) {
    return false;
  }

  // 6. 拡張子を持つこと (xxx.ttt の形式)
  int lastDotIndex = textt.lastIndexOf('.');
  // ドットがない、またはドットが最初の文字、またはドットが最後の文字の場合は無効
  if (lastDotIndex == -1 || lastDotIndex == 0 || lastDotIndex == textt.length() - 1) {
    return false; // 拡張子がないか、形式が不正
  }

  // 新しいチェック: .の右側に来る文字が「A~Z」「a~z」「0~9」のみであること
  String extension = textt.substring(lastDotIndex + 1);
  for (int i = 0; i < extension.length(); i++) {
    char c = extension.charAt(i);
    // 文字が英数字でない場合、無効とする
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
      return false;
    }
  }

  // 7. 予約名と一致しないこと (拡張子を除くベース名でチェック)
  String baseName = textt.substring(0, lastDotIndex); // 最後のドットまでの部分を取得
  // (isReservedName関数は別途定義されていると仮定)
  if (isReservedName(baseName)) {
    return false;
  }

  // 8. 最大長 (Windowsの標準は255文字) を超えないこと
  if (textt.length() > 255) { // Windowsの一般的なファイル/ディレクトリ名の最大長
    return false;
  }

  // 9. 既存のファイルリストとの重複チェック (FilelistとForDlistが定義されていると仮定)
  // この部分の変数 (Filelist, ForDlist) は、この関数が動作する環境で適切に定義されている必要があります。
  // 例: String[] Filelist; String[] ForDlist;
  for(int ii = 0;ii < 100;ii++){
    // ForDlist[ii] == "0" はファイルを示していると仮定
    if(Filelist[ii] == textt && ForDlist[ii] == "0"){
      return false; // 同じ名前のファイルが既に存在する場合
    }
  }
  if(containsForbiddenChars(textt)){
      return false;
  }
  return true; // すべてのチェックを通過
}

String maeredirect(String path){
  int lastSlashIndex = path.lastIndexOf('/'); // 右から最初のスラッシュの位置を探す


  if (lastSlashIndex != -1) { // スラッシュが見つかった場合


//変換前:/ああ/aa/(最後の/はいらないので二回カット)
    String ss = path.substring(0, lastSlashIndex);
    
    if(ss == ""){
      ss = "/";
    }else{
      lastSlashIndex = ss.lastIndexOf('/');
      ss = path.substring(0, lastSlashIndex);
      ss = ss + "/";
    }
    return(ss);
    
  } else { // スラッシュが見つからなかった場合（例: "hhhhh"）
    return("error");
  }
}

String wirecheck() {
    delay(1); // 1msのディレイ
    byte error, address;
    int ndevices = 0;
    // Wire.begin()とWire.setClock()はsetup()で一度だけ行うべきですが、
    // ここでは毎フレーム呼ばれることを前提としているため、
    // 厳密にはここではなくsetup()で行うのが適切です。
    // しかし、ユーザーの指示によりsetup()が削除されているため、
    // 互換性を保つためにこの関数内に残します。
    // 実際の運用では、M5.begin()の後に一度だけ呼び出すようにしてください。
    // Wire.begin();
    // Wire.setClock(400000); 

    address = 95; // CardKB1.1のI2Cアドレス
    Wire.beginTransmission(address);
    error = Wire.endTransmission(); // デバイスの存在を確認

    if (error == 0) { // デバイスが正常に検知された場合
        //Serial.println("I2Cデバイス検知開始"); // デバッグ用
        if (address == 95) {
            //Serial.println("CardKB1.1検知完了"); // デバッグ用
            ndevices++;
            int milcounter = 0;
            // データが利用可能になるまで待機、またはタイムアウト
            while (!Wire.available()) {
                Wire.requestFrom(95, 1); // 1バイトのリクエスト
                milcounter++;
                delay(1);
                if (milcounter > 10) break; // 100msでタイムアウト
            }

            if (Wire.available()) { // データが利用可能かチェック
                char key = Wire.read(); // データを読み取る
                if (key != 0) { // 非ゼロデータはキープレスを示す可能性
                    switch ((int)key) {
                        case 13: return "ENT";   // Enterキー
                        case 8:  return "BACK";  // Backspaceキー
                        case 27: return "ESC";   // ESCキー
                        case 32: return "SPACE"; // スペースキー
                        case 9:  return "TAB";   // Tabキー
                        case 181: return "UP";    // 上矢印キー
                        case 183: return "RIGHT"; // 右矢印キー
                        case 182: return "DOWN";  // 下矢印キー
                        case 180: return "LEFT";  // 左矢印キー
                        default:
                            //Serial.println("その他の文字キー: " + String(key)); // デバッグ用
                            return String(key); // その他の文字キー
                    }
                } else {
                    //Serial.println("osaretenai"); // デバッグ用
                    return "NULL"; // キーが押されていない (キーコードが0)
                }
            }
        }
    } else if (error == 4) {
        // Serial.println("I2Cデバイスが見つかりません"); // デバッグ用
        return "error"; // デバイスが見つからないエラー
    } else {
        // Serial.println("えらー" + String(error)); // デバッグ用
        return "error"; // その他のI2Cエラー
    }

    if (ndevices == 0) {
        // Serial.println("なんも接続されていません"); // デバッグ用
        return "nokey"; // デバイスが何も接続されていない
    }

    return "whattf"; // 何らかの予期せぬ状態
}

String migidkae(String karac){
  int lastSlashIndex = karac.lastIndexOf('/');
String extractedName;

if (lastSlashIndex != -1) {
  // スラッシュが見つかった場合、その次の文字から最後までを抽出
  extractedName = karac.substring(lastSlashIndex + 1);
} else {
  // スラッシュが見つからなかった場合（例: "filename.txt"）、文字列全体が名前
  extractedName = karac;
}

return extractedName;
}

int nowposit() {
    
    Serial.println(String(positpoint) + "ff"  + String(imano_page) +"gg" + String(maxLinesPerPage) );
    
    return (positpoint + (imano_page * maxLinesPerPage)); // 2以上の場合は元の計算を行う
}
int nowpositZ() {
    
   // Serial.println(String(positpoint) + "ff"  + String(imano_page) +"gg" + String(maxLinesPerPage) );
    
    return (positpointmain1 + (imano_page * maxLinesPerPage)); // 2以上の場合は元の計算を行う
}



// Function to remove the trailing slash from a path (except for the root directory)
String cleanPath(String path) {
    if (path == "/") {
        return path;
    }
    if (path.length() > 1 && path.endsWith("/")) {
        return path.substring(0, path.length() - 1);
    }
    return path;
}

// Function to get the parent directory of a given path
// This handles both file paths and directory paths robustly.
String getParentDirectory(String path) {
    // Find the last slash to determine the parent directory.
    int lastSlash = path.lastIndexOf('/');

    // If the path is a root-level file (e.g., "/file.txt") or the root itself, its parent is the root directory.
    if (lastSlash == 0) {
        return "/";
    }

    // If there is no slash, it's not a valid path for this context.
    if (lastSlash == -1) {
        return "";
    }
    
    // The parent directory is the substring up to the last slash.
    return path.substring(0, lastSlash);
}

// Function to recursively create nested directories
bool createDirRecursive(const char* path) {
    String currentPath = "";
    String pathString = String(path);
    if (pathString.startsWith("/")) {
        currentPath += "/";
    }
    
    int lastSlash = pathString.lastIndexOf('/');
    if (lastSlash == 0 && pathString.length() > 1) { // Root directory with content e.g. "/folder"
      if (!SD.exists(pathString)) {
        return SD.mkdir(pathString);
      }
      return true;
    } else if (lastSlash == -1) { // No path given e.g. "file.txt"
        return true;
    }

    int start = pathString.startsWith("/") ? 1 : 0;
    int slashIndex;
    
    while ((slashIndex = pathString.indexOf('/', start)) != -1) {
        currentPath += pathString.substring(start, slashIndex) + "/";
        if (!SD.exists(currentPath)) {
            if (!SD.mkdir(currentPath)) {
                return false;
            }
        }
        start = slashIndex + 1;
    }
    return true;
}

// Function to check for and rename files with a unique new name to avoid duplication
String checkAndRename(String filePath) {
    if (!SD.exists(filePath)) {
        return filePath;
    }

    int dotIndex = filePath.lastIndexOf('.');
    String baseName = filePath;
    String extension = "";
    if (dotIndex != -1) {
        baseName = filePath.substring(0, dotIndex);
        extension = filePath.substring(dotIndex);
    }
    
    int slashIndex = baseName.lastIndexOf('/');
    String fileNameOnly = baseName;
    String directoryPath = "/";
    if (slashIndex != -1) {
        fileNameOnly = baseName.substring(slashIndex + 1);
        directoryPath = baseName.substring(0, slashIndex + 1);
    }

    for (int i = 1; i <= 1000; i++) {
        String newFileName = directoryPath + fileNameOnly + "(" + String(i) + ")" + extension;
        if (!SD.exists(newFileName)) {
            return newFileName;
        }
    }
    
    return "";
}

// Function to copy a file with a progress display and a cancel option
// Returns 0 for success, 1 for cancellation, 2 for failure.
int copyFile(const char* sourcePath, const char* destinationPath, long totalSize) {
    Serial.println("Starting file copy operation...");
    File sourceFile = SD.open(sourcePath, FILE_READ);
    if (!sourceFile) {
        Serial.println("[ERROR] Failed to open source file for reading.");
        return 2;
    }
    String destFullPath = String(destinationPath);
    int lastSlash = destFullPath.lastIndexOf('/');
    if (lastSlash > 0) {
        String parentDir = destFullPath.substring(0, lastSlash);
        if (!createDirRecursive(parentDir.c_str())) {
            Serial.println("[ERROR] Failed to create destination directories.");
            sourceFile.close();
            return 2;
        }
    }
    
    File destinationFile = SD.open(destinationPath, FILE_WRITE);
    if (!destinationFile) {
        Serial.println("[ERROR] Failed to open destination file for writing.");
        sourceFile.close();
        return 2;
    }

    uint32_t bytesRead = 0;
    uint32_t totalCopiedSize = 0;
    uint8_t buffer[512];
    bool cancelled = false;
    
    // Copy the file in a loop
    while ((bytesRead = sourceFile.read(buffer, sizeof(buffer))) > 0) {
        M5.update();
        if (M5.BtnC.wasPressed()) {
            Serial.println("Copy operation cancelled by user.");
            cancelled = true;
            break;
        }

        // Write to the destination and check if the number of written bytes matches the number of read bytes.
        size_t bytesWritten = destinationFile.write(buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            Serial.println("[ERROR] Write failed or was incomplete. Aborting copy.");
            cancelled = true;
            break;
        }
        totalCopiedSize += bytesWritten;
        if (totalSize > 0) {
            int percent = (int)((float)totalCopiedSize / totalSize * 100);
            M5.Lcd.setCursor(0, 40);
            M5.Lcd.printf("Copying... %d%% ", percent);
        }
    }
    
    // Explicitly flush the buffer to ensure all data is written to the physical SD card.
    destinationFile.flush();
    destinationFile.close();
    sourceFile.close();

    if (cancelled) {
        Serial.println("Removing incomplete destination file.");
        SD.remove(destinationPath);
        return 1;
    }

    // The key fix: verify the final file size after all operations are complete.
    File finalFile = SD.open(destinationPath, FILE_READ);
    bool success = false;
    if (finalFile) {
        if (finalFile.size() == totalSize) {
            success = true;
            Serial.println("File size verification successful.");
        } else {
            // Mismatched file size. Cleanup and fail.
            Serial.printf("[ERROR] Mismatched file size. Expected: %ld, Got: %ld\n", totalSize, finalFile.size());
            SD.remove(destinationPath); // Remove the incomplete file
        }
        finalFile.close();
    } else {
        Serial.println("[ERROR] Failed to open destination file for size verification.");
    }
    
    return success ? 0 : 2;
}

// Function to recursively remove a file or folder
bool removePath(const char* path) {
    File item = SD.open(path);
    if (!item) return false;
    if (item.isDirectory()) {
        File subItem = item.openNextFile();
        while (subItem) {
            String subPath = String(path) + "/" + subItem.name();
            if (!removePath(subPath.c_str())) {
                item.close();
                return false;
            }
            subItem = item.openNextFile();
        }
        item.close();
        return SD.rmdir(path);
    } else {
        item.close();
        return SD.remove(path);
    }
}

// Main function to perform file operations
bool smartCopy(String sourcePath, String destinationPath, bool isCut) {

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.println("Initializing SD card...");
    Serial.println("Initializing SD card...");
    if (!SD.begin()) {
        M5.Lcd.println("[ERROR] SD card initialization failed!");
        Serial.println("[ERROR] SD card initialization failed!");
        return false;
    }

    sourcePath = cleanPath(sourcePath);
    destinationPath = cleanPath(destinationPath);
    
    // Get the parent directories of the source and destination paths
    String sourceParentDir = getParentDirectory(sourcePath);
    Serial.println("Source parent: " + sourceParentDir + ", Destination: " + destinationPath); 
    // 1. Verify that the source and destination directories are not the same
    // Check if we are trying to copy a file into its current directory
    if (sourceParentDir == destinationPath) {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        
        M5.Lcd.println("No Copy Use!");
        Serial.println("No Copy Use!");
        delay(2000);
        return false;
    }
    
    // 2. Verify that the source file exists
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("Checking source: %s\n", sourcePath.c_str());
    Serial.printf("Checking source: %s\n", sourcePath.c_str());
    if (!SD.exists(sourcePath)) {
        M5.Lcd.println("[ERROR] Source path does not exist.");
        Serial.println("[ERROR] Source path does not exist.");
        return false;
    }

    // 3. Open the source file in read mode and get its size
    File source = SD.open(sourcePath, FILE_READ);
    if (!source) {
        M5.Lcd.println("[ERROR] Failed to open source file for reading.");
        Serial.println("[ERROR] Failed to open source file for reading.");
        return false;
    }
    if (source.isDirectory()) {
        M5.Lcd.println("[ERROR] Folder copy is not supported.");
        Serial.println("[ERROR] Folder copy is not supported.");
        source.close();
        return false;
    }
    // Get file size in long type
    long totalSize = source.size();
    // Close the file after getting the size
    source.close();
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    // Output file size to M5Lcd using %ld specifier
    M5.Lcd.printf("File size: %ld bytes\n", totalSize);
    Serial.printf("File size: %ld bytes\n", totalSize);

    // 4. Display initial progress and wait for 1 second with a cancel option.
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("Copying: 0%% 0/%ld byte\n", totalSize);
    M5.Lcd.setCursor(0, 40);
    M5.Lcd.println("Press C to cancel.");
    for (int i = 0; i < 10; i++) {
        M5.update();
        if (M5.BtnC.wasPressed()) {
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.println("Copy aborted!");
            Serial.println("Copy aborted!");
            delay(1000);
            return false;
        }
        delay(100);
    }
    
    // 5. Determine the final destination path
    int lastSlash = sourcePath.lastIndexOf('/');
    String fileNameOnly = sourcePath.substring(lastSlash + 1);
    String finalDestinationPath;
    if (destinationPath == "/") {
        finalDestinationPath = "/" + fileNameOnly;
    } else {
        finalDestinationPath = destinationPath + "/" + fileNameOnly;
    }
    
    // 6. Get a numbered file path to prevent overwriting
    String uniqueDestPath = checkAndRename(finalDestinationPath);
    if (uniqueDestPath == "") {
        M5.Lcd.println("[ERROR] Paste overflowed!");
        Serial.println("[ERROR] Paste overflowed!");
        return false;
    }
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("Copying to: %s\n", uniqueDestPath.c_str());
    Serial.printf("Copying to: %s\n", uniqueDestPath.c_str());

    // Execute the copy operation
    int copyResult = copyFile(sourcePath.c_str(), uniqueDestPath.c_str(), totalSize);
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);

    if (copyResult == 0) {
        bool removeSucceeded = true;
        if (isCut) {
            M5.Lcd.println("Starting removal...");
            Serial.println("Starting removal...");
            if(!removePath(sourcePath.c_str())) {
                M5.Lcd.fillScreen(BLACK);
                M5.Lcd.setCursor(0, 0);
                M5.Lcd.println("[ERROR] Paste and delete failed!");
                Serial.println("[ERROR] Paste and delete failed!");
                removeSucceeded = false;
            } else {
                M5.Lcd.fillScreen(BLACK);
                M5.Lcd.setCursor(0, 0);
                M5.Lcd.println("Paste and delete succeed!");
                Serial.println("Paste and delete succeed!");
            }
        } else {
            M5.Lcd.println("Paste successful!");
            Serial.println("Paste successful!");
        }
        
    } else if (copyResult == 1) {
        M5.Lcd.println("Copy cancelled!");
        Serial.println("Copy cancelled!");
        return false;
    } else { // copyResult == 2
        M5.Lcd.println("[ERROR] Copy failed!");
        Serial.println("Copy failed!");
        return false;
    }
    
    delay(2000);

    return true;
}


bool areusure(){
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.println("Are you sure?"); // 確認メッセージを表示
  M5.Lcd.println("C to Confirm A to Cancel");
  while(true){
    M5.update();
    delay(1);
    if(M5.BtnA.wasPressed()){
      M5.Lcd.clear();
      M5.Lcd.setCursor(0, 0);
      return false;
    }else if(M5.BtnC.wasPressed()){
      M5.Lcd.clear();
      M5.Lcd.setCursor(0, 0);
      return true;
    }
  }
  
}

bool areubunki(String texta,String textb){
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.println("Are you sure?"); // 確認メッセージを表示
  M5.Lcd.println("C:" + texta);
  M5.Lcd.println("A:" + textb);
  while(true){
    M5.update();
    delay(1);
    if(M5.BtnA.wasPressed()){
      M5.Lcd.clear();
      M5.Lcd.setCursor(0, 0);
      return false;
    }else if(M5.BtnC.wasPressed()){
      M5.Lcd.clear();
      M5.Lcd.setCursor(0, 0);
      return true;
    }
  }
  
}

void nummempty(){
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  if(serious_errorsd){
    M5.Lcd.setTextColor(RED);
    otroot = false;
    M5.Lcd.println("  SD Error! Press Button to go back"); // 切り詰められた文字列を表示
  }else{
    M5.Lcd.setTextColor(WHITE);
    otroot = true;
    M5.Lcd.println("  No Files... Press Button"); // 切り詰められた文字列を表示
  }
  
  nosd = true;
  return;
}
// Function to list all files and folders in the SD card's root directory
// SDカードのルートディレクトリの内容をページごとに表示する関数
void listSDRootContents(int pagetax,String Directtory,bool checkfirstMaxLine ) {
  Serial.println("Direcx:"  + Directtory);
  
  nosd = false;
  mainmode = 1;
  bool numempty = false;
  serious_errorsd = false;
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(File_goukeifont); // フォントサイズをFile_goukeifontに設定
  
  if (!SD.begin(GPIO_NUM_4, SPI, 20000000)) {//SDカード入ってない
    serious_errorsd = true;
    nummempty();
    
    return;
  }
  //ルートではなく、"/"で終わる場合のみ語尾を削除する(重要)
  String gg = Directtory;
  if (Directtory.endsWith("/") && Directtory.length() > 1) {
    gg = Directtory.substring(0, Directtory.length() - 1);
  }
  
  File root = SD.open(gg); // ルートディレクトリを開く
  Serial.println("WWW!" + gg);
  if (!root) {
    M5.Lcd.println("Failed to open root directory!"); // ルートディレクトリの展開に失敗しました
    if (Directtory == "") {//ルートから空の時
      serious_errorsd = true;
    } else{
      serious_errorsd = false;
      karadirectname = Directtory;
    }
    
    nummempty();
    return;
  }

  if (!root.isDirectory()) {
    M5.Lcd.println("Root is not a directory!"); // ルートはディレクトリではありません
    serious_errorsd = true;
    nummempty();
    return;
  }

  // 現在のフォントサイズ (setTextSize(1) のデフォルト) に基づいて1ページあたりの最大行数を計算
 
  maxLinesPerPage = (M5.Lcd.height() / M5.Lcd.fontHeight()) - 1; 

  positpointmaxg  = maxLinesPerPage;
  std::vector<String> stdfilelist; 
  stdfilelist.clear();
  std::vector<std::pair<String, bool>> entries;
  // Vector to store entry names and their types (directory or file)
  std::vector<String> tempDirectListVector; // Temporary vector to store full paths
  int maxfileperd = 0;
  while (true) {
    File entry = root.openNextFile(); // Open the next file or directory
    if (!entry) {
      break; // Exit the loop if there are no more files
    }else{
      maxfileperd++;
    }
    String entryName = String(entry.name()); // Get the entry name
    entries.push_back({entryName, entry.isDirectory()}); // Add the entry to the local 'entries' vector
    stdfilelist.push_back(entryName); // Add the file name to the global stdfilelist

    // Generate and store the full path in the temporary vector
    String fullPath = "/" + entryName;
    if (entry.isDirectory()) {
      fullPath += "/"; // Add a trailing slash for directories
    }
    tempDirectListVector.push_back(fullPath); // Add the full path to the temporary vector
    
    entry.close(); // Close the entry
  }
  root.close();

  if (entries.empty()) { // もし entryNames.empty() であれば、独立したvectorを使用する場合
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);
    
    goukei_page = 0;
    if(Directtory == ""){
      serious_errorsd = true;
    }else{
      Serial.println("No files in this directory: " + Directtory);
      karadirectname = Directtory;
      serious_errorsd = false;
      
    }
    nummempty();
    return;
  }

  int currentPage = pagetax;

  // ページ数の計算を修正: 端数がある場合でも次のページとしてカウント
  // 例: 11個のファイルがあり、1ページに10行表示できる場合、(11 + 10 - 1) / 10 = 2ページ
 maxpage = (maxfileperd + maxLinesPerPage - 1) / maxLinesPerPage;
  if (maxpage == 0 && maxfileperd > 0) { // ファイルが1つでもあるが、計算上0ページになる場合の補正
      maxpage = 1;
  }

  maxLinesPerPage2 = (int)entries.size();
  maxLinesPerPage3 = (int)entries.size() % maxLinesPerPage;
  Serial.println("Nokori Files: " + (String)maxLinesPerPage2 + "g " + (int)entries.size() + "b " + (int)maxpage);
  goukei_page = maxpage; // グローバル変数に合計ページ数を設定
  Serial.println("Total files/dirs: " + String(maxfileperd));
  Serial.println("Max lines per page: " + String(maxLinesPerPage));
  Serial.println("Calculated max pages: " + String(maxpage));

  // positpointmax の計算を修正
  if (pagetax == maxpage - 1) { // 現在のページが最後のページの場合
    int remainingFiles = maxfileperd % maxLinesPerPage;
    if (remainingFiles == 0 && maxfileperd > 0) { // 最後のページがちょうど満杯の場合
      positpointmax = maxLinesPerPage - 1;
    } else if (maxfileperd == 0) { // ファイルが全くない場合
      positpointmax = -1; // 選択可能な項目がないことを示す
    }
    else { // 最後のページに端数がある場合
      positpointmax = remainingFiles - 1;
    }
  } else { // 現在のページが最後のページではない場合
    positpointmax = maxLinesPerPage - 1;
  }
  
  // 指定されたページ番号が有効範囲内かチェック
  if (pagetax < 0 || pagetax >= maxpage) {
    positpoint--;
    return;
  }

  // 指定されたページのみを描画
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);

  int startIdx = pagetax * maxLinesPerPage;
  int endIdx = std::min((int)entries.size(), startIdx + maxLinesPerPage);

  for (int i = startIdx; i < endIdx; ++i) {
    String fullEntryName;
    // プレフィックスと色を決定 (isDirectory の情報を使用)
    if (entries[i].second) { // ディレクトリの場合
      M5.Lcd.setTextColor(GREEN); // 緑色
      fullEntryName = "   [D] " + entries[i].first; // [D]でディレクトリを示す
    } else { // ファイルの場合
      M5.Lcd.setTextColor(SKYBLUE); // 水色
      fullEntryName = "   [F] " + entries[i].first; // [F]でファイルを示す
    }

    // 表示幅を計算
    int displayWidth = M5.Lcd.width();

    // 文字列を表示幅に収まるように切り詰める
    String displayString = "";
      
    
    for (int j = 0; j < fullEntryName.length(); ++j) {
      String testString = displayString + fullEntryName.charAt(j);
      if (M5.Lcd.textWidth(testString) <= displayWidth) {
        displayString = testString;
      
      } else {
        
        break; // 次の文字を追加すると表示幅を超える場合は停止
      }
    }
    M5.Lcd.println(displayString); // 切り詰められた文字列を表示
    M5.Lcd.setTextColor(WHITE);     // 次の行のために色をリセット
    
  }

  // ページ情報を下部に表示 (表示しているページのみ)
  


  
    // グローバル配列Filelist[100]を初期化し、stdfilelistのファイル名を格納
  // まずFilelistを空の文字列で初期化（既存のデータをクリア）
  for (int i = 0; i < 100; ++i) {
    Filelist[i] = "";
    directlist[i] = "";
    ForDlist[i] = "";
  }
  // stdfilelistの内容をFilelistにコピー
  // Filelistのサイズ制限 (100) を考慮し、超える場合はコピーしない
  for (int i = 0; i < std::min((int)entries.size(), 100); ++i) {
    Filelist[i] = entries[i].first;
   // Serial.println(Filelist[i]); // デバッグ用にファイル名をシリアルモニタに出力
  }

  for (int i = 0; i < std::min((int)stdfilelist.size(), 100); ++i) {
    Filelist[i] = stdfilelist[i];
  }

  // After populating Filelist, copy the full paths from tempDirectListVector to directlist
  // Consider the size limit of directlist (100) and do not copy if it exceeds
  for (int i = 0; i < std::min((int)tempDirectListVector.size(), 100); ++i) {
    directlist[i] = tempDirectListVector[i];
    ForDlist[i] = entries[i].second ? "1" : "0";
    //Serial.println(directlist[i] + "::" + ForDlist[i]);
  }

  delay(5);
  
}

bool copyFileContent(String sourcePath, String destinationPath) {
  M5.Lcd.printf("Copying content from: %s to %s\n", sourcePath.c_str(), destinationPath.c_str());
  File sourceFile = SD.open(sourcePath.c_str(), FILE_READ);
  if (!sourceFile) {
    M5.Lcd.println("  Failed to open source file for reading.");
    return false;
  }

  File destinationFile = SD.open(destinationPath.c_str(), FILE_WRITE);
  if (!destinationFile) {
    M5.Lcd.println("  Failed to open destination file for writing.");
    sourceFile.close();
    return false;
  }

  byte buffer[512]; // 512 byte buffer
  size_t bytesRead;

  // Read from source file and write to destination file
  while ((bytesRead = sourceFile.read(buffer, sizeof(buffer))) > 0) {
    destinationFile.write(buffer, bytesRead);
  }

  sourceFile.close();
  destinationFile.close();
  M5.Lcd.println("  Content copy successful.");
  return true;
}
// Function to move/copy a directory and its direct files
// If keepOriginal is true, the original directory and its files are kept (copy).
// If keepOriginal is false, the original directory and its files are deleted (move).
void moveDir(String srcDir, String destDir, bool keepOriginal) {
  M5.Lcd.printf("Processing directory from %s to %s (Keep Original: %s)\n",
                srcDir.c_str(), destDir.c_str(), keepOriginal ? "Yes" : "No");

  // Get the base name of the source directory (e.g., "source_dir" from "/source_dir")
  int lastSlashIndex = srcDir.lastIndexOf('/');
  String srcDirBaseName = srcDir.substring(lastSlashIndex + 1);
  if (srcDirBaseName.length() == 0 && srcDir.length() > 0) { // Handle root directory case or trailing slash
      srcDirBaseName = srcDir; // For root, use the full path as base name (e.g., "/")
  } else if (srcDirBaseName.length() == 0 && srcDir.length() == 0) {
      srcDirBaseName = "root"; // Fallback for empty string
  }


  // Construct the full path for the new directory inside the destination
  String finalDestDir = destDir + "/" + srcDirBaseName;

  // Open the source directory
  File root = SD.open(srcDir.c_str());
  if (!root) {
    M5.Lcd.printf("Failed to open source directory: %s\n", srcDir.c_str());
    return;
  }
  if (!root.isDirectory()) {
    M5.Lcd.printf("%s is not a directory.\n", srcDir.c_str());
    root.close();
    return;
  }

  // Create the final destination directory (e.g., /garbage/source_dir) if it doesn't exist
  if (!SD.exists(finalDestDir.c_str())) {
    if (SD.mkdir(finalDestDir.c_str())) {
      M5.Lcd.printf("Created destination directory: %s\n", finalDestDir.c_str());
    } else {
      M5.Lcd.printf("Failed to create destination directory: %s\n", finalDestDir.c_str());
      root.close();
      return;
    }
  } else {
    M5.Lcd.printf("Destination directory already exists: %s\n", finalDestDir.c_str());
  }

  // Iterate through files in the source directory and copy/move them
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) { // Process only files, not subdirectories
      String sourcePath = srcDir + "/" + file.name();
      String destinationPath = finalDestDir + "/" + file.name();
      // Always copy content to the new location first
      if (copyFileContent(sourcePath, destinationPath)) {
        M5.Lcd.printf("  Copied file: %s\n", file.name());
      } else {
        M5.Lcd.printf("  Failed to copy file: %s\n", file.name());
      }
    }
    file.close(); // Close the current file/directory entry
    file = root.openNextFile(); // Open the next entry
  }
  root.close(); // Close the root source directory

  // If keepOriginal is false, delete the original files and the source directory
  if (!keepOriginal) {
    M5.Lcd.printf("Deleting original files and directory: %s\n", srcDir.c_str());
    File deleteRoot = SD.open(srcDir.c_str());
    if (deleteRoot) {
      File fileToDelete = deleteRoot.openNextFile();
      while (fileToDelete) {
        if (!fileToDelete.isDirectory()) {
          String pathToDelete = srcDir + "/" + fileToDelete.name();
          if (SD.remove(pathToDelete.c_str())) {
            M5.Lcd.printf("  Deleted original file: %s\n", pathToDelete.c_str());
          } else {
            M5.Lcd.printf("  Failed to delete original file: %s\n", pathToDelete.c_str());
          }
        }
        fileToDelete.close();
        fileToDelete = deleteRoot.openNextFile();
      }
      deleteRoot.close();
      // Finally, remove the original source directory itself
      if (SD.rmdir(srcDir.c_str())) {
        M5.Lcd.printf("Successfully removed original directory: %s\n", srcDir.c_str());
      } else {
        M5.Lcd.printf("Failed to remove original directory: %s\n", srcDir.c_str());
      }
    } else {
      M5.Lcd.printf("Failed to open original directory for deletion: %s\n", srcDir.c_str());
    }
  }

  M5.Lcd.println("Finished directory processing.");
}

// Function to create test files
void createTestFiles(String dirPath) {
  M5.Lcd.printf("Creating test files in %s...\n", dirPath.c_str());
  if (!SD.exists(dirPath.c_str())) {
    if (SD.mkdir(dirPath.c_str())) {
      M5.Lcd.printf("Created directory: %s\n", dirPath.c_str());
    } else {
      M5.Lcd.printf("Failed to create directory: %s\n", dirPath.c_str());
      return;
    }
  }

  for (int i = 0; i < 3; i++) {
    String filePath = dirPath + "/test_file_" + i + ".txt";
    File testFile = SD.open(filePath.c_str(), FILE_WRITE);
    if (testFile) {
      testFile.printf("This is content for test file %d.\n", i);
      testFile.close();
      M5.Lcd.printf("  Created: %s\n", filePath.c_str());
    } else {
      M5.Lcd.printf("  Failed to create: %s\n", filePath.c_str());
    }
  }
  M5.Lcd.println("Test files creation finished.");
}
#pragma region <text input>
int createFile(String fileNameToCreate, String targetDirectoryPath) {
  // SDカードが利用可能か確認
  if (!SD.begin()) {
    Serial.println("SD card initialization error.");
    // M5.Lcd.clear(); // 画面クリアのコメントアウトを有効にする場合は、適切な場所で呼び出す
    // M5.Lcd.setCursor(0, 0);
    // M5.Lcd.println("SD Error!");
    return 1; // SDカードエラーを示すコードを返す
  }

  // 作成するファイルのフルパスを構築
  // targetDirectoryPath がルートディレクトリ ("/") の場合とそうでない場合で結合方法を調整
  String fullPath;
  if (targetDirectoryPath.endsWith("/")) { // 例: "/MyFolder/" の場合
    fullPath = targetDirectoryPath + fileNameToCreate;
  } else if (targetDirectoryPath.length() > 0) { // 例: "/MyFolder" の場合
    fullPath = targetDirectoryPath + "/" + fileNameToCreate;
  } else { // 空文字列の場合、ルートディレクトリと見なす
    fullPath = "/" + fileNameToCreate;
  }

  Serial.print("Attempting to create file: ");
  Serial.println(fullPath);

  // ファイルの作成を試みる（書き込みモード "w" で開く）
  // SD.open() は、ファイルが存在しない場合は新規作成し、存在する場合は上書きします。
  File file = SD.open(fullPath, FILE_WRITE);

  if (file) {
    // ファイル作成（オープン）が成功した場合
    Serial.println("File created successfully: " + fullPath);
    // ここにファイルの初期内容を書き込むことができます。
    file.println("This is a new file created by M5Stack.");
    file.println("File name: " + fileNameToCreate);
    file.close(); // ファイルをクローズすることが重要！

    delay(1000); // 1秒間表示
    return 0; // 成功を示すコードを返す
  } else {
    // ファイル作成（オープン）が失敗した場合
    // (例: 指定されたパスが存在しない、メモリ不足、ファイル名が無効など)
    Serial.println("Failed to create file: " + fullPath);
    // M5.Lcd.clear(); // 画面クリア
    // M5.Lcd.setCursor(0, 0);
    // M5.Lcd.println("Failed!"); // 「Failed!」メッセージ
    return 2; // ファイル作成失敗を示すコードを返す
  }
}

int createDirectory(String SuperTT, String DirecXX) {
  // SDカードが利用可能か確認
  // SD.begin()はSDカードの初期化を試み、成功すればtrueを返す
  if (!SD.begin()) {
    // SDカードの初期化に失敗した場合の表示
    Serial.println("sd init error");
// 画面をクリア
    return 1; // SDカードエラーを示すコードを返す
  }

  // 作成するディレクトリのフルパスを構築
  // 例: "/data" + "/" + "logs" -> "/data/logs"
  String fullPath =  "/" + DirecX + SuperT;

  // ディレクトリ作成中のメッセージを表示
  

  // ディレクトリの作成を試みる
  // SD.mkdir()はディレクトリの作成を試み、成功すればtrueを返す
  if (SD.mkdir(fullPath)) {
    // ディレクトリ作成が成功した場合の表示
    
    delay(1000); // 1秒間表示
   
    return 0; // 成功を示すコードを返す
  } else {
    // ディレクトリ作成が失敗した場合の表示
    // (例: 容量オーバー、無効なパス、既に存在する場合など)

    // 画面中央に「Failed!」メッセージを表示
    Serial.println("something error:" + fullPath);
    return 2; // ディレクトリ作成失敗を示すコードを返す
  }
}

// Helper function to get character pixel width for font size 3
// フォントサイズ3での文字のピクセル幅を取得するヘルパー関数
int getCharWidth() {
    // M5.Lcd.setTextSize(3) は textluck() の先頭で設定されるため、ここでは省略
    return M5.Lcd.textWidth("A"); // モノスペースフォントを仮定
}

// Helper function to get font height for font size 3
// フォントサイズ3でのフォントの高さを取得するヘルパー関数
int getFontHeight() {
    // M5.Lcd.setTextSize(3) は textluck() の先頭で設定されるため、ここでは省略
    return M5.Lcd.fontHeight();
}



CursorPosInfo calculateCursorPixelPos(int index, const String& text) {
    int currentX = 0;
    int currentY = 0;
    int lineNum = 0;
    int charInLine = 0;
    int charWidth = getCharWidth();
    int fontHeight = getFontHeight();

    for (int i = 0; i < index; ++i) {
        if (i >= text.length()) break; // インデックスが有効でない場合はループを抜ける

        char c = text.charAt(i);
        if (c == '\n') {
            currentX = 0;
            currentY += fontHeight;
            lineNum++;
            charInLine = 0;
        } else {
            currentX += charWidth;
            charInLine++;
        }
    }
    return {currentX, currentY, lineNum, charInLine};
}

// Function to update offsetX and offsetY based on cursor position
// カーソル位置に基づいて offsetX と offsetY を更新する関数
void adjustScroll() {
    int screenWidth = M5.Lcd.width();
    int screenHeight = M5.Lcd.height();
    int charWidth = getCharWidth();
    int fontHeight = getFontHeight();

    int oldOffsetX = offsetX;
    int oldOffsetY = offsetY;

    // Horizontal scroll (水平スクロール)
    // カーソルが画面の右端を超えた場合
    if (cursorPixelX - offsetX >= screenWidth - charWidth) {
        offsetX = cursorPixelX - (screenWidth - charWidth) + charWidth;
    }
    // カーソルが画面の左端を超えた場合
    else if (cursorPixelX - offsetX < 0) {
        offsetX = cursorPixelX;
        if (offsetX < 0) offsetX = 0; // offsetX が負にならないようにする
    }

    // Vertical scroll (垂直スクロール)
    // カーソルが画面の下端を超えた場合
    if (cursorPixelY - offsetY >= screenHeight - fontHeight) {
        offsetY = cursorPixelY - (screenHeight - fontHeight) + fontHeight;
    }
    // カーソルが画面の上端を超えた場合
    else if (cursorPixelY - offsetY < 0) {
        offsetY = cursorPixelY;
        if (offsetY < 0) offsetY = 0; // offsetY が負にならないようにする
    }

    // スクロールオフセットが変更された場合、再描画が必要
    if (oldOffsetX != offsetX || oldOffsetY != offsetY) {
        needsRedraw = true;
    }
}

// Helper struct for line information
// 行情報のためのヘルパーストラクト
struct LineInfo;

// Gets information about the line containing the given index.
// This function assumes index is a valid cursor position (0 to text.length()).
// 指定されたインデックスを含む行の情報を取得します。
// この関数は、インデックスが有効なカーソル位置（0からtext.length()）であることを前提とします。
LineInfo getCurrentLineInfo(int index, const String& text) {
    if (text.length() == 0) return {0, 0};

    int lineStart = 0;
    // Find the start of the line containing or preceding 'index'
    // If index is at the very beginning of the string (0), lineStart remains 0.
    // If index is after a newline, lineStart becomes the character after that newline.
    // If index is at a newline, it's considered the end of the previous line.
    // The loop finds the last newline before or at 'index'.
    for (int i = index; i >= 0; --i) {
        if (text.charAt(i) == '\n') {
            lineStart = i + 1;
            break;
        }
    }

    int lineEnd = text.length();
    // Find the end of the line containing or following 'index'
    // The loop finds the first newline after 'lineStart'.
    for (int i = lineStart; i < text.length(); ++i) {
        if (text.charAt(i) == '\n') {
            lineEnd = i;
            break;
        }
    }
    return {lineStart, lineEnd - lineStart};
}

// Gets information about the line *before* the line containing currentIndex.
// If currentIndex is on the first line, returns {0, 0}.
LineInfo getPreviousLineInfo(int currentIndex, const String& text) {
    if (currentIndex == 0) {
        return {0, 0}; // Already at the beginning, no previous line
    }

    // Find the start of the current line
    int currentLineStart = 0;
    for (int i = currentIndex - 1; i >= 0; --i) {
        if (text.charAt(i) == '\n') {
            currentLineStart = i + 1;
            break;
        }
    }

    if (currentLineStart == 0) { // Current line is the first line
        return {0, 0}; // No previous line
    }

    // The character before currentLineStart is the newline of the previous line.
    int prevLineEnd = currentLineStart - 1; // This is the '\n' char
    
    int prevLineStart = 0;
    for (int i = prevLineEnd - 1; i >= 0; --i) {
        if (text.charAt(i) == '\n') {
            prevLineStart = i + 1;
            break;
        }
    }
    return {prevLineStart, prevLineEnd - prevLineStart};
}

// Gets information about the line *after* the line containing currentIndex.
// If currentIndex is on the last line, returns {text.length(), 0}.
LineInfo getNextLineInfo(int currentIndex, const String& text) {
    // Find the end of the current line (first newline after currentIndex)
    int currentLineEnd = text.length();
    for (int i = currentIndex; i < text.length(); ++i) {
        if (text.charAt(i) == '\n') {
            currentLineEnd = i;
            break;
        }
    }

    int nextLineStart = currentLineEnd + 1;
    if (nextLineStart > text.length()) { // No next line (index is already at or past the last char)
        return {text.length(), 0}; // Return info for an empty line at the very end
    }

    // Find the end of the next line
    int nextLineEnd = text.length();
    for (int i = nextLineStart; i < text.length(); ++i) {
        if (text.charAt(i) == '\n') {
            nextLineEnd = i;
            break;
        }
    }
    return {nextLineStart, nextLineEnd - nextLineStart};
}

// Helper function to find the character index on a line closest to a target pixel X position
// ターゲットピクセルX位置に最も近い行上の文字インデックスを見つけるヘルパー関数
int findIndexFromPixelX(int targetPixelX, int lineStartIndex, int lineLength, const String& text) {
    int charWidth = getCharWidth();
    int closestIndex = lineStartIndex;
    // ターゲットピクセルXと行の開始位置の差を初期の最小差とする
    int minDiff = abs(targetPixelX - 0); 

    for (int i = 0; i <= lineLength; ++i) { // 行の末尾（文字の直後）も考慮するため <= length
        int currentX = i * charWidth;
        int diff = abs(targetPixelX - currentX);
        if (diff < minDiff) {
            minDiff = diff;
            closestIndex = lineStartIndex + i;
        }
    }
    return closestIndex;
}


// Helper function to perform arrow key actions
// 矢印キーのアクションを実行するヘルパー関数
void performArrowKeyAction(const String& key) {
    // 現在のカーソルのピクセルX位置をターゲットとして保持
    int targetPixelX = cursorPixelX; 
    int newCursorIndex = cursorIndex;

    if (key == "UP") {
        LineInfo prevLine = getPreviousLineInfo(cursorIndex, SuperT);
        if (prevLine.length == 0 && prevLine.startIndex == 0 && getCurrentLineInfo(cursorIndex, SuperT).startIndex == 0) {
            // すでに最初の行にいる場合 (かつ現在の行も最初の行)
            newCursorIndex = 0;
        } else {
            // 前の行の適切なインデックスを、現在の水平位置に近い場所で検索
            newCursorIndex = findIndexFromPixelX(targetPixelX, prevLine.startIndex, prevLine.length, SuperT);
        }
    } else if (key == "DOWN") {
        LineInfo nextLine = getNextLineInfo(cursorIndex, SuperT);
        if (nextLine.length == 0 && nextLine.startIndex == SuperT.length() && (getCurrentLineInfo(cursorIndex, SuperT).startIndex + getCurrentLineInfo(cursorIndex, SuperT).length == SuperT.length())) {
            // すでに最後の行にいる場合 (かつ現在の行も最後の行)
            newCursorIndex = SuperT.length();
        } else {
            // 次の行の適切なインデックスを、現在の水平位置に近い場所で検索
            newCursorIndex = findIndexFromPixelX(targetPixelX, nextLine.startIndex, nextLine.length, SuperT);
        }
    } else if (key == "LEFT") {
        if (cursorIndex > 0) {
            cursorIndex--;
            // 改行文字を越えて左に移動した場合、前の行の末尾に移動
            if (cursorIndex > 0 && SuperT.charAt(cursorIndex) == '\n') {
                cursorIndex--;
            }
        }
    } else if (key == "RIGHT") {
        if (cursorIndex < SuperT.length()) {
            cursorIndex++;
            // 改行文字を越えて右に移動した場合、次の行の先頭に移動
            // この条件は、カーソルが改行文字の直前にあり、右に移動したときに
            // その改行文字を飛び越えて次の行の先頭に位置する場合に適用されるべき
            // 現在のロジックでは、cursorIndex++ の後、cursorIndex-1 が改行文字の場合に何もしない
            // これは、改行文字の直後にカーソルを置くという意図と一致する
            if (cursorIndex <= SuperT.length() && SuperT.charAt(cursorIndex - 1) == '\n') {
                // 何もしない。cursorIndexはすでに改行文字の次を指している
            }
        }
    }
    
    // 新しいカーソルインデックスを適用
    // UP/DOWNキーの場合のみnewCursorIndexを適用し、LEFT/RIGHTは直接cursorIndexを操作する
    if (key == "UP" || key == "DOWN") {
        cursorIndex = newCursorIndex;
    }

    // カーソルインデックスが範囲内にあることを確認
    if (cursorIndex < 0) cursorIndex = 0;
    if (cursorIndex > SuperT.length()) cursorIndex = SuperT.length();

    // 矢印キーのアクション後、カーソル位置が変更された可能性があるので、再計算してスクロールを調整
    // adjustScroll()内でneedsRedrawがtrueに設定される可能性がある
    CursorPosInfo currentCursorInfo = calculateCursorPixelPos(cursorIndex, SuperT);
    cursorPixelX = currentCursorInfo.pixelX;
    cursorPixelY = currentCursorInfo.pixelY;
    adjustScroll();
}


// SDカード上のファイルまたはディレクトリをリネームする関数
// String入力、bool出力（エラー用）
bool renameSDItem(String oldPath, String newPath) {
  M5.Lcd.printf("Renaming: %s to %s\n", oldPath.c_str(), newPath.c_str());
  Serial.println("Renaming: " + oldPath + " to " + newPath);
  if (SD.rename(oldPath.c_str(), newPath.c_str())) {
    Serial.println("  Rename successful.");
    return true; // 成功
  } else {
    Serial.println("  Rename failed!");
    return false; // 失敗
  }
}

int deleteRightmostSDItem(String itemPath) {
  // 1. パスが空でないか確認
  if (itemPath.length() == 0) {
    Serial.println("Error: Path is empty.");
    return 2; // パスが空
  }

  // 2. SDカードが初期化されているか確認
  // SD.begin() は一度しか呼び出す必要がないため、ここではチェックのみ行う
  // もしSD.begin()がこの関数の前に呼び出されていない場合、ここで呼び出す必要があるかもしれません。
  // ただし、listSDRootContentsの前に呼び出されていることを前提とします。
  // 必要であれば、ここでSD.begin()を再度呼び出すロジックを追加してください。
  // 例: if (!SD.begin(GPIO_NUM_4, SPI, 20000000)) { return 1; }

  // 3. アイテムのタイプを判断し、削除
  File item = SD.open(itemPath);
  if (!item) {
    Serial.println("Error: Item not found or cannot be opened: " + itemPath);
    return 3; // アイテムが見つからない、または開けない
  }

  bool isDir = item.isDirectory();
  item.close(); // ファイル/ディレクトリを開いた後は閉じる

  bool deleteSuccess = false;
  if (isDir) {
    // ディレクトリの場合
    // ディレクトリが空でないとrmdirは失敗するので注意
    deleteSuccess = SD.rmdir(itemPath);
    if (!deleteSuccess) {
      Serial.println("Error: Failed to remove directory (might not be empty): " + itemPath);
    }
  } else {
    // ファイルの場合
    deleteSuccess = SD.remove(itemPath);
    if (!deleteSuccess) {
      Serial.println("Error: Failed to remove file: " + itemPath);
    }
  }

  if (deleteSuccess) {
    Serial.println("Successfully deleted: " + itemPath);
    return 0; // 成功
  } else {
    return 4; // 削除失敗
  }
}




void kanketu(String texx,int frame){
   M5.Lcd.fillScreen(BLACK); // 画面を黒でクリア
    M5.Lcd.setTextColor(WHITE); // テキストの色を白に設定
    M5.Lcd.setTextFont(1); // フォントサイズを2に設定
    M5.Lcd.setCursor(0, 0); // カーソルを
    M5.Lcd.print(texx); // テキストを表示
    delay(frame);
    M5.Lcd.fillScreen(BLACK); // 画面を黒でクリア
    M5.Lcd.setTextFont(File_goukeifont); // フォントサイズを元に戻す
}


/**
 * @brief Retrieves the value of a variable by name as a String from a vector of loaded MettVariableInfo.
 * @param variables The vector of MettVariableInfo to search.
 * @param varName The name of the variable to search for.
 * @return The value String if found, an empty String otherwise.
 */
String getVariableString(const std::vector<MettVariableInfo>& variables, const String& varName) {
    for (const auto& var : variables) {
        if (var.variableName == varName) {
            return var.valueString;
        }
    }
    return "";
}


/**
 * @brief Retrieves the value of a variable by name as an int from a vector of loaded MettVariableInfo.
 * @param variables The vector of MettVariableInfo to search.
 * @param varName The name of the variable to search for.
 * @return The converted int value if found, 0 otherwise.
 */
int getVariableInt(const std::vector<MettVariableInfo>& variables, const String& varName) {
    String value = getVariableString(variables, varName);
    return value.toInt();
}

/**
 * @brief Retrieves the value of a variable by name as a char from a vector of loaded MettVariableInfo.
 * @param variables The vector of MettVariableInfo to search.
 * @param varName The name of the variable to search for.
 * @return The first character of the value if found, '\0' otherwise.
 */
char getVariableChar(const std::vector<MettVariableInfo>& variables, const String& varName) {
    String value = getVariableString(variables, varName);
    if (value.length() > 0) {
        return value.charAt(0);
    }
    return '\0';
}

/**
 * @brief Retrieves the value of a variable by name as a double from a vector of loaded MettVariableInfo.
 * @param variables The vector of MettVariableInfo to search.
 * @param varName The name of the variable to search for.
 * @return The converted double value if found, 0.0 otherwise.
 */
double getVariableDouble(const std::vector<MettVariableInfo>& variables, const String& varName) {
    String value = getVariableString(variables, varName);
    return value.toDouble();
}

/**
 * @brief Retrieves the comma-separated value of a variable by name as a vector of Strings.
 * @param variables The vector of MettVariableInfo to search.
 * @param varName The name of the variable to search for.
 * @return A vector of value strings if found, an empty vector otherwise.
 */
std::vector<String> getVariableStringArray(const std::vector<MettVariableInfo>& variables, const String& varName) {
    std::vector<String> result;
    String value = getVariableString(variables, varName);
    if (!value.isEmpty()) {
        int lastIndex = 0;
        int commaIndex;
        while ((commaIndex = value.indexOf(',', lastIndex)) != -1) {
            String element = value.substring(lastIndex, commaIndex);
            element.trim();
            result.push_back(element);
            lastIndex = commaIndex + 1;
        }
        String lastElement = value.substring(lastIndex);
        lastElement.trim();
        result.push_back(lastElement);
    }
    return result;
}

/**
 * @brief Retrieves the comma-separated value of a variable by name as a vector of ints.
 * @param variables The vector of MettVariableInfo to search.
 * @param varName The name of the variable to search for.
 * @return A vector of value ints if found, an empty vector otherwise.
 */
std::vector<int> getVariableIntArray(const std::vector<MettVariableInfo>& variables, const String& varName) {
    std::vector<int> result;
    std::vector<String> stringArray = getVariableStringArray(variables, varName);
    for (const auto& s : stringArray) {
        result.push_back(s.toInt());
    }
    return result;
}

/**
 * @brief Retrieves the comma-separated value of a variable by name as a vector of chars.
 * @param variables The vector of MettVariableInfo to search.
 * @param varName The name of the variable to search for.
 * @return A vector of value chars if found, an empty vector otherwise.
 */
std::vector<char> getVariableCharArray(const std::vector<MettVariableInfo>& variables, const String& varName) {
    std::vector<char> result;
    std::vector<String> stringArray = getVariableStringArray(variables, varName);
    for (const auto& s : stringArray) {
        if (s.length() > 0) {
            result.push_back(s.charAt(0));
        } else {
            result.push_back('\0');
        }
    }
    return result;
}

/**
 * @brief Retrieves the comma-separated value of a variable by name as a vector of doubles.
 * @param variables The vector of MettVariableInfo to search.
 * @param varName The name of the variable to search for.
 * @return A vector of value doubles if found, an empty vector otherwise.
 */
std::vector<double> getVariableDoubleArray(const std::vector<MettVariableInfo>& variables, const String& varName) {
    std::vector<double> result;
    std::vector<String> stringArray = getVariableStringArray(variables, varName);
    for (const auto& s : stringArray) {
        result.push_back(s.toDouble());
    }
    return result;
}

/**
 * @brief Converts a string to a boolean value.
 * @param valueString The string to convert.
 * @return The converted boolean value.
 */
bool stringToBool(const String& valueString) {
    String lowerCase = valueString;
    lowerCase.toLowerCase();
    return (lowerCase == "true" || lowerCase == "1" || lowerCase == "yes");
}

/**
 * @brief Creates the necessary directory and file.
 * @param filePath The full path of the file to check for existence and create.
 * @return bool True if initialization is successful, false otherwise.
 */
bool initializeSDCardAndCreateFile(const String& filePath) {
    if (filePath.isEmpty() || !filePath.startsWith("/")) {
        Serial.println("Error: Invalid file path.");
        return false;
    }
    int lastSlash = filePath.lastIndexOf('/');
    if (lastSlash == -1 || lastSlash == filePath.length() - 1) {
        Serial.println("Error: File path must include a directory and a file name.");
        return false;
    }
    String direcD = filePath.substring(0, lastSlash);
    if (!SD.exists(direcD) && !SD.mkdir(direcD)) {
        Serial.printf("Error: Failed to create directory: %s\n", direcD.c_str());
        return false;
    }
    if (!SD.exists(filePath)) {
        File file = SD.open(filePath.c_str(), FILE_WRITE);
        if (!file) {
            Serial.printf("Error: Failed to create file: %s\n", filePath.c_str());
            return false;
        }
        file.close();
    }
    Serial.printf("Info: File and directory prepared: %s\n", filePath.c_str());
    return true;
}

/**
 * @brief Prints the data saved in a specific table within the specified file.
 * @param fileName The full path of the file.
 * @param tableName The name of the table to print.
 * @param variables The vector of MettVariableInfo saved in the table.
 */
void printTable(const String& fileName, const String& tableName, const std::vector<MettVariableInfo>& variables) {
    Serial.printf("\n--- Data Summary for Table '%s' in File: %s ---\n", tableName.c_str(), fileName.c_str());
    if (variables.empty()) {
        Serial.printf("Info: No variables loaded for table '%s' in file '%s'.\n", tableName.c_str(), fileName.c_str());
    } else {
        for (const auto& var : variables) {
            Serial.printf("   - Variable: %s, Data Type: %s, Value: %s\n",
                          var.variableName.c_str(), var.dataType.c_str(), var.valueString.c_str());
        }
    }
    Serial.println("--------------------");
}

/**
 * @brief Prints a list of table names from all .mett files in the specified folder.
 * @param extractedDataList The vector of extracted FileMettData.
 */
void printFileM(const std::vector<FileMettData>& extractedDataList) {
    Serial.println("\n--- List of .mett Files and Tables in the Folder ---");
    if (extractedDataList.empty()) {
        Serial.println("Info: No .mett files found or no data extracted.");
    } else {
        for (const auto& fileData : extractedDataList) {
            Serial.printf("File: %s (Size: %u bytes)\n", fileData.fileName.c_str(), fileData.fileSize);
            std::set<String> uniqueTableNames;
            for (const auto& var : fileData.variables) {
                if (!var.tableName.isEmpty()) {
                    uniqueTableNames.insert(var.tableName);
                }
            }
            if (uniqueTableNames.empty()) {
                Serial.println("   - Tables: (none)");
            } else {
                for (const String& tableName : uniqueTableNames) {
                    Serial.printf("   - Table: %s\n", tableName.c_str());
                }
            }
            Serial.println("--------------------");
        }
    }
}

/**
 * @brief Joins elements of a std::vector into a comma-separated String.
 * @param vec The vector to join.
 * @return The joined String.
 */
template <typename T>
String joinVectorToString(const std::vector<T>& vec) {
    if (vec.empty()) {
        return "";
    }
    String result = "";
    for (size_t i = 0; i < vec.size(); ++i) {
        result += String(vec[i]);
        if (i < vec.size() - 1) {
            result += ",";
        }
    }
    return result;
}

/**
 * @brief Joins elements of a std::vector<String> into a comma-separated String.
 * @param vec The vector of Strings to join.
 * @return The joined String.
 */
String joinStringVectorToString(const std::vector<String>& vec) {
    if (vec.empty()) {
        return "";
    }
    String result = "";
    for (size_t i = 0; i < vec.size(); ++i) {
        result += vec[i];
        if (i < vec.size() - 1) {
            result += ",";
        }
    }
    return result;
}

// Corrected updateMenuDisplay(ril) function
void updateMenuDisplay(int ril) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.println("Initial execution text.");
}

// New function to copy a loaded vector of variables to a MettDataMap
MettDataMap copyVectorToMap(const std::vector<MettVariableInfo>& variables) {
    MettDataMap dataMap;
    for (const auto& var : variables) {
        dataMap[var.variableName] = var.valueString;
    }
    return dataMap;
}

/**
 * @brief テーブル名として有効な構文を持ち、かつ既存のテーブル名と重複しないかをチェックします。
 * (引数3つのシグネチャを維持)
 *
 * @param tableName チェックする新しいテーブル名 (String)
 * @param existingNames 既存の全テーブル名の配列 (String配列)
 * @param arraySize existingNames配列のサイズ
 * @return bool 有効な場合は true、無効な場合は false (構文エラーまたは重複)
 */
bool isValidTableName(const String& tableName, const String existingNames[], size_t arraySize) {
    // 1. 構文チェック
    if (tableName.length() == 0 || tableName.length() > 1000) return false;
    
    // 空白のみのチェック
    bool containsNonSpaceChar = false;
    for (int i = 0; i < tableName.length(); i++) {
        char c = tableName.charAt(i);
        if (c < 0 || c > 0x7F) { containsNonSpaceChar = true; break; }
        if (c > 0 && !isspace(c)) { containsNonSpaceChar = true; break; }
    }
    if (!containsNonSpaceChar) return false;

    // 禁止文字チェック (半角スペースも含む)
    const char* prohibited = "#$:&-,\\\n\r "; 
    for (int i = 0; i < tableName.length(); i++) {
        char c = tableName.charAt(i);
        if (strchr(prohibited, c) != NULL) return false;
    }
    
    // システム予約語チェック (大文字・小文字を無視)
    String upperName = tableName;
    upperName.toUpperCase();
    for (const String& reserved : reservedWords) {
        String upperReserved = reserved;
        upperReserved.toUpperCase();
        if (upperName.equals(upperReserved)) return false; 
    }

    // 2. 重複チェック (大小文字区別なし)
    for (size_t i = 0; i < arraySize; ++i) {
        // existingNames配列が宣言されたサイズまで初期化されていることを前提とします
        // 配列外アクセスを防ぐため、arraySizeのチェックは重要です。
        if (tableName.equalsIgnoreCase(existingNames[i])) return false; 
    }
    return true;
}

// ----------------------------------------------------------------------
// 2. getAllTableNamesInFile の高速化 (ブロック読み取りを使用)
// ----------------------------------------------------------------------

// containsInvalidTableNameChars の定義 (getAllTableNamesInFileの依存関係を解決)
bool containsInvalidTableNameChars(const String& tableName) {
    // isValidTableNameのロジックから禁止文字チェック部分を流用
    const char* prohibited = "#$:&-,\\\n\r "; 
    for (int i = 0; i < tableName.length(); i++) {
        char c = tableName.charAt(i);
        if (strchr(prohibited, c) != NULL) {
            return true;
        }
    }
    return false;
}

// 読み込みバッファサイズ


/**
 * @brief 指定された .mett ファイルに保存されているすべてのテーブル名を高速に取得します。
 * * **【高速化のポイント】**
 * 1. ファイル全体を固定バッファ (4KB) でブロック読み込み (File::read) します。
 * 2. `readStringUntil('\n')` を使用しないことで、I/Oと動的メモリ確保のオーバーヘッドを大幅に削減します。
 * * @param fs The SD card filesystem object.
 * @param fullFilePath The full path of the file to get table names from.
 * @return std::vector<String> A list of extracted unique table names.
 */
std::vector<String> getAllTableNamesInFile(fs::FS &fs, const String& fullFilePath, bool& isZero) {
    std::vector<String> tableNames;
    std::set<String> uniqueTableNames;
    
    // 戻り値パラメーターの初期化
    isZero = false; 

    if (!fullFilePath.endsWith(".mett") || !fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error: File not found or not a valid .mett file: %s\n", fullFilePath.c_str());
        isZero = true; // ファイルエラーもテーブルゼロと見なす場合
        return tableNames;
    }

    File file = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!file) {
        Serial.printf("Error: Failed to open file for reading: %s\n", fullFilePath.c_str());
        isZero = true;
        return tableNames;
    }

    char buffer[BUFFER_SIZE];
    String lineBuffer = ""; // バッファ境界を跨ぐ行を一時的に保持

    // ファイルの最後までブロック単位で読み込む (高速化処理)
    while (file.available()) {
        size_t bytesRead = file.read((uint8_t*)buffer, BUFFER_SIZE);
        
        // 前回の残り + 今回のデータを結合
        String currentBlock = lineBuffer;
        currentBlock.concat(buffer, bytesRead); 
        
        lineBuffer = ""; // リセット

        int start = 0;
        int end = 0;
        
        // ブロック内で改行を見つけ、行を処理
        while ((end = currentBlock.indexOf('\n', start)) != -1) {
            String line = currentBlock.substring(start, end);
            
            if (line.startsWith(TABLE_NAME_PATTERN)) {
                String tableName = line.substring(PATTERN_LEN);
                tableName.trim();
                
                if (!containsInvalidTableNameChars(tableName)) {
                    uniqueTableNames.insert(tableName);
                } 
            }
            start = end + 1; // 次の行へ
        }

        // 最後に残ったデータ（次のブロックへ引き継ぐ行の断片）を保持
        if (start < currentBlock.length()) {
            lineBuffer = currentBlock.substring(start);
        }
    }

    // 最終行の処理
    if (lineBuffer.startsWith(TABLE_NAME_PATTERN)) {
        String tableName = lineBuffer.substring(PATTERN_LEN);
        tableName.trim();
        if (!containsInvalidTableNameChars(tableName)) {
            uniqueTableNames.insert(tableName);
        }
    }
    
    file.close();

    // テーブル数がゼロの場合に isZero を true に設定
    if (uniqueTableNames.empty()) {
        isZero = true;
    }
    
    Serial.printf("Info: Found %d unique table names in file '%s'. isZero: %s\n", 
                  uniqueTableNames.size(), 
                  fullFilePath.c_str(), 
                  isZero ? "True" : "False");
    
    // setの内容をvectorにコピーして返す
    for (const auto& name : uniqueTableNames) {
        tableNames.push_back(name);
    }
    
    return tableNames;
}

/**
 * @brief Gets all variable names and data types from a specific table within a file.
 * @param fs The SD card filesystem object.
 * @param fullFilePath The full path of the file to load.
 * @param targetTableName The name of the table to extract variable info from.
 * @param variableNames Reference to a vector to store the extracted variable names.
 * @param dataTypes Reference to a vector to store the extracted data types.
 * @return bool True if the process is successful and the table is found, false otherwise.
 */
bool getVariablesFromTable(fs::FS &fs, const String& fullFilePath, const String& targetTableName, std::vector<String>& variableNames, std::vector<String>& dataTypes) {
    variableNames.clear();
    dataTypes.clear();
    bool tableFound = false;
    if (!fullFilePath.endsWith(".mett") || !fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error: File not found or not a valid .mett file: %s\n", fullFilePath.c_str());
        return false;
    }
    File file = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!file) {
        Serial.printf("Error: Failed to open file for reading: %s\n", fullFilePath.c_str());
        return false;
    }
    String currentTableNameInFile = "";
    bool inTargetTable = false;
    Serial.printf("Info: Searching for table '%s' in file '%s'.\n", targetTableName.c_str(), fullFilePath.c_str());
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.startsWith("--- NEW DATA SET ---")) {
            inTargetTable = false;
            currentTableNameInFile = "";
            continue;
        }
        if (line.startsWith("TABLE_NAME:")) {
            int colonIndex = line.indexOf(':');
            if (colonIndex != -1) {
                currentTableNameInFile = line.substring(colonIndex + 1);
                currentTableNameInFile.trim();
                if (currentTableNameInFile == targetTableName) {
                    inTargetTable = true;
                    tableFound = true;
                }
            }
            continue;
        }
        if (line.startsWith("#") || line.isEmpty()) {
            continue;
        }
        if (inTargetTable) {
            int firstColonIndex = line.indexOf(':');
            int secondColonIndex = line.indexOf(':', firstColonIndex + 1);
            if (firstColonIndex != -1 && secondColonIndex != -1) {
                String varName = line.substring(0, firstColonIndex);
                String dataType = line.substring(firstColonIndex + 1, secondColonIndex);
                varName.trim();
                dataType.trim();
                if (!containsInvalidVariableNameChars(varName)) {
                    variableNames.push_back(varName);
                    dataTypes.push_back(dataType);
                } else {
                    Serial.printf("Warning: Invalid variable name '%s' in table '%s'. Skipping.\n", varName.c_str(), targetTableName.c_str());
                }
            }
        }
    }
    file.close();
    if (tableFound) {
        Serial.printf("Info: Found and extracted %d variables from table '%s'.\n", variableNames.size(), targetTableName.c_str());
    } else {
        Serial.printf("Warning: Table '%s' was not found in file '%s'.\n", targetTableName.c_str(), fullFilePath.c_str());
    }
    return tableFound;
}

/**
 * @brief Prints all variables of type String and StringArray in a vector to the serial monitor.
 * @param variables The vector of MettVariableInfo to print from.
 */
void printAllStringsInVector(const std::vector<MettVariableInfo>& variables) {
    Serial.println("\n--- Printing all String/StringArray variables in the vector ---");
    if (variables.empty()) {
        Serial.println("Info: Vector is empty.");
        return;
    }
    for (const auto& var : variables) {
        if (var.dataType == "String" || var.dataType == "StringArray") {
            Serial.printf("Variable: %s, Data Type: %s, Value: %s\n",
                          var.variableName.c_str(), var.dataType.c_str(), var.valueString.c_str());
        }
    }
    Serial.println("----------------------------------------------");
}


bool deleteTableInFile(fs::FS &fs, const String& fullFilePath, const String& tableNameToDelete) {
    
    if (!fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error: File not found: %s\n", fullFilePath.c_str());
        return false;
    }
    
    String tempFilePath = fullFilePath + ".tmp";
    
    File inputFile = fs.open(fullFilePath.c_str(), FILE_READ);
    File tempFile = fs.open(tempFilePath.c_str(), FILE_WRITE);

    if (!inputFile || !tempFile) {
        Serial.printf("Error: Failed to open files. Input: %s, Temp: %s\n", 
                       inputFile ? "OK" : "FAIL", 
                       tempFile ? "OK" : "FAIL");
        if (tempFile) tempFile.close();
        if (fs.exists(tempFilePath.c_str())) fs.remove(tempFilePath.c_str()); 
        return false;
    }

    bool tableWasFound = false;
    bool isDeleting = false; 
    
    char buffer[BUFFER_SIZE];
    String lineBuffer = ""; 
    String targetNameUpper = tableNameToDelete;
    targetNameUpper.toUpperCase();

    while (inputFile.available()) {
        size_t bytesRead = inputFile.read((uint8_t*)buffer, BUFFER_SIZE);
        
        String currentBlock = lineBuffer;
        currentBlock.concat(buffer, bytesRead); 
        
        lineBuffer = ""; 

        int start = 0;
        int end = 0;
        
        while ((end = currentBlock.indexOf('\n', start)) != -1) {
            String line = currentBlock.substring(start, end);
            
            if (line.startsWith(TABLE_NAME_PATTERN)) {
                String currentTableName = line.substring(PATTERN_LEN);
                currentTableName.trim();
                currentTableName.toUpperCase();
                
                if (currentTableName.equals(targetNameUpper)) {
                    isDeleting = true; 
                    tableWasFound = true;
                    // 削除モード中は行をスキップ
                } else {
                    isDeleting = false; 
                    tempFile.print(line);
                    tempFile.print('\n');
                }
            } else if (!isDeleting) {
                tempFile.print(line);
                tempFile.print('\n');
            }
            
            start = end + 1; 
        }

        if (start < currentBlock.length()) {
            lineBuffer = currentBlock.substring(start);
        }
    }

    if (lineBuffer.length() > 0) {
        if (lineBuffer.startsWith(TABLE_NAME_PATTERN)) {
            String currentTableName = lineBuffer.substring(PATTERN_LEN);
            currentTableName.trim();
            currentTableName.toUpperCase();

            if (!currentTableName.equals(targetNameUpper)) {
                isDeleting = false;
                if (!isDeleting) {
                    tempFile.print(lineBuffer);
                    tempFile.print('\n');
                }
            }
        } else if (!isDeleting) {
            tempFile.print(lineBuffer);
            tempFile.print('\n');
        }
    }
    
    inputFile.close();
    tempFile.close();

    if (tableWasFound) {
        Serial.printf("Info: Successfully removed table block for '%s'. Replacing original file.\n", tableNameToDelete.c_str());
    } else {
        Serial.printf("Warning: Table '%s' not found in file. No changes made.\n", tableNameToDelete.c_str());
        fs.remove(tempFilePath.c_str());
        return true; 
    }
    
    if (!fs.remove(fullFilePath.c_str())) {
        Serial.printf("Error: Failed to remove original file: %s\n", fullFilePath.c_str());
        fs.remove(tempFilePath.c_str()); 
        return false; 
    }
    
    if (!fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
        Serial.printf("Error: Failed to rename temporary file to original: %s\n", fullFilePath.c_str());
        return false; 
    }
    
    return true; 
}

/**
 * @brief Helper function to infer data type from a string value.
 * @param valueString The string value.
 * @return The inferred data type name (String).
 */
String inferDataType(const String& valueString) {
    if (valueString.isEmpty()) {
        return "String";
    }
    if (valueString.indexOf(',') != -1) {
        int commaIndex = 0;
        bool isIntArray = true;
        bool isDoubleArray = true;
        bool isStringArray = false;
        String tempValue = valueString;
        while (tempValue.length() > 0) {
            commaIndex = tempValue.indexOf(',');
            String element;
            if (commaIndex == -1) {
                element = tempValue;
                tempValue = "";
            } else {
                element = tempValue.substring(0, commaIndex);
                tempValue = tempValue.substring(commaIndex + 1);
            }
            element.trim();
            if (element.isEmpty()) {
                isStringArray = true;
                isIntArray = false;
                isDoubleArray = false;
                break;
            }
            if (element.indexOf('.') != -1) {
                isIntArray = false;
            }
            bool isNumber = true;
            for(size_t i = 0; i < element.length(); i++) {
                if (i == 0 && element.charAt(i) == '-') continue;
                if (!isdigit(element.charAt(i)) && element.charAt(i) != '.') {
                    isNumber = false;
                    break;
                }
            }
            if (!isNumber) {
                isIntArray = false;
                isDoubleArray = false;
                isStringArray = true;
                break;
            }
        }
        if (isStringArray) {
            return "StringArray";
        }
        if (isIntArray) {
            return "IntArray";
        }
        if (isDoubleArray) {
            return "DoubleArray";
        }
        return "StringArray";
    }

    bool isInt = true;
    bool isDouble = false;
    bool hasDecimal = false;
    if (valueString.isEmpty()) return "String";
    for(size_t i = 0; i < valueString.length(); i++) {
        if (i == 0 && valueString.charAt(i) == '-') continue;
        if (valueString.charAt(i) == '.') {
            if (hasDecimal) {
                isInt = false;
                isDouble = false;
                break;
            }
            hasDecimal = true;
            isDouble = true;
        } else if (!isdigit(valueString.charAt(i))) {
            isInt = false;
            isDouble = false;
            break;
        }
    }
    if (isInt && !hasDecimal) {
        return "int";
    }
    if (isDouble) {
        return "double";
    }
    return "String";
}

/**
 * @brief Helper function to check if a variable name contains invalid characters.
 * @param name The variable name to check.
 * @return true if invalid characters are found, otherwise false.
 */
bool containsInvalidVariableNameChars(const String& name) {
    for (size_t i = 0; i < name.length(); i++) {
        char c = name.charAt(i);
        if (c == ':' || c == '&' || c == ' ' || c == '#' || c == '-') {
            return true;
        }
    }
    return false;
}

bool formatSDCardFull(int formatType) {
    // ロギングをM5.Lcd.printからSerial.printに変更
    Serial.print("SD Card Formatting Request: ");
    bool result = false; // 最終的な成否を保持する変数

    // SPIバスをピン番号の直接入力で明示的に開始 (SCK, MISO, MOSI, CS)
    SDSPI.begin(39, 38, 40, 4);

    // SdFatによる物理的なカード初期化チェック（SPI設定を明示）
    if (!sd.cardBegin(SD_SPI_CONFIG)) {
        Serial.println("Failed to initialize SD card hardware. (Cannot format)");
        // フォーマットに失敗した場合、SPIバスは維持したままにする
        return false; 
    }
    
    // 可能な限り最大容量を取得 (SdFat v2: card()->sectorCount() * 512ULL を使用)
    // autoを使用して、明示的なFsCard型の参照を避ける
    auto cardPtr = sd.card(); 
    if (cardPtr == nullptr) {
        Serial.println("Hardware Error: Card pointer is NULL.");
        sd.end();
        return false;
    }
    uint64_t totalBytes = (uint64_t)cardPtr->sectorCount() * 512ULL;

    // ***************************************************************
    // sd.begin()（ボリュームのマウント）の成功をチェックせず、
    // cardBegin()が成功した時点で直ちにフォーマットを試みます。
    // ***************************************************************

    switch (formatType) {
        case FORMAT_FAT16:
            Serial.println("FAT16 (Full Capacity) - Attempting format...");
            // FAT16は最大4GBまでのパーティションにしか使用できません（ここでは2GB超をエラーとする）
            if (totalBytes > (2ULL * 1024 * 1024 * 1024)) {
                Serial.println("Error: FAT16 is unsuitable for this large capacity ( > 2GB).");
                result = false;
                break;
            }
            // SdFatは容量に基づいてFAT16/FAT32を自動選択するため、
            // 2GB以下の場合はFAT16が適用されることが期待されます。
            if (sd.format()) { 
                Serial.println("FAT16/Auto Format SUCCESS.");
                result = true;
            } else {
                Serial.println("FAT16/Auto Format FAILED.");
                result = false;
            }
            break;
            
        case FORMAT_FAT32:
            Serial.println("FAT32 (Full Capacity) - Attempting format...");
            // SdFatは容量に基づいてFAT16/FAT32を自動選択するため、
            // 2GB超の場合はFAT32が適用されることが期待されます。
            if (sd.format()) { 
                Serial.println("FAT32/Auto Format SUCCESS.");
                result = true;
            } else {
                Serial.println("FAT32/Auto Format FAILED.");
                result = false;
            }
            break;

        default:
            Serial.println("Invalid format type specified. (Only FAT16/FAT32 supported)");
            result = false;
            break;
    }
    
    // -----------------------------------------------------------------------
    // !!! クリーンアップ: フォーマット処理後は安全のためSPIバスを解放する !!!
    // -----------------------------------------------------------------------
    Serial.println("Releasing SD resources...");
    sd.end(); 
    
    return result;
}

// ----------------------------------------------------------------------
// 3. パーティションを無視したカード全体の容量をStringリターンする関数
// ----------------------------------------------------------------------

/**
 * @brief SDカード全体の物理容量をGB, MB, KB単位でStringとして返します。（SdFat使用）
 * * @return String 容量情報 (例: "64.00 GB (67108864 KB, 65536.00 MB)")
 */
String getSDCardRawCapacity() {
    // SPIバスをピン番号の直接入力で明示的に開始 (SCK, MISO, MOSI, CS)
    SDSPI.begin(39, 38, 40, 4);

    // SdFatによる初期化チェック（SPI設定を明示）
    if (!sd.cardBegin(SD_SPI_CONFIG)) {
        return "SD Card Not Initialized (SdFat)";
    }

    // SdFatのカードオブジェクトからセクター数を取得し、512バイトを掛けて総バイト数を計算
    // autoを使用して、明示的なFsCard型の参照を避ける
    auto cardPtr = sd.card();
    if (cardPtr == nullptr) {
        // 情報取得関数ではsd.end()を呼び出さない（次のアクセスに備えるため）
        return "NO_CARD / Hardware Error";
    }
    uint64_t totalBytes = (uint64_t)cardPtr->sectorCount() * 512ULL;
    
    // 単位変換のための定数 (1024ベース)
    const double KB_SIZE = 1024.0;
    const double MB_SIZE = KB_SIZE * 1024.0;
    const double GB_SIZE = MB_SIZE * 1024.0; 

    // バイトから各単位に変換
    double totalGB = (double)totalBytes / GB_SIZE;
    double totalMB = (double)totalBytes / MB_SIZE;
    double totalKB = (double)totalBytes / KB_SIZE;

    String result = "";

    // メインの表示単位を決定 (GB > MB > KB の優先度)
    if (totalGB >= 1.0) {
        // 1GB以上の場合はGB単位で表示 (小数点第2位まで)
        result += String(totalGB, 2) + " GB";
    } else if (totalMB >= 1.0) {
        // 1MB以上の場合はMB単位で表示
        result += String(totalMB, 2) + " MB";
    } else {
        // それ以下の場合はKB単位で表示
        result += String(totalKB, 0) + " KB";
    }

    // すべての単位での詳細情報も追加
    result += " (" + String(totalKB, 0) + " KB, ";
    result += String(totalMB, 2) + " MB)";

    // sd.end()を削除し、SPIバスのリソースを維持 (連続アクセスを可能にするため)

    return result;
}

// ----------------------------------------------------------------------
// 4. SDカードの規格タイプとフォーマット形式を判定する関数
// ----------------------------------------------------------------------

/**
 * @brief SDカードの規格タイプとファイルシステム形式を判定し、一つのStringとして返します。
 * * 規格タイプ / フォーマット形式 (例: "SDHC / FAT32")
 * * CID情報は含まない
 * * @return String 規格タイプとフォーマット情報
 */
String getSDCardType() {
    // SPIバスをピン番号の直接入力で明示的に開始 (SCK, MISO, MOSI, CS)
    SDSPI.begin(39, 38, 40, 4);

    // 1. 物理カード初期化チェック
    if (!sd.cardBegin(SD_SPI_CONFIG)) {
        return "NO_CARD / Init Failed";
    }

    // --- 容量に基づく規格タイプ判定 ---
    // autoを使用して、明示的なFsCard型の参照を避ける
    auto cardPtr = sd.card();
    if (cardPtr == nullptr) {
        // 情報取得関数ではsd.end()を呼び出さない（連続アクセスを可能にするため）
        return "NO_CARD / Hardware Error";
    }
    
    uint64_t totalBytes = (uint64_t)cardPtr->sectorCount() * 512ULL;
    String typeString = ""; // 規格タイプを格納

    // 容量閾値の定義 (バイト単位)
    const uint64_t GB_UNIT = 1024ULL * 1024 * 1024;
    const uint64_t THRESHOLD_2GB = 2ULL * GB_UNIT;
    const uint64_t THRESHOLD_32GB = 32ULL * GB_UNIT;
    const uint64_t THRESHOLD_2TB = 2000ULL * GB_UNIT; 

    // 容量に基づく規格判定
    if (totalBytes > THRESHOLD_2TB) {
        typeString = "SDUC";
    } else if (totalBytes > THRESHOLD_32GB) {
        typeString = "SDXC";
    } else if (totalBytes > THRESHOLD_2GB) {
        typeString = "SDHC";
    } else {
        typeString = "SDSC";
    }

    // 3. フォーマット形式判定（ファイルシステム層）
    String formatString = "";

    // ボリューム/ファイルシステムの初期化を試みる
    if (sd.begin()) { 
        // sd.begin()が成功した場合（FAT16/32/exFAT）
        int fatType = sd.fatType();
        switch (fatType) {
            case 16:
                formatString = "FAT16";
                break;
            case 32:
                formatString = "FAT32";
                break;
            case 64:
                formatString = "exFAT";
                break;
            default:
                formatString = "Unknown FAT Type";
                break;
        }
    } else {
        // sd.begin()が失敗した場合（未フォーマット、破損、または非サポートFS）
        formatString = "Unformatted/Corrupt/Non-FAT";
    }
    
    // sd.end()を削除し、SPIバスのリソースを維持

    // 規格タイプとフォーマット形式を結合して返す
    return typeString + " / " + formatString;
}

// ----------------------------------------------------------------------
// 4-B. SDカードのCID情報を取得する関数
// ----------------------------------------------------------------------

/**
 * @brief SDカードのCIDレジスタからモデル名、製造日時、シリアル番号、その他の識別情報を取得します。
 * * @return String CID情報 (モデル名、製造日時、シリアル番号、その他識別子を含む)
 */
String getSDCardCIDInfo() {
    // SPIバスをピン番号の直接入力で明示的に開始 (SCK, MISO, MOSI, CS)
    SDSPI.begin(39, 38, 40, 4);

    // 1. 物理カード初期化チェック
    if (!sd.cardBegin(SD_SPI_CONFIG)) {
        return "- Model(PNM): N/A\n- Mfg. Date: N/A\n- Serial Num: N/A\n(Card Init Failed)";
    }

    // --- CID情報取得 ---
    cid_t cidData; 
    
    // CID読み取りを試みる 
    if (!sd.card()->readCID(&cidData)) {
        // sd.end()を削除し、SPIバスのリソースを維持
        return "- Model(PNM): N/A\n- Mfg. Date: N/A\n- Serial Num: N/A\n(CID Read Failed)";
    }
    
    // CIDフィールドの抽出と整形
    
    // 1. 製造業者ID (MID: Manufacturer ID) - 16進数で表示
    uint8_t mid = cidData.mid;
    
    // 2. OEM/アプリケーションID (OID: OEM/Application ID) - ASCII文字列で表示
    char oid_buf[3]; 
    memcpy(oid_buf, cidData.oid, 2);
    oid_buf[2] = '\0';
    String oidString = String(oid_buf);
    
    // 3. 製品名 (PNM: Product Name) - ASCII文字列で表示 (5文字)
    char pnm_buf[6]; 
    memcpy(pnm_buf, cidData.pnm, 5); 
    pnm_buf[5] = '\0';
    String modelNameString = String(pnm_buf);
    
    // 4. 製品リビジョン (PRV: Product Revision) - メジャー.マイナー形式で表示
    uint8_t prv = cidData.prv; 
    int majorRev = (prv >> 4) & 0xF;
    int minorRev = prv & 0xF;
    String revString = String(majorRev) + "." + String(minorRev);
    
    // 5. 製品シリアル番号 (PSN: Product Serial Number) - 16進数で表示
    // cidData.psn から cidData.psn() へ修正
    uint32_t serialNum = cidData.psn(); 

    // 6. 製造日時 (MDT: Manufacturing Date) - 年/月形式で表示
    uint16_t mdtRaw = (cidData.mdt[0] << 8) | cidData.mdt[1];

    // SD規格のMDTフィールドから年と月を抽出
    int year_raw = (mdtRaw >> 4) & 0xF; 
    int month_raw = mdtRaw & 0xF;      

    int mfgYear = 2000 + year_raw;
    int mfgMonth = month_raw; 

    // 月の値が不正な場合はエラー表示
    if (mfgMonth < 1 || mfgMonth > 12) {
        mfgMonth = 0; 
    }
    
    String mfgDateString = String(mfgYear) + "/" + (mfgMonth < 10 ? "0" : "") + String(mfgMonth);
    // 月が不正な場合は "?" を表示
    if (mfgMonth == 0) {
        mfgDateString = String(mfgYear) + "/??";
    }


    // sd.end()を削除し、SPIバスのリソースを維持

    // CID情報をまとめて返す
    String result = "- MID (Mfg ID): 0x" + String(mid, HEX);
    result += "\n- OID (OEM ID): " + oidString;
    result += "\n- PNM (Model): " + modelNameString;
    result += "\n- PRV (Revision): " + revString;
    result += "\n- PSN (Serial Num): 0x" + String(serialNum, HEX); 
    result += "\n- MDT (Mfg. Date): " + mfgDateString;
    
    return result;
}

// ----------------------------------------------------------------------
// 5. FAT16/FAT32以外のフォーマット判定関数
// ----------------------------------------------------------------------

/**
 * @brief SDカードが以下のいずれかの異常な状態にある場合にtrueを返します。
 * 1. FAT16/FAT32以外のファイルシステムでフォーマットされている (例: exFAT, NTFS)。
 * 2. 有効な単一ボリュームが認識できない (データ破綻、未フォーマット、パーティション0個/異常なパーティション構成)。
 * * @return true FAT16/FAT32以外の形式または異常な状態である, false FAT16/FAT32である
 */
bool isNonFAT16orFAT32Format() {
    Serial.print("Checking Format Type/Health... "); 
    
    // SPIバスをピン番号の直接入力で明示的に開始 (SCK, MISO, MOSI, CS)
    SDSPI.begin(39, 38, 40, 4);

    // 1. 物理カードの初期化チェック（SPI設定を明示）
    if (!sd.cardBegin(SD_SPI_CONFIG)) {
        Serial.println("Card Init Failed (No Card/Physical Error)."); 
        return true; // 破綻として扱う
    }
    
    // 2. ボリューム/ファイルシステムの初期化を試みる
    if (!sd.begin()) { 
        Serial.println("Volume/FS Not Recognized (Corrupt/Non-standard Partition or non-FAT/exFAT)."); 
        // sd.end()を削除し、SPIバスのリソースを維持
        return true; // 破綻/パーティション異常として扱う
    }
    // ここでsd.begin()が成功したということは、SdFatが読み取れる有効なFAT/exFATボリュームがマウントされたと見なせます。

    // 3. FATタイプをチェック
    int fatType = sd.fatType();
    
    // sd.end()を削除し、SPIバスのリソースを維持

    // SdFatでは FAT16->16, FAT32->32, exFAT->64 (FAT64) と判定される
    if (fatType == 16 || fatType == 32) {
        // FAT16 または FAT32 である
        Serial.printf("Format is FAT%d (OK).\n", fatType); 
        return false;
    } else {
        // exFAT (64) やその他の認識できない/非標準のFATタイプの場合
        Serial.printf("Format is Non-FAT16/32: Type %d.\n", fatType); 
        return true; 
    }
}

// ----------------------------------------------------------------------
// 6. SdFatリソースを解放し、他のSDライブラリに切り替える準備をする関数
// ----------------------------------------------------------------------

/**
 * @brief SdFs (SdFat) のリソースを解放し、SPIバスをクリーンな状態に戻します。
 * * この関数実行後、標準のSD.hなど他のSDライブラリでbegin()を呼び出すことが可能になります。
 */
void releaseSdFatAndPrepareForSDLibrary() {
    Serial.println("--- Switching SD Library Context ---");
    // 1. SdFs (SdFat) オブジェクトのリソースを解放
    // これにより、SdFatが使用していたSPIバスのCSピンの制御が解除されます。
    sd.end();
    SD.end(); // 念のため、標準SDライブラリのend()も呼び出す
    Serial.println("SdFs (SdFat) resources released.");

    // 2. 専用のSDSPIインスタンスを終了し、SPIポートのリソースを解放
    // これにより、SDカードとの通信に使用していたSPIバスの物理リソースが解放され、他のライブラリが使用できるようになります。
    SDSPI.end(); 
    Serial.println("Dedicated SDSPI instance (SPI Port) terminated.");
    
    // 3. (SD.hライブラリの明示的な終了は、SD.hをインポートし、SD.begin()が呼び出されている場合にのみ可能であり、
    // このファイルはSdFat専用のため、専用のSDSPIの終了によってポートはクリーンになります。)
    
    Serial.println("Ready to initialize with standard SD.h or other libraries.");
    Serial.println("------------------------------------");
}
