#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include "baseWindow.h"

#include "krbcam_paintWindow.hpp"

#define KRBCAM_NAME                 L"KRbCam: iXon Fast Kinetics Imaging"
#define KRBCAM_NAME_PAINTWINDOW     L"KRbCam: Images"

#define KRBCAM_PAINTWINDOW_WIDTH 400
#define KRBCAM_PAINTWINDOW_HEIGHT 300

#define ID_PAINTWINDOW 0x20

extern int HandleMessagesControl(UINT uMsg, WPARAM wParam, LPARAM lParam);
extern int InitializeSDK(void);
extern int exitGracefully(void);
extern BOOL _CreateWindows(HINSTANCE hInstance, HWND hParent);
extern int SetupAcquisition(void);

HWND gHandleMain;

class MainWindow : public BaseWindow<MainWindow>
{
public:
    PCWSTR ClassName() const {return L"Main Window";}
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(m_hwnd, &ps);

                FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

                EndPaint(m_hwnd, &ps);
            }
            return 0;
        default:
            HandleMessagesControl(uMsg, wParam, lParam);
            return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
        }

        return TRUE;
    }
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    MainWindow win;
    PaintWindow paintWin;

    if (!win.Create(KRBCAM_NAME, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU)) {
        return 0;
    }

    BOOL paintWinCreate = paintWin.Create(KRBCAM_NAME_PAINTWINDOW,
                                            WS_OVERLAPPEDWINDOW,
                                            0,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            KRBCAM_PAINTWINDOW_WIDTH,
                                            KRBCAM_PAINTWINDOW_HEIGHT
                                            );

    if (!paintWinCreate)
        return 0;

    _CreateWindows(hInstance, win.Window());
    gHandleMain = win.Window();
    InitializeSDK();

    // Wait for 2 seconds to allow MCD to calibrate fully before allowing an
    // acquisition to begin
    int time1(GetTickCount()), time2;
    do{
        time2 = GetTickCount() - time1;
    } while (time2 < 2000);

    ShowWindow(win.Window(), nCmdShow);
    ShowWindow(paintWin.Window(), nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    exitGracefully();
    return 0;
}