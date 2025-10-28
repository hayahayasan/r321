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


int holdpositpoints;
int holdpositpointx = 0;
int holdmaxpagex = 0;
int holdimanopagex = 0;
#pragma endregion

//配列のNULL代入も作る
/**
 * @brief テーブル名が有効な形式であるか検証します。
 * * @param tableName 検証するテーブル名
 * @return bool 有効な場合はtrue
 */

std::vector<String> allhensuname;
std::vector<String> allhensuvalue;
String TTM;





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
inline bool renameTableInMettFile(fs::FS &fs, const String& fullFilePath, const String& oldTableName, const String& newTableName, bool& isError) {
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

void saveMettFile(fs::FS &fs, const String& fullFilePath, const String& tableName, const MettDataMap& data, bool& isError) { // bool* を bool& に変更
    isError = true; // デフォルトでエラー

    // --- 0. パスとテーブル名の検証 ---
    
    if (containsInvalidChars(tableName)) {
        return;
    }
    for (const auto& pair : data) {
        if (containsInvalidChars(pair.first)) {
             return;
        }
        // 値(pair.second)には無効文字チェックは不要
    }


    // --- 1. 既存のデータを読み込んでマージ用マップを作成 ---
    MettDataMap mergedData;
    if (fs.exists(fullFilePath.c_str())) {
        std::vector<MettVariableInfo> existingVariables; // ここで宣言
        bool loadSuccess, isEmpty, loadError; // isErrorをここで宣言
        loadMettFile(fs, fullFilePath, tableName, loadSuccess, isEmpty, existingVariables); // isError と引数順序を変更

        if (!loadSuccess) {
             Serial.printf("Error (Save): 既存データのロードに失敗しました。保存を中止します。\n");
             return;
        }

        if (loadSuccess && !isEmpty) {
            for (const auto& var : existingVariables) {
                mergedData[var.variableName] = var.valueString;
            }
        }
    }

    // --- 2. 新しいデータをマージ ---
    for (const auto& pair : data) {
        mergedData[pair.first] = pair.second;
    }

    // --- 3. 一時ファイルへのストリーミング書き込み ---
    String tempFilePath = fullFilePath + ".tmp";
    File tempFile = fs.open(tempFilePath.c_str(), FILE_WRITE);
    if (!tempFile) {
        Serial.printf("Error (Save): Failed to open temp file: %s\n", tempFilePath.c_str());
        return;
    }

    bool tableReplaced = false;
    if (fs.exists(fullFilePath.c_str())) {
        File originalFile = fs.open(fullFilePath.c_str(), FILE_READ);
        if (!originalFile) {
            Serial.printf("Error (Save): Failed to open original file for reading: %s\n", fullFilePath.c_str());
            tempFile.close();
            fs.remove(tempFilePath.c_str());
            return;
        }

        String currentBlockBuffer = "";
        bool isTargetBlock = false;
        String currentTableName = ""; // 現在処理中のテーブル名を保持

        // ブロック単位で書き込むラムダ関数
        auto writeBlock = [&](const String& blockBuffer) {
            if (isTargetBlock) {
                // (1) ヘッダー（IDとTABLE_NAME）は保持
                String headerBuffer = "";
                String dataBuffer = ""; // 変数行とオプション行
                int newlinePos1 = blockBuffer.indexOf('\n');
                if (newlinePos1 != -1) {
                    headerBuffer += blockBuffer.substring(0, newlinePos1 + 1); // ### METT_TABLE_ID ###\n
                    int newlinePos2 = blockBuffer.indexOf('\n', newlinePos1 + 1);
                    if (newlinePos2 != -1) {
                        headerBuffer += blockBuffer.substring(newlinePos1 + 1, newlinePos2 + 1); // TABLE_NAME:...\n
                        dataBuffer = blockBuffer.substring(newlinePos2 + 1); // 残りのデータ
                    } else {
                        headerBuffer += blockBuffer.substring(newlinePos1 + 1); // TABLE_NAME:... (改行なしの場合)
                    }
                } else {
                    headerBuffer = blockBuffer; // ### METT_TABLE_ID ### のみ
                }

                tempFile.print(headerBuffer);

                std::set<String> varsWithOption;

                // (2) 既存のHENSU_OPTIONSをスキャンして保持＆変数名を記録
                int startIdx = 0;
                while(startIdx < dataBuffer.length()) {
                    int endIdx = dataBuffer.indexOf('\n', startIdx);
                    if (endIdx == -1) endIdx = dataBuffer.length();
                    String line = dataBuffer.substring(startIdx, endIdx);
                    String trimmedLine = line;
                    trimmedLine.trim();

                    // 終端マーカーはここでは書き込まない
                    if (trimmedLine.startsWith("------END_TABLE_")) {
                        startIdx = endIdx + 1;
                        continue;
                    }

                    if (trimmedLine.startsWith("HENSU_OPTIONS:")) {
                         // 新形式 HENSU_OPTIONS:variableName:options
                         int c1 = trimmedLine.indexOf(':');
                         if (c1 != -1) {
                             String varName = trimmedLine.substring(c1 + 1);
                             int c2 = varName.indexOf(':');
                             if (c2 != -1) {
                                 varName = varName.substring(0, c2); // varName部分
                                 tempFile.println(line); // 一致したらそのまま書き戻す
                                 varsWithOption.insert(varName);
                             } else {
                                // 不正な形式のHENSU_OPTIONSは無視
                             }
                         } else {
                            // 不正な形式のHENSU_OPTIONSは無視
                         }
                    }
                    startIdx = endIdx + 1;
                }

                // (3) mergedDataからvar:String:valueを書き込む
                //    (削除された変数はmergedDataに含まれないので書き込まれない)
                for (const auto& pair : mergedData) {
                    // String dataType = inferDataType2(pair.second); // データ型推測は不要に
                    tempFile.printf("%s:String:%s\n", pair.first.c_str(), pair.second.c_str()); // データ型は常にString

                    // (4) 新しい変数 or 既存変数でオプションがまだない場合、空のオプションを追記
                    if (varsWithOption.find(pair.first) == varsWithOption.end()) {
                        tempFile.printf("HENSU_OPTIONS:%s:\n", pair.first.c_str()); // テーブル名を削除
                    }
                }

                // (5) 終端マーカーを追加
                tempFile.printf("------END_TABLE_%s------\n", tableName.c_str());
                tableReplaced = true;
            } else {
                // 対象外のブロックはそのまま書き込む
                tempFile.print(blockBuffer);
            }
        };


        // 元ファイルを1行ずつ読み、ブロックを構築
        while (originalFile.available()) {
            String line = originalFile.readStringUntil('\n');
            String trimmedLine = line;
            trimmedLine.trim();

            if (trimmedLine.startsWith("### METT_TABLE_ID ###")) {
                if (!currentBlockBuffer.isEmpty()) {
                    writeBlock(currentBlockBuffer);
                }
                currentBlockBuffer = line + "\n";
                isTargetBlock = false;
                currentTableName = ""; // テーブル名をリセット
            } else {
                currentBlockBuffer += line + "\n";
                if (trimmedLine.startsWith("TABLE_NAME:")) {
                    String name = trimmedLine.substring(trimmedLine.indexOf(':') + 1);
                    name.trim();
                    currentTableName = name; // 現在のテーブル名を記録
                    if (name == tableName) {
                        isTargetBlock = true;
                    }
                }
            }
        }
        // 最後のブロックを書き込む
        if (!currentBlockBuffer.isEmpty()) {
            writeBlock(currentBlockBuffer);
        }
        originalFile.close();
    }

    // --- 4. ファイル内に既存テーブルがなかった場合 (新規追加) ---
    if (!tableReplaced) {
        tempFile.println("### METT_TABLE_ID ###");
        tempFile.println("TABLE_NAME:" + tableName);
        for (const auto& pair : mergedData) {
             // String dataType = inferDataType2(pair.second); // データ型推測は不要に
            tempFile.printf("%s:String:%s\n", pair.first.c_str(), pair.second.c_str()); // データ型は常にString
            // 新規テーブルなので、空のオプション行も追記
            tempFile.printf("HENSU_OPTIONS:%s:\n", pair.first.c_str()); // テーブル名を削除
        }
        // 新規テーブルの終端マーカーを追加
        tempFile.printf("------END_TABLE_%s------\n", tableName.c_str());
    }

    tempFile.close();

    // --- 5. ファイルを入れ替え ---
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

    if (!(isError)) {
        Serial.printf("Info (Save): File saved successfully. Table '%s' was %s.\n", tableName.c_str(), tableReplaced ? "updated" : "added");
    }
}

void loadMettFile(fs::FS &fs, const String& fullFilePath, const String& targetTableName,
                    bool& success, bool& isEmpty, std::vector<MettVariableInfo>& variables) { // isError 引数を削除
    // isError = false; // デフォルトで成功 -> 削除
    variables.clear();
    success = false;
    isEmpty = true;
    bool internalError = false; // 内部エラーフラグ

    // ファイルパスが絶対パスで、.tbl拡張子を持つことを確認
  

    if (!fs.exists(fullFilePath.c_str())) {
        Serial.printf("Info (Load): File does not exist: %s\n", fullFilePath.c_str());
        success = true; // ファイルがなくてもエラーではない
        return;
    }

    // --- 解析用にファイルを開く ---
    File file = fs.open(fullFilePath.c_str(), FILE_READ);
    if (!file) {
        Serial.printf("Error (Load): Failed to open file for parsing: %s\n", fullFilePath.c_str());
        internalError = true;
        return; // isErrorの代わりにここでリターン
    }

    if (file.size() == 0) {
        Serial.printf("Info (Load): File is empty: %s\n", fullFilePath.c_str());
        file.close();
        success = true;
        return;
    }

    Serial.printf("Info: ファイルをロード中: %s (テーブル: %s)\n", fullFilePath.c_str(), targetTableName.c_str());

    String currentTableNameInFile = "";
    bool shouldLoadCurrentTable = false;
    bool inTableContext = false;

    while (file.available()) {
        String line = file.readStringUntil('\n');
        String originalLine = line;
        line.trim();

        if (line.startsWith("#") || line.isEmpty()) {
            continue;
        }

        // ### METT_TABLE_ID ### 区切り文字を使用
        if (line.startsWith("### METT_TABLE_ID ###")) {
            currentTableNameInFile = "";
            shouldLoadCurrentTable = false;
            inTableContext = false;
            continue;
        }

        // ------END_TABLE_...------ 行は無視
        if (line.startsWith("------END_TABLE_")) {
            continue;
        }

        if (line.startsWith("TABLE_NAME:")) {
            inTableContext = true;
            int colonIndex = line.indexOf(':');
            currentTableNameInFile = (colonIndex != -1) ? line.substring(colonIndex + 1) : "";
            currentTableNameInFile.trim();

            shouldLoadCurrentTable = (targetTableName == currentTableNameInFile);
            continue;
        }

        // テーブルコンテキスト内（TABLE_NAME:行の後）かつ、対象テーブルの場合
        if (inTableContext && shouldLoadCurrentTable) {

            // HENSU_OPTIONS行は無視する
            if (line.startsWith("HENSU_OPTIONS:")) {
                continue;
            }

            // 形式は var:type:value だが、typeは常にStringとして扱う
            int firstColon = line.indexOf(':');
            int secondColon = line.indexOf(':', firstColon + 1);
            if (firstColon > 0 && secondColon > firstColon) {
                MettVariableInfo varInfo;
                varInfo.variableName = line.substring(0, firstColon);
                // varInfo.dataType = line.substring(firstColon + 1, secondColon); // ファイルから読み込むが使わない
                varInfo.dataType = "String"; // 常にStringとする
                varInfo.valueString = line.substring(secondColon + 1);
                varInfo.tableName = currentTableNameInFile;

                // 他のフィールドはデフォルト値
                varInfo.Options = ""; // loadMettFileでは読み込まない
                varInfo.Test = "";
                varInfo.id1 = 0;
                varInfo.id2 = 0.0f;
                varInfo.id3 = 0.0;
                varInfo.id4 = 0L;

                variables.push_back(varInfo);
            } else {
                Serial.printf("Warning (Load): Skipping malformed variable line in target table: %s\n", originalLine.c_str());
            }
        }
    }

    file.close();

    if (!internalError) { // ファイルオープンなどの内部エラーがなければ成功とする
      success = true;
    }
    isEmpty = variables.empty();

    if (!(isEmpty)) {
        Serial.printf("Info: ロード成功。変数: %d個\n", variables.size());
    } else if (success) { // 成功したが空の場合
        Serial.println("Warning: 指定されたテーブルが見つからないか、データがありませんでした。");
    }
    // internalErrorの場合は、エラーメッセージは既に出力済み
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

  int itemsPerP = 8;
        bool iszero = false;
        int ahc = 0;
  ExtractTablePageMett(SD,DirecX + ggmode,TTM,pagenum,itemsPerP,allhensuname,allhensuvalue,false,iszero,tt,ahc);
  if(!tt){
        M5.Lcd.fillScreen(BLACK);
        Serial.println("Load Error.");
        maxLinesPerPage = -1;
        kanketu("Load Error!",500);
        mainmode = 13;
        imano_page = 0;
      shokaipointer2(holdimanopage,DirecX + ggmode);
      maxpage = maxLinesPerPage;
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
  if (iszero) {
        M5.Lcd.fillScreen(BLACK);
        Serial.println("No tables found.");
        maxLinesPerPage = -1;
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextFont(3);
      M5.Lcd.println("No Tables!\n Press B");
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
    
    if(ahc % itemsPerPage == 0){
      maxLinesPerPage = ahc / itemsPerPage;
    }else{
      maxLinesPerPage = (ahc / itemsPerPage) + 1;
    }
    for (int i = start; i < end; ++i) {
        M5.Lcd.println("  " + allhensuname[i] +":val:" + allhensuvalue[i]);
        
    }
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, M5.Lcd.height() - 20);
    M5.Lcd.printf("Page: %d/%d", pagenum + 1, totalPages2);


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

//変数機能：作成，削除，複製，リネーム，変数値設定，変数値のNULL，変数値デフォルト値の設定，変数値タイプ（数値またはリストまたはDate），変数値のリセット，オプション，全テーブルへの列追加，全テーブルから列削除
void loop() {
  M5.update(); // ボタン状態を更新
 delay(1);//serial.println暴走対策,Allname[positpoint]はテーブル名
 if(mainmode == 17){

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
      }else if(positpoint == 0){//open
        M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0,0);
          TTM = AllName[holdpositpoints];
          holdpositpointx = positpoint;
          holdmaxpagex = maxpage;
          holdimanopagex = imano_page;
          positpoint = 0;
          imano_page = 0;
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
      M5.Lcd.println("  Open\n  Create\n  Rename\n  Delete\n  TableOptions\n  Duplicate\n  Deleterowswithname" );
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






