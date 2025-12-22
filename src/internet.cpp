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
#include <WiFi.h>
#include <esp_wpa2.h>
#include <lwip/etharp.h>

// Direct inclusion of ESP-IDF headers for low-level WiFi operations
extern "C" {
  #include "esp_system.h"
  #include "esp_wifi.h"
  #include "esp_wifi_types.h"
  #include "esp_event.h"
  // Note: Older ESP-IDF versions use these functions to handle Enterprise WiFi
  #include "esp_wpa2.h"
}


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


bool textnetsette(String tablemozi){
      std::vector<MettVariableInfo> loadedVariablesE;
       bool loadSuccess = false;
      bool fileIsEmpty =false;
          loadMettFile(SD, "/save/save2.mett" ,"TestOpt2", loadSuccess, fileIsEmpty, loadedVariablesE);
    if(!loadSuccess){
      Serial.println("load_error!!!");
      return false;
    }
  dataToSaveE = copyVectorToMap(loadedVariablesE);
    SuperT = GyakuhenkanTxt(dataToSaveE[tablemozi]);
    Textex = "No kaigho,please Enter mozis";
    entryenter = 0;
    while(true){
        M5.update();
        delay(1);
        textluck();
        if(entryenter == 2){
    SuperT = "";
            entryenter = 0;
      return true;
    }else if(entryenter == 1){
        entryenter = 0;
        if(!isValidFormat(SuperT)){
            Textex = "No Right Formats!";
            continue;
        }
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0,0);
        M5.Lcd.println("Saving...");
        saveMettFile(SD, "/save/save2.mett" ,"TestOpt2", dataToSaveE, loadSuccess);
        if(!loadSuccess){
            kanketu("saved",300);
             M5.Lcd.fillScreen(BLACK);
            return true;
        }else{
            return false;
        }

    }
    }

}







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


const char* E_SSID = "SCHOOL_WIFI_NAME";
const char* E_ID   = "STUDENT_ID";
const char* E_PASS = "STUDENT_PASSWORD";


// 状態管理
std::vector<String> networkData;
WebSocketsServer webSocket = WebSocketsServer(81);

/**
 * ゲートウェイのMACアドレス取得 (LwIP準拠)
 */
String getGatewayMAC() {
    IPAddress gw = WiFi.gatewayIP();
    ip4_addr_t gateway_ip;
    gateway_ip.addr = (uint32_t)gw; 
    
    struct eth_addr* eth_ret = nullptr;
    const ip4_addr_t* ip_ret = nullptr;
    
    // ARPテーブル更新
    WiFiClient client;
    client.connect(gw, 80);
    delay(50);

    // netif_default を使用してネットワークインターフェースを指定
    struct netif* netif_default = netif_find("st1"); // Station mode
    if (!netif_default) netif_default = netif_list;

    if (etharp_find_addr(netif_default, &gateway_ip, &eth_ret, &ip_ret) >= 0 && eth_ret != nullptr) {
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 eth_ret->addr[0], eth_ret->addr[1], eth_ret->addr[2],
                 eth_ret->addr[3], eth_ret->addr[4], eth_ret->addr[5]);
        return String(macStr);
    }
    return "00:00:00:00:00:00";
}

String getAuthModeName(wifi_auth_mode_t auth_mode) {
    switch (auth_mode) {
        case WIFI_AUTH_OPEN: return "Open";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA_PSK";
        case WIFI_AUTH_WPA2_PSK: return "WPA2_PSK";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA_WPA2_PSK";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2_ENTERPRISE";
        case WIFI_AUTH_WPA3_PSK: return "WPA3_PSK";
        default: return "Unknown";
    }
}

#include <vector>

std::vector<String> collectDetailedNetworkInfo() {
    std::vector<String> info;

    // 接続状態
    info.push_back(String("connected=") +
                   (WiFi.status() == WL_CONNECTED ? "true" : "false"));

    if (WiFi.status() == WL_CONNECTED) {
        info.push_back("ssid=" + WiFi.SSID());
        info.push_back("ip=" + WiFi.localIP().toString());
        info.push_back("rssi=" + String(WiFi.RSSI()));
        info.push_back("channel=" + String(WiFi.channel()));
        info.push_back("mac=" + WiFi.macAddress());
    }

    info.push_back("uptime=" + String(millis() / 1000));

    return info;
}


bool connectToEnterpriseWiFi() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    // エンタープライズ認証の設定
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)E_ID, strlen(E_ID));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)E_ID, strlen(E_ID));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)E_PASS, strlen(E_PASS));
    esp_wifi_sta_wpa2_ent_enable();

    WiFi.begin(E_SSID);

    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 40) {
        delay(500);
        Serial.print(".");
        timeout++;
    }
    return (WiFi.status() == WL_CONNECTED);
}