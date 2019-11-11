// FBRendererTest.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "FBRendererTest.h"
#include "GeometryGenerator.h"
#include "../FBRenderer.h"
#include "../ITexture.h"
#include "RenderItem.h"
#include "../Colors.h"
#include "../AxisRenderer.h"
#include "../../FBCommon/glm.h"
#include "../../FBCommon/AABB.h"
#include "../../FBCommon/Utility.h"
#include "FrameResource.h"
#include "Waves.h"
#include "Material.h"

#define MAX_LOADSTRING 100

struct Vertex
{
	glm::vec3 Pos;
	glm::vec3 Normal;
	glm::vec2 TexC;
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

fb::IRenderer* gRenderer = nullptr;
//struct MeshGeometry* gBoxMesh = nullptr;
float Radius = 30.0f;
float Phi = glm::four_over_pi<float>();
float Theta = 1.5f * glm::pi<float>();
glm::mat4 WorldMat(1.0f), ViewMat(1.0f), ProjMat(1.0f);
fb::IUploadBuffer* PerPassCBs = nullptr;
POINT LastMousePos;
fb::PSOID SimpleBoxPSO;
fb::PSOID SimpleBoxPSOWireframe;
UINT CurrentFrameResourceIndex = 0;
std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> Geometries;
std::unordered_map<std::string, std::unique_ptr<Material>> Materials;
std::unordered_map<std::string, fb::ITextureIPtr> Textures;
std::vector<std::unique_ptr<RenderItem>> AllRitems;
fb::IDescriptorHeapIPtr DescriptorHeap;
UINT Num_CBV_SRV_UAV = 1;
enum class ERenderLayer : int
{
	Opaque = 0,
	Count
};

std::vector<fb::FInputElementDesc> InputLayout;
std::vector<RenderItem*> RenderItemLayers[(int)ERenderLayer::Count];

UINT PassCbvOffset = 0;
std::unordered_map<std::string, fb::IShaderIPtr> Shaders;
fb::IRootSignatureIPtr SimpleBoxRootSig;
fb::IRootSignatureIPtr CBVRootSig;
fb::IRootSignatureIPtr LightingRootSig;
bool IsWireframe = false;
// Global Variables:
HINSTANCE hInst;       // current instance
HWND WindowHandle;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
UINT Width = 200;
UINT Height = 100;
Waves* gWaves = nullptr;

RenderItem* gWavesRitem = nullptr;
fb::AxisRenderer* gAxisRenderer = nullptr;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
//bool				BuildBoxGeometry();
void				Update(float dt);

void OnMouseMove(WPARAM btnState, int x, int y);
void OnMouseDown(WPARAM btnState, int x, int y);
void OnMouseUp(WPARAM btnState, int x, int y);

void BuildShadersAndInputLayout();
void DestroyShaders();

void BuildLandGeometry();
void DestroyGeometries();

void BuildWavesGeometryBuffers();

void BuildPSO();

void BuildShapeGeometry();
void BuildRenderItems();
void BuildWaves();
void LoadTextures();
void BuildShaderResourceView();
void Draw(float dt);
void OnKeyboardInput();
void BuildMaterials();

void DestroyRenderItems();


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

	std::wofstream out("log.txt");
	
	std::wstreambuf* coutbuf = std::wcout.rdbuf(); //save old buf
	std::wcout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

    // TODO: Place code here.
	Test();
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FBRENDERERTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
		std::wcout << L"InitInstance() failed!" << std::endl;
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
				Draw(dt);
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(5ms);
			}
			CurrentFrameResourceIndex = (CurrentFrameResourceIndex + 1) % gRenderer->GetNumSwapchainBuffers();
		}
	}

	gRenderer->SignalFence();
	gRenderer->FlushCommandQueue();
	//delete gBoxMesh; gBoxMesh = nullptr;
	DestroyRenderItems();
	DestroyGeometries();
	delete gWaves; gWaves = nullptr;
	delete PerPassCBs; PerPassCBs = nullptr;
	DestroyFrameResources();
	LightingRootSig.reset();
	CBVRootSig.reset();
	SimpleBoxRootSig.reset();
	delete gAxisRenderer; gAxisRenderer = nullptr;
	DestroyShaders();
	fb::FinalizeRenderer(gRenderer);
	std::wcout << L"Renderer Finalized!" << std::endl;
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
   RECT r;
   GetClientRect(WindowHandle, &r);
   Width = r.right - r.left;
   Height = r.bottom - r.top;
   gRenderer = fb::InitRenderer(fb::RendererType::D3D12, (void*)WindowHandle);

   BuildFrameResources();
   BuildMaterials();

   gRenderer->ResetCommandList(nullptr, 0);
   std::wcout << L"Reset Command List" << std::endl;

   BuildShapeGeometry();

   LoadTextures();

   BuildShaderResourceView();

   BuildWaves();

   BuildShadersAndInputLayout();

   BuildLandGeometry();

   BuildWavesGeometryBuffers();

   BuildRenderItems();

   BuildConstantBuffers((int)RenderItemLayers[(int)ERenderLayer::Opaque].size());
   
   SimpleBoxRootSig = gRenderer->CreateRootSignature("DTable,1,0,CBV");
   CBVRootSig = gRenderer->CreateRootSignature("RootCBV,0;RootCBV,1;");
   // per object, material, per frame, texture
   LightingRootSig = gRenderer->CreateRootSignature("RootCBV,0;RootCBV,1;RootCBV,2;DTable,1,0,SRV");

   std::wcout << L"Root sig." << std::endl;

   BuildPSO();

   std::wcout << L"Build PSO." << std::endl;

   //BuildBoxGeometry();

   gAxisRenderer = new fb::AxisRenderer(gRenderer, 100, 100, 300, 300);
   gAxisRenderer->SetShaders(Shaders["axisVS"], Shaders["axisPS"]);
   
   gRenderer->CloseCommandList();
   gRenderer->ExecuteCommandList();
   gRenderer->SignalFence();
   gRenderer->FlushCommandQueue();
   ProjMat = glm::perspectiveFov(0.25f * glm::pi<float>(), (float)Width, (float)Height, 1.0f, 1000.0f);
   
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
		Width = LOWORD(lParam);
		Height = HIWORD(lParam);

		if (gRenderer)
		{
			if (!Resizing)
			{
				gRenderer->OnResized();
				ProjMat = glm::perspectiveFov(0.25f * glm::pi<float>(), (float)Width, (float)Height, 1.0f, 1000.0f);
			}
		}
		break;
	}
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
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




//bool BuildBoxGeometry()
//{
//	std::array<Vertex, 8> vertices =
//	{
//		Vertex{-1.0f, -1.0f, -1.0f,		1.0f, 1.0f, 1.0f, 1.0f},
//		Vertex{-1.0f, +1.0f, -1.0f,		0.0f, 0.0f, 0.0f, 1.0f},
//		Vertex{+1.0f, +1.0f, -1.0f,		1.0f, 0.0f, 0.0f, 1.0f},
//		Vertex{+1.0f, -1.0f, -1.0f,		0.0f, 1.0f, 0.0f, 1.0f},
//		Vertex{-1.0f, -1.0f, +1.0f,		0.0f, 0.0f, 1.0f, 1.0f},
//		Vertex{-1.0f, +1.0f, +1.0f,		1.0f, 1.0f, 0.0f, 1.0f}, // yellow
//		Vertex{+1.0f, +1.0f, +1.0f,		0.0f, 1.0f, 1.0f, 1.0f}, // cyan
//		Vertex{+1.0f, -1.0f, +1.0f,		1.0f, 0.0f, 1.0f, 1.0f}, // magenta
//	};
//
//	std::array<std::uint16_t, 36> indices =
//	{
//		// front face
//		0, 1, 2,
//		0, 2, 3,
//
//		// back face
//		4, 6, 5,
//		4, 7, 6,
//
//		// left face
//		4, 5, 1,
//		4, 1, 0,
//
//		// right face
//		3, 2, 6,
//		3, 6, 7,
//
//		// top face
//		1, 5, 6,
//		1, 6, 2,
//
//		// bottom face
//		4, 0, 3,
//		4, 3, 7
//	};
//
//	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
//	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);
//
//	gBoxMesh = new MeshGeometry;
//	gBoxMesh->Name = "boxGeo";
//	gBoxMesh->VertexBuffer = gRenderer->CreateVertexBuffer(vertices.data(), vbByteSize, sizeof(Vertex), false);
//	gBoxMesh->IndexBuffer = gRenderer->CreateIndexBuffer(indices.data(), ibByteSize, fb::EDataFormat::R16_UINT, false);
//	return gBoxMesh->IsValid();
//}

void UpdateObjectCBs(float dt, FFrameResource& curFR)
{
	auto currObjectCB = GetFrameResource(CurrentFrameResourceIndex).CBPerObject;
	for (auto& e : AllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if (e->NumFramesDirty > 0)
		{
			ObjectConstants objConstants;
			objConstants.World = glm::transpose(e->World);

			currObjectCB->CopyData(e->ObjectCBIndex, &objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}	
}

void UpdateMaterialCBs(float dt, FFrameResource& curFR)
{
	auto currMaterialCB = curFR.CBPerMaterial;
	for (auto& e : Materials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			glm::mat4 matTransform = glm::transpose(mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;

			currMaterialCB->CopyData(mat->MatCBIndex, &matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void UpdateWaves(float dt)
{
	static float totalTime = 0;
	totalTime += dt;
	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;
	if ((totalTime - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		int i = fb::Rand(4, gWaves->RowCount() - 5);
		int j = fb::Rand(4, gWaves->ColumnCount() - 5);

		float r = fb::RandF(0.2f, 0.5f);

		gWaves->Disturb(i, j, r);
	}

	// Update the wave simulation.
	gWaves->Update(dt);
	auto& curFR = GetFrameResource(CurrentFrameResourceIndex);
	// Update the wave vertex buffer with the new solution.
	auto currWavesVB = curFR.WavesVB.get();
	for (int i = 0; i < gWaves->VertexCount(); ++i)
	{
		Vertex v;

		v.Pos = gWaves->Position(i);
		v.Normal = gWaves->Normal(i);

		currWavesVB->CopyData(i, &v);
	}

	// Set the dynamic VB of the wave renderitem to the current frame VB.
	if (gWavesRitem)
		gWavesRitem->VB->FromUploadBuffer(currWavesVB);
}

glm::vec3 SphericalToCartesian(float radius, float theta, float phi)
{
	return glm::vec3(
		radius * sinf(phi) * cosf(theta),
		radius * cosf(phi),
		radius * sinf(phi) * sinf(theta));
}

void Update(float dt)
{
	OnKeyboardInput();

	auto& curFR = GetFrameResource_WaitAvailable(CurrentFrameResourceIndex);

	// Convert Spherical to Cartesian coordinates.
	float x = Radius * sinf(Phi) * cosf(Theta);
	float z = Radius * sinf(Phi) * sinf(Theta);
	float y = Radius * cosf(Phi);

	// Build the view matrix.
	glm::vec3 eyePos(x, y, z);
	glm::vec3 target(0, 0, 0);
	ViewMat = glm::lookAt(eyePos, target, glm::vec3(0, 1, 0));
	if (gAxisRenderer)
	{
		float x = 5.0f * sinf(Phi) * cosf(Theta);
		float z = 5.0f * sinf(Phi) * sinf(Theta);
		float y = 5.0f * cosf(Phi);
		gAxisRenderer->SetCameraPos(glm::vec3(x, y, z));
	}
	
	PassConstants pc;
	pc.ViewProj = glm::transpose(ProjMat * ViewMat);
	pc.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	glm::vec3 lightDir = -SphericalToCartesian(1.0f, 1.25 * glm::pi<float>(), glm::four_over_pi<float>());
	pc.Lights[0].Direction = lightDir;
	pc.Lights[0].Strength = { 1.0f, 1.0f, 0.9f };
	curFR.CBPerFrame->CopyData(0, &pc);

	UpdateObjectCBs(dt, curFR);
	UpdateMaterialCBs(dt, curFR);
	UpdateWaves(dt);
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
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.2f * static_cast<float>(x - LastMousePos.x);
		float dy = 0.2f * static_cast<float>(y - LastMousePos.y);

		// Update the camera radius based on input.
		Radius += dx - dy;

		// Restrict the radius.
		Radius = glm::clamp(Radius, 5.0f, 150.0f);
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

void OnMouseUp(WPARAM btnState, int x, int y)
{
	LastMousePos.x = x;
	LastMousePos.y = y;


	ReleaseCapture();
}

void BuildShadersAndInputLayout()
{
	Shaders["standardVS"] = gRenderer->CompileShader(L"Shaders/SimpleShader.hlsl", nullptr, 0, fb::EShaderType::VertexShader, "VS");
	Shaders["opaquePS"] = gRenderer->CompileShader(L"Shaders/SimpleShader.hlsl", nullptr, 0, fb::EShaderType::PixelShader, "PS");

	Shaders["axisVS"] = gRenderer->CompileShader(L"Shaders/Axis.hlsl", nullptr, 0, fb::EShaderType::VertexShader, "VS");
	Shaders["axisPS"] = gRenderer->CompileShader(L"Shaders/Axis.hlsl", nullptr, 0, fb::EShaderType::PixelShader, "PS");

	Shaders["lightingVS"] = gRenderer->CompileShader(L"Shaders/DefaultLight.hlsl", nullptr, 0, fb::EShaderType::VertexShader, "VS");
	Shaders["lightingPS"] = gRenderer->CompileShader(L"Shaders/DefaultLight.hlsl", nullptr, 0, fb::EShaderType::PixelShader, "PS");

	Shaders["crateVS"] = gRenderer->CompileShader(L"Shaders/Default.hlsl", nullptr, 0, fb::EShaderType::VertexShader, "VS");
	Shaders["cratePS"] = gRenderer->CompileShader(L"Shaders/Default.hlsl", nullptr, 0, fb::EShaderType::PixelShader, "PS");

	InputLayout = {
		{ "POSITION", 0, fb::EDataFormat::R32G32B32_FLOAT, 0, 0, fb::EInputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, fb::EDataFormat::R32G32B32_FLOAT, 0, 12, fb::EInputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, fb::EDataFormat::R32G32_FLOAT, 0, 24, fb::EInputClassification::PerVertexData, 0 },
	};
}

void DestroyShaders()
{
	Shaders.clear();
}

float GetHillsHeight(float x, float z)
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

glm::vec3 GetHillsNormal(float x, float z)
{
	// n = (-df/dx, 1, -df/dz)
	glm::vec3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	return glm::normalize(n);
}


void BuildLandGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  In addition, color the vertices based on their height so we have
	// sandy looking beaches, grassy low hills, and snow mountain peaks.
	//

	std::vector<Vertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		auto& p = grid.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Pos.y = GetHillsHeight(p.x, p.z);
		vertices[i].Normal = GetHillsNormal(p.x, p.z);
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = grid.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "landGeo";

	geo->VertexBuffer = gRenderer->CreateVertexBuffer(vertices.data(), vbByteSize, sizeof(Vertex), false);
	geo->IndexBuffer = gRenderer->CreateIndexBuffer(indices.data(), ibByteSize, fb::EDataFormat::R16_UINT, false);

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	Geometries["landGeo"] = std::move(geo);
}

void DestroyGeometries()
{
	Geometries.clear();
}

void BuildWavesGeometryBuffers()
{
	std::vector<std::uint16_t> indices(3 * gWaves->TriangleCount()); // 3 indices per face
	assert(gWaves->VertexCount() < 0x0000ffff);

	// Iterate over each quad.
	int m = gWaves->RowCount();
	int n = gWaves->ColumnCount();
	int k = 0;
	for (int i = 0; i < m - 1; ++i)
	{
		for (int j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	UINT vbByteSize = gWaves->VertexCount() * sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "waterGeo";

	// Set dynamically.
	geo->VertexBuffer = gRenderer->CreateVertexBuffer(nullptr, 0, 0, false);
	geo->IndexBuffer = gRenderer->CreateIndexBuffer(indices.data(), ibByteSize, fb::EDataFormat::R16_UINT, false);

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	Geometries["waterGeo"] = std::move(geo);
}

void BuildPSO()
{
	fb::FPSODesc psoDesc;
	psoDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
	psoDesc.pRootSignature = LightingRootSig;
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(Shaders["crateVS"]->GetByteCode()),
		Shaders["crateVS"]->Size()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(Shaders["cratePS"]->GetByteCode()),
		Shaders["cratePS"]->Size()
	};
	//psoDesc.RasterizerState
	//psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = fb::ECullMode::NONE;
	psoDesc.PrimitiveTopologyType = fb::EPrimitiveTopologyType::TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = gRenderer->GetBackBufferFormat();
	psoDesc.SampleDesc.Count = gRenderer->GetSampleCount();
	psoDesc.SampleDesc.Quality = gRenderer->GetMsaaQuality();
	psoDesc.DSVFormat = gRenderer->GetDepthStencilFormat();
	SimpleBoxPSO = gRenderer->CreateGraphicsPipelineState(psoDesc);
	psoDesc.RasterizerState.FillMode = fb::EFillMode::WIREFRAME;
	SimpleBoxPSOWireframe = gRenderer->CreateGraphicsPipelineState(psoDesc);
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
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
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


	// Texture Box
	{
		GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);

		SubmeshGeometry boxSubmesh;
		boxSubmesh.IndexCount = (UINT)box.Indices32.size();
		boxSubmesh.StartIndexLocation = 0;
		boxSubmesh.BaseVertexLocation = 0;


		std::vector<Vertex> vertices(box.Vertices.size());

		for (size_t i = 0; i < box.Vertices.size(); ++i)
		{
			vertices[i].Pos = box.Vertices[i].Position;
			vertices[i].Normal = box.Vertices[i].Normal;
			vertices[i].TexC = box.Vertices[i].TexC;
		}

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		UINT ibByteSize;

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = "crateGeo";

		geo->VertexBuffer = gRenderer->CreateVertexBuffer(vertices.data(), vbByteSize, sizeof(Vertex), false);

		if (box.Vertices.size() <= 0xffff)
		{
			std::vector<std::uint16_t> indices = box.GetIndices16();
			ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);
			geo->IndexBuffer = gRenderer->CreateIndexBuffer(indices.data(), ibByteSize, fb::EDataFormat::R16_UINT, false);
		}
		else
		{
			ibByteSize = (UINT)box.Indices32.size() * sizeof(std::uint32_t);
			geo->IndexBuffer = gRenderer->CreateIndexBuffer(box.Indices32.data(), ibByteSize, fb::EDataFormat::R32_UINT, false);
		}		

		geo->DrawArgs["crate"] = boxSubmesh;
		Geometries[geo->Name] = std::move(geo);
	}
}

void BuildMaterials()
{
	auto grass = std::make_unique<Material>();
	grass->Name = "grass";
	grass->MatCBIndex = 0;
	grass->DiffuseAlbedo = glm::vec4(1.f, 1.0f, 1.0f, 1.0f);
	grass->FresnelR0 = glm::vec3(0.01f, 0.01f, 0.01f);
	grass->Roughness = 0.125f;

	// This is not a good water material definition, but we do not have all the rendering
	// tools we need (transparency, environment reflection), so we fake it for now.
	auto water = std::make_unique<Material>();
	water->Name = "water";
	water->MatCBIndex = 1;
	water->DiffuseAlbedo = glm::vec4(0.0f, 0.2f, 0.6f, 1.0f);
	water->FresnelR0 = glm::vec3(0.1f, 0.1f, 0.1f);
	water->Roughness = 0.0f;

	Materials["grass"] = std::move(grass);
	Materials["water"] = std::move(water);
}

void BuildRenderItems()
{
	auto crateItem = std::make_unique<RenderItem>();
	crateItem->ObjectCBIndex = 0;
	auto crateGeo = Geometries["crateGeo"].get();
	crateItem->Mat = Materials["grass"].get();
	crateItem->VB = crateGeo->VertexBuffer;
	crateItem->IB = crateGeo->IndexBuffer;
	crateItem->PrimitiveTopology = fb::EPrimitiveTopology::TRIANGLELIST;
	crateItem->IndexCount = crateGeo->DrawArgs["crate"].IndexCount;
	crateItem->StartIndexLocation = crateGeo->DrawArgs["crate"].StartIndexLocation;
	crateItem->BaseVertexLocation = crateGeo->DrawArgs["crate"].BaseVertexLocation;
	RenderItemLayers[(int)ERenderLayer::Opaque].push_back(crateItem.get());
	AllRitems.push_back(std::move(crateItem));

	/*auto wavesRitem = std::make_unique<RenderItem>();
	wavesRitem->ObjectCBIndex = 0;
	auto waterGeo = Geometries["waterGeo"].get();
	wavesRitem->Mat = Materials["water"].get();
	wavesRitem->VB = waterGeo->VertexBuffer;
	wavesRitem->IB = waterGeo->IndexBuffer;
	wavesRitem->PrimitiveTopology =  fb::EPrimitiveTopology::TRIANGLELIST;
	wavesRitem->IndexCount = waterGeo->DrawArgs["grid"].IndexCount;
	wavesRitem->StartIndexLocation = waterGeo->DrawArgs["grid"].StartIndexLocation;
	wavesRitem->BaseVertexLocation = waterGeo->DrawArgs["grid"].BaseVertexLocation;

	gWavesRitem = wavesRitem.get();

	RenderItemLayers[(int)ERenderLayer::Opaque].push_back(wavesRitem.get());

	auto gridRitem = std::make_unique<RenderItem>();
	gridRitem->ObjectCBIndex = 1;
	auto landGeo = Geometries["landGeo"].get();
	gridRitem->Mat = Materials["grass"].get();
	gridRitem->VB = landGeo->VertexBuffer;
	gridRitem->IB = landGeo->IndexBuffer;
	gridRitem->PrimitiveTopology = fb::EPrimitiveTopology::TRIANGLELIST;
	gridRitem->IndexCount = landGeo->DrawArgs["grid"].IndexCount;
	gridRitem->StartIndexLocation = landGeo->DrawArgs["grid"].StartIndexLocation;
	gridRitem->BaseVertexLocation = landGeo->DrawArgs["grid"].BaseVertexLocation;

	RenderItemLayers[(int)ERenderLayer::Opaque].push_back(gridRitem.get());

	AllRitems.push_back(std::move(wavesRitem));
	AllRitems.push_back(std::move(gridRitem));*/
}

void DestroyRenderItems()
{
	AllRitems.clear();
}

void BuildWaves()
{
	gWaves = new Waves(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);
	auto numFrameResources = gRenderer->GetNumSwapchainBuffers();
	for (int frameIndex = 0; frameIndex < numFrameResources; ++frameIndex)
	{
		auto& curFR = GetFrameResource(frameIndex);
		curFR.WavesVB = gRenderer->CreateUploadBuffer(sizeof(Vertex), gWaves->VertexCount(), false);
	}
}

void LoadTextures()
{
	Textures["woodCrateTex"] = gRenderer->LoadTexture(L"Textures/WoodCrate01.dds");
}

void BuildShaderResourceView()
{
	if (!DescriptorHeap)
		DescriptorHeap = gRenderer->CreateDescriptorHeap(fb::EDescriptorHeapType::CBV_SRV_UAV, Num_CBV_SRV_UAV);

	assert(Textures["woodCrateTex"]);
	auto result = DescriptorHeap->CreateDescriptor(0, Textures["woodCrateTex"]);
	assert(result);
}

void DrawRenderItems()
{
	fb::EPrimitiveTopology LastPrimitiveTopology = fb::EPrimitiveTopology::UNDEFINED;
	auto& curFR = GetFrameResource(CurrentFrameResourceIndex);
	auto& opaqueRenderItems = RenderItemLayers[(int)ERenderLayer::Opaque];
	// For each render item...
	for (size_t i = 0; i < opaqueRenderItems.size(); ++i)
	{
		auto ri = opaqueRenderItems[i];

		ri->VB->Bind(0);
		ri->IB->Bind();
		if (LastPrimitiveTopology != ri->PrimitiveTopology) {
			gRenderer->SetPrimitiveTopology(ri->PrimitiveTopology);
			LastPrimitiveTopology = ri->PrimitiveTopology;
		}

		gRenderer->SetGraphicsRootConstantBufferView(0, curFR.CBPerObject, ri->ObjectCBIndex);
		assert(ri->Mat);
		gRenderer->SetGraphicsRootConstantBufferView(1, curFR.CBPerMaterial, ri->Mat->MatCBIndex);
		// index 2 is being used for Per-frame CB.

		gRenderer->SetGraphicsRootDescriptorTable(3, DescriptorHeap, 0);

		//UINT cbvIndex = (UINT)opaqueRenderItems.size() + ri->ConstantBufferIndex;
		//gRenderer->SetGraphicsRootDescriptorTable(0, fb::EDescriptorHeapType::Default, cbvIndex);
		gRenderer->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0 );
	}
}

void Draw(float dt)
{
	auto& curFR = GetFrameResource(CurrentFrameResourceIndex);
	auto cmdListAlloc = curFR.CommandAllocator;
	cmdListAlloc->Reset();

	if (IsWireframe) {
		gRenderer->ResetCommandList(cmdListAlloc, SimpleBoxPSOWireframe);
	}
	else {
		gRenderer->ResetCommandList(cmdListAlloc, SimpleBoxPSO);
	}
	gRenderer->SetViewportAndScissor(0, 0, Width, Height);
	gRenderer->ResourceBarrier_Backbuffer_PresentToRenderTarget();
	gRenderer->ClearRenderTargetDepthStencil();
	gRenderer->SetDefaultRenderTargets();

	//gRenderer->BindDescriptorHeap(fb::EDescriptorHeapType::Default);
	if (DescriptorHeap)
		DescriptorHeap->Bind();
	LightingRootSig->Bind();

	gRenderer->SetGraphicsRootConstantBufferView(2, curFR.CBPerFrame, 0);

	DrawRenderItems();

	gAxisRenderer->Render();

	gRenderer->ResourceBarrier_Backbuffer_RenderTargetToPresent();

	gRenderer->CloseCommandList();
	gRenderer->ExecuteCommandList();
	gRenderer->PresentAndSwapBuffer();

	 curFR.Fence = gRenderer->SignalFence();
}

void OnKeyboardInput()
{
	if (GetAsyncKeyState('1') & 0x8000)
		IsWireframe = true;
	else
		IsWireframe = false;
}