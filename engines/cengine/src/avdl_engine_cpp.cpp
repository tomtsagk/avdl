#include "avdl_engine.h"

#ifdef AVDL_DIRECT3D11
/*
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <assert.h>
#include <dd_log.h>
#include "dd_game.h"
#include "avdl_assetManager.h"
#include "avdl_steam.h"
#include <wchar.h>

#include <Windows.h>
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
ID3D11Device* device_ptr = NULL;
ID3D11DeviceContext* device_context_ptr = NULL;
IDXGISwapChain* swap_chain_ptr = NULL;
#ifdef AVDL_SCARLETT
IDXGISwapChain1* swap_chain_ptr1 = NULL;
DXGI_SWAP_CHAIN_DESC1* swap_chain_desc_ptr1 = NULL;
#endif
ID3D11RenderTargetView* render_target_view_ptr = NULL;

ID3D11InputLayout* input_layout_ptr = NULL;
ID3D11Buffer* vertex_buffer_ptr = NULL;
UINT vertex_stride = 0;
UINT vertex_offset = 0;
UINT vertex_count = 0;
ID3D11VertexShader* vertex_shader_ptr = NULL;
ID3D11PixelShader* pixel_shader_ptr = NULL;
*/

extern int dd_flag_exit;

/*
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
*/

int avdl_engine_init(struct avdl_engine *o) {

	o->isPaused = 1;
	o->cworld = 0;
	o->nworld = 0;
	o->nworld_ready = 0;
	//nworld_ready = 0;
	o->nworld_loading = 0;
	o->nworld_size = 0;
	o->nworld_constructor = 0;

	o->input_key = 0;
	/*

	#ifdef AVDL_STEAM
	if (!o->verify) {
		if (!avdl_steam_init()) {
			dd_log("avdl: error initialising steam");
			return -1;
		}
	}
	#endif

	avdl_input_Init(&o->input);

	#if defined(_WIN32) || defined(WIN32)
	const PROJ_LOC_TYPE *proj_loc = avdl_getProjectLocation();
	if (proj_loc) {
		if (_wchdir(proj_loc) != 0) {
			dd_log("avdl: failed to change directory: %lS", _wcserror(errno));
			return -1;
		}
	}
	else {
		dd_log("avdl error: unable to get project location");
	}
	#endif

	avdl_assetManager_init();

	// initialise pre-game data to defaults then to game-specifics
	dd_gameInitDefault();
	dd_gameInit();

	LPCSTR CLASS_NAME = "Sample Window Class";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = o->hInstance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);

	// Create the window
	o->hwnd = CreateWindowEx(
		0, // optional window styles
		CLASS_NAME, // window class
		"Learning Window", // Window Text
		WS_OVERLAPPEDWINDOW, // window style

		// size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL, // parent window
		NULL, // menu
		o->hInstance, // instance handle
		NULL // additional application data
	);
	// error making window
	if (o->hwnd == NULL) {
		return -1;
	}

	ShowWindow(o->hwnd, o->nCmdShow);

	// Direct3d 11 stuff
	DXGI_SWAP_CHAIN_DESC swap_chain_descr = { 0 };
	swap_chain_descr.BufferDesc.RefreshRate.Numerator = 0;
	swap_chain_descr.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_descr.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swap_chain_descr.SampleDesc.Count = 1;
	swap_chain_descr.SampleDesc.Quality = 0;
	swap_chain_descr.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_descr.BufferCount = 1;
	swap_chain_descr.OutputWindow = o->hwnd;
	swap_chain_descr.Windowed = true;

	D3D_FEATURE_LEVEL feature_level;
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
	#ifdef AVDL_SCARLETT
	HRESULT hr = D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		flags,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&device_ptr,
		&feature_level,
		&device_context_ptr
	);
	assert(S_OK == hr && device_ptr && device_context_ptr);

	IDXGIDevice *device = 0;
	hr = device_ptr->QueryInterface(__uuidof(IDXGIDevice), (void**) &device);
	assert(SUCCEEDED(hr));

	IDXGIAdapter *adapter = 0;
	hr = device_ptr->GetParent(__uuidof(IDXGIAdapter), (void**)&adapter);
	assert(SUCCEEDED(hr));

	IDXGIFactory *factory = 0;
	hr = adapter->GetParent(__uuidof(IDXGIFactory), (void**)&factory);
	assert(SUCCEEDED(hr));

	IDXGIFactory2 *factory2;
	hr = factory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&factory2));
	if (SUCCEEDED(hr)) {
		// DirectX 11.1 or later
		//factory2->CreateSwapChainForCoreWindow(
		factory2->CreateSwapChainForHwnd(
			device_ptr,
			o->hwnd,
			swap_chain_desc_ptr1,
			0,
			0,
			&swap_chain_ptr1
		);
	}
	else {
	}
	#else
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		flags,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&swap_chain_descr,
		&swap_chain_ptr,
		&device_ptr,
		&feature_level,
		&device_context_ptr
	);
	assert(S_OK == hr && swap_chain_ptr && device_ptr && device_context_ptr);
	#endif

	ID3D11Texture2D* framebuffer;
	hr = swap_chain_ptr->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&framebuffer
	);
	assert(SUCCEEDED( hr ));

	hr = device_ptr->CreateRenderTargetView(
		framebuffer, 0, &render_target_view_ptr
	);
	assert(SUCCEEDED(hr));
	framebuffer->Release();

	// shaders
	UINT flagsSD = D3DCOMPILE_ENABLE_STRICTNESS;
	ID3DBlob* vs_blob_ptr = NULL;
	ID3DBlob* ps_blob_ptr = NULL;
	ID3DBlob* error_blob  = NULL;

	// vertex shader
	hr = D3DCompileFromFile(
		L"bin/shaders.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs_main",
		"vs_5_0",
		flagsSD,
		0,
		&vs_blob_ptr,
		&error_blob
	);
	if (FAILED(hr)) {
		if (error_blob) {
			OutputDebugStringA((char*)error_blob->GetBufferPointer());
			error_blob->Release();
		}
		if (vs_blob_ptr) {
			vs_blob_ptr->Release();
		}
		assert(false);
	}

	// pixel shader
	hr = D3DCompileFromFile(
		L"bin/shaders.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ps_main",
		"ps_5_0",
		flagsSD,
		0,
		&ps_blob_ptr,
		&error_blob
	);
	if (FAILED(hr)) {
		if (error_blob) {
			OutputDebugStringA((char*)error_blob->GetBufferPointer());
			error_blob->Release();
		}
		if (ps_blob_ptr) {
			ps_blob_ptr->Release();
		}
		assert(false);
	}

	vertex_shader_ptr = NULL;
	pixel_shader_ptr = NULL;

	hr = device_ptr->CreateVertexShader(
		vs_blob_ptr->GetBufferPointer(),
		vs_blob_ptr->GetBufferSize(),
		NULL,
		&vertex_shader_ptr
	);
	assert(SUCCEEDED(hr));

	hr = device_ptr->CreatePixelShader(
		ps_blob_ptr->GetBufferPointer(),
		ps_blob_ptr->GetBufferSize(),
		NULL,
		&pixel_shader_ptr
	);
	assert(SUCCEEDED(hr));

	input_layout_ptr = NULL;
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = device_ptr->CreateInputLayout(
		inputElementDesc,
		ARRAYSIZE(inputElementDesc),
		vs_blob_ptr->GetBufferPointer(),
		vs_blob_ptr->GetBufferSize(),
		&input_layout_ptr
	);
	assert(SUCCEEDED(hr));

	// create vertex buffer
	float vertex_data_array[] = {
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
	};
	vertex_stride = 3 * sizeof(float);
	vertex_offset = 0;
	vertex_count = 3;

	vertex_buffer_ptr = NULL;
	{
		D3D11_BUFFER_DESC vertex_buff_descr = {};
		vertex_buff_descr.ByteWidth = sizeof(vertex_data_array);
		vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
		vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA sr_data = { 0 };
		sr_data.pSysMem = vertex_data_array;
		hr = device_ptr->CreateBuffer(
			&vertex_buff_descr,
			&sr_data,
			&vertex_buffer_ptr
		);
		assert(SUCCEEDED(hr));
	}

//	// run the message loop
//	MSG msg = {};
//	bool should_close = false;
//	while (!should_close) {
//		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
//			TranslateMessage(&msg);
//			DispatchMessage(&msg);
//		}
//		if (msg.message == WM_QUIT) {
//			break;
//		}
//
//		// render here with direct 3d
//		float background_colour[4] = {
//			0x64 / 255.0f, 0x95 / 255.0f, 0xED / 255.0f, 1.0f
//		};
//		device_context_ptr->ClearRenderTargetView(
//			render_target_view_ptr, background_colour
//		);
//
//		RECT winRect;
//		GetClientRect(hwnd, &winRect);
//		D3D11_VIEWPORT viewport = {
//			0.0f,
//			0.0f,
//			(FLOAT)(winRect.right - winRect.left),
//			(FLOAT)(winRect.bottom - winRect.top),
//			0.0f,
//			1.0f
//		};
//		device_context_ptr->RSSetViewports(1, &viewport);
//
//		device_context_ptr->OMSetRenderTargets(1, &render_target_view_ptr, NULL);
//
//		device_context_ptr->IASetPrimitiveTopology(
//			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
//		);
//		device_context_ptr->IASetInputLayout(input_layout_ptr);
//		device_context_ptr->IASetVertexBuffers(
//			0,
//			1,
//			&vertex_buffer_ptr,
//			&vertex_stride,
//			&vertex_offset
//		);
//
//		device_context_ptr->VSSetShader(vertex_shader_ptr, NULL, 0);
//		device_context_ptr->PSSetShader(pixel_shader_ptr, NULL, 0);
//
//		device_context_ptr->Draw(vertex_count, 0);
//		swap_chain_ptr->Present(1, 0);
//	}
*/

	return 0;
}

int avdl_engine_draw(struct avdl_engine *o) {

	// render here with direct 3d
	/*
	float background_colour[4] = {
		0x64 / 255.0f, 0x95 / 255.0f, 0xED / 255.0f, 1.0f
	};
	device_context_ptr->ClearRenderTargetView(
		render_target_view_ptr, background_colour
	);
	*/
	/*
	avdl_graphics_ClearToColour();

	RECT winRect;
	GetClientRect(o->hwnd, &winRect);
	D3D11_VIEWPORT viewport = {
		0.0f,
		0.0f,
		(FLOAT)(winRect.right - winRect.left),
		(FLOAT)(winRect.bottom - winRect.top),
		0.0f,
		1.0f
	};
	device_context_ptr->RSSetViewports(1, &viewport);

	device_context_ptr->OMSetRenderTargets(1, &render_target_view_ptr, NULL);

	device_context_ptr->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);
	device_context_ptr->IASetInputLayout(input_layout_ptr);
	device_context_ptr->IASetVertexBuffers(
		0,
		1,
		&vertex_buffer_ptr,
		&vertex_stride,
		&vertex_offset
	);

	device_context_ptr->VSSetShader(vertex_shader_ptr, NULL, 0);
	device_context_ptr->PSSetShader(pixel_shader_ptr, NULL, 0);

	device_context_ptr->Draw(vertex_count, 0);
	swap_chain_ptr->Present(1, 0);
	*/

	return 0;

}

int avdl_engine_loop(struct avdl_engine *o) {
	/*
	// run the message loop
	int isRunning = 1;
	MSG msg = {};
	while (isRunning) {

		// get msg
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// msg to quit
		if (msg.message == WM_QUIT) {
			break;
		}

		//update();

		#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
		if (avdl_engine_isPaused(o)) {
			return 0;
		}
		#endif
		avdl_engine_update(o);

		// prepare next frame
		#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
		//draw();
		#endif

		#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
		if (dd_flag_exit) {
			return 0;
		}
		#endif

		#if defined( AVDL_LINUX ) || defined( AVDL_WINDOWS )
		avdl_engine_draw(o);
		#endif

		// close the game
		if (dd_flag_exit) {
//			JNIEnv *env;
//			int getEnvStat = (*jvm)->GetEnv(jvm, &env, JNI_VERSION_1_4);
//
//			if (getEnvStat == JNI_EDETACHED) {
//				if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != 0) {
//					dd_log("avdl: failed to attach thread for new world");
//				}
//			} else if (getEnvStat == JNI_OK) {
//			} else if (getEnvStat == JNI_EVERSION) {
//				dd_log("avdl: GetEnv: version not supported");
//			}
//			jniEnv = env;
//			dd_log("get method");
//			jmethodID MethodID = (*(*jniEnv)->GetStaticMethodID)(jniEnv, clazz, "CloseApplication", "()V");
//			dd_log("call method");
//			(*(*jniEnv)->CallStaticVoidMethod)(jniEnv, clazz, MethodID);
//			dd_log("detach");
//			if (getEnvStat == JNI_EDETACHED) {
//				(*jvm)->DetachCurrentThread(jvm);
//			}
		}
	}
	*/
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif
