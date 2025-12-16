//SD書き込み，内蔵フラッシュ読み書き，内蔵フラッシュの再起動時にRAMのように削除される変数の策定によるフラッシュ越しのリアルタイム通信
//内蔵フラッシュのデータのSDバックアップ自動処理，そしてデバイスからSDとフラッシュ両方に読み書き可能にする機能追加






#include <M5Unified.h>
#include <Wire.h>
#include<SD.h>
#include <USB.h> 
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <FS.h>
#include <vector>    // std::vector を使用するために必要
#include <algorithm>
#include <map>      // std::mapを使用するため
#include <set>
#include <SDFat.h>
#include <SPIFFS.h>
#include <SPI.h> 
#include <ETH.h>        // Ethernetライブラリを追加
#include <ESPmDNS.h>    // mDNSライブラリを追加
#include "shares.h"

// スクロール表示用のグローバル変数
// M5Stackの画面解像度: 320x240
// 最下部の表示エリアのY座標と高さ
int scrollXOffset = 320;
const int SCROLL_SPEED = 1;
const int BOTTOM_AREA_Y = M5.Lcd.height() - 30; // 240 - 30 = 210
const int BOTTOM_AREA_HEIGHT = 30;
// スクロールテキストのX座標オフセット
int scrollXOffsett = M5.Lcd.width(); // 初期位置は画面右外側 (320)
int counterSC = 4;
int minic = 0;
String LANS = "  S0:LANPORT\n  S1:WIFI SET\n  S2:WEBSOCKET\n  S3:DATA_STATUS\n  S4:DISCONNECT";

void clearBottomArea() {
    M5.Lcd.fillRect(0, BOTTOM_AREA_Y, M5.Lcd.width(), BOTTOM_AREA_HEIGHT, BLACK);
}
String mozikk[7] = {"No BOM","BOM UTF8","UTF16BE","UTF16LE","No Texts","UTF32 BE","UTF32 LE"};
/**
 * @brief 画面最下部に指定された文字列を左にスクロール表示し続けます。
 * * スクロール位置はグローバル変数 `scrollXOffset` で管理されます。
 * @param text スクロール表示する文字列
 */
void scrollTextBottom(const String& text) {
    // Textex グローバル変数を更新
    Textex = text;
    minic++;
    if(minic < counterSC){
        return;
    }else{
        minic = 0;
    }
    // 1. 最下部エリアを黒で塗りつぶして古い表示をクリア
    // M5.Lcd.height() - getFontHeight() で、文字のベースラインY座標を取得
    int scrollLineY = M5.Lcd.height() - getFontHeight(); 

    // スクロールテキスト領域をクリア（BOTTOM_AREA_HEIGHTではなく、文字の高さに合わせる）
    M5.Lcd.fillRect(0, scrollLineY, M5.Lcd.width(), getFontHeight(), BLACK);
    
    // 2. テキスト設定
    M5.Lcd.setTextSize(2); // サイズ 2 (約 16x16 pixel)
    M5.Lcd.setTextColor(YELLOW, BLACK); // 文字色: 黄, 背景色: 黒

    // テキスト幅の計算
    int textWidth = M5.Lcd.textWidth(Textex.c_str()); 
    
    // --- スクロールロジック (テキスト長に関わらず常時スクロールするよう修正) ---
    scrollFrameCounter++;
    if (scrollFrameCounter >= SCROLL_INTERVAL_FRAMES) {
        
        // テキスト長に関わらず、常に左へスクロール
        scrollOffset -= SCROLL_SPEED_PIXELS; 

        // テキストが完全に画面外に出たら、右端から再開
        // 条件: scrollOffset が -textWidth になったらリセット（完全に脱走）
        if (scrollOffset < -textWidth) {
            scrollOffset = M5.Lcd.width(); // 画面右端から再開
        }
        
        // 初回ループフラグは、常時スクロールでは不要になるため、ここでは無視
        // firstScrollLoop = false; 

        scrollFrameCounter = 0;
    }

    // 3. Textexが空でなければ描画
    if (Textex.length() > 0) {
        // スクロールオフセットを考慮して描画
        // Y座標は scrollLineY (M5.Lcd.height() - getFontHeight()) を使用し、文字が最下部に配置されるようにする
        M5.Lcd.drawString(Textex, scrollOffset, scrollLineY);
    }
}