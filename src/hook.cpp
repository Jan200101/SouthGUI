#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include "kiero.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "ns_plugin.h"
#include "hook.h"
#include "plugin.h"

#define IsMouseMsg(uMsg) ((uMsg) >= WM_MOUSEFIRST && (uMsg) <= WM_MOUSELAST)
#define IsKeyMsg(uMsg) ((uMsg) >= WM_KEYFIRST && (uMsg) <= WM_KEYLAST)

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

Hook* g_hook = nullptr;

static Present oPresent;
static HWND g_window = nullptr;
static WNDPROC oWndProc;
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Don't pass to imgui if there's no cursor visible
    if (uMsg == WM_SETCURSOR)
    {
        return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
    }

    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

    if (IsMouseMsg(uMsg) && ImGui::GetIO().WantCaptureMouse)
    {
        return 0;
    }

    if (IsKeyMsg(uMsg) && ImGui::GetIO().WantCaptureKeyboard)
    {
        return 0;
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    static bool hk_init;
    if (!hk_init)
    {
        if (g_hook && SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& g_pDevice)))
        {
            g_pDevice->GetImmediateContext(&g_pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            g_window = sd.OutputWindow;
            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
            g_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
            pBackBuffer->Release();
            oWndProc = (WNDPROC)SetWindowLongPtr(g_window, GWLP_WNDPROC, (LONG_PTR)WndProc);
            g_hook->init(g_window, g_pDevice, g_pContext);
            hk_init = true;
        }
        else
            return oPresent(pSwapChain, SyncInterval, Flags);
    }

    g_hook->render(g_pContext, &g_mainRenderTargetView);
    return oPresent(pSwapChain, SyncInterval, Flags);
}

void ConCommand_southgui_toggle(const CCommand& command)
{
    if (g_hook)
    {
        g_hook->toggle();
        spdlog::info("Toggled SouthGUI");
    }
}

Hook::Hook(Plugin* parent):
    parent(parent)
{
    if (g_hook)
    {
        spdlog::error("a hook is already initialized");
        return;
    }

    parent->ConCommand("southgui_toggle", ConCommand_southgui_toggle, "Toggle SouthGUI", 0);

    g_hook = this;

    bool init_hook = false;
    do
    {
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
        {
            spdlog::info("D3D11 hook intiialized");
            kiero::bind(8, (void**)&oPresent, (void*)hkPresent);
            init_hook = true;
        }
    } while (!init_hook);
}

Hook::~Hook()
{
    g_hook = nullptr;
    kiero::unbind(8);

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Hook::init(HWND window, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    spdlog::info("Initializing ImGui context");

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);
    ImGui_ImplDX11_CreateDeviceObjects();
}

void Hook::renderMenuBar()
{
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    static ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace Demo", NULL, window_flags);

    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Windows"))
        {
            if (ImGui::MenuItem("Squirrel Console")) { this->renderSQVMConsole(true); }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void Hook::renderSQVMConsole(bool pShow)
{
    static bool show_console;
    if (pShow)
    {
        show_console = true;
        return;
    }
    else if (!show_console)
    {
        return;
    }

    ImGui::Begin("Squirrel Console", &show_console);
    ImGui::End();
}

void Hook::render(ID3D11DeviceContext* pContext, ID3D11RenderTargetView** pMainRenderTargetView)
{
    if (!this->should_show())
        return;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    this->renderMenuBar();
    this->renderSQVMConsole();

    ImGui::ShowDemoWindow();

    ImGui::Render();
    
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}