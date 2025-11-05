//SD書き込み，内蔵フラッシュ読み書き，内蔵フラッシュの再起動時にRAMのように削除される変数の策定によるフラッシュ越しのリアルタイム通信
//内蔵フラッシュのデータのSDバックアップ自動処理，そしてデバイスからSDとフラッシュ両方に読み書き可能にする機能追加






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
#include <SPIFFS.h>
#include <SPI.h> 
#include "shares.h"
int NUM_RESERVED_NAMES;
const char* TABLE_ID = "--- METT BLOCK ---";
SPIClass SDSPI; 
bool nosd;
const int SD_MISO_PIN = 35;
const int SD_MOSI_PIN = 37;
const int SD_SCK_PIN = 36; // SCKピン番号は39
bool serious_errorsd;
String potlist[] = {"fileext","stringtype","delete broken data","sort type","wifipasstype","back"};
String karadirectname;
int numMenuItems = 6;
int currentPos;
int goukei_page;

bool otroot;
bool rootnofile;
String SuperT = ""; // 入力されたテキストを保持する文字列
int cursorIndex = 0; // SuperT内の現在のカーソル位置（文字インデックス）
int cursorPixelX = 0; // カーソルのX座標（ピクセル）
int cursorPixelY = 0; // カーソルのY座標（ピクセル）
int offsetX = 0; // テキスト描画の水平オフセット（スクロール用）
int offsetY = 0; // テキスト描画の垂直オフセット（スクロール用）
int scrollpx = 50;
bool needsRedraw = false;

SdFs sd;
// SPI設定構造体を定義 (SdSpiConfig(CSピン, 最大速度, モード, SPIインスタンス))
// SD_SCK_MHZ(50)はSdFatのヘルパーマクロで、50MHzのクロックスピードを設定します。
const SdSpiConfig SD_SPI_CONFIG = SdSpiConfig(SD_CS_PIN, SD_SCK_MHZ(20), SPI_MODE0, &SDSPI);
const std::vector<String> reservedWords = {
    "CONFIG", "METT", "VERSION", "TABLE", "OPTION", "DEFAULT",
    "ENCRYPT", "NOTREAD", "REQUIRED", "NULL", "TRUE", "FALSE",
    "READ_ONLY", "ENCRYPTED", "TABLE_KEY", "USRDATA",
    "MAX_ROWS", "STORAGE", "INITIAL_DATA", "PRIMARY_KEY", 
    "SCHEMA_VERSION", "MIN_VERSION", "ACCESS", 
    "FORCE_LAST_MODIFIED", "FORCE_LAST_USER"
};

const std::vector<String> reservedHensuName;

bool boolmax(){
  Serial.println("maxLinesPerPage3:" + String(maxLinesPerPage3) + "positpoint:" + String(positpoint) + "maxLinesPerPage:" + String(maxLinesPerPage)    );
  Serial.println( "G" + String(imano_page) + "H" + String(maxpage));
  return (maxLinesPerPage3 != 0 && positpoint == maxLinesPerPage3 - 1) || (maxLinesPerPage3 == 0 && positpoint == maxLinesPerPage - 1);
}


// ポインターの位置を更新し、画面下部にテキストをスクロールさせる関数
void updatePointer(bool notext) {
    if(notext){
      delay(1);
    }
    if(DirecX != "/" && mainmode == 1){
        modordir = true;
    }else{
        modordir = false;
    }
    // 以前のポインター位置を記憶 (-1は初期状態を示す。これはstaticで一度だけ初期化される)
    static int prev_positpoint = -1;
    // 関数呼び出し時点のpositpoint（ボタン押下前のpositpoint）を保存
    int current_positpoint_on_entry = positpoint; 
    
    // ポインター表示のフォントをFile_goukeifontに固定
    M5.Lcd.setTextFont(File_goukeifont);
    if (M5.BtnA.wasPressed() && mainmode == 2 && positpoint == 0) {
        pagemoveflag = 4;
        return;
    }

    if(M5.BtnA.wasPressed()){
      btna  = true;
    }else{
      btna = false;
    }
    if(M5.BtnC.wasPressed()){
      btnc = true;
    }else{
      btnc = false;
    }

    if(M5.BtnA.isPressed()){
      if(frameleft < 10000){
        frameleft++;
      }
      
    }else{
      frameleft = 1;
    }
    if(M5.BtnC.isPressed()){
      if(frameright < 10000){
        frameright++;
      }
    }else{
      frameright = 1;
    }
    if(righttrue() && mainmode == 1 && DirecX == "/" && imano_page == maxpage - 1 && boolmax()){
      
      pagemoveflag = 5;
      btnc = true;
      btna = false;
        return;
    }
    
    if(mainmode == 8){
       // Serial.println("r" + String(btna) + "l" + String(btnc) + " " + positpoint + "  " + frameright + frameleft);
    }

    
    if ( righttrue() && !(imano_page == maxpage - 1 && mainmode == 1 && boolmax())) {
      Serial.println("F" + String(DirecX) + "G" + String(positpoint));
        btna = false;
        btnc = true;  
      
      if(!(notext && positpoint == positpointmax)){
          positpoint++; // 下へ移動
        }
        
        
    }
    
    // ポインターの移動処理
    else if (lefttrue() && positpoint != 0) {
        positpoint--; // 上へ移動
          Serial.println("F" + String(DirecX) + "G" + String(positpoint));
          btna  =true;
          btnc = false;
    }else if(lefttrue() && positpoint == 0){
      btna = true;
      btnc = false;
      if(!notext){
        if(!modordir && imano_page == 0 && mainmode == 1 ) { //ルートフォルダでこれ使うと強制的に最後のページに逆算できる
        pagemoveflag = 4;
        return;
      }
      else if((mainmode == 2 || mainmode == 4) && positpoint == 0){
        pagemoveflag = 4;
        return;
      }
      else if(modordir && imano_page == 0 && mainmode == 1) { //ルートフォルダでこれ使うと強制的に最後のページに逆算できる
        pagemoveflag = 3;
        return;
      }
      else if(mainmode == 1 && positpoint == 0 && imano_page > 0) {
        pagemoveflag = 2;
        return;
      }
      }else{
        if( positpoint == 0){
          return;
        }
      }
      

    }else {
      btna = false;
      btnc = false;
    }
    
    
    
    // ページ移動フラグのロジック
    // これらの条件はpositpointが更新された後に評価されるべき
    
    
    if (positpoint == positpointmax + 1 && imano_page < maxpage - 1 ) {
      //  Serial.println(String("dd") + maxLinesPerPage3 + "ss s" + imano_page + "pp" + maxpage + "ss" + positpoint + "ee" + positpointmax);
        if((imano_page == maxpage - 1 && mainmode == 1 && positpoint == maxLinesPerPage3 - 1)){
          
          return;
        
        }else{
          pagemoveflag = 1;
        
        return;
        }
        
        
        
    
    } else {
      pagemoveflag = 0;
    }

    // ポインターの境界チェック
    if(maxLinesPerPage2 == 1 && mainmode == 1){
        positpoint = 0;
    }else{
        positpoint = std::max(0, positpoint); // 負の方向には移動できない (最小値は0)

        int effective_filelist_count = positpointmaxg;

        // notextがtrueの場合、画面最下部のテキストがないため、
        // ポインターの最大可動範囲を1つ追加し、`fillRect`や描画位置をその分下にずらします。
        

        
    }
    
    // ポインターの位置が変更された場合、または初回描画時の処理
    // prev_positpoint と現在の positpoint が異なる場合、または prev_positpoint が初期値(-1)の場合
    if (prev_positpoint != positpoint) { 
        // ポインター文字 (">") の幅と高さを取得
        //M5.Lcd.setTextFont(File_goukeifont); // ポインターフォントが設定されていることを確認
        int pointer_char_width = M5.Lcd.textWidth(">");
        int font_height = M5.Lcd.fontHeight();

        // ポインターとその隣接するスペースをクリアするための幅
        // ">" とその右側の文字が重なることを避けるため、少し広めに取る
        int clear_zone_width = pointer_char_width + M5.Lcd.textWidth(" "); 
        // もし文字の高さが一定でない場合、font_height を使うのが安全

        // 以前のポインターを消去
        // prev_positpoint が -1 でない場合 (つまり、updatePointerが既に一度以上ポインターを描画している場合)
        if (prev_positpoint != -1) {
            // prev_positpoint の位置のポインターを黒で塗りつぶす
            M5.Lcd.fillRect(0, prev_positpoint * font_height, clear_zone_width, font_height, BLACK);
            // ここで、もしリストのコンテンツがポインターによって隠されていた場合、その部分のテキストを白で再描画する必要があるかもしれません。
            // しかし、現在の問題は「ポインターが消えない」ことなので、まずポインターのクリアに集中します。
        } else {
            // updatePointerが初回呼び出し時で、かつshokaipointerが既に初期ポインターを描画している場合
            // current_positpoint_on_entry (updatePointer呼び出し時のpositpoint) の位置のポインターをクリアする
            // これは shokaipointer が描画した最初のポインターを消すための措置
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
    
        // ここから画面最下部のスクロールテキスト処理 (変更なし)
    unsigned long currentMillis = millis();

    // テキストスクロールを1秒ごとに更新 (1 FPS)
    if (currentMillis - lastTextScrollTime >= TEXT_SCROLL_INTERVAL_MS) {
        lastTextScrollTime = currentMillis;

        M5.Lcd.setTextFont(1);
        int textWidth = M5.Lcd.textWidth(Tex2);
        int textHeight = M5.Lcd.fontHeight();

        if (textWidth == 0 && Tex2.length() > 0) {
            textWidth = Tex2.length() * 6;
        } else if (Tex2.length() == 0) {
            textWidth = 0;
        }

        int bottomY = M5.Lcd.height() - textHeight;

        M5.Lcd.fillRect(0, bottomY, M5.Lcd.width(), textHeight, BLACK);

        scrollPos -= SCROLL_SPEED_PIXELS;

        if (scrollPos < -textWidth) {
            scrollPos = M5.Lcd.width();
        }

        int visibleStartX = std::max(0, -scrollPos);
        int visibleEndX = std::min(textWidth, M5.Lcd.width() - scrollPos);

        if (visibleStartX < visibleEndX) {
            int charWidthApprox = M5.Lcd.textWidth("A");
            if (charWidthApprox == 0) charWidthApprox = 6;

            int startIndex = visibleStartX / charWidthApprox;
            int endIndex = visibleEndX / charWidthApprox;

            String visibleText = Tex2.substring(startIndex, endIndex);

            M5.Lcd.setCursor(std::max(0, scrollPos), bottomY);
            M5.Lcd.setTextColor(WHITE);
            M5.Lcd.print(visibleText);
        }
    }
    
    
}



void shokaipointer(){
    otroot = false;
    modordir = false;
    listSDRootContents(imano_page,DirecX);
    Serial.println(otroot);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setCursor(0, positpoint * M5.Lcd.fontHeight());
    M5.Lcd.print(">");
    btna = false;
    btnc = false;
    frameright = 1;
    frameleft = 1;
    M5.Lcd.setTextColor(WHITE);
    Tex2 = "Pages:" + String(imano_page ) + "/" + String(maxpage - 1) + "  Press B to Options Now Dir C:/" + DirecX + " :total bytes:" + formatBytes(SD.totalBytes()) + " :used bytes:" + formatBytes(SD.usedBytes());
    resercounter = 0;
    positpointmaxg  = (M5.Lcd.height() / M5.Lcd.fontHeight()) - 1; 
    return;
}

// shokaipointer関数 (オーバーロード版、変更なし)
void shokaipointer(bool yessdd){
    otroot = false;
    modordir = false;
    nosd = false;
    if(yessdd){
        listSDRootContents(imano_page,DirecX);
    }
    Serial.println(otroot);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setCursor(0, positpoint * M5.Lcd.fontHeight());
    M5.Lcd.print(">");
    M5.Lcd.setTextColor(WHITE);
    btna = false;
    btnc = false;
    frameright = 1;
    frameleft = 1;
    Tex2 = "Press B to Options Now Dir C:/" + DirecX + " :total bytes:" + formatBytes(SD.totalBytes()) + " :used bytes:" + formatBytes(SD.usedBytes());
    return;
}
template String joinVectorToString<int>(const std::vector<int>&);
template String joinVectorToString<String>(const std::vector<String>&);

bool righttrue(){
  return frameright == 2 || frameright % scrollpx == 0;
}
bool lefttrue(){
  return frameleft == 2 || frameleft % scrollpx == 0;
}


// ポインターの位置を更新する関数
void updatePointer2(int size) {
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
    }else if (lefttrue() && positpoint == 0 && maxpage == -1){
      btna = true;
      btnc = false;
      pagemoveflag = 5;
        Serial.println("pagemoved");
        return;
    
    } else if(lefttrue() && positpoint == -1){
      btna = true;
      btnc = false;
      Serial.println("HHH" + String(positpoint));
      
      if(imano_page >0){
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
       M5.Lcd.setTextSize(size);
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

bool isValidFilesystemPath(const String& path) {
    // 1. 空のパスは許可しない
    if (path.isEmpty()) {
        Serial.println("Path Validation Error: Path is empty.");
        return false;
    }
    if(path.indexOf("METT_TABLE_ID") != -1 || path.indexOf("TABLE_NAME") != -1 || path.indexOf("HENSU_OPTIONS") != -1){
        Serial.println("Path Validation Error: Path contains reserved keywords.");
        return false;
    }
    // 2. パスは '/' (ルート) から始まる必要がある (Windowsのドライブレター形式は想定外)
    if (path.charAt(0) != '/') {
        Serial.printf("Path Validation Error: Path must start with '/'. Found: %c\n", path.charAt(0));
        return false;
    }
    
    // 3. ファイルシステムで一般的に禁止されている文字をチェック
    // Windowsの禁止文字: < > : " / \ | ? *
    // 組み込みFS (SPIFFS/LittleFS) では一般的に \ と : や ? は避けるべき
    // ここでは、METTファイル形式で使用される : も含め、一般的な予約文字をチェックします。
    // \ (バックスラッシュ), : (コロン), * (アスタリスク), ? (クエスチョン), 
    // " (ダブルクォーテーション), < (小なり), > (大なり), | (パイプ)
    const char* forbiddenChars = "\\:*?\"<>|"; 
    
    for (int i = 0; i < path.length(); ++i) {
        char c = path.charAt(i);
        
        // 制御文字 (ASCII 0-31) も通常は禁止
        if (c < 32 && c != '\n' && c != '\r') { // ただし、改行コードはファイル内容に影響するため除外 (ここでは単なるパスなので影響はないが念のため)
            Serial.printf("Path Validation Error: Path contains control character (ASCII %d).\n", (int)c);
            return false;
        }

        // 禁止文字リストのチェック
        for (int j = 0; forbiddenChars[j] != '\0'; ++j) {
            if (c == forbiddenChars[j]) {
                Serial.printf("Path Validation Error: Path contains forbidden character: %c\n", c);
                return false;
            }
        }
    }
    
    // 4. パスの末尾が '/' で終わっていないかチェック (ファイル名として不適切)
    if (path.length() > 1 && path.endsWith("/")) {
        Serial.println("Path Validation Error: Path should not end with a '/'.");
        return false;
    }
    
    return true; // すべてのチェックを通過
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
std::vector<String> getUniqueTableNames(const std::vector<MettVariableInfo>& variables) {
    std::set<String> nameSet; 

    for (const auto& varInfo : variables) {
        if (!varInfo.tableName.isEmpty()) {
            nameSet.insert(varInfo.tableName);
        }
    }
    
    // setからvectorにコピー
    std::vector<String> uniqueNames;
    for (const auto& name : nameSet) {
        uniqueNames.push_back(name);
    }

    return uniqueNames;
}

/**
 * @brief 一つのテーブルのデータが入ったMettDataMapから、指定した変数名に対応する値を取得します。
 * * @param tableData 特定のテーブルデータ（変数名:値）を含むMettDataMap
 * @param variableName 検索したい変数名
 * @return String 存在する場合はその値、存在しない場合は空の文字列("")を返却します。
 */
String getMettVariableValue(const MettDataMap& tableData, const String& variableName) {
    // マップ内にキーが存在するかチェック
    if (tableData.count(variableName)) {
        // 存在する場合は値を取得
        return tableData.at(variableName);
    }
    // 存在しない場合は空の文字列を返却
    return "";
}
void displayLoadedVariables(const MettDataMap& dataMap) {
    // 画面全体をクリア
    

    Serial.println("--- Mett Data Map ---");
    Serial.println("");
    
    int index = 1;
    
    // マップ内の各要素 (キーと値のペア) をループして画面に出力
    // pair.firstが変数名 (Key), pair.secondが値 (Value)
    for (const auto& pair : dataMap) {
        
        // 変数名と値を使って表示用の文字列を整形
        // 例: 1. Temp: 25.5
        String displayString = String(index++) + ". " + pair.first + ": " + pair.second;
        
        // 整形した文字列を出力
        Serial.println(displayString);
    }
    
    if (dataMap.empty()) {
        Serial.println("Map is empty.");
    }
}
// --- SDカードからオプションリストを読み込む関数 ---
void loadPotlistFromSD() {
    // SD.begin()は既にこの関数の外で成功していると仮定
    File potlistFile = SD.open("/potlist.txt", FILE_READ);

    int i = 0;
    if (potlistFile) { // ファイルが正常に開けた場合のみ読み込み
        while (potlistFile.available() && i < numMenuItems) { // numMenuItems (配列全長) まで読み込みを試みる
            potlist[i] = potlistFile.readStringUntil('\n');
            potlist[i].trim(); // 前後の空白や改行を削除
            i++;
        }
        potlistFile.close();
    } else {
        // デフォルト項目を設定 (numMenuItemsは変更せず、配列に値をセット)
        // ここにデフォルト設定のコードを追加
    }
}

// --- 画面にテキストを中央揃えで描画する汎用関数 ---
// 引数で受け取ったテキストをyPosの位置に描画します
void drawCenteredText(const String& text, int yPos) {
    int screenWidth = M5.Lcd.width();
    M5.Lcd.setTextSize(3);
    int textWidth = M5.Lcd.textWidth(text);
    int xPos = (screenWidth - textWidth) / 2; // 中央揃え

    M5.Lcd.setCursor(xPos, yPos);
    M5.Lcd.print(text); // 引数で受け取ったtextを描画
}

// --- 画面上部にpotlist[currentPos]を描画する関数 ---
// 点滅ロジックを含む
void drawTopText(bool showAngleBrackets) {
    int yPos = 10;
    int screenWidth = M5.Lcd.width();
    int charHeight = M5.Lcd.fontHeight();
    int padding = 2;
    
    // 画面の該当領域をクリア
    M5.Lcd.fillRect(0, yPos, screenWidth, charHeight + padding, BLACK);
    
    M5.Lcd.setTextColor(GREEN, BLACK);
    
    String rawText = potlist[currentPos];
    String textToDisplay = rawText;
    
    // テキストの幅をチェックし、はみ出す場合は切り詰める
    M5.Lcd.setTextSize(3);
    int angleBracketWidth = M5.Lcd.textWidth("<>");
    int maxTextWidthExcludingBrackets = screenWidth - angleBracketWidth - 4;
    
    while (M5.Lcd.textWidth(rawText) > maxTextWidthExcludingBrackets && rawText.length() > 0) {
        rawText = rawText.substring(0, rawText.length() - 1);
    }

    if (showAngleBrackets) {
        textToDisplay = "<" + rawText + ">";
    } else {
        textToDisplay = rawText; 
    }
    drawCenteredText(textToDisplay, yPos);
}

// --- 画面中央に"Test"を描画する関数 ---
void drawCenterText() {
    int yPos = (M5.Lcd.height() / 2) - (M5.Lcd.getTextSizeY() / 2); // getTextSizeY()に置き換え
    int screenWidth = M5.Lcd.width();
    int charHeight = M5.Lcd.fontHeight();
    int padding = 2;

    M5.Lcd.fillRect(0, yPos, screenWidth, charHeight + padding, BLACK);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    drawCenteredText("Test", yPos);
}

// --- 画面下部にoptiontxt[currentPos]を描画する関数 ---
// --- 画面下部にoptiontxt[currentPos]を描画する関数 ---
void drawBottomText() {
    // 描画位置を上に少しずらす
    int yPos = M5.Lcd.height() - M5.Lcd.getTextSizeY() - 30; 
    String currentOptionText = optiontxt[currentPos]; // optionlistをoptiontxtに置き換え
    if(currentPos == 0){
       currentOptionText = optiontxt[0];
    }else if(currentPos == 1){
      currentOptionText = optiontxt[1];
    }else if(currentPos == 2){
      currentOptionText = " ";
    }else if(currentPos == 3){
      currentOptionText = optiontxt[2];
    }else if(currentPos == 4){
      currentOptionText = optiontxt[3]; 
    }else if(currentPos == 5){
      currentOptionText = " ";
    }
    // テキストが画面幅からはみ出さないように切り詰める
    int screenWidth = M5.Lcd.width();
    int maxTextWidth = screenWidth - 10;
    while (M5.Lcd.textWidth(currentOptionText) > maxTextWidth && currentOptionText.length() > 0) {
        currentOptionText = currentOptionText.substring(0, currentOptionText.length() - 1);
    }
    
    // 描画する行の左右全体を塗りつぶす
    M5.Lcd.fillRect(0, yPos, screenWidth, M5.Lcd.height() - yPos, BLACK);
    M5.Lcd.setTextColor(WHITE, BLACK);
    drawCenteredText(currentOptionText, yPos);
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
    SD.end();
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
  rootnofile = false;
  // After populating Filelist, copy the full paths from tempDirectListVector to directlist
  // Consider the size limit of directlist (100) and do not copy if it exceeds
  for (int i = 0; i < std::min((int)tempDirectListVector.size(), 100); ++i) {
    directlist[i] = tempDirectListVector[i];
    ForDlist[i] = entries[i].second ? "1" : "0";
    if(ForDlist[i] == "0"){
      rootnofile = true;
    }
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

bool getVariableNamesInTable(fs::FS &fs, const String& fullFilePath, const String& targetTableName, bool& isZero, std::vector<String>& variableNames) {
    variableNames.clear();
    isZero = false;

    // --- 1. 物理ファイルチェック ---
    if (!fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error (Get Var Names): File not found: %s\n", fullFilePath.c_str());
        return false;
    }

    // --- 2. テーブル存在チェック ---
    bool dummyIsZero;
    std::vector<String> allTables = getAllTableNamesInFile(fs, fullFilePath, dummyIsZero);
    bool tableFound = false;
    for (const auto& name : allTables) {
        if (name == targetTableName) {
            tableFound = true;
            break;
        }
    }
    if (!tableFound) {
        Serial.printf("Error (Get Var Names): Table '%s' not found in file.\n", targetTableName.c_str());
        return false; // テーブル名が存在しない
    }

    // --- 3. テーブルが存在するので、変数を読み込む ---
    // (loadMettFileは指定テーブルが見つからなくてもtrue, empty=trueで返すが、
    //  ここでは既にテーブルの存在がわかっているので、確実にそのテーブルを読みに行く)
    std::vector<MettVariableInfo> loadedVariables;
    bool loadSuccess, fileIsEmpty;
    loadMettFile(fs, fullFilePath, targetTableName, loadSuccess, fileIsEmpty, loadedVariables);

    if (!loadSuccess) {
        // これは基本的に発生しないはずだが、念のため
        Serial.printf("Error (Get Var Names): loadMettFile failed unexpectedly.\n");
        return false; // 物理的な読み込みエラー
    }

    // --- 4. 結果を処理 ---
    if (fileIsEmpty || loadedVariables.empty()) {
        Serial.printf("Info (Get Var Names): Table '%s' was found but contains 0 variables.\n", targetTableName.c_str());
        isZero = true;
    } else {
        isZero = false;
        for (const auto& varInfo : loadedVariables) {
            variableNames.push_back(varInfo.variableName);
        }
        Serial.printf("Info (Get Var Names): Found %d variables in table '%s'.\n", (int)variableNames.size(), targetTableName.c_str());
    }

    return true; // 処理成功
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

String joinStringVector(const std::vector<String>& vec, const char* delim /* = "," */) { // デフォルト引数を削除
    String result = "";
    if (vec.empty()) {
        return ""; // 空の場合は空文字列
    }
    for (size_t i = 0; i < vec.size(); ++i) {
        result += vec[i];
        result += delim; // 各要素の後ろに区切り文字を追加
    }
    return result;
}

/**
 * @brief カンマ区切りのString（末尾にカンマがある可能性あり）をStringのベクターに変換します。
 * (デフォルト引数はヘッダーでのみ指定)
 */
std::vector<String> splitString(const String& str, char delim /* = ',' */) { // デフォルト引数を削除
    std::vector<String> result;
    if (str.isEmpty()) {
        return result;
    }

    int startIndex = 0;
    int endIndex = 0;
    String tempStr = str;

    // 末尾のカンマを削除（もし存在すれば）
    if (tempStr.endsWith(String(delim))) {
        tempStr = tempStr.substring(0, tempStr.length() - 1);
    }

    while ((endIndex = tempStr.indexOf(delim, startIndex)) != -1) {
        result.push_back(tempStr.substring(startIndex, endIndex));
        startIndex = endIndex + 1;
    }
    // 最後の要素を追加 (末尾カンマ削除後なので必ず存在するはず)
    if (startIndex < tempStr.length()) {
       result.push_back(tempStr.substring(startIndex));
    } else if (startIndex == tempStr.length() && str.endsWith(String(delim)) && result.empty() && str.length() > 1) {
         // ケース: "single," -> 末尾カンマ削除 -> "single" -> ループ入らず -> ここで追加
         result.push_back(tempStr);
    } else if (startIndex == tempStr.length() && !str.endsWith(String(delim)) && !str.isEmpty()){
         // ケース: "single" -> 末尾カンマなし -> ループ入らず -> ここで追加
         result.push_back(tempStr);
    }


    return result;
}

// -------------------------

// 予約文字をテーブル名/変数名/オプション要素が含んでいないかチェック
bool containsInvalidChars(const String& str) {
    // 空白、コロン、アンパサンド、シャープ、ハイフン、カンマを予約文字としてチェック
    bool invalid = (str.indexOf(':') != -1 || str.indexOf('&') != -1 || str.indexOf(' ') != -1 || str.indexOf('#') != -1 || str.indexOf('-') != -1 || str.indexOf(',') != -1);
    if (invalid) {
        Serial.printf("Error: '%s' に無効な文字 (:, &, ' ', #, -, ,) が含まれています。\n", str.c_str());
    }
    return invalid;
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

    // ★★★ 追加: 変換後のマップ内容をシリアル出力 ★★★
    Serial.println("\n--- Converted MettDataMap from Vector ---");
    if (dataMap.empty()) {
        Serial.println("Map is empty.");
    } else {
        int index = 1;
        for (const auto& pair : dataMap) {
            String displayString = String(index++) + ". " + pair.first + ": " + pair.second;
            Serial.println(displayString);
        }
    }
    Serial.println("--- End of Converted Map ---");
    // ★★★ 追加ここまで ★★★

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
inline std::vector<String> getAllTableNamesInFile(fs::FS &fs, const String& fullFilePath, bool& isZero) {
    std::vector<String> tableNames;
    std::set<String> uniqueTableNames; // 重複を避けるためにsetを使用

    isZero = true; // デフォルトではテーブルは無いものとする

    if (!fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error (Get All Names): File not found: %s\n", fullFilePath.c_str());
        return tableNames; // 空のベクターを返す
    }

    File file = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!file) {
        Serial.printf("Error (Get All Names): Failed to open file: %s\n", fullFilePath.c_str());
        return tableNames;
    }

    const char* TABLE_NAME_PREFIX = "TABLE_NAME:";
    const int TABLE_NAME_PREFIX_LEN = 11;

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim(); // 前後の空白を削除

        // ★ 修正: startsWith から indexOf == 0 に変更 (より堅牢なチェック)
        if (line.indexOf(TABLE_NAME_PREFIX) == 0) {
            String tableName = line.substring(TABLE_NAME_PREFIX_LEN);
            tableName.trim();
            // ★ 空のテーブル名もカウントする
            uniqueTableNames.insert(tableName);
        }
    }
    file.close();

    if (!uniqueTableNames.empty()) {
        isZero = false;
        // setの内容をvectorにコピーして返す
        for (const auto& name : uniqueTableNames) {
            tableNames.push_back(name);
        }
    }
    
    Serial.printf("Info: Found %d unique table names in file '%s'. isZero: %s\n", 
                  (int)uniqueTableNames.size(), 
                  fullFilePath.c_str(), 
                  isZero ? "True" : "False");

    return tableNames;
}



bool _overwriteMettFile(fs::FS &fs, const String& fullFilePath, const String& newContent) {
    File outFile = fs.open(fullFilePath.c_str(), FILE_WRITE);
    if (!outFile) {
        Serial.printf("Error: Failed to open file for writing: %s\n", fullFilePath.c_str());
        return false;
    }

    if (outFile.print(newContent) == newContent.length()) {
        outFile.close();
        return true;
    } else {
        Serial.printf("Error: Failed to write all content to file: %s\n", fullFilePath.c_str());
        outFile.close();
        return false;
    }
}
bool _findTableBlock(const String& content, const String& tableName, int& blockStartPos, int& blockEndPos) {
    String targetTableLine = "TABLE_NAME:" + tableName;
    blockStartPos = -1;
    blockEndPos = -1;
    int namePos = content.indexOf(targetTableLine);
    
    if (namePos == -1) {
        return false; // テーブル名が見つからない
    }

    // TABLE_ID（ブロック開始行）を特定 (直前の TABLE_ID)
    int markerStartPos = content.lastIndexOf(TABLE_ID, namePos);
    
    if (markerStartPos != -1) {
        // TABLE_ID行の次の行からをブロック開始とする
        blockStartPos = content.indexOf('\n', markerStartPos) + 1;
        if (blockStartPos == 0) blockStartPos = markerStartPos; // TABLE_IDがファイルの先頭にある場合
    } else {
        // TABLE_IDマーカーが見つからないがテーブル名が見つかった場合、テーブル名の行から始める
        blockStartPos = namePos;
    }

    // 次のTABLE_ID（ブロック終了行）を特定
    blockEndPos = content.indexOf(TABLE_ID, namePos + targetTableLine.length());
    
    if (blockEndPos == -1) {
         blockEndPos = content.length(); // ファイルの終端をブロック終了とする
    }

    // 範囲の調整: ブロック終了は、TABLE_IDとその後の改行を含むようにする
    if (blockEndPos != content.length()) {
        blockEndPos = content.indexOf('\n', blockEndPos) + 1;
        if (blockEndPos == 0) blockEndPos = content.length(); // 改行がない場合
    }
    
    // blockStartPos と blockEndPos の整合性チェック
    if (blockStartPos == -1 || blockStartPos >= blockEndPos) {
        return false;
    }
    
    return true;
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


bool deleteTableInFile(fs::FS &fs, const String& fullFilePath, const String& tableName) { // bool* を bool& に変更
   
    if (!fs.exists(fullFilePath.c_str())) {
        Serial.printf("Info (Delete Table): File does not exist, nothing to remove: %s\n", fullFilePath.c_str());
        return true; // ファイルが存在しないのはエラーではない
    }

    String tempFilePath = fullFilePath + ".tmp";
    File originalFile = fs.open(fullFilePath.c_str(), FILE_READ);
    File tempFile = fs.open(tempFilePath.c_str(), FILE_WRITE);

    if (!originalFile || !tempFile) {
        Serial.printf("Error (Delete Table): Failed to open files for processing.\n");
        if(originalFile) originalFile.close();
        if(tempFile) tempFile.close();
      
        return false;
    }

    String finalFileContentForDebug = "";
    String blockBuffer = "";
    bool isTargetBlock = false;
    bool tableFoundAndRemoved = false;
    String currentTableName = "";

    while (originalFile.available()) {
        String line = originalFile.readStringUntil('\n');
        String trimmedLine = line;
        trimmedLine.trim();

        if (trimmedLine.startsWith("### METT_TABLE_ID ###")) {
            // 新しいブロックの開始。前のブロックを処理する
            if (!blockBuffer.isEmpty()) {
                if (isTargetBlock) {
                    tableFoundAndRemoved = true; // このブロックは書き込まない
                    Serial.printf("Debug (Delete Table): Skipping block for table '%s'.\n", tableName.c_str());
                } else {
                    tempFile.print(blockBuffer); // ターゲットでなければ書き込む
                    finalFileContentForDebug += blockBuffer;
                }
            }
            // 新しいブロックのバッファを開始
            blockBuffer = line + "\n";
            isTargetBlock = false; // フラグをリセット
            currentTableName = "";
        } else {
            // ブロックの途中。行をバッファに追加
            blockBuffer += line + "\n";
            if (trimmedLine.startsWith("TABLE_NAME:")) {
                currentTableName = trimmedLine.substring(trimmedLine.indexOf(':') + 1);
                currentTableName.trim();
                if (currentTableName == tableName) {
                    isTargetBlock = true; // このブロックを削除対象としてマーク
                }
            }
        }
    }

    // ファイルの最後のブロックを処理
    if (!blockBuffer.isEmpty()) {
        if (isTargetBlock) {
            tableFoundAndRemoved = true;
             Serial.printf("Debug (Delete Table): Skipping final block for table '%s'.\n", tableName.c_str());
        } else {
            tempFile.print(blockBuffer);
            finalFileContentForDebug += blockBuffer;
        }
    }

    originalFile.close();
    tempFile.close();

    // --- デバッグ出力 ---
    Serial.println("\n--- Deleting table. New content will be: ---");
    Serial.print(finalFileContentForDebug);
    Serial.println("------------------------------------------");


    // --- ファイルの入れ替え ---
    if (fs.remove(fullFilePath.c_str())) {
        if (fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
            if (tableFoundAndRemoved) {
                 Serial.printf("Info (Delete Table): Table '%s' was successfully removed.\n", tableName.c_str());
            } else {
                 Serial.printf("Warning (Delete Table): Table '%s' not found. No changes made.\n", tableName.c_str());
            }
            return true;
        } else {
            Serial.printf("Error (Delete Table): Failed to rename temp file.\n");
            fs.remove(tempFilePath.c_str()); // クリーンアップ
           
            return false;
        }
    } else {
        Serial.printf("Error (Delete Table): Failed to remove original file.\n");
        fs.remove(tempFilePath.c_str()); // クリーンアップ
     
        return false;
    }
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

bool initializeSDCard() {
    // 1. SPIバスのピン設定
    // M5Stack CoreS3のカスタムピンを明示的に指定 (SCK:36, MISO:35, MOSI:37, CS:4)
    SPI.begin(36, 35, 37, 4);
    Serial.printf("SPI Bus configured (SCK:%d, MISO:%d, MOSI:%d, CS:%d).\n", 
                  36, 35, 37, 4);
    
    // 2. SdFsの初期化（物理カードとファイルシステムの両方）
    // チップセレクトピン(4)のみを指定
    if (!sd.begin(4)) {
        // sd.card()を介して物理カードエラーコードを取得
        uint8_t errorCode = sd.card()->errorCode();
        uint32_t errorData = sd.card()->errorData();
        Serial.printf("SD Card initialization (sd.begin(4)) failed. Error Code: 0x%02X, Error Data: 0x%08X\n", 
                      errorCode, errorData);
        return false;
    }
    
    Serial.println("SD Card initialized successfully (sd.begin(4)).");
    return true;
}


// ----------------------------------------------------------------------
// 3. SDカードのフルフォーマットを行う関数 (初期化ロジックを置き換え)
// ----------------------------------------------------------------------

/**
 * @brief SDカード全体をフルフォーマットします。
 * @param formatType フォーマットタイプ (FORMAT_FAT16 または FORMAT_FAT32)
 * @return bool 成功したら true
 */
bool formatSDCardFull() {
    Serial.print("SD Card Formatting Request: ");
    
    if (!initializeSDCard()) { 
        return false; 
    }
    
    // SD処理を開始する前に、専用SPIバスを確保する (トランザクション開始)
    // NOTE: sd.format()は長時間かかるため、SPIバスを占有し、WDT対策も兼ねてトランザクション管理を行う
    // 修正: spiSettings()の使用を避け、標準的な高速SPI設定を直接指定
    // SDカードの高速動作として、16MHz、MSBFIRST、SPI_MODE0を設定します。
    SDSPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));

    // 可能な限り最大容量を取得 (SdFs/SdFatのAPIを使用)
    auto cardPtr = sd.card(); 
    if (cardPtr == nullptr) {
        Serial.println("Hardware Error: Card pointer is NULL.");
        releaseSDBusForOtherUse(); // バスを解放
        return false;
    }
    // FAT16/2GB強制ロジックを削除したため、容量チェックは不要になりました。
    bool result = false;

    // ウォッチドッグタイマー対策として、長時間のブロッキング処理の前にCPUを解放
    yield(); 
    Serial.println("Starting format process... (This may take several seconds/minutes)");

    Serial.println("FAT32 (Full Capacity) - Attempting format...");
            result = sd.format(); // これが長時間ブロックする処理
            Serial.printf("FAT32/Auto Format %s.\n", result ? "SUCCESS" : "FAILED");
            
    
    // 処理完了後、安全にSPIバスを解放
    releaseSDBusForOtherUse(); 
    
    return result;
}

// ----------------------------------------------------------------------
// 4. パーティションを無視したカード全体の容量をStringリターンする関数
// ----------------------------------------------------------------------

/**
 * @brief SDカード全体の物理容量をStringとして返します。（SdFat使用）
 * @return String 容量情報 (例: "64.00 GB (67108864 KB, 65536.00 MB)")
 */
String getSDCardRawCapacity() {
    // --- 変更点: initializeSDCard()でSPIとSdFsを初期化 ---
    if (!initializeSDCard()) { 
        return "SD Card Not Initialized (SdFs)";
    }

    // SdFatのカードオブジェクトからセクター数を取得
    auto cardPtr = sd.card();
    if (cardPtr == nullptr) {
        
        return "NO_CARD / Hardware Error";
    }
    uint64_t totalBytes = (uint64_t)cardPtr->sectorCount() * 512ULL;
    
    // 単位変換
    const double KB_SIZE = 1024.0;
    const double MB_SIZE = KB_SIZE * 1024.0;
    const double GB_SIZE = MB_SIZE * 1024.0; 

    double totalGB = (double)totalBytes / GB_SIZE;
    double totalMB = (double)totalBytes / MB_SIZE;
    double totalKB = (double)totalBytes / KB_SIZE;

    String result = "";
    if (totalGB >= 1.0) {
        result += String(totalGB, 2) + " GB";
    } else if (totalMB >= 1.0) {
        result += String(totalMB, 2) + " MB";
    } else {
        result += String(totalKB, 0) + " KB";
    }
    result += " (" + String(totalKB, 0) + " KB, ";
    result += String(totalMB, 2) + " MB)";

    // リソースを解放 (元のコードのパターンを踏襲)
    
    return result;
}

// ----------------------------------------------------------------------
// 5. SDカードの規格タイプとフォーマット形式を判定する関数
// ----------------------------------------------------------------------

/**
 * @brief SDカードの規格タイプとファイルシステム形式を判定し、一つのStringとして返します。
 * @return String 規格タイプとフォーマット情報
 */
String getSDCardType() {
    
    // --- 変更点: initializeSDCard()でSPIとSdFsを初期化 ---
    if (!initializeSDCard()) { 
        return "NO_CARD / Init Failed";
    }

    // --- 容量に基づく規格タイプ判定 ---
    auto cardPtr = sd.card();
    if (cardPtr == nullptr) {
        
        return "NO_CARD / Hardware Error";
    }
    
    uint64_t totalBytes = (uint64_t)cardPtr->sectorCount() * 512ULL;
    String typeString = ""; // 規格タイプを格納

    // 容量に基づく規格判定ロジック
    const uint64_t GB_UNIT = 1024ULL * 1024 * 1024;
    const uint64_t THRESHOLD_32GB = 32ULL * GB_UNIT;
    const uint64_t THRESHOLD_2TB = 2000ULL * GB_UNIT; 

    if (totalBytes > THRESHOLD_2TB) {
        typeString = "SDUC";
    } else if (totalBytes > THRESHOLD_32GB) {
        typeString = "SDXC";
    } else if (totalBytes > 2ULL * GB_UNIT) {
        typeString = "SDHC";
    } else {
        typeString = "SDSC";
    }

    // 3. フォーマット形式判定（ファイルシステム層）
    String formatString = "";

    // sd.begin()が成功している（=initializeSDCard()が成功）ので、fatTypeを取得
    int fatType = sd.fatType();
    switch (fatType) {
        case 16: formatString = "FAT16"; break;
        case 32: formatString = "FAT32"; break;
        case 64: formatString = "exFAT"; break;
        default: formatString = "Unknown FAT Type"; break;
    }
    
    // リソースを解放 (元のコードのパターンを踏襲)
    
    return typeString + " / " + formatString;
}

// ----------------------------------------------------------------------
// 5-B. SDカードのCID情報を取得する関数
// ----------------------------------------------------------------------

/**
 * @brief SDカードのCIDレジスタから識別情報を取得します。
 * @return String CID情報 (モデル名、製造日時、シリアル番号、その他識別子を含む)
 */
String getSDCardCIDInfo() {
    
    // --- 変更点: initializeSDCard()でSPIとSdFsを初期化 ---
    if (!initializeSDCard()) { 
        return "- Model(PNM): N/A\n- Mfg. Date: N/A\n- Serial Num: N/A\n(Card Init Failed)";
    }

    // --- CID情報取得 ---
    cid_t cidData; // cid_tはSdFatが定義する構造体
    
    // card()で物理カードオブジェクトを取得し、readCIDを呼び出す
    if (!sd.card()->readCID(&cidData)) {
        
        return "- Model(PNM): N/A\n- Mfg. Date: N/A\n- Serial Num: N/A\n(CID Read Failed)";
    }
    
    // CIDフィールドの抽出と整形 (元のロジックを保持)
    uint8_t mid = cidData.mid;
    
    char oid_buf[3]; 
    memcpy(oid_buf, cidData.oid, 2);
    oid_buf[2] = '\0';
    String oidString = String(oid_buf);
    
    char pnm_buf[6]; 
    memcpy(pnm_buf, cidData.pnm, 5); 
    pnm_buf[5] = '\0';
    String modelNameString = String(pnm_buf);
    
    uint8_t prv = cidData.prv; 
    int majorRev = (prv >> 4) & 0xF;
    int minorRev = prv & 0xF;
    String revString = String(majorRev) + "." + String(minorRev);
    
    uint32_t serialNum = cidData.psn(); 

    uint16_t mdtRaw = *reinterpret_cast<uint16_t*>(cidData.mdt); 
    int mfgYear = 2000 + ((mdtRaw >> 4) & 0xF);
    int mfgMonth = mdtRaw & 0xF; 
    String mfgDateString = (mfgMonth >= 1 && mfgMonth <= 12) 
                             ? String(mfgYear) + "/" + (mfgMonth < 10 ? "0" : "") + String(mfgMonth)
                             : String(mfgYear) + "/??";

    // CID情報をまとめて返す
    String result = "- MID (Mfg ID): 0x" + String(mid, HEX);
    result += "\n- OID (OEM ID): " + oidString;
    result += "\n- PNM (Model): " + modelNameString;
    result += "\n- PRV (Revision): " + revString;
    result += "\n- PSN (Serial Num): 0x" + String(serialNum, HEX); 
    result += "\n- MDT (Mfg. Date): " + mfgDateString;
    
    // リソースを解放 (元のコードのパターンを踏襲)
    
    return result;
}

// ----------------------------------------------------------------------
// 6. FAT16/FAT32以外のフォーマット判定関数
// ----------------------------------------------------------------------

/**
 * @brief SDカードがFAT16/FAT32以外の形式または異常な状態にある場合にtrueを返します。
 */
bool isNonFAT16orFAT32Format() {
    Serial.print("Checking Format Type/Health... "); 
    
    // --- 変更点: initializeSDCard()でSPIとSdFsを初期化 ---
    if (!initializeSDCard()) { 
        Serial.println("Card Init Failed (No Card/Physical Error)."); 
        return true; 
    }
    
    bool result = true; // 既定値はNon-FAT/エラー

    // initializeSDCard()が成功しているため、ボリューム情報を使用
    int fatType = sd.fatType();
    
    if (fatType == 16 || fatType == 32) {
        Serial.printf("Format is FAT%d (OK).\n", fatType); 
        result = false;
    } else {
        Serial.printf("Format is Non-FAT16/32: Type %d.\n", fatType); 
        result = true; 
    }
    
    // リソースを解放 (元のコードのパターンを踏襲)
    
    return result;
}

void releaseSDBusForOtherUse() {
    if (!initializeSDCard()) {
        Serial.println("SD Bus release skipped: SD Card was not initialized.");
        return;
    }

    // 1. SPIトランザクションを終了
    // これにより、他のデバイス(M5.Lcdなど)が独自のトランザクションを開始できます。
    SDSPI.endTransaction();
    Serial.println("SD SPI Transaction Ended.");

    // 2. チップセレクトピンをHIGHに戻し、SDカードを非選択状態にする
    digitalWrite(SD_CS_PIN, HIGH);
    Serial.println("SD CS Pin set to HIGH.");
    
    // NOTE: sd.end()はフリーズの原因となるため、この関数では実行しません。
}

void displayMapContents(const String& context, const MettDataMap& dataMap) {
    Serial.printf("\n--- %s (キー数: %d) ---\n", context.c_str(), dataMap.size());
    Serial.println("  Index | Key (変数名) | Value (内部データ)");
    Serial.println("-------------------------------------------------------");
    int count = 0;
    for (const auto& pair : dataMap) {
        // "table_name" キーとその値（テーブル名）もここに出力される
        Serial.printf("  [%d]   | %s | %s\n", count++, pair.first.c_str(), pair.second.c_str());
    }
    Serial.println("-------------------------------------------------------");
}

void logExistingFileContents(fs::FS &fs, const String& fullFilePath) {
    // 既存ファイルの内容を読み取りモードで開き、シリアル出力
    File read_file = fs.open(fullFilePath.c_str(), FILE_READ);
    if (read_file) {
        Serial.printf("\n--- 既存ファイルの内容 (上書き前): %s ---\n", fullFilePath.c_str());
        while (read_file.available()) {
            // 1文字ずつ読み込み、改行も含めてシリアル出力
            Serial.write(read_file.read());
        }
        Serial.println("\n--- 既存ファイル内容 終了 ---\n");
        read_file.close();
    } else {
        Serial.printf("Info: ファイル %s は存在しないか、読み取りに失敗したため、既存コンテンツはありません。\n", fullFilePath.c_str());
    }
}


void displayPageInfo(int currentPage, int totalPages) {
  // 画面の最も下の中央にページ情報を描画
  int y_pos = M5.Lcd.height() - M5.Lcd.fontHeight() - 2; // 下端から少し上
  String pageText = String(currentPage + 1) + " / " + String(totalPages);
  int textWidth = M5.Lcd.textWidth(pageText);
  int x_pos = (M5.Lcd.width() - textWidth) / 2;

  M5.Lcd.setTextColor(YELLOW, BLACK);
  M5.Lcd.fillRect(0, y_pos - 2, M5.Lcd.width(), M5.Lcd.fontHeight() + 4, BLACK); // 背景をクリア
  M5.Lcd.setCursor(x_pos, y_pos);
  M5.Lcd.print(pageText);
  M5.Lcd.setTextColor(WHITE, BLACK); // 色をリセット
}

bool browseFlashDirectoryPaginated(int pagetax, String Directtory) {
  Serial.printf("\n--- フラッシュ閲覧開始 (ページ %d, パス %s) ---\n", pagetax, Directtory.c_str());
  serious_error_flash = false;

  // 1. SPIFFS 初期化チェック
  if (!SPIFFS.begin(false)) {
    Serial.println("【エラー】: SPIFFSの初期化に失敗しました。");
    serious_error_flash = true;
    return false;
  }

  // 画面の初期化とフォント設定
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(File_goukeifont);

  // パスの正規化
  String gg = Directtory;
  if (!gg.startsWith("/")) {
    gg = "/" + gg;
  }
  // ルート("/")以外で末尾の"/"を削除 (open()に影響しないが、慣例として)
  if (gg.length() > 1 && gg.endsWith("/")) {
    gg = gg.substring(0, gg.length() - 1);
  }

  // 2. ディレクトリのオープン
  File root = SPIFFS.open(gg);

  if (!root) {
    M5.Lcd.println("Failed to open directory!"); 
    M5.Lcd.println(gg);
    Serial.printf("【エラー】: パス '%s' のオープンに失敗しました。\n", gg.c_str());
    serious_error_flash = true;
    return false;
  }

  // 3. ディレクトリであることの確認
  if (!root.isDirectory()) {
    M5.Lcd.println("Path is not a directory!");
    M5.Lcd.println(gg);
    Serial.printf("【エラー】: パス '%s' はファイルです。\n", gg.c_str());
    root.close();
    serious_error_flash = true;
    return false;
  }

  // 4. 表示行数の計算
  // 画面高 - ページ情報表示行 / 1行の高さ
  // ここではページ情報を最下行に表示するため、1行分減らす
  maxLinesPerPage = (M5.Lcd.height() / M5.Lcd.fontHeight()) - 1; 
  if (maxLinesPerPage <= 0) maxLinesPerPage = 1; // 少なくとも1行は表示できるように
  
  // 5. 全エントリのリストアップとページ数計算
  std::vector<std::pair<String, bool>> entries; // <EntryName, isDirectory>
  int maxfileperd = 0; // 全ファイル/フォルダ数
  
  // ディレクトリ内の全エントリを走査
  while (File entry = root.openNextFile()) {
    String entryName = String(entry.name()); 
    
    // SPIFFSのルート直下のパス名から'/'を削除 (例: "/data" -> "data")
    if (gg == "/" && entryName.startsWith("/")) {
        entryName = entryName.substring(1);
    }
    
    entries.push_back({entryName, entry.isDirectory()});
    maxfileperd++;
    entry.close(); 
  }
  root.close(); // ディレクトリ走査完了後に閉じる

  // 項目がない場合の処理
  if (entries.empty()) {
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("No files or folders.");
    goukei_page = 0;
    maxpage = 0;
    positpointmax = -1; // 選択肢なし
    return true; // 成功だが項目なし
  }

  // 6. ページ数の計算
  // 例: 11項目 / 10行 per Page = 2ページ
  maxpage = (maxfileperd + maxLinesPerPage - 1) / maxLinesPerPage;
  goukei_page = maxpage; 

  // 7. ページ番号のチェック
  if (pagetax < 0 || pagetax >= maxpage) {
    Serial.printf("【エラー】: 無効なページ番号が指定されました (%d/%d)。\n", pagetax, maxpage);
    return false;
  }

  // 8. 選択可能項目の最大インデックス計算
  if (pagetax == maxpage - 1) { // 最後のページの場合
    int remaining = maxfileperd % maxLinesPerPage;
    positpointmax = (remaining == 0 && maxfileperd > 0) ? (maxLinesPerPage - 1) : (remaining - 1);
  } else { // 途中のページの場合
    positpointmax = maxLinesPerPage - 1;
  }
  
  // 9. 指定されたページのみを描画
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);

  int startIdx = pagetax * maxLinesPerPage;
  int endIdx = std::min((int)entries.size(), startIdx + maxLinesPerPage);

  for (int i = startIdx; i < endIdx; ++i) {
    String fullEntryName;
    
    // 色とプレフィックスを設定
    if (entries[i].second) { // ディレクトリの場合
      M5.Lcd.setTextColor(GREEN, BLACK); 
      fullEntryName = " [D] " + entries[i].first + "/"; // 末尾に/を追加
    } else { // ファイルの場合
      M5.Lcd.setTextColor(SKYBLUE, BLACK);
      fullEntryName = " [F] " + entries[i].first;
    }

    // 文字列を画面幅に合わせて切り詰めて表示
    String displayString = "";
    int displayWidth = M5.Lcd.width();
    
    for (int j = 0; j < fullEntryName.length(); ++j) {
      String testString = displayString + fullEntryName.charAt(j);
      if (M5.Lcd.textWidth(testString) <= displayWidth) {
        displayString = testString;
      } else {
        break; 
      }
    }
    M5.Lcd.println(displayString); 
  }
  
  // ページ情報を表示
  displayPageInfo(pagetax, maxpage);

  // 10. グローバルリストの更新 (全エントリを格納)
  // Filelist, directlist, ForDlistにはページに関わらず**全項目**を格納する
  for (int i = 0; i < 100; ++i) {
    Filelist[i] = "";
    directlist[i] = "";
    ForDlist[i] = "";
  }
  
  int limit = std::min((int)entries.size(), 100);
  for (int i = 0; i < limit; ++i) {
    Filelist[i] = entries[i].first;
    // directlistにはフルパスを格納 (例: /configs/wifi.txt)
    directlist[i] = (gg == "/") ? (gg + entries[i].first) : (gg + "/" + entries[i].first);
    if (entries[i].second) {
        ForDlist[i] = "1"; // ディレクトリ
        // directlistのディレクトリパスの末尾に/を確実に追加
        if (!directlist[i].endsWith("/")) {
            directlist[i] += "/";
        }
    } else {
        ForDlist[i] = "0"; // ファイル
    }
  }

  Serial.printf("--- 閲覧成功。合計 %d 項目, %d ページ --- \n", maxfileperd, maxpage);
  return true;
}


/**
 * @brief ファイル内のテーブル名を変更する関数
 * HENSU_OPTIONS行は変更しません (テーブル名を含まないため)。
 */



/**
 * @brief Scans for metadata files (.mett) in the specified directory on the SD card
 * and returns a list of file names, sizes, and all variable data found within them.
 * @param fs The SD card filesystem object.
 * @param DirecD The directory path to scan.
 * @return std::vector<FileMettData> A list of scan results.
 */
std::vector<FileMettData> scanAndExtractMettData(fs::FS &fs, String DirecD) {
    std::vector<FileMettData> allMettFilesData;
    File root = fs.open(DirecD.c_str());
    if (!root || !root.isDirectory()) {
        Serial.printf("Error: Failed to open directory or not a directory: %s\n", DirecD.c_str());
        return allMettFilesData;
    }
    File file = root.openNextFile();
    while(file){
        if (file.isDirectory()) {
            Serial.printf("Info: Skipping directory: %s\n", file.name());
        } else {
            String fileName = file.name();
            if (fileName.endsWith(".mett")) {
                FileMettData currentFileData;
                currentFileData.fileName = DirecD + "/" + fileName;
                currentFileData.fileSize = file.size();
                Serial.printf("Info: Processing mett file: %s (Size: %u bytes)\n", currentFileData.fileName.c_str(), currentFileData.fileSize);
                File mettFile = fs.open(currentFileData.fileName.c_str(), FILE_READ);
                if (!mettFile) {
                    Serial.printf("Error: Failed to open mett file for reading: %s\n", currentFileData.fileName.c_str());
                    file = root.openNextFile();
                    continue;
                }
                String currentTableName = "";
                while(mettFile.available()){
                    String line = mettFile.readStringUntil('\n');
                    line.trim();
                    if (line.startsWith("--- NEW DATA SET ---")) {
                        currentTableName = "";
                        continue;
                    }
                    if (line.startsWith("TABLE_NAME:")) {
                        int colonIndex = line.indexOf(':');
                        if (colonIndex != -1) {
                            String rawTableName = line.substring(colonIndex + 1);
                            int firstChar = 0;
                            while (firstChar < rawTableName.length() && isspace(rawTableName.charAt(firstChar))) {
                                firstChar++;
                            }
                            int lastChar = rawTableName.length() - 1;
                            while (lastChar >= firstChar && isspace(rawTableName.charAt(lastChar))) {
                                lastChar--;
                            }
                            if (firstChar <= lastChar) {
                                currentTableName = rawTableName.substring(firstChar, lastChar + 1);
                            } else {
                                currentTableName = "";
                            }
                        } else {
                            currentTableName = "";
                            Serial.printf("Debug: 'TABLE_NAME:' line without colon: '%s' in file '%s'\n", line.c_str(), currentFileData.fileName.c_str());
                        }
                        if (containsInvalidTableNameChars(currentTableName)) {
                            Serial.printf("Warning: Table name '%s' in file '%s' contains invalid characters. Treating as empty table name.\n", currentTableName.c_str(), currentFileData.fileName.c_str());
                            currentTableName = "";
                        }
                        continue;
                    }
                    if (line.startsWith("#") || line.isEmpty()) {
                        continue;
                    }
                    int firstColonIndex = line.indexOf(':');
                    int secondColonIndex = line.indexOf(':', firstColonIndex + 1);
                    if (firstColonIndex == -1 || secondColonIndex == -1) {
                        Serial.printf("Warning: Invalid mett line format: %s in file %s\n", line.c_str(), currentFileData.fileName.c_str());
                        continue;
                    }
                    MettVariableInfo varInfo;
                    varInfo.variableName = line.substring(0, firstColonIndex);
                    varInfo.dataType = line.substring(firstColonIndex + 1, secondColonIndex);
                    varInfo.valueString = line.substring(secondColonIndex + 1);
                    varInfo.tableName = currentTableName;
                    varInfo.variableName.trim();
                    varInfo.dataType.trim();
                    varInfo.valueString.trim();
                    if (containsInvalidVariableNameChars(varInfo.variableName)) {
                        Serial.printf("Warning: Variable name '%s' in file '%s' (Table: %s) contains invalid characters. Skipping this variable.\n", varInfo.variableName.c_str(), currentFileData.fileName.c_str(), currentTableName.c_str());
                        continue;
                    }
                    currentFileData.variables.push_back(varInfo);
                }
                mettFile.close();
                allMettFilesData.push_back(currentFileData);
            }
        }
        file = root.openNextFile();
    }
    return allMettFilesData;
}

/**
 * @brief Loads variables from a metadata file into a vector.
 * @param fs The SD card filesystem object.
 * @param fullFilePath The full path of the file to load.
 * @param targetTableName The name of the table to load.
 * @param success Reference to a boolean that will be set to true if loading is successful, false otherwise.
 * @param isEmpty Reference to a boolean that will be set to true if the loaded file is empty, false otherwise.
 * @param variables Reference to the MettVariableInfo vector to store the loaded variables.
 */
void parseMettBlockToMap(const String& variableLines, MettDataMap& dataMap) {
    int currentPos = 0;
    while (currentPos < variableLines.length()) {
        int nextNewline = variableLines.indexOf('\n', currentPos); 
        String line;
        if (nextNewline == -1) {
            line = variableLines.substring(currentPos);
            currentPos = variableLines.length();
        } else {
            line = variableLines.substring(currentPos, nextNewline); 
            currentPos = nextNewline + 1;
        }
        
        line = trimString(line);
        if (line.isEmpty() || line.startsWith("#")) continue;

        int firstColonIndex = line.indexOf(':');
        int secondColonIndex = line.indexOf(':', firstColonIndex + 1);
        
        // 形式: VAR_NAME:DATA_TYPE:VALUE_STRING の3要素が揃っていることを確認
        if (firstColonIndex != -1 && secondColonIndex != -1) {
            String varName = trimString(line.substring(0, firstColonIndex));
            // データ型をスキップし、値を取得
            String valueStr = trimString(line.substring(secondColonIndex + 1));
            
            if (!varName.isEmpty()) {
                dataMap[varName] = valueStr;
            }
        }
    }
}


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
        } else if (inputChar == "SPACE" ) {
            if (SuperT.length() < MAX_STRING_LENGTH) {
                SuperT = SuperT.substring(0, cursorIndex) + " " + SuperT.substring(cursorIndex);
                cursorIndex++;
            }
            if(mainmode == 20){
                entryenter = -1;
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

std::vector<String> loadHensuOptions(fs::FS &fs, const String& fullFilePath, const String& targetTableName, const String& targetVariableName, bool& isNull, bool& isError) {
    isError = false;
    isNull = true; // デフォルトでは見つからない(Null)
    std::vector<String> options; // 空のベクター

    if (!fs.exists(fullFilePath.c_str())) {
        isError = true;
        Serial.printf("Error (loadHensuOptions): File does not exist: %s\n", fullFilePath.c_str());
        return options;
    }

    File file = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!file) {
        isError = true;
        Serial.printf("Error (loadHensuOptions): Failed to open file: %s\n", fullFilePath.c_str());
        return options;
    }

    String currentTableName = "";
    bool inTargetTable = false;
    String targetLinePrefix = "HENSU_OPTIONS:" + targetVariableName + ":";

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();

        if (line.startsWith("TABLE_NAME:")) {
            currentTableName = line.substring(line.indexOf(':') + 1);
            currentTableName.trim();
            inTargetTable = (currentTableName == targetTableName);
        }

        // 対象テーブル内で、かつ変数名が一致するHENSU_OPTIONS行を探す
        if (inTargetTable && line.startsWith(targetLinePrefix)) {
            String optionsString = line.substring(targetLinePrefix.length());
            optionsString.trim(); // ★ 値のトリム
            
            if (optionsString.isEmpty()) {
                isNull = true; // ★ 見つかったが、値が空だった
                options.clear(); // 念のためクリア
            } else {
                isNull = false; // ★ 見つかり、値も空ではなかった
                options = splitString(optionsString, ','); // ヘルパー関数で分割
            }

            file.close();
            Serial.printf("Info (loadHensuOptions): Options loaded for %s:%s. isNull: %s\n", targetTableName.c_str(), targetVariableName.c_str(), isNull ? "true" : "false");
            return options; // 処理完了
        }
    }

    file.close();
    Serial.printf("Warning (loadHensuOptions): Options not found for %s:%s\n", targetTableName.c_str(), targetVariableName.c_str());
    isError = true; // ★ オプション行が見つからなかった場合もエラー(isError)扱いとする
    return options; // 見つからなかった場合 (空のベクター, isNull=true)
}


/**
 * @brief 指定されたテーブル・変数のHENSU_OPTIONSを保存する
 * 形式: HENSU_OPTIONS:variableName:options
 * 各オプション要素にカンマが含まれていないかチェックします。
 */
void saveHensuOptions(fs::FS &fs, const String& fullFilePath, const String& targetTableName, const String& targetVariableName, const std::vector<String>& options, bool& isError) {
    isError = false;

    // --- オプション要素の検証 ---
    for(const String& opt : options) {
        if (opt.indexOf(',') != -1) {
            Serial.printf("Error (saveHensuOptions): Option element '%s' for %s:%s contains invalid character ','.\n", opt.c_str(), targetTableName.c_str(), targetVariableName.c_str());
            isError = true;
            return;
        }
         if (containsInvalidChars(opt)) { // ':' や '-' などもチェック
            Serial.printf("Error (saveHensuOptions): Option element '%s' for %s:%s contains invalid characters.\n", opt.c_str(), targetTableName.c_str(), targetVariableName.c_str());
            isError = true;
            return;
        }
    }

    String tempFilePath = fullFilePath + ".tmp";

    File originalFile = fs.open(fullFilePath.c_str(), FILE_READ);
    File tempFile = fs.open(tempFilePath.c_str(), FILE_WRITE);

    if (!tempFile) {
        isError = true;
        Serial.printf("Error (saveHensuOptions): Failed to open temp file: %s\n", tempFilePath.c_str());
        if (originalFile) originalFile.close();
        return;
    }

    if (!originalFile) {
        isError = true;
        Serial.printf("Error (saveHensuOptions): Failed to open original file: %s\n", fullFilePath.c_str());
        tempFile.close();
        fs.remove(tempFilePath.c_str());
        return;
    }

    String currentTableName = "";
    bool inTargetTable = false;
    bool optionWritten = false;
    String targetLinePrefix = "HENSU_OPTIONS:" + targetVariableName + ":"; // テーブル名を削除
    String newOptionLine = targetLinePrefix + joinStringVector(options, ","); // 末尾カンマ付き

    while (originalFile.available()) {
        String line = originalFile.readStringUntil('\n'); // 元の改行を保持するためにprintlnを使う
        String trimmedLine = line;
        trimmedLine.trim();

        if (trimmedLine.startsWith("TABLE_NAME:")) {
            currentTableName = trimmedLine.substring(trimmedLine.indexOf(':') + 1);
            currentTableName.trim();
            inTargetTable = (currentTableName == targetTableName);
        }

        // 対象テーブル内で、かつ変数名が一致するHENSU_OPTIONS行を探す
        if (inTargetTable && trimmedLine.startsWith(targetLinePrefix)) {
            // 対象の行を見つけたら、新しい内容で書き換える
            tempFile.println(newOptionLine);
            optionWritten = true;
        } else {
            // 対象外の行はそのままコピー
            tempFile.println(line);
        }
    }

    originalFile.close();
    tempFile.close();

    // ファイルを入れ替え
    if (fs.remove(fullFilePath.c_str())) {
        if (!fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
            Serial.printf("Error (saveHensuOptions): Failed to rename temp file.\n");
            isError = true;
        }
    } else {
        Serial.printf("Error (saveHensuOptions): Failed to remove original file.\n");
        isError = true;
    }

    if (!optionWritten) {
        Serial.printf("Warning (saveHensuOptions): Target variable %s:%s not found. Options were not saved.\n", targetTableName.c_str(), targetVariableName.c_str());
        // saveMettFile が空の行を追加しているはず
    } else if (!isError) {
        Serial.println("Info (saveHensuOptions): Options saved successfully.");
    }
}

void ExtractTablePageMett(fs::FS &fs, const String& fullFilePath, const String& targetTableName,
                                int pageNumber, int itemsPerPage,
                                std::vector<String>& variableNames, std::vector<String>& values,
                                std::vector<String>& ids, // ★★★ 変更: int -> String ★★★
                                bool zenbu,  bool& isError, int& allhensucount) { // allhensucountを追加

    // ... (初期化は変更なし) ...
    isError = false; 
    allhensucount = 0; 
    variableNames.clear();
    values.clear();
    ids.clear(); // ★★★ 新しい引数の初期化 ★★★

    // 1. 全データをロード
    std::vector<MettVariableInfo> fullVariables;
    bool loadSuccess, isEmpty;
    loadMettFile(fs, fullFilePath, targetTableName, loadSuccess, isEmpty, fullVariables);

    // 2. loadMettFileのエラーチェック
    if (!loadSuccess) {
        Serial.println("Error: loadMettFileでエラーが発生しました。");
        isError = true;
        return;
    }

    // 3. isZero判定と総変数数の設定
    const size_t totalVariables = fullVariables.size();
    allhensucount = (int)totalVariables; 

    // --- 4. ページ計算とデータ抽出 ---
    int startIndex = 0;
    int endIndex = totalVariables; // デフォルトは全件
    int calculatedMaxPage = 0; // ページング検証/ログ出力用

    if (zenbu) {
        // zenbu = true (全件ロード)の場合
        Serial.println("Info: 全変数をロードします (zenbu = true)。");
    } else {
        // zenbu = false (ページング)の場合

        if (itemsPerPage <= 0) {
            Serial.printf("Error: itemsPerPageはページングには正の数である必要があります。指定値: %d\n", itemsPerPage);
            isError = true; 
            return;
        }
        
        int effectiveItemsPerPage = itemsPerPage;

        // ▼▼▼ 修正: 総ページ数の計算 ▼▼▼
        if (totalVariables == 0) {
            calculatedMaxPage = 0; // 0件の場合は0ページ
        } else {
            // ★ 修正: 0ページから始まるため、(total - 1) / items + 1 ではなく、ceil(total / items) を使う
            calculatedMaxPage = (int)std::ceil((double)totalVariables / effectiveItemsPerPage);
        }

        // ▼▼▼ 修正: ページ番号の検証 (pageNumberは0ベース) ▼▼▼
        // 0ページしかなく、totalVariablesも0の場合、pageNumber 0 は有効
        if (totalVariables == 0 && pageNumber == 0) {
           // 有効なケース
        }
        // calculatedMaxPageが1 (つまり0ページのみ) の場合、pageNumber 0 は有効
        else if (calculatedMaxPage == 1 && pageNumber == 0) {
           // 有効なケース
        }
        // calculatedMaxPageが0より大きく、pageNumberが範囲外の場合
        else if (pageNumber < 0 || (calculatedMaxPage > 0 && pageNumber >= calculatedMaxPage) ) { // ★ 修正: calculatedMaxPageが0でないことを確認
            String validPages = (calculatedMaxPage == 0) ? "0" : "0 ~ " + String(calculatedMaxPage - 1);
            Serial.printf("Error: 不正なページ番号です。指定されたページ: %d, 有効なページ: %s\n", pageNumber, validPages.c_str());
            isError = true; 
            return;
        }


        // ▼▼▼ 修正: startIndexの計算 (0ベース) ▼▼▼
        startIndex = pageNumber * effectiveItemsPerPage;
        endIndex = std::min(startIndex + effectiveItemsPerPage, (int)totalVariables);
    }


    // 5. データ抽出 (★ 変更: IDをStringとして格納)
    for (int i = startIndex; i < endIndex; ++i) {
        const MettVariableInfo& var = fullVariables[i];
        variableNames.push_back(var.variableName);
        values.push_back(var.valueString);
        ids.push_back(var.dataType); // ★★★ ID(String)をベクターに追加 ★★★
    }

    // 6. ログ出力 (変更なし)
    if (zenbu) {
       Serial.printf("Info: 全 %d 件のデータを抽出しました (総数: %d)。\n", (int)variableNames.size(), allhensucount);
    } else {
       // ページング時のログには effectiveItemsPerPage (itemsPerPageと同じ値)も出力
       int effectiveItemsPerPage = itemsPerPage; 
       // ★ 修正: ページ番号の表示を (pageNumber + 1) から pageNumber (0ベース) に変更
       Serial.printf("Info: ページ %d / %dページ中 のデータ %d件を抽出しました。(%d件/ページ, 総数: %d)\n", 
                     pageNumber, (calculatedMaxPage == 0 ? 0 : calculatedMaxPage), (int)variableNames.size(), effectiveItemsPerPage, allhensucount);
    }
}

bool DeleteHensuInMettTable(fs::FS &fs, const String& fullFilePath, const String& tableName, const String& variableName, bool* isError) {
    *isError = false; // デフォルトで成功

    // --- 1. ファイル存在チェック ---
    if (!fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error (Delete Hensu): File does not exist: %s\n", fullFilePath.c_str());
        *isError = true;
        return false;
    }

    // --- 2. 一時ファイル準備 ---
    String tempFilePath = fullFilePath + ".tmp";
    File originalFile = fs.open(fullFilePath.c_str(), FILE_READ);
    File tempFile = fs.open(tempFilePath.c_str(), FILE_WRITE);

    if (!originalFile || !tempFile) {
        Serial.printf("Error (Delete Hensu): Failed to open files for processing.\n");
        if(originalFile) originalFile.close();
        if(tempFile) tempFile.close();
        *isError = true;
        return false;
    }

    String finalFileContentForDebug = ""; // デバッグ用
    bool inTargetTable = false;
    String currentTableName = "";
    bool hensuDeleted = false;
    String targetVarLinePrefix = variableName + ":String:"; // 変数行のプレフィックス (データ型はString固定)
    String targetOptLinePrefix = "HENSU_OPTIONS:" + variableName + ":"; // オプション行のプレフィックス

    // --- 3. 逐次処理とコピー ---
    while (originalFile.available()) {
        String line = originalFile.readStringUntil('\n');
        String originalLine = line;
        String trimmedLine = line;
        trimmedLine.trim();

        bool skipLine = false; // この行をスキップするかどうか

        if (trimmedLine.startsWith("TABLE_NAME:")) {
            currentTableName = trimmedLine.substring(trimmedLine.indexOf(':') + 1);
            currentTableName.trim();
            inTargetTable = (currentTableName == tableName);
        }

        // 対象テーブル内でのみ削除チェックを行う
        if (inTargetTable) {
            // 変数行が一致するかチェック
            if (trimmedLine.startsWith(targetVarLinePrefix)) {
                skipLine = true;
                hensuDeleted = true; // 削除対象が見つかった
                Serial.printf("Debug (Delete Hensu): Skipping variable line for '%s' in table '%s'.\n", variableName.c_str(), tableName.c_str());
            }
            // オプション行が一致するかチェック
            else if (trimmedLine.startsWith(targetOptLinePrefix)) {
                skipLine = true;
                 Serial.printf("Debug (Delete Hensu): Skipping options line for '%s' in table '%s'.\n", variableName.c_str(), tableName.c_str());
            }
        }

        // スキップしない行のみ一時ファイルに書き込む
        if (!skipLine) {
            tempFile.print(originalLine); // 元の改行を保持して書き込み
            finalFileContentForDebug += originalLine;
        }
    }

    originalFile.close();
    tempFile.close();

    // --- 4. デバッグ出力 ---
    Serial.println("\n--- Deleting hensu. New content will be: ---");
    Serial.print(finalFileContentForDebug);
    Serial.println("------------------------------------------");

    // --- 5. ファイルを入れ替え ---
    if (fs.remove(fullFilePath.c_str())) {
        if (fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
            if (hensuDeleted) {
                Serial.printf("Info (Delete Hensu): Variable '%s' successfully removed from table '%s'.\n", variableName.c_str(), tableName.c_str());
                return true; // 成功
            } else {
                Serial.printf("Warning (Delete Hensu): Variable '%s' not found in table '%s'. No changes made.\n", variableName.c_str(), tableName.c_str());
                return false; // 変数が見つからなかった
            }
        } else {
            Serial.printf("Error (Delete Hensu): Failed to rename temp file.\n");
            fs.remove(tempFilePath.c_str()); // クリーンアップ
            *isError = true;
            return false;
        }
    } else {
        Serial.printf("Error (Delete Hensu): Failed to remove original file.\n");
        fs.remove(tempFilePath.c_str()); // クリーンアップ
        *isError = true;
        return false;
    }
}

/**
 * @brief ファイル内の指定されたテーブルの変数名を変更する関数
 * @param fs SDカードファイルシステムオブジェクト
 * @param fullFilePath 対象ファイルのフルパス
 * @param tableName 対象テーブル名
 * @param oldVariableName 変更前の変数名
 * @param newVariableName 変更後の変数名
 * @param isError エラーが発生した場合にtrue
 * @return 変数が見つかり正常に変更された場合はtrue、見つからなかったりエラーの場合はfalse
 */
bool renameHensuInTable(fs::FS &fs, const String& fullFilePath, const String& tableName, const String& oldVariableName, const String& newVariableName, bool& isError) { // bool* を bool& に変更
    isError = false; // デフォルトで成功

    // --- 1. Validation ---
    if (!fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error (Rename Hensu): File does not exist: %s\n", fullFilePath.c_str());
        isError = true;
        return false;
    }
    if (oldVariableName == newVariableName || newVariableName.isEmpty()) {
        Serial.printf("Error (Rename Hensu): New variable name is invalid or same as old name.\n");
        isError = true;
        return false;
    }
     if (containsInvalidChars(newVariableName)) { // 新しい名前もチェック
        Serial.printf("Error (Rename Hensu): Invalid new variable name: '%s'\n", newVariableName.c_str());
        isError = true;
        return false;
    }

    // --- 2. Pre-scan for existence and conflict ---
    bool oldVarFound = false;
    bool newVarConflict = false;
    std::vector<MettVariableInfo> currentVars;
    bool loadSuccess, isEmpty, loadErrorFlag; // loadError -> loadErrorFlag
    loadMettFile(fs, fullFilePath, tableName, loadSuccess, isEmpty, currentVars); // 引数順序を変更
    if(!loadSuccess){ // loadError -> !loadSuccess
        Serial.printf("Error (Rename Hensu): Failed to load table '%s' for pre-scan.\n", tableName.c_str());
        isError = true; // エラーフラグを設定 (loadMettFile自体はエラーを返さないので)
        return false;
    }
    for(const auto& var : currentVars){
        if(var.variableName == oldVariableName) oldVarFound = true;
        if(var.variableName == newVariableName) newVarConflict = true;
    }

    if (!oldVarFound) {
        Serial.printf("Warning (Rename Hensu): Variable '%s' not found in table '%s'. No changes made.\n", oldVariableName.c_str(), tableName.c_str());
        return false; // Not an error, but no change
    }
    if (newVarConflict) {
        Serial.printf("Error (Rename Hensu): Variable name '%s' already exists in table '%s'. Aborting.\n", newVariableName.c_str(), tableName.c_str());
        isError = true;
        return false;
    }

    // --- 3. Rebuild file with replacement ---
    String tempFilePath = fullFilePath + ".tmp";
    File originalFile = fs.open(fullFilePath.c_str(), FILE_READ);
    File tempFile = fs.open(tempFilePath.c_str(), FILE_WRITE);

    if (!originalFile || !tempFile) {
        Serial.printf("Error (Rename Hensu): Failed to open files for processing.\n");
        if(originalFile) originalFile.close();
        if(tempFile) tempFile.close();
        isError = true;
        return false;
    }

    String finalFileContentForDebug = ""; // デバッグ用
    bool inTargetTable = false;
    String currentTableName = "";
    String targetVarLinePrefix = oldVariableName + ":String:";
    String targetOptLinePrefix = "HENSU_OPTIONS:" + oldVariableName + ":";
    bool hensuRenamed = false;

    while(originalFile.available()){
        String line = originalFile.readStringUntil('\n');
        String originalLine = line;
        String trimmedLine = line;
        trimmedLine.trim();

        String outputLine = originalLine; // デフォルトはそのまま

        if (trimmedLine.startsWith("TABLE_NAME:")) {
            currentTableName = trimmedLine.substring(trimmedLine.indexOf(':') + 1);
            currentTableName.trim();
            inTargetTable = (currentTableName == tableName);
        }

        if(inTargetTable) {
            // 変数行の置換
            if (trimmedLine.startsWith(targetVarLinePrefix)) {
                String valuePart = trimmedLine.substring(targetVarLinePrefix.length());
                outputLine = newVariableName + ":String:" + valuePart + "\n";
                hensuRenamed = true;
                Serial.printf("Debug (Rename Hensu): Replacing variable line for '%s'.\n", oldVariableName.c_str());
            }
            // オプション行の置換
            else if (trimmedLine.startsWith(targetOptLinePrefix)) {
                String optionsPart = trimmedLine.substring(targetOptLinePrefix.length());
                outputLine = "HENSU_OPTIONS:" + newVariableName + ":" + optionsPart + "\n";
                 Serial.printf("Debug (Rename Hensu): Replacing options line for '%s'.\n", oldVariableName.c_str());
            }
        }

        tempFile.print(outputLine);
        finalFileContentForDebug += outputLine;
    }

    originalFile.close();
    tempFile.close();

    // --- 4. デバッグ出力 ---
    Serial.println("\n--- Renaming hensu. New content will be: ---");
    Serial.print(finalFileContentForDebug);
    Serial.println("------------------------------------------");


    // --- 5. ファイルを入れ替え ---
    if (fs.remove(fullFilePath.c_str())) {
        if (fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
            if (hensuRenamed) {
                Serial.printf("Info (Rename Hensu): Variable '%s' successfully renamed to '%s' in table '%s'.\n", oldVariableName.c_str(), newVariableName.c_str(), tableName.c_str());
                return true; // 成功
            } else {
                // Pre-scanでチェック済みだが念のため
                Serial.printf("Warning (Rename Hensu): Variable '%s' somehow not found during rename process in table '%s'.\n", oldVariableName.c_str(), tableName.c_str());
                return false;
            }
        } else {
            Serial.printf("Error (Rename Hensu): Failed to rename temp file.\n");
            fs.remove(tempFilePath.c_str()); // クリーンアップ
            isError = true;
            return false;
        }
    } else {
        Serial.printf("Error (Rename Hensu): Failed to remove original file.\n");
        fs.remove(tempFilePath.c_str()); // クリーンアップ
        isError = true;
        return false;
    }
}

 bool duplicateMettFile(fs::FS &fs, const String& fullFilePath, const String& oldTableName, const String& newTableName, bool& isError) {
    isError = false;

    // --- 1. 既存の全テーブル名をチェック ---
    bool isZero;
    std::vector<String> allNames = getAllTableNamesInFile(fs, fullFilePath, isZero);
    
    if (!fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error (Duplicate): File does not exist: %s\n", fullFilePath.c_str());
        isError = true;
        return false;
    }

    bool oldTableExists = false;
    for (const auto& name : allNames) {
        if (name == newTableName) {
            Serial.printf("Error (Duplicate): New table name '%s' already exists.\n", newTableName.c_str());
            isError = true;
            return false;
        }
        if (name == oldTableName) {
            oldTableExists = true;
        }
    }

    if (!oldTableExists) {
        Serial.printf("Error (Duplicate): Old table name '%s' not found.\n", oldTableName.c_str());
        isError = true;
        return false;
    }

    // --- 2. ★ 修正: 複製元のブロック全体 (HENSU_OPTIONS: も含む) を読み込む ---
    String blockToCopy = "";
    File originalFile = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!originalFile) {
        Serial.printf("Error (Duplicate): Failed to open file for reading: %s\n", fullFilePath.c_str());
        isError = true;
        return false;
    }
    
    // --- 2 (改): 複製元のブロック全体を読み込む (ID行から) ---
    blockToCopy = ""; // リセット
    originalFile = fs.open(fullFilePath.c_str(), FILE_READ); // 再オープン
    bool inTargetBlock = false;
    String idLineOfBlock = "";

    while (originalFile.available()) {
        String line = originalFile.readStringUntil('\n');
        String trimmedLine = line;
        trimmedLine.trim();

        if (trimmedLine.startsWith("### METT_TABLE_ID ###")) {
            if (inTargetBlock) {
                inTargetBlock = false;
                break; // ターゲットブロック終了
            }
            idLineOfBlock = line + "\n"; // ID行を保持
        } 
        else if (trimmedLine.startsWith("TABLE_NAME:")) {
            String currentTableName = trimmedLine.substring(String("TABLE_NAME:").length());
            currentTableName.trim();
            if (currentTableName == oldTableName) {
                inTargetBlock = true;
                blockToCopy = idLineOfBlock + line + "\n"; // ID行 + TABLE_NAME行
            }
        }
        else if (inTargetBlock) {
            blockToCopy += line + "\n"; // 変数行、HENSU_OPTIONS行、空行など
        }
    }
    originalFile.close();
    
    if (blockToCopy.isEmpty()) {
         Serial.printf("Error (Duplicate): Failed to read block for table '%s'.\n", oldTableName.c_str());
        isError = true;
        return false;
    }

    // --- 3 (改): テーブル名を置換 ---
    blockToCopy.replace("TABLE_NAME:" + oldTableName, "TABLE_NAME:" + newTableName);


    // --- 4. ファイルの末尾に追記 ---
    File file = fs.open(fullFilePath.c_str(), FILE_APPEND);
    if (!file) {
        Serial.printf("Error (Duplicate): Failed to open file for appending: %s\n", fullFilePath.c_str());
        isError = true;
        return false;
    }
    
    // ファイルの先頭がIDでない場合、ID行を先に追加する (追記時の安全対策)
    // (追記なので、先頭に改行を1つ追加して、前のブロックと分離する)
    if (file.print("\n" + blockToCopy)) {
        Serial.printf("Info (Duplicate): Table '%s' successfully duplicated to '%s'.\n", oldTableName.c_str(), newTableName.c_str());
        file.close();
        return true;
    } else {
        Serial.printf("Error (Duplicate): Failed to write to file.\n");
        file.close();
        isError = true;
        return false;
    }
}

bool isValidAndUniqueVariableName(fs::FS &fs, const String& fullFilePath, const String& targetTableName, const String& newVariableName) {
    
    // 1. 禁止文字のチェック
    const char* forbiddenChars = ":.\\/ \t\r\n*?\"<>|-;"; // \ と半角・全角空白、その他ファイル名禁止文字
    
    // 基本的な禁止文字
    for (int i = 0; i < strlen(forbiddenChars); ++i) {
        if (newVariableName.indexOf(forbiddenChars[i]) != -1) {
            Serial.printf("Error (ValidateName): 変数名 '%s' に禁止文字 '%c' が含まれています。\n", newVariableName.c_str(), forbiddenChars[i]);
            return false;
        }
    }
    // 全角スペース (UTF-8で 0xE3 0x80 0x80)
    if (newVariableName.indexOf("\xE3\x80\x80") != -1) {
        Serial.printf("Error (ValidateName): 変数名 '%s' に全角スペースが含まれています。\n", newVariableName.c_str());
        return false;
    }

    // 2. 文字長のチェック
    if (newVariableName.length() > 1000) {
        Serial.printf("Error (ValidateName): 変数名が長すぎます (Max 1000文字)。\n");
        return false;
    }
    
    // 3. 空でないかのチェック
    if (newVariableName.isEmpty()) {
        Serial.printf("Error (ValidateName): 変数名を空にすることはできません。\n");
        return false;
    }

    // 4. 重複チェック
    bool isZero;
    std::vector<String> existingNames;
    if (!getVariableNamesInTable(fs, fullFilePath, targetTableName, isZero, existingNames)) {
        // テーブル読み込み自体に失敗した場合 (テーブルが存在しないなど)
        // (getVariableNamesInTableがfalseを返すのは、loadMettFile失敗時のみ)
        // (isValidAndUniqueVariableNameは「もしテーブルが存在したら」という前提で動くべき)
        // (ここでは、テーブルが存在しない場合は重複しようがないのでtrueを返す、という仕様も考えられるが、)
        // (安全のため、読み込めなかったらfalseとする)
        Serial.printf("Error (ValidateName): 重複チェックのためテーブル '%s' を読み込めませんでした。\n", targetTableName.c_str());
        return false;
    }

    if (!isZero) { // 変数が1つ以上ある場合のみチェック
        for (const auto& name : existingNames) {
            if (name == newVariableName) {
                Serial.printf("Error (ValidateName): 変数名 '%s' はテーブル '%s' 内に既に存在します。\n", newVariableName.c_str(), targetTableName.c_str());
                return false;
            }
        }
    }

    // すべてのチェックを通過
    return true;
}


void createMettHensu(fs::FS &fs, const String& fullFilePath, const String& targetTableName, 
                           const String& targetVariableName, const String& value, 
                           bool isWrite, int id, bool& isError) {
    isError = false;
    Serial.printf("\n--- Input for createMettHensu (Var: %s, Value: %s, isWrite: %s, ID: %d) ---\n", 
                  targetVariableName.c_str(), value.c_str(), isWrite ? "true" : "false", id);

    // --- 1. ストリーミング書き込みのための準備 ---
    String tempFilePath = fullFilePath + ".tmp";
    File tempFile = fs.open(tempFilePath.c_str(), FILE_WRITE);
    if (!tempFile) {
        Serial.printf("Error (createMettHensu): Failed to open temp file: %s\n", tempFilePath.c_str());
        isError = true;
        return;
    }

    bool tableFound = false;
    bool inTargetBlock = false;
    bool variableProcessed = false; // 変数が処理（上書きまたは新規追加）されたか
    // ★ 修正: オプション行の形式を変更
    String optionLinePrefix = "HENSU_OPTIONS:" + targetVariableName + ":";

    const char* TABLE_ID_PREFIX = "### METT_TABLE_ID ###";
    const char* TABLE_NAME_PREFIX = "TABLE_NAME:";
    const int TABLE_NAME_PREFIX_LEN = 11;

    // --- 2. 元ファイルが存在する場合のみ、ストリーミング読み書き実行 ---
    if (fs.exists(fullFilePath.c_str())) {
        File originalFile = fs.open(fullFilePath.c_str(), FILE_READ);
        if (!originalFile) {
            Serial.printf("Error (createMettHensu): Failed to open original file: %s\n", fullFilePath.c_str());
            tempFile.close();
            fs.remove(tempFilePath.c_str());
            isError = true;
            return;
        }

        while (originalFile.available()) {
            String line = originalFile.readStringUntil('\n');
            String trimmedLine = line;
            trimmedLine.trim();

            if (trimmedLine.startsWith(TABLE_ID_PREFIX)) {
                // ブロック境界
                if (inTargetBlock && !variableProcessed) {
                    // ★ 新規作成 (ブロックの末尾に追加)
                    Serial.printf("Debug (createMettHensu): Variable not found in block. Appending new var '%s'.\n", targetVariableName.c_str());
                    String newId = isWrite ? String(id) : "";
                    tempFile.println(targetVariableName + ":" + newId + ":" + value);
                    // ★ 修正: 新しいオプション行の形式
                    tempFile.println("HENSU_OPTIONS:" + targetVariableName + ":"); 
                    variableProcessed = true;
                }
                inTargetBlock = false;
                tempFile.println(line); 
            } 
            else if (trimmedLine.startsWith(TABLE_NAME_PREFIX)) {
                // テーブル名行
                String currentTableName = trimmedLine.substring(TABLE_NAME_PREFIX_LEN);
                currentTableName.trim();
                
                tempFile.println(line); // テーブル名行は常に書き込む

                if (currentTableName == targetTableName) { 
                    inTargetBlock = true;
                    tableFound = true;
                } else {
                    inTargetBlock = false;
                }
            } 
            else if (inTargetBlock) {
                // ターゲットブロック内の変数行
                int firstColon = trimmedLine.indexOf(':');
                if (firstColon > 0) {
                    String varName = trimmedLine.substring(0, firstColon);
                    
                    if (varName == targetVariableName) {
                        // ★ 上書き対象の変数を発見
                        variableProcessed = true;
                        String newLine;
                        if (isWrite) {
                            // IDも上書き
                            newLine = targetVariableName + ":" + String(id) + ":" + value;
                        } else {
                            // IDは既存のものを保持
                            int secondColon = trimmedLine.indexOf(':', firstColon + 1);
                            String existingId = (secondColon > firstColon) ? trimmedLine.substring(firstColon + 1, secondColon) : "";
                            newLine = targetVariableName + ":" + existingId + ":" + value;
                        }
                        tempFile.println(newLine);
                        Serial.printf("Debug (createMettHensu): Rewriting var '%s' (isWrite: %s).\n", targetVariableName.c_str(), isWrite ? "true" : "false");
                    } 
                    // ★ 修正: オプション行の判定
                    else if (trimmedLine.startsWith(optionLinePrefix)) {
                        // オプション行 (上書き時)
                        tempFile.println(line); // そのまま保持
                    }
                    else {
                        // 他の変数行
                        tempFile.println(line);
                    }
                } else {
                    // ブロック内の空行やコメント
                    tempFile.println(line);
                }
            } 
            else {
                // ターゲットブロック外の行
                tempFile.println(line);
            }
        } // while loop
        originalFile.close();
    } // if fs.exists

    // --- 3. ファイル末尾の処理 ---
    if (inTargetBlock && !variableProcessed) {
        // ★ 新規作成 (ファイルの末尾が対象ブロックだった場合)
        Serial.printf("Debug (createMettHensu): Variable not found at EOF. Appending new var '%s'.\n", targetVariableName.c_str());
        String newId = isWrite ? String(id) : "";
        tempFile.println(targetVariableName + ":" + newId + ":" + value);
        // ★ 修正: 新しいオプション行の形式
        tempFile.println("HENSU_OPTIONS:" + targetVariableName + ":"); 
        variableProcessed = true;
    }

    // --- 4. テーブル自体がファイル内に見つからなかった場合 (新規テーブル追加) ---
    if (!tableFound) {
        Serial.printf("Debug (createMettHensu): Appending new table '%s' and new var '%s'.\n", targetTableName.c_str(), targetVariableName.c_str());
        tempFile.println(TABLE_ID_PREFIX);
        tempFile.println(String(TABLE_NAME_PREFIX) + targetTableName);
        
        String newId = isWrite ? String(id) : "";
        tempFile.println(targetVariableName + ":" + newId + ":" + value);
        // ★ 修正: 新しいオプション行の形式
        tempFile.println("HENSU_OPTIONS:" + targetVariableName + ":");
        
        tempFile.println();
        variableProcessed = true;
    }

    tempFile.close();

    // --- 5. ファイルを入れ替え ---
    if (fs.remove(fullFilePath.c_str())) {
        if (!fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
            Serial.printf("Error (createMettHensu): Failed to rename temp file.\n");
            isError = true;
        }
    } else if (fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error (createMettHensu): Failed to remove original file.\n");
        isError = true;
    } else { // 元ファイルがなかった場合
        if (!fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
             Serial.printf("Error (createMettHensu): Failed to rename temp file for new file.\n");
             isError = true;
        }
    }

    if (!isError) {
        Serial.printf("Info (createMettHensu): File saved successfully. Variable '%s' was processed in table '%s'.\n", targetVariableName.c_str(), targetTableName.c_str());
    }
}

 bool loadMettHensu(fs::FS &fs, const String& fullFilePath, const String& targetTableName, const String& targetVariableName, 
                         String& value, bool& idIsNull, int& idReturn) {
    
    idIsNull = true; // デフォルト
    idReturn = 0;    // デフォルト
    value = "";      // ★ value を初期化

    std::vector<MettVariableInfo> variables;
    bool loadSuccess, isEmpty;
    loadMettFile(fs, fullFilePath, targetTableName, loadSuccess, isEmpty, variables);

    if (!loadSuccess) {
        Serial.printf("Error (LoadHensu): loadMettFile failed for table '%s'.\n", targetTableName.c_str());
        return false; // ファイル読み込み自体に失敗
    }

    for (const auto& var : variables) {
        if (var.variableName == targetVariableName) {
            value = var.valueString; // ★ value を設定
            
            if (var.dataType.isEmpty()) {
                idIsNull = true;
                idReturn = 0;
            } else {
                idIsNull = false;
                idReturn = var.id1; // (var.dataType.toInt())
            }

            // ★ 修正: ログに Value を再度追加
            Serial.printf("Info (LoadHensu): Found variable '%s'. Value: %s, ID: %s (isNull: %s)\n", 
                          targetVariableName.c_str(), value.c_str(), var.dataType.c_str(), idIsNull ? "true" : "false");
            return true; // 変数発見
        }
    }

    Serial.printf("Warning (LoadHensu): Variable '%s' not found in table '%s'.\n", targetVariableName.c_str(), targetTableName.c_str());
    return false; // 変数が見つからなかった
}

 bool loadHensuId(fs::FS &fs, const String& fullFilePath, const String& targetTableName, const String& targetVariableName, 
                       bool& idIsNull, int& idReturn) {
    
    idIsNull = true; // デフォルト
    idReturn = 0;    // デフォルト

    std::vector<MettVariableInfo> variables;
    bool loadSuccess, isEmpty;
    loadMettFile(fs, fullFilePath, targetTableName, loadSuccess, isEmpty, variables);

    if (!loadSuccess) {
        Serial.printf("Error (LoadHensuId): loadMettFile failed for table '%s'.\n", targetTableName.c_str());
        return false; // ファイル読み込み自体に失敗
    }

    for (const auto& var : variables) {
        if (var.variableName == targetVariableName) {
            if (var.dataType.isEmpty()) {
                idIsNull = true;
                idReturn = 0;
            } else {
                idIsNull = false;
                idReturn = var.id1;
            }
            Serial.printf("Info (LoadHensuId): Found variable '%s'. ID: %s (isNull: %s)\n", 
                          targetVariableName.c_str(), var.dataType.c_str(), idIsNull ? "true" : "false");
            return true; // 変数発見
        }
    }

    Serial.printf("Warning (LoadHensuId): Variable '%s' not found in table '%s'.\n", targetVariableName.c_str(), targetTableName.c_str());
    return false; // 変数が見つからなかった
}

bool saveHensuId(fs::FS &fs, const String& fullFilePath, const String& targetTableName, 
                       const String& targetVariableName, bool isNull, int idValue) {
    bool isError = false;
    Serial.printf("\n--- Input for saveHensuId (Var: %s, isNull: %s, ID: %d) ---\n", 
                  targetVariableName.c_str(), isNull ? "true" : "false", idValue);

    // --- 1. ストリーミング書き込みのための準備 ---
    String tempFilePath = fullFilePath + ".tmp";
    File tempFile = fs.open(tempFilePath.c_str(), FILE_WRITE);
    if (!tempFile) {
        Serial.printf("Error (saveHensuId): Failed to open temp file: %s\n", tempFilePath.c_str());
        return false;
    }

    bool tableFound = false;
    bool inTargetBlock = false;
    bool variableProcessed = false; // 変数が処理（上書き）されたか

    const char* TABLE_ID_PREFIX = "### METT_TABLE_ID ###";
    const char* TABLE_NAME_PREFIX = "TABLE_NAME:";
    const int TABLE_NAME_PREFIX_LEN = 11;

    // --- 2. 元ファイルが存在する場合のみ、ストリーミング読み書き実行 ---
    if (fs.exists(fullFilePath.c_str())) {
        File originalFile = fs.open(fullFilePath.c_str(), FILE_READ);
        if (!originalFile) {
            Serial.printf("Error (saveHensuId): Failed to open original file: %s\n", fullFilePath.c_str());
            tempFile.close();
            fs.remove(tempFilePath.c_str());
            return false;
        }

        while (originalFile.available()) {
            String line = originalFile.readStringUntil('\n');
            String trimmedLine = line;
            trimmedLine.trim();

            if (trimmedLine.startsWith(TABLE_ID_PREFIX)) {
                inTargetBlock = false;
                tempFile.println(line); 
            } 
            else if (trimmedLine.startsWith(TABLE_NAME_PREFIX)) {
                String currentTableName = trimmedLine.substring(TABLE_NAME_PREFIX_LEN);
                currentTableName.trim();
                
                tempFile.println(line); // テーブル名行は常に書き込む

                if (currentTableName == targetTableName) { 
                    inTargetBlock = true;
                    tableFound = true;
                } else {
                    inTargetBlock = false;
                }
            } 
            else if (inTargetBlock) {
                // ターゲットブロック内の変数行
                int firstColon = trimmedLine.indexOf(':');
                if (firstColon > 0) {
                    String varName = trimmedLine.substring(0, firstColon);
                    
                    if (varName == targetVariableName) {
                        // ★ 上書き対象の変数を発見
                        variableProcessed = true;
                        
                        // IDとValueを分離
                        int secondColon = trimmedLine.indexOf(':', firstColon + 1);
                        String existingValue = (secondColon > firstColon) ? trimmedLine.substring(secondColon + 1) : "";
                        String newId = isNull ? "" : String(idValue);
                        
                        String newLine = targetVariableName + ":" + newId + ":" + existingValue;
                        
                        tempFile.println(newLine);
                        Serial.printf("Debug (saveHensuId): Rewriting ID for var '%s' to '%s'.\n", targetVariableName.c_str(), newId.c_str());
                    } 
                    else {
                        // 他の変数行 (オプション行も含む)
                        tempFile.println(line);
                    }
                } else {
                    // ブロック内の空行やコメント
                    tempFile.println(line);
                }
            } 
            else {
                // ターゲットブロック外の行
                tempFile.println(line);
            }
        } // while loop
        originalFile.close();
    } // if fs.exists

    tempFile.close();

    // --- 3. 変数が見つからなかった場合の処理 ---
    if (!variableProcessed) {
        Serial.printf("Error (saveHensuId): Variable '%s' not found in table '%s'. No changes made.\n", targetVariableName.c_str(), targetTableName.c_str());
        fs.remove(tempFilePath.c_str()); // 不要な一時ファイルを削除
        return false;
    }

    // --- 4. ファイルを入れ替え ---
    if (fs.remove(fullFilePath.c_str())) {
        if (!fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
            Serial.printf("Error (saveHensuId): Failed to rename temp file.\n");
            isError = true;
        }
    } else if (fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error (saveHensuId): Failed to remove original file.\n");
        isError = true;
    } 
    // (元ファイルがなかった場合、variableProcessedはfalseになるはずなので、ここは考慮不要)
    
    if (isError) {
        return false;
    }

    Serial.printf("Info (saveHensuId): File saved successfully. ID for '%s' was updated in table '%s'.\n", targetVariableName.c_str(), targetTableName.c_str());
    return true;
}

void showmozinn(const String& text) {
    // 1. LCDの幅を自動的に取得
    int32_t lcdWidth = M5.Lcd.width();
    
    String currentLine = ""; // 現在構築中の行
    int currentWidth = 0;  // 現在の行の幅
    
    int i = 0;
    while (i < text.length()) {
        // 2. 次の1文字 (UTF-8) を取得
        int charLen = 1;
        uint8_t b = text[i];
        if (b >= 0xF0) charLen = 4;
        else if (b >= 0xE0) charLen = 3;
        else if (b >= 0xC0) charLen = 2;

        // 文字が途中で途切れないようにする
        if (i + charLen > text.length()) {
            charLen = text.length() - i;
        }
        String newChar = text.substring(i, i + charLen);

        // 3. 次の1文字の幅を計算
        int newCharWidth = M5.Lcd.textWidth(newChar.c_str());

        // 4. 幅をチェック
        if (currentWidth + newCharWidth > lcdWidth) {
            // この文字を追加すると幅を超える場合
            // 4a. 現在の行を表示
            M5.Lcd.println(currentLine);
            // 4b. 新しい行をこの文字から始める
            currentLine = newChar;
            currentWidth = newCharWidth;
        } else {
            // 4c. 幅に収まる場合、現在の行に追加
            currentLine += newChar;
            currentWidth += newCharWidth;
        }
        
        // 5. 次の文字（バイトインデックス）へ
        i += charLen;
    }
    
    // 6. ループ終了後、残った最後の行を表示
    if (currentLine.length() > 0) {
        M5.Lcd.println(currentLine);
    }
}


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
std::vector<String> allTableNames2;
String fefe;


/**
 * @brief Creates or appends a metadata file at the specified path and saves variables from a map.
 * @param fs The SD card filesystem object.
 * @param fullFilePath The full path of the file to save.
 * @param tableName The name of the table to save within the file.
 * @param data The reference to the MettDataMap to save.
 * @param isError Reference to a boolean that will be set to true if an error occurs, false otherwise.
 */
bool renameTableInMettFile(fs::FS &fs, const String& fullFilePath, const String& oldTableName, const String& newTableName, bool& isError) {
    isError = false;

    // --- 1. Basic Validation ---
    if (!fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error (Rename): File does not exist: %s\n", fullFilePath.c_str());
        isError = true;
        return false;
    }
    if (oldTableName == newTableName) {
        Serial.println("Warning (Rename): Old and new table names are the same. No changes made.");
        return true; 
    }
    if (newTableName.isEmpty() /* || containsInvalidTableNameChars(newTableName) */) {
        Serial.printf("Error (Rename): Invalid new table name: '%s'\n", newTableName.c_str());
        isError = true;
        return false;
    }

    // --- 2. Pre-scan file for conflicts and existence ---
    File preScanFile = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!preScanFile) {
        Serial.printf("Error (Rename): Could not open file for pre-scan: %s\n", fullFilePath.c_str());
        isError = true;
        return false;
    }
    bool oldTableFound = false;
    bool newTableConflict = false;
    String targetLinePrefix = "TABLE_NAME:";
    while(preScanFile.available()){
        String line = preScanFile.readStringUntil('\n');
        line.trim();
        if (line.startsWith(targetLinePrefix)) {
            String currentTableName = line.substring(targetLinePrefix.length());
            if (currentTableName == newTableName) {
                newTableConflict = true;
            }
            if (currentTableName == oldTableName) {
                oldTableFound = true;
            }
        }
    }
    preScanFile.close();

    if (newTableConflict) {
        Serial.printf("Error (Rename): A table with the name '%s' already exists. Aborting.\n", newTableName.c_str());
        isError = true;
        return false;
    }
    if (!oldTableFound) {
        Serial.printf("Warning (Rename): Table '%s' not found. No changes made.\n", oldTableName.c_str());
        return true; 
    }

    // --- 3. Rebuild file line-by-line to temp file with replacement ---
    File originalFile = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!originalFile) {
        Serial.printf("Error (Rename): Could not re-open original file for processing: %s\n", fullFilePath.c_str());
        isError = true;
        return false;
    }
    String tempFilePath = fullFilePath + ".tmp";
    File tempFile = fs.open(tempFilePath.c_str(), FILE_WRITE);
    if (!tempFile) {
        Serial.printf("Error (Rename): Failed to open temp file: %s\n", tempFilePath.c_str());
        originalFile.close();
        isError = true;
        return false;
    }

    String finalFileContentForDebug = "";
    
    while (originalFile.available()) {
        String line = originalFile.readStringUntil('\n');
        String trimmedLine = line;
        trimmedLine.trim();
        
        String linePrefix = "TABLE_NAME:";
        if (trimmedLine.startsWith(linePrefix)) {
            String currentTableName = trimmedLine.substring(linePrefix.length());
            if (currentTableName == oldTableName) {
                // This is the exact line to change. Reconstruct it safely.
                line = linePrefix + newTableName;
                tempFile.println(line);
                finalFileContentForDebug += line + "\n";
            } else {
                // It's a TABLE_NAME line, but not the one we're looking for.
                tempFile.println(line);
                finalFileContentForDebug += line + "\n";
            }
        } else {
            // Not a TABLE_NAME line, write it as-is.
            tempFile.println(line);
            finalFileContentForDebug += line + "\n";
        }
    }
    originalFile.close();
    tempFile.close();
    
    // --- 4. Print debug info ---
    Serial.println("\n--- Renaming table. New content will be: ---");
    Serial.print(finalFileContentForDebug);
    Serial.println("------------------------------------------");

    // --- 5. Swap files ---
    if (fs.remove(fullFilePath.c_str())) {
        if (fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
            Serial.printf("Info (Rename): Table '%s' successfully renamed to '%s'.\n", oldTableName.c_str(), newTableName.c_str());
            return true;
        } else {
            Serial.printf("Error (Rename): Failed to rename temp file.\n");
            isError = true;
            fs.remove(tempFilePath.c_str()); // Clean up temp file
            return false;
        }
    } else {
        Serial.printf("Error (Rename): Failed to remove original file.\n");
        isError = true;
        fs.remove(tempFilePath.c_str()); // Clean up temp file
        return false;
    }
}



/**
 * @brief .mettファイル内の指定したテーブルから、指定した変数を1つ削除します。
 *
 * @param fs ファイルシステム
 * @param fullFilePath 対象の.mettファイルのフルパス
 * @param tableName 変数を削除するテーブル名
 * @param variableName 削除する変数名
 * @param isError (出力) エラーが発生した場合 true
 * @return bool 処理が成功したか
 */
bool removeMettVariable(fs::FS &fs, const String& fullFilePath, const String& tableName, const String& variableName, bool& isError) {
    isError = false;
    if (!fs.exists(fullFilePath.c_str())) return true; // ファイルがなければ何もしない

    String tempFilePath = fullFilePath + ".tmp";
    File file = fs.open(fullFilePath.c_str(), FILE_READ);
    File tempFile = fs.open(tempFilePath.c_str(), FILE_WRITE);

    if (!file || !tempFile) {
        if(file) file.close();
        if(tempFile) tempFile.close();
        isError = true;
        return false;
    }

    String currentTableNameInFile = "";
    bool variableRemoved = false;

    while (file.available()) {
        String line = file.readStringUntil('\n');
        String trimmedLine = line;
        trimmedLine.trim();

        bool shouldSkipLine = false;

        if (trimmedLine.startsWith("### METT_TABLE_ID ###")) {
            currentTableNameInFile = "";
        } else if (trimmedLine.startsWith("TABLE_NAME:")) {
            int colonIndex = trimmedLine.indexOf(':');
            currentTableNameInFile = (colonIndex != -1) ? trimmedLine.substring(colonIndex + 1) : "";
            currentTableNameInFile.trim();
        } else if (currentTableNameInFile == tableName) {
            int firstColon = trimmedLine.indexOf(':');
            if (firstColon > 0) {
                String varName = trimmedLine.substring(0, firstColon);
                if (varName == variableName) {
                    shouldSkipLine = true;
                    variableRemoved = true;
                }
            }
        }
        
        if (!shouldSkipLine) {
            tempFile.println(line);
        }
    }
    file.close();
    tempFile.close();

    // ファイルの入れ替え
    fs.remove(fullFilePath.c_str());
    if (!fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
        Serial.printf("Error: Failed to rename temp file after removing variable.\n");
        isError = true;
        return false;
    }

    if (variableRemoved) {
        Serial.printf("Info: Variable '%s' removed from table '%s'.\n", variableName.c_str(), tableName.c_str());
    } else {
        Serial.printf("Info: Variable '%s' not found in table '%s'. No changes made.\n", variableName.c_str(), tableName.c_str());
    }
    return true;
}

/**
 * @brief .mettファイル内の指定したテーブル（ヘッダーと変数すべて）を削除します。
 *
 * @param fs ファイルシステム
 * @param fullFilePath 対象の.mettファイルのフルパス
 * @param tableName 削除するテーブル名
 * @param isError (出力) エラーが発生した場合 true
 * @return bool 処理が成功したか
 */
bool removeMettTable(fs::FS &fs, const String& fullFilePath, const String& tableName, bool& isError) {
    isError = false;
    if (!fs.exists(fullFilePath.c_str())) return true;

    String tempFilePath = fullFilePath + ".tmp";
    File file = fs.open(fullFilePath.c_str(), FILE_READ);
    File tempFile = fs.open(tempFilePath.c_str(), FILE_WRITE);
    
    if (!file || !tempFile) {
        if(file) file.close();
        if(tempFile) tempFile.close();
        isError = true;
        return false;
    }

    bool isSkippingBlock = false;
    bool tableRemoved = false;

    while (file.available()) {
        String line = file.readStringUntil('\n');
        String trimmedLine = line;
        trimmedLine.trim();

        if (trimmedLine.startsWith("### METT_TABLE_ID ###")) {
            isSkippingBlock = false;
        }
        
        if (trimmedLine.startsWith("TABLE_NAME:")) {
            int colonIndex = trimmedLine.indexOf(':');
            String currentTableName = (colonIndex != -1) ? trimmedLine.substring(colonIndex + 1) : "";
            currentTableName.trim();
            if (currentTableName == tableName) {
                isSkippingBlock = true;
                tableRemoved = true;
            }
        }
        
        if (!isSkippingBlock) {
            tempFile.println(line);
        }
    }
    file.close();
    tempFile.close();

    // ファイルの入れ替え
    fs.remove(fullFilePath.c_str());
    if (!fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
        Serial.printf("Error: Failed to rename temp file after removing table.\n");
        isError = true;
        return false;
    }

    if (tableRemoved) {
        Serial.printf("Info: Table '%s' removed from file.\n", tableName.c_str());
    } else {
        Serial.printf("Info: Table '%s' not found. No changes made.\n", tableName.c_str());
    }
    return true;
}



bool test_load(){
  std::vector<MettVariableInfo> loadedVariablesE;
  bool temp1;
  bool temp2;
  
  loadMettFile(SD, DirecX + ggmode, fefe, temp1, temp2, loadedVariablesE);
  dataToSaveE = copyVectorToMap(loadedVariablesE);
  if(temp1){
    
    return true;
  }else{
    return false;
  }
}

bool optkobun(){
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(1);
  std::vector<MettVariableInfo> loadedVariablesE;
    if(!test_load()){
    Serial.println("ロードエラー");
    return false;
  }
 
  M5.Lcd.println(String("  Index Number:") + dataToSaveE["table_opt1"] 
            + "\n  tabletype:" + dataToSaveE["table_opt2"] +
             "\n  tag:" + dataToSaveE["table_opt3"] + "\n\n table options!");
          positpoint = 0;
          maxpage = -1;
          imano_page = 0;
          positpointmax = 3;
          mainmode = 16;
         
          bool temp1;
          bool temp2;
  return true;
          
}

void opt1_kaimei(int id){
  if(!test_load()){
    kanketu("Load Failed!",500);
    return;
  }
  Serial.println("Current SuperT: " + dataToSaveE["table_opt1"]);
  SuperT = dataToSaveE["table_opt1"];
  
  while(true){
    textluck();
    delay(1);
    if(entryenter == 1){
      entryenter = 0;
      if(true){
        
        bool sus = false;
        if(!test_load()){
          return;
        }
        if(id == 0){
            dataToSaveE["table_opt1"] = SuperT;
        }else if(id == 1){
            dataToSaveE["table_opt3"] = SuperT;
        }
        
        shokaipointer3();
        Serial.println("fefff" + fefe);
        saveMettFile(SD, DirecX + ggmode, fefe, dataToSaveE, sus);
        if(sus){
          kanketu("Set Success!",500);
        }else{
          kanketu("Set Failed!",500);
        }
        
        if(!optkobun()){
          kanketu("Load Failed!",500);
          M5.Lcd.fillScreen(BLACK);
          positpoint = holdpositpointd;
          imano_page = holdimanopaged;
          positpointmax = holdpositpointmaxd;
          mainmode = 1;
          return;
        }
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

void opt_hukusei(){
  if(!test_load()){
    kanketu("Load Failed!",500);
    return;
  }
  Serial.println("Current SuperT: " + dataToSaveE["table_opt1"]);
  
  
  while(true){
    textluck();
    delay(1);
    if(entryenter == 1){
      entryenter = 0;
      if(isValidTableName(SuperT,AllName,100)){
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0,0);
        M5.Lcd.println("Duplicating...");
        shokaipointer3();
        bool ee = false;
        //fs::FS &fs, const String& fullFilePath, const String& oldTableName, const String& newTableName, bool& isError
        duplicateMettFile(SD,DirecX + ggmode,fefe,SuperT,ee);
        if(!ee){
          kanketu("Duplicated!",200);
          positpoint = 0;
      imano_page = holdimanopage;
      mainmode = 13;
      M5.Lcd.fillScreen(BLACK);
      positpointmax = 5;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;

      return;
        }else{
          kanketu("Duplicate Failed!",500);
          positpoint = 0;
      imano_page = holdimanopage;
      mainmode = 13;
      M5.Lcd.fillScreen(BLACK);
      positpointmax = 5;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;

      return;
        }

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




void createjj(){
  std::vector<MettVariableInfo> loadedVariablesE;
       bool loadSuccess = false;
      bool fileIsEmpty = false;
    loadMettFile(SD, DirecX + ggmode, fefe, loadSuccess, fileIsEmpty, loadedVariablesE);
    if(!loadSuccess){
      Serial.println("load_error!!!");
    }
  dataToSaveE = copyVectorToMap(loadedVariablesE);
        bool jj = false;

        if(datt("table_opt1","0")){
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
          shokaipointer3();
          saveMettFile(SD, DirecX + ggmode, fefe, dataToSaveE, loadSuccess);
          if(!loadSuccess){
            //kanketu("Option Saved!",200);
          }else{
           kanketu("Option Save Failed!",200);
          }
        }
        return;
}


bool isValidHensuValue(String& text, bool isHairetsu) {
    // 1. 文字長のチェック
    if (text.length() > 10000) {
        Serial.printf("Error (ValidateValue): テキストが長すぎます (Max 10000文字)。\n");
        return false;
    }

    // 2. 改行のチェック
    if (!isHairetsu) {
        // 配列でない場合、改行は許可されない
        if (text.indexOf('\n') != -1) {
            Serial.printf("Error (ValidateValue): 改行は許可されていません (isHairetsu=false)。\n");
            return false;
        }
    }

    // --- ここから下は検証成功 (true を返す) ---
    
    // 3. エンコード処理
    
    // 3a. まず '&' を '&6' に置換 (他の置換より先に実行)
    text.replace("&", "&6");

    // 3b. isHairetsu=true の場合のみ改行を '&n' に置換
    if (isHairetsu) {
        text.replace("\n", "&n");
    }
    
    // 3c. その他の禁止文字を置換
    text.replace(":", "&1");
    text.replace("_", "&2"); // (注: ユーザー指定の ',' ではなく '_' を採用)
    text.replace("#", "&3");
    text.replace("%", "&4");
    text.replace(";", "&5");

    // ★ 修正: エンドマーカー "&e" を追加
    text += "&e";

    return true; // 検証・エンコード成功
}

// ★★★ 追加: GyakuhenkanTxt ★★★
/**
 * @brief isValidHensuValueでエンコードされた文字列を、元の表示可能な形式にデコード（逆変換）します。
 * ★ 修正: 末尾のエンドマーカー "&e" を削除します。
 *
 * @param text エンコードされたテキスト (空文字も対応可)
 * @return String デコードされたテキスト
 */
String GyakuhenkanTxt(const String& text) {
    String decodedText = text;

    // ★ 修正: 末尾のエンドマーカー "&e" があれば削除
    if (decodedText.endsWith("&e")) {
        decodedText = decodedText.substring(0, decodedText.length() - 2);
    }

    // 1. '&n' や '&1' などの特殊コードを文字に戻す
    decodedText.replace("&1", ":");
    decodedText.replace("&2", "_");
    decodedText.replace("&3", "#");
    decodedText.replace("&4", "%");
    decodedText.replace("&5", ";");
    decodedText.replace("&n", "\n");

    // 2. 最後に '&6' を '&' に戻す (他の置換が完了した後に実行)
    decodedText.replace("&6", "&");

    return decodedText;
}