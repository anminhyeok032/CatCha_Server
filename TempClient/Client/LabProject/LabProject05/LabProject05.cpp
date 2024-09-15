// LabProject05.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "stdafx.h"
#include "LabProject05.h"
#include "GameFramework.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
CGameFramework gGameFramework;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int g_myid;
SOCKET g_server_socket;
SOCKET g_udp_socket;
SOCKADDR_IN g_server_a;
std::string avatar_name;
WSAOVERLAPPED g_wsaover;





//#define ENTERIP
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LABPROJECT05, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABPROJECT05));

    MSG msg;

    //==============================================
    // 서버 초기화 작업
    //==============================================

    std::cout << "Enter User Name : ";
    //std::cin >> avatar_name;
    avatar_name = "Temp01";
#ifdef ENTERIP
    char SERVER_ADDR[BUFSIZE];
    std::cout << "\nEnter IP Address : ";
    std::cin.getline(SERVER_ADDR, BUFSIZE);
#endif
    char SERVER_ADDR[BUFSIZE] = "127.0.0.1";
    
    int res;
    std::wcout.imbue(std::locale("korean"));
    WSADATA WSAData;
    res = WSAStartup(MAKEWORD(2, 2), &WSAData);
    if(0 != res)
    {
        print_error("WSAStartup", WSAGetLastError());
    }
    g_server_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    g_server_a.sin_family = AF_INET;
    g_server_a.sin_port = htons(PORT);

    inet_pton(AF_INET, SERVER_ADDR, &g_server_a.sin_addr);
    res = connect(g_server_socket, reinterpret_cast<sockaddr*>(&g_server_a), sizeof(g_server_a));
    if (SOCKET_ERROR == res)
    {
        //print_error("Connect", WSAGetLastError());
        closesocket(g_server_socket);
        WSACleanup();
        exit(0);
    }

    // Nagle 알고리즘 OFF
    int DelayZeroOpt = 1;
    setsockopt(g_server_socket, SOL_SOCKET, TCP_NODELAY, (const char*)&DelayZeroOpt, sizeof(DelayZeroOpt));

    // TODO: 해당 로그인 부분은 로그인 서버 완성시 그쪽으로 옮겨질 코드
    CS_LOGIN_PACKET p;
    p.size = sizeof(p);
    p.type = CS_LOGIN;
    strcpy_s(p.name, avatar_name.c_str());
    DoSend(&p);

    unsigned long noblock = 1;
    int nRet = ioctlsocket(g_server_socket, FIONBIO, &noblock);


    // UDP 소켓 생성 및 초기화
    g_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_udp_socket == INVALID_SOCKET)
    {
        print_error("UDP socket creation failed", WSAGetLastError());
        return -1;
    }

    // UDP 소켓 비동기 모드 설정
    u_long mode = 1;
    ioctlsocket(g_udp_socket, FIONBIO, &mode);

    // 서버 주소 설정 (예: 로컬 호스트와 특정 포트로 설정)
    sockaddr_in udpServerAddr;
    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_port = htons(PORT);  // 서버의 UDP 포트
    inet_pton(AF_INET, SERVER_ADDR, &udpServerAddr.sin_addr);

    // UDP 소켓을 서버에 바인딩
    bind(g_udp_socket, (sockaddr*)&udpServerAddr, sizeof(udpServerAddr));


    while (1)
    {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;
            if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }
        else
        {
            gGameFramework.FrameAdvance();
        }
        DoRecv();
    }
    gGameFramework.OnDestroy();
    return (int)msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LABPROJECT05));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU |
        WS_BORDER;
    RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
    AdjustWindowRect(&rc, dwStyle, FALSE);
    HWND hWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT,
        CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
        NULL);

    if (!hWnd) return FALSE;

    gGameFramework.OnCreate(hInstance, hWnd);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

#ifdef _WITH_SWAPCHAIN_FULLSCREEN_STATE
    gGameFramework.ChangeSwapChainState();
#endif

    return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_MOUSEWHEEL:
        gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
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
