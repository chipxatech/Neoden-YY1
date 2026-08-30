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
HWND g_hChkAutoMatch_c = NULL;
bool g_auto_match_feeder_c = true;
HWND g_hEditBoardWidth_c = NULL;
double g_board_width_c = 0.0;
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

static void recalc_bottom_coordinates_c(void) {
    g_board_width_c = 0.0;
    if (g_hEditBoardWidth_c) {
        wchar_t buf[64] = {0};
        GetWindowTextW(g_hEditBoardWidth_c, buf, 64);
        if (buf[0] != L'\0') {
            g_board_width_c = _wtof(buf);
        }
    }
    for (size_t i = 0; i < g_components.count; ++i) {
        Component* c = &g_components.items[i];
        bool is_bot = (strstr(c->layer, "Bottom") || strcmp(c->layer, "BottomLayer") == 0 || strstr(c->layer, "bot"));
        if (is_bot) {
            if (g_board_width_c > 0.0) {
                c->mid_x = g_board_width_c - c->raw_mid_x;
            } else {
                c->mid_x = c->raw_mid_x;
            }
            c->mid_y = c->raw_mid_y;
        }
    }
}

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
        ListView_SetItemText(g_hListView, (int)shown_count, 13, (LPWSTR)(c->skip ? L"1" : L"0"));

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

    if (g_auto_match_feeder_c) {
        for (size_t i = 0; i < g_components.count; ++i) {
            g_components.items[i].feeder_no = match_feeder_slot_c(g_components.items[i].comment, g_components.items[i].footprint);
        }
    } else {
        for (size_t i = 0; i < g_components.count; ++i) {
            g_components.items[i].feeder_no = g_components.items[i].raw_feeder_no;
        }
    }

    recalc_bottom_coordinates_c();
    refresh_list_view();
}

static void on_save_clicked(void) {
    if (g_components.count == 0) {
        MessageBoxW(g_hWnd, L"Chưa có dữ liệu để lưu!", L"Thông Báo", MB_ICONWARNING);
        return;
    }

    recalc_bottom_coordinates_c();

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

// In-Place Cell Editing trực tiếp trên từng ô
static WNDPROC g_oldEditProc = NULL;
static int g_inPlaceItem = -1;
static int g_inPlaceSubItem = -1;
static HWND g_hInPlaceEdit = NULL;

static Component* get_component_at_displayed_index(int item_idx) {
    size_t match_idx = 0;
    for (size_t i = 0; i < g_components.count; ++i) {
        Component* c = &g_components.items[i];
        bool is_bot = (strstr(c->layer, "Bottom") || strcmp(c->layer, "BottomLayer") == 0 || strstr(c->layer, "bot"));
        if (is_bot != g_showing_top) {
            if ((int)match_idx == item_idx) {
                return c;
            }
            match_idx++;
        }
    }
    return NULL;
}

static void commit_in_place_edit(bool save) {
    if (!g_hInPlaceEdit) return;
    HWND hEdit = g_hInPlaceEdit;
    g_hInPlaceEdit = NULL;
    int item = g_inPlaceItem;
    int subItem = g_inPlaceSubItem;
    g_inPlaceItem = -1;
    g_inPlaceSubItem = -1;

    if (save) {
        wchar_t buf[256] = {0};
        GetWindowTextW(hEdit, buf, 256);
        Component* c = get_component_at_displayed_index(item);
        if (c) {
            switch (subItem) {
                case 1: WideCharToMultiByte(CP_UTF8, 0, buf, -1, c->designator, sizeof(c->designator), NULL, NULL); break;
                case 2: WideCharToMultiByte(CP_UTF8, 0, buf, -1, c->comment, sizeof(c->comment), NULL, NULL); break;
                case 3: WideCharToMultiByte(CP_UTF8, 0, buf, -1, c->footprint, sizeof(c->footprint), NULL, NULL); break;
                case 4: c->mid_x = _wtof(buf); break;
                case 5: c->mid_y = _wtof(buf); break;
                case 6: c->rotation = _wtof(buf); break;
                case 7: c->head = _wtoi(buf); break;
                case 8: c->feeder_no = _wtoi(buf); break;
                case 9: c->mount_speed = _wtoi(buf); break;
                case 10: c->pick_height = _wtof(buf); break;
                case 11: c->place_height = _wtof(buf); break;
                case 12: c->mode = _wtoi(buf); break;
            }
        }
        refresh_list_view();
    }
    DestroyWindow(hEdit);
}

static LRESULT CALLBACK InPlaceEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_KILLFOCUS:
        commit_in_place_edit(true);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_RETURN) {
            commit_in_place_edit(true);
            return 0;
        } else if (wParam == VK_ESCAPE) {
            commit_in_place_edit(false);
            return 0;
        }
        break;
    case WM_CHAR:
        if (wParam == VK_RETURN || wParam == VK_ESCAPE) return 0;
        break;
    }
    return CallWindowProc(g_oldEditProc, hWnd, message, wParam, lParam);
}

typedef struct {
    char comment[64];
    char footprint[64];
    int head;
    int speed;
} FeederSlotC;

static FeederSlotC g_feeder_matrix_c[51];
static wchar_t g_active_profile_c[64] = L"Mac_Dinh";
static wchar_t g_inputDlgBufferC[128] = {0};
static const wchar_t* g_inputDlgTitleC = L"";
static const wchar_t* g_inputDlgPromptC = L"";

static void init_default_feeder_matrix_c(void) {
    memset(g_feeder_matrix_c, 0, sizeof(g_feeder_matrix_c));
    for (int i = 1; i <= 50; ++i) {
        strcpy(g_feeder_matrix_c[i].footprint, "0603");
        g_feeder_matrix_c[i].speed = 100;
    }
    // Bottom-Left 1..13 theo ảnh mẫu
    strcpy(g_feeder_matrix_c[1].comment, "1K-0603");
    strcpy(g_feeder_matrix_c[2].comment, "100uF-0603");
    strcpy(g_feeder_matrix_c[3].comment, "10K-0805"); strcpy(g_feeder_matrix_c[3].footprint, "0805");
    strcpy(g_feeder_matrix_c[4].comment, "100uF-0805"); strcpy(g_feeder_matrix_c[4].footprint, "0805");
    strcpy(g_feeder_matrix_c[5].comment, "1K-0805"); strcpy(g_feeder_matrix_c[5].footprint, "0805");
    strcpy(g_feeder_matrix_c[6].comment, "10K-0603");
    strcpy(g_feeder_matrix_c[7].comment, "10uF-0805"); strcpy(g_feeder_matrix_c[7].footprint, "0805");
    strcpy(g_feeder_matrix_c[8].comment, "2SC1805"); strcpy(g_feeder_matrix_c[8].footprint, "SOT-23");
    strcpy(g_feeder_matrix_c[9].comment, "4.7K-0805"); strcpy(g_feeder_matrix_c[9].footprint, "0805");

    // Bottom-Right 30..39 theo ảnh mẫu
    strcpy(g_feeder_matrix_c[30].comment, "Red-0805"); strcpy(g_feeder_matrix_c[30].footprint, "0805");
    strcpy(g_feeder_matrix_c[31].comment, "Green-0805"); strcpy(g_feeder_matrix_c[31].footprint, "0805");
    strcpy(g_feeder_matrix_c[32].comment, "Yellow-0805"); strcpy(g_feeder_matrix_c[32].footprint, "0805");
    strcpy(g_feeder_matrix_c[33].comment, "Blue-0805"); strcpy(g_feeder_matrix_c[33].footprint, "0805");
    strcpy(g_feeder_matrix_c[34].comment, "Red-0603");
    strcpy(g_feeder_matrix_c[35].comment, "Blue-0603");
}

static void canonicalize_feeder_keywords_c(const char* src, char* dst, size_t dst_len) {
    if (!src || !dst || dst_len == 0) return;
    dst[0] = '\0';

    char temp[256] = {0};
    size_t ti = 0;
    for (size_t i = 0; src[i] && ti < sizeof(temp) - 1; ++i) {
        unsigned char c = (unsigned char)src[i];
        if (c >= 'A' && c <= 'Z') c = (unsigned char)tolower(c);
        temp[ti++] = (char)c;
    }
    temp[ti] = '\0';

    char step1[256] = {0};
    size_t s1 = 0;
    for (size_t i = 0; temp[i] && s1 < sizeof(step1) - 8; ) {
        if (strncmp(&temp[i], "xanh la", 7) == 0 || strncmp(&temp[i], "xanh cay", 8) == 0) {
            strcpy(&step1[s1], "green"); s1 += 5; i += (temp[i+5] == 'l' ? 7 : 8);
        } else if (strncmp(&temp[i], "xanh duong", 10) == 0 || strncmp(&temp[i], "xanh bien", 9) == 0) {
            strcpy(&step1[s1], "blue"); s1 += 4; i += (temp[i+5] == 'd' ? 10 : 9);
        } else if (strncmp(&temp[i], "xanh", 4) == 0) {
            strcpy(&step1[s1], "green"); s1 += 5; i += 4;
        } else if (strncmp(&temp[i], "vang", 4) == 0) {
            strcpy(&step1[s1], "yellow"); s1 += 6; i += 4;
        } else if (strncmp(&temp[i], "trang", 5) == 0) {
            strcpy(&step1[s1], "white"); s1 += 5; i += 5;
        } else if (strncmp(&temp[i], "do", 2) == 0) {
            bool b_ok = (i == 0 || !isalnum((unsigned char)temp[i-1]));
            bool a_ok = (temp[i+2] == '\0' || !isalnum((unsigned char)temp[i+2]));
            if (b_ok && a_ok) {
                strcpy(&step1[s1], "red"); s1 += 3; i += 2;
            } else {
                step1[s1++] = temp[i++];
            }
        } else {
            step1[s1++] = temp[i++];
        }
    }
    step1[s1] = '\0';
    strncpy(dst, step1, dst_len - 1);
    dst[dst_len - 1] = '\0';
}

typedef struct {
    char items[16][32];
    int count;
} TokenListC;

static void extract_tokens_c(const char* str, TokenListC* out) {
    out->count = 0;
    if (!str) return;
    char can[128] = {0};
    canonicalize_feeder_keywords_c(str, can, sizeof(can));

    char cur[32] = {0};
    int ci = 0;
    for (int i = 0; can[i]; ++i) {
        char c = can[i];
        if (isalnum((unsigned char)c) || c == '.') {
            if (ci < (int)sizeof(cur) - 1) cur[ci++] = c;
        } else {
            if (ci > 0 && out->count < 16) {
                cur[ci] = '\0';
                strcpy(out->items[out->count++], cur);
                ci = 0;
            }
        }
    }
    if (ci > 0 && out->count < 16) {
        cur[ci] = '\0';
        strcpy(out->items[out->count++], cur);
    }

    int orig_count = out->count;
    for (int i = 0; i < orig_count && out->count < 16; ++i) {
        const char* t = out->items[i];
        if (strncmp(t, "0402", 4) == 0 && strcmp(t, "0402") != 0) strcpy(out->items[out->count++], "0402");
        else if (strncmp(t, "0603", 4) == 0 && strcmp(t, "0603") != 0) strcpy(out->items[out->count++], "0603");
        else if (strncmp(t, "0805", 4) == 0 && strcmp(t, "0805") != 0) strcpy(out->items[out->count++], "0805");
        else if (strncmp(t, "1206", 4) == 0 && strcmp(t, "1206") != 0) strcpy(out->items[out->count++], "1206");
        else if (strncmp(t, "1210", 4) == 0 && strcmp(t, "1210") != 0) strcpy(out->items[out->count++], "1210");
        else if (strstr(t, "sot23") && out->count < 15) { strcpy(out->items[out->count++], "sot23"); strcpy(out->items[out->count++], "sot-23"); }
        else if (strstr(t, "sod323") && out->count < 15) { strcpy(out->items[out->count++], "sod323"); strcpy(out->items[out->count++], "sod-323"); }
        else if (strstr(t, "sod123") && out->count < 15) { strcpy(out->items[out->count++], "sod123"); strcpy(out->items[out->count++], "sod-123"); }
        else if ((strstr(t, "sop16") || strstr(t, "soic16")) && out->count < 15) { strcpy(out->items[out->count++], "sop16"); strcpy(out->items[out->count++], "sop-16"); }
    }
}

static int match_feeder_slot_c(const char* cmt, const char* fp) {
    if (!cmt && !fp) return 0;
    TokenListC comp_tokens;
    extract_tokens_c(cmt, &comp_tokens);
    TokenListC fp_tokens;
    extract_tokens_c(fp, &fp_tokens);
    for (int i = 0; i < fp_tokens.count && comp_tokens.count < 16; ++i) {
        strcpy(comp_tokens.items[comp_tokens.count++], fp_tokens.items[i]);
    }
    if (comp_tokens.count == 0) return 0;

    int best_slot = 0;
    int max_matched = 0;

    for (int slot = 1; slot <= 50; ++slot) {
        if (!g_feeder_matrix_c[slot].comment[0]) continue;
        TokenListC f_tokens;
        extract_tokens_c(g_feeder_matrix_c[slot].comment, &f_tokens);
        if (g_feeder_matrix_c[slot].footprint[0]) {
            TokenListC ft;
            extract_tokens_c(g_feeder_matrix_c[slot].footprint, &ft);
            for (int i = 0; i < ft.count && f_tokens.count < 16; ++i) {
                strcpy(f_tokens.items[f_tokens.count++], ft.items[i]);
            }
        }
        if (f_tokens.count == 0) continue;

        int all_matched = 1;
        int match_count = 0;

        for (int i = 0; i < f_tokens.count; ++i) {
            const char* ft = f_tokens.items[i];
            if (strcmp(ft, "smd") == 0 || strcmp(ft, "chip") == 0) continue;

            int found = 0;
            for (int j = 0; j < comp_tokens.count; ++j) {
                if (strcmp(comp_tokens.items[j], ft) == 0) {
                    found = 1;
                    break;
                }
            }
            if (found) {
                match_count++;
            } else {
                all_matched = 0;
                break;
            }
        }

        if (all_matched && match_count > max_matched) {
            max_matched = match_count;
            best_slot = slot;
        }
    }

    return best_slot;
}

static LRESULT CALLBACK InputDlgProcC(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        if (wmId == IDOK) {
            GetDlgItemTextW(hWnd, 101, g_inputDlgBufferC, 128);
            DestroyWindow(hWnd);
        } else if (wmId == IDCANCEL) {
            g_inputDlgBufferC[0] = 0;
            DestroyWindow(hWnd);
        }
        break;
    }
    case WM_CLOSE:
        g_inputDlgBufferC[0] = 0;
        DestroyWindow(hWnd);
        break;
    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

static bool show_input_box_c(HWND parent, const wchar_t* title, const wchar_t* prompt, wchar_t* outStr, int maxLen) {
    g_inputDlgTitleC = title;
    g_inputDlgPromptC = prompt;
    g_inputDlgBufferC[0] = 0;
    RECT pr;
    GetWindowRect(parent, &pr);
    int dlgW = 360, dlgH = 150;
    int dlgX = pr.left + (pr.right - pr.left - dlgW) / 2;
    int dlgY = pr.top + (pr.bottom - pr.top - dlgH) / 2;

    HWND hDlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"#32770", title, WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, dlgX, dlgY, dlgW, dlgH, parent, NULL, g_hInst, NULL);
    if (!hDlg) return false;
    SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LONG_PTR)InputDlgProcC);

    HWND hLbl = CreateWindowExW(0, L"STATIC", prompt, WS_CHILD | WS_VISIBLE, 15, 12, 315, 20, hDlg, NULL, g_hInst, NULL);
    SendMessageW(hLbl, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
    HWND hEd = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 15, 36, 315, 24, hDlg, (HMENU)101, g_hInst, NULL);
    SendMessageW(hEd, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
    SetFocus(hEd);
    HWND hOk = CreateWindowExW(0, L"BUTTON", L"Đồng Ý", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 155, 72, 80, 28, hDlg, (HMENU)IDOK, g_hInst, NULL);
    SendMessageW(hOk, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
    HWND hCancel = CreateWindowExW(0, L"BUTTON", L"Hủy", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 72, 80, 28, hDlg, (HMENU)IDCANCEL, g_hInst, NULL);
    SendMessageW(hCancel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

    EnableWindow(parent, FALSE);
    MSG msg;
    while (IsWindow(hDlg) && GetMessageW(&msg, NULL, 0, 0)) {
        if (!IsDialogMessageW(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    EnableWindow(parent, TRUE);
    SetForegroundWindow(parent);

    if (g_inputDlgBufferC[0] != 0) {
        wcsncpy(outStr, g_inputDlgBufferC, maxLen - 1);
        outStr[maxLen - 1] = L'\0';
        return true;
    }
    return false;
}

static void get_profiles_directory_c(wchar_t* outDir, int maxLen) {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(exePath, L'\\');
    if (!lastSlash) lastSlash = wcsrchr(exePath, L'/');
    if (lastSlash) *lastSlash = L'\0';
    else wcscpy(exePath, L".");

    wchar_t p1[MAX_PATH], p2[MAX_PATH];
    swprintf(p1, MAX_PATH, L"%ls\\feeder_profiles", exePath);
    swprintf(p2, MAX_PATH, L"%ls\\..\\feeder_profiles", exePath);

    DWORD dw1 = GetFileAttributesW(p1);
    if (dw1 != INVALID_FILE_ATTRIBUTES && (dw1 & FILE_ATTRIBUTE_DIRECTORY)) {
        wcsncpy(outDir, p1, maxLen - 1);
        outDir[maxLen - 1] = L'\0';
        return;
    }
    DWORD dw2 = GetFileAttributesW(p2);
    if (dw2 != INVALID_FILE_ATTRIBUTES && (dw2 & FILE_ATTRIBUTE_DIRECTORY)) {
        wcsncpy(outDir, p2, maxLen - 1);
        outDir[maxLen - 1] = L'\0';
        return;
    }
    CreateDirectoryW(p1, NULL);
    wcsncpy(outDir, p1, maxLen - 1);
    outDir[maxLen - 1] = L'\0';
}

static void save_profile_to_disk_c(const wchar_t* profName) {
    wchar_t pDir[MAX_PATH];
    get_profiles_directory_c(pDir, MAX_PATH);
    wchar_t path[MAX_PATH];
    swprintf(path, MAX_PATH, L"%ls\\%ls.json", pDir, profName);
    FILE* fp = _wfopen(path, L"w, ccs=UTF-8");
    if (fp) {
        char pNameA[128] = {0};
        WideCharToMultiByte(CP_UTF8, 0, profName, -1, pNameA, 128, NULL, NULL);
        fwprintf(fp, L"{\n  \"profile_name\": \"%hs\",\n  \"feeders\": {\n", pNameA);
        for (int i = 1; i <= 50; ++i) {
            wchar_t wVal[128] = {0};
            MultiByteToWideChar(CP_UTF8, 0, g_feeder_matrix_c[i].comment, -1, wVal, 128);
            fwprintf(fp, L"    \"%d\": \"%ls\"%ls\n", i, wVal, (i < 50) ? L"," : L"");
        }
        fwprintf(fp, L"  }\n}\n");
        fclose(fp);
    }

    FILE* fpM = fopen("feeder_matrix.json", "w");
    if (fpM) {
        char pNameA[128] = {0};
        WideCharToMultiByte(CP_UTF8, 0, profName, -1, pNameA, 128, NULL, NULL);
        fprintf(fpM, "{\n  \"profile_name\": \"%s\",\n  \"feeders\": {\n", pNameA);
        for (int i = 1; i <= 50; ++i) {
            fprintf(fpM, "    \"%d\": \"%s\"%s\n", i, g_feeder_matrix_c[i].comment, (i < 50) ? "," : "");
        }
        fprintf(fpM, "  }\n}\n");
        fclose(fpM);
    }
}

static void load_profile_from_disk_c(const wchar_t* profName) {
    for (int i = 1; i <= 50; ++i) g_feeder_matrix_c[i].comment[0] = '\0';
    wchar_t pDir[MAX_PATH];
    get_profiles_directory_c(pDir, MAX_PATH);
    wchar_t path[MAX_PATH];
    swprintf(path, MAX_PATH, L"%ls\\%ls.json", pDir, profName);
    FILE* fp = _wfopen(path, L"r, ccs=UTF-8");
    if (!fp) return;

    wchar_t line[256];
    while (fgetws(line, 256, fp)) {
        wchar_t* q1 = wcschr(line, L'"');
        if (!q1) continue;
        wchar_t* q2 = wcschr(q1 + 1, L'"');
        if (!q2) continue;
        *q2 = L'\0';
        int slot = _wtoi(q1 + 1);
        if (slot >= 1 && slot <= 50) {
            wchar_t* col = wcschr(q2 + 1, L':');
            if (col) {
                wchar_t* vq1 = wcschr(col, L'"');
                if (vq1) {
                    wchar_t* vq2 = wcschr(vq1 + 1, L'"');
                    if (vq2) {
                        *vq2 = L'\0';
                        WideCharToMultiByte(CP_UTF8, 0, vq1 + 1, -1, g_feeder_matrix_c[slot].comment, sizeof(g_feeder_matrix_c[slot].comment), NULL, NULL);
                    }
                }
            }
        }
    }
    fclose(fp);
}

static void populate_profile_combobox_c(HWND hCb) {
    SendMessageW(hCb, CB_RESETCONTENT, 0, 0);
    wchar_t pDir[MAX_PATH];
    get_profiles_directory_c(pDir, MAX_PATH);

    // Đảm bảo Mac_Dinh.json luôn tồn tại
    wchar_t macDinhPath[MAX_PATH];
    swprintf(macDinhPath, MAX_PATH, L"%ls\\Mac_Dinh.json", pDir);
    DWORD dw = GetFileAttributesW(macDinhPath);
    if (dw == INVALID_FILE_ATTRIBUTES) {
        save_profile_to_disk_c(L"Mac_Dinh");
    }

    SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)L"Mac_Dinh");

    WIN32_FIND_DATAW ffd;
    wchar_t searchPattern[MAX_PATH];
    swprintf(searchPattern, MAX_PATH, L"%ls\\*.json", pDir);
    HANDLE hFind = FindFirstFileW(searchPattern, &ffd);
    int selIdx = 0, count = 1;
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                wchar_t* ext = wcsrchr(ffd.cFileName, L'.');
                if (ext && _wcsicmp(ext, L".json") == 0) {
                    *ext = L'\0';
                    if (_wcsicmp(ffd.cFileName, L"Mac_Dinh") != 0) {
                        SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)ffd.cFileName);
                        if (wcscmp(ffd.cFileName, g_active_profile_c) == 0) selIdx = count;
                        count++;
                    }
                }
            }
        } while (FindNextFileW(hFind, &ffd) != 0);
        FindClose(hFind);
    }
    if (wcscmp(g_active_profile_c, L"Mac_Dinh") == 0) selIdx = 0;
    SendMessageW(hCb, CB_SETCURSEL, selIdx, 0);
}

static HBRUSH g_hBrushDarkDlg_c = NULL;
static HBRUSH g_hBrushEditDark_c = NULL;
static HWND g_hLblActiveProfile_c = NULL;

static wchar_t g_dialog_profile_c[64] = L"Mac_Dinh";

static void refresh_active_profile_label_c(void) {
    if (g_hLblActiveProfile_c) {
        wchar_t buf[128];
        swprintf(buf, 128, L"⚙️ Quy tắc đang áp dụng: [%ls]", g_active_profile_c[0] ? g_active_profile_c : L"Mac_Dinh");
        SetWindowTextW(g_hLblActiveProfile_c, buf);
        InvalidateRect(g_hLblActiveProfile_c, NULL, TRUE);
        UpdateWindow(g_hLblActiveProfile_c);
    }
}

static void create_feeder_slot_control_c(HWND hParent, int slot, int x, int y, const char* val) {
    wchar_t lblText[32];
    swprintf(lblText, 32, L"#%02d:", slot);
    HWND hLbl = CreateWindowExW(0, L"STATIC", lblText, WS_CHILD | WS_VISIBLE | SS_RIGHT, x, y + 2, 45, 18, hParent, NULL, g_hInst, NULL);
    SendMessageW(hLbl, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    wchar_t w_val[128];
    MultiByteToWideChar(CP_UTF8, 0, val, -1, w_val, 128);
    HWND hEd = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", w_val, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, x + 50, y, 220, 21, hParent, (HMENU)(INT_PTR)(5000 + slot), g_hInst, NULL);
    SendMessageW(hEd, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
}

static LRESULT CALLBACK FeederDlgProcC(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CTLCOLORDLG:
        return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, TRANSPARENT);
        HWND hCtl = (HWND)lParam;
        int id = GetDlgCtrlID(hCtl);
        if (id >= 5001 && id <= 5050) {
            SetTextColor(hdc, RGB(30, 41, 59)); // #1E293B (chữ tối rõ nét trên nền xám)
        } else {
            SetTextColor(hdc, RGB(2, 132, 199)); // #0284C7
        }
        return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
    }

    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(15, 23, 42)); // Chữ đen đậm
        SetBkColor(hdc, RGB(255, 255, 255)); // Nền trắng sáng
        return (INT_PTR)GetStockObject(WHITE_BRUSH);
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        if (wmId == 3001 && wmEvent == CBN_SELCHANGE) {
            HWND hCb = GetDlgItem(hWnd, 3001);
            int idx = (int)SendMessageW(hCb, CB_GETCURSEL, 0, 0);
            if (idx != CB_ERR) {
                SendMessageW(hCb, CB_GETLBTEXT, idx, (LPARAM)g_dialog_profile_c);
                load_profile_from_disk_c(g_dialog_profile_c);
                for (int slot = 1; slot <= 50; ++slot) {
                    HWND hEd = GetDlgItem(hWnd, 5000 + slot);
                    if (hEd) {
                        wchar_t wVal[128] = {0};
                        MultiByteToWideChar(CP_UTF8, 0, g_feeder_matrix_c[slot].comment, -1, wVal, 128);
                        SetWindowTextW(hEd, wVal);
                    }
                }
            }
        } else if (wmId == 3002) { // + Tạo Mới
            wchar_t newName[64] = {0};
            if (show_input_box_c(hWnd, L"Tạo Cấu Hình Mới", L"Nhập tên cấu hình mới (VD: Bo_Mach_A):", newName, 64)) {
                for (int slot = 1; slot <= 50; ++slot) {
                    HWND hEd = GetDlgItem(hWnd, 5000 + slot);
                    if (hEd) {
                        wchar_t buf[128] = {0};
                        GetWindowTextW(hEd, buf, 128);
                        WideCharToMultiByte(CP_UTF8, 0, buf, -1, g_feeder_matrix_c[slot].comment, sizeof(g_feeder_matrix_c[slot].comment), NULL, NULL);
                    }
                }
                wcscpy(g_dialog_profile_c, newName);
                save_profile_to_disk_c(g_dialog_profile_c);
                HWND hCb = GetDlgItem(hWnd, 3001);
                populate_profile_combobox_c(hCb);
                MessageBoxW(hWnd, L"🎉 Đã tạo cấu hình mới thành công! Nhấn 'LƯU VÀ ÁP DỤNG NGAY' nếu muốn áp dụng cho mạch.", L"Thành Công", MB_ICONINFORMATION);
            }
        } else if (wmId == 3003) { // Lưu
            for (int slot = 1; slot <= 50; ++slot) {
                HWND hEd = GetDlgItem(hWnd, 5000 + slot);
                if (hEd) {
                    wchar_t buf[128] = {0};
                    GetWindowTextW(hEd, buf, 128);
                    WideCharToMultiByte(CP_UTF8, 0, buf, -1, g_feeder_matrix_c[slot].comment, sizeof(g_feeder_matrix_c[slot].comment), NULL, NULL);
                }
            }
            save_profile_to_disk_c(g_dialog_profile_c);
            MessageBoxW(hWnd, L"💾 Đã lưu cấu hình hiện tại thành công!", L"Thành Công", MB_ICONINFORMATION);
        } else if (wmId == 3004) { // Lưu Thành...
            wchar_t newName[64] = {0};
            if (show_input_box_c(hWnd, L"Lưu Thành Cấu Hình Khác", L"Nhập tên cấu hình mới:", newName, 64)) {
                for (int slot = 1; slot <= 50; ++slot) {
                    HWND hEd = GetDlgItem(hWnd, 5000 + slot);
                    if (hEd) {
                        wchar_t buf[128] = {0};
                        GetWindowTextW(hEd, buf, 128);
                        WideCharToMultiByte(CP_UTF8, 0, buf, -1, g_feeder_matrix_c[slot].comment, sizeof(g_feeder_matrix_c[slot].comment), NULL, NULL);
                    }
                }
                wcscpy(g_dialog_profile_c, newName);
                save_profile_to_disk_c(g_dialog_profile_c);
                HWND hCb = GetDlgItem(hWnd, 3001);
                populate_profile_combobox_c(hCb);
                MessageBoxW(hWnd, L"🎉 Đã lưu thành cấu hình mới!", L"Thành Công", MB_ICONINFORMATION);
            }
        } else if (wmId == 3005) { // Xóa
            if (wcscmp(g_dialog_profile_c, L"Mac_Dinh") == 0 || wcscmp(g_dialog_profile_c, L"Mặc Định") == 0 || g_dialog_profile_c[0] == L'\0') {
                MessageBoxW(hWnd, L"Cấu hình mặc định [Mac_Dinh] là cấu hình gốc của máy và không thể xóa!", L"Thông Báo", MB_ICONWARNING);
                break;
            }
            if (MessageBoxW(hWnd, L"Bạn có chắc muốn xóa vĩnh viễn cấu hình này?", L"Xác Nhận Xóa", MB_ICONQUESTION | MB_YESNO) == IDYES) {
                wchar_t pDir[MAX_PATH];
                get_profiles_directory_c(pDir, MAX_PATH);
                wchar_t path[MAX_PATH];
                swprintf(path, MAX_PATH, L"%ls\\%ls.json", pDir, g_dialog_profile_c);
                DeleteFileW(path);
                wcscpy(g_dialog_profile_c, L"Mac_Dinh");
                HWND hCb = GetDlgItem(hWnd, 3001);
                populate_profile_combobox_c(hCb);
                load_profile_from_disk_c(g_dialog_profile_c);
                for (int slot = 1; slot <= 50; ++slot) {
                    HWND hEd = GetDlgItem(hWnd, 5000 + slot);
                    if (hEd) {
                        wchar_t wVal[128] = {0};
                        MultiByteToWideChar(CP_UTF8, 0, g_feeder_matrix_c[slot].comment, -1, wVal, 128);
                        SetWindowTextW(hEd, wVal);
                    }
                }
                MessageBoxW(hWnd, L"Đã xóa cấu hình! Đã chuyển về xem [Mac_Dinh].", L"Thành Công", MB_ICONINFORMATION);
            }
        } else if (wmId == IDOK || wmId == 2001) { // LƯU & ÁP DỤNG
            wcscpy(g_active_profile_c, g_dialog_profile_c);
            for (int slot = 1; slot <= 50; ++slot) {
                HWND hEd = GetDlgItem(hWnd, 5000 + slot);
                if (hEd) {
                    wchar_t buf[128] = {0};
                    GetWindowTextW(hEd, buf, 128);
                    WideCharToMultiByte(CP_UTF8, 0, buf, -1, g_feeder_matrix_c[slot].comment, sizeof(g_feeder_matrix_c[slot].comment), NULL, NULL);
                }
            }
            save_profile_to_disk_c(g_active_profile_c);

            if (g_auto_match_feeder_c) {
                for (size_t i = 0; i < g_components.count; ++i) {
                    g_components.items[i].feeder_no = match_feeder_slot_c(g_components.items[i].comment, g_components.items[i].footprint);
                }
            }
            refresh_list_view();
            refresh_active_profile_label_c();
            MessageBoxW(hWnd, L"🎉 Đã áp dụng cấu hình và cập nhật số khay Feeder trên bảng mạch!", L"Thành Công", MB_ICONINFORMATION);
            DestroyWindow(hWnd);
        } else if (wmId == IDCANCEL) {
            DestroyWindow(hWnd);
        }
        break;
    }
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

static void open_feeder_matrix_dialog_c(HWND parent) {
    wcscpy(g_dialog_profile_c, g_active_profile_c[0] ? g_active_profile_c : L"Mac_Dinh");

    RECT pr;
    GetWindowRect(parent, &pr);
    int dlgW = 675, dlgH = 765;
    int dlgX = pr.left + (pr.right - pr.left - dlgW) / 2;
    int dlgY = pr.top + (pr.bottom - pr.top - dlgH) / 2;

    HWND hDlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"#32770", L"Cấu Hình 50 Khay Feeder 4 Góc (NeoDen YY1)",
                               WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
                               dlgX, dlgY, dlgW, dlgH, parent, NULL, g_hInst, NULL);
    if (!hDlg) return;

    SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LONG_PTR)FeederDlgProcC);

    // Profile Bar ở trên
    HWND hLblProf = CreateWindowExW(0, L"STATIC", L"Cấu Hình:", WS_CHILD | WS_VISIBLE | SS_RIGHT, 15, 14, 85, 20, hDlg, NULL, g_hInst, NULL);
    SendMessageW(hLblProf, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hCbProf = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 105, 11, 165, 350, hDlg, (HMENU)3001, g_hInst, NULL);
    SendMessageW(hCbProf, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
    populate_profile_combobox_c(hCbProf);

    HWND hBtnNew = CreateWindowExW(0, L"BUTTON", L"+ Tạo Mới", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 280, 10, 85, 26, hDlg, (HMENU)3002, g_hInst, NULL);
    SendMessageW(hBtnNew, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hBtnSaveP = CreateWindowExW(0, L"BUTTON", L"Lưu", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 370, 10, 65, 26, hDlg, (HMENU)3003, g_hInst, NULL);
    SendMessageW(hBtnSaveP, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hBtnSaveAs = CreateWindowExW(0, L"BUTTON", L"Lưu Thành...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 440, 10, 105, 26, hDlg, (HMENU)3004, g_hInst, NULL);
    SendMessageW(hBtnSaveAs, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

    HWND hBtnDelP = CreateWindowExW(0, L"BUTTON", L"Xóa", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 550, 10, 65, 26, hDlg, (HMENU)3005, g_hInst, NULL);
    SendMessageW(hBtnDelP, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

    // 4 Khung GroupBox 4 Góc
    HWND hGrp1 = CreateWindowExW(0, L"BUTTON", L" [1] Góc Trên Trái (Khay 14 -> 24) ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 15, 45, 305, 280, hDlg, NULL, g_hInst, NULL);
    SendMessageW(hGrp1, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hGrp2 = CreateWindowExW(0, L"BUTTON", L" [2] Góc Trên Phải (Khay 40 -> 50) ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 335, 45, 305, 280, hDlg, NULL, g_hInst, NULL);
    SendMessageW(hGrp2, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hGrp3 = CreateWindowExW(0, L"BUTTON", L" [3] Góc Dưới Trái (Khay 1 -> 13) ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 15, 330, 305, 330, hDlg, NULL, g_hInst, NULL);
    SendMessageW(hGrp3, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hGrp4 = CreateWindowExW(0, L"BUTTON", L" [4] Góc Dưới Phải (Khay 30 -> 39) ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 335, 330, 305, 330, hDlg, NULL, g_hInst, NULL);
    SendMessageW(hGrp4, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    // 1. Góc Trên Trái: Khay 14..24 (#14 ở dưới cùng, #24 ở trên cùng)
    for (int i = 0; i < 11; ++i) {
        int slot = 24 - i;
        create_feeder_slot_control_c(hDlg, slot, 25, 68 + i * 22, g_feeder_matrix_c[slot].comment);
    }

    // 2. Góc Trên Phải: Khay 40..50 (#40 ở dưới cùng, #50 ở trên cùng)
    for (int i = 0; i < 11; ++i) {
        int slot = 50 - i;
        create_feeder_slot_control_c(hDlg, slot, 345, 68 + i * 22, g_feeder_matrix_c[slot].comment);
    }

    // 3. Góc Dưới Trái: Khay 1..13 (#01 ở dưới cùng, #13 ở trên cùng)
    for (int i = 0; i < 13; ++i) {
        int slot = 13 - i;
        create_feeder_slot_control_c(hDlg, slot, 25, 352 + i * 23, g_feeder_matrix_c[slot].comment);
    }

    // 4. Góc Dưới Phải: Khay 30..39 (#30 ở dưới cùng, #39 ở trên cùng)
    for (int i = 0; i < 10; ++i) {
        int slot = 39 - i;
        create_feeder_slot_control_c(hDlg, slot, 345, 352 + i * 23, g_feeder_matrix_c[slot].comment);
    }

    HWND hBtnSave = CreateWindowExW(0, L"BUTTON", L"LƯU VÀ ÁP DỤNG NGAY", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 440, 675, 200, 36, hDlg, (HMENU)2001, g_hInst, NULL);
    SendMessageW(hBtnSave, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hBtnCancel = CreateWindowExW(0, L"BUTTON", L"Đóng", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 25, 675, 90, 36, hDlg, (HMENU)IDCANCEL, g_hInst, NULL);
    SendMessageW(hBtnCancel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
}

static void start_in_place_edit(int item, int sub_item) {
    if (item < 0 || sub_item < 1 || sub_item > 12) return;
    commit_in_place_edit(true);

    g_inPlaceItem = item;
    g_inPlaceSubItem = sub_item;

    RECT rc;
    ListView_GetSubItemRect(g_hListView, item, sub_item, LVIR_LABEL, &rc);

    wchar_t text[256] = {0};
    ListView_GetItemText(g_hListView, item, sub_item, text, 256);

    g_hInPlaceEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", text,
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top + 1,
        g_hListView, (HMENU)9999, g_hInst, NULL
    );
    if (!g_hInPlaceEdit) return;

    SendMessageW(g_hInPlaceEdit, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
    g_oldEditProc = (WNDPROC)SetWindowLongPtr(g_hInPlaceEdit, GWLP_WNDPROC, (LONG_PTR)InPlaceEditProc);
    SetFocus(g_hInPlaceEdit);
    SendMessageW(g_hInPlaceEdit, EM_SETSEL, 0, -1);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        component_list_init(&g_components);
        init_default_feeder_matrix_c();

        g_hFontTitle = CreateFontW(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        g_hFontBold = CreateFontW(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        g_hFontNormal = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        g_hWnd = hWnd;

        // Nút Mở Ma Trận Feeder 4 Góc & Nhãn Profile Nằm Ngay Dưới Nút
        HWND hBtnFeeder = CreateWindowExW(0, L"BUTTON", L"⚙️ CẤU HÌNH KHAY FEEDER 4 GÓC", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1050, 8, 290, 32, hWnd, (HMENU)301, g_hInst, NULL);
        SendMessageW(hBtnFeeder, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        wchar_t initProfText[128];
        swprintf(initProfText, 128, L"⚙️ Quy tắc đang áp dụng: [%ls]", g_active_profile_c[0] ? g_active_profile_c : L"Mac_Dinh");
        g_hLblActiveProfile_c = CreateWindowExW(0, L"STATIC", initProfText, WS_CHILD | WS_VISIBLE | SS_CENTER, 1050, 42, 290, 22, hWnd, (HMENU)404, g_hInst, NULL);
        SendMessageW(g_hLblActiveProfile_c, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

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

        g_hRadioTop = CreateWindowExW(0, L"BUTTON", L"Mat TOP", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 35, 180, 85, 24, hWnd, (HMENU)401, g_hInst, NULL);
        SendMessageW(g_hRadioTop, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
        SendMessageW(g_hRadioTop, BM_SETCHECK, BST_CHECKED, 0);

        g_hRadioBot = CreateWindowExW(0, L"BUTTON", L"Mat BOTTOM", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 125, 180, 105, 24, hWnd, (HMENU)402, g_hInst, NULL);
        SendMessageW(g_hRadioBot, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        // Checkbox Tự động nhận diện Feeder
        g_hChkAutoMatch_c = CreateWindowExW(0, L"BUTTON", L"Tự động nhận diện Feeder theo Cấu hình", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 235, 180, 275, 24, hWnd, (HMENU)302, g_hInst, NULL);
        SendMessageW(g_hChkAutoMatch_c, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
        SendMessageW(g_hChkAutoMatch_c, BM_SETCHECK, g_auto_match_feeder_c ? BST_CHECKED : BST_UNCHECKED, 0);

        // Ô Nhập Chiều Rộng Bo Mạch X (mm)
        HWND hLblBw = CreateWindowExW(0, L"STATIC", L"Chiều rộng bo X (mm):", WS_CHILD | WS_VISIBLE, 520, 183, 155, 20, hWnd, NULL, g_hInst, NULL);
        SendMessageW(hLblBw, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hEditBoardWidth_c = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 680, 180, 75, 24, hWnd, (HMENU)303, g_hInst, NULL);
        SendMessageW(g_hEditBoardWidth_c, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hStatus = CreateWindowExW(0, L"STATIC", L"Chua chon file CAD nao", WS_CHILD | WS_VISIBLE, 765, 183, 560, 20, hWnd, (HMENU)104, g_hInst, NULL);
        SendMessageW(g_hStatus, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hListView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL, 35, 210, 1290, 390, hWnd, (HMENU)105, g_hInst, NULL);
        SendMessageW(g_hListView, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
        ListView_SetExtendedListViewStyle(g_hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // 13 Cot chuan NeoDen YY1 (Tong 1286px, them 2px vua khit 100% mep phai)
        LVCOLUMNW col = {0};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
        
        col.fmt = LVCFMT_CENTER; col.cx = 42;  col.pszText = L"STT";         ListView_InsertColumn(g_hListView, 0, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 96;  col.pszText = L"Designator";  ListView_InsertColumn(g_hListView, 1, &col);
        col.fmt = LVCFMT_LEFT;   col.cx = 208; col.pszText = L"Comment";     ListView_InsertColumn(g_hListView, 2, &col);
        col.fmt = LVCFMT_LEFT;   col.cx = 162; col.pszText = L"Footprint";   ListView_InsertColumn(g_hListView, 3, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 90;  col.pszText = L"Mid X";       ListView_InsertColumn(g_hListView, 4, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 90;  col.pszText = L"Mid Y";       ListView_InsertColumn(g_hListView, 5, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 80;  col.pszText = L"Rotation";   ListView_InsertColumn(g_hListView, 6, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 55;  col.pszText = L"Head";       ListView_InsertColumn(g_hListView, 7, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 78;  col.pszText = L"FeederNo";   ListView_InsertColumn(g_hListView, 8, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 78;  col.pszText = L"Speed%";     ListView_InsertColumn(g_hListView, 9, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 88;  col.pszText = L"Pick(mm)";   ListView_InsertColumn(g_hListView, 10, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 88;  col.pszText = L"Place(mm)";  ListView_InsertColumn(g_hListView, 11, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 56;  col.pszText = L"Mode";       ListView_InsertColumn(g_hListView, 12, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 48;  col.pszText = L"Skip";       ListView_InsertColumn(g_hListView, 13, &col);

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

        // Khởi động giao diện sạch sẽ, người dùng tự chọn file cần mở
        break;
    }

    case WM_SIZE: {
        int width = LOWORD(lParam);
        if (width > 0) {
            int rightX = width - 310;
            if (rightX < 850) rightX = 850;
            HWND hBtn = GetDlgItem(hWnd, 301);
            if (hBtn) {
                SetWindowPos(hBtn, NULL, rightX, 8, 290, 32, SWP_NOZORDER | SWP_NOACTIVATE);
            }
            if (g_hLblActiveProfile_c) {
                SetWindowPos(g_hLblActiveProfile_c, NULL, rightX, 42, 290, 22, SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
        break;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        HWND hwndStatic = (HWND)lParam;
        if (hwndStatic == g_hLblActiveProfile_c) {
            SetTextColor(hdcStatic, RGB(2, 132, 199)); // #0284C7
            SetBkMode(hdcStatic, OPAQUE);
            SetBkColor(hdcStatic, GetSysColor(COLOR_BTNFACE));
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
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
                    size_t match_idx = 0;
                    for (size_t i = 0; i < g_components.count; ++i) {
                        Component* c = &g_components.items[i];
                        bool is_top = (strcmp(c->layer, "TopLayer") == 0 || strcmp(c->layer, "Top") == 0);
                        if (is_top == g_showing_top) {
                            if ((int)match_idx == item) {
                                if (subItem == 8) { // Cột FeederNo
                                    if (c->feeder_no == 0) {
                                        // CHƯA CÓ FEEDER (0): Khối Màu Đỏ Nổi Bật Cảnh Báo, Chữ Trắng
                                        lplvcd->nmcd.uItemState &= ~(CDIS_SELECTED | CDIS_FOCUS | CDIS_HOT);
                                        lplvcd->clrTextBk = RGB(220, 38, 38);
                                        lplvcd->clrText = RGB(255, 255, 255);
                                    } else {
                                        lplvcd->clrTextBk = RGB(255, 255, 255);
                                        lplvcd->clrText = RGB(30, 41, 59);
                                    }
                                } else if (subItem == 13) { // Cột Skip
                                    lplvcd->nmcd.uItemState &= ~(CDIS_SELECTED | CDIS_FOCUS | CDIS_HOT);
                                    if (c->skip != 0) {
                                        // BẬT SKIP (1): Khối Màu Đỏ Nổi Bật, Chữ Trắng (Bỏ Qua)
                                        lplvcd->clrTextBk = RGB(220, 38, 38);
                                        lplvcd->clrText = RGB(255, 255, 255);
                                    } else {
                                        // MẶC ĐỊNH (0): Khối Màu Xanh Lá Cây Đẹp (Gắp Linh Kiện)
                                        lplvcd->clrTextBk = RGB(22, 163, 74); // #16A34A (Màu Xanh Lá Cây)
                                        lplvcd->clrText = RGB(255, 255, 255);
                                    }
                                } else {
                                    // CÁC CỘT KHÁC (0..7, 9..12): MÀU TRẮNG MẶC ĐỊNH KHÔNG BỊ LOANG ĐỎ
                                    lplvcd->clrTextBk = RGB(255, 255, 255);
                                    lplvcd->clrText = RGB(30, 41, 59);
                                }
                                break;
                            }
                            match_idx++;
                        }
                    }
                    return CDRF_DODEFAULT;
                }
                }
                return CDRF_DODEFAULT;
            }
            else if (pnmh->code == NM_CLICK) {
                LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)lParam;
                if (pia->iItem >= 0 && pia->iSubItem == 13) {
                    Component* c = get_component_at_displayed_index(pia->iItem);
                    if (c) {
                        c->skip = (c->skip == 0) ? 1 : 0;
                        refresh_list_view();
                        ListView_SetItemState(g_hListView, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
                    }
                }
            }
            else if (pnmh->code == NM_DBLCLK) {
                LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)lParam;
                if (pia->iItem >= 0) {
                    if (pia->iSubItem == 13) {
                        Component* c = get_component_at_displayed_index(pia->iItem);
                        if (c) {
                            c->skip = (c->skip == 0) ? 1 : 0;
                            refresh_list_view();
                            ListView_SetItemState(g_hListView, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
                        }
                    } else if (pia->iSubItem >= 1 && pia->iSubItem <= 12) {
                        start_in_place_edit(pia->iItem, pia->iSubItem);
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
        case 301: {
            open_feeder_matrix_dialog_c(hWnd);
            break;
        }
        case 302: { // Checkbox Tự động nhận diện Feeder
            g_auto_match_feeder_c = (SendMessageW(g_hChkAutoMatch_c, BM_GETCHECK, 0, 0) == BST_CHECKED);
            if (g_auto_match_feeder_c) {
                for (size_t i = 0; i < g_components.count; ++i) {
                    g_components.items[i].feeder_no = match_feeder_slot_c(g_components.items[i].comment, g_components.items[i].footprint);
                }
            } else {
                for (size_t i = 0; i < g_components.count; ++i) {
                    g_components.items[i].feeder_no = g_components.items[i].raw_feeder_no;
                }
            }
            refresh_list_view();
            break;
        }
        case 303: { // Ô Nhập Chiều rộng bo X
            if (HIWORD(wParam) == EN_CHANGE) {
                if (g_hListView != NULL && g_hEditBoardWidth_c != NULL) {
                    recalc_bottom_coordinates_c();
                    if (!g_showing_top) {
                        refresh_list_view();
                    }
                }
            }
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
