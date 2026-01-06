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
// true: <X> を表示, false: X を表示 (<>なし)

// メインモード変数（テスト用に7に初期化）



#pragma endregion
#pragma region <funcdef>
bool Tflag = false;
bool isrename = false;
String TTM2;
bool isdup = false;
int holdpositpoints;
int holdpositpointx = 0;
int holdmaxpagex = 0;
int holdimanopagex = 0;
std::vector<String> optt;
int imano_pagek = 0;
int ssoptdok = 0;
std::vector<String> hensuopt2;
int holdpositpointx2 = 0;
int holdimanopagex2 = 0;
int holdpositpointx3 = 0;
int holdimanopagex3 = 0;
String ascopt[] = {"nameasc","namedesc","dateasc","datedesc","crtdnew","crtdold"};
String MMName;
std::vector<ClientSession> SessionList;
#pragma endregion

//配列のNULL代入も作る
/**
 * @brief テーブル名が有効な形式であるか検証します。
 * * @param tableName 検証するテーブル名
 * @return bool 有効な場合はtrue
 */

std::vector<String> allhensuname;
std::vector<String> allhensuvalue;
//テーブル複製時は保持する関数と保持しない関数を個別に設定
std::vector<String> hensuopt = {"Edit","Delete","Create","Rename","Options","Data Type","R/W ID","DFLTValue","FillNULL","Duplicate","Check Date","Set/DelLink","pasteAlltable","Copy Value","Duplicatetotbl","Put a Linker","Addhensu2alltbl","Sort","Exit"};
String TTM;
//変数データのコピペは入力画面時にBtnBを押す




#pragma region <potlist>


void setup() {
  
  TEXT_SCROLL_INTERVAL_MS = 40; 
  auto cfg = M5.config();
  Serial.begin(115200);
  lastTextScrollTime = 0;
  SCROLL_SPEED_PIXELS = 4;
  M5.begin();
  frameleft = 1;
  frameright = 1;
  statustext = "NetStep:0,No Internet!";
  scrollPos = M5.Lcd.width();
  Serial.println("M5Stack initialized");
processingQueue = xQueueCreate(1000, sizeof(TaskMessage));
    
    if (processingQueue == NULL) {
        Serial.println("Failed to create queue");
        return;
    }

    // 2. ワーカータスクの起動
    // Core 0 (WiFi処理とは別のコア) または Core 1 で実行
    xTaskCreatePinnedToCore(
        backgroundProcessingTask, // タスク関数
        "StorageWorker",          // タスク名
        8192,                     // スタックサイズ (SD扱うなら大きめに)
        NULL,                     // パラメータ
        1,                        // 優先度 (1 = 低, ネットワーク処理を邪魔しない)
        NULL,                     // タスクハンドル
        1                         // Core 1 (Arduino loopと同じ) で実行推奨
    );
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

void saveMettFile(fs::FS &fs, const String& fullFilePath, const String& tableName, const MettDataMap& data, bool& isError) {
    isError = false;

    struct TableData {
        String name;
        std::vector<MettVariableInfo> variables;
        std::vector<String> optionsLines;
    };
    std::vector<TableData> allTables;
    bool foundTargetTable = false;

    // --- 2. 既存ファイルの読み込み ---
    if (fs.exists(fullFilePath.c_str())) {
        File file = fs.open(fullFilePath.c_str(), FILE_READ);
        if (file) {
            TableData* currentTable = nullptr;
            
            Serial.printf("Debug (Save): Reading existing file %s...\n", fullFilePath.c_str());

            while (file.available()) {
                String line = file.readStringUntil('\n');
                String trimmed = line;
                trimmed.trim();

                if (trimmed.isEmpty()) continue;

                // 【修正箇所】判定順序を変更。### を先にチェックする。
                if (trimmed.startsWith("### METT_TABLE_ID ###")) {
                    allTables.emplace_back();
                    currentTable = &allTables.back();
                    // Serial.println("Debug (Save): Found new table section.");
                    continue;
                }

                // その他の # で始まる行はコメントとして無視
                if (trimmed.startsWith("#")) continue;

                if (!currentTable) continue; 

                if (trimmed.startsWith("TABLE_NAME:")) {
                    currentTable->name = trimmed.substring(11); 
                    currentTable->name.trim();
                    Serial.printf("Debug (Save): Found table '%s'\n", currentTable->name.c_str());
                    continue;
                }

                if (trimmed.startsWith("HENSU_OPTIONS:")) {
                    currentTable->optionsLines.push_back(line);
                } else {
                    // 変数行の解析
                    int firstColon = trimmed.indexOf(':');
                    if (firstColon > 0) {
                        int secondColon = trimmed.indexOf(':', firstColon + 1);
                        if (secondColon > firstColon) {
                            MettVariableInfo var;
                            var.variableName = trimmed.substring(0, firstColon);
                            var.dataType = trimmed.substring(firstColon + 1, secondColon);
                            var.valueString = trimmed.substring(secondColon + 1);
                            currentTable->variables.push_back(var);
                        }
                    }
                }
            }
            file.close();
        }
    } else {
        Serial.println("Debug (Save): New file creation.");
    }

    // --- 3. メモリ上でのデータマージ ---
    TableData* target = nullptr;
    String targetNameClean = tableName;
    targetNameClean.trim();

    for (auto& t : allTables) {
        if (t.name == targetNameClean) {
            target = &t;
            foundTargetTable = true;
            break;
        }
    }

    if (!target) {
        allTables.emplace_back();
        target = &allTables.back();
        target->name = targetNameClean;
        Serial.printf("Debug (Save): Creating new table '%s' in memory.\n", targetNameClean.c_str());
    } else {
        Serial.printf("Debug (Save): Updating existing table '%s'.\n", targetNameClean.c_str());
    }

    // データ更新・追加
    for (const auto& pair : data) {
        const String& newVarName = pair.first;
        const String& newValue = pair.second;
        bool exists = false;

        for (auto& v : target->variables) {
            if (v.variableName == newVarName) {
                v.valueString = newValue;
                exists = true;
                break;
            }
        }

        if (!exists) {
            MettVariableInfo newVar;
            newVar.variableName = newVarName;
            newVar.dataType = ""; 
            newVar.valueString = newValue;
            target->variables.push_back(newVar);

            // オプション行の重複チェックと追加
            String newOptPrefix = "HENSU_OPTIONS:" + newVarName + ":";
            bool optExists = false;
            for(const auto& opt : target->optionsLines) {
                if(opt.startsWith(newOptPrefix)) { optExists = true; break; }
            }
            if(!optExists) {
                target->optionsLines.push_back("HENSU_OPTIONS:" + newVarName + ":");
            }
        }
    }

    // --- 4. ファイルへの一括書き出し ---
    File file = fs.open(fullFilePath.c_str(), FILE_WRITE);
    if (!file) {
        Serial.printf("Error (Save): Cannot open file for writing: %s\n", fullFilePath.c_str());
        isError = true;
        return;
    }

    Serial.printf("Debug (Save): Writing %d tables to file.\n", allTables.size());

    for (const auto& t : allTables) {
        file.println("### METT_TABLE_ID ###");
        file.println("TABLE_NAME:" + t.name);
        
        for (const auto& v : t.variables) {
            file.printf("%s:%s:%s\n", v.variableName.c_str(), v.dataType.c_str(), v.valueString.c_str());
        }
        
        for (const auto& opt : t.optionsLines) {
            String o = opt; 
            o.trim();
            if (!o.isEmpty()) file.println(o);
        }
        file.println(); // 可読性のためテーブル間に空行を入れる（読み込みロジックは空行無視に対応済み）
    }

    file.close();
    Serial.println("Info (Save): Save completed.");
}

void loadMettFile(fs::FS &fs, const String& fullFilePath, const String& targetTableName, bool& success, bool& isEmpty, std::vector<MettVariableInfo>& variables) {
    variables.clear();
    success = false;
    isEmpty = true; 

    if (!fs.exists(fullFilePath.c_str())) {
        Serial.printf("Info (Load): File does not exist: %s\n", fullFilePath.c_str());
        success = true; // ファイルがない＝空として正常
        return;
    }
    
    File file = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!file) {
        Serial.printf("Error (Load): Failed to open file: %s\n", fullFilePath.c_str());
        return;
    }

    if (file.size() == 0) {
        file.close();
        success = true;
        return;
    }
    
    String currentTableNameInFile = "";
    bool shouldLoadCurrentTable = false; 
    bool inTableContext = false; 
    bool foundFirstContent = false; 

    // 検索対象のテーブル名も空白除去して正規化しておく（重要）
    String targetNameClean = targetTableName;
    targetNameClean.trim();

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim(); // 行の前後の空白を除去（\r も消える）

        if (line.isEmpty() || line.startsWith("#")) {
            // "### METT_TABLE_ID ###" は "#" で始まるためここで弾かれないよう注意が必要だが、
            // 元コードでは "#" チェックが先にあるため、セクション区切り行がコメント扱いされて無視される可能性がある
            // 修正: セクション区切りは特別扱いする
            if (!line.startsWith("### METT_TABLE_ID ###")) {
                 continue;
            }
        }

        // --- セクション開始/切り替え判定 ---
        if (line.startsWith("### METT_TABLE_ID ###")) {
            foundFirstContent = true;
            currentTableNameInFile = "";
            shouldLoadCurrentTable = false; 
            inTableContext = false; 
            continue;
        }
        
        if (line.startsWith("TABLE_NAME:")) {
            foundFirstContent = true;
            inTableContext = true; 
            
            // テーブル名抽出処理
            // "TABLE_NAME:" (11文字) 以降を取得
            if (line.length() > 11) {
                currentTableNameInFile = line.substring(11);
            } else {
                currentTableNameInFile = "";
            }
            currentTableNameInFile.trim(); // ファイル内の名前もトリム

            // 比較（完全一致）
            shouldLoadCurrentTable = (targetNameClean == currentTableNameInFile);
            
            if (shouldLoadCurrentTable) {
                // Serial.printf("Debug (Load): Found target table '%s'\n", currentTableNameInFile.c_str());
            }
            continue;
        }

        // --- 変数読み込みフェーズ ---
        // まだ最初のセクションにも到達していないなら何もしない
        if (!foundFirstContent) continue;

        // 対象テーブルでない、またはオプション行ならスキップ
        if (!inTableContext || !shouldLoadCurrentTable) continue;
        if (line.startsWith("HENSU_OPTIONS:")) continue;

        // 変数解析 (Name:DataType:Value)
        int firstColon = line.indexOf(':');
        
        // 変数名があり、かつ区切り文字が存在する場合
        if (firstColon > 0) {
            int secondColon = line.indexOf(':', firstColon + 1);
            
            if (secondColon > firstColon) {
                MettVariableInfo varInfo;
                varInfo.variableName = line.substring(0, firstColon);
                varInfo.variableName.trim(); // 変数名の余分な空白を除去

                varInfo.dataType = line.substring(firstColon + 1, secondColon);
                // 型情報はトリムしないほうが安全（ID等の場合）、または必要に応じてトリム
                
                varInfo.valueString = line.substring(secondColon + 1);
                // 値のトリムは仕様によるが、line.trim()で行全体がトリムされているため、
                // 末尾の空白は既に消えていることに注意。

                varInfo.tableName = currentTableNameInFile;
                
                // 型変換の安全策
                if (varInfo.dataType.length() > 0) {
                    varInfo.id1 = varInfo.dataType.toInt();
                } else {
                    varInfo.id1 = 0; // 空の場合は0（または適切なデフォルト値）
                }
                
                // 初期化
                varInfo.id2 = 0; varInfo.id3 = 0; varInfo.id4 = 0;
                
                variables.push_back(varInfo);
            }
        }
    }

    file.close();

    // 読み込み結果の判定
    if (foundFirstContent) {
        success = true;
        isEmpty = variables.empty();
    } else {
        // 有効なデータ構造が一つもなかった場合
        success = true; // エラーにはしない
        isEmpty = true;
    }
}

/**
 * @brief (必須ヘルパー) .mettファイル内の特定テーブルからすべての変数名を取得します。
 */
inline bool getVariableNamesInTable(fs::FS &fs, const String& fullFilePath, const String& targetTableName, bool& isZero, std::vector<String>& variableNames) {
    variableNames.clear();
    isZero = true; // デフォルトは0個とする

    std::vector<MettVariableInfo> fullVariables;
    bool loadSuccess, isEmpty;
    loadMettFile(fs, fullFilePath, targetTableName, loadSuccess, isEmpty, fullVariables);

    if (!loadSuccess) {
        Serial.printf("Error (GetVarNames): loadMettFile failed. File might not exist: %s\n", fullFilePath.c_str());
        return false; // ファイル読み込み失敗
    }
    
    if (isEmpty) {
        Serial.printf("Info (GetVarNames): Table '%s' was found but contains 0 variables.\n", targetTableName.c_str());
        // isZero は true のまま
        return true; // 処理自体は成功
    }

    // 変数があった場合
    isZero = false;
    for (const auto& var : fullVariables) {
        variableNames.push_back(var.variableName);
    }
    
    Serial.printf("Info (GetVarNames): Found %d variable names in table '%s'.\n", (int)variableNames.size(), targetTableName.c_str());
    return true; // 処理成功
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




void shokaioptionhensu(){
  
  bool nullp;
  bool sus;
  int optionlength = 16;
  
  std::vector<String> opttt = loadHensuOptions(SD, DirecX + ggmode, TTM,TTM2,nullp,sus);
  Serial.println("Opt:" + String(opttt.size()));
  if(sus){
    Serial.println("HensuOptions Load Error.");
  }else if(optionlength > opttt.size()){
    Serial.println("shokaioption");
    shokaivector(opttt,"maxlength;");
    shokaivector(opttt,"datatype;");
    shokaivector(opttt,"defaultvalue;");
    shokaivector(opttt,"isvalueNULL;False");
    shokaivector(opttt,"data_lock;False");
    shokaivector(opttt,"tagname;");
    shokaivector(opttt,"vectorlength;");
    shokaivector(opttt,"kinshi_moji;");
    shokaivector(opttt,"made_date;");
    shokaivector(opttt,"last_date;");
    shokaivector(opttt,"sort_length;");
    shokaivector(opttt,"dupflag_lock;");
    shokaivector(opttt,"white_list;");
    shokaivector(opttt,"black_list;");
    shokaivector(opttt,"enable_kaigho;");
    shokaivector(opttt,"read_only;False");
    Serial.println("Opt:" + String(opttt.size()));
    saveHensuOptions(SD, DirecX + ggmode, TTM,TTM2,opttt,sus);
    
    if(sus){
      Serial.println("HensuOptions Save Error.");
    }else{
      Serial.println("HensuOptions Save Succeed.");
    }
  }else{
    Serial.println("Already Loaded");
    return;
  }
}
void shokaipointer5(int pagenum ,int itemsPerP ){
  bool tt = false;

  
        hensuopt2 = {};
        int ahc = hensuopt.size();
        int dd = pagenum * itemsPerP;
        int tta = itemsPerP;

        if(dd + itemsPerP > hensuopt.size()){
          tta = hensuopt.size() - dd;
        }
        for(int i = dd; i < dd + tta  ; i++){
          Serial.println("HENSUOPT:" + hensuopt[i]);
          hensuopt2.push_back(hensuopt[i]);
        }
    
    M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.println("Loading...");
    
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    frameright = 0;
   
    M5.Lcd.setTextSize(3);
         M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    if(ahc % itemsPerP == 0){
      maxpage = ahc / itemsPerP;
    }else{
      maxpage = (ahc / itemsPerP) + 1;
    }
    Serial.println("HEPP" + String(imano_pagek) + " " +   String(maxpage));
    if(imano_pagek == maxpage - 1){
      if(ahc % itemsPerP == 0){
        positpointmax = itemsPerP ;
      }else{
        positpointmax = ahc % itemsPerP ;
      }
    }else{
      positpointmax = itemsPerP;
    }
    
  


      
  Serial.println("GG:" + String(pagenum) + "AHC:" + String(ahc) + "IPP:" + String(positpointmax));
   // Serial.printf("Debug: positpointmax = %d\n", positpointmax); // Debugging line
    // Use positpointmax for the loop
    int start = dd;
    
    int end = dd + positpointmax  ;
    
    
    int totalPages2 = maxpage;
    M5.Lcd.setTextSize(3);
    for (int i = start; i < end; ++i) {
      
        M5.Lcd.println("  " + hensuopt[i] );

    }
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, M5.Lcd.height() - 20);
    M5.Lcd.printf("Page: %d/%d ", pagenum + 1, totalPages2);
    //maxpageを決める

}   


void looe(bool retr){
  if(retr ){
            M5.Lcd.fillScreen(BLACK);
      M5.Lcd.println("loading..");
      mainmode = 19;
      imano_page = 0;
      M5.Lcd.setTextFont(3);
      shokaipointer5(imano_page);
      return;
          }else{
            positpoint = holdpositpointx2;
      M5.Lcd.fillScreen(BLACK);
      shokaipointer4(holdimanopagex2);
      mainmode = 17;
            return;
          }
}


SemaphoreHandle_t sessionMutex;

//後でファイル名作成時の拡張子オプションロード追加，テーブルの丸ごとコピー機能追加
//変数は変更ロックの登録・解除機能，デフォルト数値，NULL置き換え追加
//ログ機能の追加．ログ追加後，メニューから一発でテーブル編集に飛ぶ機能，つまりファイルのお気に入り指定の追加
//テーブルコピペ機能（名称を変更したテーブルを複数作成する）
bool tututu;
//変数機能：作成，削除，複製，リネーム，変数値設定，変数値のNULL，変数値デフォルト値の設定，変数値タイプ（数値またはリストまたはDate），変数値のリセット，オプション，全テーブルへの列追加，全テーブルから列削除

// Forward declaration
String getDateTimeString();

void loop() {


 delay(1);



//delay入れたらサーバー起動時にエラー出ます
mainkansu_intmain();
  
   
  

  if(mainmode == 13){
    tututu = false;
  }
  M5.update(); // ボタン状態を更新
  

  if(checkWiFiConnection){
    monitorConnectionLoss();
  }

if(M5.Touch.getCount() > 1){
  kanketu("Soft Reset Starting...",1000);
  SD.end();
  ESP.restart();
}
 //serial.println暴走対策,Allname[positpoint]はテーブル名
if(mainmode == 22){
     updatePointer2(3,imano_pagek);
      
      if(pagemoveflag == 1){
      pagemoveflag = 0;
      
      return;
    
    }else if(pagemoveflag == 5){
      
      positpoint = holdpositpointx2;
      M5.Lcd.fillScreen(BLACK);
      shokaipointer4(holdimanopagex2);
      mainmode = 17;
      return;
    }else if(M5.BtnB.wasPressed()){
      bool retr = false;  
      if(positpoint == 0){
        
        defval(0,retr);
        looe(retr);
          
      }else if(positpoint == 1){
         defval(1,retr);
          looe(retr);
        
      }else if(positpoint == 2){
        defval(2,retr);
         looe(retr);
      }else if(positpoint == 4){
        defval(3,retr);
         looe(retr);
      }else if(positpoint == 3){
        defval(4,retr);
        looe(retr);        
      }else if(positpoint == 5){
        String uui = "datatype;";
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0,0);
        M5.Lcd.println("loading options...");
        bool ge = GetOptDirect("datatype;", uui);
        Serial.println("gege" + uui);
        if(ge && (uui == "String" || uui == "int" || uui == "double")){
          kanketu("This hensu is not kind of list!",400);
          M5.Lcd.fillScreen(BLACK);
      M5.Lcd.println("loading..");
      mainmode = 19;
      imano_page = 0;
      M5.Lcd.setTextFont(3);
      shokaipointer5(imano_page);
      return;
        }else{
          defval(5,retr);
          looe(retr);
        }
      }else if(positpoint == 6){
        defval(6,retr);
        looe(retr); 
      }
}
}
 else if(mainmode == 21){//変数Opt
    updatePointer2(3,imano_pagek);
      
      if(pagemoveflag == 1){
      pagemoveflag = 0;

      positpoint = 0;
      
      
      return;
    }else if(pagemoveflag == 5){
      
      positpoint = holdpositpointx2;
      M5.Lcd.fillScreen(BLACK);
      shokaipointer4(holdimanopagex2);
      mainmode = 17;
      return;
    }else if(M5.BtnB.wasPressed()){
      String keshiki[] = {"String","int","double","date","strlist","intlist","dbllist"};
      String keshikk = keshiki[positpoint];
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,0);
      int gg = 0;
      bool tt = false;
              bool isn = false;
      
      std::vector<String> optta = loadHensuOptions(SD,DirecX + ggmode,TTM,TTM2,isn,tt);
      if(tt){
        kanketu("HensuOptions Load Error!",500);
        positpoint = holdpositpointx2;
      M5.Lcd.fillScreen(BLACK);
      shokaipointer4(holdimanopagex2);
      mainmode = 17;
      return;
      }
      
      String ssg = findLineStartingWithPrefix(optta, "datatype;", gg);
      if(gg != -1){
        Serial.println("Found datatype:" + ssg);
        ssg = ssg.substring(ssg.indexOf(";") + 1);
        optta[gg] = "datatype;" + keshikk;
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0,0);
        M5.Lcd.println("Saving...");
        saveHensuOptions(SD,DirecX + ggmode,TTM,TTM2,optta,tt);
        if(tt){
          kanketu("HensuSaving Error!",500);
          positpoint = holdpositpointx2;
      M5.Lcd.fillScreen(BLACK);
      shokaipointer4(holdimanopagex2);
      mainmode = 17;
      return;
        }else{
          kanketu("HensuOption Saved!",500);
          positpoint = holdpositpointx2;
          
      M5.Lcd.fillScreen(BLACK);
      shokaipointer4(holdimanopagex2);
      mainmode = 17;
      return;
        }

      }else{
        Serial.println("no datatype!");
      }

      
    }
 }
else  if(mainmode == 20){
    textluck();
    if(entryenter == -1){
      SuperT + SuperT + ",";
      entryenter = 0;
    }else if(entryenter == 1){
      entryenter = 0;

      String uui = "!";
        
        bool ge = GetOptDirect("datatype;", uui);
        bool kaigho = false;
      if(ge){
        Serial.println("Datatype:kaigho " + uui + "]");
        if(uui == "strlist" || uui == "intlist" || uui == "dbllist"){
          
          kaigho = true;
        }
        else if(uui == "String" || uui== "int" || uui == "double" || uui == "date"){
          kaigho = false;

        }
        
      }
      ge = GetOptDirect("maxlength;", uui);
      int maxlen = 10000;
      if(ge){
        if(uui == "" || uui == "0"){
          maxlen = 100000;
        }else if(uui.toInt() > 0){
          maxlen = uui.toInt();
        }
        
      }
      Serial.println("fem" + String(kaigho));
      if(!isValidHensuValue(SuperT,kaigho)){//配列指定してないとき
        Textex = "Invalid Value!";
        return;
      }
      
     
      bool ss;
      TTM2 = allhensuname[holdpositpointx3];
      
        if(SuperT.length() > maxlen){
          Textex = "Value Too Long!";
          return;
        }else{
           SuperT = HenkanTxt(SuperT);
          createMettHensu(SD,DirecX + ggmode,TTM,TTM2,SuperT,false,0,ss);
        }
      
      
      if(!ss){
        kanketu("Hensu Edited!",500);
      }else{
        kanketu("Hensu Edit Failed!",500);
      }
      M5.Lcd.fillScreen(BLACK);
              imano_page = holdimanopagex2;
      positpoint = holdpositpointx2;
      shokaipointer4(holdimanopagex2);
      mainmode = 17;
      return;

    }else if(entryenter == 2){
      entryenter = 0;
      positpoint = holdpositpointx3;
      imano_page = holdimanopagex3;
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.println("loading..");
      mainmode = 19;
      imano_page = 0;
      M5.Lcd.setTextFont(3);
      shokaipointer5(imano_page);
      return;
    }
 }
 else if(mainmode == 19){
      updatePointer2(3,imano_pagek);
      
      if(pagemoveflag == 1){
      pagemoveflag = 0;
      imano_pagek = 0;
      positpoint = 0;
      Serial.println("fefe1!");
      shokaipointer5(imano_pagek);
      
      return;
    }else if(pagemoveflag == 2){
      pagemoveflag = 0;
    
        imano_pagek = imano_pagek + 1;
      positpoint = 0;
      Serial.println("fefe2!");
      shokaipointer5(imano_pagek);
      
      
      
      return;
    }else if(pagemoveflag == 3){
      pagemoveflag = 0;
      imano_pagek = imano_pagek - 1;
      Serial.println("fefe3G!");
      positpoint = 7;
      shokaipointer5(imano_pagek);
  
      return;
    }else if (pagemoveflag == 4){
      Serial.println("fefe4!");
           //
      positpoint = holdpositpointx2;
      M5.Lcd.fillScreen(BLACK);
      shokaipointer4(holdimanopagex2);
      mainmode = 17;

      
      
      return;
    }else if(M5.BtnB.wasPressed()){
      
      if(imano_pagek == 0 && positpoint == 0){//Edit
        M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0,0);
         
          TTM2 = allhensuname[holdpositpointx3];
          shokaioptionhensu();
          bool tt;
          int id;
          String sus;
          bool yy = loadMettHensu(SD,DirecX + ggmode,TTM,TTM2,sus,tt,id);
          sus = GyakuhenkanTxt(sus);

        String uui = "!";
        
        bool ge = GetOptDirect("data_lock;", uui);
        Serial.println("editl:" + uui);
        if(ge && uui == "True"){
            ge = true;
        }else{
          ge = false;
        }

          if(yy){
            M5.Lcd.setTextSize(1);
            if(ge){
              M5.Lcd.println("(You Can't Edit,A to quit) your Value is...");
            }else{
              M5.Lcd.println("(Press C to Edit,A to quit) your Value is...");
            }
            
          showmozinn(sus);
          while(true){
            M5.update();
            delay(1);
            if(M5.BtnC.wasPressed() && !ge){
              mainmode = 20;
              M5.Lcd.fillScreen(BLACK);
              SuperT = sus;
              Textex = "Re-Enter New Value(String).";
   
              return;
            }else if(M5.BtnA.wasPressed()){
              M5.Lcd.fillScreen(BLACK);
      M5.Lcd.println("loading..");
      mainmode = 19;
      imano_page = 0;
      M5.Lcd.setTextFont(3);
      shokaipointer5(imano_page);
      return;
             
            }
            
          }

          }else{
            kanketu("Load Error!",500);
            M5.Lcd.fillScreen(BLACK);
             Serial.println("fefe4!" + TTM + ";;" + TTM2);
            //imano_page = holdimanopagex2;
      positpoint = holdpositpointx2;
      M5.Lcd.fillScreen(BLACK);
      shokaipointer4(holdimanopagex2);
      mainmode = 17;
      return;
          }
          
      }else if(imano_pagek == 0 && positpoint == 2){//create
              bool ddd = areusure();
              if(!ddd){
                M5.Lcd.fillScreen(BLACK);
              imano_page = holdimanopagex2;
      positpoint = holdpositpointx3;
      shokaipointer4(holdimanopagex3);
      mainmode = 17;
              }else{
                mainmode = 18;
                isrename = false;
                isdup = false;
                M5.Lcd.fillScreen(BLACK);
                M5.Lcd.setCursor(0,0);
                Textex = "Enter Hensu Table Name.";
                SuperT = "";
                return;
              }
            }else if(imano_pagek == 0 && positpoint ==1){//delete
              String uui = "";
              M5.Lcd.fillScreen(BLACK);
                M5.Lcd.setCursor(0,0);
                M5.Lcd.println("loading...");
               bool ge = GetOptDirect("data_lock;", uui);
        if(ge && uui == "True"){
            ge = true;
        }else{
          ge = false;
        }
            if(ge){
              M5.Lcd.fillScreen(BLACK);
              TTM2 = allhensuname[holdpositpointx3];
              imano_page = holdimanopagex2;
      positpoint = holdpositpointx3;
      shokaipointer4(holdimanopagex3);
      mainmode = 17;
              return;
            }
              bool tt = areubunki("Delete Hensu","Cancel");
              if(!tt){
                M5.Lcd.fillScreen(BLACK);
              imano_page = holdimanopagex2;
      positpoint = holdpositpointx3;
      shokaipointer4(holdimanopagex3);
      mainmode = 17;
              }else{
                 if(DeleteHensuInMettTable(SD,DirecX + ggmode,TTM,TTM2)){
                  kanketu("Hensu Deleted!",500);
                  M5.Lcd.fillScreen(BLACK);
                 }else{
                  kanketu("Hensu Failed!",500);
                  M5.Lcd.fillScreen(BLACK);
                 }
              }
              M5.Lcd.fillScreen(BLACK);
              imano_page = holdimanopagex2;
      positpoint = holdpositpointx3;
      shokaipointer4(holdimanopagex3);
      mainmode = 17;
            }else if(imano_pagek == 0 && positpoint == 3){//Renamwe
              String uui = "";
              TTM2 = allhensuname[holdpositpointx3];
              M5.Lcd.fillScreen(BLACK);
                M5.Lcd.setCursor(0,0);
                M5.Lcd.println("loading...");
                 bool ge = GetOptDirect("data_lock;", uui);
        if(ge && uui == "True"){
            ge = true;
        }else{
          ge = false;
        }
            if(ge){
              M5.Lcd.fillScreen(BLACK);
              imano_page = holdimanopagex2;
      positpoint = holdpositpointx3;
      shokaipointer4(holdimanopagex3);
      mainmode = 17;
              return;
            }




              M5.Lcd.fillScreen(BLACK);
              M5.Lcd.setCursor(0,0);
              isrename = true;
              isdup = false;
              TTM2 = allhensuname[holdpositpointx3];
              SuperT = TTM2;
              Textex = "Rename Hensus...";
              mainmode = 18;
              return;
            }else if(imano_pagek == 0 &&positpoint == 5){//Data_Type
              M5.Lcd.fillScreen(BLACK);
              M5.Lcd.setCursor(0,0);
              M5.Lcd.println("loading\noptions...");
              bool tt = false;
              bool isn = false;
              optt = loadHensuOptions(SD,DirecX + ggmode,TTM,TTM2,isn,tt);
              if(tt){
                M5.Lcd.fillScreen(BLACK);
                imano_page = holdimanopagex2;
                positpoint = holdpositpointx3;
                shokaipointer4(holdimanopagex3);
                mainmode = 17;
              }else{
                TTM2 = allhensuname[holdpositpointx3];
                if(isn){
                  Serial.println("NULLED and SAVED");
                    shokaioptionhensu();
                    optt = loadHensuOptions(SD,DirecX + ggmode,TTM,TTM2,isn,tt);
                if(tt){
                  M5.Lcd.fillScreen(BLACK);
                imano_page = holdimanopagex2;
                positpoint = holdpositpointx3;
                shokaipointer4(holdimanopagex3);
                mainmode = 17;
                }
                }
                M5.Lcd.fillScreen(BLACK);
                String ssg = findLineStartingWithPrefix(optt,"datatype;",ssoptdok);
                if(ssoptdok != -1){
                  String JJ = "";
                  if(ssg == "#EMPMOJI"){
                    JJ = "String";
                  }else{
                    JJ = ssg;
                  }
                  M5.Lcd.fillScreen(BLACK);
                  M5.Lcd.setCursor(0,0);
                  M5.Lcd.println("  String\n  int\n  double\n  date\n  HairetsStr\n  Hairetsint\n  Hairetsdouble");
                  M5.Lcd.println("nowvalue:" + JJ);
                  mainmode = 21;
                  positpointmax = 7;
                  positpoint = 0;
                  maxpage = -1;
                  return;

                }else{
                  Serial.println("optionnousederror");
                  M5.Lcd.fillScreen(BLACK);
                imano_page = holdimanopagex2;
                positpoint = holdpositpointx3;
                shokaipointer4(holdimanopagex3);
                mainmode = 17;
                }
              }
            }else if(imano_pagek == 0 && positpoint == 4){//Options
              M5.Lcd.fillScreen(BLACK);
              M5.Lcd.setCursor(0,0);
              M5.Lcd.println("loading\noptions...");
              bool tt = false;
              bool isn = false;
              optt = loadHensuOptions(SD,DirecX + ggmode,TTM,TTM2,isn,tt);
              if(tt){
                M5.Lcd.fillScreen(BLACK);
                imano_page = holdimanopagex2;
                positpoint = holdpositpointx3;
                shokaipointer4(holdimanopagex3);
                mainmode = 17;
                return;
              }
              if(isn){
                  Serial.println("NULLED and SAVED");
                    shokaioptionhensu();
              }
              M5.Lcd.fillScreen(BLACK);
              M5.Lcd.setCursor(0,0);
              TTM2 = allhensuname[holdpositpointx3];
              M5.Lcd.println("  Max LengthMozi\n  DefaultVal\n  DataLock\n  ReadOnly\n  Tags\n  LengList\n  Enable kaigho");
              positpoint = 0;
              maxpage = -1;
              positpointmax = 7;
              mainmode = 22;
              return;




            }else if(imano_pagek == 0 && positpoint == 7){//Default
              M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0,0);
        String uui = "!";
        
        bool ge = GetOptDirect("defaultvalue;", uui);
        Serial.println("defval:" + uui);
        if(ge && uui != ""){
            ge = true;
        }else{
          ge = false;
        }

        if(!ge){
          kanketu("No default value!",500);
          int ss = areubunki2("Set EmpTxt","back","Set DefaultNULL");
          if(ss == -1){
         
            mainmode = 19;
      positpoint = 0;
      imano_page = 0;
      imano_pagek = 0;
      TTM2 = allhensuname[holdpositpointx3];
      M5.Lcd.setTextFont(3);
      shokaipointer5(0);
      return;
          }else if(ss == 0){
            setoptnul();
            return;

          }else if(ss == 1){
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0,0);
            M5.Lcd.println("Setting...");
            TTM2 = allhensuname[holdpositpointx3];
            SuperT = HenkanTxt("");
          createMettHensu(SD,DirecX + ggmode,TTM,TTM2,SuperT,false,0,ge);
            if(!ge){
              kanketu("seiko!",500);
            }else{
              kanketu("sippai!",500);
            }
             mainmode = 19;
      positpoint = 0;
      imano_page = 0;
      imano_pagek = 0;
      
      M5.Lcd.setTextFont(3);
      shokaipointer5(0);
      return;
          }
        }else{
          int ss = areubunki2("Go back default","back","Set NULL");
          if(ss == 0){
            setoptnul();
            return;
          }else if(ss == -1){
            mainmode = 19;
      positpoint = 0;
      imano_page = 0;
      imano_pagek = 0;
      TTM2 = allhensuname[holdpositpointx3];
      M5.Lcd.setTextFont(3);
      shokaipointer5(0);
      return;
          }else if(ss == 1){
             TTM2 = allhensuname[holdpositpointx3];
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0,0);
            M5.Lcd.println("Setting...");
             bool ge = GetOptDirect("defaultvalue;", uui);
             if(ge){
              SuperT = HenkanTxt(uui);
             }else{
              kanketu("error",500);
              mainmode = 19;
      positpoint = 0;
      imano_page = 0;
      imano_pagek = 0;
     
      M5.Lcd.setTextFont(3);
      shokaipointer5(0);
      return;
             }
          createMettHensu(SD,DirecX + ggmode,TTM,TTM2,SuperT,false,0,ge);
            if(!ge){
              kanketu("seiko!",500);
            }else{
              kanketu("sippai!",500);
            }
            
             mainmode = 19;
      positpoint = 0;
      imano_page = 0;
      imano_pagek = 0;
     
      M5.Lcd.setTextFont(3);
      shokaipointer5(0);
      return;
          }
        }

        
           


            }else if (imano_pagek == 1 && positpoint == 1){//Duplicate
              // Placeholder for future functionality
              bool jj = areusure();
              if(!jj){
                 M5.Lcd.fillScreen(BLACK);
              imano_page = holdimanopagex2;
      positpoint = holdpositpointx3;
      shokaipointer4(holdimanopagex3);
      mainmode = 17;
              }else{
                mainmode = 18;
                isrename = false;
                isdup = true;
                M5.Lcd.fillScreen(BLACK);
                M5.Lcd.setCursor(0,0);
                Textex = "Enter New Hensu Table Name.";
                SuperT = "";
                return;
              }
            }
          }
}else if(mainmode ==18){
  entryenter = 0;
   textluck();
   if(entryenter == 1){
    entryenter = 0;
    bool mm = isValidAndUniqueVariableName(SD,DirecX + ggmode,TTM,SuperT);
    if(!mm){
      Textex = "Invalid Name!";
    }else{
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.println("Creating...");
      bool gg = false;
      if(isrename){
        Serial.println("RNM]" + TTM2 + SuperT + TTM);
        gg = renameHensuInTable(SD,DirecX + ggmode,TTM,TTM2,SuperT);
        if(!gg){
        kanketu("Hensu Created!",500);
      }else{
        kanketu("Hensu Create Failed!",500);
      }
      }else if(isdup){
        String ga = allhensuname[holdpositpointx3];
        duplicateMettHensu(SD,DirecX + ggmode,TTM,ga,SuperT,false,0,gg);
      }else{
        createMettHensu(SD,DirecX + ggmode,TTM,SuperT,"",false,0,gg);
        
      if(!gg){
        kanketu("Hensu Created!",500);
      }else{
        kanketu("Hensu Create Failed!",500);
      }
      }
      
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0,0);
          TTM = AllName[holdpositpoints];
          holdpositpointx = positpoint;
          holdmaxpagex = maxpage;
          holdimanopagex = imano_page;
          positpoint = 0;
          imano_page = 0;
        shokaipointer4();
        maxpage = maxLinesPerPage;
        mainmode = 17;
        return;
    }
   }else if(entryenter == 2){
    entryenter = 0;
      M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0,0);
          TTM = AllName[holdpositpoints];
          holdpositpointx = positpoint;
          holdmaxpagex = maxpage;
          holdimanopagex = imano_page;
          positpoint = 0;
          imano_page = 0;
        shokaipointer4();
        maxpage = maxLinesPerPage;
        mainmode = 17;
        return;
   }  
 }
 else if(mainmode == 17){
  
    if(maxLinesPerPage == -1){
      if(M5.BtnB.wasPressed()){
        
        bool ii = areubunki("Create Hensu","Cancel");
        if(ii){
          mainmode = 18;
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0,0);
          shokaipointer4(imano_page);
          isdup = false;
          isrename = false;
          maxpage = maxLinesPerPage;
      maxpage = maxLinesPerPage;
      SuperT = "";
      Textex = "Enter Hensu Table Name.";
      return;
        }else{
                imano_page = holdimanopagex;
      positpoint = holdpositpointx;
      shokaipointer2(holdimanopagex,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      mainmode = 13;
      M5.Lcd.fillScreen(BLACK);
      
      
      return;
        }
      }
    }else{
          updatePointer2(1);
        holdpositpoints = positpoint;
    if(pagemoveflag == 1){
      pagemoveflag = 0;
      imano_page = 0;
      positpoint = 0;
      Serial.println("fefe1!");
      shokaipointer4(imano_page);
      maxpage = maxLinesPerPage;
      return;
    }else if(pagemoveflag == 2){
      pagemoveflag = 0;
      imano_page = imano_page + 1;
      positpoint = 0;
      Serial.println("fefe2!");
      shokaipointer4(imano_page);
      maxpage = maxLinesPerPage;
      return;
    }else if(pagemoveflag == 3){
      pagemoveflag = 0;
      imano_page = imano_page - 1;
      
      positpoint = positpointmaxg - 1;
      shokaipointer4(imano_page);
      maxpage = maxLinesPerPage;
      return;
    }else if (pagemoveflag == 4){
      
            imano_page = holdimanopagex;
      positpoint = holdpositpointx;
      shokaipointer2(holdimanopagex,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      mainmode = 13;
      
      
      
      return;
    }else if(M5.BtnB.wasPressed()){
      
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.println("loading..");
      mainmode = 19;
      holdpositpointx3 = positpoint;
      holdimanopagex3 = imano_page;
      holdmaxpagex3 = maxpage;
      maxpage = 3;//ページ増えたら変える
      imano_pagek = 0;
      Tflag = true;
      TTM2 = allhensuname[holdpositpointx3];
      M5.Lcd.setTextFont(3);
      shokaipointer5(0);
      
      return;


    }
 }
 }
else if(mainmode == 16){
    updatePointer2(1);
    if(pagemoveflag == 2){
      pagemoveflag = 0;
      return;
    }else if(pagemoveflag == 1){
      pagemoveflag = 0;
      return;
    }else if((pagemoveflag == 5) ){
      pagemoveflag = 0;
      positpoint = 0;
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
        Textex = "Enter Index Numbers.";
        firstScrollLoop = true;
         SuperT = dataToSaveE["table_opt1"];
          opt1_kaimei(0);
          M5.Lcd.fillScreen(BLACK);
          
          if(!optkobun()){
            M5.Lcd.fillScreen(BLACK);
            positpoint = holdpositpointd;
            imano_page = holdimanopaged;
            positpointmax = holdpositpointmaxd;
            mainmode = 1;
            return;
          }
          return;
        }else if(positpoint == 1){
          M5.Lcd.fillScreen(BLACK);
          String opt22[4] = {"normal","readonly","oncewrite"};//oncewriteは空白セルまたはデフォルトに1回しか書き込めない
          int tt = selectOption(opt22,4,"select option!","read options!");
          dataToSaveE["table_opt2"] = opt22[tt];
          M5.Lcd.fillScreen(BLACK);
          bool loadSuccess = false;
           saveMettFile(SD, DirecX + ggmode, fefe, dataToSaveE, loadSuccess);
          if(!optkobun() || loadSuccess){
            M5.Lcd.fillScreen(BLACK);
            positpoint = holdpositpointd;
            imano_page = holdimanopaged;
            positpointmax = holdpositpointmaxd;
            mainmode = 1;
            return;
          }
          return;
        }else if(positpoint == 2){
                    M5.Lcd.fillScreen(BLACK);
          SCROLL_INTERVAL_FRAMES = 1;
        SCROLL_SPEED_PIXELS = 3;
        Textex = "Enter tag name.";
        firstScrollLoop = true;
        SuperT = dataToSaveE["table_opt3"];
          opt1_kaimei(1);
          M5.Lcd.fillScreen(BLACK);
          
          if(!optkobun()){
            M5.Lcd.fillScreen(BLACK);
            positpoint = holdpositpointd;
            imano_page = holdimanopaged;
            positpointmax = holdpositpointmaxd;
            mainmode = 1;
            return;
          }
          return;
        }else if(positpoint == 4){
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0,0);
          M5.Lcd.setTextSize(1);
          showmozinn2("The Date:\n  Createdat:" + dataToSaveE["table_opt4"] + "\n  Lastat:" + dataToSaveE["table_opt5"] + "\n  NowTime:" + getDateTimeString());
          while(true){
            M5.delay(1);
            M5.update();
            if(M5.BtnB.wasPressed()){
              M5.Lcd.fillScreen(BLACK);
              M5.Lcd.setCursor(0,0);
              M5.Lcd.setTextSize(3);
              break;
            }
          }
          
          
            shokaipointer3();
        }else if(positpoint == 3){
          M5.Lcd.fillScreen(BLACK);
          int tt = selectOption(ascopt,6,"select option!","sort options!");
          dataToSaveE["table_opt6"] = ascopt[tt];
          M5.Lcd.fillScreen(BLACK);
          bool loadSuccess = false;
           saveMettFile(SD, DirecX + ggmode, fefe, dataToSaveE, loadSuccess);
          if(!optkobun() || loadSuccess){
            M5.Lcd.fillScreen(BLACK);
            positpoint = holdpositpointd;
            imano_page = holdimanopaged;
            positpointmax = holdpositpointmaxd;
            mainmode = 1;
            return;
          }
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
        if(loadSuccess){
          fefe = SuperT;
          createjj();
          positpoint = 0;
          
        }
        }else if (holdpositpointt == 2){//rename
          renameTableInMettFile(SD, DirecX + ggmode, AllName[holdpositpoint], SuperT, loadSuccess);
          positpoint = 0;
        }
        
        

        if(loadSuccess){
          Textex = "Save/Rename Error!";
        }else{
          kanketu("Create Success!",500);
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0,0);
          M5.Lcd.setTextSize(1);
          M5.Lcd.println("Loading...");
        
        imano_page = holdimanopage;
          mainmode = 13;
          
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
//後でフォルダ位置を保存してお気に入りリスト作れるようにする
//テーブルオプション一覧
//フラッシュファイル空のバックアップ元変更String
//読み込み方式(通常，readonly,appendonly)
//形式(int,String,boolean,float,double,Char,date)
else if(mainmode == 14){
  updatePointer2();
  #pragma region <komakai_table>
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
      mainmode = 13;
      M5.Lcd.fillScreen(BLACK);
      
      imano_page = 0;
     
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
      mainmode = 13;
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
      M5.Lcd.setTextSize(1);
      
      
      Serial.println("point:" + String(holdpositpoint) + "name:" + AllName[holdpositpoint]);
      fefe = AllName[holdpositpoints];
        
      holdimanopage = imano_page;

      bool loadSuccess = false;
    bool fileIsEmpty = false;
    std::vector<MettVariableInfo> loadedVariables;
    M5.Lcd.fillScreen(BLACK);   
    M5.Lcd.println("loading...");
    shokaipointer3();
    loadMettFile(SD, DirecX + ggmode, fefe, loadSuccess, fileIsEmpty, loadedVariables);
       if(loadSuccess){
        
        createjj();
          
        
        if(!optkobun()){
            M5.Lcd.fillScreen(BLACK);
            positpoint = holdpositpointd;
            imano_page = holdimanopaged;
            positpointmax = holdpositpointmaxd;
            mainmode = 1;
            return;
          }
            

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
      }else if(positpoint == 5){//Duplicate
        bool tt = areusure();
        holdpositpointt = positpoint;
        if(tt){
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0,0);
            Textex = "Choose New Name.Tab to Enter";
            SuperT = "";
            opt_hukusei();

        }else{
          positpoint = holdpositpoint;
      imano_page = holdimanopage;
      mainmode = 13;
      M5.Lcd.fillScreen(BLACK);
      positpointmax = 5;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      return;
        }
#pragma endregion
      }else if(positpoint == 0){//open
        M5.Lcd.fillScreen(BLACK);
        fefe = AllName[holdpositpoints];
        
          M5.Lcd.setCursor(0,0);
          TTM = AllName[holdpositpoints];
          holdpositpointx = positpoint;
          holdmaxpagex = maxpage;
          holdimanopagex = imano_page;
          positpoint = 0;
          imano_page = 0;
        
        maxpage = maxLinesPerPage;
        
   
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.println("Checking the table_options...");
    createjj();
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0,0);
      shokaipointer4();
  
        
        
        mainmode = 17;
        return;
      }
      
    }
}
else if(mainmode == 13){
  
  if(maxLinesPerPage != -1){
    
        updatePointer2();
        holdpositpoints = positpoint;
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
      holdpositpointmax = positpointmax;
      holdpositpoint = positpoint;
      holdimanopage = imano_page;
      M5.Lcd.println("  Open\n  Create\n  Rename\n  Delete\n  TableOptions\n  Duplicate\n  Deleterow:name\n  Kensaku Value" );
      positpoint = 0;
      positpointmax = 8;
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
 
 #pragma region <optmodee>//0=拡張子 1=文字コード 2=ソート 3=オンラインタイプ
 


#pragma endregion  

else if(mainmode == -1){


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
        sita = sitagar[maindex];
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
        sita = sitagar[maindex];
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
        sita = sitagar[maindex];
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
       sita = sitagar[maindex];
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

String getDateTimeString() {
    m5::rtc_datetime_t dt;
    M5.Rtc.getDateTime(&dt); // RTCから現在時刻を取得

    // ゼロパディング用のヘルパーラムダ
    auto pad = [](int num) -> String {
        return (num < 10) ? "0" + String(num) : String(num);
    };

    // String 連結で "YYYY-MM-DD_hh:mm:ss" 形式を作成
    String dateTimeString = String(dt.date.year) + "-" +
                            pad(dt.date.month) + "-" +
                            pad(dt.date.date) + "_" +
                            pad(dt.time.hours) + ":" +
                            pad(dt.time.minutes) + ":" +
                            pad(dt.time.seconds);

    return dateTimeString;
}


