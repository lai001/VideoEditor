#include "WindowsPlatform.hpp"
#include <spdlog/spdlog.h>

WindowsPlatform::WindowsPlatform(const Configuration & cfg)
	:cfg(cfg)
{
	Init(cfg);
}

WindowsPlatform::~WindowsPlatform()
{
	cleanupRenderTarget();
	cleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);
}

void WindowsPlatform::Init(const Configuration &cfg)
{
	this->cfg = cfg;
	wc = WNDCLASSEX { sizeof(WNDCLASSEX), CS_CLASSDC, WindowsPlatform::WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T(cfg.windowName.c_str()), NULL };
	::RegisterClassEx(&wc);
	hwnd = ::CreateWindow(wc.lpszClassName, _T(cfg.windowName.c_str()), WS_OVERLAPPEDWINDOW, cfg.x, cfg.y, cfg.windowWidth, cfg.windowHeight, NULL, NULL, wc.hInstance, this);

	if (createDeviceD3D(hwnd))
	{
		createRenderTarget();
	}
	else
	{
		cleanupRenderTarget();
		cleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		assert(false);
	}

	bool showWindowStatus = showWindow(cfg.showWindowCommandType);
	bool updateWindowStatus = updateWindow();
}

bool WindowsPlatform::shouldClose()
{
	bool done = false;
	MSG msg;
	while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			done = true;
	}
	return done;
}

void WindowsPlatform::present(const int vsync) const noexcept
{
	pSwapChain->Present(vsync, 0);
}

void WindowsPlatform::setRenderTarget() const noexcept
{
	pd3dDeviceContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
}

void WindowsPlatform::clearColor(const float* color)
{
	if (color)
	{
		pd3dDeviceContext->ClearRenderTargetView(mainRenderTargetView, color);
	}
	else
	{
		const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		pd3dDeviceContext->ClearRenderTargetView(mainRenderTargetView, color);
	}
}

ID3D11Device * WindowsPlatform::getDevice() const noexcept
{
	return pd3dDevice;
}

ID3D11DeviceContext * WindowsPlatform::getDeviceContext() const noexcept
{
	return pd3dDeviceContext;
}

bool WindowsPlatform::showWindow(const ShowWindowCommandType type)
{
	assert(hwnd);
	return ::ShowWindow(hwnd, static_cast<int>(type));
}

bool WindowsPlatform::updateWindow() const noexcept
{
	assert(hwnd);
	return ::UpdateWindow(hwnd);
}

HWND WindowsPlatform::getHwnd() const noexcept
{
	return hwnd;
}

bool WindowsPlatform::createDeviceD3D(HWND hWnd)
{
	assert(hwnd);
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	if (cfg.isDebugEnable)
	{
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT status = D3D11CreateDeviceAndSwapChain(nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		featureLevelArray,
		2,
		D3D11_SDK_VERSION,
		&sd,
		&pSwapChain,
		&pd3dDevice,
		&featureLevel,
		&pd3dDeviceContext);
	assert(SUCCEEDED(status));
	return true;
}

void WindowsPlatform::cleanupDeviceD3D()
{
	if (pSwapChain) { pSwapChain->Release(); pSwapChain = nullptr; }
	if (pd3dDeviceContext) { pd3dDeviceContext->Release(); pd3dDeviceContext = nullptr; }
	if (pd3dDevice) { pd3dDevice->Release(); pd3dDevice = nullptr; }
}

void WindowsPlatform::createRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
	pBackBuffer->Release();
}

void WindowsPlatform::cleanupRenderTarget()
{
	if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = NULL; }
}

LRESULT WINAPI WindowsPlatform::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WindowsPlatform* p_ptr = (WindowsPlatform*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (p_ptr)
	{
		if (p_ptr->WndProcCallback)
		{
			bool intercept = false;
			bool status = p_ptr->WndProcCallback(hWnd, msg, wParam, lParam, intercept);
			if (intercept)
			{
				return status;
			}
		}
	}

	switch (msg)
	{
	case WM_SIZE:
		if (p_ptr && p_ptr->pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
		{
			p_ptr->cleanupRenderTarget();
			p_ptr->pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			p_ptr->createRenderTarget();

			if (p_ptr->sizeChange)
			{
				p_ptr->sizeChange((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
			}
		}
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;

	case WM_CREATE:
		LPCREATESTRUCT lpcreatestruct = (LPCREATESTRUCT)lParam;
		WindowsPlatform* p_ptr = reinterpret_cast<WindowsPlatform*>(lpcreatestruct->lpCreateParams);
		LONG_PTR long_ptr = reinterpret_cast<LONG_PTR>(p_ptr);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, long_ptr);
		break;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
