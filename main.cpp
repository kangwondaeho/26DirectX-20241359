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
#include <DirectXMath.h>

 // 라이브러리 링크
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct ConstantBuffer {
    DirectX::XMMATRIX mWorldViewProj;
    DirectX::XMMATRIX mWorld;
    DirectX::XMFLOAT3 LightDir;
    float padding1;
    DirectX::XMFLOAT3 ViewPos;
    float padding2;
};

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
    float x = 0.0f;
    float y = 0.0f;
    std::vector<Component*> components;

    ID3D11Buffer* pVBuffer = nullptr;
    ID3D11Buffer* pIBuffer = nullptr; // [추가] 인덱스 버퍼 포인터

    GameObject(std::string n) {
        name = n;
    }

    ~GameObject() {
        for (int i = 0; i < (int)components.size(); i++) {
            delete components[i];
        }
        if (pVBuffer) pVBuffer->Release();
        if (pIBuffer) pIBuffer->Release(); // [추가] 해제
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
ID3D11Buffer* g_pConstantBuffer = nullptr;

// --- [추가] 깊이 버퍼(Z-Buffer) 전역 변수 ---
ID3D11Texture2D* g_pDepthStencil = nullptr;           // 깊이 값을 저장할 텍스처 (도화지)
ID3D11DepthStencilView* g_pDepthStencilView = nullptr; // 텍스처를 렌더 파이프라인에 연결하는 뷰

bool g_isRunning = true;

DirectX::XMVECTOR g_Eye = DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f);
DirectX::XMVECTOR g_At = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
DirectX::XMVECTOR g_Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

// 정점 구조체
struct Vertex {
    float x, y, z;       // 위치 (12 byte)
    float nx, ny, nz;    // 법선 (12 byte) - 추가됨!
    float r, g, b, a;    // 색상 (16 byte)
};

// 정점 배열 수정 (법선 데이터 추가: 위치 데이터를 정규화한 값)
Vertex g_sphereVertices[] = {
    { -0.2628f,  0.4253f,  0.0000f,   -0.5256f,  0.8506f,  0.0000f,   1.0f, 1.0f, 1.0f, 1.0f },
    {  0.2628f,  0.4253f,  0.0000f,    0.5256f,  0.8506f,  0.0000f,   1.0f, 1.0f, 1.0f, 1.0f },
    { -0.2628f, -0.4253f,  0.0000f,   -0.5256f, -0.8506f,  0.0000f,   1.0f, 1.0f, 1.0f, 1.0f },
    {  0.2628f, -0.4253f,  0.0000f,    0.5256f, -0.8506f,  0.0000f,   1.0f, 1.0f, 1.0f, 1.0f },

    {  0.0000f, -0.2628f,  0.4253f,    0.0000f, -0.5256f,  0.8506f,   1.0f, 1.0f, 1.0f, 1.0f },
    {  0.0000f,  0.2628f,  0.4253f,    0.0000f,  0.5256f,  0.8506f,   1.0f, 1.0f, 1.0f, 1.0f },
    {  0.0000f, -0.2628f, -0.4253f,    0.0000f, -0.5256f, -0.8506f,   1.0f, 1.0f, 1.0f, 1.0f },
    {  0.0000f,  0.2628f, -0.4253f,    0.0000f,  0.5256f, -0.8506f,   1.0f, 1.0f, 1.0f, 1.0f },

    {  0.4253f,  0.0000f, -0.2628f,    0.8506f,  0.0000f, -0.5256f,   1.0f, 1.0f, 1.0f, 1.0f },
    {  0.4253f,  0.0000f,  0.2628f,    0.8506f,  0.0000f,  0.5256f,   1.0f, 1.0f, 1.0f, 1.0f },
    { -0.4253f,  0.0000f, -0.2628f,   -0.8506f,  0.0000f, -0.5256f,   1.0f, 1.0f, 1.0f, 1.0f },
    { -0.4253f,  0.0000f,  0.2628f,   -0.8506f,  0.0000f,  0.5256f,   1.0f, 1.0f, 1.0f, 1.0f }
};

WORD g_sphereIndices[] = {
    0, 11, 5,   0, 5, 1,   0, 1, 7,   0, 7, 10,  0, 10, 11,
    1, 5, 9,    5, 11, 4,  11, 10, 2, 10, 7, 6,  7, 1, 8,
    3, 9, 4,    3, 4, 2,   3, 2, 6,   3, 6, 8,   3, 8, 9,
    4, 9, 5,    2, 4, 11,  6, 2, 10,  8, 6, 7,   9, 8, 1
};


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

class Transform : public Component {
public:
    float velocity = 2.0f;
    float rotationSpeed = 2.0f; // [추가] 회전 속도

    bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false;
    bool rotateLeft = false, rotateRight = false; // [추가] 회전 입력 상태

    int playerType = 0;
    float rotationY = 0.0f; // [추가] Y축 회전 각도 (라디안)

    float z = 0.0f;

    Transform(int type) { playerType = type; }

    void Start() override {
        // 1. 정점 버퍼 (Vertex Buffer) 생성
        D3D11_BUFFER_DESC vbd = { sizeof(g_sphereVertices), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
        D3D11_SUBRESOURCE_DATA vInitData = { g_sphereVertices, 0, 0 };
        g_pd3dDevice->CreateBuffer(&vbd, &vInitData, &pOwner->pVBuffer);

        // 2. 인덱스 버퍼 (Index Buffer) 생성
        D3D11_BUFFER_DESC ibd = { sizeof(g_sphereIndices), D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0, 0 };
        D3D11_SUBRESOURCE_DATA iInitData = { g_sphereIndices, 0, 0 };
        g_pd3dDevice->CreateBuffer(&ibd, &iInitData, &pOwner->pIBuffer);
    }

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

        rotateLeft = (GetAsyncKeyState('3') & 0x8000);
        rotateRight = (GetAsyncKeyState('4') & 0x8000);
    }

    void OnUpdate(float dt) override {
        // 1. 자신의 좌표 이동 로직
        if (moveLeft)  pOwner->x -= velocity * dt;
        if (moveRight) pOwner->x += velocity * dt;
        if (moveUp)    pOwner->y += velocity * dt;
        if (moveDown)  pOwner->y -= velocity * dt;

        // [추가] 2. 회전 로직 (라디안 값 누적)
        if (rotateLeft)  rotationY -= rotationSpeed * dt;
        if (rotateRight) rotationY += rotationSpeed * dt;
    }

    DirectX::XMMATRIX GetWorldMatrix() {
        // [핵심 수정] 크기(S) * 회전(R) * 이동(T) 순서로 곱해야 로컬 회전이 됩니다!

        // 1. 제자리에서 Y축을 기준으로 회전하는 행렬 생성
        DirectX::XMMATRIX mRot = DirectX::XMMatrixRotationY(rotationY);

        // 2. 원하는 좌표로 이동하는 행렬 생성
        DirectX::XMMATRIX mTrans = DirectX::XMMatrixTranslation(pOwner->x, pOwner->y, z);

        // 3. 회전을 먼저 하고, 그 다음에 이동시킵니다 (mRot * mTrans)
        return mRot * mTrans;
    }
};

class CameraController : public Component {
public:
    float angle = 0.0f;     // 현재 회전 각도 (라디안)
    float radius = 5.0f;    // 중심(0,0,0)으로부터의 카메라 거리
    float speed = 2.0f;     // 카메라 회전 속도

    void Start() override {}

    void OnUpdate(float dt) override {
        // 1번, 2번 키 입력에 따라 각도 조절
        if (GetAsyncKeyState('1') & 0x8000) angle -= speed * dt;
        if (GetAsyncKeyState('2') & 0x8000) angle += speed * dt;

        // 핵심: 삼각함수를 이용해 원주 위의 X, Z 좌표를 구합니다.
        // 처음 시작 위치가 (0, 0, -5) 이므로, -cos를 사용하여 기준을 맞춥니다.
        float x = sin(angle) * radius;
        float z = -cos(angle) * radius;

        // 갱신된 위치를 전역 카메라 변수에 적용
        g_Eye = DirectX::XMVectorSet(x, 0.0f, z, 0.0f);
    }
};

class MeshRenderer : public Component {
public:
    void Start() override {}
    void OnUpdate(float dt) override {}

    void OnRender() override {
        if (!pOwner->pVBuffer) return;

        DirectX::XMMATRIX mView = DirectX::XMMatrixLookAtLH(g_Eye, g_At, g_Up);
        DirectX::XMMATRIX mProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 800.0f / 600.0f, 0.01f, 100.0f);

        DirectX::XMMATRIX mWorld = DirectX::XMMatrixIdentity();
        for (auto comp : pOwner->components) {
            Transform* transform = dynamic_cast<Transform*>(comp);
            if (transform) {
                mWorld = transform->GetWorldMatrix();
                break;
            }
        }

        ConstantBuffer cb;

        cb.mWorldViewProj = DirectX::XMMatrixTranspose(mWorld * mView * mProjection);
        cb.mWorld = DirectX::XMMatrixTranspose(mWorld);
        cb.LightDir = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f); // 빛이 들어오는 임의의 뱡향
        cb.padding1 = 0.0f;
        cb.ViewPos = DirectX::XMFLOAT3(DirectX::XMVectorGetX(g_Eye), DirectX::XMVectorGetY(g_Eye), DirectX::XMVectorGetZ(g_Eye));
        cb.padding2 = 0.0f;

        g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
        g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
        g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer); // 픽셀 쉐이더에서도 상수 버퍼를 쓰도록 추가

        g_pImmediateContext->IASetInputLayout(g_pInputLayout);
        UINT stride = sizeof(Vertex), offset = 0;
        g_pImmediateContext->IASetVertexBuffers(0, 1, &pOwner->pVBuffer, &stride, &offset);

        g_pImmediateContext->IASetIndexBuffer(pOwner->pIBuffer, DXGI_FORMAT_R16_UINT, 0);
        g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        g_pImmediateContext->VSSetShader(g_vShader, nullptr, 0);
        g_pImmediateContext->PSSetShader(g_pShader, nullptr, 0);

        g_pImmediateContext->DrawIndexed(60, 0, 0);
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

    // --- [추가] 3. 매 프레임 깊이 버퍼를 1.0(가장 먼 거리)으로 초기화 ---
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // --- [수정] 4. 파이프라인에 렌더 타겟과 깊이 스텐실 뷰를 동시에 연결 ---
    // (기존 nullptr 자리에 g_pDepthStencilView를 넣어줍니다)
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

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

    // --- [추가] 1. 깊이 버퍼 텍스처(Z-Buffer) 생성 ---
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = 800;
    descDepth.Height = 600;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24비트는 깊이(Z), 8비트는 스텐실
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil);

    // --- [추가] 2. 깊이 스텐실 뷰(DSV) 생성 ---
    g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, nullptr, &g_pDepthStencilView);

    // 4. 셰이더 및 버퍼 설정 (이전과 동일한 로직)
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(
        L"VertexShader.hlsl",               // 파일 경로
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,  // HLSL 내부의 #include를 인식하게 해줌
        "VS",                               // 엔트리 포인트(함수명)
        "vs_4_0",                           // 셰이더 타겟 버전
        0, 0, &vsBlob, nullptr
    );
    hr = D3DCompileFromFile(
        L"PixelShader_Lambertian.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PS",
        "ps_4_0",
        0, 0, &psBlob, nullptr
    );


    g_pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_vShader);
    g_pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_pShader);

    // Input Layout 수정 (WinMain 내부)
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // 새로 추가됨
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // 오프셋 12 -> 24로 변경
    };

    // (이후 CreateInputLayout의 두 번째 인자인 배열 개수도 2에서 3으로 변경해야 합니다)
    g_pd3dDevice->CreateInputLayout(layout, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_pInputLayout);


    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;

    g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer);

    if (vsBlob) vsBlob->Release();
    if (psBlob) psBlob->Release();

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
    sysInfo->AddComponent(new CameraController()); // <-- 새로 만든 카메라 컨트롤러 추가!
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
    if (g_pDepthStencilView) g_pDepthStencilView->Release(); // [추가]
    if (g_pDepthStencil) g_pDepthStencil->Release();         // [추가]
    if (g_pInputLayout) g_pInputLayout->Release();
    if (g_vShader) g_vShader->Release();
    if (g_pShader) g_pShader->Release();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();

    return 0;
}