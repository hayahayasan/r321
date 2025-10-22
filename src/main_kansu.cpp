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

const String METT_TABLE_NAME_KEY = "table_name"; 
const int METT_CHUNK_SIZE = 1024;
const int MAX_STRING_LENGTH = 65535; 
const int CURSOR_BLINK_INTERVAL = 10; // カーソル点滅のフレーム間隔
String sitagar[10] = {"Net Status","Wifi","FLASHBrowser","SDBrowser","Configs","Options","SD Eject/Format","User Management","Log","Help/About"};
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

bool btnc;
unsigned long lastTextScrollTime;

unsigned long TEXT_SCROLL_INTERVAL_MS;

int scrollPos;
//後で実装　ファイルのゴミ箱移動オプション
//ファイルコピペ時に、ペースト後「カットしますか」を出す
#pragma region <hensu>
String mainprintex = "M5Core3 LAN Activationer";
String sita = "";
String ggmode = "";

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


 // SuperTに格納可能な最大文字数


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
bool showAngleBrackets = true; 


bool datt(String opthensuname,String setname){
  Serial.println("fff" + getMettVariableValue(dataToSaveE,opthensuname) );
  if(getMettVariableValue(dataToSaveE,opthensuname) == ""){
      dataToSaveE[opthensuname] = setname;
      Serial.println("ddf" + dataToSaveE[opthensuname]);
      return true;
  }
  return false;
}

int selectOption(const String options[], int numOptions, const String upperText, const String lowerText) {
    // 実際に表示するオプションと元のインデックスを抽出
    // (options[i]が空文字列でないものだけを抽出)
    String validOptions[100]; // 最大100個まで
    int validIndices[100];
    int validCount = 0;

    for (int i = 0; i < numOptions && validCount < 100; i++) {
        if (options[i].length() > 0) {
            validOptions[validCount] = options[i];
            validIndices[validCount] = i; 
            validCount++;
        }
    }

    if (validCount == 0) {
        // 表示するオプションが一つもなかった場合
        M5.Lcd.fillScreen(RED);
        M5.Lcd.setTextFont(3); // フォントサイズ3
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setCursor(10, M5.Lcd.height() / 2 - 10);
        M5.Lcd.printf("No options!");
        delay(3000);
        M5.Lcd.fillScreen(BLACK);
        return -1; // エラーとして-1を返す
    }

    int selectedIndex = 0; // validOptions配列における現在の選択インデックス
    const int FONT_SIZE = 3; // フォントサイズを3に固定

    // 表示位置の計算
    M5.Lcd.setTextFont(FONT_SIZE);
    int text_height = M5.Lcd.fontHeight(FONT_SIZE); // フォントサイズ3の高さを取得 (約24px)
    int padding_y = 10;
    
    // Y座標の定義
    const int Y_UPPER = padding_y;
    const int Y_SELECT = Y_UPPER + text_height + padding_y;
    const int Y_LOWER = Y_SELECT + text_height + padding_y;
    
    // X座標（左端からのパディング）
    const int X_CURSOR = 10;
    const int TEXT_MAX_WIDTH = M5.Lcd.width() - X_CURSOR; // テキスト表示の最大幅

    // 画面の初期描画
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE); // デフォルトの文字色

    // 1. Upper Text (例: "SELECT OPTION") を描画
    M5.Lcd.setCursor(X_CURSOR, Y_UPPER);
    M5.Lcd.print(upperText);

    // 3. Lower Text (操作説明など) を描画
    M5.Lcd.setCursor(X_CURSOR, Y_LOWER);
    M5.Lcd.print(lowerText);
    
    // 2. 選択肢の初期描画
    // 選択肢表示エリアを黒で初期化
    M5.Lcd.fillRect(X_CURSOR, Y_SELECT, TEXT_MAX_WIDTH, text_height, BLACK); 
    M5.Lcd.setCursor(X_CURSOR, Y_SELECT);
    
    // ★修正箇所 1/2: 背景色を BLUE から BLACK に変更
    M5.Lcd.setTextColor(YELLOW, BLACK); // 選択時の色（文字色=黄, 背景色=黒）
    M5.Lcd.printf("-%s-", validOptions[selectedIndex].c_str());

    while (true) {
        // ボタン状態の更新
        M5.update();

        bool selectionChanged = false;

        // BtnA (左): 上へ (インデックスを減らす)
        if (M5.BtnA.wasPressed()) {
            selectedIndex = (selectedIndex == 0) ? validCount - 1 : selectedIndex - 1;
            selectionChanged = true;
        }

        // BtnC (右): 下へ (インデックスを増やす)
        if (M5.BtnC.wasPressed()) {
            selectedIndex = (selectedIndex == validCount - 1) ? 0 : selectedIndex + 1;
            selectionChanged = true;
        }

        // 選択が変更された場合、該当行のみを再描画
        if (selectionChanged) {
            // 選択肢の表示領域を黒でクリア
            M5.Lcd.fillRect(X_CURSOR, Y_SELECT, TEXT_MAX_WIDTH, text_height, BLACK);
            
            // 新しい選択肢を描画
            M5.Lcd.setCursor(X_CURSOR, Y_SELECT);
            
            // ★修正箇所 2/2: 背景色を BLUE から BLACK に変更
            M5.Lcd.setTextColor(YELLOW, BLACK); // 選択肢の色
            M5.Lcd.printf("-%s-", validOptions[selectedIndex].c_str());
        }

        // BtnB (中央): 決定
        if (M5.BtnB.wasPressed()) {
            // 元の配列(options[])におけるインデックスを返す
            return validIndices[selectedIndex]; 
        }

        delay(1); // CPU負荷軽減
    }
}








void mainkansu_optsd(){
  if(mainmode == 11){
   updatePointer(true);
  
   if(M5.BtnB.wasPressed()){
      String gg = "";
      if(positpoint == 0){
          gg = "nameasc";
      }else if(positpoint == 1){
          gg = "namedesc";
      }else if (positpoint == 2){
          gg  ="dateasc";
      }
      
      bool loadSuccess = false;
    bool fileIsEmpty = false;
    std::vector<MettVariableInfo> loadedVariables;
    M5.Lcd.fillScreen(BLACK);   
    loadMettFile(SD, "/save/save1.mett", "TestOpt1", loadSuccess, fileIsEmpty, loadedVariables);
       if(loadSuccess){
        optiontxt[3] = gg;
        MettDataMap dataToSave = copyVectorToMap(loadedVariables);
        dataToSave["onlinetype"] = gg;
        saveMettFile(SD, "/save/save1.mett", "TestOpt1", dataToSave, loadSuccess);
        if(!loadSuccess){
          kanketu("save success!",500);
         
          M5.Lcd.fillScreen(BLACK);
          mainmode = 7;
          return;
        }else{
          kanketu("save error!",500);
          M5.Lcd.fillScreen(BLACK);
          mainmode = 7;
          return;
        }
       }else{
          kanketu("load error!",500);
          M5.Lcd.fillScreen(BLACK);
          mainmode = 7;
          return;
       }
   }





 }
  else if(mainmode == 10){
   updatePointer(true);
  
   if(M5.BtnB.wasPressed()){
      String gg = "";
      if(positpoint == 0){
          gg = "nameasc";
      }else if(positpoint == 1){
          gg = "namedesc";
      }else if (positpoint == 2){
          gg  ="dateasc";
      }else if(positpoint == 3){
          gg = "datedesc";
      }else if(positpoint == 4){
        gg = "sizeasc";
      }else if(positpoint == 5){
          gg = "sizedesc";
      }
      
      bool loadSuccess = false;
    bool fileIsEmpty = false;
    std::vector<MettVariableInfo> loadedVariables;
    M5.Lcd.fillScreen(BLACK);   
    loadMettFile(SD, "/save/save1.mett", "TestOpt1", loadSuccess, fileIsEmpty, loadedVariables);
       if(loadSuccess){
        MettDataMap dataToSave = copyVectorToMap(loadedVariables);
        dataToSave["sorttype"] = gg;
        saveMettFile(SD, "/save/save1.mett", "TestOpt1", dataToSave, loadSuccess);
        if(!loadSuccess){
          kanketu("save success!",500);
         optiontxt[2] = gg;
          M5.Lcd.fillScreen(BLACK);
          mainmode = 7;
          return;
        }else{
          kanketu("save error!",500);
          M5.Lcd.fillScreen(BLACK);
          mainmode = 7;
          return;
        }
       }else{
          kanketu("load error!",500);
          M5.Lcd.fillScreen(BLACK);
          mainmode = 7;
          return;
       }
   }





 }
 else if(mainmode == 9){
   updatePointer(true);
  
   if(M5.BtnB.wasPressed()){
      String gg = "";
      if(positpoint == 0){
          gg = "uni";
      }else if(positpoint == 1){
          gg = "ansi";
      }else if (positpoint == 2){
          gg  ="utf8";
      }else if(positpoint == 3){
          gg = "utf16";
      }
      bool loadSuccess = false;
    bool fileIsEmpty = false;
    std::vector<MettVariableInfo> loadedVariables;
    M5.Lcd.fillScreen(BLACK);   
    loadMettFile(SD, "/save/save1.mett", "TestOpt1", loadSuccess, fileIsEmpty, loadedVariables);
       if(loadSuccess){
        optiontxt[1] = gg;
        MettDataMap dataToSave = copyVectorToMap(loadedVariables);
        dataToSave["stringtype"] = gg;
        saveMettFile(SD, "/save/save1.mett", "TestOpt1", dataToSave, loadSuccess);
        if(!loadSuccess){
          kanketu("save success!",500);
         
          M5.Lcd.fillScreen(BLACK);
          mainmode = 7;
          return;
        }else{
          kanketu("save error!",500);
          M5.Lcd.fillScreen(BLACK);
          mainmode = 7;
          return;
        }
       }else{
          kanketu("load error!",500);
          M5.Lcd.fillScreen(BLACK);
          mainmode = 7;
          return;
       }
   }





 }
else if(mainmode == 8){
   updatePointer(true);
  
   if(M5.BtnB.wasPressed()){
      String gg = "";
      if(positpoint == 0){
          gg = "txt";
      }else if(positpoint == 1){
          gg = "mett";
      }else if (positpoint == 2){
          gg  ="tbl";
      }else if(positpoint == 3){
          gg = "yourself";
      }else if(positpoint == 4){
          gg = "cpp";
      }else if(positpoint == 5){
        M5.Lcd.fillScreen(BLACK);
        
          mainmode = 7;
          return;
      }
      bool loadSuccess = false;
    bool fileIsEmpty = false;
    std::vector<MettVariableInfo> loadedVariables;
    M5.Lcd.fillScreen(BLACK);   
    loadMettFile(SD, "/save/save1.mett", "TestOpt1", loadSuccess, fileIsEmpty, loadedVariables);
       if(loadSuccess){
        MettDataMap dataToSave = copyVectorToMap(loadedVariables);
        dataToSave["file_ext"] = gg;
        
        saveMettFile(SD, "/save/save1.mett", "TestOpt1", dataToSave, loadSuccess);
        if(!loadSuccess){
          kanketu("save success!",500);
         optiontxt[0] = gg;
          M5.Lcd.fillScreen(BLACK);
          mainmode = 7;
          return;
        }else{
          kanketu("save error!",500);
          M5.Lcd.fillScreen(BLACK);
          mainmode = 7;
          return;
        }
       }else{
          kanketu("load error!",500);
          M5.Lcd.fillScreen(BLACK);
          mainmode = 7;
          return;
       }
   }





 } else if(mainmode == 7){  

   
int ril = 0; // rilを0で初期化 (ボタンが押されていない状態)

String sse = wirecheck();
if(sse == "E"){
  Serial.println("created test");
}
    
        if (M5.BtnA.wasPressed()) {
            ril = 1; // BtnAが押された
        } else if (M5.BtnC.wasPressed()) {
            ril = 2; // BtnCが押された
        }

        updatePointerAndDisplay(ril); // rilの値に応じてポインターを更新し、表示
        if(M5.BtnB.wasPressed()){
          if(currentPos == 0){
            M5.Lcd.fillScreen(BLACK);
            
            mainmode = 8;
            positpoint = 0;
            holdpositpoint = 0;
            positpointmax = 4;
            maxpage = 1;
            imano_page = 0;
            M5.Lcd.fillScreen(BLACK);
         
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.println("  .txt\n  .mett\n  .tbl\n  .(yourself)\n  .cpp\n  back");
            shokaipointer(false);
            
            return;
            


          }
          else if(currentPos == 1){
            M5.Lcd.fillScreen(BLACK);
            
            mainmode = 9;
            positpoint = 0;
            holdpositpoint = 0;
            positpointmax = 3;
            maxpage = 1;
            imano_page = 0;
            M5.Lcd.fillScreen(BLACK);
         
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.println("  unicode\n  ANSI\n  UTF8\n  UTF16");
            shokaipointer(false);
            
            return;
            


          }
          else if(currentPos == 3){
            M5.Lcd.fillScreen(BLACK);
            
            mainmode = 10;
            positpoint = 0;
            holdpositpoint = 0;
            positpointmax = 5;
            maxpage = 1;
            imano_page = 0;
            M5.Lcd.fillScreen(BLACK);
         
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.println("  name asc\n  name desc\n  date asc\n date desc\n size asc\n size desc");
            shokaipointer(false);
            
            return;
            


          }
          else if(currentPos == 4){
            M5.Lcd.fillScreen(BLACK);
            
            mainmode = 11;
            positpoint = 0;
            holdpositpoint = 0;
            positpointmax = 2;
            maxpage = 1;
            imano_page = 0;
            M5.Lcd.fillScreen(BLACK);
         
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.println("  only pass\n  no pass\n  passandusid");
            shokaipointer(false);
            
            return;
            


          }else if(currentPos == 5){
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0, 0);
            sita = "hello";
            textexx();
            positpoint = 0;
            holdpositpoint = 0;
            imano_page = 0;
            mainmode = 0;
          }
        }

 }
#pragma endregion
 # pragma region <filemode>
  else if(mainmode == 6){
    delay(1);
    if(maxLinesPerPage2 == 1){
          positpoint = 0;
    }
    if(entryenter == 2){
      entryenter = 0;
      mainmode = 1;
      SuperT = "";
      // 次のページを表示
      shokaipointer();
      
      return;
    }
    else if(entryenter == 1){
      entryenter = 0;
      if(filebrat){
          if(isValidWindowsFileName(SuperT)){
        Textex = "renaming file...";
        Serial.println("jj" + DirecX + Filelist[nowposit()] + " J" + nowposit());
        bool gg = renameSDItem(DirecX + Filelist[nowposit()], DirecX + SuperT);
        if(gg){
            entryenter = 0;
          mainmode = 1;
          SuperT = "";
            // 次のページを表示
          
          kanketu("Rename succeed" , 500);
          shokaipointer();
          return;
        }else{
          Textex = "Error Occured!";

        }
      }else{
        Textex = "Invalid File Name! try again";
      }
      Serial.println(SuperT);
      }else{
        if(isValidWindowsDirName(SuperT)){
        Textex = "renaming dir...";
        Serial.println(maeredirect(DirecX) + "/" + SuperT + ":" + DirecX);
        bool gg = renameSDItem(DirecX + maereposit,  maeredirect(DirecX) +  SuperT);
        if(gg){
            entryenter = 0;
          mainmode = 1;
          SuperT = "";
            // 次のページを表示
          
          kanketu("Rename succeed" , 500);
          shokaipointer();
          return;
        }else{
          Textex = "Error Occured!";

        }
      }else{
        Textex = "Invalid Directory Name! try again";
      }
      Serial.println(SuperT);
      }
      
    }else{
      textluck();
    }
  }
  //Serial.println("mainmode:" + String(mainmode));
  else if(mainmode == 5){
    delay(1);
    textluck();
    if(entryenter == 2){
            entryenter = 0;
      mainmode = 1;
      SuperT = "";
      // 次のページを表示
      shokaipointer();
      
      return;
    }else if (entryenter == 1){
      entryenter = 0;
      if(isValidWindowsFileName(SuperT)){
        Textex = "making file...";
        int g = createFile(SuperT,DirecX);
        if(g == 0){
          
          mainmode = 1;
          SuperT = "";
          kanketu("we made it",500);
          positpoint = 0;
          imano_page = 0;
          // 次のページを表示
          shokaipointer();
          
          return;
        }else{
          kanketu("making file failed",500);
          DirecX = maeredirect(DirecX);
          shokaipointer();
          mainmode = 2;
          return;
        }
    }else{
      Textex = "Invalid File Name! try again";
    }
  }
  }
  else if(mainmode == 4){
    updatePointer((bool)false);
     if(pagemoveflag == 4 && btna){
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
   }
    if(positpoint == 2 && M5.BtnB.wasPressed() && DirecX != "/"){//delete dir
      // ディレクトリの削除
      bool dd = areusure();
      if(dd){
        int result = deleteRightmostSDItem(DirecX);
        if(result == 0){
          kanketu("success deleted dir",500);
          DirecX = maeredirect(DirecX);
          Serial.println(DirecX);
          positpoint = 0;
          imano_page = 0;

          shokaipointer();
          mainmode = 1;
          return;
        }else{
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
        }
      

      }else{
        kanketu("delete failed",500);
        DirecX = maeredirect(DirecX);
        shokaipointer();
        mainmode = 2;
        return;
      }
    }
    if(positpoint == 1 && M5.BtnB.wasPressed()){  //rename dir
      
          M5.Lcd.fillScreen(BLACK);
      firstScrollLoop = true;
        mainmode = 6;
        entryenter = false;
        SuperT=migidkae(karadirectname);
        karadirectname = SuperT;
        Serial.println(SuperT);
        SCROLL_INTERVAL_FRAMES = 1;
        SCROLL_SPEED_PIXELS = 3;
        firstScrollLoop = true;
        filebrat = false;
        cursorIndex = 0;
      Textex = "If you wanna end,press tab key. If you wanna edit, please end with "".txt""";
      return;
    }
    if(positpoint == 0 && M5.BtnB.wasPressed()){//make file
    
        M5.Lcd.fillScreen(BLACK);
      firstScrollLoop = true;
        mainmode = 5;
        entryenter = false;
        bool loadSuccess = false;
        bool fileIsEmpty = false;
        std::vector<MettVariableInfo> loadedVariables;
        loadMettFile(SD, "/save/save1.mett", "TestOpt1", loadSuccess, fileIsEmpty, loadedVariables);
       if(loadSuccess){
        MettDataMap dataToSave = copyVectorToMap(loadedVariables);
        if(dataToSave["file_ext"] == "yourself"){
          SuperT = "";
        }else if(dataToSave["file_ext"] == "txt"){
          SuperT = ".txt";
       }else if(dataToSave["file_ext"] == "cpp"){
          SuperT = ".cpp";
       }else if(dataToSave["file_ext"] == "mett"){
          SuperT = ".mett";
        }else if(dataToSave["file_ext"] == "tbl"){
          SuperT = ".tbl"; 
        }
       }else{
        SuperT = "";
       }
        
        SCROLL_INTERVAL_FRAMES = 1;
        SCROLL_SPEED_PIXELS = 3;
        firstScrollLoop = true;
        cursorIndex = 0;
      Textex = "If you wanna end,press tab key. If you wanna edit, please end with "".txt""";
      return;
    }
    if(positpoint == 3 && M5.BtnB.wasPressed()){//make dir
      bool bb = areusure();
      if(bb){
      M5.Lcd.fillScreen(BLACK);
        SuperT = "";
        Textex = "If you wanna end,press tab key.";
        SCROLL_INTERVAL_FRAMES = 1;
        SCROLL_SPEED_PIXELS = 3;
        firstScrollLoop = true;
        mainmode = 3;
        cursorIndex = 0;
        entryenter = false;
        return;

      }else{
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
      }
    }
    if(positpoint == 4 && M5.BtnB.wasPressed()){//paste file
    
     
    //Serial.println("cc " + copymotroot + "dd2 " + DirecX);
    M5.Lcd.fillScreen(BLACK);
    if(copymotroot == ""){
      kanketu("No Copy Dir!",500);
      M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
    }
    // Serial.println(DirecX + ":::" + copymotroot);
    bool dd =     areubunki("Cut","No Cut");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(File_goukeifont);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    //M5.Lcd.println("   Create Dir\n   Delete File\n   Rename\n   Make File\n   CopyFileorPDir\n   Paste Them\n   Rename/DelPDir\n   FileInfo/Edit\n   Back Home\n  File Property" );
    Serial.println("try paste:" + (String)dd);
    
    String h2hh = DirecX;
    if(DirecX != "/"){
      h2hh = DirecX.substring(0,DirecX.length() - 1);
    }
    Serial.println("CC: " + copymotroot + "DD: " + h2hh);
    bool success = smartCopy(copymotroot,h2hh,dd);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(File_goukeifont);
    
    if(success){
     kanketu("paste successed!",500);
    }else{
      kanketu("paste failed!",500);
    }
    positpoint = 0 ;
    holdpositpoint = 0;
        imano_page = 0;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
    }
    if(positpoint == 5 && M5.BtnB.wasPressed()){//戻る
        bool cc = areubunki("back to SD Viewer","back to Main menu");
    if(!cc){
      M5.Lcd.setTextSize(sizex);
       M5.Lcd.setTextColor(WHITE, BLACK); // 白文字、黒背景
  
  // 左上すれすれ (0,0) に表示
        M5.Lcd.setCursor(0, 0);
        sita = tttt;
        textexx();
        positpoint = 0;
        holdpositpoint = 0;
        imano_page = 0;
        mainmode = 0;
        return;
    }else{
      

      M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = 0;
        mainmode = 1;
        DirecX = maeredirect(DirecX);
        holdpositpoint = 0;
        imano_page = 0;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
    }
      }
    }

  else if(mainmode == 3){
    delay(1);
    if(entryenter == 2){
      entryenter = 0;
      mainmode = 1;
      SuperT = "";
      // 次のページを表示
      shokaipointer();
      
      return;
    }
    else if(entryenter == 1){
      entryenter = 0;
      if(isValidWindowsDirName(SuperT)){
        Textex = "making dir...";
        int g = createDirectory(SuperT,DirecX);
        if(g == 0){
            entryenter = 0;
          mainmode = 1;
          SuperT = "";
            // 次のページを表示
          shokaipointer();
          
          return;
        }else{
          Textex = "Error Occured!";

        }
      }else{
        Textex = "Invalid Directory Name! try again";
      }
      Serial.println(SuperT);
    }else{
      textluck();
    }

  }

  else if(mainmode == 2){
    
    String key = wirecheck(); // wirecheck()は常に呼び出される
    updatePointer(false);
   if(pagemoveflag == 4 && btna){
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
   }
    else if(M5.BtnB.wasPressed() && positpoint == 0){//create dir
      bool dd = areusure();
      if(dd){
        M5.Lcd.fillScreen(BLACK);
        SuperT = "";
        Textex = "If you wanna end,press tab key.";
        SCROLL_INTERVAL_FRAMES = 1;
        SCROLL_SPEED_PIXELS = 3;
        firstScrollLoop = true;
        mainmode = 3;
        cursorIndex = 0;
        entryenter = false;
        return;

      }else{
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
      }
    }
  else  if(M5.BtnB.wasPressed() && positpoint == 3){//Make File
      M5.Lcd.fillScreen(BLACK);
      firstScrollLoop = true;
        mainmode = 5;
        entryenter = false;
        bool loadSuccess = false;
        bool fileIsEmpty = false;
        std::vector<MettVariableInfo> loadedVariables;
        loadMettFile(SD, "/save/save1.mett", "TestOpt1", loadSuccess, fileIsEmpty, loadedVariables);
        if(loadSuccess){
        MettDataMap dataToSave = copyVectorToMap(loadedVariables);
        if(dataToSave["file_ext"] == "yourself"){
          SuperT = "";
        }else if(dataToSave["file_ext"] == "txt"){
          SuperT = ".txt";
       }else if(dataToSave["file_ext"] == "cpp"){
          SuperT = ".cpp";
       }else if(dataToSave["file_ext"] == "mett"){
          SuperT = ".mett";
        }else if(dataToSave["file_ext"] == "tbl"){
          SuperT = ".tbl"; 
        }
       }else{
        SuperT = "";
       }

        
        SCROLL_INTERVAL_FRAMES = 1;
        SCROLL_SPEED_PIXELS = 3;
        firstScrollLoop = true;
        cursorIndex = 0;
      Textex = "If you wanna end,press tab key. If you wanna edit, please end with "".txt""";
      return;
    }
  else  if(M5.BtnB.wasPressed() && positpoint == 1){//Delete File
      M5.Lcd.fillScreen(BLACK);
      bool dd = areusure();
      if(dd){
        int positpointgg = positpoint;
        Serial.println("AAAJJJ   " +DirecX +  Filelist[nowpositZ()] + ":J" + String(nowpositZ()));
        int result = deleteRightmostSDItem(DirecX +  Filelist[nowpositZ()]);
        if(result == 0){
          kanketu("success deleted file",500);
          //DirecX = maeredirect(DirecX);
          Serial.println(DirecX);
          positpoint = 0;
          imano_page = 0;

          shokaipointer();
          mainmode = 1;
          return;
        }else{ 
          kanketu("delete file error",500);
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
        }
      
    }else{
      M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
    }

  }
  else if(M5.BtnB.wasPressed() && positpoint == 2){ //ファイルリネーム
    M5.Lcd.fillScreen(BLACK);
      firstScrollLoop = true;
        mainmode = 6;
        positpoint = 0;
        holdpositpoint = 0;
        filebrat = true;
        entryenter = false;
        SuperT=Filelist[nowpositZ()];
        SCROLL_INTERVAL_FRAMES = 1;
        SCROLL_SPEED_PIXELS = 3;
        firstScrollLoop = true;
        cursorIndex = 0;
      Textex = "If you wanna end,press tab key. If you wanna edit, please end with "".txt""";
      return;
  }
  else if(M5.BtnB.wasPressed() && positpoint == 4){ //ファイルコピー
   // bool dd = areubunki("Copy this file","Copy this pdir");   //フォルダコピーは技術的に難しいため没
   bool dd = true;
    if(ForDlist[positpoint] == 0){//ファイルコピー
      copymotroot =  DirecX +  Filelist[nowpositZ()];
      copymotdir = false;
      kanketu("copied(file)",500);
      M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = 0;
        holdpositpoint = 0;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
    }else{//ディレクトリコピー
       kanketu("you cannot copy folder!",500);
      M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = 0;
        holdpositpoint = 0;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
    }
  }
    else if(M5.BtnB.wasPressed() && positpoint == 5){ //ファイルペースト
    //Serial.println("cc " + copymotroot + "dd2 " + DirecX);
    M5.Lcd.fillScreen(BLACK);
    if(copymotroot == ""){
      kanketu("No Copy Dir!",500);
      M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
    }
    // Serial.println(DirecX + ":::" + copymotroot);
    bool dd =     areubunki("Cut","No Cut");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(File_goukeifont);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    //M5.Lcd.println("   Create Dir\n   Delete File\n   Rename\n   Make File\n   CopyFileorPDir\n   Paste Them\n   Rename/DelPDir\n   FileInfo/Edit\n   Back Home\n  File Property" );
    Serial.println("try paste:" + (String)dd);
    
    String hh = DirecX;
    if(DirecX != "/"){
      hh = DirecX.substring(0,DirecX.length() - 1);
    }
    Serial.println("CC: " + copymotroot + "DD: " + hh);
    bool success = smartCopy(copymotroot,hh,dd);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(File_goukeifont);
    
    if(success){
      
    }else{
      kanketu("paste failed!",500);
    }
    positpoint = 0 ;
    holdpositpoint = 0;
        imano_page = 0;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
    
    
  }
  else if(M5.BtnB.wasPressed() && positpoint == 6){//Delete or Rename Pdir
    if(DirecX == "/"){
      kanketu("root folder cannot be edited!",500);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(File_goukeifont);
      positpoint = holdpositpoint;
      mainmode = 1;

      // SDカードコンテンツの初期表示
      shokaipointer();
      return;
    }
    bool cc1 = areubunki("Next","Back");
    if(cc1){
      
        DirecX = maeredirect(DirecX);
        firstScrollLoop = true;
        mainmode = 6;
        entryenter = false;
        SuperT=migidkae(karadirectname);
        karadirectname = SuperT;
        Serial.println(SuperT);
        
        firstScrollLoop = true;
        filebrat = false;
        cursorIndex = 0;
        Textex = "If you wanna end,press tab key. If you wanna edit, please end with "".txt""";
        return;
      
    }else{
      M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpointd;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
    }



       
  }
  else if(M5.BtnB.wasPressed() && positpoint == 8){ //戻る

    bool cc = areubunki("back to SD Viewer","back to Main menu");
    if(!cc){
      M5.Lcd.setTextSize(sizex);
       M5.Lcd.setTextColor(WHITE, BLACK); // 白文字、黒背景
  
  // 左上すれすれ (0,0) に表示
        M5.Lcd.setCursor(0, 0);
        sita = "hello ";
        textexx();
        positpoint = 0;
        holdpositpoint = 0;
        imano_page = 0;
        mainmode = 0;
        return;
    }else{
      

      M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
    }
  }
  
  else if(M5.BtnB.wasPressed() && positpoint == 7){ //edit file
  String gggs = DirecX + Filelist[holdpositpoint];
  // ファイル名の末尾にある改行文字や空白を削除
  gggs.trim();

  // シリアルモニタで開こうとしているファイル名を確認
  Serial.println("Attempting to open file: '" + gggs + "'");

  // ファイルを読み取りモードで開く
  File myFile = SD.open(gggs, FILE_READ);

  // SD.open()が成功したかを確認
  if (myFile) {
    Serial.println("File opened successfully.");

    // 1. ファイル容量を取得 (long)
    long fileSize = myFile.size();

    // 2. 最終更新日時を取得
    time_t lastWriteTime = myFile.getLastWrite();
    
    // 取得後は必ずファイルを閉じる
    myFile.close();

    // 日時情報を構造体に変換
    struct tm *timeinfo;
    timeinfo = localtime(&lastWriteTime);

    // M5Stackの画面をクリア
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);

    // 取得した情報を画面に表示
    M5.Lcd.println("File Information:");
    M5.Lcd.printf("File Name: %s\n", gggs.c_str());
    M5.Lcd.printf("File Size: %ld bytes\n", fileSize);
    
    // 日時が取得できたか確認し、表示
    if (timeinfo->tm_year > 70) { // tm_yearは1900年からの年数なので、2000年以上の値なら有効と判断
      M5.Lcd.printf("Last Write: %04d/%02d/%02d %02d:%02d:%02d\n",
                    timeinfo->tm_year + 1900, 
                    timeinfo->tm_mon + 1,
                    timeinfo->tm_mday, 
                    timeinfo->tm_hour,
                    timeinfo->tm_min, 
                    timeinfo->tm_sec);
    } else {
      M5.Lcd.println("Last Write: Not available (RTC not set)");
    }

  } else {
    // ファイルが開けなかった場合
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Error: File not found or failed to open!");
    M5.Lcd.printf("File: %s\n", gggs.c_str());
  }

      holdpositpointd = positpoint;
      holdimanopaged = imano_page;
      holdpositpointmaxd = positpointmax;
      mainmode = 12;
      if(ggmode.endsWith(".mett") || ggmode.endsWith(".tbl") || ggmode.endsWith(".txt")){
        M5.Lcd.printf("Wanna Edit? Press BtnB\n");
      }
      return;
      
  }


}
  // mainmodeの値に基づいて処理を分岐
  else if (mainmode == 1) { // SDリスト表示モードの場合
    
    delay(1);
  

    //Serial.println("IMA:" + ForDlist[nowposit()] + "bango" + nowposit());
    if(!nosd){ // !nosd の if ブロック開始

       // Serial.println((String)maxpage);
    String key = wirecheck(); // wirecheck()は常に呼び出される
    
    if(!otroot && !nosd){
      updatePointer(false);
    }
       
       if(btnc && pagemoveflag == 5){
          imano_page = 0;
        positpoint = 0;
        
        pagemoveflag = 0;
        
        shokaipointer();
        mainmode = 1;
        return;
       }
       else if(btna && pagemoveflag == 4){
          imano_page = maxpage - 1;
          if(maxLinesPerPage3 == 0){
            positpoint = maxLinesPerPage - 1;
          }else{
            positpoint = maxLinesPerPage3 - 1;
          }
          
          holdpositpoint = positpoint;
          pagemoveflag = 0;
          
          shokaipointer();
          mainmode = 1;
          return;
       }
      else if(btna && pagemoveflag == 3){
        imano_page = 0;
        positpoint = 0;
        
        pagemoveflag = 0;
        DirecX = maeredirect(DirecX);
        shokaipointer();
        mainmode = 1;
        return;
      }
      else if (btnc && pagemoveflag == 1) {

        imano_page++;
        pagemoveflag = 0;
        
        positpoint = 0;
        holdpositpoint = 0;
        shokaipointer();
        mainmode = 1;
        return;

      } else if (btna && pagemoveflag == 2) {

        imano_page--;
        pagemoveflag = 0;
        
        positpoint = maxLinesPerPage - 1;
        shokaipointer();
        mainmode = 1;
        return;

      } else if(M5.BtnB.wasPressed() && ForDlist[nowposit()] == "1"){ // ここに閉じ括弧が不足していました
        if((Filelist[nowposit()] == "System Volume Information" || Filelist[nowposit()] == "m5stack.server")&& !(positpointmax == 0 && maxpage == 1)){
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setTextSize(1);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.println("This Directory is cannot be edited!");
          Serial.println("pos" + String(positpointmax)  + "m" + String(maxpage));
          delay(1000);
          M5.Lcd.setTextSize(File_goukeifont);

          shokaipointer();
          return;
        } else if((Filelist[0] == "System Volume Information"  && positpointmax == 0 && maxpage == 1) || (DirecX == "/" && !rootnofile)) {
                  mainmode = 4;
        holdpositpoint = positpoint;
        positpointmax = 5;
        positpoint = 0;
        holdpositpoint = 0;
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(WHITE);
        otroot = false;
        M5.Lcd.println("  Make File\n  Rename\n  Delete Dir\n  Make Dir\n  Paste File\n  Back Home" );
        shokaipointer(false);
        return;
        
        
      }else{//次のファイルに進む。
          maereposit = Filelist[nowposit()];
          DirecX = DirecX  + Filelist[nowposit()] + "/"; // 選択したディレクトリのパスを更新
          Serial.println("DZOON:" + DirecX);
          positpoint = 0;
          holdpositpoint = 0;
          imano_page = 0;
          bool ddd = false;
          
           
            resercounter = 1;  
            M5.Lcd.fillScreen(BLACK);
            
            while(M5.BtnB.isPressed()){
              delay(1);
              M5.update();
              resercounter++;
              if(resercounter > 600){
                ddd = true;
                break;
                
              } 
              
            }

           
            if(ddd){//ディレクトリの中にディレクトリだけ作った場合に強制的にmainmode2を起動する隠しコマンド
              mainmode = 2;
              holdpositpoint = positpoint;
              positpointmax = 9;
              positpointmain1 = positpoint;
             positpoint = 0;
              M5.Lcd.fillScreen(BLACK);
              M5.Lcd.setTextSize(3);
              M5.Lcd.setCursor(0, 0);
              M5.Lcd.setTextColor(WHITE);
              
                M5.Lcd.println("  Create Dir\n  Delete File\n  Rename\n  Make File\n  CopyFileorPDir\n  Paste Them\n  Rename/DelPDir\n  FileInfo/Edit\n   Back Home\n  File Property" );
                shokaipointer(false);
                return;
            }else{
              shokaipointer();
              return;
            }
         
    
          
          
          return;
        }
      } // ここに閉じ括弧を追加しました

      else if(M5.BtnB.wasPressed() && ForDlist[nowposit()] == "0"){
        mainmode = 2;
        ggmode = Filelist[nowposit()];
        holdpositpoint = positpoint;
        positpointmax = 9;
        positpointmain1 = positpoint;
        positpoint = 0;

        
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(WHITE);
        if(Filelist[nowposit()] )
       M5.Lcd.println("  Create Dir\n  Delete File\n  Rename\n  Make File\n  CopyFileorPDir\n  Paste Them\n  RenamePDir\n  FileInfo/Edit\n   Back Home\n  File Property" );
        return;
      }
      else if(DirecX != "/" && btna && imano_page == 0 && positpoint == -1){//前リダイレクトに戻る
        DirecX = maeredirect(DirecX); // 一つ前のディレクトリに戻る
          Serial.println(DirecX);
          holdpositpoint = 0;
          imano_page = 0;
          positpoint = 0;
        // 一つ前のディレクトリの内容を表示
          shokaipointer();
          return; 
      }
      
    } else if (otroot){
        if (M5.BtnB.wasPressed() ) {
        mainmode = 4;
        holdpositpoint = positpoint;
        positpointmax = 5;
        positpoint = 0;
        holdpositpoint = 0;
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(WHITE);
        otroot = false;
        M5.Lcd.println("  Make File\n  Rename\n  Delete Dir\n  Make Dir\n  Paste File\n  Back Home" );
        shokaipointer(false);
        return;

      }else if( M5.BtnA.wasPressed()){
        Serial.println(otroot);

          nosd = false;
          DirecX = maeredirect(DirecX); // 一つ前のディレクトリに戻る
          Serial.println(DirecX);
          holdpositpoint = 0;
          imano_page = 0;
          positpoint = 0;
          otroot = false;
        // 一つ前のディレクトリの内容を表示
          shokaipointer();
          return; 
        }
      
    }else if (serious_errorsd && M5.BtnB.wasPressed()) {
        // SDカードのエラーが発生した場合の処理
        
        serious_errorsd = false;
      nosd = false;
        mainmode = 0;
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        textexx();
        return;// !nosd の if ブロック終了 (ここに閉じ括弧を追加しました)
  }

}
}


