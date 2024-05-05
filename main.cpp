#include <iostream>
#include <windows.h>
#include <wincodec.h>
#include <opencv2/opencv.hpp>

using namespace std;

bool compareScreenshots(int x, int y, int width, int height, double threshold) {

    //todo

    wstring name = L"teste1.png";
    wstring name2 = L"teste2.png";

    takeScreenshot(x, y, width, height, name.c_str());
    takeScreenshot(x, y, width, height, name2.c_str());

    cout << "Done";

    return false;
}

void takeScreenshot(int left, int top, int width, int height, LPCWSTR filename) {
    // Copy screen to bitmap
    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
    HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
    BOOL bRet = BitBlt(hDC, 0, 0, width, height, hScreen, left, top, SRCCOPY);

    // Save bitmap to PNG file
    IWICImagingFactory* pFactory = NULL;
    IWICBitmapEncoder* pEncoder = NULL;
    IWICBitmapFrameEncode* pFrame = NULL;
    IWICStream* pStream = NULL;

    CoInitialize(NULL);
    CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
    pFactory->CreateStream(&pStream);
    pStream->InitializeFromFilename(filename, GENERIC_WRITE);
    pFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
    pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
    pEncoder->CreateNewFrame(&pFrame, NULL);
    pFrame->Initialize(NULL);
    pFrame->SetSize(width, height);

    // Convert HBITMAP to IWICBitmapSource
    IWICBitmap* pWICBitmap = NULL;
    pFactory->CreateBitmapFromHBITMAP(hBitmap, NULL, WICBitmapUseAlpha, &pWICBitmap);

    // Set pixel format
    GUID format = GUID_WICPixelFormat32bppBGRA;
    pFrame->SetPixelFormat(&format);

    // Write bitmap source
    pFrame->WriteSource(pWICBitmap, NULL);
    pFrame->Commit();
    pEncoder->Commit();

    // Clean up
    if (pWICBitmap) pWICBitmap->Release();
    if (pFrame) pFrame->Release();
    if (pEncoder) pEncoder->Release();
    if (pStream) pStream->Release();
    if (pFactory) pFactory->Release();
    SelectObject(hDC, old_obj);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
    DeleteObject(hBitmap);
    CoUninitialize();

    cout << "done1";
}