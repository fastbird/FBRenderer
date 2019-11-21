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
#include "../../FBMathExt/MathExt.h"

#define MAX_LOADSTRING 100

struct Vertex
{
	glm::vec3 Pos;
	glm::vec3 Normal;
	glm::vec2 TexC;

	Vertex()
	{

	}
	Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v)
		: Pos(x, y, z)
		, Normal(nx, ny, nz)
		, TexC(u, v)
	{
	}
};

struct TreeSpriteVertex
{
	glm::vec3 Pos;
	glm::vec2 Size;
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
glm::vec3 gEyePos;
glm::mat4 WorldMat(1.0f), ViewMat(1.0f), ProjMat(1.0f);
fb::IUploadBuffer* PerPassCBs = nullptr;
POINT LastMousePos;
std::unordered_map<std::string, fb::PSOID> PSOs;
UINT CurrentFrameResourceIndex = 0;
std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> Geometries;
std::unordered_map<std::string, std::unique_ptr<Material>> Materials;
std::unordered_map<std::string, fb::ITextureIPtr> Textures;
std::vector<std::unique_ptr<RenderItem>> AllRitems;
fb::IDescriptorHeapIPtr DescriptorHeap;
PassConstants MainPassConstants;
PassConstants ReflectedPassConstants;
RenderItem* gSkullRitem = nullptr;
RenderItem* gReflectedSkullRitem = nullptr;
RenderItem* gShadowedSkullRitem = nullptr;
glm::vec3 gSkullTranslation(0.f, 1.f, -5.f);
enum class ERenderLayer : int
{
	Opaque = 0,
	AlphaTested,
	AlphaTestedTreeSprites,
	Reflected,
	Shadow,
	TransparentMirror,
	Transparent,
	Count
};

std::vector<fb::FInputElementDesc> StandardInputLayout;
std::vector<fb::FInputElementDesc> BillboardInputLayout;
std::vector<RenderItem*> RenderItemLayers[(int)ERenderLayer::Count];

UINT PassCbvOffset = 0;
std::unordered_map<std::string, fb::IShaderIPtr> Shaders;
fb::IRootSignatureIPtr SimpleBoxRootSig;
fb::IRootSignatureIPtr CBVRootSig;
fb::IRootSignatureIPtr gRootSignature;
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
void UpdateMainPassCB(float dt, float totalTime, FFrameResource& curFR);
void UpdateReflectedPassCB(float dt, FFrameResource& curFR);

void OnMouseMove(WPARAM btnState, int x, int y);
void OnMouseDown(WPARAM btnState, int x, int y);
void OnMouseUp(WPARAM btnState, int x, int y);

void BuildShadersAndInputLayout();
void DestroyShaders();

void BuildLandGeometry();
void DestroyGeometries();

void BuildWavesGeometryBuffers();

void BuildPSO_Skull();
void BuildPSO_Wave();

void BuildShapeGeometry();
void BuildTreeSpritesGeometry();
void BuildRenderItems_Wave();
void BuildRenderItems_MirroredSkull();
void BuildWaves();
void LoadTextures_Skull();
void LoadTextures_Wave();
void BuildShaderResourceView_Skull();
void BuildShaderResourceView_Wave();
void BuildMaterials_Skull();
void BuildMaterials_Wave();
void Draw_Wave(float dt);
void Draw_Skull(float dt);
void OnKeyboardInput(float dt);
void BuildRoomGeometry();
void BuildSkullGeometry();

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
	static auto time = std::chrono::high_resolution_clock::now();
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
			auto now = std::chrono::high_resolution_clock::now();
			auto dt = std::chrono::duration<float>(now - time).count();
			time = now;
			Update(dt);
			if (gRenderer)
			{
				Draw_Wave(dt);
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
	gRootSignature.reset();
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
   //BuildMaterials_Skull();
   BuildMaterials_Wave();

   gRenderer->ResetCommandList(nullptr, 0);
   std::wcout << L"Reset Command List" << std::endl;

   BuildShapeGeometry();
   BuildTreeSpritesGeometry();

   LoadTextures_Wave();

   BuildShaderResourceView_Wave();

   BuildShadersAndInputLayout();

   BuildWaves();
   BuildLandGeometry();
   BuildWavesGeometryBuffers();

   //BuildRoomGeometry();
   //BuildSkullGeometry();

   //BuildRenderItems_Skull();
   BuildRenderItems_Wave();

   BuildConstantBuffers((int)AllRitems.size(), (int)Materials.size() , 2);
   
   SimpleBoxRootSig = gRenderer->CreateRootSignature("DTable,1,0,CBV");
   CBVRootSig = gRenderer->CreateRootSignature("RootCBV,0;RootCBV,1;");
   // per object, material, per frame, texture
   gRootSignature = gRenderer->CreateRootSignature("RootCBV,0;RootCBV,1;RootCBV,2;DTable,3,0,SRV");

   std::wcout << L"Root sig." << std::endl;

   BuildPSO_Wave();

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
			objConstants.TexTransform = glm::transpose(e->TexTransform);

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
			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			glm::mat4 matTransform = glm::transpose(mat->MatTransform);
			matConstants.MatTransform = matTransform;

			currMaterialCB->CopyData(mat->MatCBIndex, &matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void UpdateMainPassCB(float dt, float totalTime, FFrameResource& curFR)
{
	glm::mat4 viewProj = ProjMat * ViewMat;
	glm::mat4 invView = glm::inverse(ViewMat);
	glm::mat4 invProj = glm::inverse(ProjMat);
	glm::mat4 invViewProj = glm::inverse(viewProj);

	MainPassConstants.View = glm::transpose(ViewMat);
	MainPassConstants.InvView = glm::transpose(invView);
	MainPassConstants.Proj = glm::transpose(ProjMat);
	MainPassConstants.InvProj = glm::transpose(invProj);
	MainPassConstants.ViewProj = glm::transpose(viewProj);
	MainPassConstants.InvViewProj = glm::transpose(invViewProj);
	MainPassConstants.EyePosW = gEyePos;
	MainPassConstants.RenderTargetSize = glm::vec2((float)Width, (float)Height);
	MainPassConstants.InvRenderTargetSize = glm::vec2(1.0f / Width, 1.0f / Height);
	MainPassConstants.NearZ = 1.0f;
	MainPassConstants.FarZ = 1000.0f;
	MainPassConstants.TotalTime = totalTime;
	MainPassConstants.DeltaTime = dt;
	MainPassConstants.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	MainPassConstants.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	MainPassConstants.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
	MainPassConstants.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	MainPassConstants.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
	MainPassConstants.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	MainPassConstants.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	curFR.CBPerFrame->CopyData(0, &MainPassConstants);
}

void UpdateReflectedPassCB(float dt, FFrameResource& curFR)
{
	ReflectedPassConstants = MainPassConstants;

	glm::vec4 mirrorPlane(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
	glm::mat4 R = fb::MatrixReflect(mirrorPlane);

	// Reflect the lighting.
	for (int i = 0; i < 3; ++i)
	{
		ReflectedPassConstants.Lights[i].Direction = R * glm::vec4(MainPassConstants.Lights[i].Direction, 0);
	}

	curFR.CBPerFrame->CopyData(1, &ReflectedPassConstants);
}

void Print(const glm::vec3& v)
{
	std::wcout << v.x << L"," << v.y << L"," << v.z << std::endl;
}

void UpdateWaves(float dt, FFrameResource& curFR)
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
	// Update the wave vertex buffer with the new solution.
	auto currWavesVB = curFR.WavesVB.get();
	for (int i = 0; i < gWaves->VertexCount(); ++i)
	{
		Vertex v;
		v.Pos = gWaves->Position(i);
		v.Normal = gWaves->Normal(i);
		v.TexC.x = 0.5f + v.Pos.x / gWaves->Width();
		v.TexC.y = 0.5f - v.Pos.z / gWaves->Depth();
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

void AnimateMaterials(float dt)
{
	auto it = Materials.find("water");
	if (it == Materials.end())
		return;
	auto waterMat = Materials["water"].get();
	if (!waterMat)
		return;

	float& tu = waterMat->MatTransform[3][0];
	float& tv = waterMat->MatTransform[3][1];

	tu += 0.1f * dt;
	tv += 0.02f * dt;

	if (tu >= 1.0f)
		tu -= 1.0f;

	if (tv >= 1.0f)
		tv -= 1.0f;

	// Material has changed, so need to update cbuffer.
	waterMat->NumFramesDirty = NUM_SWAPCHAIN_BUFFERS;
}

void Update(float dt)
{
	static float TotalTime = 0;
	TotalTime += dt;

	OnKeyboardInput(dt);

	auto& curFR = GetFrameResource_WaitAvailable(CurrentFrameResourceIndex);

	// Convert Spherical to Cartesian coordinates.
	float x = Radius * sinf(Phi) * cosf(Theta);
	float z = Radius * sinf(Phi) * sinf(Theta);
	float y = Radius * cosf(Phi);

	// Build the view matrix.
	gEyePos = glm::vec3(x, y, z);
	glm::vec3 target(0, 0, 0);
	ViewMat = glm::lookAt(gEyePos, target, glm::vec3(0, 1, 0));
	if (gAxisRenderer)
	{
		float x = 5.0f * sinf(Phi) * cosf(Theta);
		float z = 5.0f * sinf(Phi) * sinf(Theta);
		float y = 5.0f * cosf(Phi);
		gAxisRenderer->SetCameraPos(glm::vec3(x, y, z));
	}
	
	/*MainPassConstants.ViewProj = glm::transpose(ProjMat * ViewMat);
	MainPassConstants.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	glm::vec3 lightDir = -SphericalToCartesian(1.0f, 1.25f * glm::pi<float>(), glm::four_over_pi<float>());
	MainPassConstants.Lights[0].Direction = lightDir;
	MainPassConstants.Lights[0].Strength = { 1.0f, 1.0f, 0.9f };
	MainPassConstants.EyePosW = gEyePos;
	curFR.CBPerFrame->CopyData(0, &MainPassConstants);*/

	AnimateMaterials(dt);
	UpdateObjectCBs(dt, curFR);
	UpdateMaterialCBs(dt, curFR);
	UpdateMainPassCB(dt, TotalTime, curFR);
	//UpdateReflectedPassCB(dt, curFR);
	UpdateWaves(dt, curFR);
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
	Shaders["axisVS"] = gRenderer->CompileShader(L"Shaders/Axis.hlsl", nullptr, fb::EShaderType::VertexShader, "VS");
	Shaders["axisPS"] = gRenderer->CompileShader(L"Shaders/Axis.hlsl", nullptr, fb::EShaderType::PixelShader, "PS");

	const fb::FShaderMacro defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const fb::FShaderMacro alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	Shaders["standardVS"] = gRenderer->CompileShader(L"Shaders/Default.hlsl", nullptr, fb::EShaderType::VertexShader, "VS");
	Shaders["opaquePS"] = gRenderer->CompileShader(L"Shaders/Default.hlsl", defines, fb::EShaderType::PixelShader, "PS");
	Shaders["alphaTestedPS"] = gRenderer->CompileShader(L"Shaders/Default.hlsl", alphaTestDefines, fb::EShaderType::PixelShader, "PS");


	Shaders["treeSpriteVS"] = gRenderer->CompileShader(L"Shaders/TreeSprite.hlsl", nullptr, fb::EShaderType::VertexShader, "VS");
	Shaders["treeSpriteGS"] = gRenderer->CompileShader(L"Shaders/TreeSprite.hlsl", nullptr, fb::EShaderType::GeometryShader, "GS");
	Shaders["treeSpritePS"] = gRenderer->CompileShader(L"Shaders/TreeSprite.hlsl", alphaTestDefines,  fb::EShaderType::PixelShader, "PS");

	StandardInputLayout = {
		{ "POSITION", 0, fb::EDataFormat::R32G32B32_FLOAT, 0, 0, fb::EInputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, fb::EDataFormat::R32G32B32_FLOAT, 0, 12, fb::EInputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, fb::EDataFormat::R32G32_FLOAT, 0, 24, fb::EInputClassification::PerVertexData, 0 },
	};

	BillboardInputLayout = {
		{ "POSITION", 0, fb::EDataFormat::R32G32B32_FLOAT, 0, 0, fb::EInputClassification::PerVertexData, 0 },
		{ "SIZE", 0, fb::EDataFormat::R32G32_FLOAT, 0, 12, fb::EInputClassification::PerVertexData, 0 },
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
		vertices[i].TexC = grid.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = grid.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "landGeo";

	geo->VertexBuffer = gRenderer->CreateVertexBuffer(vertices.data(), vbByteSize, sizeof(Vertex), false);
	geo->IndexBuffer = gRenderer->CreateIndexBuffer(indices.data(), ibByteSize, fb::EIndexBufferFormat::R16, false);

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
	geo->IndexBuffer = gRenderer->CreateIndexBuffer(indices.data(), ibByteSize, fb::EIndexBufferFormat::R16, false);
	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	geo->DrawArgs["grid"] = submesh;
	Geometries["waterGeo"] = std::move(geo);
}

void BuildPSO_Wave()
{
	fb::FPSODesc psoDesc;
	psoDesc.InputLayout = { StandardInputLayout.data(), (UINT)StandardInputLayout.size() };
	psoDesc.pRootSignature = gRootSignature;
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(Shaders["standardVS"]->GetByteCode()),
		Shaders["standardVS"]->Size()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(Shaders["opaquePS"]->GetByteCode()),
		Shaders["opaquePS"]->Size()
	};
	psoDesc.PrimitiveTopologyType = fb::EPrimitiveTopologyType::TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RasterizerState.CullMode = fb::ECullMode::FRONT;
	psoDesc.RTVFormats[0] = gRenderer->GetBackBufferFormat();
	psoDesc.SampleDesc.Count = gRenderer->GetSampleCount();
	psoDesc.SampleDesc.Quality = gRenderer->GetMsaaQuality();
	psoDesc.DSVFormat = gRenderer->GetDepthStencilFormat();
	PSOs["Default"] = gRenderer->CreateGraphicsPipelineState(psoDesc);

	fb::FPSODesc alphaTestedPSODesc = psoDesc;
	alphaTestedPSODesc.PS =
	{
		reinterpret_cast<BYTE*>(Shaders["alphaTestedPS"]->GetByteCode()),
		Shaders["alphaTestedPS"]->Size()
	};
	alphaTestedPSODesc.RasterizerState.CullMode = fb::ECullMode::NONE;
	PSOs["AlphaTested"] = gRenderer->CreateGraphicsPipelineState(alphaTestedPSODesc);

	fb::FPSODesc alphaBlendedPSODesc = psoDesc;
	fb::FRenderTargetBlendDesc blendDesc;
	blendDesc.BlendEnable = true;
	blendDesc.LogicOpEnable = false;
	blendDesc.SrcBlend = fb::EBlend::SRC_ALPHA;
	blendDesc.DestBlend = fb::EBlend::INV_SRC_ALPHA;
	blendDesc.BlendOp = fb::EBlendOp::ADD;
	blendDesc.SrcBlendAlpha = fb::EBlend::ONE;
	blendDesc.DestBlendAlpha = fb::EBlend::ZERO;
	blendDesc.BlendOpAlpha = fb::EBlendOp::ADD;
	blendDesc.LogicOp = fb::ELogicOp::NOOP;
	blendDesc.RenderTargetWriteMask = fb::EColorWriteEnable::ALL;
	alphaBlendedPSODesc.BlendState.RenderTarget[0] = blendDesc;
	PSOs["AlphaBlended"] = gRenderer->CreateGraphicsPipelineState(alphaBlendedPSODesc);

	//
	// PSO for tree sprites
	//
	fb::FPSODesc treeSpritePsoDesc = psoDesc;
	treeSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(Shaders["treeSpriteVS"]->GetByteCode()),
		Shaders["treeSpriteVS"]->Size()
	};
	treeSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(Shaders["treeSpriteGS"]->GetByteCode()),
		Shaders["treeSpriteGS"]->Size()
	};
	treeSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(Shaders["treeSpritePS"]->GetByteCode()),
		Shaders["treeSpritePS"]->Size()
	};
	treeSpritePsoDesc.PrimitiveTopologyType = fb::EPrimitiveTopologyType::POINT;
	treeSpritePsoDesc.InputLayout = { BillboardInputLayout.data(), (UINT)BillboardInputLayout.size() };
	treeSpritePsoDesc.RasterizerState.CullMode = fb::ECullMode::NONE;
	PSOs["treeSprites"] = gRenderer->CreateGraphicsPipelineState(treeSpritePsoDesc);

	psoDesc.RasterizerState.FillMode = fb::EFillMode::WIREFRAME;
	PSOs["Wireframe"] = gRenderer->CreateGraphicsPipelineState(psoDesc);
}

void BuildPSO_Skull()
{
	//
	// Opaque
	//
	fb::FPSODesc psoDesc;
	psoDesc.InputLayout = { StandardInputLayout.data(), (UINT)StandardInputLayout.size() };
	psoDesc.pRootSignature = gRootSignature;
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(Shaders["standardVS"]->GetByteCode()),
		Shaders["standardVS"]->Size()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(Shaders["opaquePS"]->GetByteCode()),
		Shaders["opaquePS"]->Size()
	};
	psoDesc.PrimitiveTopologyType = fb::EPrimitiveTopologyType::TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RasterizerState.FrontCounterClockwise = true;
	psoDesc.RTVFormats[0] = gRenderer->GetBackBufferFormat();
	psoDesc.SampleDesc.Count = gRenderer->GetSampleCount();
	psoDesc.SampleDesc.Quality = gRenderer->GetMsaaQuality();
	psoDesc.DSVFormat = gRenderer->GetDepthStencilFormat();
	psoDesc.DepthStencilState.StencilEnable = true;
	psoDesc.DepthStencilState.FrontFace.StencilPassOp = fb::EStencilOp::REPLACE;
	psoDesc.DepthStencilState.FrontFace.StencilFunc = fb::EComparisonFunc::ALWAYS;
	psoDesc.DepthStencilState.BackFace.StencilPassOp = fb::EStencilOp::REPLACE;
	psoDesc.DepthStencilState.BackFace.StencilFunc = fb::EComparisonFunc::ALWAYS;

	PSOs["Default"] = gRenderer->CreateGraphicsPipelineState(psoDesc);

	//
	// alphaTested
	//
	fb::FPSODesc alphaTestedPSODesc = psoDesc;
	alphaTestedPSODesc.PS = 
	{
		reinterpret_cast<BYTE*>(Shaders["alphaTestedPS"]->GetByteCode()),
		Shaders["alphaTestedPS"]->Size()
	};
	alphaTestedPSODesc.RasterizerState.CullMode = fb::ECullMode::NONE;
	PSOs["AlphaTested"] = gRenderer->CreateGraphicsPipelineState(alphaTestedPSODesc);

	fb::FPSODesc alphaBlendedPSODesc = psoDesc;
	fb::FRenderTargetBlendDesc blendDesc;
	blendDesc.BlendEnable = true;
	blendDesc.LogicOpEnable = false;
	
	blendDesc.SrcBlend = fb::EBlend::SRC_ALPHA;
	blendDesc.DestBlend = fb::EBlend::INV_SRC_ALPHA;
	blendDesc.BlendOp = fb::EBlendOp::ADD;

	blendDesc.SrcBlendAlpha = fb::EBlend::ONE;
	blendDesc.DestBlendAlpha = fb::EBlend::ZERO;
	blendDesc.BlendOpAlpha = fb::EBlendOp::ADD;
	
	blendDesc.LogicOp = fb::ELogicOp::NOOP;
	blendDesc.RenderTargetWriteMask = fb::EColorWriteEnable::ALL;
	alphaBlendedPSODesc.BlendState.RenderTarget[0] = blendDesc;
	alphaBlendedPSODesc.DepthStencilState.DepthEnable = false;
	alphaBlendedPSODesc.DepthStencilState.StencilEnable = true;
	alphaBlendedPSODesc.DepthStencilState.FrontFace.StencilFunc = fb::EComparisonFunc::EQUAL;
	alphaBlendedPSODesc.RasterizerState.FrontCounterClockwise = true;
	PSOs["TransparentMirror"] = gRenderer->CreateGraphicsPipelineState(alphaBlendedPSODesc);


	//
	// PSO for stencil reflections.
	//
	fb::FPSODesc reflectionPSODesc = psoDesc;
	reflectionPSODesc.DepthStencilState.DepthEnable = false;
	reflectionPSODesc.DepthStencilState.StencilEnable = true;
	reflectionPSODesc.DepthStencilState.FrontFace.StencilFunc = fb::EComparisonFunc::EQUAL;
	reflectionPSODesc.DepthStencilState.FrontFace.StencilPassOp = fb::EStencilOp::INCR;
	reflectionPSODesc.DepthStencilState.BackFace.StencilFunc = fb::EComparisonFunc::EQUAL;
	reflectionPSODesc.DepthStencilState.BackFace.StencilPassOp = fb::EStencilOp::INCR;
	reflectionPSODesc.RasterizerState.FrontCounterClockwise = false;
	PSOs["drawStencilReflections"] = gRenderer->CreateGraphicsPipelineState(reflectionPSODesc);

	//
	// PSO for shadow
	//
	fb::FPSODesc shadowPSODesc = alphaBlendedPSODesc;
	fb::FDepthStencilDesc depthStencilState;
	depthStencilState.StencilEnable = true;
	depthStencilState.FrontFace.StencilFunc = fb::EComparisonFunc::EQUAL;
	depthStencilState.FrontFace.StencilPassOp = fb::EStencilOp::INCR_SAT;
	shadowPSODesc.DepthStencilState = depthStencilState;
	shadowPSODesc.RasterizerState.DepthBias = -100;
	PSOs["shadow"] = gRenderer->CreateGraphicsPipelineState(shadowPSODesc);

	// Wireframe
	psoDesc.RasterizerState.FillMode = fb::EFillMode::WIREFRAME;
	PSOs["Wireframe"] = gRenderer->CreateGraphicsPipelineState(psoDesc);
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

	geo->IndexBuffer = gRenderer->CreateIndexBuffer(indices.data(), ibByteSize, fb::EIndexBufferFormat::R16, false);

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
			geo->IndexBuffer = gRenderer->CreateIndexBuffer(indices.data(), ibByteSize, fb::EIndexBufferFormat::R16, false);
		}
		else
		{
			ibByteSize = (UINT)box.Indices32.size() * sizeof(std::uint32_t);
			geo->IndexBuffer = gRenderer->CreateIndexBuffer(box.Indices32.data(), ibByteSize, fb::EIndexBufferFormat::R32, false);
		}		

		geo->DrawArgs["crate"] = boxSubmesh;
		Geometries[geo->Name] = std::move(geo);
	}
}

void BuildTreeSpritesGeometry()
{

	static const int treeCount = 16;
	std::array<TreeSpriteVertex, 16> vertices;
	for (UINT i = 0; i < treeCount; ++i)
	{
		float x = fb::RandF(-45.0f, 45.0f);
		float z = fb::RandF(-45.0f, 45.0f);
		float y = GetHillsHeight(x, z);

		// Move tree slightly above land height.
		y += 8.0f;

		vertices[i].Pos = glm::vec3(x, y, z);
		vertices[i].Size = glm::vec2(20.0f, 20.0f);
	}

	/*std::array<std::uint16_t, 16> indices =
	{
		0, 1, 2, 3, 4, 5, 6, 7,
		8, 9, 10, 11, 12, 13, 14, 15
	};*/

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(TreeSpriteVertex);
	//const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "treeSpritesGeo";
	geo->VertexBuffer = gRenderer->CreateVertexBuffer(vertices.data(), vbByteSize, sizeof(TreeSpriteVertex), false);

	Geometries["treeSpritesGeo"] = std::move(geo);
}

void BuildSkullGeometry()
{
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<Vertex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

		// Model does not have texture coordinates, so just zero them out.
		vertices[i].TexC = { 0.0f, 0.0f };
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<std::int32_t> indices(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "skullGeo";
	geo->VertexBuffer = gRenderer->CreateVertexBuffer(vertices.data(), vbByteSize, sizeof(Vertex), false);
	geo->IndexBuffer = gRenderer->CreateIndexBuffer(indices.data(), ibByteSize, fb::EIndexBufferFormat::R32, false);

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["skull"] = submesh;

	Geometries[geo->Name] = std::move(geo);
}

void BuildRoomGeometry()
{
	// Create and specify geometry.  For this sample we draw a floor
// and a wall with a mirror on it.  We put the floor, wall, and
// mirror geometry in one vertex buffer.
//
//   |--------------|
//   |              |
//   |----|----|----|
//   |Wall|Mirr|Wall|
//   |    | or |    |
//   /--------------/
//  /   Floor      /
// /--------------/

	std::array<Vertex, 20> vertices =
	{
		// Floor: Observe we tile texture coordinates.
		Vertex(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f), // 0 
		Vertex(-3.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f),
		Vertex(7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f),
		Vertex(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f),

		// Wall: Observe we tile texture coordinates, and that we
		// leave a gap in the middle for the mirror.
		Vertex(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 4
		Vertex(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f),
		Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f),

		Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 8 
		Vertex(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f),
		Vertex(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f),

		Vertex(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 12
		Vertex(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f),
		Vertex(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f),

		// Mirror
		Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 16
		Vertex(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f),
		Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f)
	};

	std::array<std::int16_t, 30> indices =
	{
		// Floor
		0, 1, 2,
		0, 2, 3,

		// Walls
		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		// Mirror
		16, 17, 18,
		16, 18, 19
	};

	SubmeshGeometry floorSubmesh;
	floorSubmesh.IndexCount = 6;
	floorSubmesh.StartIndexLocation = 0;
	floorSubmesh.BaseVertexLocation = 0;

	SubmeshGeometry wallSubmesh;
	wallSubmesh.IndexCount = 18;
	wallSubmesh.StartIndexLocation = 6;
	wallSubmesh.BaseVertexLocation = 0;

	SubmeshGeometry mirrorSubmesh;
	mirrorSubmesh.IndexCount = 6;
	mirrorSubmesh.StartIndexLocation = 24;
	mirrorSubmesh.BaseVertexLocation = 0;

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "roomGeo";

	geo->VertexBuffer = gRenderer->CreateVertexBuffer(vertices.data(), vbByteSize, sizeof(Vertex), false);
	geo->IndexBuffer = gRenderer->CreateIndexBuffer(indices.data(), ibByteSize, fb::EIndexBufferFormat::R16, false);

	geo->DrawArgs["floor"] = floorSubmesh;
	geo->DrawArgs["wall"] = wallSubmesh;
	geo->DrawArgs["mirror"] = mirrorSubmesh;

	Geometries[geo->Name] = std::move(geo);
}

void BuildMaterials_Skull()
{
	auto bricks = std::make_unique<Material>();
	bricks->Name = "bricks";
	bricks->MatCBIndex = 0;
	bricks->DiffuseSrvHeapIndex = 0;
	bricks->DiffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks->FresnelR0 = glm::vec3(0.05f, 0.05f, 0.05f);
	bricks->Roughness = 0.25f;

	auto checkertile = std::make_unique<Material>();
	checkertile->Name = "checkertile";
	checkertile->MatCBIndex = 1;
	checkertile->DiffuseSrvHeapIndex = 1;
	checkertile->DiffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	checkertile->FresnelR0 = glm::vec3(0.07f, 0.07f, 0.07f);
	checkertile->Roughness = 0.3f;

	auto icemirror = std::make_unique<Material>();
	icemirror->Name = "icemirror";
	icemirror->MatCBIndex = 2;
	icemirror->DiffuseSrvHeapIndex = 2;
	icemirror->DiffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 0.3f);
	icemirror->FresnelR0 = glm::vec3(0.1f, 0.1f, 0.1f);
	icemirror->Roughness = 0.5f;

	auto skullMat = std::make_unique<Material>();
	skullMat->Name = "skullMat";
	skullMat->MatCBIndex = 3;
	skullMat->DiffuseSrvHeapIndex = 3;
	skullMat->DiffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	skullMat->FresnelR0 = glm::vec3(0.05f, 0.05f, 0.05f);
	skullMat->Roughness = 0.3f;

	auto shadowMat = std::make_unique<Material>();
	shadowMat->Name = "shadowMat";
	shadowMat->MatCBIndex = 4;
	shadowMat->DiffuseSrvHeapIndex = 3;
	shadowMat->DiffuseAlbedo = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
	shadowMat->FresnelR0 = glm::vec3(0.001f, 0.001f, 0.001f);
	shadowMat->Roughness = 0.0f;

	Materials["bricks"] = std::move(bricks);
	Materials["checkertile"] = std::move(checkertile);
	Materials["icemirror"] = std::move(icemirror);
	Materials["skullMat"] = std::move(skullMat);
	Materials["shadowMat"] = std::move(shadowMat);
}

void BuildMaterials_Wave()
{
	auto grass = std::make_unique<Material>();
	grass->Name = "grass";
	grass->MatCBIndex = 0;
	grass->DiffuseAlbedo = glm::vec4(1.f, 1.0f, 1.0f, 1.0f);
	grass->DiffuseSrvHeapIndex = 0;
	grass->FresnelR0 = glm::vec3(0.01f, 0.01f, 0.01f);
	grass->Roughness = 0.125f;
	Materials[grass->Name] = std::move(grass);

	// This is not a good water material definition, but we do not have all the rendering
	// tools we need (transparency, environment reflection), so we fake it for now.
	auto water = std::make_unique<Material>();
	water->Name = "water";
	water->MatCBIndex = 1;
	//water->DiffuseAlbedo = glm::vec4(0.0f, 0.2f, 0.6f, 1.0f);
	water->DiffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
	water->DiffuseSrvHeapIndex = 1;
	water->FresnelR0 = glm::vec3(0.02f, 0.02f, 0.02f);
	water->Roughness = 0.0f;
	Materials[water->Name] = std::move(water);

	auto crateMat = std::make_unique<Material>();
	crateMat->Name = "crate";
	crateMat->MatCBIndex = 2;
	crateMat->DiffuseSrvHeapIndex = 2;
	crateMat->DiffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	crateMat->FresnelR0 = glm::vec3(0.1f, 0.1f, 0.1f);
	crateMat->Roughness = 0.99999f;
	Materials[crateMat->Name] = std::move(crateMat);

	auto treeSprites = std::make_unique<Material>();
	treeSprites->Name = "treeSprites";
	treeSprites->MatCBIndex = 3;
	treeSprites->DiffuseSrvHeapIndex = 3;
	treeSprites->DiffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	treeSprites->FresnelR0 = glm::vec3(0.01f, 0.01f, 0.01f);
	treeSprites->Roughness = 0.125f;
	Materials[treeSprites->Name] = std::move(treeSprites);

}

void BuildRenderItems_MirroredSkull()
{
	auto floorRitem = std::make_unique<RenderItem>();
	floorRitem->ObjectCBIndex = 0;
	assert(Materials["checkertile"].get());
	floorRitem->Mat = Materials["checkertile"].get();
	auto& roomGeom = Geometries["roomGeo"];
	floorRitem->VB = roomGeom->VertexBuffer;
	floorRitem->IB = roomGeom->IndexBuffer;
	auto& floorSubMesh = roomGeom->DrawArgs["floor"];
	floorRitem->IndexCount = floorSubMesh.IndexCount;
	floorRitem->StartIndexLocation = floorSubMesh.StartIndexLocation;
	floorRitem->BaseVertexLocation = floorSubMesh.BaseVertexLocation;
	RenderItemLayers[(int)ERenderLayer::Opaque].push_back(floorRitem.get());
	AllRitems.push_back(std::move(floorRitem));

	auto wallsRitem = std::make_unique<RenderItem>();
	wallsRitem->ObjectCBIndex = 1;
	wallsRitem->Mat = Materials["bricks"].get();
	wallsRitem->VB = roomGeom->VertexBuffer;
	wallsRitem->IB = roomGeom->IndexBuffer;
	auto& wallSubMesh = roomGeom->DrawArgs["wall"];
	wallsRitem->IndexCount = wallSubMesh.IndexCount;
	wallsRitem->StartIndexLocation = wallSubMesh.StartIndexLocation;
	wallsRitem->BaseVertexLocation = wallSubMesh.BaseVertexLocation;
	RenderItemLayers[(int)ERenderLayer::Opaque].push_back(wallsRitem.get());
	AllRitems.push_back(std::move(wallsRitem));

	auto skullRitem = std::make_unique<RenderItem>();
	skullRitem->ObjectCBIndex = 2;
	skullRitem->Mat = Materials["skullMat"].get();
	skullRitem->World = glm::scale(glm::vec3(0.45f)) * glm::translate(glm::vec3(0.f, 1.f, -5.f));
	auto& skullGeom = Geometries["skullGeo"];	
	skullRitem->VB = skullGeom->VertexBuffer;
	skullRitem->IB = skullGeom->IndexBuffer;
	auto& skullSubMesh = skullGeom->DrawArgs["skull"];
	skullRitem->IndexCount = skullSubMesh.IndexCount;
	skullRitem->StartIndexLocation = skullSubMesh.StartIndexLocation;
	skullRitem->BaseVertexLocation = skullSubMesh.BaseVertexLocation;
	gSkullRitem = skullRitem.get();
	RenderItemLayers[(int)ERenderLayer::Opaque].push_back(skullRitem.get());
	

	// Reflected skull will have different world matrix, so it needs to be its own render item.
	auto reflectedSkullRitem = std::make_unique<RenderItem>();
	*reflectedSkullRitem = *skullRitem;
	reflectedSkullRitem->ObjectCBIndex = 3;
	reflectedSkullRitem->StencilRef = 1;
	reflectedSkullRitem->World = fb::MatrixReflect(glm::vec4(0, 0, 1, 0)) * skullRitem->World;
	gReflectedSkullRitem = reflectedSkullRitem.get();
	RenderItemLayers[(int)ERenderLayer::Reflected].push_back(reflectedSkullRitem.get());
	AllRitems.push_back(std::move(reflectedSkullRitem));

	// Shadowed skull will have different world matrix, so it needs to be its own render item.
	auto shadowedSkullRitem = std::make_unique<RenderItem>();
	*shadowedSkullRitem = *skullRitem;
	shadowedSkullRitem->ObjectCBIndex = 4;
	shadowedSkullRitem->StencilRef = 0;
	shadowedSkullRitem->Mat = Materials["shadowMat"].get();
	gShadowedSkullRitem = shadowedSkullRitem.get();
	RenderItemLayers[(int)ERenderLayer::Shadow].push_back(shadowedSkullRitem.get());
	AllRitems.push_back(std::move(shadowedSkullRitem));
	
	AllRitems.push_back(std::move(skullRitem));

	auto mirrorRitem = std::make_unique<RenderItem>();
	mirrorRitem->ObjectCBIndex = 5;
	mirrorRitem->Mat = Materials["icemirror"].get();
	mirrorRitem->VB = roomGeom->VertexBuffer;
	mirrorRitem->IB = roomGeom->IndexBuffer;
	auto& mirrorSubMesh = roomGeom->DrawArgs["mirror"];
	mirrorRitem->IndexCount = mirrorSubMesh.IndexCount;
	mirrorRitem->StartIndexLocation = mirrorSubMesh.StartIndexLocation;
	mirrorRitem->BaseVertexLocation = mirrorSubMesh.BaseVertexLocation;
	mirrorRitem->StencilRef = 1;
	RenderItemLayers[(int)ERenderLayer::Opaque].push_back(mirrorRitem.get());

	auto mirrorRitemBlending = std::make_unique<RenderItem>();
	*mirrorRitemBlending = *mirrorRitem;
	mirrorRitemBlending->StencilRef = 2;
	RenderItemLayers[(int)ERenderLayer::TransparentMirror].push_back(mirrorRitemBlending.get());

	AllRitems.push_back(std::move(mirrorRitem));
	AllRitems.push_back(std::move(mirrorRitemBlending));
	
}

void BuildRenderItems_Wave()
{
	auto wavesRitem = std::make_unique<RenderItem>();
	wavesRitem->ObjectCBIndex = 0;
	auto waterGeo = Geometries["waterGeo"].get();
	wavesRitem->Mat = Materials["water"].get();
	wavesRitem->TexTransform = glm::scale(glm::vec3{ 5.0f, 5.0f, 1.0f });
	wavesRitem->VB = waterGeo->VertexBuffer;
	wavesRitem->IB = waterGeo->IndexBuffer;
	wavesRitem->PrimitiveTopology =  fb::EPrimitiveTopology::TRIANGLELIST;
	wavesRitem->IndexCount = waterGeo->DrawArgs["grid"].IndexCount;
	wavesRitem->StartIndexLocation = waterGeo->DrawArgs["grid"].StartIndexLocation;
	wavesRitem->BaseVertexLocation = waterGeo->DrawArgs["grid"].BaseVertexLocation;
	gWavesRitem = wavesRitem.get();
	RenderItemLayers[(int)ERenderLayer::Transparent].push_back(wavesRitem.get());
	AllRitems.push_back(std::move(wavesRitem));

	auto gridRitem = std::make_unique<RenderItem>();
	gridRitem->ObjectCBIndex = 1;
	auto landGeo = Geometries["landGeo"].get();
	gridRitem->Mat = Materials["grass"].get();
	gridRitem->TexTransform = glm::scale(glm::vec3{ 5.0f, 5.0f, 1.0f });
	gridRitem->VB = landGeo->VertexBuffer;
	gridRitem->IB = landGeo->IndexBuffer;
	gridRitem->PrimitiveTopology = fb::EPrimitiveTopology::TRIANGLELIST;
	gridRitem->IndexCount = landGeo->DrawArgs["grid"].IndexCount;
	gridRitem->StartIndexLocation = landGeo->DrawArgs["grid"].StartIndexLocation;
	gridRitem->BaseVertexLocation = landGeo->DrawArgs["grid"].BaseVertexLocation;
	RenderItemLayers[(int)ERenderLayer::Opaque].push_back(gridRitem.get());
	AllRitems.push_back(std::move(gridRitem));

	auto crateItem = std::make_unique<RenderItem>();
	crateItem->ObjectCBIndex = 2;
	auto crateGeo = Geometries["crateGeo"].get();
	crateItem->Mat = Materials["crate"].get();
	crateItem->World = glm::scale(glm::vec3{ 5.0f, 5.0f, 5.0f });
	crateItem->VB = crateGeo->VertexBuffer;
	crateItem->IB = crateGeo->IndexBuffer;
	crateItem->PrimitiveTopology = fb::EPrimitiveTopology::TRIANGLELIST;
	crateItem->IndexCount = crateGeo->DrawArgs["crate"].IndexCount;
	crateItem->StartIndexLocation = crateGeo->DrawArgs["crate"].StartIndexLocation;
	crateItem->BaseVertexLocation = crateGeo->DrawArgs["crate"].BaseVertexLocation;
	RenderItemLayers[(int)ERenderLayer::AlphaTested].push_back(crateItem.get());
	AllRitems.push_back(std::move(crateItem));

	auto treeSpritesRitem = std::make_unique<RenderItem>();
	treeSpritesRitem->ObjectCBIndex = 3;
	treeSpritesRitem->Mat = Materials["treeSprites"].get();
	auto& treeSpritesGeo = Geometries["treeSpritesGeo"];
	treeSpritesRitem->VB = treeSpritesGeo->VertexBuffer;
	treeSpritesRitem->PrimitiveTopology = fb::EPrimitiveTopology::POINTLIST;
	treeSpritesRitem->IndexCount = treeSpritesGeo->VertexBuffer->GetNumVertices();
	RenderItemLayers[(int)ERenderLayer::AlphaTestedTreeSprites].push_back(treeSpritesRitem.get());
	AllRitems.push_back(std::move(treeSpritesRitem));
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

void LoadTextures_Wave()
{
	Textures["grassTex"] = gRenderer->LoadTexture(L"Textures/grass.dds");
	Textures["waterTex"] = gRenderer->LoadTexture(L"Textures/water1.dds");
	Textures["woodCrateTex"] = gRenderer->LoadTexture(L"Textures/WoodCrate01.dds");
	Textures["wireFenceTex"] = gRenderer->LoadTexture(L"Textures/WireFence.dds");
	Textures["treeArrayTex"] = gRenderer->LoadTexture(L"Textures/treeArray2.dds");
}

void LoadTextures_Skull()
{
	Textures["grassTex"] = gRenderer->LoadTexture(L"Textures/grass.dds");
	Textures["waterTex"] = gRenderer->LoadTexture(L"Textures/water1.dds");
	Textures["woodCrateTex"] = gRenderer->LoadTexture(L"Textures/WoodCrate01.dds");
	Textures["wireFenceTex"] = gRenderer->LoadTexture(L"Textures/WireFence.dds");
	Textures["bricksTex"] = gRenderer->LoadTexture(L"Textures/bricks3.dds");
	Textures["checkboardTex"] = gRenderer->LoadTexture(L"Textures/checkboard.dds");
	Textures["iceTex"] = gRenderer->LoadTexture(L"Textures/ice.dds");
	Textures["white1x1Tex"] = gRenderer->LoadTexture(L"Textures/white1x1.dds");
}

void BuildShaderResourceView_Wave()
{
	const UINT Num_CBV_SRV_UAV = 4;
	if (!DescriptorHeap)
		DescriptorHeap = gRenderer->CreateDescriptorHeap(fb::EDescriptorHeapType::CBV_SRV_UAV, Num_CBV_SRV_UAV);

	assert(Textures["woodCrateTex"]);
	auto result = DescriptorHeap->CreateDescriptor(0, Textures["grassTex"]);
	assert(result);
	result = DescriptorHeap->CreateDescriptor(1, Textures["waterTex"]);
	assert(result);
	result = DescriptorHeap->CreateDescriptor(2, Textures["wireFenceTex"]);
	assert(result);
	result = DescriptorHeap->CreateDescriptor(3, Textures["treeArrayTex"]);
	assert(result);

}
void BuildShaderResourceView_Skull()
{
	const UINT Num_CBV_SRV_UAV = 4;
	if (!DescriptorHeap)
		DescriptorHeap = gRenderer->CreateDescriptorHeap(fb::EDescriptorHeapType::CBV_SRV_UAV, Num_CBV_SRV_UAV);

	auto result = DescriptorHeap->CreateDescriptor(0, Textures["bricksTex"]);
	assert(result);
	result = DescriptorHeap->CreateDescriptor(1, Textures["checkboardTex"]);
	assert(result);
	result = DescriptorHeap->CreateDescriptor(2, Textures["iceTex"]);
	assert(result);
	result = DescriptorHeap->CreateDescriptor(3, Textures["white1x1Tex"]);
	assert(result);
}

void DrawRenderItems(const std::vector<RenderItem*>& ritems)
{
	fb::EPrimitiveTopology LastPrimitiveTopology = fb::EPrimitiveTopology::UNDEFINED;
	auto& curFR = GetFrameResource(CurrentFrameResourceIndex);
	// For each render item...
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		ri->VB->Bind(0);
		if (ri->IB)
			ri->IB->Bind();
		if (LastPrimitiveTopology != ri->PrimitiveTopology) {
			gRenderer->SetPrimitiveTopology(ri->PrimitiveTopology);
			LastPrimitiveTopology = ri->PrimitiveTopology;
		}

		gRenderer->SetGraphicsRootConstantBufferView(0, curFR.CBPerObject, ri->ObjectCBIndex);
		assert(ri->Mat);
		gRenderer->SetGraphicsRootConstantBufferView(1, curFR.CBPerMaterial, ri->Mat->MatCBIndex);
		// index 2 is being used for Per-frame CB.
		if (ri->Mat->DiffuseSrvHeapIndex != -1) {
			gRenderer->SetGraphicsRootDescriptorTable(3, DescriptorHeap, ri->Mat->DiffuseSrvHeapIndex);
		}
		gRenderer->SetStencilRef(ri->StencilRef);
		if (ri->IB)
			gRenderer->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0 );
		else
			gRenderer->DrawInstanced(ri->IndexCount, 1, ri->BaseVertexLocation, 0);
	}
}

void Draw_Wave(float dt)
{
	auto& curFR = GetFrameResource(CurrentFrameResourceIndex);
	auto cmdListAlloc = curFR.CommandAllocator;
	cmdListAlloc->Reset();
	auto defaultPSO = PSOs.find("Default");
	if (defaultPSO == PSOs.end())
		return;

	if (IsWireframe) {
		auto wireFramePSO = PSOs.find("Wireframe");
		gRenderer->ResetCommandList(cmdListAlloc, wireFramePSO->second);
	}
	else {
		gRenderer->ResetCommandList(cmdListAlloc, defaultPSO->second);
	}

	gRenderer->SetViewportAndScissor(0, 0, Width, Height);
	gRenderer->ResourceBarrier_Backbuffer_PresentToRenderTarget();
	gRenderer->ClearRenderTargetDepthStencil((float*)&MainPassConstants.FogColor);
	gRenderer->SetDefaultRenderTargets();

	if (DescriptorHeap)
		DescriptorHeap->Bind();
	gRootSignature->Bind();

	gRenderer->SetGraphicsRootConstantBufferView(2, curFR.CBPerFrame, 0);

	DrawRenderItems(RenderItemLayers[(int)ERenderLayer::Opaque]);

	auto alphaTestedPSO = PSOs.find("AlphaTested");
	if (alphaTestedPSO != PSOs.end()) {
		gRenderer->SetPipelineState(alphaTestedPSO->second);
		DrawRenderItems(RenderItemLayers[(int)ERenderLayer::AlphaTested]);
	}
	auto treeSpritesPSO = PSOs.find("treeSprites");
	if (treeSpritesPSO != PSOs.end()) {
		gRenderer->SetPipelineState(treeSpritesPSO->second);
		DrawRenderItems(RenderItemLayers[(int)ERenderLayer::AlphaTestedTreeSprites]);
	}

	auto alphaBlendedPSO = PSOs.find("AlphaBlended");
	if (alphaBlendedPSO != PSOs.end()) {
		gRenderer->SetPipelineState(alphaBlendedPSO->second);
		DrawRenderItems(RenderItemLayers[(int)ERenderLayer::Transparent]);
	}

	gAxisRenderer->Render();

	gRenderer->ResourceBarrier_Backbuffer_RenderTargetToPresent();

	gRenderer->CloseCommandList();
	gRenderer->ExecuteCommandList();
	gRenderer->PresentAndSwapBuffer();

	curFR.Fence = gRenderer->SignalFence();
}

void Draw_Skull(float dt)
{
	auto& curFR = GetFrameResource(CurrentFrameResourceIndex);
	auto cmdListAlloc = curFR.CommandAllocator;
	cmdListAlloc->Reset();
	auto defaultPSO = PSOs.find("Default");
	if (defaultPSO == PSOs.end())
		return;

	if (IsWireframe) {
		auto wireFramePSO = PSOs.find("Wireframe");
		gRenderer->ResetCommandList(cmdListAlloc, wireFramePSO->second);
	}
	else {
		gRenderer->ResetCommandList(cmdListAlloc, defaultPSO->second);
	}

	gRenderer->SetViewportAndScissor(0, 0, Width, Height);
	gRenderer->ResourceBarrier_Backbuffer_PresentToRenderTarget();
	gRenderer->ClearRenderTargetDepthStencil((float*)&MainPassConstants.FogColor);
	gRenderer->SetDefaultRenderTargets();

	if (DescriptorHeap)
		DescriptorHeap->Bind();
	gRootSignature->Bind();

	// Opaque
	gRenderer->SetGraphicsRootConstantBufferView(2, curFR.CBPerFrame, 0);
	DrawRenderItems(RenderItemLayers[(int)ERenderLayer::Opaque]);

	// Draw the reflection into the mirror only (only for pixels where the stencil buffer is 1).
	// Note that we must supply a different per-pass constant buffer--one with the lights reflected.
	gRenderer->SetGraphicsRootConstantBufferView(2, curFR.CBPerFrame, 1);
	gRenderer->SetPipelineState(PSOs["drawStencilReflections"]);
	DrawRenderItems(RenderItemLayers[(int)ERenderLayer::Reflected]);

	/*auto alphaTestedPSO = PSOs.find("AlphaTested");
	if (alphaTestedPSO != PSOs.end()) {
		gRenderer->SetPipelineState(alphaTestedPSO->second);
		DrawRenderItems(RenderItemLayers[(int)ERenderLayer::AlphaTested]);
	}*/

	auto transparentMirrorPSO = PSOs.find("TransparentMirror");
	if (transparentMirrorPSO != PSOs.end()) {
		gRenderer->SetPipelineState(transparentMirrorPSO->second);
		DrawRenderItems(RenderItemLayers[(int)ERenderLayer::TransparentMirror]);
	}

	auto shadowPSO = PSOs.find("shadow");
	if (shadowPSO != PSOs.end()) {
		gRenderer->SetPipelineState(shadowPSO->second);
		DrawRenderItems(RenderItemLayers[(int)ERenderLayer::Shadow]);
	}

	gAxisRenderer->Render();

	gRenderer->ResourceBarrier_Backbuffer_RenderTargetToPresent();

	gRenderer->CloseCommandList();
	gRenderer->ExecuteCommandList();
	gRenderer->PresentAndSwapBuffer();

	 curFR.Fence = gRenderer->SignalFence();
}

void OnKeyboardInput(float dt)
{
	if (GetAsyncKeyState('1') & 0x8000)
		IsWireframe = true;
	else
		IsWireframe = false;

	if (gSkullRitem)
	{
		if (GetAsyncKeyState('A') & 0x8000)
			gSkullTranslation.x -= 1.0f * dt;

		if (GetAsyncKeyState('D') & 0x8000)
			gSkullTranslation.x += 1.0f * dt;

		if (GetAsyncKeyState('W') & 0x8000)
			gSkullTranslation.y += 1.0f * dt;

		if (GetAsyncKeyState('S') & 0x8000)
			gSkullTranslation.y -= 1.0f * dt;

		// Don't let user move below ground plane.
		gSkullTranslation.y = std::max(gSkullTranslation.y, 0.0f);

		// Update the new world matrix.
		glm::mat4 skullRotate = glm::rotate(glm::half_pi<float>(), glm::vec3(0, 1, 0));
		glm::mat4 skullScale = glm::scale(glm::vec3(0.45f, 0.45f, 0.45f));
		glm::mat4 skullOffset = glm::translate(gSkullTranslation);
		gSkullRitem->World = skullOffset * skullRotate * skullScale;

		if (gReflectedSkullRitem) {
			// Update reflection world matrix.
			gReflectedSkullRitem->World = fb::MatrixReflect(glm::vec4(0, 0, 1.f, 0)) * gSkullRitem->World;
		}

		// Update shadow world matrix.
		glm::vec4 shadowPlane(0.0f, 1.0f, 0.0f, 0.0f); // xz plane
		glm::vec3 toMainLight = -MainPassConstants.Lights[0].Direction;
		glm::mat4 S = fb::MatrixShadow(shadowPlane, glm::vec4(toMainLight, 0.f));
		//glm::mat4 shadowOffsetY = glm::translate(glm::vec3(0.0f, 0.001f, 0.0f));
		//gShadowedSkullRitem->World = shadowOffsetY * S * gSkullRitem->World;
		gShadowedSkullRitem->World = S * gSkullRitem->World;

		gSkullRitem->NumFramesDirty = NUM_SWAPCHAIN_BUFFERS;
		gReflectedSkullRitem->NumFramesDirty = NUM_SWAPCHAIN_BUFFERS;
		gShadowedSkullRitem->NumFramesDirty = NUM_SWAPCHAIN_BUFFERS;
	}
}