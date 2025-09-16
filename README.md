# 📡 CYD Wi-Fi Air Monitor

A simple Wi-Fi congestion monitor for the **Cheap Yellow Display (CYD)**  
(ESP32 + 2.8" ILI9341 TFT + XPT2046 touchscreen).  
 
It scans 2.4 GHz networks and visualizes channel usage as bar graphs,  
or lists the strongest SSIDs. Tap the screen to switch views instantly.  

---

## ✨ Features
- 📶 Async Wi-Fi scanning (non-blocking, smooth UI)  
- 📊 Weighted channel bar graph (visual congestion map)  
- 📜 SSID feed (top networks by RSSI)  
- 👆 Tap screen to toggle views instantly  
- ⚡ Powered by [LovyanGFX](https://github.com/lovyan03/LovyanGFX)  

---

## 🛠 Hardware
- **Cheap Yellow Display (CYD)**  
  - ESP32-WROOM  
  - 2.8" ILI9341 TFT (240x320)  
  - XPT2046 touchscreen
- Micro-USB 

### Pinout Reference
| Function     | GPIO |
|--------------|------|
| TFT_MISO     | 12   |
| TFT_MOSI     | 13   |
| TFT_SCLK     | 14   |
| TFT_CS       | 15   |
| TFT_DC       | 2    |
| TFT_RST      | -1 (EN) |
| TFT_BL       | 21   |
| TOUCH_MISO   | 39   |
| TOUCH_MOSI   | 32   |
| TOUCH_SCLK   | 25   |
| TOUCH_CS     | 33   |
| TOUCH_IRQ    | 36   |

---

## 🔧 Easy Flash Using webflasher

### 1. Download 3 bins from the releace page (partitions.bin, bootloader.bin and CYD_wifi_monitor_v*.bin

### 2. Go to the [ESP WEB FLASHER](https://espressif.github.io/esptool-js/)

### 3. Set the flash addresses as follows
- 0x1000 bootloader.bin
- 0x8000 partitions.bin
- 0x10000 CYD_wifi_monitor_v*.bin

### 4. Press Program

### 5. Restart your CYD (unplug and plug back in)

<img width="1051" height="604" alt="image" src="https://github.com/user-attachments/assets/146786dc-f1d2-4ec4-854b-f773a98f43e6" />


---

## 🔧 Build From Source (Dev Setup)

### 1. Clone this repo
```bash
git clone https://github.com/yourusername/cyd-wifi-air-monitor.git
cd cyd-wifi-air-monitor
```
### 2. Install PlatformIO
- Install [Visual Studio Code](https://code.visualstudio.com/)  
- Add the [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)  

### 3. Build & Flash
1. Connect CYD with USB  
2. In VS Code / PlatformIO:  
   - Select **ESP32 Dev Module** as board  
   - Press **Upload** (PlatformIO toolbar ✔️→▶️ button)  

---

## ▶️ Usage
- The display boots into **Channel Overview** (bar graph).  
- Tap the screen to switch to **SSID Feed** (top 12 strongest networks).  
- Scans run automatically every few seconds and update results live.  

---

## 📸 Preview
[![Watch Demo on YouTube](https://img.youtube.com/vi/jSBA8VyDLIY/0.jpg)](https://youtube.com/shorts/jSBA8VyDLIY)
 

---

## 📜 License
MIT — free to use, modify, and share.
