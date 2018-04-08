#ifndef KRBCAM_PAINTWINDOW_HPP
#define KRBCAM_PAINTWINDOW_HPP

#include "baseWindow.h"

#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")

#include<string>
#include<fstream>


template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


class PaintWindow : public BaseWindow<PaintWindow>
{
	ID2D1Factory            *pFactory;
    ID2D1HwndRenderTarget   *pRenderTarget;
    ID2D1SolidColorBrush    *pBrush;
    D2D1_SIZE_U            	bitmapSize;
    long					*pBitmapRaw;

    void    CalculateLayout();
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();

    long*	GetBitmapFromData();

public:
	PaintWindow() : pFactory(NULL), pRenderTarget(NULL), pBrush(NULL)
    {
    }

    PCWSTR  ClassName() const { return L"PaintWindow Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);	
};


void PaintWindow::CalculateLayout()
{
    if (pRenderTarget != NULL)
    {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        bitmapSize = D2D1::SizeU((int)size.width, (int)size.height);
    }
}

HRESULT PaintWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);

        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 0);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);

            if (SUCCEEDED(hr))
            {
                CalculateLayout();
            }
        }
    }
    return hr;
}

long* PaintWindow::GetBitmapFromData() {
	std::ifstream fs;
	fs.open("\\data\\20180406\\iXon_img24a.csv");

	if (pBitmapRaw) {
		delete pBitmapRaw;
	}

	pBitmapRaw = new long[512 * 1024];
	int index;

	return NULL;
}

void PaintWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void PaintWindow::OnPaint()
{
    // HRESULT hr = CreateGraphicsResources();
    // if (SUCCEEDED(hr))
    // {
    //     PAINTSTRUCT ps;
    //     BeginPaint(m_hwnd, &ps);
     
    //     pRenderTarget->BeginDraw();

    //     pRenderTarget->Clear( D2D1::ColorF(D2D1::ColorF::SkyBlue) );
    //     pRenderTarget->FillEllipse(ellipse, pBrush);

    //     hr = pRenderTarget->EndDraw();
    //     if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
    //     {
    //         DiscardGraphicsResources();
    //     }
    //     EndPaint(m_hwnd, &ps);
    // }
}

void PaintWindow::Resize()
{
    // if (pRenderTarget != NULL)
    // {
    //     RECT rc;
    //     GetClientRect(m_hwnd, &rc);

    //     D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

    //     pRenderTarget->Resize(size);
    //     CalculateLayout();
    //     InvalidateRect(m_hwnd, NULL, FALSE);
    // }
}

LRESULT PaintWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            return -1;  // Fail CreateWindowEx.
        }
        return 0;

    case WM_DESTROY:
        DiscardGraphicsResources();
        SafeRelease(&pFactory);
        return 0;

    case WM_PAINT:
        /// OnPaint();
        return 0;


    case WM_SIZE:
        // Resize();
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}




#endif