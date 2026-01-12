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
#include <ESPAsyncWebServer.h>

#include <algorithm>
// Direct inclusion of ESP-IDF headers for low-level WiFi operations
extern "C" {
  #include "esp_system.h"
  #include "esp_wifi.h"
  #include "esp_wifi_types.h"
  #include "esp_event.h"
  // Note: Older ESP-IDF versions use these functions to handle Enterprise WiFi
  #include "esp_wpa2.h"
}

// 現在のIPアドレスを保持する変数
String currentIPString = "0.0.0.0";

// --- LAN設定 (M5Stack CoreS3 + LAN Module W5500) ---
// W5500はSPI接続です。CoreS3の底面MBUSのSPIピンアサインに合わせます。
#define ETH_SPI_SCK  36
#define ETH_SPI_MISO 35
#define ETH_SPI_MOSI 37
#define ETH_CS_PIN   9   // LANモジュールのCSピン（Baseによって26の場合もあり）
#define ETH_INT_PIN  10  // 割り込みピン
#define ETH_RST_PIN  -1  // リセットピン（なければ-1）
#define ETH_ADDR     1   // PHYアドレス（通常1）
#define ETH_POWER_PIN -1 // 電源制御ピン（なければ-1）

// 直結用固定IP設定 (DHCP失敗時のフォールバック用)
IPAddress staticIP(192, 168, 0, 200);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);
String stationIPString;
static bool eth_connected = false;

bool issoftap;
// スクロール表示用のグローバル変数
// M5Stackの画面解像度: 320x240
// 最下部の表示エリアのY座標と高さ
int scrollXOffset = 320;
bool wifi_nosoapap = false;
const int SCROLL_SPEED = 1;
const int BOTTOM_AREA_Y = M5.Lcd.height() - 30; // 240 - 30 = 210
const int BOTTOM_AREA_HEIGHT = 30;
int SSListc = 0;
// スクロールテキストのX座標オフセット
int scrollXOffsett = M5.Lcd.width(); // 初期位置は画面右外側 (320)
int counterSC = 4;
int minic = 0;
String UU;
String TexNet = "  S0:GETWIFI\n  S1:SOFTAP\n  S2:WEBSERVER\n  S3:SESSIONS\n  S4:DISCONNECT\n  S5:DO_AUTO";
int IntNet = 6;
String TexNet2 = "  Send Text\n  Receive Text\n  All UserID LIST\n  Force Exit";
int IntNet2 = 4;
bool manual_wifi = false;
bool g_isWorldInternet;
std::vector<String> WSTT; // 詳細情報を格納するベクター
int SessionSized = 0;
bool isshokai = false;
// 状態管理用のグローバル変数
int g_lastListSize = -1; // 初期値を-1にして初回描画を強制
int g_currentScrollY = 0; // 行単位のスクロール位置
int g_totalWrappedLines = 0; // 折り返しを含めた総行数

// 追加: 更新メッセージ表示制御用
bool g_isShowingUpdateMessage = false;
unsigned long g_updateMessageStartTime = 0;
WebSocketsServer webSocket = WebSocketsServer(65500);
bool isWebSocketActive = false; // WebSocketサーバーの稼働状態フラグ
// セッションリスト (接続中のユーザーを管理)

std::vector<String> MailRList;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");   // ws://<IP>/ws

bool isServerRunning = false;
bool isInitialized   = false;

// セッション管理用マップ
// 1. メイン管理マップ: Key=ユーザーID, Value=詳細情報(IP, 時刻, デバイス名など)
std::map<String, String> sessionMap;

// 2. 逆引き用マップ: Key=接続番号(num), Value=ユーザーID
// ※ 切断イベント(numしか分からない)時に、素早くユーザーIDを特定して削除するために使用
std::map<uint8_t, String> clientLookup;



// キューハンドル
QueueHandle_t processingQueue;




String getTimestampStr() {
    // もしNTP同期していれば getLocalTime を使えますが、ここでは稼働時間を使用
    return String(millis() / 1000) + "s";
}

String TexNet1(MettDataMap mmmc){
    return "  SSID:" + GyakuhenkanTxt(mmmc["table_SSID"]) + "\n  Username:" + GyakuhenkanTxt(mmmc["table_Usrname"] )+ "\n  Password:" + GyakuhenkanTxt(mmmc["table_Pass"]) + "\n  Login\n  Wifi Status\n  SSID Scanner" ;
}
int IntNet1 = 6;
bool createEE(MettDataMap& MDM,int type){
    
    
  std::vector<MettVariableInfo> loadedVariablesE;
       bool loadSuccess = false;
      bool fileIsEmpty =false;


        //step1:ファイルを作成する
bool tt = initializeSDCardAndCreateFile("/save/save2.mett");
if(!tt){
    Serial.println("チェック失敗ファイル");
    return false;
}

    

    loadMettFile(SD, "/save/save2.mett" ,"TestOpt2", loadSuccess, fileIsEmpty, loadedVariablesE);
    if(!loadSuccess){
      Serial.println("load_error!!!");
      return false;
    }
  dataToSaveE = copyVectorToMap(loadedVariablesE);
  if(type == 0){
    bool jj = false;

        if(datt("table_SSID","",dataToSaveE)){
          jj = true;
        }
        if(datt("table_Usrname","",dataToSaveE)){
          jj = true;
        }
        if(datt("table_Pass","",dataToSaveE)){
          jj = true;
        }

       
        if(jj){
       
          saveMettFile(SD, "/save/save2.mett" ,"TestOpt2", dataToSaveE, loadSuccess);
  }else {
    loadSuccess = false;
  }
}else if(type == 1){
    loadSuccess = false;
}
        
          if(!loadSuccess){
            //kanketu("Option Saved!",200);
            mmmc = dataToSaveE;
            return true;
          }else{
           kanketu("Option Save Failed!",200);
           return false;
          }
        
        
}

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
        dataToSaveE[tablemozi] = HenkanTxt(SuperT);
        saveMettFile(SD, "/save/save2.mett" ,"TestOpt2", dataToSaveE, loadSuccess);
        if(!loadSuccess){
            kanketu("saved",300);
            M5.Lcd.setTextSize(1);
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

/**
 * 暗号化タイプの名称取得ヘルパー
 */
String getCipherName(wifi_cipher_type_t cipher) {
    switch (cipher) {
        case WIFI_CIPHER_TYPE_NONE: return "NONE";
        case WIFI_CIPHER_TYPE_WEP40: return "WEP40";
        case WIFI_CIPHER_TYPE_WEP104: return "WEP104";
        case WIFI_CIPHER_TYPE_TKIP: return "TKIP";
        case WIFI_CIPHER_TYPE_CCMP: return "CCMP";
        case WIFI_CIPHER_TYPE_TKIP_CCMP: return "TKIP_CCMP";
        case WIFI_CIPHER_TYPE_AES_CMAC128: return "AES_CMAC128";
        case WIFI_CIPHER_TYPE_SMS4: return "SMS4";
        case WIFI_CIPHER_TYPE_GCMP: return "GCMP";
        case WIFI_CIPHER_TYPE_GCMP256: return "GCMP256";
        default: return "Unknown";
    }
}

/**
 * WSTTベクターに指定順序でネットワーク情報を格納する
 * 可能な限りの情報を網羅して格納します。
 */
void collectWSTT() {
    WSTT.clear();
    
    // 1. ローカルIPv4 (DHCPでルーターから割り当てられたアドレス)
    // 接続成功後に呼ばれるため、ここには 192.168.x.x 等の有効なIPが入ります
    WSTT.push_back(WiFi.localIP().toString());
    
    // 2. ルーターIPv4
    WSTT.push_back(WiFi.gatewayIP().toString());
    // 3. ローカルMAC
    WSTT.push_back(WiFi.macAddress());
    // 4. ルーターMAC
    WSTT.push_back(getGatewayMAC());
    // 5. サブネットマスク
    WSTT.push_back(WiFi.subnetMask().toString());
    
    // 6. グローバル接続確認 (bool string)
    WSTT.push_back(g_isWorldInternet ? "true" : "false");
    
    // AP詳細情報の取得
    wifi_ap_record_t info;
    bool apInfoOK = (esp_wifi_sta_get_ap_info(&info) == ESP_OK);

    // 7. WiFiセキュリティタイプ (Auth Mode)
    String security = "Unknown";
    if (apInfoOK) {
        security = getAuthModeName(info.authmode);
    }
    WSTT.push_back(security);

    // 8. SSID
    WSTT.push_back(WiFi.SSID());
    // 9. BSSID (APのMACアドレス)
    WSTT.push_back(WiFi.BSSIDstr());
    // 10. RSSI (信号強度)
    WSTT.push_back(String(WiFi.RSSI()));
    // 11. Channel
    WSTT.push_back(String(WiFi.channel()));
    // 12. DNS 1
    WSTT.push_back(WiFi.dnsIP(0).toString());

    // 13. DNS 2 (セカンダリDNS)
    WSTT.push_back(WiFi.dnsIP(1).toString());

    // 14. IPv6 Address (Link Local)
    WSTT.push_back(WiFi.localIPv6().toString());

    // 15. Hostname
    WSTT.push_back(String(WiFi.getHostname()));

    // 16. Broadcast IP
    WSTT.push_back(WiFi.broadcastIP().toString());

    // 17. Pairwise Cipher (ユニキャスト暗号化方式)
    if (apInfoOK) {
        WSTT.push_back(getCipherName(info.pairwise_cipher));
    } else {
        WSTT.push_back("Unknown");
    }

    // 18. Group Cipher (マルチキャスト/ブロードキャスト暗号化方式)
    if (apInfoOK) {
        WSTT.push_back(getCipherName(info.group_cipher));
    } else {
        WSTT.push_back("Unknown");
    }

    // 19. PHY Mode (通信規格: 11b/g/n)
    if (apInfoOK) {
        String phy = "";
        if (info.phy_11b) phy += "11b ";
        if (info.phy_11g) phy += "11g ";
        if (info.phy_11n) phy += "11n ";
        if (info.phy_lr)  phy += "LR ";
        if (phy == "") phy = "Unknown";
        WSTT.push_back(phy);
    } else {
        WSTT.push_back("Unknown");
    }

    // 20. WiFi Country Code
    wifi_country_t country;
    if (esp_wifi_get_country(&country) == ESP_OK) {
        String cc = String(country.cc[0]) + String(country.cc[1]); // e.g., "JP", "CN", "US"
        WSTT.push_back(cc);
    } else {
        WSTT.push_back("Unknown");
    }
}




void disconnectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nDisconnecting WiFi...");
        WiFi.disconnect(true); // true = WiFiオフにする
        WiFi.mode(WIFI_OFF);
        delay(100);
        MailRList.clear();
        SSListc = 0;
        SessionSized = 0;
        wifi_links = false;
        issta = false;
        Serial.println("WiFi Disconnected.");
        manual_wifi = false;
        WSTT.clear();
        UU = "Disconnected by User";
        showStatus("WiFi Disconnected", YELLOW);
    } else {
        Serial.println("WiFi was not connected.");
    }
}
/**
 * 画面クリア付きのステータス表示関数
 */
void showStatus(String msg, uint16_t color) {
    M5.Lcd.fillScreen(BLACK); // 前の文字を消す
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(color, BLACK);
    M5.Lcd.println(msg);
    M5.Lcd.setTextColor(WHITE, BLACK); // 色を戻す
}

// 引数で SSID, ID, Password を受け取るように変更
bool connectToEnterpriseWiFi(String ssid, String id, String pass) {
    Serial.println("\n--- WiFi Connection Start ---");
    UU = ""; // エラー情報をリセット
    
    // 初期表示
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("WiFi Connecting...");
    M5.Lcd.printf("Target: %s\n", ssid.c_str());

    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    // --- 事前スキャンでSSIDの存在を確認 ---
    Serial.println("Scanning for SSID...");
    int n = WiFi.scanNetworks();
    bool ssidFound = false;
    for (int i = 0; i < n; ++i) {
        if (WiFi.SSID(i) == ssid) {
            ssidFound = true;
            break;
        }
    }

    if (!ssidFound) {
        String err = "[E00] SSID Not Found (Scan Failed)";
        Serial.println("\n" + err);
        UU = err;
        showStatus("SSID Not Found\n(Scan)", RED);
        return false;
    }
    Serial.println("SSID Found. Connecting...");

    // --- 接続モードの判定 ---
    // IDもPassも空 -> Openネットワーク
    if (id.isEmpty() && pass.isEmpty()) {
        Serial.println("Mode: OPEN (No Password)");
        M5.Lcd.println("Mode: OPEN");
        WiFi.begin(ssid.c_str());
    } 
    // IDが空でPassがある -> WPA2/WPA3 Personal (PSK)
    else if (id.isEmpty()) {
        Serial.println("Mode: WPA Personal (PSK)");
        M5.Lcd.println("Mode: Personal");
        WiFi.begin(ssid.c_str(), pass.c_str());
    } 
    // IDがある -> WPA2/WPA3 Enterprise
    else {
        Serial.println("Mode: WPA Enterprise");
        M5.Lcd.println("Mode: Enterprise");
        // String型から c_str() で const char* を取得し、length() で長さを指定
        esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)id.c_str(), id.length());
        esp_wifi_sta_wpa2_ent_set_username((uint8_t *)id.c_str(), id.length());
        esp_wifi_sta_wpa2_ent_set_password((uint8_t *)pass.c_str(), pass.length());
        esp_wifi_sta_wpa2_ent_enable();
        WiFi.begin(ssid.c_str());
    }

    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 40) { // 20秒タイムアウト (500ms * 40)
        delay(500);
        Serial.print(".");
        M5.Lcd.print("."); // ドットは追記していく
        timeout++;

        // --- 接続ステータスの詳細チェック ---
        wl_status_t status = WiFi.status();
        
        // 即座に失敗とみなすステータス
        if (status == WL_CONNECT_FAILED) {
            String err = "[E01] Connect Failed: Password/Auth Error";
            Serial.println("\n" + err);
            UU = err;
            showStatus("Connect Failed!\nCheck Pass/ID", RED);
            return false;
        } else if (status == WL_NO_SSID_AVAIL) {
            String err = "[E02] SSID Not Found";
            Serial.println("\n" + err);
            UU = err;
            showStatus("SSID Not Found", RED);
            return false;
        } else if (status == WL_NO_SHIELD) {
            String err = "[E04] Hardware Error: No WiFi Shield";
            Serial.println("\n" + err);
            UU = err;
            showStatus("Hardware Error:\nNo Shield", RED);
            return false;
        }
        
        // 以下のステータスは待機中に発生しうるため、タイムアウトまで待つがログには出す
        // WL_IDLE_STATUS, WL_SCAN_COMPLETED, WL_DISCONNECTED, WL_CONNECTION_LOST
    }

    // タイムアウト後の最終チェック
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[Success] WiFi Connected & IP Obtained");
        
        // --- インターネット接続確認 (NTP取得) ---
        Serial.print("Checking Internet (NTP)... ");
        M5.Lcd.println("\nChecking Net...");
        
        // NTP設定 (JST, pool.ntp.org)
        configTime(9 * 3600, 0, "pool.ntp.org", "time.google.com");
        
        struct tm timeinfo;
        bool isWorldInternet = false;
        
        // 時刻同期を待つ (最大5秒)
        if (getLocalTime(&timeinfo, 5000)) {
            Serial.println("World Internet");
            M5.Lcd.setTextColor(GREEN, BLACK);
            M5.Lcd.println("World Internet");
            M5.Lcd.setTextColor(WHITE, BLACK);
            isWorldInternet = true;
            g_isWorldInternet = true;
        } else {
            Serial.println("Local Internet Only");
            M5.Lcd.setTextColor(YELLOW, BLACK);
            M5.Lcd.println("Local Only");
            M5.Lcd.setTextColor(WHITE, BLACK);
            isWorldInternet = false;
            g_isWorldInternet = false;
        }
        
        // 読みやすさのため少し待機
        delay(1000);

        // 情報収集
        collectWSTT();
        wifi_links = true;
        // 成功表示のために画面クリア
        showStatus("Connected!", GREEN);
        manual_wifi = true;
        // 収集した情報を表示 (文字サイズを小さくして一覧表示)
        M5.Lcd.setTextSize(1);
        M5.Lcd.println("IP: " + WSTT[0]);
        M5.Lcd.println("GW: " + WSTT[1]);
        M5.Lcd.println("MAC: " + WSTT[2]);
        M5.Lcd.println("Auth: " + WSTT[6]);
        M5.Lcd.setTextSize(2); // サイズを戻す
        issta = true;
        // シリアルにも全情報を出力
        Serial.println("--- WSTT Info ---");
        for(int i=0; i<WSTT.size(); i++) {
            Serial.printf("[%d] %s\n", i, WSTT[i].c_str());
        }

        return true;
    } else {
        // タイムアウトまで待っても繋がらなかった場合の最終エラー診断
        wl_status_t finalStatus = WiFi.status();
        String statusName = getWiFiStatusName(finalStatus);
        
        String err = "[E03] Timeout or DHCP Failed. Status: " + statusName;
        Serial.println("\n" + err);
        UU = err;
        
        // エラー表示のために画面クリア
        showStatus("Timeout/DHCP Err\nStat: " + statusName, RED);
        return false;
    }
}



void monitorConnectionLoss() {
    static bool wasConnected = false;
    bool isConnected = checkWiFiConnection();

    // 以前までオンラインだった状態の上で接続遮断が行われた場合
    if (wasConnected && !isConnected && !manual_wifi) {
        Serial.println("\n[ALERT] WiFi Connection Lost physically!");
        UU = "[Log] Connection Lost";
        if(manual_wifi){
            manual_wifi = false;
        }


        // WebSocket閉鎖
        stopWebSocket();
        statustext = "NetStep:0,and disConnected by Router!";
        // M5のWiFi接続を閉じる
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        
        Serial.println("[System] WiFi & WebSocket Closed.");
    }

    // 状態更新
    wasConnected = isConnected;
}

String getWiFiStatusName(wl_status_t status) {
    switch (status) {
        case WL_IDLE_STATUS:     return "Idle";
        case WL_NO_SSID_AVAIL:   return "No SSID Available";
        case WL_SCAN_COMPLETED:  return "Scan Completed";
        case WL_CONNECTED:       return "Connected";
        case WL_CONNECT_FAILED:  return "Connect Failed";
        case WL_CONNECTION_LOST: return "Connection Lost";
        case WL_DISCONNECTED:    return "Disconnected";
        case WL_NO_SHIELD:       return "No WiFi Shield";
        default:                 return "Unknown Error (" + String(status) + ")";
    }
}


void removeSession(uint8_t num) {
    // クライアント番号からユーザーIDを特定
    if (clientLookup.count(num)) {
        String userId = clientLookup[num];
        
        // メインマップから削除
        if (sessionMap.count(userId)) {
            String details = sessionMap[userId];
            Serial.printf("[Session] Closed & Removed: ID=%s [%s]\n", userId.c_str(), details.c_str());
            sessionMap.erase(userId);
        }
        
        // 逆引きマップからも削除
        clientLookup.erase(num);
        
        Serial.printf("[Session] Current Active Users: %d\n", sessionMap.size());
    } else {
        // ID登録前に切断された場合など
        Serial.printf("[Session] Client [%u] disconnected (No ID registered)\n", num);
    }
}

/**
 * 新しいセッションをマップに登録する関数
 * "id:..." メッセージ受信時に呼び出されます
 */
void registerSession(uint8_t num, String userId) {
    // 以前のIDがあれば削除（ID変更のケース）
    if (clientLookup.count(num)) {
        String oldId = clientLookup[num];
        if (oldId != userId) {
            sessionMap.erase(oldId);
        }
    }

    // 逆引きマップを更新
    clientLookup[num] = userId;

    // 詳細情報の構築
    String ip = webSocket.remoteIP(num).toString();
    String timeStr = getTimestampStr();
    String deviceName = "Web Browser"; // 必要に応じて拡張可能
    
    // 値として詳細情報を文字列で結合
    String details = "IP:" + ip + ", ConnectAt:" + timeStr + ", Device:" + deviceName;

    // メインマップに登録
    sessionMap[userId] = details;
    
    Serial.printf("[Session] Registered: ID=%s\n", userId.c_str());
    Serial.printf("          Details: %s\n", details.c_str());
    Serial.printf("[Session] Current Active Users: %d\n", sessionMap.size());
}

/**
 * WebSocket イベントハンドラ
 */
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    
    
    
    
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WS] [%u] Disconnected!\n", num);
            removeSession(num); // 切断時にマップから削除
            break;

        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[WS] [%u] Connected from %s\n", num, ip.toString().c_str());
                webSocket.sendTXT(num, "Server Ready. Please send 'id:YOUR_ID'");
            }
            break;

        case WStype_TEXT:
            {
                String msg = String((char*)payload).substring(0, length);

                // RTC時刻取得
                auto dt = M5.Rtc.getDateTime();
                char timeBuffer[25];
                snprintf(timeBuffer, sizeof(timeBuffer), "%04d/%02d/%02d %02d:%02d:%02d",
                         dt.date.year, dt.date.month, dt.date.date,
                         dt.time.hours, dt.time.minutes, dt.time.seconds);
                String rtcTimeString = String(timeBuffer);

                if (msg.startsWith("id:")) {
                    String receivedId = msg.substring(3);
                    receivedId.trim();
                    if (receivedId.length() > 0) {
                        // ここで確実にマップに登録する
                        registerSession(num, receivedId); 
                        webSocket.sendTXT(num, "ID Registered: " + receivedId);
                        checkidandsave1(int(num));
                        checkidandsave2(int(num));
                    }
                } else {
                    // 通常メッセージ処理
                    if (msg.startsWith("dataload:")) {
                        // ロード命令をキューに入れる
                        sendToWorkerTask(TASK_DATA_LOAD, (int)num, msg.substring(9));
                    } 
                    else if (msg.startsWith("datasave:")) {
                        // セーブ命令とデータ本体をキューに入れる
                        Serial.println("saving;" + msg.substring(9));

                        
                        int commaIndex = msg.indexOf(',');
                        if (commaIndex != -1) {
                            String m1 = msg.substring(9, commaIndex);
                            String m2 = msg.substring(commaIndex + 1);
                            // 引数2つを渡す
                            sendToWorkerTask(TASK_DATA_SAVE, (int)num, m1, m2);
                        } else {
                            // カンマがない場合は全体をmsg1とするなどのフォールバック
                            sendToWorkerTask(TASK_DATA_SAVE, (int)num, msg.substring(9), "");
                        }



                        sendToWorkerTask(TASK_DATA_SAVE, (int)num, msg.substring(9));
                    }else if (msg.startsWith("ping:")) { // 追加
                        // Ping命令 (引数なし、ペイロード内容は不要なのでnumのみ渡す)
                        sendToWorkerTask(TASK_DATA_PING, (int)num);
                    }else if(msg.startsWith("test:")){
                        sendToWorkerTask(TASK_DATA_SAMPLE, (int)num);
                    }else if(msg.startsWith("list:")){
                        sendToWorkerTask(TASK_DATA_LIST, (int)num);
                    }else if(msg.startsWith("sendo:")){
                        String params = msg.substring(6);
                        int commaIndex = params.indexOf(',');
                        if (commaIndex != -1) {
                            String m1 = params.substring(0, commaIndex);
                            String m2 = params.substring(commaIndex + 1);
                            // 引数2つを渡す
                            sendToWorkerTask(TASK_DATA_SEND_OTHER, (int)num, m1, m2);
                        } else {
                            // カンマがない場合は全体をmsg1とするなどのフォールバック
                            sendToWorkerTask(TASK_DATA_SEND_OTHER, (int)num, params, "");
                        }
                    }else if(msg.startsWith("sendme:")){
                        String m1 = msg.substring(7);
                        sendToWorkerTask(TASK_DATA_SOME, (int)num,"bb", m1);
                    }else{

                     // ログ記録などはそのまま実行（またはこれもキュー化可能）
                    ReceiveWebM(num, msg, rtcTimeString);
                    }
                    
                    
                }
            }
            break;

        case WStype_ERROR:
            Serial.printf("[WS] [%u] Error!\n", num);
            break;

        default:
            break;
    }

    std::vector<String> idlist = getConnectedUserIds();
    SSListc = idlist.size();
}

void startWebSocket() {
  WiFi.setSleep(false);
    if (isWebSocketActive) return;

    webSocket.begin();
    webSocket.onEvent(onWebSocketEvent);
    
    isWebSocketActive = true;
    
    Serial.println("[WS] Server Started on Port 65500");
}

void stopWebSocket() {
    if (!isWebSocketActive) return;

    webSocket.close();
    isWebSocketActive = false;
    
    Serial.println("[WS] Server Stopped");
}

void handleWebSocketLoop() {
    if (isWebSocketActive) {
        webSocket.loop();
    }
}

bool checkWiFiConnection() {
    return (WiFi.status() == WL_CONNECTED);
}

void updateSessionDisplay() {
    static unsigned long lastUpdate = 0;
    // 1秒ごとに画面を更新（描画負荷軽減）
    if (millis() - lastUpdate < 1000) return;
    lastUpdate = millis();

    M5.Lcd.startWrite();
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(1); // フォントサイズを1に固定
    M5.Lcd.setTextColor(WHITE, BLACK);

    M5.Lcd.println("=== Active Sessions List ===");

    int count = 0;
    const int max_display = 20; // 1行が細いため、表示件数を確保

    // sessionMap (userId -> ipAddress) をループ
    // 以前のコードで表示されなかった不具合対策として、イテレータの参照を確実に処理
    for (auto it = sessionMap.begin(); it != sessionMap.end(); ++it) {
        if (count >= max_display) break;

        String userId = it->first;
        String ipAddress = it->second;
        
        // clientLookup (num -> userId) から逆引きして num を取得
        String clientNumStr = "??";
        for (auto const& [num, cUserId] : clientLookup) {
            if (cUserId == userId) {
                clientNumStr = String(num);
                break; 
            }
        }

        // 1行に収めるフォーマット
        // 例: 1.[#5] 192.168.1.5 ID:user123
        M5.Lcd.printf("%d.[#%s] %s ID:%s\n", 
                      count + 1, 
                      clientNumStr.c_str(), 
                      ipAddress.c_str(), 
                      userId.c_str());
        count++;
    }

    if (count == 0) {
        M5.Lcd.println("No Active Sessions.");
    }
    
    SessionSized = count;

    // 画面最下部に合計数を表示
    int bottomY = M5.Lcd.height() - 10;
    M5.Lcd.setCursor(0, bottomY);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.printf("Total: %d / MapSize: %d", count, (int)sessionMap.size());
    
    M5.Lcd.endWrite();
}


bool sendMessageByNum(String numStr, String message) {
    if (!isWebSocketActive) return false;

    // 二重送信防止 (デバウンス): 前回の送信から200ms以内は無視する
    static unsigned long lastSendTime = 0;
    if (millis() - lastSendTime < 200) {
        // Serial.println("[WS] Skipped duplicate send request.");
        return false;
    }
    lastSendTime = millis();

    // 入力文字列の整形
    numStr.trim();

    // --- 分岐処理 ---
    // 1. 全員へのブロードキャスト (Broadcast)
    if (numStr == "" || numStr.equalsIgnoreCase("all")) {
        Serial.println("[WS] Broadcasting message to ALL clients.");
        return webSocket.broadcastTXT(message);
    } 
    // 2. 特定クライアントへの送信 (Unicast)
    else {
        // 数値変換チェック
        // "0" 以外の文字列で toInt() が 0 になる場合は不正入力とみなす
        if (numStr != "0" && numStr.toInt() == 0) {
            Serial.printf("[WS] Invalid Client Num: %s\n", numStr.c_str());
            return false;
        }

        int num = numStr.toInt();
        
        // 有効なクライアント番号範囲かチェック
        if (num < 0 || num > 255) {
            Serial.printf("[WS] Client Num out of range: %d\n", num);
            return false;
        }

        Serial.printf("[WS] Sending unicast to client [%d]\n", num);
        return webSocket.sendTXT((uint8_t)num, message);
    }
}

/**
 * [受信関数] ブラウザからのメッセージを取り出す (loop内で使用)
 * @param content [出力] 受信したメッセージ内容
 * @param num [出力] 送信元のクライアント番号
 * @param userId [出力] 送信元のユーザーID (未登録なら"Unknown")
 * @return メッセージがあればtrue、なければfalse
 */
void ReceiveWebM(uint8_t num, String content,String RTCDate) {
    String userId = clientLookup[num];
    String gg = "num:" + String(num) + " id:" + userId + " ::RTC:" + RTCDate +  "\n log:" + content ;
    MailRList.push_back(gg);
}


int sessionSelectAndSendNonBlocking(String Txxxtsosin,String annnai) {
    static bool initialized = false;
    static int selection = 0;
    static bool needRedraw = true;

    if (!initialized) {
        M5.Lcd.fillScreen(BLACK);
        selection = 0;
        initialized = true;
        needRedraw = true;
    }

    // clientLookup (num) のサイズを基準にする
    int totalSessions = clientLookup.size();
    int maxItems = totalSessions + 2; 

    if (M5.BtnA.wasPressed()) {
        selection--;
        if (selection < 0) selection = maxItems - 1;
        needRedraw = true;
    }
    if (M5.BtnC.wasPressed()) {
        selection++;
        if (selection >= maxItems) selection = 0;
        needRedraw = true;
    }

    if (needRedraw) {
        M5.Lcd.startWrite();
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.println("Send Message:");
        M5.Lcd.setTextColor(CYAN, BLACK);
        String dispText = Txxxtsosin.length() > 20 ? Txxxtsosin.substring(0, 17) + "..." : Txxxtsosin;
        M5.Lcd.fillRect(0, 20, 320, 20, BLACK);
        M5.Lcd.setCursor(0, 20);
        M5.Lcd.println(dispText);
        
        M5.Lcd.drawFastHLine(0, 50, 320, WHITE);
        M5.Lcd.setCursor(0, 70);
        M5.Lcd.setTextColor(YELLOW, BLACK);
        M5.Lcd.print(annnai);

        String targetName = "";
        String detailInfo = "";
        
        // 選択肢の表示ロジック (clientLookup基準に変更)
        if (selection < totalSessions) {
            // clientLookup は std::map<uint8_t, String> なのでキー(num)の昇順に並んでいる
            auto it = clientLookup.begin();
            std::advance(it, selection);
            uint8_t num = it->first;
            String userId = it->second;
            
            targetName = "User: " + userId;
            
            // IPなどの詳細は sessionMap から引く
            String ip = "Unknown IP";
            if (sessionMap.count(userId)) {
                ip = sessionMap[userId];
            } else {
                // 登録直後などで sessionMap にまだない場合のフォールバック
                ip = webSocket.remoteIP(num).toString();
            }

            detailInfo = "IP:" + ip + " (#" + String(num) + ")";
        } else if (selection == totalSessions) {
            targetName = "ALL Users";
            detailInfo = "Broadcast to all";
        } else {
            targetName = "BACK";
            detailInfo = "Cancel & Return";
        }
        
        M5.Lcd.fillRect(0, 90, 320, 60, BLACK);
        M5.Lcd.setCursor(10, 90);
        M5.Lcd.println(targetName);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.setCursor(10, 120);
        M5.Lcd.println(detailInfo);
        
        M5.Lcd.setTextSize(2);
        int bottomY = M5.Lcd.height() - 40;
        M5.Lcd.fillRect(0, bottomY, 320, 40, BLACK);
        M5.Lcd.setCursor(120, bottomY);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.printf("< %d >", selection);
        M5.Lcd.endWrite();
        
        needRedraw = false;
    }

    if (M5.BtnB.wasPressed()) {
        int returnValue = -99;
        
        if (selection == totalSessions + 1) { // Back
            returnValue = -2;
        } else {
            bool sendResult = false;
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(10, 100);
            M5.Lcd.setTextSize(2);
            M5.Lcd.setTextColor(WHITE, BLACK);
            M5.Lcd.println("Sending...");

            if (selection == totalSessions) { // All
                sendResult = sendMessageByNum("", Txxxtsosin);
                returnValue = -1;
            } else { // Individual
                // 選択対象の num を直接取得
                auto it = clientLookup.begin();
                std::advance(it, selection);
                uint8_t num = it->first;
                
                // 直接 num を指定して送信
                sendResult = sendMessageByNum(String(num), Txxxtsosin);
                returnValue = selection;
            }
            
            if (sendResult) {
                M5.Lcd.setTextColor(GREEN, BLACK);
                M5.Lcd.println("Success!");
            } else {
                M5.Lcd.setTextColor(RED, BLACK);
                M5.Lcd.println("Failed!");
            }
            delay(500); 
        }

        M5.Lcd.fillScreen(BLACK);
        initialized = false;
        return returnValue;
    }

    return -99; 
}
int countWrappedLines(const char* text, int charsPerLine) {
    if (!text || text[0] == '\0') return 1;
    int len = strlen(text);
    if (len == 0) return 1;
    
    int lines = 0;
    int currentLen = 0;
    
    for (int i = 0; i < len; i++) {
        if (text[i] == '\n') {
            lines++;
            currentLen = 0;
        } else {
            currentLen++;
            if (currentLen >= charsPerLine) {
                lines++;
                currentLen = 0;
            }
        }
    }
    // 最後の行に文字が残っていれば行数を加算
    if (currentLen > 0 || lines == 0) lines++;
    
    return lines;
}

void updateMailDisplay2(const String& MailText) {
    static String g_lastMailText = "";
    static std::vector<String> g_mailListCache; // std::stringではなくArduino Stringを使用
    // ★追加: この関数専用の初回実行フラグ
    static bool isStringVerFirstRun = true;

    bool textChanged = false;

    // 1. 文字列の変更検知と分割
    if (MailText != g_lastMailText) {
        g_lastMailText = MailText;
        g_mailListCache.clear();
        textChanged = true;

        int len = MailText.length();
        int pos = 0;
        
        if (len > 0) {
            while (pos < len) {
                int nextPos = MailText.indexOf('\n', pos);
                if (nextPos == -1) {
                    // 最後の行
                    g_mailListCache.push_back(MailText.substring(pos));
                    break;
                } else {
                    // 改行まで
                    g_mailListCache.push_back(MailText.substring(pos, nextPos));
                    pos = nextPos + 1; // 次の文字へ
                }
            }
        }
    }

    // --- ここから独立した描画・スクロールロジック ---
    
    int listSize = (int)g_mailListCache.size();
    bool needsRedraw = false;

    // リスト変動時の処理
    if (textChanged) {
        // 初回実行時はメッセージを出さずに即座に表示
        if (isStringVerFirstRun) {
            isStringVerFirstRun = false; // フラグをオフ
            g_currentScrollY = 0;
            M5.Lcd.fillScreen(BLACK);
            needsRedraw = true;
        } 
        // 2回目以降の変動で、まだ更新メッセージを出していない場合
        else if (!g_isShowingUpdateMessage) {
            g_isShowingUpdateMessage = true;
            g_updateMessageStartTime = millis();

            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(10, 100);
            M5.Lcd.setTextSize(2);
            M5.Lcd.setTextColor(GREEN, BLACK);
            M5.Lcd.println("Mail Updated");
            return; 
        }
    }

    // 更新メッセージ表示中の待機
    if (g_isShowingUpdateMessage) {
        if (millis() - g_updateMessageStartTime < 1000) {
            return;
        }
        g_isShowingUpdateMessage = false;
        g_currentScrollY = 0;
        M5.Lcd.fillScreen(BLACK);
        needsRedraw = true;
    }

    // データなし
    if (listSize == 0) {
        if (needsRedraw) {
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.setTextSize(1);
            M5.Lcd.setTextColor(WHITE, BLACK);
            M5.Lcd.println("no logs.");
        }
        // 終了時にフラグを戻す
        isStringVerFirstRun = true;
        return;
    }

    // 画面計算
    int screenHeight = M5.Lcd.height();
    int screenWidth = M5.Lcd.width();
    int lineHeight = 8;
    int charWidth = 6;
    int charsPerLine = screenWidth / charWidth; 
    int maxRowsOnScreen = screenHeight / lineHeight;

    // 再描画が必要な場合、総行数を計算
    if (needsRedraw) {
        int targetCount = (listSize > 1000) ? 1000 : listSize;
        int startIndex = listSize - targetCount;
        
        g_totalWrappedLines = 0;
        for (int i = startIndex; i < listSize; i++) {
            g_totalWrappedLines += countWrappedLines(g_mailListCache[i].c_str(), charsPerLine);
        }
    }

    // スクロール処理
    int maxScroll = g_totalWrappedLines - maxRowsOnScreen;
    if (maxScroll < 0) maxScroll = 0;

    if (g_currentScrollY > maxScroll) {
        g_currentScrollY = maxScroll;
        needsRedraw = true;
    }

    if (M5.BtnA.wasPressed()) {
        if (g_currentScrollY > 0) {
            g_currentScrollY--;
            needsRedraw = true;
        }
    }
    if (M5.BtnC.wasPressed()) {
        if (g_currentScrollY < maxScroll) {
            g_currentScrollY++;
            needsRedraw = true;
        }
    }

    // 描画実行
    if (needsRedraw) {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(WHITE, BLACK);

        int targetCount = (listSize > 1000) ? 1000 : listSize;
        int startIndex = listSize - targetCount;
        int currentVirtualLine = 0; 
        bool stopDrawing = false;

        for (int i = startIndex; i < listSize; i++) {
            if (stopDrawing) break;

            String text = g_mailListCache[i]; // String型を使用
            int len = text.length();
            int pos = 0;

            while (pos < len || (len == 0 && pos == 0)) { 
                int nextNewLine = text.indexOf('\n', pos);
                int chunkLen;
                
                if (nextNewLine != -1 && (nextNewLine - pos) < charsPerLine) {
                    chunkLen = nextNewLine - pos;
                } else {
                    chunkLen = (len - pos) > charsPerLine ? charsPerLine : (len - pos);
                }
                
                String lineStr = text.substring(pos, pos + chunkLen);
                
                pos += chunkLen;
                if (pos < len && text[pos] == '\n') pos++;
                else if (chunkLen == 0 && len > 0) pos++;
                else if (len == 0) pos++;

                if (currentVirtualLine >= g_currentScrollY && 
                    currentVirtualLine < g_currentScrollY + maxRowsOnScreen) {
                    M5.Lcd.println(lineStr);
                }
                
                currentVirtualLine++;

                if (currentVirtualLine >= g_currentScrollY + maxRowsOnScreen) {
                    stopDrawing = true;
                    break;
                }
            }
        }
    }
    
    // ★実行終了時に初回フラグを立て直す
    // これにより、次回呼び出し時にテキスト変更があれば、再び「初回」として処理され、
    // 「Mail Updated」のメッセージが表示されずに即座に描画される
    isStringVerFirstRun = true;
}

void updateMailDisplay(const std::vector<String>& MailRList) {
    int listSize = (int)MailRList.size();
    bool needsRedraw = false;

    // --- 1. リスト変動の検知と更新メッセージ処理 ---
    // サイズが変わった場合、または初回起動(isshokai)の場合
    if (listSize != g_lastListSize || isshokai) {
        
        // 初回実行時: "Mail Updated" を出さずに即座に表示
        if (isshokai) {
            isshokai = false; // 初回フラグをオフ
            g_lastListSize = listSize;
            g_currentScrollY = 0;
            M5.Lcd.fillScreen(BLACK);
            needsRedraw = true; 
        } 
        // 2回目以降の変動: "Mail Updated" を表示して待機モードへ
        else if (!g_isShowingUpdateMessage) {
            g_lastListSize = listSize;
            g_isShowingUpdateMessage = true;
            g_updateMessageStartTime = millis();

            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(10, 100);
            M5.Lcd.setTextSize(2);
            M5.Lcd.setTextColor(GREEN, BLACK);
            M5.Lcd.println("Mail Updated");
            return; // 更新通知を表示して終了
        }
    }

    // --- 2. 更新メッセージ表示中の待機処理 ---
    if (g_isShowingUpdateMessage) {
        if (millis() - g_updateMessageStartTime < 1000) {
            return; // 待機中
        }
        // 1秒経過後、復帰
        g_isShowingUpdateMessage = false;
        g_currentScrollY = 0;
        M5.Lcd.fillScreen(BLACK);
        needsRedraw = true;
    }

    // --- 3. データがない場合の処理 ---
    if (listSize == 0) {
        if (needsRedraw) {
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.setTextSize(1);
            M5.Lcd.setTextColor(WHITE, BLACK);
            M5.Lcd.println("no logs.");
        }
        return;
    }

    // --- 4. 画面パラメータ計算 ---
    int screenHeight = M5.Lcd.height();
    int screenWidth = M5.Lcd.width();
    int lineHeight = 8; // フォントサイズ1の高さ
    int charWidth = 6;  // フォントサイズ1の文字幅(概算)
    int charsPerLine = screenWidth / charWidth; 
    int maxRowsOnScreen = screenHeight / lineHeight;

    // --- 5. 総行数の計算 (再描画が必要な場合のみ) ---
    if (needsRedraw) {
        int targetCount = (listSize > 1000) ? 1000 : listSize;
        int startIndex = listSize - targetCount;
        
        g_totalWrappedLines = 0;
        for (int i = startIndex; i < listSize; i++) {
            g_totalWrappedLines += countWrappedLines(MailRList[i].c_str(), charsPerLine);
        }
    }

    // --- 6. ボタン入力によるスクロール処理 ---
    int maxScroll = g_totalWrappedLines - maxRowsOnScreen;
    if (maxScroll < 0) maxScroll = 0;

    // 位置補正
    if (g_currentScrollY > maxScroll) {
        g_currentScrollY = maxScroll;
        needsRedraw = true;
    }

    if (M5.BtnA.wasPressed()) { // 上へ
        if (g_currentScrollY > 0) {
            g_currentScrollY--;
            needsRedraw = true;
        }
    }
    if (M5.BtnC.wasPressed()) { // 下へ
        if (g_currentScrollY < maxScroll) {
            g_currentScrollY++;
            needsRedraw = true;
        }
    }

    // --- 7. 描画実行 ---
    if (needsRedraw) {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(WHITE, BLACK);

        int targetCount = (listSize > 1000) ? 1000 : listSize;
        int startIndex = listSize - targetCount;
        int currentVirtualLine = 0; // 現在の仮想行カウント
        bool stopDrawing = false;   // 描画終了フラグ

        for (int i = startIndex; i < listSize; i++) {
            if (stopDrawing) break;

            String text = MailRList[i].c_str();
            int len = text.length();
            int pos = 0;

            // 要素を行単位に分割して処理
            while (pos < len || (len == 0 && pos == 0)) { 
                int nextNewLine = text.indexOf('\n', pos);
                int chunkLen;
                
                if (nextNewLine != -1 && (nextNewLine - pos) < charsPerLine) {
                    chunkLen = nextNewLine - pos;
                } else {
                    chunkLen = (len - pos) > charsPerLine ? charsPerLine : (len - pos);
                }
                
                String lineStr = text.substring(pos, pos + chunkLen);
                
                pos += chunkLen;
                if (pos < len && text[pos] == '\n') pos++;
                else if (chunkLen == 0 && len > 0) pos++;
                else if (len == 0) pos++;

                // 表示範囲内なら描画
                if (currentVirtualLine >= g_currentScrollY && 
                    currentVirtualLine < g_currentScrollY + maxRowsOnScreen) {
                    M5.Lcd.println(lineStr);
                }
                
                currentVirtualLine++;

                // 画面下端を超えたらループ脱出
                if (currentVirtualLine >= g_currentScrollY + maxRowsOnScreen) {
                    stopDrawing = true;
                    break;
                }
            }
        }
    }
}
bool appendUniqueLine(String kakikomitxt, String& kakikomisaki) {
    // 1. 入力チェック: 改行コードが含まれている、または空文字の場合はエラー
    if (kakikomitxt.indexOf('\n') != -1 || kakikomitxt.length() == 0) {
        return false;
    }

    // 2. kakikomisaki を一行単位で検索（重複チェック）
    int len = kakikomisaki.length();
    int pos = 0;
    
    while (pos < len) {
        int nextNewLine = kakikomisaki.indexOf('\n', pos);
        String line;
        
        if (nextNewLine == -1) {
            line = kakikomisaki.substring(pos);
            pos = len; // 終了
        } else {
            // 改行を除いて取得
            line = kakikomisaki.substring(pos, nextNewLine);
            pos = nextNewLine + 1; // 次の行へ
        }
        
        // 厳密な一致判定
        if (line == kakikomitxt) {
            return true; // 既に存在するので何もせず終了（成功扱い）
        }
    }

    // 3. 追加処理
    // 既存の文字列があり、かつ末尾が改行でない場合のみ改行を補完（空行回避）
    if (kakikomisaki.length() > 0 && !kakikomisaki.endsWith("\n")) {
        kakikomisaki += "\n";
    }
    
    // 文字列を追加し、末尾に改行を付与して「完全な行」として保存する
    // 空行（\n\n）が生成されないよう、必ずコンテンツの後に\nを置くルールで統一
    kakikomisaki += kakikomitxt + "\n";
    
    return true;
}

void checkidandsave1(int sendbynum){
    
   String useridd = clientLookup[sendbynum];
    String useridd2 = "txd1;" + useridd;
    if(useridd == ""){
        Serial.println("ユーザーidがまだセーブされていないエラー");
        return;
    }
    bool ss = false;
    bool tt = false;
    std::vector<MettVariableInfo> loadedVariablesE;
    loadMettFile(SD,"/save/save3.mett","testsus",tt,ss,loadedVariablesE);
    if(!tt){
        Serial.println("Hensuload Error!");
        return;
    }
   MettDataMap datatosaveEE = copyVectorToMap(loadedVariablesE);
    bool bb = !datt2(useridd2 + ";1",datatosaveEE) * !datt2(useridd2 + ";2",datatosaveEE) * !datt2(useridd2 + ";3",datatosaveEE)* !datt2(useridd2 + ";4",datatosaveEE);
    if(bb){
        Serial.println("正常終了：" + useridd +"のテストデータ保存済み" + useridd2);
        return;
    }

    saveMettFile(SD, "/save/save3.mett", "testsus", datatosaveEE, tt);
    if(tt){
        Serial.println("Hensusave Error!");
        return;
    }else{
        Serial.println("正常終了：" + useridd +"のテストデータ新規保存");
        return;
    } 
}

void checkidandsave2(int sendbynum){
    
    String useridd = clientLookup[sendbynum];
    
    if(useridd == ""){
        Serial.println("ユーザーidがまだセーブされていないエラー");
        return;
    }
    bool ss = false;
    bool tt = false;
    std::vector<MettVariableInfo> loadedVariablesE;
    loadMettFile(SD,"/save/save3.mett","testsus2",tt,ss,loadedVariablesE);
    if(!tt){
        Serial.println("Hensuload Error!");
        return;
    }
   MettDataMap AdataToSaveE = copyVectorToMap(loadedVariablesE);
    if(!datt2("txd2list",AdataToSaveE)){
        Serial.println(useridd +"のテストリスト捜索開始");
    
    }else{
       Serial.println(useridd +"のテストリストと全体リスト新規作成");
    }
    String sse = GyakuhenkanTxt(AdataToSaveE["txd2list"]);
    appendUniqueLine(";" + useridd,sse);
    Serial.println(appendUniqueLine(";" + useridd,sse));
    AdataToSaveE["txd2list"] = HenkanTxt(sse);
    saveMettFile(SD, "/save/save3.mett", "testsus2", AdataToSaveE, tt);
    if(tt){
    Serial.println("Hensusave Error!");
        return;
    }else{
        Serial.println("正常終了：" + useridd +"のテストリスト保存");
    }

    
}

int getClientNumByUserId(String userId) {
    for (auto const& [num, uid] : clientLookup) {
        if (uid == userId) {
            return (int)num;
        }
    }
    return -1;
}


void thedataload(int nummm,String id){
    if(id != "1" && id != "2" && id != "3" && id != "4"){
        sendMessageByNum(String(nummm), "load error:error code 03");
        return;
    }
    Serial.println("loadkaishi: id:" + clientLookup[nummm]);
    String useriidd = clientLookup[nummm];
    String useridd2 = "txd1;" + useriidd + ";" + id;
    if(useriidd == ""){
        Serial.println("ユーザーidがまだセーブされていないエラー");
        sendMessageByNum(String(nummm), "load error:error code 01");
        return;
    }
    bool ss = false;
    bool tt = false;
    std::vector<MettVariableInfo> loadedVariablesE;
    loadMettFile(SD,"/save/save3.mett","testsus",tt,ss,loadedVariablesE);
    MettDataMap dataToloadEE = copyVectorToMap(loadedVariablesE);
    if(!tt){
        Serial.println("Hensuload Error!");
        sendMessageByNum(String(nummm), "load error:error code 02");
        return;
    }
    if(dataToloadEE[useridd2] == ""){
        sendMessageByNum(String(nummm), "loaddata: !!EMPTY");
        return;
    }else{
        sendMessageByNum(String(nummm), "loaddata:" + GyakuhenkanTxt(dataToloadEE[useridd2]));
    }
}

void thedatasomething(int nummm,String isall,String textf){
    sendMessageByNum(String(nummm),textf);
}

void thedatasave(int nummm,String datasavecontent,String id){
    if(id != "1" && id != "2" && id != "3" && id != "4"){
        sendMessageByNum(String(nummm), "load error:error code 04");
        return;
    }
    Serial.println("loadkaishi: id:" + clientLookup[nummm]);
    String useriidd = clientLookup[nummm];
    String useridd2 = "txd1;" + useriidd + ";" + id;
    if(!isValidHensuValue(datasavecontent,false)){
        Serial.println("セーブする内容に禁止文字などのエラー");
        sendMessageByNum(String(nummm), "save error:error code 04");
        return;
    }
    if(useriidd == ""){

        Serial.println("ユーザーidがまだセーブされていないエラー");
        sendMessageByNum(String(nummm), "save error:error code 01");
        return;
    }
    bool ss = false;
    bool tt = false;
    std::vector<MettVariableInfo> loadedVariablesE;
    loadMettFile(SD,"/save/save3.mett","testsus",tt,ss,loadedVariablesE);
    MettDataMap dataToloadEE = copyVectorToMap(loadedVariablesE);
    if(!tt){
        Serial.println("Hensuload Error!");
        sendMessageByNum(String(nummm), "save error:error code 02");
        return;
    }
    
    dataToloadEE[useridd2] = HenkanTxt(datasavecontent);
    Serial.println("datasave content:" + useridd2 + " "+ datasavecontent);
    saveMettFile(SD, "/save/save3.mett", "testsus", dataToloadEE, tt);
    if(tt){
        sendMessageByNum(String(nummm), "save error:error code 03");
    }else{
        if(id != "4"){
            sendMessageByNum(String(nummm), "savedata:ok");
        }
        
        Serial.println("datasave completed!" + useridd2);
    }
}

void thedatasendother(int nummm,String user1,String user2){
    if(user1 == "" || user2 == ""){
        sendMessageByNum(String(nummm), "sendother error:error code 01");
        return;
    }
    String usersosinmot = clientLookup[nummm];
    if(usersosinmot == ""){
        sendMessageByNum(String(nummm), "sendother error:error code 02");
        return;
    }
    int numA = getClientNumByUserId(user2);
    if(numA == -1){
        sendMessageByNum(String(nummm), "sendother error:error code 03(no client)");
        return;
    }
    Serial.println("M5 usersented:from:" + usersosinmot + " to:" + user2 +  "\n body: " + user1);
    sendMessageByNum(String(numA), "M5:from:" + usersosinmot + "\n body: " + user1 );
    sendMessageByNum(String(nummm), "You Sent to:" + user2 + " :succeed" );
}
std::vector<String> getConnectedUserIds() {
    std::vector<String> idList;
    // sessionMapのキー(UserID)をすべて抽出してベクターに格納
    for (auto const& [userId, ip] : sessionMap) {
        idList.push_back(userId);
    }
    return idList;
}
void thedatasample(int nummm){
    sendMessageByNum(String(nummm), "Test message responded!!!!! id:" + clientLookup[nummm] + " san");
}
void thedataping(int nummm){
    sendMessageByNum(String(nummm), "M5:resp:" + clientLookup[nummm] );
    Serial.println("pingouto:" + clientLookup[nummm] );
}
void thedatalist(int nummm){

    

    std::vector<String> idlist = getConnectedUserIds();
    String liststr = "idlist of active:(count:" + String(idlist.size()) + "): ";
    for(const String& id : idlist){
        liststr += id + ", ";
    }
    sendMessageByNum(String(nummm).substring(0,String(nummm).length() - 2), liststr );
}


void backgroundProcessingTask(void *pvParameters) {
    TaskMessage msg;

    while (true) {
        // キューにデータが来るまで待機 (CPUを使わない)
        if (xQueueReceive(processingQueue, &msg, portMAX_DELAY) == pdTRUE) {
            
            Serial.printf("[Task] Processing request for Client %d (Type: %d)\n", msg.clientNum, msg.type);

            // 処理の分岐
            if (msg.type == TASK_DATA_LOAD) {
                thedataload(msg.clientNum,String(msg.dataPayload));
            } 
            else if (msg.type == TASK_DATA_SAVE) {
                if (msg.dataPayload != NULL) {
                    thedatasave(msg.clientNum, String(msg.dataPayload),String(msg.dataPayload2));
                }
            }
            else if (msg.type == TASK_DATA_SEND_OTHER) {
                // 2つのメッセージを復元して関数呼び出し
                String s1 = (msg.dataPayload != NULL) ? String(msg.dataPayload) : "";
                String s2 = (msg.dataPayload2 != NULL) ? String(msg.dataPayload2) : "";
                thedatasendother(msg.clientNum, s1, s2);
            }
            else if (msg.type == TASK_DATA_SAMPLE) {
                thedatasample(msg.clientNum);
            }
            else if (msg.type == TASK_DATA_PING) { // 追加
                thedataping(msg.clientNum);
            }else if (msg.type == TASK_DATA_LIST) { // 追加
                thedatalist(msg.clientNum);
            }else if (msg.type == TASK_DATA_SOME) { // 追加
                String s1 = (msg.dataPayload != NULL) ? String(msg.dataPayload) : "";
                String s2 = (msg.dataPayload2 != NULL) ? String(msg.dataPayload2) : "";
                thedatasomething(msg.clientNum, s1, s2);
            }

            // 確保したメモリを必ず解放する (重要)
            // どのタイプであっても、ポインタがNULLでなければ解放を試みる
            if (msg.dataPayload != NULL) {
                free(msg.dataPayload);
                msg.dataPayload = NULL;
            }
            if (msg.dataPayload2 != NULL) {
                free(msg.dataPayload2);
                msg.dataPayload2 = NULL;
            }

            // 処理後のウェイト（必要であれば）
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}

/**
 * [Helper] タスクキューへ命令を送る関数
 * 引数を拡張し、最大2つの文字列データを渡せるように変更
 */
void sendToWorkerTask(TaskType type, int num, String data1 , String data2) {
    TaskMessage msg;
    msg.type = type;
    msg.clientNum = num;
    msg.dataPayload = NULL;
    msg.dataPayload2 = NULL;

    // データがある場合はヒープメモリにコピーしてポインタを渡す
    if (data1.length() > 0) {
        msg.dataPayload = strdup(data1.c_str());
        if (msg.dataPayload == NULL) {
            Serial.println("[Error] Memory allocation failed for data1");
            return;
        }
    }
    
    if (data2.length() > 0) {
        msg.dataPayload2 = strdup(data2.c_str());
        if (msg.dataPayload2 == NULL) {
            Serial.println("[Error] Memory allocation failed for data2");
            // data1も解放して終了
            if (msg.dataPayload) free(msg.dataPayload);
            return;
        }
    }

    // キューへ送信 (満杯なら0ms待機して諦める=通信ブロック防止)
    if (xQueueSend(processingQueue, &msg, 0) != pdTRUE) {
        Serial.println("[Error] Queue is full, task dropped.");
        // 送信失敗時は確保したメモリを即座に解放
        if (msg.dataPayload != NULL) free(msg.dataPayload);
        if (msg.dataPayload2 != NULL) free(msg.dataPayload2);
    }
}


bool forceDisconnectClient(int nummm) {
    if (!isWebSocketActive) return false;

    String banMessage = "you will banned from m5";

    // 1. 全員切断 (-1)
    if (nummm == -1) {
        Serial.println("[WS] Disconnecting ALL clients...");
        // 全員に警告
        webSocket.broadcastTXT(banMessage);
        
        // 切断処理
        std::vector<uint8_t> targets;
        for (auto const& [num, id] : clientLookup) {
            targets.push_back(num);
        }
        for (uint8_t num : targets) {
            webSocket.disconnect(num);
        }
        return true;
    }

    // 2. 個別切断 (0-255)
    // 範囲チェック
    if (nummm < 0 || nummm > 255) {
        Serial.printf("[WS] Invalid disconnect target: %d\n", nummm);
        return false;
    }
    
    // 接続が存在するか確認（念のため）
    // WebSocketsServerには `clientIsConnected(num)` のようなAPIはないため、
    // clientLookupに存在するかどうかで判断するのが安全
    if (clientLookup.find((uint8_t)nummm) == clientLookup.end()) {
        Serial.printf("[WS] Client %d not found in registry. Skipping msg.\n", nummm);
        // 登録がなくても接続自体はあるかもしれないのでdisconnectは試みる
        webSocket.disconnect((uint8_t)nummm);
        return false;
    }

    Serial.printf("[WS] Force disconnecting client [%d]\n", nummm);
    
    // ★重要: broadcastではなくsendTXTで「その相手だけ」に送る
    webSocket.sendTXT((uint8_t)nummm, banMessage);
    delay(50); // 送信完了待ち
    webSocket.disconnect((uint8_t)nummm);
    
    return true;
}



void onWsEvent(
    AsyncWebSocket *server,
    AsyncWebSocketClient *client,
    AwsEventType type,
    void *arg,
    uint8_t *data,
    size_t len
) {
    switch (type) {
        case WS_EVT_CONNECT:
            M5_LOGI("WS client #%u connected from %s",
                client->id(),
                client->remoteIP().toString().c_str()
            );
            break;

        case WS_EVT_DISCONNECT:
            M5_LOGI("WS client #%u disconnected", client->id());
            break;

        case WS_EVT_DATA:
            // 必要ならここでコマンド処理
            break;

        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void startWebServer() {
    static bool started = false;
    // すでに正常に開始している場合は何もしない
    if (started) return;

    // Wi-FiのIPアドレスが確定しているか確認 (接続前のRefused防止)
    if (WiFi.localIP() == IPAddress(0,0,0,0) && WiFi.softAPIP() == IPAddress(0,0,0,0)) {
        // IPがまだ割り振られていない場合は、次のループでリトライさせるためstartedはfalseのまま
        return;
    }

    // SDカードの初期化確認
    // 起動直後は失敗することがあるため、失敗時はstartedをtrueにせず戻る
    if (!SD.begin(GPIO_NUM_4, SPI, 20000000)) {
        Serial.println("[HTTP] SD Mount Failed. Retrying later...");
        return;
    }

    // ファイルの存在確認
    if (!SD.exists("/data/index.html")) {
        Serial.println("[HTTP] Fatal: /data/index.html not found.");
        return;
    }

    // --- 接続拒否(Refused)対策 ---

    // 1. ブラウザに「通信が終わったらすぐに接続を閉じろ」と指示する
    // これを入れないと、iPhoneなどが古いTCP接続を維持し続け、新しい接続枠を奪います。
    DefaultHeaders::Instance().addHeader("Connection", "close");
    
    // 2. CORS許可 (外部からのアクセスによる拒否を防止)
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    // 通知用LED
    // turnOnBatteryLedGreen(); // 必要に応じて有効化
    turnOnLED(CRGB::Blue);

    // WebSocketの登録
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    // 静的ファイルの配信設定
    // setDefaultFile("index.html") により "/" アクセスで index.html を返します
    server.serveStatic("/", SD, "/data/")
          .setDefaultFile("index.html")
          .setCacheControl("max-age=3600"); // キャッシュを1時間に設定してリクエスト回数を削減

    // サーバー開始
    server.begin();
    
    started = true;
    Serial.println("[HTTP] Web Server Started (Optimized for Stability)");
    Serial.print("[HTTP] IP Address: ");
    Serial.println(WiFi.getMode() <= 2 ? WiFi.localIP() : WiFi.softAPIP());
}



void stopWebServer() {
    if (!isServerRunning) return;

    // WebSocketは維持する前提なので closeAll しない
    // ws.closeAll(); ← やらない

    server.end();
    isServerRunning = false;

    M5_LOGI("WebServer stopped (WebSocket kept alive)");
}


void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            Serial.println("[ETH] Started");
            ETH.setHostname("m5stack-server");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("[ETH] Cable Connected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.print("[ETH] Got IP: ");
            Serial.println(ETH.localIP());
            
            eth_connected = true;
            
            // IPアドレスを再取得・更新
            refreshServerIP();
            
            // LAN接続時にサーバーが動いていなければ起動
            startWebServer(); 
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("[ETH] Disconnected");
            eth_connected = false;
            refreshServerIP(); // 切断時もIP情報を更新
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("[ETH] Stopped");
            eth_connected = false;
            refreshServerIP();
            break;
        default: break;
    }
}


void startSoftAP(String ssid, String pass) {
    Serial.println("[Net] Starting SoftAP...");
    issta  = false;
    // WiFiをAPモードに
    WiFi.mode(WIFI_AP);

    // パスワードなし or あり
    bool ok;
    if (pass.length() == 0) {
        ok = WiFi.softAP(ssid.c_str());
    } else {
        ok = WiFi.softAP(ssid.c_str(), pass.c_str());
    }

    if (!ok) {
        Serial.println("[Error] SoftAP Start Failed");
        return;
    }

    // IP更新
    wifi_links = true;

    Serial.println("[Net] SoftAP Started!");
    Serial.print("[Net] SSID: "); Serial.println(ssid);
    Serial.print("[Net] AP IP Address: "); Serial.println(currentIPString);

    startWebServer();
    refreshServerIP();
}

void stopSoftAP() {
    WiFi.softAPdisconnect(true);
    Serial.println("[Net] SoftAP Stopped");
    refreshServerIP();
    wifi_links = false;
    issoftap = false;
    issta = false;
}

String refreshServerIP() {
    String newIP = "0.0.0.0";

    if ((WiFi.getMode() == WIFI_AP) || (WiFi.getMode() == WIFI_AP_STA)) {
        newIP = WiFi.softAPIP().toString();
    }
    else if (WiFi.status() == WL_CONNECTED) {
        newIP = WiFi.localIP().toString();
    }
    currentIPString = newIP;
    stationIPString = WiFi.localIP().toString();
    return currentIPString;
}


String scanAndGetSSIDList() {
    Serial.println("[WiFi] Starting Scan...");
    
    // WiFiがOFFの場合は一時的にSTAモードにする
    if (WiFi.getMode() == WIFI_OFF) {
        WiFi.mode(WIFI_STA);
        delay(100);
    }

    // スキャン実行 (async=false: 完了まで待機, show_hidden=true)
    int n = WiFi.scanNetworks(false, true);
    
    String ssidList = "";

    if (n == 0) {
        ssidList = "No networks found";
    } else {
        Serial.printf("[WiFi] %d networks found\n", n);
        for (int i = 0; i < n; ++i) {
            String ssid = WiFi.SSID(i);
            if (ssid.length() == 0) ssid = "<Hidden>";
            
            // SSID (RSSI) の形式で追加
            ssidList += ssid + " (" + String(WiFi.RSSI(i)) + "dBm)\n";
            delay(10); // WDトリガ防止の小休止
        }
    }
    
    // スキャン結果をメモリから削除
    WiFi.scanDelete();
    
    Serial.println("[WiFi] Scan done");
    return ssidList;
}