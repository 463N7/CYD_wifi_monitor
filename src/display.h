#pragma once
#include <LovyanGFX.hpp>

// CYD guess: SPI + ILI9341 (240x320). Adjust pins if needed.
class LGFX_CYD : public lgfx::LGFX_Device {
public:
  LGFX_CYD() 
  {
    { // --- SPI BUS config (v1: DC belongs here) ---
      auto cfg = _bus_instance.config();
      cfg.spi_host   = SPI2_HOST;   // VSPI
      cfg.spi_mode   = 0;
      cfg.freq_write = 40000000;    // 40 MHz (lower if unstable)
      cfg.freq_read  = 16000000;
      cfg.spi_3wire  = false;       // 4-wire (MISO present)
      cfg.use_lock   = true;

      cfg.pin_sclk   = 14;
      cfg.pin_mosi   = 13;
      cfg.pin_miso   = 12;
      cfg.pin_dc     = 2;          // <-- D/C (try 15; if no image, try 21 or 22)

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    { // --- PANEL config (ILI9341) ---
      auto cfg = _panel_instance.config();

      cfg.pin_cs   = 15;             
      cfg.pin_rst  = -1;             // (-1 if tied to EN)
      cfg.pin_busy = -1;            // not used

      cfg.memory_width   = 320; // swap height and width if screen is wrong (also adjust screen rotation under setup in main.cpp)
      cfg.memory_height  = 240; // swap height and width if screen is wrong (also adjust screen rotation under setup in main.cpp)
      cfg.panel_width    = 320; // swap height and width if screen is wrong (also adjust screen rotation under setup in main.cpp)
      cfg.panel_height   = 240; // swap height and width if screen is wrong (also adjust screen rotation under setup in main.cpp)
      cfg.offset_x       = 0;
      cfg.offset_y       = 0;
      cfg.offset_rotation= 0;
      cfg.dummy_read_pixel = 0;
      cfg.dummy_read_bits  = 0;
      cfg.readable       = true;
      cfg.invert         = false;
      cfg.rgb_order      = false;
      cfg.dlen_16bit     = false;
      cfg.bus_shared     = true;

      _panel_instance.config(cfg);
    }
    { // --- BACKLIGHT (PWM) ---
      auto cfg = _light_instance.config();
      cfg.pin_bl      = 21;         // try 32 → if dark, try 33 → if still dark, -1
      cfg.invert      = false;
      cfg.freq        = 44100;
      cfg.pwm_channel = 7;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }
    { // --- TOUCH: XPT2046 on its own SPI (VSPI) ---
      auto tcfg = _touch_instance.config();
      tcfg.spi_host   = SPI3_HOST;   // VSPI (separate from LCD HSPI)
      tcfg.freq       = 2000000;     // 1–2.5 MHz is safe
      tcfg.pin_sclk   = 25;          // CLK
      tcfg.pin_mosi   = 32;          // MOSI
      tcfg.pin_miso   = 39;          // MISO (input-only pin → perfect)
      tcfg.pin_cs     = 33;          // CS
      tcfg.pin_int    = 36;          // IRQ / PENIRQ
      tcfg.bus_shared = false;       // separate bus from the LCD
      _touch_instance.config(tcfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }

private:
  lgfx::Bus_SPI        _bus_instance;
//  lgfx::Panel_ST7789  _panel_instance;
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Light_PWM      _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;
};

// Global
extern LGFX_CYD lcd;
