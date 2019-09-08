// FBRendererTest.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "FBRendererTest.h"
#include "GeometryGenerator.h"
#include "../FBRenderer.h"
#include "../RenderItem.h"
#include "../Colors.h"
#include "../../FBCommon/glm.h"
#include "../../FBCommon/AABB.h"

#include <chrono>
#include <thread>
#include <array>
#include <unordered_map>

#define MAX_LOADSTRING 100

fb::IRenderer* gRenderer = nullptr;
struct MeshGeometry* gBoxMesh = nullptr;
float Radius = 5.0f;
float Phi = glm::four_over_pi<float>();
float Theta = 1.5f * glm::pi<float>();
glm::mat4 WorldMat(1.0f), ViewMat(1.0f), ProjMat(1.0f);
fb::IUploadBuffer* PerPassCBs = nullptr;
std::vector<fb::IUploadBuffer*> PerObjectCBs;
POINT LastMousePos;
fb::PSOID SimpleBoxPSO;
UINT CurrentFrameResourceIndex = 0;
std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> Geometries;
std::vector<std::unique_ptr<fb::RenderItem>> AllRitems;
std::vector<fb::RenderItem*> OpaqueRitems;
UINT PassCbvOffset = 0;
std::unordered_map<std::string, fb::IShaderIPtr> Shaders;
fb::IRootSignatureIPtr SimpleBoxRootSig;
fb::IRootSignatureIPtr MultiDrawRootSig;
// Global Variables:
HINSTANCE hInst;       // current instance
HWND WindowHandle;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
bool				BuildBoxGeometry();
void				Update(float dt);

void OnMouseMove(WPARAM btnState, int x, int y);
void OnMouseDown(WPARAM btnState, int x, int y);

void BuildShadersAndInputLayout();

void BuildPSO();

void BuildShapeGeometry();
void BuildRenderItems();
void BuildDescriptorHeap();
void BuildConstantBuffers();
void Draw();


struct Vertex
{
	float X, Y, Z;
	float R, G, B, A;
};

struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;
	// Bounding box of the geometry defined by this submesh. 
	// This is used in later chapters of the book.
	fb::AABB Bounds;
};

struct MeshGeometry
{
	// Give it a name so we can look it up by name.
	std::string Name;

	fb::IVertexBufferIPtr VertexBuffer;
	fb::IIndexBufferIPtr IndexBuffer;

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

	bool IsValid() const noexcept
	{
		return VertexBuffer != nullptr;
	}
};

struct ObjectConstants
{
	glm::mat4 World = glm::mat4(1.0f);
};

struct PassConstants
{
	glm::mat4 View = glm::mat4(1.0f);
	glm::mat4 InvView = glm::mat4(1.0f);
	glm::mat4 Proj = glm::mat4(1.0f);
	glm::mat4 InvProj = glm::mat4(1.0f);
	glm::mat4 ViewProj = glm::mat4(1.0f);
	glm::mat4 InvViewProj = glm::mat4(1.0f);
	glm::vec3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	glm::vec2 RenderTargetSize = { 0.0f, 0.0f };
	glm::vec2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;
};


void Test()
{
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
			CurrentFrameResourceIndex = (CurrentFrameResourceIndex + 1) % gRenderer->GetNumSwapchainBuffers();
		}
	}

	delete gBoxMesh; gBoxMesh = nullptr;
	delete PerPassCBs; PerPassCBs = nullptr;
	fb::FinalizeRenderer(gRenderer);

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

   WindowHandle = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!WindowHandle)
   {
      return FALSE;
   }

   ShowWindow(WindowHandle, nCmdShow);
   UpdateWindow(WindowHandle);

   gRenderer = fb::InitRenderer(fb::RendererType::D3D12, (void*)WindowHandle);

   gRenderer->TempResetCommandList();

   BuildShapeGeometry();

   BuildRenderItems();

   BuildDescriptorHeap();

   BuildConstantBuffers();

   BuildShadersAndInputLayout();
   SimpleBoxRootSig = gRenderer->CreateRootSignature("DTable,1,0");

   BuildPSO();

   gRenderer->RegisterDrawCallback(Draw);
   BuildBoxGeometry();
   gRenderer->TempCloseCommandList(true);
   auto clientWidth = gRenderer->GetBackbufferWidth();
   auto clientHeight = gRenderer->GetBackbufferHeight();
   ProjMat = glm::perspectiveFovLH(0.25f * glm::pi<float>(), (float)clientWidth, (float)clientHeight, 1.0f, 1000.0f);

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
				ProjMat = glm::perspectiveFovLH(0.25f * glm::pi<float>(), (float)clientWidth, (float)clientHeight, 1.0f, 1000.0f);
			}
		}
		return 0;
	}
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;
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
	auto wvp = ProjMat * ViewMat * WorldMat;
	wvp = glm::transpose(wvp);
	auto float16size = sizeof(float[16]);
	assert(sizeof(wvp) == sizeof(float[16]));

	auto& curFR = gRenderer->GetFrameResource_WaitAvailable(CurrentFrameResourceIndex);

	// TODO : This should be ViewProject.
	curFR.CBPerFrame->CopyData(0, &wvp);
}
void OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		
		// Make each pixel correspond to a quarter of a degree.
		float dx = glm::radians(0.25f * static_cast<float>(x - LastMousePos.x));
		float dy = glm::radians(0.25f * static_cast<float>(y - LastMousePos.y));

		// Update angles based on input to orbit camera around box.
		Theta += dx;
		Phi += dy;

		// Restrict the angle mPhi.
		Phi = glm::clamp(Phi, 0.1f, glm::pi<float>() - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f * static_cast<float>(x - LastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - LastMousePos.y);

		// Update the camera radius based on input.
		Radius += dx - dy;

		// Restrict the radius.
		Radius = glm::clamp(Radius, 3.0f, 15.0f);
	}

	LastMousePos.x = x;
	LastMousePos.y = y;
}

void OnMouseDown(WPARAM btnState, int x, int y)
{
	LastMousePos.x = x;
	LastMousePos.y = y;

	
	SetCapture(WindowHandle);
}

fb::IShaderIPtr VS, PS;
std::vector<fb::FInputElementDesc> InputLayout;
void BuildShadersAndInputLayout()
{
	Shaders["standardVS"] = gRenderer->CompileShader("Shaders/SimpleShader.hlsl", nullptr, 0, fb::EShaderType::VertexShader, "VS");
	Shaders["opaquePS"] = gRenderer->CompileShader("Shaders/SimpleShader.hlsl", nullptr, 0, fb::EShaderType::PixelShader, "PS");

	InputLayout = {
		{ "POSITION", 0, fb::EDataFormat::R32G32B32_FLOAT, 0, 0, fb::EInputClassification::PerVertexData, 0 },
		{ "COLOR", 0, fb::EDataFormat::R32G32B32A32_FLOAT, 0, 12, fb::EInputClassification::PerVertexData, 0 },
	};
}

void BuildPSO()
{
	fb::FPSODesc psoDesc;
	psoDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
	psoDesc.pRootSignature = SimpleBoxRootSig;
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(VS->GetByteCode()),
		VS->Size()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(PS->GetByteCode()),
		PS->Size()
	};
	//psoDesc.RasterizerState
	//psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.PrimitiveTopologyType = fb::EPrimitiveTopologyType::TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = gRenderer->GetBackBufferFormat();
	psoDesc.SampleDesc.Count = gRenderer->GetSampleCount();
	psoDesc.SampleDesc.Quality = gRenderer->GetMsaaQuality();
	psoDesc.DSVFormat = gRenderer->GetDepthStencilFormat();
	SimpleBoxPSO = gRenderer->CreateGraphicsPipelineState(psoDesc);
}

void BuildRootSignature()
{
	MultiDrawRootSig = gRenderer->CreateRootSignature("DTable,1,0;DTable,1,1;");
}

void BuildShapeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();

	// Define the SubmeshGeometry that cover different 
	// regions of the vertex/index buffers.

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	auto totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].X = box.Vertices[i].Position.x;
		vertices[k].Y = box.Vertices[i].Position.y;
		vertices[k].Z = box.Vertices[i].Position.z;
		vertices[k].R = fb::Colors::DarkGreen[0];
		vertices[k].G = fb::Colors::DarkGreen[1];
		vertices[k].B = fb::Colors::DarkGreen[2];
		vertices[k].A = fb::Colors::DarkGreen[3];
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].X = grid.Vertices[i].Position.x;
		vertices[k].Y = grid.Vertices[i].Position.y;
		vertices[k].Z = grid.Vertices[i].Position.z;
		vertices[k].R = fb::Colors::ForestGreen[0];
		vertices[k].G = fb::Colors::ForestGreen[1];
		vertices[k].B = fb::Colors::ForestGreen[2];
		vertices[k].A = fb::Colors::ForestGreen[3];
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].X = sphere.Vertices[i].Position.x;
		vertices[k].Y = sphere.Vertices[i].Position.y;
		vertices[k].Z = sphere.Vertices[i].Position.z;
		vertices[k].R = fb::Colors::Crimson[0];
		vertices[k].G = fb::Colors::Crimson[1];
		vertices[k].B = fb::Colors::Crimson[2];
		vertices[k].A = fb::Colors::Crimson[3];
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].X = cylinder.Vertices[i].Position.x;
		vertices[k].Y = cylinder.Vertices[i].Position.y;
		vertices[k].Z = cylinder.Vertices[i].Position.z;

		vertices[k].R = fb::Colors::SteelBlue[0];
		vertices[k].G = fb::Colors::SteelBlue[1];
		vertices[k].B = fb::Colors::SteelBlue[2];
		vertices[k].A = fb::Colors::SteelBlue[3];
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	geo->VertexBuffer = gRenderer->CreateVertexBuffer(vertices.data(), vbByteSize, sizeof(Vertex), false);
	assert(geo->VertexBuffer);

	geo->IndexBuffer = gRenderer->CreateIndexBuffer(indices.data(), ibByteSize, fb::EDataFormat::R16_UINT, false);

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;

	Geometries[geo->Name] = std::move(geo);
}

void BuildRenderItems()
{
	auto boxRitem = std::make_unique<fb::RenderItem>();
	boxRitem->World = glm::translate(glm::vec3{0.0f, 0.5f, 0.0f}) * glm::scale(glm::vec3{ 2.0f, 2.0f, 2.0f });
	boxRitem->ConstantBufferIndex = 0;
	auto& geo = Geometries["shapeGeo"];
	boxRitem->VB = geo->VertexBuffer;
	boxRitem->IB = geo->IndexBuffer;
	boxRitem->PrimitiveTopology = fb::EPrimitiveTopology::TRIANGLELIST;
	boxRitem->IndexCount = geo->DrawArgs["box"].IndexCount;
	boxRitem->StartIndexLocation = geo->DrawArgs["box"].StartIndexLocation;
	boxRitem->BaseVertexLocation = geo->DrawArgs["box"].BaseVertexLocation;
	AllRitems.push_back(std::move(boxRitem));

	auto gridRitem = std::make_unique<fb::RenderItem>();
	gridRitem->World = glm::mat4(1.f);
	gridRitem->ConstantBufferIndex = 1;
	gridRitem->VB = geo->VertexBuffer;
	gridRitem->IB = geo->IndexBuffer;
	gridRitem->PrimitiveTopology = fb::EPrimitiveTopology::TRIANGLELIST;
	gridRitem->IndexCount = geo->DrawArgs["grid"].IndexCount;
	gridRitem->StartIndexLocation = geo->DrawArgs["grid"].StartIndexLocation;
	gridRitem->BaseVertexLocation = geo->DrawArgs["grid"].BaseVertexLocation;
	AllRitems.push_back(std::move(gridRitem));

	UINT objCBIndex = 2;
	for (int i = 0; i < 5; ++i)
	{
		auto leftCylRitem = std::make_unique<fb::RenderItem>();
		auto rightCylRitem = std::make_unique<fb::RenderItem>();
		auto leftSphereRitem = std::make_unique<fb::RenderItem>();
		auto rightSphereRitem = std::make_unique<fb::RenderItem>();

		glm::mat4 leftCylWorld = glm::translate(glm::vec3(-5.0f, 1.5f, -10.0f + i * 5.0f));
		glm::mat4 rightCylWorld = glm::translate(glm::vec3(+5.0f, 1.5f, -10.0f + i * 5.0f));

		glm::mat4 leftSphereWorld = glm::translate(glm::vec3(-5.0f, 3.5f, -10.0f + i * 5.0f));
		glm::mat4 rightSphereWorld = glm::translate(glm::vec3(+5.0f, 3.5f, -10.0f + i * 5.0f));

		leftCylRitem->World = leftCylWorld;
		leftCylRitem->ConstantBufferIndex = objCBIndex++;
		leftCylRitem->PrimitiveTopology = fb::EPrimitiveTopology::TRIANGLELIST;
		leftCylRitem->IndexCount = geo->DrawArgs["cylinder"].IndexCount;
		leftCylRitem->StartIndexLocation = geo->DrawArgs["cylinder"].StartIndexLocation;
		leftCylRitem->BaseVertexLocation = geo->DrawArgs["cylinder"].BaseVertexLocation;

		rightCylRitem->World = rightCylWorld;
		rightCylRitem->ConstantBufferIndex = objCBIndex++;
		rightCylRitem->PrimitiveTopology = fb::EPrimitiveTopology::TRIANGLELIST;
		rightCylRitem->IndexCount = geo->DrawArgs["cylinder"].IndexCount;
		rightCylRitem->StartIndexLocation = geo->DrawArgs["cylinder"].StartIndexLocation;
		rightCylRitem->BaseVertexLocation = geo->DrawArgs["cylinder"].BaseVertexLocation;

		leftSphereRitem->World = leftSphereWorld;
		leftSphereRitem->ConstantBufferIndex = objCBIndex++;
		leftSphereRitem->PrimitiveTopology = fb::EPrimitiveTopology::TRIANGLELIST;
		leftSphereRitem->IndexCount = geo->DrawArgs["sphere"].IndexCount;
		leftSphereRitem->StartIndexLocation = geo->DrawArgs["sphere"].StartIndexLocation;
		leftSphereRitem->BaseVertexLocation = geo->DrawArgs["sphere"].BaseVertexLocation;

		rightSphereRitem->World = rightSphereWorld;
		rightSphereRitem->ConstantBufferIndex = objCBIndex++;
		rightSphereRitem->PrimitiveTopology = fb::EPrimitiveTopology::TRIANGLELIST;
		rightSphereRitem->IndexCount = geo->DrawArgs["sphere"].IndexCount;
		rightSphereRitem->StartIndexLocation = geo->DrawArgs["sphere"].StartIndexLocation;
		rightSphereRitem->BaseVertexLocation = geo->DrawArgs["sphere"].BaseVertexLocation;

		AllRitems.push_back(std::move(leftCylRitem));
		AllRitems.push_back(std::move(rightCylRitem));
		AllRitems.push_back(std::move(leftSphereRitem));
		AllRitems.push_back(std::move(rightSphereRitem));
	}

	// All the render items are opaque.
	for (auto& e : AllRitems)
		OpaqueRitems.push_back(e.get());
}

void BuildDescriptorHeap()
{
	UINT objCount = (UINT)OpaqueRitems.size();

	auto numFrameResources = gRenderer->GetNumSwapchainBuffers();
	// Need a CBV descriptor for each object for each frame resource,
	// +1 for the perPass CBV for each frame resource.
	UINT numDescriptors = (objCount + 1) * numFrameResources;

	// Save an offset to the start of the pass CBVs.  These are the last 3 descriptors.
	PassCbvOffset = objCount * numFrameResources;

	gRenderer->PrepareDescriptorHeap(fb::EDescriptorHeapType::Default, numDescriptors);
}

void BuildConstantBuffers()
{
	UINT objCount = (UINT)OpaqueRitems.size();
	auto numSwapchains = gRenderer->GetNumSwapchainBuffers();
	// Need a CBV descriptor for each object for each frame resource.
	for (int frameIndex = 0; frameIndex < numSwapchains; ++frameIndex)
	{
		auto& frameResource = gRenderer->GetFrameResource_WaitAvailable(frameIndex);
		frameResource.CBPerObject = gRenderer->CreateUploadBuffer(sizeof(ObjectConstants), objCount, true, fb::EDescriptorHeapType::Default);
		for (UINT i = 0; i < objCount; ++i)
		{
			int heapIndex = frameIndex * objCount + i;
			frameResource.CBPerObject->CreateCBV(i, fb::EDescriptorHeapType::Default, heapIndex);
		}
	}

	auto numFrameResources = gRenderer->GetNumSwapchainBuffers();
	for (int frameIndex = 0; frameIndex < numFrameResources; ++frameIndex)
	{
		auto& curFR = gRenderer->GetFrameResource_WaitAvailable(CurrentFrameResourceIndex);
		curFR.CBPerFrame = gRenderer->CreateUploadBuffer(sizeof(PassConstants), 1, true, fb::EDescriptorHeapType::Default);
		curFR.CBPerFrame->CreateCBV(0, fb::EDescriptorHeapType::Default, PassCbvOffset + frameIndex);
	}
}

void Draw()
{
	gRenderer->TempBindDescriptorHeap(fb::EDescriptorHeapType::Default);
	SimpleBoxRootSig->Bind();

	gRenderer->TempBindVertexBuffer(gBoxMesh->VertexBuffer);
	gRenderer->TempBindIndexBuffer(gBoxMesh->IndexBuffer);
	gRenderer->TempSetPrimitiveTopology(fb::EPrimitiveTopology::TRIANGLELIST);
	gRenderer->TempBindRootDescriptorTable(0, fb::EDescriptorHeapType::Default);
	gRenderer->TempDrawIndexedInstanced(gBoxMesh->IndexBuffer->GetElementCount());
}