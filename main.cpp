#include <Windows.h>
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
    return 0;
}
