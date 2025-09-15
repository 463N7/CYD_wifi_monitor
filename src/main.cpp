#include <Arduino.h>
#include <WiFi.h>
#include "display.h"

enum ViewMode { VIEW_ALL = 0, VIEW_SSID = 1 };
ViewMode gView = VIEW_ALL;

// cached scan results (2.4 GHz)
static const int CH_MIN = 1, CH_MAX = 11;
static int    gChCount[CH_MAX + 1] = {0};
static double gChWeight[CH_MAX + 1] = {0.0};
static int    gLastN = 0;

// cache SSID list so we can draw after scanDelete()
struct SsidItem { String ssid; int32_t rssi; int32_t ch; };
static SsidItem gSsidItems[64];
static int gSsidLen = 0;

// async scan state
static uint32_t gLastScanMs = 0;
static const uint32_t SCAN_INTERVAL_MS = 3000;
static bool gIsScanning = false;

// forward declare so we can call it from pollScanAndTally()
void renderCurrentView();

// kick off a background scan (returns immediately)
void startAsyncScan() 
{
  if (!gIsScanning) 
  {
    WiFi.scanNetworks(/*async=*/true, /*show_hidden=*/true);
    gIsScanning = true;
    gLastScanMs = millis();
  }
}

// called repeatedly; when a scan finishes, it tallies results
void pollScanAndTally() 
{
  int n = WiFi.scanComplete();         // -1 scanning, -2 idle, >=0 done

  if (n == -1) return;                 // still scanning
  if (n == -2) 
  {                       // idle: maybe start another
    if (!gIsScanning && millis() - gLastScanMs >= SCAN_INTERVAL_MS) 
    {
      WiFi.scanNetworks(true, true);   // start async scan
      gIsScanning = true;
      gLastScanMs = millis();
    }
    return;
  }

  // n >= 0 → scan finished: tally channels + cache SSIDs
  for (int ch = CH_MIN; ch <= CH_MAX; ++ch) 
  {
    gChCount[ch]  = 0;
    gChWeight[ch] = 0.0;
  }

  gSsidLen = 0;
  int m = (n > 64) ? 64 : n;

  for (int i = 0; i < n; ++i) 
  {
    int32_t rssi = WiFi.RSSI(i);
    int32_t chan = WiFi.channel(i);
    if (i < m) 
    {
      gSsidItems[i].ssid = WiFi.SSID(i);
      gSsidItems[i].rssi = rssi;
      gSsidItems[i].ch   = chan;
    }
    if (chan >= CH_MIN && chan <= CH_MAX) 
    {
      gChCount[chan] += 1;
      int w = 100 + (int)rssi; if (w < 0) w = 0;
      gChWeight[chan] += w;
    }
  }
  gSsidLen = m;
  gLastN   = n;

  WiFi.scanDelete();    // free buffers now that we cached data
  gIsScanning = false;

  // redraw immediately with fresh data
  renderCurrentView();
}

void drawBar(double value, double maxValue, int width = 40) 
{
  int n = 0;
  if (maxValue > 0) 
  {
    n = (int)((value / maxValue) * width + 0.5);
    if (n > width) n = width;
  }
  for (int i = 0; i < n; ++i) Serial.print('#');
  for (int i = n; i < width; ++i) Serial.print('-');
}

void drawSsidFeed(int n)
{
  Serial.printf("Found %d networks \n", n);
    for (int i = 0; i < n; i++)
    {
      String ssid = WiFi.SSID(i);
      int32_t rssi = WiFi.RSSI(i);
      int32_t chan  = WiFi.channel(i);
      wifi_auth_mode_t enc = (wifi_auth_mode_t)WiFi.encryptionType(i);

      Serial.printf("%2d) ch%-2ld  %-32s  RSSI:%4ld dBm  sec:%d\n", i+1, (long)chan, ssid.c_str(), (long)rssi, (int)enc); //ssid list
    }
}

void drawAllChannelsBars(const int* chCount, const double* chWeight, int chMin, int chMax, int width = 40) 
{
  // find the max weight for scaling
  double maxW = 0.0;
  for (int ch = chMin; ch <= chMax; ++ch) {
    if (chWeight[ch] > maxW) maxW = chWeight[ch];
  }

  Serial.println("\nChannel usage (2.4 GHz) — bars = RSSI-weighted congestion:");
  for (int ch = chMin; ch <= chMax; ++ch) {
    Serial.printf("  ch%-2d : %2d APs | weight:%7.1f | ", ch, chCount[ch], chWeight[ch]);
    drawBar(chWeight[ch], maxW, width);
    Serial.println();
  }
}

void drawAllChannelsLCD(LGFX_CYD& lcd, const int* chCount, const double* chWeight, int chMin, int chMax)
{
  // --- layout ---
  const int W = lcd.width();
  const int H = lcd.height();
  const int L = 6;    // left margin
  const int T = 25;   // top margin (leave room for title)
  const int R = 6;    // right margin
  const int rowH = 18;                 // row height per channel
  const int barW = W - L - R - 110;    // pixels reserved for bar (rest for labels)

  // find max for scaling
  double maxW = 0.0;
  for (int ch = chMin; ch <= chMax; ++ch) 
  {
    if (chWeight[ch] > maxW) maxW = chWeight[ch];
  }
  if (maxW <= 0) maxW = 1.0; // avoid divide by zero

  // clear and title
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.setTextSize(2);
  lcd.setCursor(L, 2);
  lcd.print("2.4 GHz: Congestion");

  // rows
  lcd.setTextSize(1);
  for (int ch = chMin; ch <= chMax; ++ch) 
  {
    int y = T + (ch - chMin) * rowH;

    // label: "chX  cnt:YY  wt:ZZZ"
    lcd.setCursor(L, y);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.printf("ch%-2d  cnt:%2d  wt:", ch, chCount[ch]);

    // bar
    int filled = (int)((chWeight[ch] / maxW) * barW + 0.5);
    if (filled < 0) filled = 0;
    if (filled > barW) filled = barW;

    int bx = L + 110;           // bar origin x
    int by = y - 2;             // small vertical padding
    int bh = rowH - 4;

    // background bar
    lcd.fillRect(bx, by, barW, bh, TFT_DARKGREY);
    // filled portion
    lcd.fillRect(bx, by, filled, bh, TFT_GREEN);
  }
}

// Draw strongest SSIDs on the LCD (top 12 by RSSI).
void drawSsidLCD(LGFX_CYD& lcd) 
{
  const int maxShow = 15;

  // build an index list [0..gSsidLen-1]
  int m = gSsidLen;
  if (m > 64) m = 64;
  int show = (m < maxShow) ? m : maxShow;
  int idx[64];
  for (int i = 0; i < m; ++i) idx[i] = i;

  // partial selection sort: top 'show' by RSSI desc
  for (int i = 0; i < show; ++i) 
  {
    int best = i;
    for (int j = i + 1; j < m; ++j) 
    {
      if (gSsidItems[idx[j]].rssi > gSsidItems[idx[best]].rssi) best = j;
    }
    if (best != i) { int t = idx[i]; idx[i] = idx[best]; idx[best] = t; }
  }

  // Draw
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.setTextSize(2);
  lcd.setCursor(6, 2);
  lcd.print("SSID Feed (top by RSSI)");

  lcd.setTextSize(1);
  int y = 24;
  for (int i = 0; i < show; ++i) 
  {
    const SsidItem& it = gSsidItems[idx[i]];
    // line: rank, ch, RSSI, SSID (trim long names)
    String name = it.ssid;
    if (name.length() > 22) name = name.substring(0, 22) + "...";

    lcd.setCursor(6, y);
    lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    lcd.printf("%2d) ch%-2ld  RSSI:%4ld  ", i + 1, (long)it.ch, (long)it.rssi);

    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.print(name);
    y += 14;
  }

  // footer hint
  lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  lcd.setCursor(6, lcd.height() - 12);
  lcd.print("Tap to toggle view");
}

void doScanAndTally() 
{
  int n = WiFi.scanNetworks(false, true);   // blocking is fine here
  // reset tallies
  for (int ch = CH_MIN; ch <= CH_MAX; ++ch) {
    gChCount[ch] = 0;
    gChWeight[ch] = 0.0;
  }
  if (n > 0) {
    for (int i = 0; i < n; ++i) {
      int32_t rssi = WiFi.RSSI(i);
      int32_t chan = WiFi.channel(i);
      if (chan >= CH_MIN && chan <= CH_MAX) {
        gChCount[chan] += 1;
        int w = 100 + (int)rssi; if (w < 0) w = 0;
        gChWeight[chan] += w;
      }
    }
  }
  gLastN = n;
  gLastScanMs = millis();
}

void renderCurrentView() 
{
  if (gView == VIEW_ALL) 
  {
    // (Serial optional)
    // drawAllChannelsBars(gChCount, gChWeight, CH_MIN, CH_MAX, 40);
    drawAllChannelsLCD(lcd, gChCount, gChWeight, CH_MIN, CH_MAX);
  } 
  else 
  { // VIEW_SSID
    // (Serial optional)
    // drawSsidFeed(gLastN);
    drawSsidLCD(lcd);
  }
}

void setup() 
{
  Serial.begin(115200);
  delay(300);
  WiFi.mode(WIFI_STA);     // station mode (don’t create an AP)
  WiFi.disconnect(true);   // ensure we’re not connected while scanning
  startAsyncScan();
  delay(200);

  // --- LCD smoke test ---
  lcd.init();            // power up panel
  lcd.setRotation(1);    // try 1 (landscape). Try 0/2/3 if orientation is weird.
  lcd.setColorDepth(16);
  lcd.setBrightness(200);// 0..255 (if BL pin is correct)

}

void loop() 
{
  // --- input: tap or 't' toggles immediately ---
  static bool wasDown = false;
  static uint32_t lastToggle = 0;
  int32_t tx, ty;
  bool down = lcd.getTouch(&tx, &ty);

  if (Serial.available() > 0) 
  {
    char c = Serial.read();
    if (c == 't' || c == 'T') 
    { 
      down = false; wasDown = true; 
    } // force a toggle path
  }
  uint32_t now = millis();
  if (wasDown && !down && (now - lastToggle) > 250) {
    gView = (gView == VIEW_ALL) ? VIEW_SSID : VIEW_ALL;
    lastToggle = now;
    renderCurrentView();              // <- redraw right away
  }
  wasDown = down;

  // --- periodic scan (non-blocking cadence) ---
  pollScanAndTally();

  delay(0);
}