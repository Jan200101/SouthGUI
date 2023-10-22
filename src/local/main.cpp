#include "dx.h"
#include "ns_plugin.h"
#include "plugin.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    spdlog::info("Main");

    Plugin plugin(nullptr, nullptr);

    DxMain(hInstance, hPrevInstance, lpCmdLine, nShowCmd);

    plugin.hook();

    WaitForSingleObject(dx_thread, INFINITE);

    return 0;
}
