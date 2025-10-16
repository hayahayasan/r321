#ifndef SHARED_H
#define SHARED_H

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
extern  int SCROLL_SPEED_PIXELS;
extern String Filelist[100];
extern String directlist[100];
extern String ForDlist[100];
extern void  updatePointer2(bool) ;
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
typedef std::map<String, String> MettDataMap;
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
bool datt(String opthensuname,String setname);
const int SD_CS_PIN = 4;
extern MettDataMap DataToSaveE;

// ----------------------------------------------------------------------
// 1. フォーマット形式を指定するための定数
// ----------------------------------------------------------------------
const int FORMAT_FAT16 = 16;
const int FORMAT_FAT32 = 32;
bool isValidTableName(const String& tableName, const String existingNames[], size_t arraySize) ;
bool deleteTableInFile(fs::FS &fs, const String& fullFilePath, const String& tableNameToDelete);
// Struct to hold information about a single metadata file

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
extern int cursorPixelY;
extern int scrollpx;
extern int cursorPixelX;
extern String SuperT;
extern bool otroot;
extern int cursorIndex;
extern int offsetX;
extern int offsetY;
extern bool needsRedraw;
extern bool renameSDItem(String oldPath, String newPath);
extern int deleteRightmostSDItem(String itemPath) ;
extern void kanketu(String texx,int frame);
extern void  updatePointer2();
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
bool renameTableInMettFile(fs::FS &fs, const String& fullFilePath, const String& oldTableName, const String& newTableName, bool& isError);
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
void textluck();
extern bool firstScrollLoop;
extern String copymotroot;
extern int sizex;
extern String tttt;
extern bool copymotdir;
extern int holdpositpointd;
extern int holdimanopaged;
extern int holdpositpointmaxd;
extern String ggmode;
extern String Filelist[];
extern void mainkansu_optsd();






#endif // SHARED