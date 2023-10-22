#ifndef HOOK_H
#define HOOK_H

#include <d3d11.h>

#include "plugin.h"
#include "internal/convarproxy.h"

class Hook
{
    private:
        Plugin* parent;
        bool show_gui = true;

        void renderMenuBar();
        void renderSQVMConsole(bool pShow = false);

    public:
        Hook(Plugin* parent);
        ~Hook();

        bool should_show() { return this->show_gui; }

        void show() { this->show_gui = true; }
        void hide() { this->show_gui = false; }
        void toggle() { this->show_gui = !this->show_gui; }

        void init(HWND window, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
        void render(ID3D11DeviceContext* pContext, ID3D11RenderTargetView** pMainRenderTargetView);
};

#endif