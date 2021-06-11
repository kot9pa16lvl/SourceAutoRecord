#include "VGui.hpp"

#include <algorithm>

#include "Features/Hud/Hud.hpp"
#include "Features/Session.hpp"
#include "Features/Timer/PauseTimer.hpp"
#include "Features/Tas/CommandQueuer.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Server.hpp"

#include "SAR.hpp"

REDECL(VGui::Paint);
REDECL(VGui::UpdateProgressBar);

Variable sar_hud_bg("sar_hud_bg", "0", "Enable the SAR HUD background.\n");

void VGui::Draw(Hud* const& hud)
{
    if (hud->ShouldDraw()) {
        hud->Paint(this->context.slot);
    }
}
void VGui::Draw(HudElement* const& element)
{
    if (element->ShouldDraw()) {
        element->Paint(&this->context);
    }
}

static void DrawHudBackground(int slot, HudContext &ctx) {
    if (!sar_hud_bg.GetBool()) return;

    int height =
        ctx.elements == 0 ?
        0 :
        ctx.elements * ctx.fontSize + (ctx.elements - 1) * ctx.spacing + 4;

    static int maxWidths[2][100];
    memmove(maxWidths[slot], maxWidths[slot] + 1, sizeof maxWidths[slot] - sizeof maxWidths[slot][0]);
    maxWidths[slot][99] = ctx.maxWidth;

    int width = 0;

    for (size_t i = 0; i < sizeof maxWidths[slot] / sizeof maxWidths[slot][0]; ++i) {
        if (maxWidths[slot][i] > width) width = maxWidths[slot][i];
    }

    if (width % 5) width += 5 - width % 5;

    if (width != 0) width += 4;

    int x = ctx.xPadding - 2;
    int y = ctx.yPadding - 2;

    surface->DrawRect(Color{ 0, 0, 0, 192 }, x, y, x + width, y + height);
}

// CEngineVGui::Paint
DETOUR(VGui::Paint, PaintMode_t mode)
{
    static HudContext lastCtx[2];

    auto result = VGui::Paint(thisptr, mode);

    surface->StartDrawing(surface->matsurface->ThisPtr());

    auto ctx = &vgui->context;

    ctx->Reset(GET_SLOT());

    if (ctx->slot == 0) {
        if (mode & PAINT_UIPANELS) {
            DrawHudBackground(0, lastCtx[0]);

            for (auto const& hud : vgui->huds) {
                vgui->Draw(hud);
            }

            for (auto const& element : vgui->elements) {
                vgui->Draw(element);
            }

            lastCtx[0] = *ctx;
        }
    } else if (ctx->slot == 1) {
        DrawHudBackground(1, lastCtx[1]);

        for (auto const& hud : vgui->huds) {
            if (hud->drawSecondSplitScreen) {
                vgui->Draw(hud);
            }
        }

        lastCtx[1] = *ctx;

        for (auto const& element : vgui->elements) {
            if (element->drawSecondSplitScreen) {
                vgui->Draw(element);
            }
        }
    }

    surface->FinishDrawing();

    if (!server->GetPlayer(GET_SLOT() + 1))
        cmdQueuer->Execute();

    return result;
}

DETOUR(VGui::UpdateProgressBar, int progress)
{
    if (vgui->lastProgressBar != progress) {
        vgui->lastProgressBar = progress;
        vgui->progressBarCount = 0;
    }
    vgui->progressBarCount++;
    if (sar_disable_progress_bar_update.GetInt() == 1 && vgui->progressBarCount > 1) {
        return 0;
    }
    if (sar_disable_progress_bar_update.GetInt() == 2) {
        return 0;
    }
    return VGui::UpdateProgressBar(thisptr, progress);
}

bool VGui::IsUIVisible(){
    return this->IsGameUIVisible(this->enginevgui->ThisPtr());
}

bool VGui::Init()
{
    this->enginevgui = Interface::Create(this->Name(), "VEngineVGui001");
    if (this->enginevgui) {
        this->IsGameUIVisible = this->enginevgui->Original<_IsGameUIVisible>(Offsets::IsGameUIVisible);

        this->enginevgui->Hook(VGui::Paint_Hook, VGui::Paint, Offsets::Paint);
        this->enginevgui->Hook(VGui::UpdateProgressBar_Hook, VGui::UpdateProgressBar, Offsets::Paint + 12);

        for (auto& hud : Hud::GetList()) {
            if (hud->version == SourceGame_Unknown || sar.game->Is(hud->version)) {
                this->huds.push_back(hud);
            }
        }

        HudElement::IndexAll();

        for (auto const& element : HudElement::GetList()) {
            if (element->version == SourceGame_Unknown || sar.game->Is(element->version)) {
                this->elements.push_back(element);
            }
        }

        std::sort(this->elements.begin(), this->elements.end(), [](const HudElement* a, const HudElement* b) {
            return a->orderIndex < b->orderIndex;
        });
    }

    return this->hasLoaded = this->enginevgui;
}
void VGui::Shutdown()
{
    Interface::Delete(this->enginevgui);
    this->huds.clear();
}

VGui* vgui;
