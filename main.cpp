/*
 * [이론 설명: Win32 API 기본 헤더]
 * windows.h: 창 생성, 메시지 처리 등 Windows OS 기능을 쓰기 위한 필수 헤더
 */

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <thread>
#include "CPPGameTimer.h"
#include <vector> 
#include <string>

 // 라이브러리 링크
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

class GameObject;

// [1단계: 컴포넌트 기저 클래스]
// 모든 기능(이동, 렌더링 등)은 이 클래스를 상속받아야 함.
class Component {
public:
    class GameObject* pOwner; // 이 기능이 누구의 것인지 저장
    bool isStarted = false;           // Start()가 실행되었는지 체크

    virtual void Start() = 0;              // 초기화
    virtual void OnInput() {}              // 입력 (선택사항)
    virtual void OnUpdate(float dt) = 0;    // 로직 (필수)
    virtual void OnRender() {}             // 그리기 (선택사항)

    virtual ~Component() {}
};

// [2단계: 게임 오브젝트 클래스]
// 컴포넌트들을 담는 바구니 역할을 함.
class GameObject {
public:
    std::string name;
    float x = 0.0f; //GameObject 클래스는 위치(Position) 정보를 가지며, 부착된 Component들의 Update와 Render를 일괄 호출해야 합니다.
    float y = 0.0f;//GameObject 클래스는 위치(Position) 정보를 가지며, 부착된 Component들의 Update와 Render를 일괄 호출해야 합니다.
    std::vector<Component*> components;

    ID3D11Buffer* pVBuffer = nullptr;

    GameObject(std::string n) {
        name = n;
    }

    // 객체가 죽을 때 담고 있던 컴포넌트들도 메모리에서 해제함
    ~GameObject() {
        for (int i = 0; i < (int)components.size(); i++) {
            delete components[i];
        }
        if (pVBuffer) pVBuffer->Release();
    }

    // 새로운 기능을 추가하는 함수
    void AddComponent(Component* pComp) {
        pComp->pOwner = this;
        pComp->isStarted = false;
        components.push_back(pComp);
    }
};



// 전역 변수 (간결한 예제를 위해 사용)
ID3D11Device* g_pd3dDevice = nullptr;                  //모든 리소스의 생성을 담당하는 핵심 객체임. 하드웨어(GPU)와의 통로 역할을 하며, 실질적으로 메모리를 할당하는 기능을 가짐.
ID3D11DeviceContext* g_pImmediateContext = nullptr;     //생성된 리소스를 사용하여 GPU에 그리기 명령(Rendering Commands)을 내리는 객체임. 파이프라인의 상태를 설정하고 실제로 "그려라(Draw)"라고 지시함.
IDXGISwapChain* g_pSwapChain = nullptr;                 //그려진 그림을 모니터 화면으로 전달하고 관리하는 시스템임. 더블 버퍼링(Double Buffering) 기술의 실체라고 보면 됨.
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;  //GPU가 결과물을 써 내려갈 대상(Target)을 정의하는 '뷰(View)' 객체임. DX11에서는 리소스(Texture2D)를 직접 파이프라인에 꽂지 않음. 대신 그 리소스를 어떤 용도(렌더 타겟용, 셰이더 읽기용 등)로 쓸 것인지 정의하는 'View'를 통해 접근함.

ID3D11InputLayout* g_pInputLayout = nullptr;
ID3D11VertexShader* g_vShader = nullptr;
ID3D11PixelShader* g_pShader = nullptr;

bool g_isRunning = true;

// 정점 구조체
struct Vertex {
    float x, y, z;
    float r, g, b, a;
};

Vertex g_tri1[3] = {
    {  0.0f,       0.5f,  0.5f,   0.0f, 0.0f, 1.0f, 1.0f },
    {  0.433013f, -0.25f, 0.5f,   0.0f, 0.0f, 1.0f, 1.0f },
    { -0.433013f, -0.25f, 0.5f,   0.0f, 0.0f, 1.0f, 1.0f }
};

Vertex g_tri2[3] = {
    {  0.0f,      -0.5f,  0.5f,   1.0f, 0.5f, 0.0f, 1.0f },
    { -0.433013f,  0.25f, 0.5f,   1.0f, 0.5f, 0.0f, 1.0f },
    {  0.433013f,  0.25f, 0.5f,   1.0f, 0.5f, 0.0f, 1.0f }
};

// HLSL 셰이더 (이전 예제와 동일)
const char* shaderSource = R"(
struct VS_INPUT { float3 pos : POSITION; float4 col : COLOR; };
struct PS_INPUT { float4 pos : SV_POSITION; float4 col : COLOR; };
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output;
    output.pos = float4(input.pos, 1.0f);
    output.col = input.col;
    return output;
}
float4 PS(PS_INPUT input) : SV_Target { return input.col; }
)";

/*
 * [이론 설명: 윈도우 프로시저 (WndProc)]
 * 운영체제가 윈도우에 보낸 메시지(마우스 클릭, 창 닫기 등)를 처리하는 함수임.
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            // ESC 키: 윈도우 종료 메시지 큐에 삽입 (이후 WM_QUIT 발생 -> 루프 탈출)
            PostQuitMessage(0);
        }
        else if (wParam == 'F') {
            // F 키: 전체화면 / 창모드 토글
            if (g_pSwapChain) {
                BOOL isFullScreen = FALSE;
                g_pSwapChain->GetFullscreenState(&isFullScreen, nullptr);
                g_pSwapChain->SetFullscreenState(!isFullScreen, nullptr); // 상태 반전
            }
        }
        break;
    case WM_DESTROY: // 창이 닫힐 때 발생하는 메시지
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

class MeshRenderer : public Component {
public:
    void Start() override {}
    void OnUpdate(float dt) override {}

    void OnRender() override {
        if (!pOwner->pVBuffer) return;
        g_pImmediateContext->IASetInputLayout(g_pInputLayout);
        UINT stride = sizeof(Vertex), offset = 0;
        g_pImmediateContext->IASetVertexBuffers(0, 1, &pOwner->pVBuffer, &stride, &offset);
        g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        g_pImmediateContext->VSSetShader(g_vShader, nullptr, 0);
        g_pImmediateContext->PSSetShader(g_pShader, nullptr, 0);

        g_pImmediateContext->Draw(3, 0);
    }
};

class Transform : public Component {
public:
    float velocity = 1.0f;

    bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false;

    int playerType = 0;

    Transform(int type) { playerType = type; }

    void Start() override {}

    void OnInput() override {
        if (playerType == 0) {
            moveLeft = (GetAsyncKeyState(VK_LEFT) & 0x8000);
            moveRight = (GetAsyncKeyState(VK_RIGHT) & 0x8000);
            moveUp = (GetAsyncKeyState(VK_UP) & 0x8000);
            moveDown = (GetAsyncKeyState(VK_DOWN) & 0x8000);
        }
        else {
            moveLeft = (GetAsyncKeyState('A') & 0x8000);
            moveRight = (GetAsyncKeyState('D') & 0x8000);
            moveUp = (GetAsyncKeyState('W') & 0x8000);
            moveDown = (GetAsyncKeyState('S') & 0x8000);
        }
    }

    void OnUpdate(float dt) override {
        // 1. 자신의 좌표 이동 로직
        if (moveLeft)  pOwner->x -= velocity * dt;
        if (moveRight) pOwner->x += velocity * dt;
        if (moveUp)    pOwner->y += velocity * dt;
        if (moveDown)  pOwner->y -= velocity * dt;

        // 2. 이동한 좌표를 바탕으로 GPU 버퍼 갱신 (전역 Device 변수 사용)
        if (pOwner->pVBuffer) pOwner->pVBuffer->Release();

        Vertex currentVertices[3];

        Vertex* baseVertices = (playerType == 0) ? g_tri1 : g_tri2;

        for (int i = 0; i < 3; ++i) {
            currentVertices[i] = baseVertices[i];
            currentVertices[i].x *= 0.75;
            currentVertices[i].x += pOwner->x; // 자기 자신의 x값을 더함
            currentVertices[i].y += pOwner->y; // 자기 자신의 y값을 더함
        }

        D3D11_BUFFER_DESC bd = { sizeof(currentVertices), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
        D3D11_SUBRESOURCE_DATA initData = { currentVertices, 0, 0 };
        g_pd3dDevice->CreateBuffer(&bd, &initData, &pOwner->pVBuffer);
    }
};

class infoDisplay : public Component {

private:
    float timeElapsed = 0.0f;
    int frameCount = 0;
    int lastFPS = 0;

public:
    void Start() override {
        timeElapsed = 0.0f;
        frameCount = 0;
        lastFPS = 0;
    }

    void OnUpdate(float dt) override {
        // --- [1. FPS 측정 및 연속 출력 (시간 손실 방지)] ---
        timeElapsed += dt;
        frameCount++;
    // 1초가 지났을 때만 FPS 값 갱신
        if (timeElapsed >= 1.0f) {
            lastFPS = frameCount; // 1초 동안 누적된 프레임 수를 저장
            timeElapsed -= 1.0f;  // 0으로 강제 초기화하지 않고 1.0f만 빼서 자투리 시간 보존 (시간 손실 방지)
            frameCount = 0;
        }

        // 매 프레임마다 연속해서 출력 (FPS는 저장된 lastFPS 사용, 델타타임은 현재 dt 사용)
        // \r 을 사용하여 줄바꿈 없이 제자리에서 덮어쓰기 (콘솔 스크롤 부하 방지)
        printf("\r현재 FPS: %4d | Delta Time: %8.6f sec", lastFPS, dt);
    }
    void OnRender() override {}
    
};

//입력단계
void ProcessInput(std::vector<GameObject*>& gameWorld) {
    MSG msg = { 0 };
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            g_isRunning = false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    for (int i = 0; i < (int)gameWorld.size(); i++) {
        for (int j = 0; j < (int)gameWorld[i]->components.size(); j++) {
            gameWorld[i]->components[j]->OnInput();
        }
    }
}


void Update(float dt, std::vector<GameObject*>& gameWorld) {

    for (int i = 0; i < (int)gameWorld.size(); i++) {
        for (int j = 0; j < (int)gameWorld[i]->components.size(); j++) {
            // Start()가 호출된 적 없다면 여기서 호출 (유니티 방식)
            if (gameWorld[i]->components[j]->isStarted == false) {
                gameWorld[i]->components[j]->Start();
                gameWorld[i]->components[j]->isStarted = true;
            }
            gameWorld[i]->components[j]->OnUpdate(dt);
        }
    }
}

void Render(float dt, std::vector<GameObject*>& gameWorld) {

    // --- 렌더링 시작 ---
    float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
    D3D11_VIEWPORT vp = { 0, 0, 800, 600, 0.0f, 1.0f };
    g_pImmediateContext->RSSetViewports(1, &vp);

    for (int i = 0; i < (int)gameWorld.size(); i++) {
        for (int j = 0; j < (int)gameWorld[i]->components.size(); j++) {
            gameWorld[i]->components[j]->OnRender();
        }
    }

    g_pSwapChain->Present(0, 0);
}

// --- Win32 메인 진입점 ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1. 윈도우 클래스 등록 (창의 속성 정의)
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc; // 메시지 처리 함수 연결
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = L"DX11Win32Class";
    RegisterClassEx(&wcex);

    RECT rc = { 0, 0, 800, 600 };

    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    // 2. 실제 윈도우 생성
    HWND hWnd = CreateWindow(L"DX11Win32Class", L" 지옥의 ESC 키: 프로그램 즉시 종료 및 관련 메모리 해제. - 지옥의 F 키: 창 모드(Windowed)와 전체 화면(Full Screen) 모드를 전환(Toggle)",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) return -1;
    ShowWindow(hWnd, nCmdShow);

    // 3. DirectX 11 초기화 (Win32 HWND 연결)
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 800;
    sd.BufferDesc.Height = 600;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd; // 생성한 Win32 창 핸들 연결
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;

    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, nullptr, &g_pImmediateContext);

    // Render Target View 생성
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();

    // 4. 셰이더 및 버퍼 설정 (이전과 동일한 로직)
    ID3DBlob* vsBlob, * psBlob;
    D3DCompile(shaderSource, strlen(shaderSource), nullptr, nullptr, nullptr, "VS", "vs_4_0", 0, 0, &vsBlob, nullptr);
    D3DCompile(shaderSource, strlen(shaderSource), nullptr, nullptr, nullptr, "PS", "ps_4_0", 0, 0, &psBlob, nullptr);


    g_pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_vShader);
    g_pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_pShader);

    // Input Layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    g_pd3dDevice->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_pInputLayout);


    std::vector<GameObject*> gameWorld;

    // Player 1 생성 (방향키 조작)
    GameObject* player1 = new GameObject("Player 1");
    player1->AddComponent(new Transform(0)); // 0번 타입
    player1->AddComponent(new MeshRenderer());
    gameWorld.push_back(player1);

    // Player 2 생성 (WASD 조작)
    GameObject* player2 = new GameObject("Player 2");
    player2->AddComponent(new Transform(1)); // 1번 타입
    player2->AddComponent(new MeshRenderer());
    gameWorld.push_back(player2);

    GameObject* sysInfo = new GameObject("SystemManager");
    sysInfo->AddComponent(new infoDisplay());
    gameWorld.push_back(sysInfo);

    CPPGameTimer timer;

    //게임 루프
    while (g_isRunning) {

        float dt = timer.Update();

        ProcessInput(gameWorld);
        Update(dt, gameWorld);
        Render(dt, gameWorld);
    }

    for (int i = 0; i < (int)gameWorld.size(); i++) {
        delete gameWorld[i];
    }
    gameWorld.clear();

    // 자원 해제
    if (g_pInputLayout) g_pInputLayout->Release();
    if (g_vShader) g_vShader->Release();
    if (g_pShader) g_pShader->Release();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();

    return 0;
}