// FBRendererTest.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "FBRendererTest.h"
#include "../FBRenderer.h"
#include "../../FBCommon/glm.h"
#include <chrono>
#include <thread>
#include <array>

#define MAX_LOADSTRING 100

fb::IRenderer* gRenderer = nullptr;
struct MeshGeometry* gBoxMesh = nullptr;
float Radius = 10.0f;
float Phi = 0.f;
float Theta = 0.f;
glm::mat4 WorldMat, ViewMat, ProjMat;
fb::IUploadBuffer* UploadBuffer = nullptr;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
bool				BuildBoxGeometry();
void Update(float dt);

void Test()
{
	auto float16size = sizeof(float[16]);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	Test();
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FBRENDERERTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	BuildBoxGeometry();

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FBRENDERERTEST));

	MSG msg = {0};
    // Main message loop:
	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			static auto time = std::chrono::high_resolution_clock::now();
			auto dt = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - time).count();
			Update(dt);
			if (gRenderer)
			{
				gRenderer->Draw(dt);
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(10ms);
			}
		}
	}

	delete gBoxMesh; gBoxMesh = nullptr;
	delete UploadBuffer; UploadBuffer = nullptr;
	gRenderer->Finalize(); gRenderer = nullptr;
	

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FBRENDERERTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_FBRENDERERTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   gRenderer = fb::InitRenderer(fb::RendererType::D3D12, (void*)hWnd);
   UploadBuffer = gRenderer->CreateUploadBuffer(sizeof(float[16]), 1, true);

   return gRenderer != nullptr;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool Resizing = false;
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

	case WM_ENTERSIZEMOVE:
		Resizing = true;
		break;

	case WM_EXITSIZEMOVE:
		Resizing = false;
		gRenderer->OnResized();
		break;

	case WM_SIZE:
	{
		// Save the new client area dimensions.
		UINT clientWidth = LOWORD(lParam);
		UINT clientHeight = HIWORD(lParam);

		if (gRenderer)
		{
			if (!Resizing)
			{
				gRenderer->OnResized();
			}
		}
		return 0;
	}
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


struct Vertex
{
	float X, Y, Z;
	float R, G, B, A;
};

struct MeshGeometry
{
	// Give it a name so we can look it up by name.
	std::string Name;

	fb::IVertexBufferIntPtr VertexBuffer;
	fb::IIndexBufferIntPtr IndexBuffer;

	bool IsValid() const noexcept
	{
		return VertexBuffer != nullptr;
	}
};

bool BuildBoxGeometry()
{
	std::array<Vertex, 8> vertices =
	{
		Vertex{-1.0f, -1.0f, -1.0f,		1.0f, 1.0f, 1.0f, 1.0f},
		Vertex{-1.0f, +1.0f, -1.0f,		0.0f, 0.0f, 0.0f, 1.0f},
		Vertex{+1.0f, +1.0f, -1.0f,		1.0f, 0.0f, 0.0f, 1.0f},
		Vertex{+1.0f, -1.0f, -1.0f,		0.0f, 1.0f, 0.0f, 1.0f},
		Vertex{-1.0f, -1.0f, +1.0f,		0.0f, 0.0f, 1.0f, 1.0f},
		Vertex{-1.0f, +1.0f, +1.0f,		1.0f, 1.0f, 0.0f, 1.0f}, // yellow
		Vertex{+1.0f, +1.0f, +1.0f,		0.0f, 1.0f, 1.0f, 1.0f}, // cyan
		Vertex{+1.0f, -1.0f, +1.0f,		1.0f, 0.0f, 1.0f, 1.0f}, // magenta
	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	gBoxMesh = new MeshGeometry;
	gBoxMesh->Name = "boxGeo";
	gBoxMesh->VertexBuffer = gRenderer->CreateVertexBuffer(vertices.data(), vbByteSize, sizeof(Vertex), false);
	gBoxMesh->IndexBuffer = gRenderer->CreateIndexBuffer(indices.data(), ibByteSize, fb::EDataFormat::R16_UINT, false);
	return gBoxMesh->IsValid();
}

void Update(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = Radius * sinf(Phi) * cosf(Theta);
	float z = Radius * sinf(Phi) * sinf(Theta);
	float y = Radius * cosf(Phi);

	// Build the view matrix.
	glm::vec3 eyePos(x, y, z);
	glm::vec3 target(0, 0, 0);
	ViewMat = glm::lookAtLH(eyePos, target, glm::vec3(0, 1, 0));
	auto wvp = WorldMat * ViewMat * ProjMat;

	auto float16size = sizeof(float[16]);
	assert(sizeof(wvp) == sizeof(float[16]));
	UploadBuffer->CopyData(0, &wvp);
	// Update the constant buffer with the latest worldViewProj matrix.
}