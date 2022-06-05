#ifndef KSImage_Example_src_Platform_WindowsPlatform_hpp
#define KSImage_Example_src_Platform_WindowsPlatform_hpp

#ifdef _WIN32
#define NOMINMAX
#include <string>
#include <functional>
#include <windows.h>
#include <d3d11.h>
#include <tchar.h>
#include <Foundation/Foundation.hpp>

class WindowsPlatform
{
public:
	enum class ShowWindowCommandType
	{
		hide,
		showNormal,
		normal,
		showMinimized,
		showMaximized,
		maximize,
		showNoActivate,
		show,
		minimize,
		showMinnoactive,
		showNa,
		restore,
		showDefault,
		forceMinimize,
		max
	};

	struct Configuration
	{
		std::string windowName = "No name";
		unsigned int windowWidth = 1280;
		unsigned int windowHeight = 720;
		int x = 0;
		int y = 0;
		bool isDebugEnable = true;
		ShowWindowCommandType showWindowCommandType = ShowWindowCommandType::showNormal;
	};

public:
	explicit WindowsPlatform(const Configuration &cfg);
	~WindowsPlatform();

	std::function<bool(HWND, UINT, WPARAM, LPARAM, bool&)> WndProcCallback;
	std::function<void(unsigned int width, unsigned int height)> sizeChange;

	bool shouldClose();
	void present(const int vsync = 1) const noexcept;
	void setRenderTarget() const noexcept;
	void clearColor(const float* color = nullptr);

	ID3D11Device* getDevice() const noexcept;
	ID3D11DeviceContext* getDeviceContext() const noexcept;

	bool showWindow(const ShowWindowCommandType type);
	bool updateWindow() const noexcept;

	HWND getHwnd() const noexcept;

private:
	Configuration cfg;
	HWND hwnd = nullptr;
	WNDCLASSEX wc;
	ID3D11Device*            pd3dDevice = nullptr;
	ID3D11DeviceContext*     pd3dDeviceContext = nullptr;
	IDXGISwapChain*          pSwapChain = nullptr;
	ID3D11RenderTargetView*  mainRenderTargetView = nullptr;

	bool createDeviceD3D(HWND hWnd);
	void cleanupDeviceD3D();
	void createRenderTarget();
	void cleanupRenderTarget();
	void Init(const Configuration &cfg);

	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif // _WIN32
#endif // !KSImage_Example_src_Platform_WindowsPlatform_hpp