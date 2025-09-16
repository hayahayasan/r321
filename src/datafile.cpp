#include <M5Unified.h>
#include <Wire.h>
#include<SD.h>
#include <USB.h> 
#include <FS.h>
#include <vector>    // std::vector を使用するために必要
#include <algorithm>
#include <map>      // std::mapを使用するため
#include <set>
#include "shares.h"
int NUM_RESERVED_NAMES;
// ポインターの位置を更新し、画面下部にテキストをスクロールさせる関数
void updatePointer2(bool notext = false) {
    
    delay(1);
    
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
        Serial.println("r" + String(btna) + "l" + String(btnc) + " " + positpoint + "  " + frameright + frameleft);
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
        M5.Lcd.setTextFont(File_goukeifont); // ポインターフォントが設定されていることを確認
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



// パスの末尾のスラッシュを削除する関数（ルートディレクトリを除く）
String cleanPath(String path) {
    if (path == "/") {
        return path;
    }
    if (path.length() > 1 && path.endsWith("/")) {
        return path.substring(0, path.length() - 1);
    }
    return path;
}
#pragma region <directory_creation>
// ネストされたディレクトリを再帰的に作成する関数
bool createDirRecursive(const char* path) {
    String currentPath = "";
    String pathString = String(path);
    int start = 0;
    if (pathString.startsWith("/")) {
        currentPath += "/";
        start = 1;
    }
    int slashIndex = pathString.indexOf('/', start);
    while (slashIndex != -1) {
        currentPath += pathString.substring(start, slashIndex);
        if (!SD.exists(currentPath)) {
            if (!SD.mkdir(currentPath)) return false;
        }
        currentPath += "/";
        start = slashIndex + 1;
        slashIndex = pathString.indexOf('/', start);
    }
    currentPath += pathString.substring(start);
    if (!SD.exists(currentPath)) {
        if (!SD.mkdir(currentPath)) return false;
    }
    return true;
}

// ファイル名をチェックし、重複する場合は新しい一意な名前を生成する関数
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
    String fileNameOnly = baseName.substring(slashIndex + 1);
    String directoryPath = baseName.substring(0, slashIndex + 1);
    directoryPath = cleanPath(directoryPath);

    for (int i = 1; i <= 1000; i++) {
        String newFileName = directoryPath;
        if (newFileName != "/") {
            newFileName += "/";
        }
        newFileName += fileNameOnly + "(" + String(i) + ")" + extension;
        if (!SD.exists(newFileName)) {
            return newFileName;
        }
    }
    
    return "";
}

// ファイルをコピーする関数（進捗表示付き）
bool copyFile(const char* sourcePath, const char* destinationPath, uint32_t totalSize) {
    File sourceFile = SD.open(sourcePath, FILE_READ);
    if (!sourceFile) {
        return false;
    }
    String destFullPath = String(destinationPath);
    int lastSlash = destFullPath.lastIndexOf('/');
    if (lastSlash > 0) {
        String parentDir = destFullPath.substring(0, lastSlash);
        if (!createDirRecursive(parentDir.c_str())) {
            sourceFile.close();
            return false;
        }
    }
    
    File destinationFile = SD.open(destinationPath, FILE_WRITE);
    if (!destinationFile) {
        sourceFile.close();
        return false;
    }

    uint32_t totalCopiedSize = 0;
    uint8_t buffer[512];
    size_t bytesRead;
    while ((bytesRead = sourceFile.read(buffer, sizeof(buffer))) > 0) {
        destinationFile.write(buffer, bytesRead);
        totalCopiedSize += bytesRead;
        if (totalSize > 0) {
            int percent = (int)((float)totalCopiedSize / totalSize * 100);
            M5.Lcd.setCursor(0, 40);
            M5.Lcd.printf("Copying... %d%% ", percent);
        }
    }
    if (totalSize == 0) {
        M5.Lcd.setCursor(0, 40);
        M5.Lcd.printf("Copying... 100%% ");
    }

    sourceFile.close();
    destinationFile.close();
    return true;
}

// ファイルまたはフォルダを再帰的に削除する関数
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

// メインの処理を実行する関数
bool smartCopy(String sourcePath, String destinationPath, bool isCut) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.println("Initializing SD card...");
    if (!SD.begin()) {
        M5.Lcd.println("[ERROR] SD card initialization failed!");
        return false;
    }

    sourcePath = cleanPath(sourcePath);
    destinationPath = cleanPath(destinationPath);
    
    // 1. コピー元ファイルの存在を確実に確認
    M5.Lcd.printf("Checking source: %s\n", sourcePath.c_str());
    if (!SD.exists(sourcePath)) {
        M5.Lcd.println("[ERROR] Source path does not exist.");
        return false;
    }
    File source = SD.open(sourcePath);
    if (source.isDirectory()) {
        M5.Lcd.println("[ERROR] Folder copy is not supported.");
        source.close();
        return false;
    }
    uint32_t totalSize = source.size();
    source.close();
    M5.Lcd.printf("File size: %d bytes\n", totalSize);

    // 2. コピー先の最終パスを決定
    int lastSlash = sourcePath.lastIndexOf('/');
    String fileNameOnly = sourcePath.substring(lastSlash + 1);
    String finalDestinationPath;
    if (destinationPath == "/") {
        finalDestinationPath = "/" + fileNameOnly;
    } else {
        finalDestinationPath = destinationPath + "/" + fileNameOnly;
    }
    
    // 3. 連番付きのファイルパスを取得
    String uniqueDestPath = checkAndRename(finalDestinationPath);
    if (uniqueDestPath == "") {
        M5.Lcd.println("[ERROR] Paste overflowed!");
        return false;
    }
    M5.Lcd.printf("Copying to: %s\n", uniqueDestPath.c_str());

    // コピー処理を実行
    if (!copyFile(sourcePath.c_str(), uniqueDestPath.c_str(), totalSize)) {
        M5.Lcd.setCursor(0, 40);
        M5.Lcd.println("[ERROR] Copy failed!");
        return false;
    }
    M5.Lcd.setCursor(0, 40);
    M5.Lcd.println("Copy successful!                    ");
    
    // 4. isCutがtrueの場合、コピー元を削除
    if (isCut) {
        M5.Lcd.setCursor(0, 50);
        M5.Lcd.println("Starting removal...");
        if(removePath(sourcePath.c_str())) {
            M5.Lcd.setCursor(0, 50);
            M5.Lcd.println("Move successful!                      ");
        } else {
            M5.Lcd.setCursor(0, 50);
            M5.Lcd.println("[ERROR] Removal failed!");
            return false;
        }
    }

    return true;
}