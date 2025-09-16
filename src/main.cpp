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


int SCROLL_INTERVAL_FRAMES = 1;
int SCROLL_SPEED_PIXELS = 3;
int frameright;
String RESERVED_NAMES[] = {
    "CON", "PRN", "AUX", "NUL",
    "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
    "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
};
int frameleft;
bool btna;
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
String sitagar[] = {"Net Status","SD Files","Configs","text editor","how to","Options","User Management","Log"};
static bool sd_card_initialized = false; // SDカードが初期化されているか

// --- コピー操作キャンセルフラグ ---
volatile bool cancelCopyOperation = false;

int mainmode = 0;
int maindex = 0;
String maereposit = "";
int sizex = 2;
int resercounter = 0;
int address = 0;
int imano_page = 0;
int goukei_page = 0;

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
String karadirectname;
bool filebrat = false;
bool nosd = false;
String g_copySourcePath; // コピー元パス (ファイルまたはフォルダ)
String g_destinationDirPath; // ペースト先ディレクトリのパス
bool g_isSourceFolder; // コピー元がフォルダであるかどうかのフラグ

String Tex2;
bool serious_errorsd = false;
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
int scrollOffset = 0; // スクロールテキストの描画オフセット
int scrollFrameCounter = 0; // スクロールフレームカウンター

bool firstScrollLoop = false;
int LONG_PRESS_THRESHOLD_FRAMES = 50; // 長押しと判断するまでのフレーム数 (変更: 500 -> 50)
int REPEAT_INTERVAL_FRAMES = 30;      // 連続実行の間隔フレーム数 (変更: 400 -> 30)
int NULL_RESET_THRESHOLD = 3; 
int entryenter = 0; 
bool otroot = false;
String copymotroot;
int positpointmain1;
bool copymotdir;
bool modordir;
static int frameCounter = 0;

struct SdEntryInfo {
  String name;
  bool isDirectory;
};
String SuperT = ""; // 入力されたテキストを保持する文字列
int cursorIndex = 0; // SuperT内の現在のカーソル位置（文字インデックス）
int cursorPixelX = 0; // カーソルのX座標（ピクセル）
int cursorPixelY = 0; // カーソルのY座標（ピクセル）
int offsetX = 0; // テキスト描画の水平オフセット（スクロール用）
int offsetY = 0; // テキスト描画の垂直オフセット（スクロール用）

bool needsRedraw = false;

const int CURSOR_BLINK_INTERVAL = 10; // カーソル点滅のフレーム間隔
const int MAX_STRING_LENGTH = 65535; // SuperTに格納可能な最大文字数


#pragma endregion <hensu>
#pragma region <hensu2>

bool isStart = true;
String potlist[] = {"fileext","stringtype","(dummy)","sort type","wifipasstype","back"}; // グローバルで定義済み
int numMenuItems = sizeof(potlist) / sizeof(potlist[0]); 
int currentPos = 0;
bool redrawRequired = true; // 再描画が必要かどうかのフラグ
int lastValidIndex = 0;     // 最後に有効な項目のインデックス
// 追加するグローバル変数
String JJ = "Test JJ Text"; // JJの初期値
String currentPosDisplayText = ""; // 最下部に表示されるCurrentPosのテキスト
// 再描画最適化のための変数
String lastDrawnJj = ""; 
String lastDrawnCurrentPosText = "";
String optiontxt[6];
String AllName[100];
// 点滅関連のグローバル変数
unsigned long lastBlinkToggleTime = 0;
bool showAngleBrackets = true; // true: <X> を表示, false: X を表示 (<>なし)

// メインモード変数（テスト用に7に初期化）



#pragma endregion
#pragma region <funcdef>



#pragma endregion



#pragma region <directsystem>






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
void listSDRootContents(int pagetax,String Directtory,bool checkfirstMaxLine = false) {
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

struct CursorPosInfo {
    int pixelX;
    int pixelY;
    int lineNum;
    int charInLine;
};

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
struct LineInfo {
    int startIndex; // 行の開始インデックス
    int length;     // 行の文字数（改行文字を除く）
};

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

#pragma endregion


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



//#endregion
bool dexx = false;

//オプション:ファイルソート順番、ファイル拡張子デフォルト、書き込み方式


bool rightrue(){
  return frameright % 50 == 0 || frameright == 2;
}
bool lefttrue(){
  return frameleft % 50 == 0 || frameleft == 2;
}

bool boolmax(){
  Serial.println("maxLinesPerPage3:" + String(maxLinesPerPage3) + "positpoint:" + String(positpoint) + "maxLinesPerPage:" + String(maxLinesPerPage)    );
  Serial.println( "G" + String(imano_page) + "H" + String(maxpage));
  return (maxLinesPerPage3 != 0 && positpoint == maxLinesPerPage3 - 1) || (maxLinesPerPage3 == 0 && positpoint == maxLinesPerPage - 1);
}


// ポインターの位置を更新し、画面下部にテキストをスクロールさせる関数
void updatePointer(bool notext = false) {
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
    if(rightrue() && mainmode == 1 && DirecX == "/" && imano_page == maxpage - 1 && boolmax()){
      
      pagemoveflag = 5;
      btnc = true;
      btna = false;
        return;
    }
    
    if(mainmode == 8){
        Serial.println("r" + String(btna) + "l" + String(btnc) + " " + positpoint + "  " + frameright + frameleft);
    }

    
    if ( rightrue() && !(imano_page == maxpage - 1 && mainmode == 1 && boolmax())) {
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
    Tex2 = "Press B to Options Now Dir C:/" + DirecX + " :total bytes:" + formatBytes(SD.totalBytes()) + " :used bytes:" + formatBytes(SD.usedBytes());
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
        return false;
    }
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
struct MettVariableInfo {
    String variableName;
    String dataType;
    String valueString;
    String tableName;
};

// Struct to hold information about a single metadata file
struct FileMettData {
    String fileName;
    size_t fileSize;
    std::vector<MettVariableInfo> variables;
};

// Map type for saving/loading metadata
// Key: variable name, Value: value string
typedef std::map<String, String> MettDataMap;

// Global variables
const int itemsPerPage = 8; // Number of items to display on a page
std::vector<String> allTableNames;


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

/**
 * @brief Helper function to check if a table name contains invalid characters.
 * @param name The table name to check.
 * @return true if invalid characters are found, otherwise false.
 */
bool containsInvalidTableNameChars(const String& name) {
    for (size_t i = 0; i < name.length(); i++) {
        char c = name.charAt(i);
        if (c == ' ' || c == '#' || c == '$' || c == ':' || c == '&' || c == '-' || c == ',') {
            return true;
        }
    }
    return false;
}

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
    if (!fullFilePath.startsWith("/")) {
        Serial.printf("Error: Cannot save. File path must be an absolute path (e.g., /%s).\n", fullFilePath.c_str());
        isError = true;
        return;
    }
    int dotIndex = fullFilePath.lastIndexOf('.');
    String extension = "";
    if (dotIndex != -1) {
        extension = fullFilePath.substring(dotIndex);
    }
    if (extension != ".mett") {
        Serial.printf("Error: Cannot save. File extension other than '.mett' is not supported: %s\n", fullFilePath.c_str());
        isError = true;
        return;
    }
    if (SD.exists(fullFilePath.c_str())) {
        File checkFile = fs.open(fullFilePath.c_str());
        if (checkFile && checkFile.isDirectory()) {
            Serial.printf("Error: Cannot save. The specified path is a directory: %s\n", fullFilePath.c_str());
            checkFile.close();
            isError = true;
            return;
        }
        if (checkFile) checkFile.close();
    }
    if (containsInvalidTableNameChars(tableName)) {
        Serial.printf("Error: Table name '%s' contains invalid characters (space, #, $, :, &, -, or ,). Skipping save operation.\n", tableName.c_str());
        isError = true;
        return;
    }
    File file = fs.open(fullFilePath.c_str(), FILE_APPEND);
    if (!file) {
        Serial.printf("Error: Failed to open file for writing: %s\n", fullFilePath.c_str());
        isError = true;
        return;
    }
    file.println("--- NEW DATA SET ---");
    file.printf("TABLE_NAME:%s\n", tableName.c_str());
    for(const auto& pair : data) {
        String varName = pair.first;
        if (containsInvalidVariableNameChars(varName)) {
            Serial.printf("Error: Variable name '%s' contains invalid characters (:, &, space, #, or -). Skipping this variable.\n", varName.c_str());
            isError = true;
            continue;
        }
        String valueStr = pair.second;
        String dataType = inferDataType(valueStr);
        file.printf("%s:%s:%s\n", varName.c_str(), dataType.c_str(), valueStr.c_str());
    }
    file.println();
    file.close();
    if (!isError) {
        Serial.printf("Info: File saved: %s (Table: %s)\n", fullFilePath.c_str(), tableName.c_str());
    } else {
        Serial.printf("Warning: File saved with some errors: %s (Table: %s)\n", fullFilePath.c_str(), tableName.c_str());
    }
}

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
void loadMettFile(fs::FS &fs, const String& fullFilePath, const String& targetTableName, bool& success, bool& isEmpty, std::vector<MettVariableInfo>& variables) {
    variables.clear();
    success = false;
    isEmpty = true;
    if (!fullFilePath.startsWith("/")) {
        Serial.printf("Error: Cannot load. File path must be an absolute path (e.g., /%s).\n", fullFilePath.c_str());
        success = false;
        isEmpty = true;
        return;
    }
    int dotIndex = fullFilePath.lastIndexOf('.');
    String extension = "";
    if (dotIndex != -1) {
        extension = fullFilePath.substring(dotIndex);
    }
    if (extension != ".mett") {
        Serial.printf("Error: Cannot load. File extension other than '.mett' is not supported: %s (%s)\n", extension.c_str(), fullFilePath.c_str());
        return;
    }
    File file = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!file) {
        Serial.printf("Error: Failed to open file for reading: %s\n", fullFilePath.c_str());
        return;
    }
    Serial.printf("Info: Loading file: %s (Target Table(s): %s)\n", fullFilePath.c_str(), targetTableName.isEmpty() ? "All" : targetTableName.c_str());
    std::vector<String> targetTableList;
    if (!targetTableName.isEmpty()) {
        String tempTableName = targetTableName;
        int commaIndex = 0;
        while (tempTableName.length() > 0) {
            commaIndex = tempTableName.indexOf(',');
            String singleTarget;
            if (commaIndex == -1) {
                singleTarget = tempTableName;
                tempTableName = "";
            } else {
                singleTarget = tempTableName.substring(0, commaIndex);
                tempTableName = tempTableName.substring(commaIndex + 1);
            }
            singleTarget.trim();
            if (!singleTarget.isEmpty()) {
                targetTableList.push_back(singleTarget);
            }
        }
    }
    String currentTableNameInFile = "";
    bool shouldLoadCurrentTable = (targetTableList.empty());
    while(file.available()){
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.startsWith("--- NEW DATA SET ---")) {
            currentTableNameInFile = "";
            shouldLoadCurrentTable = (targetTableList.empty());
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
                    currentTableNameInFile = rawTableName.substring(firstChar, lastChar + 1);
                } else {
                    currentTableNameInFile = "";
                }
            } else {
                currentTableNameInFile = "";
                Serial.printf("Debug: 'TABLE_NAME:' line without colon: '%s' in file '%s'\n", line.c_str(), fullFilePath.c_str());
            }
            if (containsInvalidTableNameChars(currentTableNameInFile)) {
                Serial.printf("Warning: Table name '%s' in file '%s' contains invalid characters. Treating as empty table name.\n", currentTableNameInFile.c_str(), fullFilePath.c_str());
                currentTableNameInFile = "";
            }
            if (!targetTableList.empty()) {
                shouldLoadCurrentTable = false;
                for (const String& target : targetTableList) {
                    if (currentTableNameInFile == target) {
                        shouldLoadCurrentTable = true;
                        break;
                    }
                }
            } else {
                shouldLoadCurrentTable = true;
            }
            continue;
        }
        if (line.startsWith("#") || line.isEmpty()) {
            continue;
        }
        if (!shouldLoadCurrentTable) {
            continue;
        }
        int firstColonIndex = line.indexOf(':');
        int secondColonIndex = line.indexOf(':', firstColonIndex + 1);
        if (firstColonIndex == -1 || secondColonIndex == -1) {
            Serial.printf("Warning: Invalid mett line format: %s in file %s (Table: %s)\n", line.c_str(), fullFilePath.c_str(), currentTableNameInFile.c_str());
            continue;
        }
        MettVariableInfo varInfo;
        varInfo.variableName = line.substring(0, firstColonIndex);
        varInfo.dataType = line.substring(firstColonIndex + 1, secondColonIndex);
        varInfo.valueString = line.substring(secondColonIndex + 1);
        varInfo.tableName = currentTableNameInFile;
        varInfo.variableName.trim();
        varInfo.dataType.trim();
        varInfo.valueString.trim();
        if (containsInvalidVariableNameChars(varInfo.variableName)) {
            Serial.printf("Warning: Variable name '%s' in file '%s' (Table: %s) contains invalid characters. Skipping this variable.\n", varInfo.variableName.c_str(), fullFilePath.c_str(), currentTableNameInFile.c_str());
            continue;
        }
        variables.push_back(varInfo);
        isEmpty = false;
    }
    file.close();
    success = true;
    Serial.printf("Info: Mett file loaded successfully: %s (isEmpty: %s, Loaded Variables: %d)\n", fullFilePath.c_str(), isEmpty ? "true" : "false", variables.size());
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
String joinVectorToString(const std::vector<String>& vec) {
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
 * @brief Gets all table names saved in a specified .mett file.
 * @param fs The SD card filesystem object.
 * @param fullFilePath The full path of the file to get table names from.
 * @return std::vector<String> A list of extracted unique table names.
 */
std::vector<String> getAllTableNamesInFile(fs::FS &fs, const String& fullFilePath) {
    std::vector<String> tableNames;
    std::set<String> uniqueTableNames;
    if (!fullFilePath.endsWith(".mett") || !fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error: File not found or not a valid .mett file: %s\n", fullFilePath.c_str());
        return tableNames;
    }
    File file = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!file) {
        Serial.printf("Error: Failed to open file for reading: %s\n", fullFilePath.c_str());
        return tableNames;
    }
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.startsWith("TABLE_NAME:")) {
            int colonIndex = line.indexOf(':');
            if (colonIndex != -1) {
                String tableName = line.substring(colonIndex + 1);
                tableName.trim();
                if (!containsInvalidTableNameChars(tableName)) {
                    uniqueTableNames.insert(tableName);
                } else {
                    Serial.printf("Warning: Invalid table name '%s' found in file. Skipping.\n", tableName.c_str());
                }
            }
        }
    }
    file.close();
    Serial.printf("Info: Found %d unique table names in file '%s'.\n", uniqueTableNames.size(), fullFilePath.c_str());
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
    bool fileIsEmpty = false; // 新しく追加されたフラグ
    loadMettFile(SD, "/save/save1.mett", "TestOpt1",loadSuccess, fileIsEmpty, loadedVariables);
    if (loadSuccess) {
        if (fileIsEmpty) {
            Serial.println("Info: ファイルは空です。");
            MettDataMap dataToSave;
            dataToSave["file_ext"] = "txt";
            dataToSave["stringtype"] = "unicode";
            dataToSave["sorttype"] = "nameasc";
            dataToSave["onlinetype"] = "only pass";
            std::vector<int> sensorInts = {1, 2, 3, -132};
            dataToSave["sensorInts_test"] = joinVectorToString(sensorInts);
            Serial.printf("Info: Saving IntArray: %s\n", dataToSave["sensorInts_test"].c_str());
             saveMettFile(SD, "/save/save1.mett", "TestOpt1", dataToSave, loadSuccess); 
           
            if(loadSuccess){
                Serial.println("Info: 初期データを保存しました。");
                  optiontxt[0] = "txt";
                  optiontxt[1] = "unicode";
                  optiontxt[2] = "nameasc";
                  optiontxt[3] = "only pass";
                return true;
            }else{
              return false;
            }
            
        } else {
            printTable("/save/save1.mett", "TestOpt1", loadedVariables);
            Serial.println("Info: プリント済み");
            loadMettFile(SD, "/save/save1.mett", "TestOpt1", loadSuccess, fileIsEmpty, loadedVariables);

            if (loadSuccess) {
                Serial.println("Info: ロード成功");
                String buildNumber = getVariableString(loadedVariables, "file_ext");
                String buildNumber2 = getVariableString(loadedVariables, "stringtype");
                String buildNumber3 = getVariableString(loadedVariables, "sorttype");
                String buildNumber4 = getVariableString(loadedVariables, "onlinetype");
                std::vector<int> sensorInts = getVariableIntArray(loadedVariables, "sensorInts_test");
                //Serial.printf("Info: Loaded file_ext: %s\n", buildNumber.c_str());
                //Serial.printf("Info: Loaded sensorInts_test: %s\n", joinVectorToString(sensorInts).c_str());
                optiontxt[0] = buildNumber;
                optiontxt[1] = buildNumber2;
                optiontxt[2] = buildNumber3;
                optiontxt[3] = buildNumber4;

                return true;
            } else {
                Serial.println("ロードエラー");


                return false;
            }

        }
    } else {
        Serial.println("Error: ファイルのロードに失敗しました。");
        return false;
    }
    return false;
}

void shokaipointer2(int pageNum, String filePath  ) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    frameright = 0;
    frameleft = 0;
    // Get all table names from a single file
    allTableNames = getAllTableNamesInFile(SD, filePath);

    if (allTableNames.empty()) {
        M5.Lcd.println("No tables found.");
        return;
    }

    int totalItems = allTableNames.size();
    int totalPages = (totalItems + itemsPerPage - 1) / itemsPerPage;

    if (pageNum < 0 || pageNum >= totalPages) {
        M5.Lcd.println("Invalid page.");
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
        } else {
            positpointmax = remainingItems ;
        }
    } else {
        positpointmax = itemsPerPage ;
    }
    Serial.printf("Debug: positpointmax = %d\n", positpointmax); // Debugging line
    // Use positpointmax for the loop
    int start = pageNum * itemsPerPage;
    int end = start + positpointmax;

    M5.Lcd.setCursor(0, 0);
    for(int i = 0; i < 100; i++){
      AllName[i] = "";
    }
    int ii = 0;
    for (int i = start; i < end; ++i) {
        M5.Lcd.println("  " + allTableNames[i]);
        AllName[ii] = allTableNames[i];
        ii++;
    }
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, M5.Lcd.height() - 20);
    M5.Lcd.printf("Page: %d/%d", pageNum + 1, totalPages);
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
  SD.begin();
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
  sita = "hello";
  textexx();
 
  wirecheck();
  mainmode = 0;
   

  
  // USB接続/切断コールバックを設定


  

}

void loop() {
  M5.update(); // ボタン状態を更新
 delay(1);//serial.println暴走対策

if(mainmode == 13){

}
 else if(mainmode == 12){
    if(M5.BtnA.wasPressed()){
      M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

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
     
      Serial.println("fe" + DirecX + ggmode);
      shokaipointer2(0,DirecX + ggmode);
      return;
    }
 }
 #pragma region <optmodee>
 
 else if(mainmode == 11){
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
            positpointmax = 3;
            maxpage = 1;
            imano_page = 0;
            M5.Lcd.fillScreen(BLACK);
         
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.println("  .txt\n  .mett\n  .tbl\n  .(yourself)");
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
    if(positpoint == 2 && M5.BtnB.wasPressed()){//delete dir
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
        SuperT=".txt";
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
    if(cc){
      M5.Lcd.setTextSize(sizex);
       M5.Lcd.setTextColor(WHITE, BLACK); // 白文字、黒背景
  
  // 左上すれすれ (0,0) に表示
        M5.Lcd.setCursor(0, 0);
        sita = "hello";
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
        SuperT=".txt";
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
    if(dd){//ファイルコピー
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
      if(DirecX == "/"){
        kanketu("root folder cannot  cannot be copied!",500);
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = 0;
        holdpositpoint = 0;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
      }else{
        copymotroot = DirecX  + Filelist[nowpositZ()];
      copymotdir = true;
      kanketu("copied(folder)",500);
      M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
      }
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
        positpoint = holdpositpoint;
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
        sita = "hello";
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
  else if(M5.BtnB.wasPressed() && positpoint == 5){//ファイルペースト
    if(copymotroot = ""){
      kanketu("copy moto is empty!",500);
      M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextSize(File_goukeifont);
        positpoint = holdpositpoint;
        mainmode = 1;

        // SDカードコンテンツの初期表示
        shokaipointer();
        return;
    }else{
      String imaroot  = DirecX + Filelist[nowposit()];
      Serial.println(imaroot + "]" + copymotroot);

    }
  }
  else if(M5.BtnB.wasPressed() && positpoint == 7){ //edit file
    String gggs = (DirecX + Filelist[positpoint]);
    gggs = gggs.substring(0,gggs.length() - 1);
    Serial.println("fef" + gggs);
     File myFile = SD.open(gggs);
     long fileSize = myFile.size();
     time_t lastWriteTime = myFile.getLastWrite(); // getLastWrite()の代わりにgetWriteTime()の場合もあります
      struct tm *timeinfo;
      timeinfo = localtime(&lastWriteTime);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.printf("File Size: %ld bytes\n", fileSize);
      M5.Lcd.printf("Last Write: %04d/%02d/%02d %02d:%02d:%02d\n",
                    timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
                    timeinfo->tm_mday, timeinfo->tm_hour,
                    timeinfo->tm_min, timeinfo->tm_sec);


      
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
        if(Filelist[nowposit()] == "System Volume Information" || Filelist[nowposit()] == "m5stack.server"){
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setTextSize(1);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.println("This Directory is cannot be edited!");
          delay(1000);
          M5.Lcd.setTextSize(File_goukeifont);

          shokaipointer();
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

          M5.Lcd.setCursor(0, 0);
        sita = "hello";
        textexx();
        positpoint = 0;
        holdpositpoint = 0;
        imano_page = 0;
          

          return;
        }
       
        M5.Lcd.fillScreen(BLACK); // 画面をクリア
        bool tt = initializeSDCard("/save/save2.mett");
        tt = tt * initializeSDCard("/save/save1.mett");
        tt = tt * initializeSDCard("/save/save3.mett");
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






