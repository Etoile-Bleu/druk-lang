#pragma once

#include <windows.h>
#include <gdiplus.h>

namespace druk::installer {

struct Theme {
    Gdiplus::Color background;
    Gdiplus::Color surface;
    Gdiplus::Color text_primary;
    Gdiplus::Color text_secondary;
    Gdiplus::Color accent;
    Gdiplus::Color success;
    Gdiplus::Color error;
    Gdiplus::Color border;
    Gdiplus::Color sidebar_top;
    Gdiplus::Color sidebar_bottom;
    Gdiplus::Color link_blue;

    static Theme Dark() {
        return {
            Gdiplus::Color(255, 12, 12, 12),    // background (deeper black)
            Gdiplus::Color(255, 28, 28, 28),    // surface
            Gdiplus::Color(255, 255, 255, 255), // text_primary (pure white)
            Gdiplus::Color(255, 190, 190, 190), // text_secondary
            Gdiplus::Color(255, 0, 162, 255),   // accent (brighter blue)
            Gdiplus::Color(255, 0, 255, 128),   // success (vibrant green)
            Gdiplus::Color(255, 255, 48, 48),    // error (vibrant red)
            Gdiplus::Color(255, 64, 64, 64),     // border
            Gdiplus::Color(255, 45, 45, 45),     // sidebar_top
            Gdiplus::Color(255, 15, 15, 15),     // sidebar_bottom
            Gdiplus::Color(255, 100, 200, 255)   // link_blue
        };
    }

    static Theme Light() {
        return {
            Gdiplus::Color(255, 255, 255, 255), // background
            Gdiplus::Color(255, 240, 240, 240), // surface
            Gdiplus::Color(255, 0, 0, 0),       // text_primary (pure black)
            Gdiplus::Color(255, 80, 80, 80),    // text_secondary
            Gdiplus::Color(255, 0, 120, 215),   // accent
            Gdiplus::Color(255, 20, 160, 80),   // success
            Gdiplus::Color(255, 230, 0, 0),     // error
            Gdiplus::Color(255, 200, 200, 200), // border
            Gdiplus::Color(255, 235, 235, 235), // sidebar_top
            Gdiplus::Color(255, 215, 215, 215), // sidebar_bottom
            Gdiplus::Color(255, 0, 80, 200)     // link_blue
        };
    }
};

class ThemeEngine {
public:
    static ThemeEngine& instance() {
        static ThemeEngine engine;
        return engine;
    }

    const Theme& current() const { return current_theme_; }
    void set_dark(bool dark) { current_theme_ = dark ? Theme::Dark() : Theme::Light(); }

private:
    ThemeEngine() : current_theme_(Theme::Dark()) {}
    Theme current_theme_;
};

}
