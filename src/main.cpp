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
bool isrename = false;
String TTM2;
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
std::vector<String> hensuopt = {"Edit","Delete","Create","Rename","Options","Data Type","R/W ID","DFLTValue","FillNULL","Duplicate","Check Date","Set/DelLink","pasteAlltable","Copy Value","Duplicatetotbl","Put a Linker","Lock Edit","Sort","Exit"};
String TTM;





#pragma region <potlist>





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
    Serial.println("\n--- Input MettDataMap for Save (4-arg) ---");
    // displayLoadedVariables(data); // 非表示
    Serial.println("--- End of Input MettDataMap ---");

    // --- 1. 既存のデータを読み込んでマージ用マップを作成 ---
    std::map<String, MettVariableInfo> mergedInfo; // MettDataMap ではなく MettVariableInfo を保持
    std::vector<String> existingOptionsLines; // ★ HENSU_OPTIONS行を保持するベクター
    
    if (fs.exists(fullFilePath.c_str())) {
        // 1a. データ変数を読み込む
        std::vector<MettVariableInfo> existingVars;
        bool loadSuccess, loadEmpty;
        loadMettFile(fs, fullFilePath, tableName, loadSuccess, loadEmpty, existingVars);
        if (loadSuccess && !loadEmpty) {
            for (const auto& var : existingVars) {
                mergedInfo[var.variableName] = var;
            }
        }

        // 1b. ★ HENSU_OPTIONS行を別途読み込む (ストリーミング)
        File originalFile_options = fs.open(fullFilePath.c_str(), FILE_READ); // 別のハンドルを使用
        if (originalFile_options) {
            bool inTargetBlock = false;
            while (originalFile_options.available()) {
                String line = originalFile_options.readStringUntil('\n');
                String trimmedLine = line;
                trimmedLine.trim();

                if (trimmedLine.startsWith("### METT_TABLE_ID ###")) {
                    inTargetBlock = false;
                } else if (trimmedLine.startsWith("TABLE_NAME:")) {
                    String currentTableName = trimmedLine.substring(String("TABLE_NAME:").length());
                    currentTableName.trim();
                    if (currentTableName == tableName) {
                        inTargetBlock = true;
                    } else {
                        inTargetBlock = false;
                    }
                } else if (inTargetBlock && trimmedLine.startsWith("HENSU_OPTIONS:")) {
                    existingOptionsLines.push_back(line); // ★ オプション行を保持
                }
            }
            originalFile_options.close();
        }
    }

    // --- 2. 新しいデータをマージ ---
    for (const auto& pair : data) {
        // 既存のID(dataType)や名前を保持しつつ、値(valueString)だけを更新
        mergedInfo[pair.first].valueString = pair.second;
        
        // もし変数が新しければ、名前と「空白のID」を設定
        if (mergedInfo[pair.first].variableName.isEmpty()) {
            mergedInfo[pair.first].variableName = pair.first;
            mergedInfo[pair.first].dataType = ""; // ★ IDを空白に設定
        }
    }

    // --- 3. 一時ファイルへのストリーミング書き込み ---
    String tempFilePath = fullFilePath + ".tmp";
    File tempFile = fs.open(tempFilePath.c_str(), FILE_WRITE);
    if (!tempFile) {
        Serial.printf("Error (Save): Failed to open temp file: %s\n", tempFilePath.c_str());
        isError = true;
        return;
    }

    bool tableProcessed = false;
    bool inTargetBlock = false;
    bool foundFirstContent = false; // ★★★ 変更: 最初の意味のある行を見つけたか
    const char* TABLE_ID_PREFIX = "### METT_TABLE_ID ###";
    const char* TABLE_NAME_PREFIX = "TABLE_NAME:";
    const int TABLE_NAME_PREFIX_LEN = 11;

    if (fs.exists(fullFilePath.c_str())) {
        File originalFile = fs.open(fullFilePath.c_str(), FILE_READ);
        if (!originalFile) {
            Serial.printf("Error (Save): Failed to open original file: %s\n", fullFilePath.c_str());
            tempFile.close();
            fs.remove(tempFilePath.c_str());
            isError = true;
            return;
        }

        while (originalFile.available()) {
            String line = originalFile.readStringUntil('\n');
            String trimmedLine = line;
            trimmedLine.trim();

            // ★★★ 変更: 最初の「意味のある行」 (IDまたはTABLE_NAME) を探す ★★★
            if (!foundFirstContent) {
                if (trimmedLine.startsWith(TABLE_ID_PREFIX)) {
                    foundFirstContent = true;
                    // この行は処理する必要があるので、下のロジックに流す
                } else if (trimmedLine.startsWith(TABLE_NAME_PREFIX)) {
                    foundFirstContent = true;
                    // この行も処理する必要があるので、下のロジックに流す
                } else {
                    // IDもTABLE_NAMEでもない行はヘッダーとしてスキップ
                    Serial.printf("Debug (Save): Skipping header text: %s\n", line.c_str());
                    continue; // 次の行へ
                }
            }
            // ★★★ ここまで ★★★

            if (trimmedLine.startsWith(TABLE_ID_PREFIX)) {
                inTargetBlock = false;
                tempFile.println(line); 
            } else if (trimmedLine.startsWith(TABLE_NAME_PREFIX)) {
                String currentTableName = trimmedLine.substring(TABLE_NAME_PREFIX_LEN);
                currentTableName.trim();
                
                tempFile.println(line); // TABLE_NAME行は常に書き込む

                if (currentTableName == tableName) { // ★ ターゲットテーブル発見
                    inTargetBlock = true;
                    tableProcessed = true;
                    
                    // ★ 1. マージした全データ変数 (VarName:ID:Value) を書き込む
                    Serial.printf("Debug (Save): Writing merged data variables for table '%s'.\n", tableName.c_str());
                    for (const auto& pair : mergedInfo) { 
                        if (pair.first != "table_name") {
                            tempFile.println(pair.second.variableName + ":" + pair.second.dataType + ":" + pair.second.valueString);
                        }
                    }

                    // ★ 2. 保持していた HENSU_OPTIONS: 行を書き込む
                    Serial.printf("Debug (Save): Writing %d preserved HENSU_OPTIONS lines.\n", (int)existingOptionsLines.size());
                    for (const auto& optLine : existingOptionsLines) {
                        tempFile.println(optLine);
                    }
                    
                    // ★ 3. 新規追加された変数の HENSU_OPTIONS: 行も書き込む
                    for (const auto& pair : mergedInfo) {
                        bool optionExists = false;
                        String optionPrefix = "HENSU_OPTIONS:" + pair.first + ":";
                        for(const auto& optLine : existingOptionsLines) {
                            if (optLine.startsWith(optionPrefix)) {
                                optionExists = true;
                                break;
                            }
                        }
                        // ★ 新規変数（オプション行がまだ存在しない）の場合
                        if (!optionExists) {
                             Serial.printf("Debug (Save): Writing new HENSU_OPTIONS line for '%s'.\n", pair.first.c_str());
                             tempFile.println("HENSU_OPTIONS:" + pair.first + ":");
                        }
                    }

                    // tempFile.println(); // ★★★ 修正: 余分な空行を削除 ★★★
                } else {
                    inTargetBlock = false;
                }
            } else {
                // 変数行、HENSU_OPTIONS行、コメント、空行
                if (!inTargetBlock) {
                    // ターゲットブロックの外側
                    tempFile.println(line);
                }
                // ★ ターゲットブロックの内側はスキップ
                // (データ変数とオプション行は上で処理済みのため)
            }
        }
        originalFile.close();
    } // if fs.exists

    // --- ファイル内に既存テーブルがなかった場合 (新規追加) ---
    if (!tableProcessed) {
        Serial.printf("Debug (Save): Appending new table '%s'.\n", tableName.c_str());
        
        // ★ 修正: 新規ファイル(またはヘッダーのみのファイル)の場合、foundFirstContentはfalse
        // その場合でも、ID行から書き始める
        if (!foundFirstContent) {
             tempFile.println(TABLE_ID_PREFIX); // 新規ファイルならID行を書き込む
        } else {
             tempFile.println(); // 既存ファイルの末尾なら改行で始める
             tempFile.println(TABLE_ID_PREFIX);
        }
        
        tempFile.println(String(TABLE_NAME_PREFIX) + tableName);
        for (const auto& pair : mergedInfo) { 
            if (pair.first != "table_name") {
                 tempFile.println(pair.second.variableName + ":" + pair.second.dataType + ":" + pair.second.valueString);
                 // ★ 新規テーブルの場合、HENSU_OPTIONS: も追加する
                 tempFile.println("HENSU_OPTIONS:" + pair.second.variableName + ":");
            }
        }
        // tempFile.println(); // ★★★ 修正: 余分な空行を削除 ★★★
    }

    tempFile.close();

    // --- ファイルを入れ替え ---
    if (fs.remove(fullFilePath.c_str())) {
        if (!fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
            Serial.printf("Error (Save): Failed to rename temp file.\n");
            isError = true;
        }
    } else if (fs.exists(fullFilePath.c_str())) {
        Serial.printf("Error (Save): Failed to remove original file.\n");
        isError = true;
    } else { // 元ファイルがなかった場合
        if (!fs.rename(tempFilePath.c_str(), fullFilePath.c_str())) {
             Serial.printf("Error (Save): Failed to rename temp file for new file.\n");
             isError = true;
        }
    }

    if (!isError) {
        Serial.printf("Info (Save): File saved successfully. Table '%s' was %s.\n", tableName.c_str(), tableProcessed ? "updated" : "added");
    }
}


void loadMettFile(fs::FS &fs, const String& fullFilePath, const String& targetTableName, bool& success, bool& isEmpty, std::vector<MettVariableInfo>& variables) {
    variables.clear();
    success = false;
    isEmpty = true; 

    if (!fs.exists(fullFilePath.c_str())) {
        Serial.printf("Info (Load): File does not exist: %s\n", fullFilePath.c_str());
        return;
    }
    
    // (デバッグ用のファイル内容表示は省略)

    // --- 解析用にファイルを再度開く ---
    File file = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!file) {
        Serial.printf("Error (Load): Failed to open file for parsing: %s\n", fullFilePath.c_str());
        return;
    }

    if (file.size() == 0) {
        file.close();
        return;
    }
    
    String currentTableNameInFile = "";
    bool shouldLoadCurrentTable = false; 
    bool inTableContext = false; 
    bool foundFirstContent = false; // ★★★ 最初の意味のある行を見つけたか

    while (file.available()) {
        String line = file.readStringUntil('\n');
        String originalLine = line; 
        line.trim();

        // ★★★ 変更: 最初の「意味のある行」 (IDまたはTABLE_NAME) を探す ★★★
        if (!foundFirstContent) {
            if (line.startsWith("### METT_TABLE_ID ###")) {
                Serial.println("Debug (Load): Found first TABLE_ID. Starting parse.");
                foundFirstContent = true;
                inTableContext = false; 
                currentTableNameInFile = "";
                shouldLoadCurrentTable = false;
                // ★ 修正: ID行自体は処理不要だが、次の行の解析に進むため 'continue' しない
                // continue; 
            }
            else if (line.startsWith("TABLE_NAME:")) {
                Serial.println("Debug (Load): Found first TABLE_NAME. Starting parse.");
                foundFirstContent = true;
                // ★ この TABLE_NAME 行は処理する必要があるので, 'continue' しない
            } else {
                // IDもTABLE_NAMEでもない行はヘッダーとしてスキップ
                continue; 
            }
        }
        // ★★★ ここまで ★★★


        if (line.startsWith("#") || line.isEmpty()) {
            continue;
        }

        if (line.startsWith("### METT_TABLE_ID ###")) {
            Serial.println("Debug (Load): Found TABLE_ID. Resetting current table context.");
            currentTableNameInFile = "";
            shouldLoadCurrentTable = false; 
            inTableContext = false; 
            continue;
        }
        
        if (line.startsWith("TABLE_NAME:")) {
            inTableContext = true; 
            int colonIndex = line.indexOf(':');
            currentTableNameInFile = (colonIndex != -1) ? line.substring(colonIndex + 1) : "";
            currentTableNameInFile.trim();
            shouldLoadCurrentTable = (targetTableName == currentTableNameInFile);
            if (shouldLoadCurrentTable) {
                Serial.printf("Debug (Load): Found target table '%s'. Loading variables.\n", currentTableNameInFile.c_str());
            } else {
                Serial.printf("Debug (Load): Found non-target table '%s'. Skipping variables.\n", currentTableNameInFile.c_str());
            }
            continue;
        }

        // ★★★ 変更: HENSU_OPTIONS: で始まる行はデータ変数ではないので無視する ★★★
        if (line.startsWith("HENSU_OPTIONS:")) {
            // "HENSU_OPTIONS::" や "HENSU_OPTIONS:" もここで安全にスキップされる
            continue;
        }

        if (inTableContext && shouldLoadCurrentTable) {
            int firstColon = line.indexOf(':');
            int secondColon = line.indexOf(':', firstColon + 1);
            if (firstColon > 0 && secondColon > firstColon) {
                MettVariableInfo varInfo;
                varInfo.variableName = "";
                varInfo.dataType = "";
                varInfo.valueString = "";
                varInfo.tableName = "";
                varInfo.Options = "";
                varInfo.Test = "";
                varInfo.id1 = 0;
                varInfo.id2 = 0;
                varInfo.id3 = 0;
                varInfo.id4 = 0;

                varInfo.variableName = line.substring(0, firstColon);
                varInfo.dataType = line.substring(firstColon + 1, secondColon); // ID (String)
                varInfo.valueString = line.substring(secondColon + 1);
                varInfo.tableName = currentTableNameInFile;
                varInfo.id1 = varInfo.dataType.toInt(); // ID (int)
                variables.push_back(varInfo);
            } else {
                Serial.printf("Warning (Load): Skipping malformed variable line in target table: %s\n", originalLine.c_str());
            }
        }
    }

    // ★★★ 変更: 最終チェック ★★★
    if (!foundFirstContent) {
        Serial.printf("Warning (Load): File %s does not contain any '### METT_TABLE_ID ###' or 'TABLE_NAME:'. No data loaded.\n", fullFilePath.c_str());
        variables.clear();
        success = true; // ファイル読み込み自体は成功
        isEmpty = true; // データは0件
        file.close();
        return;
    }

    file.close();
    success = true;
    isEmpty = variables.empty();
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

void shokaipointer3(){
  bool tt = false;
  allTableNames = allTableNames2;
  for(int i = 0; i < 100; i++){
      AllName[i] = "";
    }
    int ii = 0;
    M5.Lcd.fillScreen(BLACK);
    int totalItems = allTableNames.size();
    int totalPages = (totalItems + itemsPerPage - 1) / itemsPerPage;
    int start = imano_page * itemsPerPage;
    int end = start + holdpositpointmax;
    for (int i = start; i < end; ++i) {
   //   Serial.println("ALLT*" + allTableNames[i]);
        
        AllName[ii] = allTableNames[i];
        Serial.println("FFN:" + ii + AllName[ii]);
        ii++;
    }
  fefe = AllName[holdpositpoint];
  Serial.println("the name is" + fefe);
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
    Serial.println("Tables found: " + String(allTableNames.size()));
    allTableNames2 = allTableNames;
    if (allTableNames.empty()) {
        M5.Lcd.fillScreen(BLACK);
        Serial.println("No tables found.");
        maxLinesPerPage = -1;
      M5.Lcd.setCursor(0, 0);
      holdpositpointt = 1;
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
   // Serial.printf("Debug: positpointmax = %d\n", positpointmax); // Debugging line
    // Use positpointmax for the loop
    int start = pageNum * itemsPerPage;
    
    int end = start + positpointmax  ;
    
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
        Serial.println("    nams:" + AllName[ii]);
        ii++;
    }
    fefe = AllName[holdpositpoints];
    Serial.println("posits;" + String(holdpositpoints) + "jj" + fefe);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, M5.Lcd.height() - 20);
    M5.Lcd.printf("Page: %d/%d", pageNum + 1, totalPages);
    }
    

void shokaipointer4(int pagenum = 0){
  bool tt = false;

  int itemsPerP = 25;
        
        int ahc = 0;
        std::vector<String> ids;
  ExtractTablePageMett(SD,DirecX + ggmode,TTM,pagenum,itemsPerP,allhensuname,allhensuvalue,ids,false,tt,ahc);
  if(tt){
        M5.Lcd.fillScreen(BLACK);
        Serial.println("Load Error.");
        maxLinesPerPage = -1;
        kanketu("Load Error!",500);
        mainmode = 13;
        imano_page = 0;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;
      return;
  } 

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
     M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
  if (allhensuname.empty()) {
        M5.Lcd.fillScreen(BLACK);
        Serial.println("No tables found.");
        maxLinesPerPage = -1;
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextFont(3);
      M5.Lcd.println("No Hensus!\n Press B");
        return;
    }

    int totalPages2 = 0;
    int remainingItems = ahc % itemsPerP;
    if(remainingItems == 0){
      totalPages2 = ahc / itemsPerP;
    }else { 
      totalPages2 = (ahc / itemsPerP) + 1;
    }
    positpointmax = allhensuname.size();
  


      
  Serial.println("GG:" + String(pagenum) + "AHC:" + String(ahc) + "IPP:" + String(positpointmax));
   // Serial.printf("Debug: positpointmax = %d\n", positpointmax); // Debugging line
    // Use positpointmax for the loop
    int start = 0;
    
    int end = positpointmax  ;
    
    if(ahc % itemsPerP == 0){
      maxLinesPerPage = ahc / itemsPerP;
    }else{
      maxLinesPerPage = (ahc / itemsPerP) + 1;
    }
    M5.Lcd.setTextSize(1);
    String gga = "";
    for (int i = start; i < end; ++i) {
        if(GyakuhenkanTxt(allhensuvalue[i].c_str()) != ""){
          gga = gga + "  "  + allhensuname[i] + " id:" + ids[i].c_str() + " val:" +  GyakuhenkanTxt(allhensuvalue[i].c_str()) + "\n";
         // M5.Lcd.println("  "  + allhensuname[i] + " id:" + ids[i].c_str() + " val:" +  GyakuhenkanTxt(allhensuvalue[i].c_str()) );
        }else{
          gga = gga + "  "  + allhensuname[i] + " id:" + ids[i].c_str() + " val::EMPTXT" + "\n";
          //M5.Lcd.println("  "  + allhensuname[i] + " id:" + ids[i].c_str() + " val::EMPTY TEXT!" );
        }
        

    }
    showmozinn2(gga);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, M5.Lcd.height() - 20);
    M5.Lcd.printf("Page: %d/%d", pagenum + 1, totalPages2);


}    

void checkemptyhensu(String Hensu,String atai,String Kaerichi = ""){
  bool nullp;
  bool sus;
  std::vector<String> opttt = loadHensuOptions(SD, DirecX + ggmode, TTM,TTM2,nullp,sus);
    if(sus){
    Serial.println("HensuOptions Load Error.");
    return;
  }
  if(std::find(opttt.begin(), opttt.end(), Hensu) == opttt.end()){
    opttt.push_back(Hensu + ":" + atai);
    saveHensuOptions(SD, DirecX + ggmode, TTM,TTM2,opttt,sus);
    if(sus){
      Serial.println("HensuOptions Save Error.");
      return;
    }else{
      Serial.println("HensuOptions Save Succeed.");
      return;
    }
  }else{
    Serial.println("HensuOptions Exists.");
    String yyy = ""; // 結果を格納する変数


if(yyy == ""){
  for (String& line : opttt) {
    
    // 2. Stringの indexOf() メソッドで部分一致をチェックします。
    //    -1 以外が返ってきたら、文字列が見つかったことを意味します。
    if (line.indexOf(Hensu + ":") != -1) {
      
      if(line.substring(line.indexOf(":") + 1) ==""){
        line = Hensu + ":" + atai;
        saveHensuOptions(SD, DirecX + ggmode, TTM,TTM2,opttt,sus);
        if(sus){
          Serial.println("HensuOptions initial Save Error.");
          Kaerichi = atai;
          return;
        }else{
          Serial.println("HensuOptions initial Save Succeed.");
          Kaerichi = atai;
          return;
        }
      }else{
        Kaerichi = line.substring(line.indexOf(":") + 1);
        return;
      }
    }
  }
}

    return;
  }
}


int shokaivector(std::vector<String>& vec, const String& kakikomumozi) {
    // 1. ベクター内を検索 (前方一致)
    for (size_t i = 0; i < vec.size(); ++i) {
        // vec[i] が kakikomumozi から始まっているかチェック
        if (vec[i].startsWith(kakikomumozi)) {
            // 見つかった場合: 既存のインデックスを返す（追加はしない）
            return (int)i;
        }
    }

    // 2. 見つからなかった場合: 末尾に追加する
    vec.push_back(kakikomumozi);
    
    // 追加した要素（末尾）のインデックスを返す
    return (int)vec.size() - 1;
}


void shokaioptionhensu(){
  
  bool nullp;
  bool sus;
  int optionlength = 15;
  
  std::vector<String> opttt = loadHensuOptions(SD, DirecX + ggmode, TTM,TTM2,nullp,sus);
  Serial.println("Opt:" + String(opttt.size()));
  if(sus){
    Serial.println("HensuOptions Load Error.");
  }else if(optionlength > opttt.size()){
    Serial.println("shokaioption");
    shokaivector(opttt,"maxlength;");
    shokaivector(opttt,"datatype;");
    shokaivector(opttt,"defaultvalue;");
    shokaivector(opttt,"isvalueNULL;");
    shokaivector(opttt,"data_lock;");
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
void shokaipointer5(int pagenum = 0,int itemsPerP = 8){
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
    positpoint = 0;
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
//後でファイル名作成時の拡張子オプションロード追加，テーブルの丸ごとコピー機能追加
//変数は変更ロックの登録・解除機能，デフォルト数値，NULL置き換え追加
//ログ機能の追加．ログ追加後，メニューから一発でテーブル編集に飛ぶ機能，つまりファイルのお気に入り指定の追加
//テーブルコピペ機能（名称を変更したテーブルを複数作成する）
bool tututu;
//変数機能：作成，削除，複製，リネーム，変数値設定，変数値のNULL，変数値デフォルト値の設定，変数値タイプ（数値またはリストまたはDate），変数値のリセット，オプション，全テーブルへの列追加，全テーブルから列削除
void loop() {


  if(mainmode == 13){
    tututu = false;
  }
  M5.update(); // ボタン状態を更新
  

if(M5.Touch.getCount() > 1){
  kanketu("Soft Reset Starting...",1000);
  SD.end();
  ESP.restart();
}

 delay(1);//serial.println暴走対策,Allname[positpoint]はテーブル名
 if(mainmode == 21){//変数Opt
    updatePointer2(3,imano_pagek);
      
      if(pagemoveflag == 1){
      pagemoveflag = 0;
      imano_pagek = 0;
      positpoint = 0;
      
      
      return;
    }else if(pagemoveflag == 2){
      pagemoveflag = 0;
      imano_pagek = imano_pagek + 1;
      positpoint = 0;
      
      
      return;
    }else if(pagemoveflag == 5){
      
  
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
      if(!tt){
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
      if(!isValidHensuValue(SuperT,false)){//配列指定してないとき
        Textex = "Invalid Value!";
        return;
      }
      bool ss;
      TTM2 = allhensuname[holdpositpointx3];
      createMettHensu(SD,DirecX + ggmode,TTM,TTM2,SuperT,false,0,ss);
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
      Serial.println("fefe3!");
      positpoint = positpointmaxg - 1;
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
      
      if(positpoint == 0){
        M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0,0);
         
          TTM2 = allhensuname[holdpositpointx3];
          shokaioptionhensu();
          bool tt;
          int id;
          String sus;
          bool yy = loadMettHensu(SD,DirecX + ggmode,TTM,TTM2,sus,tt,id);
          sus = GyakuhenkanTxt(sus);
          if(yy){
            M5.Lcd.setTextSize(1);
            M5.Lcd.println("(Press C to Edit,A to quit) your Value is...");
          showmozinn(sus);
          while(true){
            M5.update();
            delay(1);
            if(M5.BtnC.wasPressed()){
              mainmode = 20;
              M5.Lcd.fillScreen(BLACK);
              SuperT = sus;
              Textex = "Re-Enter New Value(String).";
   
              return;
            }else if(M5.BtnA.wasPressed()){
              M5.Lcd.fillScreen(BLACK);
             // imano_page = holdimanopagex2;
      positpoint = holdpositpointx2;
      shokaipointer4(holdimanopagex2);
      mainmode = 17;
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
          
      }else if(positpoint == 2){//create
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
                M5.Lcd.fillScreen(BLACK);
                M5.Lcd.setCursor(0,0);
                Textex = "Enter Hensu Table Name.";
                SuperT = "";
                return;
              }
            }else if(positpoint ==1){//delete
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
            }else if(positpoint == 3){//Renamwe
              M5.Lcd.fillScreen(BLACK);
              M5.Lcd.setCursor(0,0);
              isrename = true;
              TTM2 = allhensuname[holdpositpointx3];
              SuperT = TTM2;
              Textex = "Rename Hensus...";
              mainmode = 18;
              return;
            }else if(positpoint == 5){//Data_Type
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
                  M5.Lcd.println("nowvalue:" + ssg);
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
            }


    }
 }
 else if(mainmode ==18){
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
      imano_pagek = 0;
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
  
        
        shokaioptionhensu();
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


