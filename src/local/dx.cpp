#include <d3d11.h>

#include "dx.h"

HANDLE dx_thread = nullptr;

static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

static FLOAT color[] = { 0, 0, 0, 0.0 };

HINSTANCE g_hInstance;
HINSTANCE g_hPrevInstance;
LPSTR     g_lpCmdLine;
int       g_nShowCmd;

static LRESULT WINAPI wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        if (MessageBoxA(hwnd, "Are you sure you want to close?", "Confirm Close", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
            DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_TIMER:
    case WM_MOVE:
        if (g_pd3dDeviceContext && g_pSwapChain)
        {
            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, color);
            g_pSwapChain->Present(1, 0);
        }
        break;

    case WM_SIZE: // fires when window changes its size
        if(g_pd3dDeviceContext && g_pSwapChain)
        {
            RECT rc;
            GetClientRect(hwnd, &rc);

            D3D11_VIEWPORT viewport = {
                0.0f, 0.0f,
                LOWORD(lParam),
                HIWORD(lParam),
                0.0f, 1.0f
            };

            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, color);

            g_pd3dDeviceContext->RSSetViewports(1u, &viewport);

            g_pSwapChain->Present(1, 0);

        }
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static DWORD WINAPI dx_main(void* param)
{
    HWND hwnd;
    const char* windowClass = "LocalGUI";
    MSG msg = {};
    WNDCLASSEX wc;

    wc = { sizeof(WNDCLASSEX), CS_VREDRAW | CS_HREDRAW, wndProc, 0, 0, g_hInstance, (HICON)NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, windowClass, NULL };
    RegisterClassEx(&wc);

    hwnd = CreateWindowA(wc.lpszClassName, "LocalGUI", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wc.hInstance, NULL);
    spdlog::info("Created Window");

    // initialize DirectX
    {
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.OutputWindow = hwnd;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferDesc.Width = 0;
        swapChainDesc.BufferDesc.Height = 0;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[] = { D3D_FEATURE_LEVEL_11_0 };
        D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, featureLevelArray, 1, D3D11_SDK_VERSION, &swapChainDesc, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);

        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();

        RECT rc;
        GetWindowRect(hwnd, &rc);

        D3D11_VIEWPORT viewport;
        viewport.MinDepth = 0;
        viewport.MaxDepth = 1;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = rc.right - rc.left;
        viewport.Height = rc.bottom - rc.top;
        g_pd3dDeviceContext->RSSetViewports(1u, &viewport);
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    }
    spdlog::info("Initialized DirectX");

    ShowWindow(hwnd, g_nShowCmd);
    UpdateWindow(hwnd);

    // draw the initial gray window
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, color);

    g_pSwapChain->Present(1, 0);

    SetTimer(hwnd, 0, 12, NULL);

    srand(GetTickCount());

    // message loop
    bool running = true;
    while(running)
    {
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                running = false;
        }
        if (!running)
            break;

    }

    KillTimer(hwnd, 0);

    // release resources
    g_mainRenderTargetView->Release();
    g_mainRenderTargetView = nullptr;
    g_pSwapChain->Release();
    g_pSwapChain = nullptr;
    g_pd3dDeviceContext->Release();
    g_pd3dDeviceContext = nullptr;
    g_pd3dDevice->Release();
    g_pd3dDevice = nullptr;

    DestroyWindow(hwnd);
    UnregisterClass(windowClass, g_hInstance);

    return msg.wParam;
}

int DxMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    g_hInstance = hInstance;
    g_hPrevInstance = hPrevInstance;
    g_lpCmdLine = lpCmdLine;
    g_nShowCmd = nShowCmd;

    DWORD thread_id;
    dx_thread = CreateThread(NULL, 0, dx_main, NULL, 0, &thread_id);

    return 0;
}
