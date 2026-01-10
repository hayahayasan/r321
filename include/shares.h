#ifndef SHARED_H
#define SHARED_H
#include <WiFi.h>
#include <FastLED.h>
// 他のファイルで定義された変数を参照するための宣言
extern int shared_variable;

extern int positpoint;
extern int positpointmax;
extern int positpointmaxg;
extern int positpointmain1;
extern int positpointmaxgmain1;
extern int positpointmaxmain1;
extern int positpointmaxmain2;
extern int maxLinesPerPage;
void shokaipointer2(int pageNum, String filePath  );
extern int mainmode;
extern int imano_page;
extern int maxpage;
extern int pagemoveflag;
extern bool btna;
extern bool btnc;
extern int frameright;
extern int frameleft;
extern String DirecX;
extern bool modordir;
extern int File_goukeifont;
extern int maxLinesPerPage2;
extern int maxLinesPerPage3;
extern String Tex2;
extern unsigned long lastTextScrollTime;
extern  unsigned long TEXT_SCROLL_INTERVAL_MS;
extern int current_positpoint_on_entry;
extern bool righttrue();
extern bool lefttrue();
extern int scrollPos;
extern String LANS;
typedef std::map<String, String> MettDataMap;
extern String Filelist[100];
void mainkansu_intmain();
bool createEE(MettDataMap& MDM,int type = 0);
extern String TexNet;
extern int IntNet;
extern String directlist[100];
extern String ForDlist[100];
void updatePointer2(int size = 3,int &imanopp = imano_page);
String HenkanTxt(const String& text);
extern String formatBytes(uint64_t bytes);
extern bool endsWithTxtOrDbm(String filename);
extern  String RESERVED_NAMES[];
extern  int NUM_RESERVED_NAMES;
extern bool containsForbiddenChars(String name);
extern bool isReservedName(String name) ;
extern bool isValidWindowsDirName(String textt);
extern bool isValidWindowsFileName(String textt) ;
extern String maeredirect(String path);
extern String wirecheck();
extern String migidkae(String karac);
extern int nowposit();
extern int nowpositZ();
extern String cleanPath(String path);
extern bool createDirRecursive(const char* path);
extern String checkAndRename(String filePath);
extern bool copyFile(const char* sourcePath, const char* destinationPath, uint32_t totalSize);
extern bool removePath(const char* path) ;
extern bool smartCopy(String sourcePath, String destinationPath, bool isCut) ;
// 他のファイルで定義された関数のプロトタイプ
void updatePointerLite();
extern int SCROLL_INTERVAL_FRAMES;
extern int SCROLL_SPEED_PIXELS;
bool checkSDCardOnly() ;
bool boolmax();
bool initializeSDCard();
bool areusure();
bool areubunki(String texta,String textb);
void nummempty();
void listSDRootContents(int pagetax,String Directtory,bool tt = false);
bool copyFileContent(String sourcePath, String destinationPath);
void moveDir(String srcDir, String destDir, bool keepOriginal) ;
void createTestFiles(String dirPath);
int createFile(String fileNameToCreate, String targetDirectoryPath);
int createDirectory(String SuperTT, String DirecXX);
int getCharWidth();
int getFontHeight() ;
extern int resercounter;
extern bool serious_error_flash ;
void releaseSDBusForOtherUse();
String getParentDirectory(String path);
bool browseFlashDirectoryPaginated(int pagetax, String Directtory) ;
bool textnetsette(String tablemozi);
String TexNet1(MettDataMap mmmc);
extern MettDataMap mmmc;
extern int IntNet1;
struct MettVariableInfo {
    String variableName;
    String dataType;
    String valueString;
    String tableName;
    String Options;
    String Test;
    int id1;
    float id2;
    double id3;
    long id4;
};
struct FileMettData {
    String fileName;
    size_t fileSize;
    String Optiondata;
    String Log;
    std::vector<MettVariableInfo> variables;
};
bool containsInvalidVariableNameChars(const String& name);
String getVariableString(const std::vector<MettVariableInfo>& variables, const String& varName);
int getVariableInt(const std::vector<MettVariableInfo>& variables, const String& varName);
void printFileM(const std::vector<FileMettData>& extractedDataList);
MettDataMap copyVectorToMap(const std::vector<MettVariableInfo>& variables);
void printTable(const String& fileName, const String& tableName, const std::vector<MettVariableInfo>& variables);
String joinVectorToString(const std::vector<String>& vec);
std::vector<String> getAllTableNamesInFile(fs::FS &fs, const String& fullFilePath);
std::vector<int> getVariableIntArray(const std::vector<MettVariableInfo>& variables, const String& varName);
template <typename T>
String joinVectorToString(const std::vector<T>& vec) ;
String joinStringVectorToString(const std::vector<String>& vec);
String inferDataType(const String& valueString) ;
bool containsInvalidTableNameChars(const String& name);
bool datt(String opthensuname,String setname,MettDataMap& datass);
const int SD_CS_PIN = 4;
extern MettDataMap dataToSaveE;
int areubunki2(String texta,String textb,String textc);
void displayMapContents(const String& context, const MettDataMap& dataMap);
void logExistingFileContents(fs::FS &fs, const String& fullFilePath);
extern const int CURSOR_BLINK_INTERVAL; // カーソル点滅のフレーム間隔
extern const int MAX_STRING_LENGTH ; 
extern int lastDrawnCursorScreenX ; 
extern int lastDrawnCursorScreenY ;
extern int scrollOffset ; // スクロールテキストの描画オフセット
extern int scrollFrameCounter ;
extern String mainprintex;
extern int  TABLE_IDd;
extern const String METT_TABLE_NAME_KEY ; 
extern const int METT_CHUNK_SIZE;
extern int holdimanopage;
extern int holdpositpointmax;
extern int holdpositpointt;
extern int maindex;
extern String  sitagar[10];
extern bool beginizeSD;
extern bool showAngleBrackets; 
extern unsigned long lastBlinkToggleTime;
bool isValidFormat(const String& str);

// ----------------------------------------------------------------------
// 1. フォーマット形式を指定するための定数
// ----------------------------------------------------------------------
const int FORMAT_FAT16 = 16;
const int FORMAT_FAT32 = 32;
bool isValidTableName(const String& tableName, const String existingNames[], size_t arraySize) ;
bool deleteTableInFile(fs::FS &fs, const String& fullFilePath, const String& tableNameToDelete);
// Struct to hold information about a single metadata file
void displayLoadedVariables(const MettDataMap& dataMap);
bool getVariableNamesInTable(fs::FS &fs, const String& fullFilePath, const String& targetTableName, bool& isZero, std::vector<String>& variableNames);
struct CursorPosInfo {
    int pixelX;
    int pixelY;
    int lineNum;
    int charInLine;
};
struct LineInfo {
    int startIndex; // 行の開始インデックス
    int length;     // 行の文字数（改行文字を除く）
};
struct ClientSession {
    uint8_t num;
    String ipAddress;
    String browserInfo;
    String userId;
};

extern QueueHandle_t processingQueue;
// キューに送るメッセージ構造体
enum TaskType {
    TASK_DATA_LOAD,
    TASK_DATA_SAVE,
    TASK_DATA_SEND_OTHER,
    TASK_DATA_SAMPLE,
    TASK_DATA_PING,      // 追加
    TASK_DATA_LIST,
    TASK_DATA_SOME
};

// キューに送るメッセージ構造体
struct TaskMessage {
    TaskType type;       // 処理の種類
    int clientNum;       // クライアント番号
    char* dataPayload;   // データ文字列1 (ポインタで渡す)
    char* dataPayload2;  // データ文字列2 (追加: ポインタで渡す)
};

LineInfo getCurrentLineInfo(int index, const String& text);
CursorPosInfo calculateCursorPixelPos(int index, const String& text);
LineInfo getPreviousLineInfo(int currentIndex, const String& text) ;
LineInfo getNextLineInfo(int currentIndex, const String& text);
int findIndexFromPixelX(int targetPixelX, int lineStartIndex, int lineLength, const String& text);
void performArrowKeyAction(const String& key);
void adjustScroll();
extern bool nosd;
extern bool serious_errorsd;
extern String karadirectname;
extern int goukei_page;
extern std::vector<ClientSession> SessionList;
void sessionMonitorTask(void *pvParameters);
extern int cursorPixelY;
extern int scrollpx;
extern int cursorPixelX;
extern String SuperT;
extern bool otroot;
extern std::map<String, String> sessionMap;
extern std::map<uint8_t, String> clientLookup;
extern int cursorIndex;
extern int offsetX;
extern int offsetY;
extern bool needsRedraw;
extern bool renameSDItem(String oldPath, String newPath);
extern int deleteRightmostSDItem(String itemPath) ;
extern void kanketu(String texx,int frame);

extern const size_t BUFFER_SIZE;
extern const char* TABLE_NAME_PATTERN ;
// 識別子 "NEW_DATA_SET_PATTERN" の定義
extern const char* NEW_DATA_SET_PATTERN ;
extern const size_t PATTERN_LEN ;
extern const size_t PATTERN_LEN1 ;
extern const size_t PATTERN_LEN2 ;

extern SdFs sd;
void releaseSdFatAndPrepareForSDLibrary();
bool isNonFAT16orFAT32Format();
String getSDCardType();
String getSDCardCIDInfo();
String getSDCardRawCapacity();
bool formatSDCardFull();
bool initializeSDCardAndCreateFile(const String& filePath);
std::vector<String> getAllTableNamesInFile(fs::FS &fs, const String& fullFilePath, bool& isZero);
bool boolmax();
void updatePointer(bool notext = false);
void shokaipointer();
void shokaipointer(bool yessdd);
extern String potlist[];
extern int numMenuItems;
void loadPotlistFromSD();
void drawCenteredText(const String& text, int yPos);
void drawTopText(bool showAngleBrackets);
void drawCenterText();
extern String optiontxt[6];
extern int currentPos;
void drawBottomText() ;
extern bool rootnofile;
String trimString(const String& s);
void displayPageInfo(int currentPage, int totalPages);
extern String AllName[100];
std::vector<FileMettData> scanAndExtractMettData(fs::FS &fs, String DirecD);
void parseMettBlockToMap(const String& variableLines, MettDataMap& dataMap);
std::vector<String> getUniqueTableNames(const std::vector<MettVariableInfo>& variables);
String getMettVariableValue(const MettDataMap& tableData, const String& variableName);
bool isValidFilesystemPath(const String& path);
void loadMettFile(fs::FS &fs, const String& fullFilePath, const String& targetTableName, bool& success, bool& isEmpty, std::vector<MettVariableInfo>& variables);
void saveMettFile(fs::FS &fs, const String& fullFilePath, const String& tableName, const MettDataMap& data, bool& isError);
void updatePointerAndDisplay(int ril);
extern int holdpositpoint;
void textexx();
int selectOption(const String options[], int numOptions, const String upperText, const String lowerText);
extern String sita;
extern int entryenter;
extern bool filebrat;
extern String Textex;
extern String maereposit;
void textluck(int poichi = 0); 
extern bool firstScrollLoop;
void showmozinn2(const String& txt);
extern String copymotroot;
extern int sizex;
extern String tttt;
extern bool copymotdir;
extern int holdpositpointd;
extern int holdimanopaged;
extern int holdpositpointmaxd;
extern String ggmode;
extern String Filelist[];
extern int fontdd;
extern void mainkansu_optsd();
bool readSdFileToStringForced(const String& filePath, String &SSText, int &mozikode);
bool containsInvalidChars(const String& str);
std::vector<String> splitString(const String& str, char delim = ',');
String joinStringVector(const std::vector<String>& vec, const char* delim = ",");
void saveHensuOptions(fs::FS &fs, const String& fullFilePath, const String& targetTableName, const String& targetVariableName, const std::vector<String>& options, bool* isError);
std::vector<String> loadHensuOptions(fs::FS &fs, const String& fullFilePath, const String& targetTableName, const String& targetVariableName, bool& isNull, bool& isError);
bool renameHensuInTable(fs::FS &fs, const String& fullFilePath, const String& tableName, const String& oldVariableName, const String& newVariableName);
bool DeleteHensuInMettTable(fs::FS &fs, const String& fullFilePath, const String& tableName, const String& variableName);
void ExtractTablePageMett(fs::FS &fs, const String& fullFilePath, const String& targetTableName,
                                int pageNumber, int itemsPerPage,
                                std::vector<String>& variableNames, std::vector<String>& values,
                                std::vector<String>& ids, // ★★★ 変更: int -> String ★★★
                                bool zenbu,  bool& isError, int& allhensucount);
void saveHensuOptions(fs::FS &fs, const String& fullFilePath, const String& targetTableName, const String& targetVariableName, const std::vector<String>& options, bool& isError);
bool duplicateMettFile(fs::FS &fs, const String& fullFilePath, const String& oldTableName, const String& newTableName, bool& isError);
bool deleteTableInFile(fs::FS &fs, const String& fullFilePath, const String& tableName, bool* isError);
bool isValidAndUniqueVariableName(fs::FS &fs, const String& fullFilePath, const String& targetTableName, const String& newVariableName, bool iskaigho = false);
void createMettHensu(fs::FS &fs, const String& fullFilePath, const String& targetTableName, 
                           const String& targetVariableName, const String& value, 
                           bool isWrite, int id, bool& isError);
bool loadMettHensu(fs::FS &fs, const String& fullFilePath, const String& targetTableName, const String& targetVariableName, 
                         String& value, bool& idIsNull, int& idReturn);

bool saveHensuId(fs::FS &fs, const String& fullFilePath, const String& targetTableName, 
                       const String& targetVariableName, bool isNull, int idValue);
 bool loadHensuId(fs::FS &fs, const String& fullFilePath, const String& targetTableName, const String& targetVariableName, 
                       bool& idIsNull, int& idReturn);
bool removeMettTable(fs::FS &fs, const String& fullFilePath, const String& tableName, bool& isError);
bool removeMettVariable(fs::FS &fs, const String& fullFilePath, const String& tableName, const String& variableName, bool& isError);
bool renameTableInMettFile(fs::FS &fs, const String& fullFilePath, const String& oldTableName, const String& newTableName, bool& isError);
extern const int itemsPerPage ; // Number of items to display on a page
extern std::vector<String> allTableNames;
extern std::vector<String> allTableNames2;
extern String fefe;
void updatePointerAndDisplay(int ril) ;
bool initializeSDCard(String ss) ;
void showmozinn(const String& text) ;
void createjj();
void opt_hukusei();
void shokaipointer3();
void opt1_kaimei(int id);
bool optkobun();
bool test_load();
bool isValidHensuValue(String& text, bool isHairetsu);
String GyakuhenkanTxt(const String& text);
String getDateTimeString();
String findLineStartingWithPrefix(const std::vector<String>& lines, const String& prefix, int& foundIndex);
bool GetOptDirect(String tablezenhan,String& returnstext);
bool tableopt_load(String texx,String& returnAtai);
 bool isValidInteger0To100000(String& input);
 String textsus(String defotrxt,String texnum,int rule);
void defval(int rule,bool returns);
extern int holdpositpointx2;
extern bool returnss;
void shokaipointer3();
void shokaipointer2(int pageNum, String filePath  ) ;
void shokaipointer4(int pagenum = 0);
void checkemptyhensu(String Hensu,String atai,String Kaerichi = "");
int shokaivector(std::vector<String>& vec, const String& kakikomumozi);
extern int holdimanopagex2;
extern String TTM;
extern String TTM2;
extern int holdpositpointx3;
extern int holdimanopagex3;
extern std::vector<String> allhensuvalue;
extern int imano_pagek;
extern int holdpositpoints;
extern std::vector<String> allhensuname;
void shokaioptionhensu();
bool quickWriteOptions(String opttext1,String opttext2);
void shokaipointer5(int pagenum = 0,int itemsPerP = 8);
void setoptnul();
extern int holdmaxpagex3;
void clearBottomArea();
void scrollTextBottom(const String& text);
extern bool Tflag;
extern String statustext;
void duplicateMettHensu(fs::FS &fs, const String& fullFilePath, const String& targetTableName, 
                        const String& sourceVariableName, const String& destVariableName, 
                        bool isWrite, int id, bool& isError);
bool loadmett();
void suguseni();
extern String mozikk[7];
bool writeStringToFileForced(const String& filePath, const String &SSText, int mozikode);
void initVirtualKeyboard() ;
void drawVirtualKeyboard() ;
String getGatewayMAC() ;
extern bool g_isWorldInternet ;
String getCipherName(wifi_cipher_type_t cipher) ;
void collectWSTT();
void showStatus(String msg, uint16_t color) ;
bool connectToEnterpriseWiFi(String ssid, String id, String pass);
extern String UU;
String getWiFiStatusName(wl_status_t status) ;
extern std::vector<String> WSTT; 
void resetto31();
void disconnectWiFi();
bool checkWiFiConnection();
void stopWebSocket();
void startWebSocket();
void monitorConnectionLoss();
extern bool manual_wifi ;
extern bool isWebSocketActive ;
void handleWebSocketLoop();
void updateSessionDisplay() ;
extern int SessionSized;
bool sendMessageByNum(String numStr, String message);
int sessionSelectAndSendNonBlocking(String Txxxtsosin,String annnai);
extern String TexNet2;
extern int IntNet2;
void ReceiveWebM(uint8_t num, String content,String RTCDate) ;
void updateMailDisplay(const std::vector<String>& MailRList);
extern std::vector<String> MailRList;
extern bool isshokai;
std::vector<String> getConnectedUserIds();
extern int SSListc ;
void checkidandsave2(int sendbynum);
void checkidandsave1(int sendbynum);
void thedataload(int nummm,String id);
void thedatasave(int nummm,String datasavecontent,String id);
void backgroundProcessingTask(void *pvParameters);
void sendToWorkerTask(TaskType type, int num, String data1 = "", String data2 = "");
void thedatasendother(int nummm,String user1,String user2);
void thedatasample(int nummm);
void thedataping(int nummm);
int getClientNumByUserId(String userId);
bool datt2(String opthensuname,MettDataMap& datass);
void updateMailDisplay2(const String& MailText);
void thedatasomething(int nummm,String isall,String textf);
bool forceDisconnectClient(int nummm);
extern bool isServerRunning;

void startWebServer();
void stopWebServer();
void turnOnLED(CRGB color);
void turnOffLED();
#endif // SHARED