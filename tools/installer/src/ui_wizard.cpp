#include "ui_wizard.hpp"
#include "ui_theme.hpp"
#include "util_logger.hpp"

namespace druk::installer {

void WizardRenderer::render(Gdiplus::Graphics& graphics, const InstallerContext& context, int width, int height) {
    auto& theme = ThemeEngine::instance().current();
    
    // 1. Sidebar Rendering
    float sidebar_width = 160.0f;
    Gdiplus::RectF sidebar_rect(0, 0, sidebar_width, (float)height - 60);
    Gdiplus::LinearGradientBrush sidebar_brush(
        sidebar_rect, theme.sidebar_top, theme.sidebar_bottom, 
        Gdiplus::LinearGradientModeVertical);
    graphics.FillRectangle(&sidebar_brush, sidebar_rect);

    Gdiplus::RectF content_rect(sidebar_width + 20.0f, 20.0f, (float)width - sidebar_width - 40, (float)height - 100);
    
    switch (context.current_step) {
        case WizardStep::Welcome:    render_welcome(graphics, context, content_rect); break;
        case WizardStep::Maintenance: render_maintenance(graphics, context, content_rect); break;
        case WizardStep::Options:    render_options(graphics, context, content_rect); break;
        case WizardStep::Progress:   render_progress(graphics, context, content_rect); break;
        case WizardStep::Completion: render_completion(graphics, context, content_rect); break;
    }

    // 2. Navigation Bar
    Gdiplus::RectF nav_rect(0.0f, (float)height - 60, (float)width, 60.0f);
    Gdiplus::SolidBrush nav_brush(theme.surface);
    graphics.FillRectangle(&nav_brush, nav_rect);
    Gdiplus::Pen border_pen(theme.border);
    graphics.DrawLine(&border_pen, 0.0f, (float)height - 60.0f, (float)width, (float)height - 60.0f);

    Gdiplus::Font btn_font(L"Segoe UI", 10);
    Gdiplus::SolidBrush text_brush(theme.text_primary);
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    // [Cancel] / [Close] Button (Right aligned)
    Gdiplus::RectF cancel_btn((float)width - 100, (float)height - 45, 80.0f, 30.0f);
    graphics.DrawRectangle(&border_pen, cancel_btn);
    std::wstring cancel_text = (context.current_step == WizardStep::Completion) ? L"Close" : L"Cancel";
    graphics.DrawString(cancel_text.c_str(), -1, &btn_font, cancel_btn, &format, &text_brush);

    // [Back] Button
    if (context.current_step == WizardStep::Options || context.current_step == WizardStep::Maintenance) {
        Gdiplus::RectF back_btn((float)width - 190, (float)height - 45, 80.0f, 30.0f);
        graphics.DrawRectangle(&border_pen, back_btn);
        graphics.DrawString(L"< Back", -1, &btn_font, back_btn, &format, &text_brush);
    }
}

void WizardRenderer::render_welcome(Gdiplus::Graphics& graphics, const InstallerContext& context, const Gdiplus::RectF& rect) {
    auto& theme = ThemeEngine::instance().current();
    Gdiplus::SolidBrush text_brush(theme.text_primary);
    Gdiplus::SolidBrush text_sec_brush(theme.text_secondary);
    Gdiplus::SolidBrush link_brush(theme.link_blue);
    
    Gdiplus::Font title_font(L"Segoe UI", 18);
    Gdiplus::Font sub_font(L"Segoe UI", 10);
    Gdiplus::Font action_font(L"Segoe UI", 12);
    Gdiplus::Font emoji_font(L"Segoe UI Emoji", 24);

    graphics.DrawString(L"Install Druk 1.0.0 (64-bit)", -1, &title_font, {rect.X, rect.Y, rect.Width, 40}, nullptr, &text_brush);
    graphics.DrawString(L"Select Install Now to install Druk with default settings,\nor choose Customize to enable or disable features.", 
        -1, &sub_font, {rect.X, rect.Y + 45, rect.Width, 40}, nullptr, &text_brush);

    // "Install Now" Action
    float action_y = rect.Y + 70;
    graphics.DrawString(L"\x25B8", -1, &emoji_font, {rect.X, action_y, 40, 40}, nullptr, &text_brush);
    graphics.DrawString(L"Install Now", -1, &action_font, {rect.X + 45, action_y, 200, 25}, nullptr, &link_brush);
    graphics.DrawString(context.options.target_dir.c_str(), -1, &sub_font, {rect.X + 45, action_y + 25, rect.Width - 45, 20}, nullptr, &text_sec_brush);
    graphics.DrawString(L"Includes REPL, standard library and documentation\nAdds Druk to PATH and creates shortcuts", 
        -1, &sub_font, {rect.X + 45, action_y + 45, rect.Width - 45, 40}, nullptr, &text_sec_brush);

    // "Customize" Action
    action_y += 85;
    graphics.DrawString(L"\x25B8", -1, &emoji_font, {rect.X, action_y, 40, 40}, nullptr, &text_brush);
    graphics.DrawString(L"Customize installation", -1, &action_font, {rect.X + 45, action_y, 200, 25}, nullptr, &link_brush);
    graphics.DrawString(L"Choose location and features", -1, &sub_font, {rect.X + 45, action_y + 25, rect.Width - 45, 20}, nullptr, &text_sec_brush);

    // "Uninstall & Repair" Action
    action_y += 75;
    graphics.DrawString(L"\x25B8", -1, &emoji_font, {rect.X, action_y, 40, 40}, nullptr, &text_brush);
    graphics.DrawString(L"Uninstall & Repair", -1, &action_font, {rect.X + 45, action_y, 200, 25}, nullptr, &link_brush);
    graphics.DrawString(L"Modify or remove an existing installation", -1, &sub_font, {rect.X + 45, action_y + 25, rect.Width - 45, 20}, nullptr, &text_sec_brush);

    // Bottom Options
    float bottom_y = rect.Y + rect.Height - 50;
    Gdiplus::Pen check_pen(theme.text_primary, 1.5f);
    
    // Checkbox 1: All users
    Gdiplus::RectF check1(rect.X, bottom_y + 5, 14, 14);
    graphics.DrawRectangle(&check_pen, check1);
    graphics.DrawString(L"Install launcher for all users (recommended)", -1, &sub_font, {rect.X + 25, bottom_y, rect.Width - 25, 20}, nullptr, &text_brush);
    // Draw mark
    graphics.DrawLine(&check_pen, check1.X + 3, check1.Y + 7, check1.X + 6, check1.Y + 11);
    graphics.DrawLine(&check_pen, check1.X + 6, check1.Y + 11, check1.X + 11, check1.Y + 3);

    // Checkbox 2: PATH
    bottom_y += 25;
    Gdiplus::RectF check2(rect.X, bottom_y + 5, 14, 14);
    graphics.DrawRectangle(&check_pen, check2);
    graphics.DrawString(L"Add Druk 1.0.0 to PATH", -1, &sub_font, {rect.X + 25, bottom_y, rect.Width - 25, 20}, nullptr, &text_brush);
    if (context.options.add_to_path) {
        graphics.DrawLine(&check_pen, check2.X + 3, check2.Y + 7, check2.X + 6, check2.Y + 11);
        graphics.DrawLine(&check_pen, check2.X + 6, check2.Y + 11, check2.X + 11, check2.Y + 3);
    }
}

void WizardRenderer::render_maintenance(Gdiplus::Graphics& graphics, const InstallerContext& context, const Gdiplus::RectF& rect) {
    auto& theme = ThemeEngine::instance().current();
    Gdiplus::SolidBrush text_brush(theme.text_primary);
    Gdiplus::SolidBrush text_sec_brush(theme.text_secondary);
    Gdiplus::SolidBrush link_brush(theme.link_blue);
    
    Gdiplus::Font title_font(L"Segoe UI", 18);
    Gdiplus::Font sub_font(L"Segoe UI", 10);
    Gdiplus::Font action_font(L"Segoe UI", 12);
    Gdiplus::Font emoji_font(L"Segoe UI Emoji", 24);

    graphics.DrawString(L"Modify Druk Installation", -1, &title_font, {rect.X, rect.Y, rect.Width, 40}, nullptr, &text_brush);
    graphics.DrawString(L"Select an option to modify or remove the Druk Compiler.", 
        -1, &sub_font, {rect.X, rect.Y + 45, rect.Width, 40}, nullptr, &text_brush);

    // "Repair" Action
    float action_y = rect.Y + 110;
    graphics.DrawString(L"\x25B8", -1, &emoji_font, {rect.X, action_y, 40, 40}, nullptr, &text_brush);
    graphics.DrawString(L"Repair", -1, &action_font, {rect.X + 45, action_y, 200, 25}, nullptr, &link_brush);
    graphics.DrawString(L"Fix issues with the current installation by re-installing all components.", 
        -1, &sub_font, {rect.X + 45, action_y + 25, rect.Width - 45, 40}, nullptr, &text_sec_brush);

    // "Remove" Action
    action_y += 100;
    graphics.DrawString(L"\x25B8", -1, &emoji_font, {rect.X, action_y, 40, 40}, nullptr, &text_brush);
    graphics.DrawString(L"Remove", -1, &action_font, {rect.X + 45, action_y, 200, 25}, nullptr, &link_brush);
    graphics.DrawString(L"Completely remove Druk Compiler from your computer.", 
        -1, &sub_font, {rect.X + 45, action_y + 25, rect.Width - 45, 20}, nullptr, &text_sec_brush);
}



void WizardRenderer::render_options(Gdiplus::Graphics& graphics, const InstallerContext& context, const Gdiplus::RectF& rect) {
    auto& theme = ThemeEngine::instance().current();
    Gdiplus::SolidBrush text_brush(theme.text_primary);
    Gdiplus::SolidBrush text_sec_brush(theme.text_secondary);
    Gdiplus::Font title_font(L"Segoe UI", 12, Gdiplus::FontStyleBold);
    Gdiplus::Font body_font(L"Segoe UI", 10);
    Gdiplus::Font icon_font(L"Segoe UI Emoji", 10);

    graphics.DrawString(L"⚙️ Installation Options", -1, &title_font, {rect.X, rect.Y, rect.Width, 30}, nullptr, &text_brush);
    
    graphics.DrawString(L"Installation Directory:", -1, &body_font, {rect.X, rect.Y + 40, rect.Width, 20}, nullptr, &text_brush);
    graphics.DrawString(context.options.target_dir.c_str(), -1, &body_font, {rect.X + 10, rect.Y + 60, rect.Width - 20, 25}, nullptr, &text_sec_brush);

    float item_y = rect.Y + 100;
    auto draw_check = [&](const wchar_t* label, bool checked) {
        graphics.DrawString(checked ? L"☑" : L"☐", -1, &icon_font, {rect.X, item_y, 20, 20}, nullptr, &text_brush);
        graphics.DrawString(label, -1, &body_font, {rect.X + 25, item_y, rect.Width - 25, 20}, nullptr, &text_brush);
        item_y += 25;
    };

    draw_check(L"Druk Compiler (Required)", true);
    draw_check(L"LLVM Runtime Libraries", context.options.install_llvm);
    draw_check(L"Standard Library", context.options.install_stdlib);
    draw_check(L"Documentation", context.options.install_docs);
    draw_check(L"Add to PATH", context.options.add_to_path);
    draw_check(L"Create desktop shortcut", context.options.create_shortcut);
}

void WizardRenderer::render_progress(Gdiplus::Graphics& graphics, const InstallerContext& context, const Gdiplus::RectF& rect) {
    auto& theme = ThemeEngine::instance().current();
    Gdiplus::SolidBrush text_brush(theme.text_primary);
    Gdiplus::Font title_font(L"Segoe UI", 12, Gdiplus::FontStyleBold);
    graphics.DrawString(L"Installing Druk...", -1, &title_font, {rect.X, rect.Y, rect.Width, 30}, nullptr, &text_brush);
    
    // Draw Progress Bar
    Gdiplus::RectF bar_bg(rect.X, rect.Y + 60, rect.Width, 20);
    Gdiplus::SolidBrush bg_brush(theme.border);
    graphics.FillRectangle(&bg_brush, bar_bg);
    
    Gdiplus::RectF bar_fg(rect.X, rect.Y + 60, rect.Width * context.progress, 20);
    Gdiplus::SolidBrush accent_brush(theme.accent);
    graphics.FillRectangle(&accent_brush, bar_fg);

    // Live Logs box
    Gdiplus::RectF log_bg(rect.X, rect.Y + 95, rect.Width, rect.Height - 100);
    Gdiplus::SolidBrush log_bg_brush(theme.surface);
    graphics.FillRectangle(&log_bg_brush, log_bg);
    Gdiplus::Pen log_pen(theme.border);
    graphics.DrawRectangle(&log_pen, log_bg);

    Gdiplus::Font log_font(L"Consolas", 8);
    Gdiplus::SolidBrush log_text_brush(theme.text_secondary);
    
    auto logs = Logger::instance().get_recent_logs(15);
    float log_y = log_bg.Y + 5;
    for (const auto& log : logs) {
        std::wstring wlog(log.begin(), log.end());
        graphics.DrawString(wlog.c_str(), -1, &log_font, {log_bg.X + 5, log_y, log_bg.Width - 10, 15}, nullptr, &log_text_brush);
        log_y += 14;
    }
}

void WizardRenderer::render_completion(Gdiplus::Graphics& graphics, const InstallerContext& context, const Gdiplus::RectF& rect) {
    auto& theme = ThemeEngine::instance().current();
    
    // First draw the progress and logs underneath
    // Draw Progress Bar (100% full)
    Gdiplus::RectF bar_bg(rect.X, rect.Y + 60, rect.Width, 20);
    Gdiplus::SolidBrush bg_brush(theme.border);
    graphics.FillRectangle(&bg_brush, bar_bg);
    Gdiplus::RectF bar_fg(rect.X, rect.Y + 60, rect.Width, 20);
    Gdiplus::SolidBrush accent_brush(theme.accent);
    graphics.FillRectangle(&accent_brush, bar_fg);

    // Live Logs box
    Gdiplus::RectF log_bg(rect.X, rect.Y + 95, rect.Width, rect.Height - 100);
    Gdiplus::SolidBrush log_bg_brush(theme.surface);
    graphics.FillRectangle(&log_bg_brush, log_bg);
    Gdiplus::Pen log_pen(theme.border);
    graphics.DrawRectangle(&log_pen, log_bg);

    Gdiplus::Font log_font(L"Consolas", 8);
    Gdiplus::SolidBrush log_text_brush(theme.text_secondary);
    auto logs = Logger::instance().get_recent_logs(15);
    float log_y = log_bg.Y + 5;
    for (const auto& log : logs) {
        std::wstring wlog(log.begin(), log.end());
        graphics.DrawString(wlog.c_str(), -1, &log_font, {log_bg.X + 5, log_y, log_bg.Width - 10, 15}, nullptr, &log_text_brush);
        log_y += 14;
    }

    // Now draw the completion title
    Gdiplus::SolidBrush text_brush(theme.success);
    Gdiplus::Font title_font(L"Segoe UI", 14, Gdiplus::FontStyleBold);
    Gdiplus::Font emoji_font(L"Segoe UI Emoji", 24);
    
    graphics.DrawString(L"\x2705", -1, &emoji_font, {rect.X, rect.Y, 40, 40}, nullptr, &text_brush);
    graphics.DrawString(L"Installation Complete!", -1, &title_font, {rect.X + 45, rect.Y + 8, rect.Width - 45, 30}, nullptr, &text_brush);
}

}
