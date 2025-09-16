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
bool righttrue();
bool lefttrue();
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
extern void updatePointer2();
#endif // SHARED_H