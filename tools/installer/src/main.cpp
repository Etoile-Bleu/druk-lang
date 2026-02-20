#include "installer.hpp"
#include "ui_window.hpp"
#include "logic_engine.hpp"
#include <thread>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show) {
    druk::installer::InstallerContext context;
    
    druk::installer::MainWindow window(instance, context);
    if (!window.create()) {
        return 1;
    }

    window.show(show);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}
