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
#pragma region <hensu4aa>
const String METT_TABLE_NAME_KEY = "table_name"; 
const int METT_CHUNK_SIZE = 1024;
const int MAX_STRING_LENGTH = 65535; 
const int CURSOR_BLINK_INTERVAL = 10; // カーソル点滅のフレーム間隔
MettDataMap mmmc;
String sitagar[10] = {"Net Status","Wifi","FLASHBrowser","SDBrowser","Configs","Options","SD Eject/Format","User Management","Log","Help/About"};
int SCROLL_INTERVAL_FRAMES = 1;
int SCROLL_SPEED_PIXELS = 3;
int frameright;
int holdpositpointt;
String tttt = "hello";
bool returnss;
String statustext;

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
int fontdd = 0;

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

#pragma endregion <hensu4aa>




void mainkansu_intmain(){
  delay(1);
  if(mainmode == 31){
    updatePointer2(1);
    M5.update();
      
      if(pagemoveflag == 1){
      pagemoveflag = 0;
      
      return;
    
    }else if(pagemoveflag == 5){
      
      M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0, 0);

            positpoint = 0;
            maxpage = -1;
            holdpositpoint = 0;
            imano_page = 0;
            mainmode = 30;
            M5.Lcd.setTextSize(3);
            M5.Lcd.setCursor(0, 0);
            positpointmax =IntNet;
            M5.Lcd.println(TexNet);
       
        
            return;
     
    }else if(M5.BtnB.wasPressed()){

      if(positpoint == 0 || positpoint == 1 || positpoint == 2){
        String ssgggg;

        if(positpoint == 0){
          ssgggg = "table_SSID";
        }else if(positpoint == 1){
          ssgggg = "table_Usrname";
        }else if(positpoint == 2){
          ssgggg = "table_Pass";
        }
       if(textnetsette(ssgggg)){
          resetto31();
          return;
        }else{
          positpoint = 0;
            maxpage = -1;
            holdpositpoint = 0;
            imano_page = 0;
            mainmode = 30;
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.setTextSize(3);
            positpointmax =IntNet;
            M5.Lcd.println(TexNet);
       
        
            return;
        }
      }else if(positpoint == 3){//接続
        if(checkWiFiConnection()){
          kanketu("Already Connection!\n your MAC:" + WiFi.macAddress(),2000);
          M5.Lcd.setTextSize(1);
         resetto31();

          return;
        }
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0,0);
        if(!createEE(mmmc,1)){
          kanketu("!load error!",400);
            M5.Lcd.setCursor(0, 0);
            sita = "hello";
            textexx();
            positpoint = 0;
            holdpositpoint = 0;
            imano_page = 0;
            mainmode = 0;
        }
        if(GyakuhenkanTxt(mmmc["table_SSID"]) == ""){
          kanketu("[E04] No SSID Input",1000);
          positpoint = 0;
            maxpage = -1;
            holdpositpoint = 0;
            imano_page = 0;
            mainmode = 30;
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.setTextSize(3);
            positpointmax =IntNet;
            M5.Lcd.println(TexNet);
       
        
            return;
        }
        if(!connectToEnterpriseWiFi(GyakuhenkanTxt(mmmc["table_SSID"]),GyakuhenkanTxt(mmmc["table_Usrname"]),GyakuhenkanTxt(mmmc["table_Pass"]))){
                  M5.Lcd.fillScreen(BLACK);
                  M5.Lcd.setCursor(0,0);
                  kanketu(UU,2000);
          positpoint = 0;
            maxpage = -1;
            holdpositpoint = 0;
            imano_page = 0;
            mainmode = 30;
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.setTextSize(3);
            positpointmax =IntNet;
            M5.Lcd.println(TexNet);
       
        
            return;
        }
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0,0);
        M5.Lcd.setTextSize(1);
        showmozinn2("connnecterd:\n isglobal:" + WSTT[5] + 
        "\nyour local ip:" + WSTT[0]
         + "\nyour global ip:" + WSTT[1] +
         "\nyour MAC:" + WSTT[2] +
         "\nyour router MAC:" + WSTT[3]);
         statustext = "NetStep:1,Connected to Wifi But no WebSocket";
         while(true){
          delay(1);
          M5.update();
          if(M5.BtnB.wasPressed()){
            break;
          }
         }
         M5.Lcd.setTextSize(1);
         resetto31();

          return;

      }else if(positpoint == 4){//ステータス表示
        if(!checkWiFiConnection()){
          kanketu("No Connection!\n your MAC:" + WiFi.macAddress(),2000);
          M5.Lcd.setTextSize(1);
         resetto31();

          return;
        }
        M5.Lcd.fillScreen(BLACK);
        statustext = "NetStep:0,No Internet!";
        M5.Lcd.setCursor(0,0);
        M5.Lcd.setTextSize(1);
        showmozinn2("connnecterd:\n isglobal:" + WSTT[5] + 
        "\nyour local ip:" + WSTT[0]
         + "\nyour global ip:" + WSTT[1] +
         "\nyour MAC:" + WSTT[2] +
         "\nyour router MAC:" + WSTT[3] + 
        "\nPrimary DNS:" + WSTT[11] +
      "\nSecondary DNS:" + WSTT[12] +
       "\nWifi Type:" + WSTT[18] +
       "\nHost Name:" + WSTT[14] +
       "\nBroadcast IP:" + WSTT[15] );
         while(true){
          delay(1);
          M5.update();
          if(M5.BtnB.wasPressed()){
            break;
          }
         }
         M5.Lcd.setTextSize(1);
         resetto31();

          return;
      }
    }
  }
  else if(mainmode == 30){
    updatePointer2(3);
    M5.update();
      
      if(pagemoveflag == 1){
      pagemoveflag = 0;
      
      return;
    
    }else if(pagemoveflag == 5){
      
      M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0, 0);
            sita = "hello";
            textexx();
            positpoint = 0;
            holdpositpoint = 0;
            imano_page = 0;
            mainmode = 0;
      return;
    }else if(M5.BtnB.wasPressed()){
      if(positpoint == 0){
        M5.Lcd.setCursor(0, 0);
            positpoint = 0;
            maxpage = -1;
            holdpositpoint = 0;
            imano_page = 0;
            mainmode = 31;
          M5.Lcd.fillScreen(BLACK);
          mmmc;
          M5.Lcd.println("SD Waking!");
          SD.begin(GPIO_NUM_4, SPI, 20000000);
          
          
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0, 0);
          if(!createEE(mmmc),0){
            kanketu("!load error!",400);
            M5.Lcd.setCursor(0, 0);
            sita = "hello";
            textexx();
            positpoint = 0;
            holdpositpoint = 0;
            imano_page = 0;
            mainmode = 0;
      return;
          }
          M5.Lcd.setTextSize(1);
          M5.Lcd.println(TexNet1(mmmc));
          positpointmax = IntNet1;
          return;
      }else if(positpoint == 3){//get status

      }else if(positpoint == 4){//disconnect
        if(!checkWiFiConnection()){
          kanketu("No Connection!\n your MAC:" + WiFi.macAddress(),2000);
           M5.Lcd.fillScreen(BLACK);
          positpoint = 0;
            maxpage = -1;
            holdpositpoint = 0;
            imano_page = 0;
            mainmode = 30;
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.setTextSize(3);
            positpointmax =IntNet;
            M5.Lcd.println(TexNet);
          return;
        }
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("Disconnecting...");
        disconnectWiFi();
        kanketu("Disconnected!\n your MAC:" + WiFi.macAddress(),2000);
         M5.Lcd.fillScreen(BLACK);
          positpoint = 0;
            maxpage = -1;
            holdpositpoint = 0;
            imano_page = 0;
            mainmode = 30;
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.setTextSize(3);
            positpointmax =IntNet;
            M5.Lcd.println(TexNet);
          return;
      }

    }
  }

}





void mainkansu_optsd(){
  #pragma region <optmoX>
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
          mainmode = 1;
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
        mainmode = 1;
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
   updatePointer2(3);
   if(pagemoveflag == 5){
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
   }else if(pagemoveflag == 1){
      pagemoveflag = 0;
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
      if(ggmode.endsWith(".mett") ||  ggmode.endsWith(".txt")){
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
              maxpage = -1;
              imano_page = 0;
              positpointmain1 = positpoint;
             positpoint = 0;
              M5.Lcd.fillScreen(BLACK);
              M5.Lcd.setTextSize(3);
              M5.Lcd.setCursor(0, 0);
              M5.Lcd.setTextColor(WHITE);
              
                M5.Lcd.println("  Create Dir\n  Delete File\n  Rename\n  Make File\n  CopyFileorPDir\n  Paste Them\n  Rename/DelPDir\n  FileInfo/Edit\n  Back Home" );
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
       
        suguseni();
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

}else if(mainmode == 12){
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
    if(ggmode.endsWith(".txt") && M5.BtnB.wasPressed()){
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


      
      M5.Lcd.setCursor(0,0);
      M5.Lcd.setTextSize(3);
      M5.Lcd.println("Loading...");
      Serial.println("fe" + DirecX + ggmode);
      String dirrr = DirecX + ggmode;
      
      Serial.println("dt" + dirrr);
      String Texx = "";
      bool suc = readSdFileToStringForced(dirrr,Texx,fontdd);
      SuperT = Texx;
      if(suc){
        mainmode = -11;
        M5.Lcd.fillScreen(BLACK);
        Textex =  "This character code:" + mozikk[fontdd] + "   Press Tab to save!";
        return;
      }else{
        kanketu("File Load Error!",500);
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0,0);
        mainmode = 1;
        positpoint = holdpositpoint;
        shokaipointer();
        return;
      }
      
    }else if(ggmode.endsWith(".mett") && M5.BtnB.wasPressed()){
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
 }else if(mainmode == -11){
   delay(1);
    textluck();
    if(entryenter == 2){//back
      entryenter = 0;
      bool ss = areusure();
      if(ss){
       
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0,0);
        mainmode = 1;
        positpoint = holdpositpoint;
        shokaipointer();
        return;
      }else{
        M5.Lcd.fillScreen(BLACK);
      }
    }else if(entryenter == 1){//enter
      entryenter = 0;
      M5.Lcd.fillScreen(BLACK);
      mainmode = -12;
      M5.Lcd.setCursor(0,0);
      M5.Lcd.setTextSize(3);
      M5.Lcd.println("  No BOM\n  UTF8\n  UTF16\n  UTF32");
      positpointmax = 4;
      maxpage = -1;
      positpoint = 0;
      return;
    }



 }else if(mainmode == -12){
    updatePointer2(3);
    if(pagemoveflag == 2){
      pagemoveflag = 0;
      return;
    }else if(pagemoveflag == 1){
      pagemoveflag = 0;
      return;
    }else if((pagemoveflag == 5) ){
      pagemoveflag = 0;
      M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0,0);
        mainmode = -11;

      return;
    
    }else if(M5.BtnB.wasPressed()){
      if(positpoint == 0){
        fontdd = 0;
      }else if(positpoint == 1){
        fontdd = 1;
      }else if(positpoint == 2){
        fontdd = 2;
      }else if(positpoint == 3){
        fontdd = 5;
      }
      M5.Lcd.fillScreen(BLACK);
      bool ss = writeStringToFileForced(DirecX + ggmode,SuperT,fontdd);
      if(ss){
        kanketu("File Saved!",500);
      }else{
        kanketu("File Save Error!",500);
      }
      M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0,0);
        mainmode = 1;
        positpoint = holdpositpoint;
        positpointmax = holdpositpointmax;
        shokaipointer();
        return;
    }

    #pragma endregion <optmoX>
 }else if (mainmode == 0) { // メニューモードの場合

    String key = wirecheck(); // wirecheck()は常に呼び出される
    //これを入れないとmainmode変数認識が遅れやすい
    // ボタンAが押された場合の処理 (メニューを上に移動)
  scrollTextBottom(statustext);
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
        clearBottomArea();
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
        
        textexx();
        positpoint = 0;
        holdpositpoint = 0;
        imano_page = 0;
          

          return;
        }
        else{
          clearBottomArea();
          mainmode = 7; // モードをSDリスト表示モードに切り替え
          
        }
        
        
        
        return;//mainmode0フラグ誤作動対策
      }else if(maindex == 6){//SDイジェクトフォーマット
        M5.Lcd.fillScreen(BLACK); // 画面をクリア
        M5.Lcd.setTextSize(3);
       M5.Lcd.setTextColor(WHITE, BLACK); // 白文字、黒背景
       M5.Lcd.setCursor(0, 0);
        mainmode = -1;
        clearBottomArea();
        maxpage = -1;
        positpoint = 0;
        positpointmax = 2;
        pagemoveflag = 0;
        imano_page = 0;
        M5.Lcd.println("  SD Eject\n  SD Format");
        return;
      }else if(maindex == 0){//ステップ1 wifi起動モード
        M5.Lcd.fillScreen(BLACK); // 画面をクリア
        M5.Lcd.setTextSize(3);
       M5.Lcd.setTextColor(WHITE, BLACK); // 白文字、黒背景
       M5.Lcd.setCursor(0, 0);
        mainmode = 30;
        
        M5.Lcd.setTextSize(3);
        M5.Lcd.println(TexNet);
        positpoint = 0;
        maxpage = -1;
        positpointmax = IntNet;
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
  }
}




String textsus(String defotrxt,String texnum,int rule){
  SuperT = defotrxt;
  Textex = texnum;
  returnss = false;
  entryenter = 0;
  Serial.println("textsss");
  while(true){
    delay(1);
    textluck();
    if(entryenter == -1){
      SuperT + SuperT + ",";
      entryenter = 0;
    }else if(entryenter == 1){
      entryenter = 0;
      if(rule == 0){//int値だけ(ただし0~10000)

        if(!isValidInteger0To100000(SuperT)){
          Textex = "Invalid Integer(0~100000)! Try Again";
          
          
      }else{
        returnss = false;
      return SuperT;
      }
      }else if(rule ==1){
        if(!isValidHensuValue(SuperT,false)){
          Textex = "Invalid Hensu Value(no kaigho)! Try Again";
          
          
      }else{
        returnss = false;
      return SuperT;
      }
      }else if(rule ==2){
        if(!isValidHensuValue(SuperT,true)){
          Textex = "Invalid Hensu Value(yes kaigho)! Try Again";
          
          
      }else{
        returnss = false;
      return SuperT;
      }
      }
    }else if(entryenter == 2){
      entryenter = 0;
      Serial.println("canceLLLLLLLL");
      returnss = true;
      return "";
    }
  }
}



 bool isValidInteger0To100000(String& input) {
    // 0. 改行文字が含まれていないかをチェック (ユーザーの明示的な要求)
    if (input.indexOf('\n') != -1 || input.indexOf('\r') != -1) {
        Serial.printf("Validation Error: Input contains newline character.\n");
        return false;
    }
    
    // 1. 文字列が空でないことをチェック
    if (input.length() == 0) {
        Serial.printf("Validation Error: Input is empty.\n");
        return false;
    }

    // ★★★ 追加: 先頭の不要な '0' を削除 (正規化) ★★★
    // 長さが1より大きく、かつ先頭が '0' である間、先頭を削除し続ける
    // これにより "0005" -> "5", "0" -> "0", "00" -> "0" となる
    while (input.length() > 1 && input.charAt(0) == '0') {
        input.remove(0, 1);
    }
    // ★★★ ここまで ★★★

    // 2. すべての文字が数字であることをチェック (toInt()の信頼性を高めるため。符号 '-' や '+' も拒否)
    for (size_t i = 0; i < input.length(); ++i) {
        if (!isDigit(input.charAt(i))) {
            Serial.printf("Validation Error: Input contains non-digit character: '%s'\n", input.c_str());
            return false;
        }
    }

    // 3. 整数に変換
    // String::toInt()はintを返す。100000はintに収まるため安全。
    long num = input.toInt(); 
    
    // 4. 範囲チェック: 0 <= num <= 100000
    if (num >= 0 && num <= 100000) {
        // Serial.printf("Validation Success: %ld is within range [0, 100000].\n", num);
        return true;
    } else {
        Serial.printf("Validation Error: Number %ld is outside range [0, 100000].\n", num);
        return false;
    }
}

void suguseni(){

Serial.println("feaef" + Filelist[holdpositpoint]);

  
          if(Filelist[holdpositpoint].endsWith(".txt" )||  Filelist[holdpositpoint].endsWith(".mett") ){
          bool sss = false;
          int ggcounter = 0;
          while(true){
            delay(1);
            
            if(M5.BtnB.isPressed()){
              ggcounter++;
            }
            if(!M5.BtnB.isPressed()  || ggcounter > 600){
              break;
            }
            M5.update();
          }
          if(ggcounter > 600){
            ggmode = Filelist[holdpositpoint];
            mainmode = 13;
            
      M5.Lcd.setCursor(0,0);
      M5.Lcd.setTextSize(3);
      M5.Lcd.println("Loading...");
      Serial.println("fe" + DirecX + ggmode);
      shokaipointer2(0,DirecX + ggmode);
      imano_page = 0;
      
      
      Serial.println("sus" + String(maxpage));
      return;
          }else{
            maxpage = -1;
            M5.Lcd.println("  Create Dir\n  Delete File\n  Rename\n  Make File\n  CopyFileorPDir\n  Paste Them\n  RenamePDir\n  FileInfo/Edit\n  Back Home" );
        return;
          }
        }else{
          maxpage = -1;
          M5.Lcd.println("  Create Dir\n  Delete File\n  Rename\n  Make File\n  CopyFileorPDir\n  Paste Them\n  RenamePDir\n  FileInfo/Edit\n  Back Home" );
        return;
        }
}
bool appendUtf8(String &str, uint32_t cp) {
    char buffer[5]; // 最大4バイト + ヌル終端
    int len = 0;

    if (cp <= 0x7F) {
        buffer[len++] = (char)cp;
    } else if (cp <= 0x7FF) {
        buffer[len++] = (char)(0xC0 | (cp >> 6));
        buffer[len++] = (char)(0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        buffer[len++] = (char)(0xE0 | (cp >> 12));
        buffer[len++] = (char)(0x80 | ((cp >> 6) & 0x3F));
        buffer[len++] = (char)(0x80 | (cp & 0x3F));
    } else if (cp <= 0x10FFFF) {
        buffer[len++] = (char)(0xF0 | (cp >> 18));
        buffer[len++] = (char)(0x80 | ((cp >> 12) & 0x3F));
        buffer[len++] = (char)(0x80 | ((cp >> 6) & 0x3F));
        buffer[len++] = (char)(0x80 | (cp & 0x3F));
    } else {
        // 不正なコードポイント (Unicodeの範囲外) -> 置換文字 (U+FFFD)
        buffer[len++] = (char)0xEF;
        buffer[len++] = (char)0xBF;
        buffer[len++] = (char)0xBD;
    }
    
    buffer[len] = '\0';
    return str.concat(buffer, len);
}

/**
 * @brief UTF-8のStringから1文字（コードポイント）を読み進めます。
 * @param str [in] 対象のStringオブジェクト
 * @param index [in/out] 読み取り開始位置。読み取り後、次の文字の開始位置に更新されます。
 * @return uint32_t Unicodeコードポイント。終端の場合は 0、不正なシーケンスの場合は 0xFFFD。
 */
uint32_t getNextUtf8CodePoint(const String &str, size_t &index) {
    size_t len = str.length();
    if (index >= len) {
        return 0; // 終端
    }

    uint8_t b1 = (uint8_t)str.charAt(index++);
    
    // 1-byte (ASCII)
    if (b1 < 0x80) {
        return b1;
    }

    // 不正なシーケンス（2バイト目以降が単独で出現）
    if ((b1 & 0xC0) == 0x80) {
        return 0xFFFD; 
    }

    // 2-byte
    if ((b1 & 0xE0) == 0xC0) {
        if (index >= len) return 0xFFFD; // 途中で終端
        uint8_t b2 = (uint8_t)str.charAt(index++);
        if ((b2 & 0xC0) != 0x80) return 0xFFFD;
        return ((uint32_t)(b1 & 0x1F) << 6) | (b2 & 0x3F);
    }

    // 3-byte
    if ((b1 & 0xF0) == 0xE0) {
        if (index + 1 >= len) return 0xFFFD; // 途中で終端
        uint8_t b2 = (uint8_t)str.charAt(index++);
        uint8_t b3 = (uint8_t)str.charAt(index++);
        if ((b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) return 0xFFFD;
        return ((uint32_t)(b1 & 0x0F) << 12) | ((uint32_t)(b2 & 0x3F) << 6) | (b3 & 0x3F);
    }

    // 4-byte
    if ((b1 & 0xF8) == 0xF0) {
        if (index + 2 >= len) return 0xFFFD; // 途中で終端
        uint8_t b2 = (uint8_t)str.charAt(index++);
        uint8_t b3 = (uint8_t)str.charAt(index++);
        uint8_t b4 = (uint8_t)str.charAt(index++);
        if ((b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80 || (b4 & 0xC0) != 0x80) return 0xFFFD;
        return ((uint32_t)(b1 & 0x07) << 18) | ((uint32_t)(b2 & 0x3F) << 12) | ((uint32_t)(b3 & 0x3F) << 6) | (b4 & 0x3F);
    }

    return 0xFFFD; // 不正な開始バイト (C1, F5-FF)
}


/**
 * @brief SDカードの指定されたファイルを読み込み、UTF-8のStringに変換して格納します。
 * * ファイルの先頭でBOMを検出し、文字コードを判別します。
 * * UTF-16/UTF-32の場合はUTF-8に変換しながら読み込みます。
 * * 読み込みはバッファリングされ、Stringのメモリ限界（メモリ不足）に達すると停止します。
 *
 * @param filePath [in] 読み込むファイルのフルパス (例: "/input.txt")
 * @param SSText [out] 読み込んだ内容を格納するUTF-8 String (参照渡し)。
 * この関数を呼ぶと、既存の内容はクリアされます。
 * @param mozikode [out] 検出した文字コード (参照渡し)
 * 0: BOMなし (不明/ANSI/UTF-8)
 * 1: UTF-8 (BOMあり)
 * 2: UTF-16 BE (BOMあり)
 * 3: UTF-16 LE (BOMあり)
 * 4: ファイルが空
 * 5: UTF-32 BE (BOMあり)
 * 6: UTF-32 LE (BOMあり)
 * @return bool 処理に成功した場合は true。SDカードが利用不可、ファイルが開けない場合は false。
 * (メモリ限界で読み込みが途中で停止した場合でも true を返します)
 */

bool readSdFileToStringForced(const String& filePath, String &SSText, int &mozikode) {

    // 1. SDカードのチェック
    if (SD.cardType() == CARD_NONE) {
        Serial.println("SD Card not found or not initialized.");
        return false;
    }

    // 2. ファイルを開く
    File file = SD.open(filePath, FILE_READ);
    if (!file) {
        Serial.printf("Failed to open file for reading: %s\n", filePath.c_str());
        return false;
    }

    // 3. Stringをクリアし、文字コードを初期化
    SSText = ""; // 既存の内容をクリア
    mozikode = 0; // デフォルトはBOMなし

    // 4. ファイルサイズチェック
    if (file.size() == 0) {
        mozikode = 4; // ファイルが空
        file.close();
        return true;
    }

    // 5. BOM(Byte Order Mark)の検知
    byte bom[4]; // UTF-32検知のために4バイト読み込む
    int readBOM = file.readBytes((char*)bom, 4);

    if (readBOM >= 4 && bom[0] == 0x00 && bom[1] == 0x00 && bom[2] == 0xFE && bom[3] == 0xFF) {
        mozikode = 5; // UTF-32 BE (BOMスキップ済み)
    } else if (readBOM >= 4 && bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0x00 && bom[3] == 0x00) {
        mozikode = 6; // UTF-32 LE (BOMスキップ済み)
    } else if (readBOM >= 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
        mozikode = 1; // UTF-8 (BOMスキップ済み)
        file.seek(3); // 3バイト消費したのでシーク位置を戻す
    } else if (readBOM >= 2 && bom[0] == 0xFE && bom[1] == 0xFF) {
        mozikode = 2; // UTF-16 BE
        file.seek(2); // 2バイトだけBOMだったので、シーク位置を2に戻す
    } else if (readBOM >= 2 && bom[0] == 0xFF && bom[1] == 0xFE) {
        mozikode = 3; // UTF-16 LE
        file.seek(2); // 2バイトだけBOMだったので、シーク位置を2に戻す
    } else {
        mozikode = 0; // BOMなし
        file.seek(0); // ファイルの先頭に戻る
    }

    // 6. 分割読み込み (文字コード変換)
    const int bufferSize = 128; 
    char buffer[bufferSize];
    bool memoryError = false;

    switch (mozikode) {
        case 0: // BOMなし (UTF-8 or ANSIとして読み込む)
        case 1: // UTF-8 (BOMあり)
        {
            // 高速なバイト単位の読み込み (変換不要)
            while (file.available()) {
                int bytesRead = file.read((uint8_t*)buffer, bufferSize);
                if (bytesRead > 0) {
                    if (!SSText.concat(buffer, bytesRead)) {
                        Serial.println("String memory limit reached.");
                        memoryError = true;
                        break; 
                    }
                } else {
                    break;
                }
            }
            break;
        }

        case 2: // UTF-16 BE
        case 3: // UTF-16 LE
        {
            uint16_t highSurrogate = 0;
            byte w_buffer[2];
            while (file.read(w_buffer, 2) == 2) {
                uint16_t w_char;
                if (mozikode == 2) { // BE
                    w_char = (w_buffer[0] << 8) | w_buffer[1];
                } else { // LE
                    w_char = (w_buffer[1] << 8) | w_buffer[0];
                }

                if (w_char >= 0xD800 && w_char <= 0xDBFF) {
                    // サロゲートペア（上位）
                    highSurrogate = w_char;
                    continue;
                }

                uint32_t codePoint;
                if (w_char >= 0xDC00 && w_char <= 0xDFFF) {
                    // サロゲートペア（下位）
                    if (highSurrogate != 0) {
                        // 結合
                        codePoint = 0x10000 + (((highSurrogate & 0x3FF) << 10) | (w_char & 0x3FF));
                        highSurrogate = 0;
                    } else {
                        // 上位がない下位サロゲート（不正）
                        codePoint = 0xFFFD; // 置換文字
                    }
                } else {
                    // サロゲートペアではない
                    if (highSurrogate != 0) {
                        // 下位が来なかった（不正）
                        if (!appendUtf8(SSText, 0xFFFD)) { // 置換文字を追加
                             Serial.println("String memory limit reached.");
                             memoryError = true;
                             break;
                        }
                    }
                    codePoint = w_char;
                    highSurrogate = 0;
                }

                if (memoryError) break;

                if (!appendUtf8(SSText, codePoint)) {
                     Serial.println("String memory limit reached.");
                     memoryError = true;
                     break;
                }
            }
            break;
        }

        case 5: // UTF-32 BE
        case 6: // UTF-32 LE
        {
            byte d_buffer[4];
            while (file.read(d_buffer, 4) == 4) {
                uint32_t codePoint;
                if (mozikode == 5) { // BE
                    codePoint = ((uint32_t)d_buffer[0] << 24) | ((uint32_t)d_buffer[1] << 16) | ((uint32_t)d_buffer[2] << 8) | d_buffer[3];
                } else { // LE
                    codePoint = ((uint32_t)d_buffer[3] << 24) | ((uint32_t)d_buffer[2] << 16) | ((uint32_t)d_buffer[1] << 8) | d_buffer[0];
                }
                
                if (!appendUtf8(SSText, codePoint)) {
                     Serial.println("String memory limit reached.");
                     break;
                }
            }
            break;
        }
    } // end switch(mozikode)

    // 7. ファイルを閉じる
    file.close();
    return true;
}

/**
 * @brief UTF-8 Stringの内容を、指定された文字コード(BOM)に変換してSDカードのファイルに上書き保存します。
 *
 * @param filePath [in] 書き込むファイルのフルパス (例: "/output.txt")
 * @param SSText [in] 書き込む内容のUTF-8 Stringオブジェクト。
 * @param mozikode [in] 書き込む文字コード (BOMの書き込みに使用)
 * 0: BOMなし, 1: UTF-8, 2: UTF-16 BE, 3: UTF-16 LE, 5: UTF-32 BE, 6: UTF-32 LE
 * 4 (空ファイル) の場合はBOMなしとして扱われます。
 * @return bool 処理に成功した場合は true。SDカードが利用不可、ファイルが開けない場合は false。
 */
bool writeStringToFileForced(const String& filePath, const String &SSText, int mozikode) {

    // 1. ファイルを開く
    File file = SD.open(filePath, FILE_WRITE);
    if (!file) {
        Serial.printf("Failed to open file for writing: %s\n", filePath.c_str());
        return false;
    }

    // 2. mozikodeに基づいてBOMを書き込む
    size_t bomWritten = 0;
    size_t expectedBomSize = 0;

    switch (mozikode) {
        case 1: // UTF-8
            {
                const byte bom[] = {0xEF, 0xBB, 0xBF};
                expectedBomSize = sizeof(bom);
                bomWritten = file.write(bom, expectedBomSize);
            }
            break;
        case 2: // UTF-16 BE
            {
                const byte bom[] = {0xFE, 0xFF};
                expectedBomSize = sizeof(bom);
                bomWritten = file.write(bom, expectedBomSize);
            }
            break;
        case 3: // UTF-16 LE
            {
                const byte bom[] = {0xFF, 0xFE};
                expectedBomSize = sizeof(bom);
                bomWritten = file.write(bom, expectedBomSize);
            }
            break;
        case 5: // UTF-32 BE
            {
                const byte bom[] = {0x00, 0x00, 0xFE, 0xFF};
                expectedBomSize = sizeof(bom);
                bomWritten = file.write(bom, expectedBomSize);
            }
            break;
        case 6: // UTF-32 LE
            {
                const byte bom[] = {0xFF, 0xFE, 0x00, 0x00};
                expectedBomSize = sizeof(bom);
                bomWritten = file.write(bom, expectedBomSize);
            }
            break;
        case 0: // BOMなし
        case 4: // (読み込み時)空ファイルだった場合
        default:
            bomWritten = 0;
            expectedBomSize = 0;
            break;
    }

    if (expectedBomSize > 0 && bomWritten != expectedBomSize) {
        Serial.println("Error: Failed to write BOM.");
        file.close();
        return false;
    }

    // 3. Stringの内容を変換しながら書き込む
    size_t dataWritten = 0;
    size_t expectedDataSize = 0;

    switch (mozikode) {
        case 0: // BOMなし
        case 1: // UTF-8
        case 4: // (読み込み時)空ファイル
        default:
            // String (UTF-8) の内容をそのまま書き込む
            if (SSText.length() > 0) {
                expectedDataSize = SSText.length();
                dataWritten = file.write((const uint8_t*)SSText.c_str(), SSText.length());
            }
            break;

        case 2: // UTF-16 BE
        case 3: // UTF-16 LE
        {
            size_t index = 0;
            while (index < SSText.length()) {
                uint32_t cp = getNextUtf8CodePoint(SSText, index);
                if (cp == 0) break; // 終端

                byte w_buffer[4]; // サロゲートペア用に最大4バイト
                size_t bytesToWrite = 0;

                if (cp <= 0xFFFF) {
                    // BMP (サロゲートペア不要)
                    bytesToWrite = 2;
                    if (mozikode == 2) { // BE
                        w_buffer[0] = (byte)(cp >> 8);
                        w_buffer[1] = (byte)(cp & 0xFF);
                    } else { // LE
                        w_buffer[0] = (byte)(cp & 0xFF);
                        w_buffer[1] = (byte)(cp >> 8);
                    }
                } else {
                    // サロゲートペア
                    bytesToWrite = 4;
                    uint32_t cp_s = cp - 0x10000;
                    uint16_t high = 0xD800 | ((cp_s >> 10) & 0x3FF);
                    uint16_t low = 0xDC00 | (cp_s & 0x3FF);
                    
                    if (mozikode == 2) { // BE
                        w_buffer[0] = (byte)(high >> 8);
                        w_buffer[1] = (byte)(high & 0xFF);
                        w_buffer[2] = (byte)(low >> 8);
                        w_buffer[3] = (byte)(low & 0xFF);
                    } else { // LE
                        w_buffer[0] = (byte)(high & 0xFF);
                        w_buffer[1] = (byte)(high >> 8);
                        w_buffer[2] = (byte)(low & 0xFF);
                        w_buffer[3] = (byte)(low >> 8);
                    }
                }
                
                size_t w = file.write(w_buffer, bytesToWrite);
                dataWritten += w;
                expectedDataSize += bytesToWrite;
                if (w != bytesToWrite) break; // 書き込みエラー
            }
            break;
        }

        case 5: // UTF-32 BE
        case 6: // UTF-32 LE
        {
            size_t index = 0;
            while (index < SSText.length()) {
                uint32_t cp = getNextUtf8CodePoint(SSText, index);
                if (cp == 0) break; // 終端

                byte d_buffer[4];
                expectedDataSize += 4;

                if (mozikode == 5) { // BE
                    d_buffer[0] = (byte)(cp >> 24);
                    d_buffer[1] = (byte)(cp >> 16);
                    d_buffer[2] = (byte)(cp >> 8);
                    d_buffer[3] = (byte)(cp & 0xFF);
                } else { // LE
                    d_buffer[0] = (byte)(cp & 0xFF);
                    d_buffer[1] = (byte)(cp >> 8);
                    d_buffer[2] = (byte)(cp >> 16);
                    d_buffer[3] = (byte)(cp >> 24);
                }
                
                size_t w = file.write(d_buffer, 4);
                dataWritten += w;
                if (w != 4) break; // 書き込みエラー
            }
            break;
        }
    } // end switch

    // 4. ファイルを閉じる
    file.close();

    // 5. データ書き込みサイズの確認
    if (dataWritten != expectedDataSize) {
        Serial.println("Error: Failed to write complete String data (Encoding mismatch or disk full?).");
        return false; 
    }
    
    return true;
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



void resetto31(){
  positpoint = 0;
            maxpage = -1;
            holdpositpoint = 0;
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0,0);
            M5.Lcd.println("loading...");
            imano_page = 0;
            mainmode = 31;
            if(!createEE(mmmc,1)){
            kanketu("!load error!",400);
            M5.Lcd.setCursor(0, 0);
            sita = "hello";
            textexx();
            positpoint = 0;
            holdpositpoint = 0;
            imano_page = 0;
            mainmode = 0;
      return;
          }
          M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0,0);
          M5.Lcd.println(TexNet1(mmmc));
          positpointmax = IntNet1;
          return;
}





