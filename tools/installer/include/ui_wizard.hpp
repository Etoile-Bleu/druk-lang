#pragma once

#include <windows.h>
#include <gdiplus.h>
#include "installer.hpp"

namespace druk::installer {

class WizardRenderer {
public:
    static void render(Gdiplus::Graphics& graphics, const InstallerContext& context, int width, int height);

private:
    static void render_welcome(Gdiplus::Graphics& graphics, const InstallerContext& context, const Gdiplus::RectF& rect);
    static void render_maintenance(Gdiplus::Graphics& graphics, const InstallerContext& context, const Gdiplus::RectF& rect);
    static void render_license(Gdiplus::Graphics& graphics, const InstallerContext& context, const Gdiplus::RectF& rect);
    static void render_options(Gdiplus::Graphics& graphics, const InstallerContext& context, const Gdiplus::RectF& rect);
    static void render_progress(Gdiplus::Graphics& graphics, const InstallerContext& context, const Gdiplus::RectF& rect);
    static void render_completion(Gdiplus::Graphics& graphics, const InstallerContext& context, const Gdiplus::RectF& rect);
};

}
