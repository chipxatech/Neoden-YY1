#define UNICODE
#define _UNICODE

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "converter.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "user32.lib")

typedef void* GpBitmap;
typedef void* GpGraphics;

typedef struct {
    UINT32 GdiplusVersion;
    void* DebugEventCallback;
    BOOL SuppressBackgroundThread;
    BOOL SuppressExternalCodecs;
} GdiplusStartupInput;

__declspec(dllimport) int __stdcall GdiplusStartup(ULONG_PTR *token, const GdiplusStartupInput *input, void *output);
__declspec(dllimport) void __stdcall GdiplusShutdown(ULONG_PTR token);
__declspec(dllimport) int __stdcall GdipCreateBitmapFromFile(const WCHAR* filename, GpBitmap **bitmap);
__declspec(dllimport) int __stdcall GdipCreateFromHDC(HDC hdc, GpGraphics **graphics);
__declspec(dllimport) int __stdcall GdipDrawImageRectI(GpGraphics *graphics, GpBitmap *image, INT x, INT y, INT width, INT height);
__declspec(dllimport) int __stdcall GdipDisposeImage(GpBitmap *image);
__declspec(dllimport) int __stdcall GdipDeleteGraphics(GpGraphics *graphics);

HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;
HWND g_hSplash = NULL;
HWND g_hEditInput = NULL;
HWND g_hEditTop = NULL;
HWND g_hEditBot = NULL;
HWND g_hListView = NULL;
HWND g_hStatus = NULL;
HWND g_hRadioTop = NULL;
HWND g_hRadioBot = NULL;
HFONT g_hFontTitle = NULL;
HFONT g_hFontNormal = NULL;
HFONT g_hFontBold = NULL;
HFONT g_hFontSplashTitle = NULL;
HFONT g_hFontSplashSub = NULL;

ULONG_PTR g_gdiToken = 0;
GpBitmap* g_pLogoBitmap = NULL;
int g_splashProgress = 0;
bool g_showing_top = true;

ComponentList g_components;

static void refresh_list_view(void) {
    ListView_DeleteAllItems(g_hListView);

    size_t shown_count = 0;
    size_t top_count = 0, bot_count = 0;

    for (size_t i = 0; i < g_components.count; ++i) {
        const Component* c = &g_components.items[i];
        bool is_bot = (strstr(c->layer, "Bottom") || strcmp(c->layer, "BottomLayer") == 0 || strstr(c->layer, "bot"));
        if (is_bot) bot_count++; else top_count++;

        if ((g_showing_top && is_bot) || (!g_showing_top && !is_bot)) {
            continue;
        }

        LVITEMW item = {0};
        item.mask = LVIF_TEXT;
        item.iItem = (int)shown_count;
        wchar_t stt[32];
        swprintf(stt, 32, L"%zu", shown_count + 1);
        item.pszText = stt;
        ListView_InsertItem(g_hListView, &item);

        wchar_t w_des[MAX_STR], w_cmt[MAX_STR], w_fp[MAX_STR];
        MultiByteToWideChar(CP_UTF8, 0, c->designator, -1, w_des, MAX_STR);
        MultiByteToWideChar(CP_UTF8, 0, c->comment, -1, w_cmt, MAX_STR);
        MultiByteToWideChar(CP_UTF8, 0, c->footprint, -1, w_fp, MAX_STR);

        ListView_SetItemText(g_hListView, (int)shown_count, 1, w_des);
        ListView_SetItemText(g_hListView, (int)shown_count, 2, w_cmt);
        ListView_SetItemText(g_hListView, (int)shown_count, 3, w_fp);

        wchar_t buf[64];
        swprintf(buf, 64, L"%.2f", c->mid_x); ListView_SetItemText(g_hListView, (int)shown_count, 4, buf);
        swprintf(buf, 64, L"%.2f", c->mid_y); ListView_SetItemText(g_hListView, (int)shown_count, 5, buf);
        swprintf(buf, 64, L"%.2f", c->rotation); ListView_SetItemText(g_hListView, (int)shown_count, 6, buf);
        swprintf(buf, 64, L"%d", c->head); ListView_SetItemText(g_hListView, (int)shown_count, 7, buf);
        swprintf(buf, 64, L"%d", c->feeder_no); ListView_SetItemText(g_hListView, (int)shown_count, 8, buf);
        swprintf(buf, 64, L"%d", c->mount_speed); ListView_SetItemText(g_hListView, (int)shown_count, 9, buf);
        swprintf(buf, 64, L"%.2f", c->pick_height); ListView_SetItemText(g_hListView, (int)shown_count, 10, buf);
        swprintf(buf, 64, L"%.2f", c->place_height); ListView_SetItemText(g_hListView, (int)shown_count, 11, buf);
        swprintf(buf, 64, L"%d", c->mode); ListView_SetItemText(g_hListView, (int)shown_count, 12, buf);
        ListView_SetItemText(g_hListView, (int)shown_count, 13, (LPWSTR)L"");

        shown_count++;
    }

    wchar_t status_txt[256];
    swprintf(status_txt, 256, L"✔ Mặt TOP: %zu pcs  |  Mặt BOTTOM: %zu pcs  |  Đang hiển thị: %s (%zu pcs)  |  Nhấp đúp chuột để sửa",
             top_count, bot_count, g_showing_top ? L"TOP" : L"BOTTOM", shown_count);
    SetWindowTextW(g_hStatus, status_txt);
}

static void load_and_display_data(const wchar_t* wpath) {
    char path_mb[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, wpath, -1, path_mb, sizeof(path_mb), NULL, NULL);

    char error_msg[256] = {0};
    if (!read_altium_file(path_mb, &g_components, error_msg, sizeof(error_msg))) {
        wchar_t w_err[256];
        MultiByteToWideChar(CP_UTF8, 0, error_msg, -1, w_err, sizeof(w_err)/sizeof(wchar_t));
        MessageBoxW(g_hWnd, w_err, L"Lỗi Đọc File", MB_ICONERROR);
        return;
    }

    refresh_list_view();
}

static void on_save_clicked(void) {
    if (g_components.count == 0) {
        MessageBoxW(g_hWnd, L"Chưa có dữ liệu để lưu!", L"Thông Báo", MB_ICONWARNING);
        return;
    }

    wchar_t w_top[MAX_PATH], w_bot[MAX_PATH];
    GetWindowTextW(g_hEditTop, w_top, MAX_PATH);
    GetWindowTextW(g_hEditBot, w_bot, MAX_PATH);

    char top_out[MAX_PATH], bot_out[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, w_top, -1, top_out, sizeof(top_out), NULL, NULL);
    WideCharToMultiByte(CP_UTF8, 0, w_bot, -1, bot_out, sizeof(bot_out), NULL, NULL);

    LayerSummary top_sum, bot_sum;
    memset(&top_sum, 0, sizeof(top_sum));
    memset(&bot_sum, 0, sizeof(bot_sum));
    char error_msg[256] = {0};

    bool ok_top = export_layer(&g_components, "Top", top_out, &top_sum, error_msg, sizeof(error_msg));
    bool ok_bot = export_layer(&g_components, "Bottom", bot_out, &bot_sum, error_msg, sizeof(error_msg));

    if (ok_top && ok_bot) {
        wchar_t msg[512];
        swprintf(msg, 512,
            L"🎉 ĐÃ LƯU BẢN CHỈNH SỬA CHO MÁY NEODEN YY1!\n\n"
            L"⭐ Mặt TOP:\n   • Số linh kiện: %zu\n   • File: %s\n\n"
            L"⭐ Mặt BOTTOM:\n   • Số linh kiện: %zu\n   • File: %s\n\n"
            L"Toàn bộ 13 thông số đã chỉnh sửa được lưu chính xác 100%.\nBạn có muốn mở thư mục chứa file vừa tạo?",
            top_sum.total_components, w_top,
            bot_sum.total_components, w_bot
        );

        if (MessageBoxW(g_hWnd, msg, L"Lưu Thành Công", MB_ICONINFORMATION | MB_YESNO) == IDYES) {
            ShellExecuteW(NULL, L"open", L".", NULL, NULL, SW_SHOWNORMAL);
        }
    } else {
        wchar_t w_err[256];
        MultiByteToWideChar(CP_UTF8, 0, error_msg, -1, w_err, sizeof(w_err)/sizeof(wchar_t));
        MessageBoxW(g_hWnd, w_err, L"Lỗi Xuất File", MB_ICONERROR);
    }

    if (top_sum.feeder_items) free(top_sum.feeder_items);
    if (bot_sum.feeder_items) free(bot_sum.feeder_items);
}

// Splash Proc trong C thuần
LRESULT CALLBACK SplashProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        SetTimer(hWnd, 1, 20, NULL);
        break;

    case WM_TIMER:
        g_splashProgress += 2;
        InvalidateRect(hWnd, NULL, FALSE);
        if (g_splashProgress >= 100) {
            KillTimer(hWnd, 1);
            DestroyWindow(hWnd);
            ShowWindow(g_hWnd, SW_SHOWNORMAL);
            UpdateWindow(g_hWnd);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rect;
        GetClientRect(hWnd, &rect);

        HBRUSH hBg = CreateSolidBrush(RGB(18, 24, 38));
        FillRect(hdc, &rect, hBg);
        DeleteObject(hBg);

        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 210, 255));
        HGDIOBJ oldPen = SelectObject(hdc, hPen);
        HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, 0, 0, rect.right, rect.bottom);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(hPen);

        if (g_pLogoBitmap) {
            GpGraphics *graphics = NULL;
            if (GdipCreateFromHDC(hdc, &graphics) == 0 && graphics) {
                int imgW = 90, imgH = 90;
                int imgX = (rect.right - imgW) / 2, imgY = 20;
                GdipDrawImageRectI(graphics, g_pLogoBitmap, imgX, imgY, imgW, imgH);
                GdipDeleteGraphics(graphics);
            }
        }

        SetBkMode(hdc, TRANSPARENT);

        SelectObject(hdc, g_hFontSplashTitle);
        SetTextColor(hdc, RGB(248, 250, 252));
        RECT titleRect = {0, 122, rect.right, 154};
        DrawTextW(hdc, L"NeoDen YY1 SMT Converter Pro (C Native)", -1, &titleRect, DT_CENTER | DT_SINGLELINE);

        SelectObject(hdc, g_hFontSplashSub);
        SetTextColor(hdc, RGB(0, 210, 255));
        RECT authorRect = {0, 154, rect.right, 178};
        DrawTextW(hdc, L"Phát triển bởi: CÔNG TY TNHH CÔNG NGHỆ CHIPXA", -1, &authorRect, DT_CENTER | DT_SINGLELINE);

        SetTextColor(hdc, RGB(148, 163, 184));
        RECT subRect = {0, 178, rect.right, 202};
        DrawTextW(hdc, L"Automated Pick & Place Processor • 0603Demo.csv Embedded", -1, &subRect, DT_CENTER | DT_SINGLELINE);

        int barW = 380, barH = 10;
        int barX = (rect.right - barW) / 2, barY = 220;

        HBRUSH hProgBg = CreateSolidBrush(RGB(30, 41, 59));
        RECT barBgRect = {barX, barY, barX + barW, barY + barH};
        FillRect(hdc, &barBgRect, hProgBg);
        DeleteObject(hProgBg);

        int fillW = (barW * g_splashProgress) / 100;
        if (fillW > 0) {
            HBRUSH hProgFill = CreateSolidBrush(RGB(0, 210, 255));
            RECT barFillRect = {barX, barY, barX + fillW, barY + barH};
            FillRect(hdc, &barFillRect, hProgFill);
            DeleteObject(hProgFill);
        }

        const wchar_t* loadText = L"Đang khởi tạo hệ thống C Native...";
        if (g_splashProgress > 30 && g_splashProgress <= 65) loadText = L"Tích hợp file mẫu chuẩn 0603Demo.csv...";
        else if (g_splashProgress > 65 && g_splashProgress <= 90) loadText = L"Chuẩn bị ma trận Feeder 4 góc & bộ nạp Altium...";
        else if (g_splashProgress > 90) loadText = L"Khởi động hoàn tất!";

        SelectObject(hdc, g_hFontSplashSub);
        SetTextColor(hdc, RGB(0, 220, 180));
        RECT statusRect = {0, 240, rect.right, 268};
        DrawTextW(hdc, loadText, -1, &statusRect, DT_CENTER | DT_SINGLELINE);

        EndPaint(hWnd, &ps);
        break;
    }
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        component_list_init(&g_components);

        g_hFontTitle = CreateFontW(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        g_hFontBold = CreateFontW(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        g_hFontNormal = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

        HWND hGrp1 = CreateWindowExW(0, L"BUTTON", L" 1. File Altium Pick & Place Dau Vao ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 20, 75, 1320, 70, hWnd, NULL, g_hInst, NULL);
        SendMessageW(hGrp1, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hEditInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 35, 102, 1050, 26, hWnd, (HMENU)101, g_hInst, NULL);
        SendMessageW(g_hEditInput, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        HWND hBtnBrowse = CreateWindowExW(0, L"BUTTON", L"Chon File...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1100, 101, 110, 28, hWnd, (HMENU)102, g_hInst, NULL);
        SendMessageW(hBtnBrowse, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        HWND hBtnReload = CreateWindowExW(0, L"BUTTON", L"Tai Lai", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1220, 101, 90, 28, hWnd, (HMENU)103, g_hInst, NULL);
        SendMessageW(hBtnReload, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        // Khoi 2: 13 Cot va Chuyen doi TOP/BOTTOM
        HWND hGrp2 = CreateWindowExW(0, L"BUTTON", L" 2. Toan Bo 13 Cot Chuan NeoDen YY1 (Nhap dup chuot vao dong de sua) ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 20, 155, 1320, 460, hWnd, NULL, g_hInst, NULL);
        SendMessageW(hGrp2, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hRadioTop = CreateWindowExW(0, L"BUTTON", L"Mat TOP", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 35, 180, 100, 24, hWnd, (HMENU)401, g_hInst, NULL);
        SendMessageW(g_hRadioTop, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
        SendMessageW(g_hRadioTop, BM_SETCHECK, BST_CHECKED, 0);

        g_hRadioBot = CreateWindowExW(0, L"BUTTON", L"Mat BOTTOM", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 145, 180, 120, 24, hWnd, (HMENU)402, g_hInst, NULL);
        SendMessageW(g_hRadioBot, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hStatus = CreateWindowExW(0, L"STATIC", L"Dang nap du lieu...", WS_CHILD | WS_VISIBLE, 275, 183, 1020, 20, hWnd, (HMENU)104, g_hInst, NULL);
        SendMessageW(g_hStatus, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hListView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL, 35, 210, 1290, 390, hWnd, (HMENU)105, g_hInst, NULL);
        SendMessageW(g_hListView, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
        ListView_SetExtendedListViewStyle(g_hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // 13 Cot chuan NeoDen YY1 (Tong 1286px, them 2px vua khit 100% mep phai)
        LVCOLUMNW col = {0};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
        
        col.fmt = LVCFMT_CENTER; col.cx = 45;  col.pszText = L"STT";         ListView_InsertColumn(g_hListView, 0, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 100; col.pszText = L"Designator";  ListView_InsertColumn(g_hListView, 1, &col);
        col.fmt = LVCFMT_LEFT;   col.cx = 200; col.pszText = L"Comment";     ListView_InsertColumn(g_hListView, 2, &col);
        col.fmt = LVCFMT_LEFT;   col.cx = 160; col.pszText = L"Footprint";   ListView_InsertColumn(g_hListView, 3, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 92;  col.pszText = L"Mid X";       ListView_InsertColumn(g_hListView, 4, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 92;  col.pszText = L"Mid Y";       ListView_InsertColumn(g_hListView, 5, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 80;  col.pszText = L"Rotation";   ListView_InsertColumn(g_hListView, 6, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 55;  col.pszText = L"Head";       ListView_InsertColumn(g_hListView, 7, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 80;  col.pszText = L"FeederNo";   ListView_InsertColumn(g_hListView, 8, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 80;  col.pszText = L"Speed%";     ListView_InsertColumn(g_hListView, 9, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 88;  col.pszText = L"Pick(mm)";   ListView_InsertColumn(g_hListView, 10, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 88;  col.pszText = L"Place(mm)";  ListView_InsertColumn(g_hListView, 11, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 60;  col.pszText = L"Mode";       ListView_InsertColumn(g_hListView, 12, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 66;  col.pszText = L"Skip";       ListView_InsertColumn(g_hListView, 13, &col);

        // Khoi 3: Xuat File
        HWND hGrp3 = CreateWindowExW(0, L"BUTTON", L" 3. Luu / Xuat File Sau Khi Chinh Sua ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 20, 625, 1320, 135, hWnd, NULL, g_hInst, NULL);
        SendMessageW(hGrp3, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        HWND hLblTop = CreateWindowExW(0, L"STATIC", L"Ten file TOP:", WS_CHILD | WS_VISIBLE, 35, 652, 90, 20, hWnd, NULL, g_hInst, NULL);
        SendMessageW(hLblTop, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
        g_hEditTop = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Top_Output.csv", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 130, 648, 250, 26, hWnd, NULL, g_hInst, NULL);
        SendMessageW(g_hEditTop, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        HWND hLblBot = CreateWindowExW(0, L"STATIC", L"Ten file BOTTOM:", WS_CHILD | WS_VISIBLE, 420, 652, 120, 20, hWnd, NULL, g_hInst, NULL);
        SendMessageW(hLblBot, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
        g_hEditBot = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Bot_Output.csv", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 550, 648, 250, 26, hWnd, NULL, g_hInst, NULL);
        SendMessageW(g_hEditBot, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        HWND hBtnConvert = CreateWindowExW(0, L"BUTTON", L"LUU FILE DA CHINH SUA CHO MAY NEODEN YY1", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 35, 690, 1290, 48, hWnd, (HMENU)201, g_hInst, NULL);
        SendMessageW(hBtnConvert, WM_SETFONT, (WPARAM)g_hFontTitle, TRUE);

        char auto_path[MAX_PATH] = {0};
        if (auto_detect_altium_file(auto_path, sizeof(auto_path))) {
            wchar_t w_auto[MAX_PATH];
            MultiByteToWideChar(CP_UTF8, 0, auto_path, -1, w_auto, MAX_PATH);
            SetWindowTextW(g_hEditInput, w_auto);
            load_and_display_data(w_auto);
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        if (g_pLogoBitmap) {
            GpGraphics *graphics = NULL;
            if (GdipCreateFromHDC(hdc, &graphics) == 0 && graphics) {
                GdipDrawImageRectI(graphics, g_pLogoBitmap, 25, 10, 56, 56);
                GdipDeleteGraphics(graphics);
            }
        }

        SetBkMode(hdc, TRANSPARENT);
        SelectObject(hdc, g_hFontTitle);
        SetTextColor(hdc, RGB(20, 40, 80));
        TextOutW(hdc, 90, 10, L"NeoDen YY1 SMT Pick & Place File Converter (C Native)", 53);

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, RGB(0, 102, 204));
        TextOutW(hdc, 90, 34, L"Bản quyền & Phát triển: CÔNG TY TNHH CÔNG NGHỆ CHIPXA", 53);

        SelectObject(hdc, g_hFontNormal);
        SetTextColor(hdc, RGB(90, 100, 120));
        TextOutW(hdc, 90, 52, L"Tự động 13 cột • Ma trận Feeder 4 góc (1..13, 14..24, 30..39, 40..50) • Chỉnh sửa & Lưu trực tiếp", 97);

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_NOTIFY: {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->idFrom == 105) {
            if (pnmh->code == NM_CUSTOMDRAW) {
                LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
                switch (lplvcd->nmcd.dwDrawStage) {
                case CDDS_PREPAINT:
                    return CDRF_NOTIFYITEMDRAW;
                case CDDS_ITEMPREPAINT:
                    return CDRF_NOTIFYSUBITEMDRAW;
                case CDDS_ITEMPREPAINT | CDDS_SUBITEM: {
                    int item = (int)lplvcd->nmcd.dwItemSpec;
                    int subItem = lplvcd->iSubItem;
                    if (subItem == 13) {
                        size_t match_idx = 0;
                        for (size_t i = 0; i < g_components.count; ++i) {
                            Component* c = &g_components.items[i];
                            bool is_top = (strcmp(c->layer, "TopLayer") == 0 || strcmp(c->layer, "Top") == 0);
                            if (is_top == g_showing_top) {
                                if ((int)match_idx == item) {
                                    if (c->skip != 0) {
                                        // BẬT: Khối Màu Xanh Lá Cây
                                        lplvcd->clrTextBk = RGB(34, 197, 94);
                                        lplvcd->clrText = RGB(34, 197, 94);
                                    } else {
                                        // TẮT: Khối Màu Đen
                                        lplvcd->clrTextBk = RGB(20, 24, 34);
                                        lplvcd->clrText = RGB(20, 24, 34);
                                    }
                                    break;
                                }
                                match_idx++;
                            }
                        }
                    }
                    return CDRF_DODEFAULT;
                }
                }
                return CDRF_DODEFAULT;
            }
            else if (pnmh->code == NM_CLICK || pnmh->code == NM_DBLCLK) {
                LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)lParam;
                if (pia->iItem >= 0 && (pia->iSubItem == 13 || pnmh->code == NM_DBLCLK)) {
                    size_t match_idx = 0;
                    for (size_t i = 0; i < g_components.count; ++i) {
                        Component* c = &g_components.items[i];
                        bool is_top = (strcmp(c->layer, "TopLayer") == 0 || strcmp(c->layer, "Top") == 0);
                        if (is_top == g_showing_top) {
                            if ((int)match_idx == pia->iItem) {
                                c->skip = (c->skip == 0) ? 1 : 0;
                                refresh_list_view();
                                ListView_SetItemState(g_hListView, pia->iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                                break;
                            }
                            match_idx++;
                        }
                    }
                }
            }
        }
        break;
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case 102: {
            OPENFILENAMEW ofn = {0};
            wchar_t szFile[MAX_PATH] = {0};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
            ofn.lpstrFilter = L"Pick & Place Files (*.csv;*.txt)\0*.csv;*.txt\0CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileNameW(&ofn)) {
                SetWindowTextW(g_hEditInput, ofn.lpstrFile);
                load_and_display_data(ofn.lpstrFile);
            }
            break;
        }
        case 103: {
            wchar_t path[MAX_PATH] = {0};
            GetWindowTextW(g_hEditInput, path, MAX_PATH);
            if (path[0] != L'\0') {
                load_and_display_data(path);
            }
            break;
        }
        case 201: {
            on_save_clicked();
            break;
        }
        case 401: {
            g_showing_top = true;
            refresh_list_view();
            break;
        }
        case 402: {
            g_showing_top = false;
            refresh_list_view();
            break;
        }
        }
        break;
    }

    case WM_DESTROY:
        component_list_free(&g_components);
        if (g_pLogoBitmap) GdipDisposeImage(g_pLogoBitmap);
        if (g_hFontTitle) DeleteObject(g_hFontTitle);
        if (g_hFontBold) DeleteObject(g_hFontBold);
        if (g_hFontNormal) DeleteObject(g_hFontNormal);
        if (g_hFontSplashTitle) DeleteObject(g_hFontSplashTitle);
        if (g_hFontSplashSub) DeleteObject(g_hFontSplashSub);
        GdiplusShutdown(g_gdiToken);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;
    g_hInst = hInstance;

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    GdiplusStartupInput gdiInput = { 1, NULL, FALSE, FALSE };
    GdiplusStartup(&g_gdiToken, &gdiInput, NULL);

    const wchar_t* logoPaths[] = { L"assets/logo.png", L"../assets/logo.png", L"c/assets/logo.png", L"logo.png" };
    for (int i = 0; i < 4; ++i) {
        if (GetFileAttributesW(logoPaths[i]) != INVALID_FILE_ATTRIBUTES) {
            GdipCreateBitmapFromFile(logoPaths[i], &g_pLogoBitmap);
            break;
        }
    }

    g_hFontSplashTitle = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g_hFontSplashSub = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    // Splash Class
    WNDCLASSEXW splashClass = {0};
    splashClass.cbSize = sizeof(WNDCLASSEXW);
    splashClass.style = CS_HREDRAW | CS_VREDRAW;
    splashClass.lpfnWndProc = SplashProc;
    splashClass.hInstance = hInstance;
    splashClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    splashClass.lpszClassName = L"NeoDenYY1CSplash";
    RegisterClassExW(&splashClass);

    // Main Class
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcex.lpszClassName = L"NeoDenYY1CGUI";

    HICON hIconBig = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(1), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    if (!hIconBig) hIconBig = (HICON)LoadImageW(NULL, L"assets/app_icon.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    if (!hIconBig) hIconBig = (HICON)LoadImageW(NULL, L"app_icon.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);

    HICON hIconSm = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    if (!hIconSm) hIconSm = (HICON)LoadImageW(NULL, L"assets/app_icon.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    if (!hIconSm) hIconSm = (HICON)LoadImageW(NULL, L"app_icon.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);

    wcex.hIcon = hIconBig;
    wcex.hIconSm = hIconSm;
    RegisterClassExW(&wcex);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    int winW = 1380, winH = 820;
    int winX = (screenW - winW) / 2, winY = (screenH - winH) / 2;
    g_hWnd = CreateWindowW(L"NeoDenYY1CGUI", L"NeoDen YY1 SMT Pick & Place File Converter (C Native) - ChipXa",
                           WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
                           winX, winY, winW, winH, NULL, NULL, hInstance, NULL);

    if (g_hWnd) {
        SendMessageW(g_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
        SendMessageW(g_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);
    }

    int splashW = 500, splashH = 290;
    int splashX = (screenW - splashW) / 2, splashY = (screenH - splashH) / 2;
    g_hSplash = CreateWindowExW(WS_EX_TOPMOST, L"NeoDenYY1CSplash", L"",
                                WS_POPUP | WS_VISIBLE, splashX, splashY, splashW, splashH,
                                NULL, NULL, hInstance, NULL);

    ShowWindow(g_hSplash, SW_SHOWNORMAL);
    UpdateWindow(g_hSplash);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
