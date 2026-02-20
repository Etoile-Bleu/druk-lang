#pragma once

#include <windows.h>
#include <gdiplus.h>
#include "installer.hpp"

namespace druk::installer {

class MainWindow {
public:
    MainWindow(HINSTANCE instance, InstallerContext& context);
    ~MainWindow();

    bool create();
    void show(int cmd_show);

private:
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    void on_paint();
    void on_size(int width, int height);

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;
    InstallerContext& context_;
    ULONG_PTR gdiplus_token_;
    
    int width_ = 600;
    int height_ = 520;
};

}
