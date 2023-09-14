#include "avdl_engine.h"
#include <avdl_assetManager.h>
#include <dd_string3d.h>

#ifdef AVDL_DIRECT3D11
#include "pch.h"
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

// Main d3d11 avdl application
ref class D3D11AvdlApplication sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
public:
	// must-override functions
	virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
	virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
	virtual void Load(Platform::String^ entryPoint);
	virtual void Run();
	virtual void Uninitialize();

protected:

	// lifecycle events
	void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
	void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
	void OnResuming(Platform::Object^ sender, Platform::Object^ args);

	// input
	void OnPointerPressed(Windows::UI::Core::CoreWindow^ window, Windows::UI::Core::PointerEventArgs^ args);
	void OnPointerMoved(Windows::UI::Core::CoreWindow^ window, Windows::UI::Core::PointerEventArgs^ args);
	void OnPointerReleased(Windows::UI::Core::CoreWindow^ window, Windows::UI::Core::PointerEventArgs^ args);
	void OnKeyDown(Windows::UI::Core::CoreWindow^ window, Windows::UI::Core::KeyEventArgs^ args);
	void OnKeyUp(Windows::UI::Core::CoreWindow^ window, Windows::UI::Core::KeyEventArgs^ args);

	// Window event handlers.
	void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
	void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
	void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);

	// DisplayInformation event handlers.
	void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
	void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
	void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

private:
	bool m_windowClosed;
};

// Application Source
ref class D3D11AvdlApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};

struct VertexPositionColor
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 color;
};

struct ModelViewProjectionConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};
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
// temp cube
extern ComPtr<ID3D11Buffer> avdl_constantBuffer;
extern ModelViewProjectionConstantBuffer avdl_constantBufferData;
extern ComPtr<ID3D11VertexShader> avdl_vertexShader;
extern ComPtr<ID3D11PixelShader> avdl_pixelShader;
extern ComPtr<ID3D11InputLayout> avdl_inputLayout;
extern ComPtr<ID3D11Buffer> avdl_vertexBuffer;

#ifdef __cplusplus
extern "C" {
#endif

extern ComPtr<IDXGISwapChain3> avdl_swapChain;
extern ComPtr<ID3D11DeviceContext3> avdl_d3dContext;
extern ComPtr<ID3D11RenderTargetView1> avdl_d3dRenderTargetView;

extern int dd_flag_exit;

int avdl_engine_init(struct avdl_engine *o) {

	o->isPaused = 1;
	o->cworld = 0;
	o->nworld = 0;
	//o->nworld_ready = 0;
	nworld_ready = 0;
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
	*/

	avdl_input_Init(&o->input);

	/*
	#if defined(AVDL_DIRECT3D11)
	#elif defined(_WIN32) || defined(WIN32)
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
	*/

	o->achievements = avdl_achievements_create();
	avdl_assetManager_init();

	#if DD_PLATFORM_NATIVE
	/*
	// initialise curl
	curl_global_init(CURL_GLOBAL_ALL);

	if (pthread_mutex_init(&asyncCallMutex, NULL) != 0)
	{
		dd_log("avdl: mutex for async calls init failed");
		return -1;
	}
	*/
	#endif

	// initialise pre-game data to defaults then to game-specifics
	dd_gameInitDefault();
	dd_gameInit();

	#if DD_PLATFORM_NATIVE || defined(AVDL_DIRECT3D11)
	avdl_engine_resize(o, dd_window_width(), dd_window_height());
	#endif

	avdl_graphics_Init();

	/*
	 * string3d initialisation for displaying text
	 */
	if (dd_string3d_isActive()) {
		dd_string3d_init();
	}

	dd_clearColour(0.6f, 0.9f, 0.2f);

	return 0;
}

extern "C" struct dd_matrix matPerspective;
extern "C" void avdl_graphics_direct3d11_drawMesh(struct dd_matrix *matrix);

int avdl_engine_draw(struct avdl_engine *o) {

	// render here with direct 3d

	// clear previous frame
	avdl_graphics_ClearToColour();
	avdl_graphics_ClearColourAndDepth();

	// reset view
	dd_matrix_globalInit();

	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { avdl_d3dRenderTargetView.Get() };
	avdl_d3dContext->OMSetRenderTargets(1, targets, 0);

	// draw world
	if (o->cworld && o->cworld->draw) {
		o->cworld->draw(o->cworld);
	}

	/*
	 * ??
	// Prepare the constant buffer to send it to the graphics device.
	avdl_d3dContext->UpdateSubresource1(
		avdl_constantBuffer.Get(),
		0,
		NULL,
		&avdl_constantBufferData,
		0,
		0,
		0
	);

	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	avdl_d3dContext->IASetVertexBuffers(
		0,
		1,
		avdl_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
	);

	avdl_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	avdl_d3dContext->IASetInputLayout(avdl_inputLayout.Get());

	// Attach our vertex shader.
	avdl_d3dContext->VSSetShader(
		avdl_vertexShader.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	avdl_d3dContext->VSSetConstantBuffers1(
		0,
		1,
		avdl_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
		);

	// Attach our pixel shader.
	avdl_d3dContext->PSSetShader(
		avdl_pixelShader.Get(),
		nullptr,
		0
		);
	
	// Draw cube
	avdl_d3dContext->Draw(
		9,
		0
	);
	 */

	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.
	DXGI_PRESENT_PARAMETERS parameters = { 0 };
	HRESULT hr = avdl_swapChain->Present1(1, 0, &parameters);

	// Discard the contents of the render target.
	// This is a valid operation only when the existing contents will be entirely
	// overwritten. If dirty or scroll rects are used, this call should be removed.
	avdl_d3dContext->DiscardView1(avdl_d3dRenderTargetView.Get(), nullptr, 0);

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

		#if DD_PLATFORM_NATIVE
		if (avdl_engine_isPaused(o)) {
			return 0;
		}
		#endif
		avdl_engine_update(o);

		// prepare next frame
		#if DD_PLATFORM_NATIVE
		//draw();
		#endif

		#if DD_PLATFORM_ANDROID
		if (dd_flag_exit) {
			return 0;
		}
		#endif

		#if DD_PLATFORM_NATIVE
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

extern "C" void avdl_graphics_direct3d11_drawMesh(struct dd_meshColour *m, struct dd_matrix *matrix);

// Constant buffer used to send MVP matrices to the vertex shader.
struct ModelViewProjectionConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
};

// Used to send per-vertex data to the vertex shader.
struct VertexPositionColor
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 color;
};

// parts of `avdl_graphics`
extern "C" ComPtr<ID3D11Device3> avdl_d3dDevice;
extern "C" ComPtr<ID3D11DeviceContext3> avdl_d3dContext;
extern "C" ComPtr<ID3D11RenderTargetView1> avdl_d3dRenderTargetView;

// mesh
ComPtr<ID3D11Buffer> avdl_constantBuffer;
ModelViewProjectionConstantBuffer avdl_constantBufferData;

ComPtr<ID3D11Buffer> avdl_vertexBuffer;

// shaders
ComPtr<ID3D11VertexShader> avdl_vertexShader;
ComPtr<ID3D11PixelShader> avdl_pixelShader;
ComPtr<ID3D11InputLayout> avdl_inputLayout;

double totalRotation = 0.0;

extern "C" struct avdl_engine engine;

void avdl_log2(const char *msg, ...) {

	va_list args;
	va_start(args, msg);

	char buffer[1024];
	vsnprintf(buffer, 1024, msg, args);

	std::string s_str = std::string(buffer);
	std::wstring wid_str = std::wstring(s_str.begin(), s_str.end());
	const wchar_t* w_char = wid_str.c_str();
	Platform::String^ p_string = ref new Platform::String(w_char);

	MessageDialog^ dialog = ref new MessageDialog(p_string);
	UICommand^ continueCommand = ref new UICommand("Ok");
	UICommand^ upgradeCommand = ref new UICommand("Cancel");

	dialog->DefaultCommandIndex = 0;
	dialog->CancelCommandIndex = 1;
	dialog->Commands->Append(continueCommand);
	dialog->Commands->Append(upgradeCommand);
	dialog->ShowAsync();

	va_end(args);
}


#include <fstream>

// this function loads a file into an Array^
Array<byte>^ LoadShaderFile(std::string File)
{
	Array<byte>^ FileData = nullptr;

	// open the file
	std::ifstream VertexFile(File, std::ios::in | std::ios::binary | std::ios::ate);

	// if open was successful
	if(VertexFile.is_open())
	{
		// find the length of the file
		int Length = (int)VertexFile.tellg();

		// collect the file data
		FileData = ref new Array<byte>(Length);
		VertexFile.seekg(0, std::ios::beg);
		VertexFile.read(reinterpret_cast<char*>(FileData->Data), Length);
		VertexFile.close();
	}
	else {
	}

	return FileData;
}

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	CoreApplication::Run(ref new D3D11AvdlApplicationSource());
	return 0;
}

// Application Source
IFrameworkView^ D3D11AvdlApplicationSource::CreateView()
{
	return ref new D3D11AvdlApplication();
}

// The first method called when the IFrameworkView is being created.
void D3D11AvdlApplication::Initialize(CoreApplicationView^ applicationView)
{
	// prepare avdl
	avdl_engine_init(&engine);

	// Prepare lifecycle handlers
	applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &D3D11AvdlApplication::OnActivated);
	CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &D3D11AvdlApplication::OnSuspending);
	CoreApplication::Resuming   += ref new EventHandler<Platform::Object^>   (this, &D3D11AvdlApplication::OnResuming);


	m_windowClosed = false;
}

// Called when the CoreWindow object is created (or re-created).
void D3D11AvdlApplication::SetWindow(CoreWindow^ window)
{
	// input
	window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &D3D11AvdlApplication::OnPointerPressed);
	window->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &D3D11AvdlApplication::OnPointerMoved);
	window->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &D3D11AvdlApplication::OnPointerReleased);
	window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &D3D11AvdlApplication::OnKeyDown);
	window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &D3D11AvdlApplication::OnKeyUp);

	window->SizeChanged += 
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &D3D11AvdlApplication::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &D3D11AvdlApplication::OnVisibilityChanged);

	// window event
	window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &D3D11AvdlApplication::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &D3D11AvdlApplication::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &D3D11AvdlApplication::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &D3D11AvdlApplication::OnDisplayContentsInvalidated);

	dd_width = window->Bounds.Width;
	dd_height = window->Bounds.Height;
	avdl_engine_resize(&engine, dd_width, dd_height);

	avdl_graphics_d3d11_SetWindow();
}

extern "C" struct dd_matrix matPerspective;

// Initializes scene resources, or loads a previously saved app state.
void D3D11AvdlApplication::Load(Platform::String^ entryPoint)
{
	Array<byte>^ VSFile = LoadShaderFile("SampleVertexShader.cso");
	Array<byte>^ PSFile = LoadShaderFile("SamplePixelShader.cso");
	avdl_d3dDevice->CreateVertexShader(
		VSFile->Data,
		VSFile->Length,
		nullptr,
		&avdl_vertexShader
	);
	avdl_d3dDevice->CreatePixelShader(
		PSFile->Data,
		PSFile->Length,
		nullptr,
		&avdl_pixelShader
	);

	static const D3D11_INPUT_ELEMENT_DESC vertexDesc [] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	avdl_d3dDevice->CreateInputLayout(
		vertexDesc,
		ARRAYSIZE(vertexDesc),
		VSFile->Data,
		VSFile->Length,
		&avdl_inputLayout
	);

	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer) , D3D11_BIND_CONSTANT_BUFFER);
	avdl_d3dDevice->CreateBuffer(
		&constantBufferDesc,
		nullptr,
		&avdl_constantBuffer
	);

	// Load mesh vertices. Each vertex has a position and a color.
	static const VertexPositionColor cubeVertices[] = 
	{
		{XMFLOAT3( 0.5f, -0.5f,  0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
		{XMFLOAT3(-0.5f, -0.5f,  0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
		{XMFLOAT3( 0.0f,  0.5f,  0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f)},

		{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
		{XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
		{XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},

		{XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(0.7f, 0.5f, 0.5f)},
		{XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(0.5f, 0.7f, 0.5f)},
		{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.5f, 0.5f, 0.7f)},
	};

	D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
	vertexBufferData.pSysMem = cubeVertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
	avdl_d3dDevice->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		&avdl_vertexBuffer
	);

	CoreWindow^ Window = CoreWindow::GetForCurrentThread();
	float aspectRatio = Window->Bounds.Width /Window->Bounds.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	avdl_engine_initWorld(&engine, dd_default_world_constructor, dd_default_world_size);
	avdl_engine_setPaused(&engine, false);
}

// This method is called after the window becomes active.
void D3D11AvdlApplication::Run()
{
	while (1)
	{
		// window events
		//Window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

		// avdl update
		avdl_engine_update(&engine);

		// avdl render
		avdl_engine_draw(&engine);
	}
}

// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void D3D11AvdlApplication::Uninitialize()
{
}

// Application lifecycle event handlers.

void D3D11AvdlApplication::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() won't start until the CoreWindow is activated.
	CoreWindow::GetForCurrentThread()->Activate();
}

void D3D11AvdlApplication::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
}

void D3D11AvdlApplication::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
}

void D3D11AvdlApplication::OnPointerPressed(CoreWindow^ window, PointerEventArgs^ args) {
	avdl_input_AddInput(&engine.input,
		DD_INPUT_MOUSE_BUTTON_LEFT,
		DD_INPUT_MOUSE_TYPE_PRESSED,
		args->CurrentPoint->Position.X,
		args->CurrentPoint->Position.Y
	);
}

void D3D11AvdlApplication::OnPointerMoved(CoreWindow^ window, PointerEventArgs^ args) {
	avdl_input_AddPassiveMotion(&engine.input, args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
}

void D3D11AvdlApplication::OnPointerReleased(CoreWindow^ window, PointerEventArgs^ args) {
	avdl_input_AddInput(&engine.input,
		DD_INPUT_MOUSE_BUTTON_LEFT,
		DD_INPUT_MOUSE_TYPE_RELEASED,
		args->CurrentPoint->Position.X,
		args->CurrentPoint->Position.Y
	);
}

void D3D11AvdlApplication::OnKeyDown(CoreWindow^ window, KeyEventArgs^ args) {
	if (args->VirtualKey == VirtualKey::A
	||  args->VirtualKey == VirtualKey::GamepadDPadLeft
	||  args->VirtualKey == VirtualKey::GamepadLeftThumbstickLeft
	)
	{
		// move left
		engine.input_key = 97;
	}
	else
	if (args->VirtualKey == VirtualKey::W
	||  args->VirtualKey == VirtualKey::GamepadDPadUp
	||  args->VirtualKey == VirtualKey::GamepadLeftThumbstickUp
	)
	{
		// move up
		engine.input_key = 119;
	}
	else
	if (args->VirtualKey == VirtualKey::D
	||  args->VirtualKey == VirtualKey::GamepadDPadRight
	||  args->VirtualKey == VirtualKey::GamepadLeftThumbstickRight
	)
	{
		// move right
		engine.input_key = 100;
	}
	else
	if (args->VirtualKey == VirtualKey::S
	||  args->VirtualKey == VirtualKey::GamepadDPadDown
	||  args->VirtualKey == VirtualKey::GamepadLeftThumbstickDown
	)
	{
		// move down
		engine.input_key = 115;
	}
	else
	if (args->VirtualKey == VirtualKey::Enter
	||  args->VirtualKey == VirtualKey::GamepadA
	)
	{
		// confirm
		engine.input_key = 13;
	}
	else
	if (args->VirtualKey == VirtualKey::Escape
	||  args->VirtualKey == VirtualKey::GamepadB
	)
	{
		// confirm
		engine.input_key = 13;
	}
}

void D3D11AvdlApplication::OnKeyUp(CoreWindow^ window, KeyEventArgs^ args) {
	// keyboard A
	if(args->VirtualKey == VirtualKey::A)
	{
	}
	else
	// gamepad A
	if(args->VirtualKey == VirtualKey::GamepadA)
	{
	}
	else {
	}
}

// Window event handlers.

void D3D11AvdlApplication::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	avdl_graphics_d3d11_SetWindow();
	CoreWindow^ Window = CoreWindow::GetForCurrentThread();
	dd_width = Window->Bounds.Width;
	dd_height = Window->Bounds.Height;
	avdl_engine_resize(&engine, dd_width, dd_height);
	/*
	// Set the 3D rendering viewport to target the entire window.
	D3D11_VIEWPORT avdl_screenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		Window->Bounds.Width,
		Window->Bounds.Height
		);

	avdl_d3dContext->RSSetViewports(1, &avdl_screenViewport);
	*/
}

void D3D11AvdlApplication::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
}

void D3D11AvdlApplication::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

// DisplayInformation event handlers.

void D3D11AvdlApplication::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
}

void D3D11AvdlApplication::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
}

void D3D11AvdlApplication::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
}

extern "C" void avdl_graphics_direct3d11_drawMesh(struct dd_meshColour *m, struct dd_matrix *matrix) {

	/*
	 * Matrix
	 */
	XMMATRIX orientationMatrix2(
		matrix->cell
	);
	XMStoreFloat4x4(&avdl_constantBufferData.model, orientationMatrix2);

	// Prepare the constant buffer to send it to the graphics device.
	avdl_d3dContext->UpdateSubresource1(
		avdl_constantBuffer.Get(),
		0,
		NULL,
		&avdl_constantBufferData,
		0,
		0,
		0
	);

	ComPtr<ID3D11Buffer> vertexBuffer = (ID3D11Buffer *) m->vertexBuffer;
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	avdl_d3dContext->IASetVertexBuffers(
		0,
		1,
		//avdl_vertexBuffer.GetAddressOf(),
		vertexBuffer.GetAddressOf(),
		&stride,
		&offset
	);

	avdl_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	avdl_d3dContext->IASetInputLayout(avdl_inputLayout.Get());

	// Attach our vertex shader.
	avdl_d3dContext->VSSetShader(
		avdl_vertexShader.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	avdl_d3dContext->VSSetConstantBuffers1(
		0,
		1,
		avdl_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
		);

	// Attach our pixel shader.
	avdl_d3dContext->PSSetShader(
		avdl_pixelShader.Get(),
		nullptr,
		0
		);

	// Draw cube
	avdl_d3dContext->Draw(
		//9,
		//3,
		m->parent.vcount,
		0
	);
}

extern "C" const char* getAppLocation(void) {
	static char filepath[1001];
	Platform::String ^ locationString = Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
	WideCharToMultiByte(CP_UTF8, 0, locationString->Begin(), -1, filepath, 1000, nullptr, nullptr);
	return filepath;
}
using namespace Windows::Storage;
using namespace Streams;
extern "C" FILE *avdl_filetomesh_openFile(char *filename) {

	const char *filePath = getAppLocation();
	char combined[1000];
	strncpy_s(combined, 1000, filePath, 500);
	strncat_s(combined, 1000, "\\", 500);
	strncat_s(combined, 1000, filename, 500);
	size_t size = strlen(combined) * 2 + 2;

	FILE *fp_testFile = NULL;
	errno_t err;

	err = fopen_s(&fp_testFile, combined, "rb");
	if (err == 0)
	{
		return fp_testFile;
	}

	return 0;
}

int once = 0;
extern "C" void avdl_graphics_direct3d11_setVertexBuffer(struct dd_meshColour *m) {

	m->vertexBuffer = new ComPtr<ID3D11Buffer>();
	//ComPtr<ID3D11Buffer> vertexBuffer;
	// Load mesh vertices. Each vertex has a position and a color.
	static const VertexPositionColor cubeVertices[] = 
	{
		{XMFLOAT3( 0.5f, -0.5f,  0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
		{XMFLOAT3(-0.5f, -0.5f,  0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
		{XMFLOAT3( 0.0f,  0.5f,  0.0f), XMFLOAT3(1.0f, 0.0f, 1.0f)},

		{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
		{XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		{XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},

		{XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1.7f, 0.5f, 0.5f)},
		{XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1.5f, 0.7f, 0.5f)},
		{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(1.5f, 0.5f, 0.7f)},
	};
	static const VertexPositionColor cubeVertices2[] = 
	{
		{XMFLOAT3( 0.5f, -0.5f,  0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
		{XMFLOAT3(-0.5f, -0.5f,  0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
		{XMFLOAT3( 0.0f,  0.5f,  0.0f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
	};
	//VertexPositionColor *cubeVertices3 = (VertexPositionColor *)malloc(sizeof(struct VertexPositionColor) *3);
	VertexPositionColor *cubeVertices3 = (VertexPositionColor *)malloc(sizeof(struct VertexPositionColor) *m->parent.vcount);
	/*
	cubeVertices3[0].pos = XMFLOAT3( 0.5f, -0.5f,  0.0f);
	cubeVertices3[1].pos = XMFLOAT3(-0.5f, -0.5f,  0.0f);
	cubeVertices3[2].pos = XMFLOAT3( 0.0f,  0.5f,  0.0f);
	cubeVertices3[0].color = XMFLOAT3(1.0f, 0.0f, 0.0f);
	cubeVertices3[1].color = XMFLOAT3(0.0f, 1.0f, 0.0f);
	cubeVertices3[2].color = XMFLOAT3(0.0f, 0.0f, 1.0f);
	*/
	//for (int i = 0; i < m->parent.vcount; i += 3) {
	for (int i = 0; i < m->parent.vcount; i++) {
		cubeVertices3[i].pos = XMFLOAT3(
				m->parent.v[i*3 +0],
				m->parent.v[i*3 +1],
				m->parent.v[i*3 +2]
		);
		cubeVertices3[i].color = XMFLOAT3(
				m->c[i*3 +0],
				m->c[i*3 +1],
				m->c[i*3 +2]
		);
	}

	D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
	vertexBufferData.pSysMem = cubeVertices3;
	//vertexBufferData.pSysMem = cubeVertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPositionColor) *m->parent.vcount, D3D11_BIND_VERTEX_BUFFER);
	//CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPositionColor) *3, D3D11_BIND_VERTEX_BUFFER);
	//CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
	avdl_d3dDevice->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		(ID3D11Buffer**)&m->vertexBuffer
	);
}

#ifdef __cplusplus
}
#endif

#endif
