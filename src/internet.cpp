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
String UU;
String TexNet = "  S0:GETWIFI\n  S1:LANPORT\n  S2:WEBSOCKET\n  S3:SESSIONS\n  S4:DISCONNECT\n  S5:DO_AUTO";
int IntNet = 6;
bool manual_wifi = false;
bool g_isWorldInternet;
std::vector<String> WSTT; // 詳細情報を格納するベクター


WebSocketsServer webSocket = WebSocketsServer(81);
bool isWebSocketActive = false; // WebSocketサーバーの稼働状態フラグ
// セッションリスト (接続中のユーザーを管理)

// セッション管理用マップ
// 1. メイン管理マップ: Key=ユーザーID, Value=詳細情報(IP, 時刻, デバイス名など)
std::map<String, String> sessionMap;

// 2. 逆引き用マップ: Key=接続番号(num), Value=ユーザーID
// ※ 切断イベント(numしか分からない)時に、素早くユーザーIDを特定して削除するために使用
std::map<uint8_t, String> clientLookup;


String getTimestampStr() {
    // もしNTP同期していれば getLocalTime を使えますが、ここでは稼働時間を使用
    return String(millis() / 1000) + "s";
}

String TexNet1(MettDataMap mmmc){
    return "  SSID:" + GyakuhenkanTxt(mmmc["table_SSID"]) + "\n  Username:" + GyakuhenkanTxt(mmmc["table_Usrname"] )+ "\n  Password:" + GyakuhenkanTxt(mmmc["table_Pass"]) + "\n  Login\n  Wifi Status\n";
}
int IntNet1 = 5;
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

        if(datt("table_SSID","")){
          jj = true;
        }
        if(datt("table_Usrname","")){
          jj = true;
        }
        if(datt("table_Pass","")){
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


void sessionMonitorTask(void *pvParameters) {
    
}

void disconnectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nDisconnecting WiFi...");
        WiFi.disconnect(true); // true = WiFiオフにする
        WiFi.mode(WIFI_OFF);
        delay(100);
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
        } else {
            Serial.println("Local Internet Only");
            M5.Lcd.setTextColor(YELLOW, BLACK);
            M5.Lcd.println("Local Only");
            M5.Lcd.setTextColor(WHITE, BLACK);
            isWorldInternet = false;
        }
        
        // 読みやすさのため少し待機
        delay(1000);

        // 情報収集
        collectWSTT();

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
            // 切断時: マップから削除処理を実行
            Serial.printf("[WS] [%u] Disconnected!\n", num);
            removeSession(num);
            break;

        case WStype_CONNECTED:
            {
                // 接続直後の処理
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[WS] [%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
                webSocket.sendTXT(num, "Server Ready. Please send 'id:YOUR_ID'");
            }
            break;

        case WStype_TEXT:
            {
                String msg = String((char*)payload);
                // "id:" から始まるメッセージを抽出してセッション登録
                if (msg.startsWith("id:")) {
                    String receivedId = msg.substring(3); // "id:" の後の文字列を取得
                    receivedId.trim();
                    
                    if (receivedId.length() > 0) {
                        // セッション登録関数を呼び出し
                        registerSession(num, receivedId);
                        // 応答
                        webSocket.sendTXT(num, "ID Registered: " + receivedId);
                    }
                } else {
                    // それ以外のテキストメッセージ
                    Serial.printf("[WS] [%u] Text: %s\n", num, payload);
                }
            }
            break;

        case WStype_ERROR:
            Serial.printf("[WS] [%u] Error!\n", num);
            break;
            
        case WStype_BIN:
            break;
            
        case WStype_PONG:
        case WStype_PING:
            break;
    }
}

void startWebSocket() {
    if (WiFi.status() != WL_CONNECTED) return;
    if (isWebSocketActive) return;

    webSocket.begin();
    webSocket.onEvent(onWebSocketEvent);
    isWebSocketActive = true;
    
    Serial.println("[WS] Server Started on Port 81");
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
    // 1秒ごとに画面を更新（頻繁な描画によるチラつき防止）
    if (millis() - lastUpdate < 1000) return;
    lastUpdate = millis();

    M5.Lcd.startWrite(); // 描画開始
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(WHITE, BLACK);

    M5.Lcd.println("=== Active Sessions List ===");

    int count = 0;
    const int max_display = 20;

    // sessionMap (userId -> ipAddress) をループ
    for (auto const& [userId, ipAddress] : sessionMap) {
        if (count >= max_display) break;

        // clientLookup (num -> userId) から逆引きして num を取得
        String clientNumStr = "??"; // 見つからない場合のデフォルト
        for (auto const& [num, cUserId] : clientLookup) {
            if (cUserId == userId) {
                clientNumStr = String(num);
                break; 
            }
        }

        // 1行に収まるようにフォーマットして表示
        // 例: 1. [#5] IP:192.168.1.5 (ID:user123)
        M5.Lcd.printf("%d. [#%s] %s (ID:%s)\n", 
                      count + 1, 
                      clientNumStr.c_str(), 
                      ipAddress.c_str(), 
                      userId.c_str());
        count++;
    }

    if (count == 0) {
        M5.Lcd.println("No Active Sessions.");
    }

    // 画面最下部に合計数を表示
    int bottomY = M5.Lcd.height() - 15;
    M5.Lcd.setCursor(0, bottomY);
    M5.Lcd.setTextColor(YELLOW, BLACK); // 合計数は黄色で強調
    M5.Lcd.printf("Total Sessions: %d", sessionMap.size());
    
    M5.Lcd.endWrite(); // 描画終了
}