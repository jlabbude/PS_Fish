#include <iostream>
#include <windows.h>
#include <wincodec.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <codecvt>

using namespace std;
using namespace cv;

static double storedDiff = 300;

bool findApplicationWindow(LPCSTR name) {
    HWND hwnd = FindWindow(NULL, name);
    if (hwnd != NULL) {
        SetForegroundWindow(hwnd);
        cout << "Window found!\n" << endl;
        return true;
    }
    else {
        cout << "Window not found!\n" << endl;
        return false;
    }
}

void takeScreenshot(int left, int top, int width, int height, LPCWSTR filename) {
    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
    HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
    BOOL bRet = BitBlt(hDC, 0, 0, width, height, hScreen, left, top, SRCCOPY);

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

    IWICBitmap* pWICBitmap = NULL;
    pFactory->CreateBitmapFromHBITMAP(hBitmap, NULL, WICBitmapUseAlpha, &pWICBitmap);

    GUID format = GUID_WICPixelFormat32bppBGRA;
    pFrame->SetPixelFormat(&format);

    pFrame->WriteSource(pWICBitmap, NULL);
    pFrame->Commit();
    pEncoder->Commit();

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
}

vector<Rect> findOnScreen(string image, double threshold) {

    takeScreenshot(0, 0, 1920, 1080, L"screenfind.png");

    Mat screen = imread("screenfind.png");
    Mat templateImage = imread(image);

    Mat result;
    matchTemplate(screen, templateImage, result, TM_CCOEFF_NORMED);

    Mat locations;
    cv::threshold(result, result, threshold, 1.0, cv::THRESH_TOZERO);
    cv::findNonZero(result, locations);

    vector<Rect> boundingBoxes;
    for (int i = 0; i < locations.total(); ++i) {
        Point loc = locations.at<Point>(i);
        Rect boundingBox(loc.x, loc.y, templateImage.cols, templateImage.rows);
        boundingBoxes.push_back(boundingBox);
    }

    return boundingBoxes;
}

bool compareScreenshots(LPCSTR image0name, LPCSTR image1name) {

    Mat image0 = imread(image0name);
    Mat image1 = imread(image1name);

    Mat hsvImage0, hsvImage1;
    cvtColor(image0, hsvImage0, COLOR_BGR2HSV);
    cvtColor(image1, hsvImage1, COLOR_BGR2HSV);

    Scalar lowerBound = Scalar(52, 201, 255);
    Scalar upperBound = Scalar(102, 251, 255);

    Mat filteredImage0, filteredImage1;
    inRange(hsvImage0, lowerBound, upperBound, filteredImage0);
    inRange(hsvImage1, lowerBound, upperBound, filteredImage1);
    
    Mat morphed0, morphed1;
    Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
    morphologyEx(filteredImage0, morphed0, MORPH_CLOSE, kernel);
    morphologyEx(filteredImage1, morphed1, MORPH_CLOSE, kernel);

    vector<vector<Point>> contours0, contours1;
    findContours(morphed0, contours0, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    findContours(morphed1, contours1, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    double maxArea0 = 0, maxArea1 = 0;
    for (size_t i = 0; i < contours0.size(); i++) {
        double area = contourArea(contours0[i]);
        if (area > maxArea0) {
            maxArea0 = area;
        }
    }
    for (size_t i = 0; i < contours1.size(); i++) {
        double area = contourArea(contours1[i]);
        if (area > maxArea1) {
            maxArea1 = area;
        }
    }

    double areaDifference = abs(maxArea1 - maxArea0);
    if (areaDifference > storedDiff) {
        cout << "true: " << areaDifference << endl;
        return true;
    }
    else {
        cout << "false: "<< areaDifference << endl;
        storedDiff = areaDifference;
        return false;
    }
}

int main() {
    if (findApplicationWindow("Roblox") == true) {
        while (true) {
            vector<Rect> fishingBarRegion = findOnScreen("C:\\Users\\João\\Desktop\\codigo\\cpp\\Slayerbot\\SlayerBot\\fishing_bar.png", 0.7);
            vector<Rect> fishingButtonRegion = findOnScreen("C:\\Users\\João\\Desktop\\codigo\\cpp\\Slayerbot\\SlayerBot\\fishing_button.png", 0.7);
            if (!fishingBarRegion.empty()) {
                SetCursorPos(((fishingButtonRegion[0].x + fishingButtonRegion[0].width) / 2), (fishingButtonRegion[0].y + (fishingButtonRegion[0].height) / 2));
                while (1) {
                    // todo develop break method
                    takeScreenshot(fishingBarRegion[0].x, fishingBarRegion[0].y, (fishingBarRegion[0].x + fishingBarRegion[0].width), (fishingBarRegion[0].x + fishingBarRegion[0].height), L"screen_new.png");
                    if (compareScreenshots("screen.png", "screen_new.png")) {
                        mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, ((fishingButtonRegion[0].x + fishingButtonRegion[0].width) / 2), (fishingButtonRegion[0].y + (fishingButtonRegion[0].height) / 2), 0, 0);
                        Sleep(200);
                        takeScreenshot(fishingBarRegion[0].x, fishingBarRegion[0].y, (fishingBarRegion[0].x + fishingBarRegion[0].width), (fishingBarRegion[0].x + fishingBarRegion[0].height), L"screen.png");
                    }
                }
            }
            else {
                cout << "No fishing" << endl;
            }
        }
    }
    return 0;
}