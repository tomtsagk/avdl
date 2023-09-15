#include "avdl_graphics.h"

#include "avdl_cengine.h"

#ifdef AVDL_DIRECT3D11
#include "pch.h"
/*
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <assert.h>
#include <dd_log.h>

#include <Windows.h>

#include "avdl_shaders.h"
#include "dd_game.h"
#include "dd_log.h"
#include <stdlib.h>
*/
using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Popups;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

using namespace Microsoft::WRL;
using namespace Platform;
using namespace DirectX;

#ifdef __cplusplus
extern "C" {
#endif

extern ComPtr<ID3D11Device3> avdl_d3dDevice;
extern ComPtr<ID3D11DeviceContext3> avdl_d3dContext;
extern ComPtr<ID3D11RenderTargetView1> avdl_d3dRenderTargetView;

/*
extern ID3D11DeviceContext* device_context_ptr;
extern ID3D11RenderTargetView* render_target_view_ptr;
*/

void avdl_graphics_ClearDepth() {
	//glClear(GL_DEPTH_BUFFER_BIT);
}

void avdl_graphics_ClearToColour() {
	XMVECTORF32 clearcolor = { dd_clearcolor_r, dd_clearcolor_g, dd_clearcolor_b, 1.0f };
	avdl_d3dContext->ClearRenderTargetView(avdl_d3dRenderTargetView.Get(), clearcolor);
}

void avdl_graphics_ClearColourAndDepth() {
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// context id - used to re-load assets if context is reset
static int openglContextId = 0;

void avdl_graphics_generateContextId() {
	openglContextId++;
}

int avdl_graphics_getContextId() {
	return openglContextId;
}

ComPtr<ID3D11Device3> avdl_d3dDevice;
ComPtr<ID3D11DeviceContext3> avdl_d3dContext;
ComPtr<IDXGISwapChain3> avdl_swapChain;
ComPtr<ID3D11RenderTargetView1>	avdl_d3dRenderTargetView;
Size avdl_d3dRenderTargetSize;

int avdl_graphics_Init() {

	/*
	 * Create d3d11 device
	 */

	// This flag adds support for surfaces with a different color channel ordering
	// than the API default. It is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	// Create the Direct3D 11 API device object and a corresponding context.
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;

	// Create the device and device context objects
	HRESULT hr = D3D11CreateDevice(
		nullptr, // adapter
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr, // software
		creationFlags, // flags
		nullptr, // feature levels
		0, // feature levels count
		D3D11_SDK_VERSION,
		&device, // device
		nullptr, // feature level variable
		&context // device context
	);

	// Store pointers to the Direct3D 11.3 API device and immediate context.
	device.As(&avdl_d3dDevice);
	context.As(&avdl_d3dContext);

	/*
	avdl_graphics_generateContext();
	*/
	return 0;

}

void avdl_graphics_Viewport(int x, int y, int w, int h) {
	//glViewport(x, y, w, h);
}

void avdl_graphics_PrintInfo() {
	/*
	dd_log("Vendor graphic card: %s", glGetString(GL_VENDOR));
	dd_log("Renderer: %s", glGetString(GL_RENDERER));
	dd_log("Version GL: %s", glGetString(GL_VERSION));
	dd_log("Version GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	*/
}

int avdl_graphics_GetCurrentProgram() {
	/*
	int program = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);
	return program;
	*/
	return 0;
}

void avdl_graphics_UseProgram(int program) {
	//glUseProgram(program);
}

int avdl_graphics_GetUniformLocation(int program, const char *uniform) {
	//return glGetUniformLocation(program, uniform);
	return 0;
}

extern "C" int avdl_graphics_ImageToGpu(void *pixels, int pixel_format, int width, int height) {

	return 0;
	/*
	glGenTextures(1, &o->tex);
	glBindTexture(GL_TEXTURE_2D, o->tex);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	glTexImage2D(GL_TEXTURE_2D, 0, o->pixelFormat, o->width, o->height, 0, o->pixelFormat, GL_FLOAT, o->pixels);
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, o->width, o->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, o->pixelsb);
	#endif

	glBindTexture(GL_TEXTURE_2D, 0);

	#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
	free(o->pixels);
	o->pixels = 0;
	#elif defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	free(o->pixelsb);
	o->pixelsb = 0;
	#endif
	*/

}

void avdl_graphics_DeleteTexture(unsigned int tex) {
	//glDeleteTextures(1, &tex);
}

void avdl_graphics_BindTexture(unsigned int tex) {
	//glBindTexture(GL_TEXTURE_2D, tex);
}

void avdl_graphics_EnableBlend() {
	/*
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	*/
}

void avdl_graphics_DisableBlend() {
	//glDisable(GL_BLEND);
}

void avdl_graphics_EnableVertexAttribArray(int attrib) {
	//glEnableVertexAttribArray(attrib);
}

void avdl_graphics_DisableVertexAttribArray(int attrib) {
	//glDisableVertexAttribArray(attrib);
}

void avdl_graphics_VertexAttribPointer(int p, int size, int format, int normalised, int stride, void *data) {
	/*
	int norm;
	if (!normalised) {
		norm = GL_FALSE;
	}
	else {
		norm = GL_TRUE;
	}
	glVertexAttribPointer(p, size, format, norm, stride, data);
	*/
}

void avdl_graphics_DrawArrays(int vcount) {
	//glDrawArrays(GL_TRIANGLES, 0, vcount);
}

int avdl_graphics_generateContext() {
//	avdl_graphics_generateContextId();
//
//	glEnable(GL_DEPTH_TEST);
//	glClearColor(0.8, 0.6, 1.0, 1);
//
//	/*
//	 * load shaders
//	 /
//	defaultProgram = avdl_loadProgram(avdl_shaderDefault_vertex, avdl_shaderDefault_fragment);
//	if (!defaultProgram) {
//		dd_log("avdl: error loading shaders");
//		return -1;
//	}
//
//	fontProgram = avdl_loadProgram(avdl_shaderFont_vertex, avdl_shaderFont_fragment);
//	if (!fontProgram) {
//		dd_log("avdl: error loading font shaders");
//		return -1;
//	}
//
//	glUseProgram(defaultProgram);
//	currentProgram = defaultProgram;
	return 0;
}

void avdl_graphics_d3d11_SetWindow() {
	CoreWindow^ window = CoreWindow::GetForCurrentThread();
	// Clear the previous window size specific context.
	ID3D11RenderTargetView* nullViews[] = {nullptr};
	avdl_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
	avdl_d3dRenderTargetView = nullptr;
	avdl_d3dContext->Flush1(D3D11_CONTEXT_TYPE_ALL, nullptr);

	CoreWindow^ Window = CoreWindow::GetForCurrentThread();
	avdl_d3dRenderTargetSize.Width = Window->Bounds.Width;
	avdl_d3dRenderTargetSize.Height = Window->Bounds.Height;

	if (avdl_swapChain != nullptr)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = avdl_swapChain->ResizeBuffers(
			2, // Double-buffered swap chain.
			lround(avdl_d3dRenderTargetSize.Width),
			lround(avdl_d3dRenderTargetSize.Height),
			DXGI_FORMAT_B8G8R8A8_UNORM,
			0
		);
	}
	else
	{
		// Otherwise, create a new one using the same adapter as the existing Direct3D device.
		DXGI_SCALING scaling = DXGI_SCALING_NONE;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};

		swapChainDesc.Width = lround(avdl_d3dRenderTargetSize.Width);		// Match the size of the window.
		swapChainDesc.Height = lround(avdl_d3dRenderTargetSize.Height);
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;				// This is the most common swap chain format.
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;								// Don't use multi-sampling.
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;									// Use double-buffering to minimize latency.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// All Microsoft Store apps must use this SwapEffect.
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = scaling;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
		ComPtr<IDXGIDevice3> dxgiDevice;
		avdl_d3dDevice.As(&dxgiDevice);

		ComPtr<IDXGIAdapter> dxgiAdapter;
		dxgiDevice->GetAdapter(&dxgiAdapter);

		ComPtr<IDXGIFactory4> dxgiFactory;
		dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

		ComPtr<IDXGISwapChain1> swapChain;
		dxgiFactory->CreateSwapChainForCoreWindow(
			avdl_d3dDevice.Get(),
			reinterpret_cast<IUnknown*>(window),
			&swapChainDesc,
			nullptr,
			&swapChain
		);
		swapChain.As(&avdl_swapChain);

		// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
		// ensures that the application will only render after each VSync, minimizing power consumption.
		dxgiDevice->SetMaximumFrameLatency(1);
	}

	// Create a render target view of the swap chain back buffer.
	ComPtr<ID3D11Texture2D1> backBuffer;
	avdl_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

	avdl_d3dDevice->CreateRenderTargetView1(
		backBuffer.Get(),
		nullptr,
		&avdl_d3dRenderTargetView
	);

	// Set the 3D rendering viewport to target the entire window.
	D3D11_VIEWPORT avdl_screenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		Window->Bounds.Width,
		Window->Bounds.Height
		);

	avdl_d3dContext->RSSetViewports(1, &avdl_screenViewport);
}

#ifdef __cplusplus
}
#endif

#endif
