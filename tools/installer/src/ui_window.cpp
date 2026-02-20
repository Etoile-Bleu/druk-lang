#include "ui_window.hpp"
#include "ui_theme.hpp"
#include "ui_wizard.hpp"
#include "logic_engine.hpp"
#include "util_logger.hpp"
#include <objidl.h>
#include <gdiplus.h>
#include <thread>

namespace druk::installer {

MainWindow::MainWindow(HINSTANCE instance, InstallerContext& context)
    : instance_(instance), context_(context) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplus_token_, &gdiplusStartupInput, nullptr);
}

MainWindow::~MainWindow() {
    Gdiplus::GdiplusShutdown(gdiplus_token_);
}

bool MainWindow::create() {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = window_proc;
    wc.hInstance = instance_;
    wc.lpszClassName = L"DrukInstallerMainWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // Let on_paint handle background

    if (!RegisterClassExW(&wc)) return false;

    // Center window
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    int x = (screen_w - width_) / 2;
    int y = (screen_h - height_) / 2;

    hwnd_ = CreateWindowExW(
        0, wc.lpszClassName, L"Druk Compiler Installer",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        x, y, width_, height_,
        nullptr, nullptr, instance_, this
    );

    return hwnd_ != nullptr;
}

void MainWindow::show(int cmd_show) {
    ShowWindow(hwnd_, cmd_show);
    UpdateWindow(hwnd_);
}

LRESULT CALLBACK MainWindow::window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    MainWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        self = reinterpret_cast<MainWindow*>(create->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) return self->handle_message(hwnd, msg, wparam, lparam);
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

LRESULT MainWindow::handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1; // Prevent flickering by not erasing background
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lparam);
            int y = HIWORD(lparam);
            Logger::instance().info(std::format("Mouse Click: {}, {}", x, y));
            
            if (context_.current_step == WizardStep::Welcome) {
                // Check "Install Now" region
                if (x > 180 && y > 90 && y < 150) {
                    Logger::instance().info("User clicked 'Install Now' / 'Repair Now'");
                    context_.current_step = WizardStep::Progress;
                    std::thread([this]() {
                        // Background UI refresher
                        bool is_running = true;
                        std::thread refresher([this, &is_running]() {
                            while (is_running) {
                                InvalidateRect(hwnd_, nullptr, TRUE);
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            }
                        });

                        InstallationEngine engine(context_);
                        bool success = (context_.mode == InstallMode::Uninstall) ? engine.uninstall() : engine.run();
                        
                        is_running = false;
                        if (refresher.joinable()) refresher.join();

                        if (success) {
                            context_.current_step = WizardStep::Completion;
                        } else {
                            context_.has_error = true;
                            context_.error_message = "Operation failed. See logs for details.";
                        }
                        InvalidateRect(hwnd_, nullptr, TRUE);
                    }).detach();
                } 
                // Check "Customize" region
                else if (x > 180 && y > 165 && y < 225) {
                    if (context_.mode == InstallMode::Install) {
                        Logger::instance().info("User clicked 'Customize'");
                        context_.current_step = WizardStep::Options;
                    }
                }
                // Check "Uninstall & Repair" region
                else if (x > 180 && y > 240 && y < 300) {
                    Logger::instance().info("User clicked 'Modify/Remove'");
                    context_.current_step = WizardStep::Maintenance;
                }
                // Check "Install for all users" checkbox region
                if (x > 180 && x < 500 && y > height_ - 135 && y < height_ - 105) {
                    // Logic for all users (not implemented in Engine yet, but we toggle the UI state)
                    Logger::instance().info("Toggle Install for all users");
                }
                // Check "Add to PATH" checkbox region
                else if (x > 180 && x < 500 && y > height_ - 105 && y < height_ - 75) {
                    context_.options.add_to_path = !context_.options.add_to_path;
                    Logger::instance().info(std::format("Toggle Add to PATH: {}", context_.options.add_to_path));
                }
            } else if (context_.current_step == WizardStep::Maintenance) {
                // Repair region
                if (x > 180 && y > 130 && y < 220) {
                    Logger::instance().info("User selected 'Repair'");
                    context_.mode = InstallMode::Repair;
                    context_.current_step = WizardStep::Progress;
                    std::thread([this]() {
                        bool is_running = true;
                        std::thread refresher([this, &is_running]() {
                            while (is_running) { InvalidateRect(hwnd_, nullptr, TRUE); std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
                        });
                        InstallationEngine engine(context_);
                        if (engine.run()) context_.current_step = WizardStep::Completion;
                        else { context_.has_error = true; context_.error_message = "Repair failed."; }
                        is_running = false; if (refresher.joinable()) refresher.join();
                        InvalidateRect(hwnd_, nullptr, TRUE);
                    }).detach();
                }
                // Remove region
                else if (x > 180 && y > 230 && y < 300) {
                    Logger::instance().info("User selected 'Remove'");
                    context_.mode = InstallMode::Uninstall;
                    context_.current_step = WizardStep::Progress;
                    std::thread([this]() {
                        bool is_running = true;
                        std::thread refresher([this, &is_running]() {
                            while (is_running) { InvalidateRect(hwnd_, nullptr, TRUE); std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
                        });
                        InstallationEngine engine(context_);
                        if (engine.uninstall()) context_.current_step = WizardStep::Completion;
                        else { context_.has_error = true; context_.error_message = "Removal failed."; }
                        is_running = false; if (refresher.joinable()) refresher.join();
                        InvalidateRect(hwnd_, nullptr, TRUE);
                    }).detach();
                }
            }
            
            // Navigation bar buttons
            if (y > height_ - 60) {
                if (x > width_ - 100) {
                    Logger::instance().info("User clicked 'Cancel' or 'Close'");
                    PostQuitMessage(0);
                } else if (x > width_ - 190 && x < width_ - 110) {
                    if (context_.current_step == WizardStep::Options || context_.current_step == WizardStep::Maintenance) {
                        Logger::instance().info("User clicked 'Back'");
                        context_.current_step = WizardStep::Welcome;
                    }
                 }
            }
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        case WM_PAINT:
            on_paint();
            return 0;
        case WM_SIZE:
            on_size(LOWORD(lparam), HIWORD(lparam));
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

void MainWindow::on_paint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd_, &ps);
    
    // Double buffering
    HDC mem_dc = CreateCompatibleDC(hdc);
    HBITMAP mem_bitmap = CreateCompatibleBitmap(hdc, width_, height_);
    HBITMAP old_bitmap = (HBITMAP)SelectObject(mem_dc, mem_bitmap);

    {
        Gdiplus::Graphics graphics(mem_dc);
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

        auto& theme = ThemeEngine::instance().current();
        graphics.Clear(theme.background);

        WizardRenderer::render(graphics, context_, width_, height_);
    }

    BitBlt(hdc, 0, 0, width_, height_, mem_dc, 0, 0, SRCCOPY);

    SelectObject(mem_dc, old_bitmap);
    DeleteObject(mem_bitmap);
    DeleteDC(mem_dc);
    
    EndPaint(hwnd_, &ps);
}

void MainWindow::on_size(int width, int height) {
    width_ = width;
    height_ = height;
    InvalidateRect(hwnd_, nullptr, TRUE);
}

}
