#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <vector>
#include <string>


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

//ウィンドウプロシージャ
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    //メッセージに応じてゲーム固有の処理を行う
    switch (msg) {
        //ウィンドウが破棄された
    case WM_DESTROY:
        //osに対して、アプリの終了を伝える
        PostQuitMessage(0);
        return 0;
    }
    //標準メッセージ処理を行う
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

//windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    //コンソールへの文字出力
    OutputDebugStringA("Hellow,DirectX!!\n");
    //ウィンドウサイズ
// ウィンドウ横幅
    const int WIN_WIDTH = 1280;
    // ウィンドウ縦幅
    const int WIN_HEIGHT = 720;

    //ウィンドウクラスの設定
    WNDCLASSEX w{};
    w.cbSize = sizeof(WNDCLASSEX);
    w.lpfnWndProc = (WNDPROC)WindowProc;//ウィンドウプロシージャを設定
    w.lpszClassName = L"DirectXGame";//ウィンドクラス名
    w.hInstance = GetModuleHandle(nullptr);//ウィンドハンドル
    w.hCursor = LoadCursor(NULL, IDC_ARROW);//カーソル指定

    //ウィンドクラスを05に登録する
    RegisterClassEx(&w);
    //ウィンドサイズ{X座標　Y座標　横幅　縦幅}
    RECT wrc = { 0, 0, WIN_WIDTH, WIN_HEIGHT };
    //自動でサイズを補正する
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    //ウィンドウオブジェクトの生成
    HWND hwnd = CreateWindow(w.lpszClassName,//クラス名
        L"DirectXGame",//タイトルバーの文字
        WS_OVERLAPPEDWINDOW,//標準的なウィンドウスタイル
        CW_USEDEFAULT,//標準X座標 (05に任せる)
        CW_USEDEFAULT,//標準Y座標 (05に任せる)
        wrc.right - wrc.left,//ウィンドウ横幅
        wrc.bottom - wrc.top,//ウィンドウ縦幅
        nullptr,
        nullptr,
        w.hInstance,
        nullptr);
    //ウィンドウを表示状態にする
    ShowWindow(hwnd, SW_SHOW);

    MSG msg{};//メッセージ

   //DirectX初期化処理　ここから
#ifdef _DEBUG
//デバックレイヤーをオンに
    ID3D12Debug* debugCountroller;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugCountroller)))) {
        debugCountroller->EnableDebugLayer();
    }
#endif

    HRESULT result;
    ID3D12Device* device = nullptr;
    IDXGIFactory7* dxgiFactory = nullptr;
    IDXGISwapChain4* swapChain = nullptr;
    ID3D12CommandAllocator* cmdAllocator = nullptr;
    ID3D12GraphicsCommandList* commandList = nullptr;
    ID3D12CommandQueue* commandQueue = nullptr;
    ID3D12DescriptorHeap* rtvHeap = nullptr;

    //DXGIファクトリーの生成
    result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
    assert(SUCCEEDED(result));

    //アダプターの列挙用
    std::vector<IDXGIAdapter4*> adapters;
    //ここに特定の名前を持つアダプターオブジェクトが入る
    IDXGIAdapter4* tmpAdapter = nullptr;

    //パフォーマンスが高いものから順に、全てのアダプターを列挙する
    for (UINT i = 0;
        dxgiFactory->EnumAdapterByGpuPreference(i,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND;
        i++) {
        //動的配列に追加する
        adapters.push_back(tmpAdapter);
    }

    //妥当なアダプターを選別する
    for (size_t i = 0; i < adapters.size(); i++) {
        DXGI_ADAPTER_DESC3 adapterDesc;
        //アダプターの情報を取得する
        adapters[i]->GetDesc3(&adapterDesc);

        //ソフトウェアデバイスを回避
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            //デバイスを採用してループを抜ける
            tmpAdapter = adapters[i];
            break;
        }
    }

    //対応レベルの配列
    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    D3D_FEATURE_LEVEL featureLevel;

    for (size_t i = 0; i < _countof(levels); i++) {
        //採用したアダプターでデバイスを生成
        result = D3D12CreateDevice(tmpAdapter, levels[i],
            IID_PPV_ARGS(&device));
        if (result == S_OK) {
            //デバイスを生成できた時点でループを抜ける
            featureLevel = levels[i];
            break;
        }
    }

    //コマンドアロケーターを生成
    result = device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&cmdAllocator));
    assert(SUCCEEDED(result));

    //コマンドリストを生成
    result = device->CreateCommandList(0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        cmdAllocator, nullptr,
        IID_PPV_ARGS(&commandList));
    assert(SUCCEEDED(result));

    //コマンドキューの設定
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    //コマンドキューを生成
    result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
    assert(SUCCEEDED(result));

    //スワップチェーンの設定
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = 1280;
    swapChainDesc.Height = 720;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色情報の書式
    swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;//バックバッファ用
    swapChainDesc.BufferCount = 2;//バッファ数を2つに設定
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;// フリップ用は破棄
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    //スワップチェーンの生成
    result = dxgiFactory->CreateSwapChainForHwnd(
        commandQueue, hwnd, &swapChainDesc, nullptr, nullptr,
        (IDXGISwapChain1**)&swapChain);
    assert(SUCCEEDED(result));

    //デスクリプタヒープの設定
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//レンゲーターゲットレビュー
    rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;//裏表の2つ

    //デスクリプタヒープの生成
    device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

    //バックバッファ
    std::vector<ID3D12Resource*> backBuffers;
    backBuffers.resize(swapChainDesc.BufferCount);

    //スワップチェーンの全てのバッファについて処理する
    for (size_t i = 0; i < backBuffers.size(); i++) {
        //スワップチェーンからバッファを取得
        swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
        //デクリプタヒープからバッファを取得
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        //裏か表かでアドレスがずれる
        rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
        //レンダーターゲットレビューの設定
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
        //シェーダーの計算結果をSRGBに変換して書き込む
        rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        //レンダーターゲットレビューの生成
        device->CreateRenderTargetView(backBuffers[i], &rtvDesc, rtvHandle);
    }

    //フェンスの生成
    ID3D12Fence* fence = nullptr;
    UINT64 fenceVal = 0;

    result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));



   //DirectX初期化処理　ここまで

    while (true) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);//キー入力メッセージの処理
            DispatchMessage(&msg);//プロシージャーにメッセージを送る
        }

        //バツボタンで終了メッセージが来たらゲームループを抜ける
        if (msg.message == WM_QUIT) {
            break;
        }
        //DirectX毎フレーム処理　ここから

        //DirectX毎フレーム処理　ここまで
    }

    //ウィンドウクラスを登録解除
    UnregisterClass(w.lpszClassName, w.hInstance);

   
    return 0;
}
