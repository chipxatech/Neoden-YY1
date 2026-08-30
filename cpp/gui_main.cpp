#define UNICODE
#define _UNICODE

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <objidl.h>
#include <gdiplus.h>
#include <shlobj.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <iomanip>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace Gdiplus;

// Cấu trúc 13 thông số chuẩn máy NeoDen YY1
struct Component {
    std::wstring designator;
    std::wstring comment;
    std::wstring footprint;
    double mid_x = 0.0;
    double mid_y = 0.0;
    double rotation = 0.0;
    int head = 0;
    int feeder_no = 1;
    int mount_speed = 100;
    double pick_height = 0.0;
    double place_height = 0.0;
    int mode = 1;
    int skip = 0;
    std::wstring layer = L"Top";
};

struct FeederSlot {
    std::wstring comment;
    std::wstring footprint;
    int head = 0;
    int speed = 100;
};

// Global variables
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
Gdiplus::Image* g_pLogoImage = NULL;
ULONG_PTR g_gdiplusToken = 0;
HFONT g_hFontTitle = NULL;
HFONT g_hFontNormal = NULL;
HFONT g_hFontBold = NULL;
HFONT g_hFontSplashTitle = NULL;
HFONT g_hFontSplashSub = NULL;

std::vector<Component> g_top_components;
std::vector<Component> g_bot_components;
std::map<int, FeederSlot> g_feeder_matrix;
bool g_showing_top = true;
int g_splashProgress = 0;

// Header chuẩn nhúng sẵn
static const char* EMBEDDED_HEADER = 
    "NEODEN,YY1,P&P FILE,,,,,,,,,,\r\n"
    ",,,,,,,,,,,,\r\n"
    "PanelizedPCB,UnitLength,0,UnitWidth,0,Rows,1,Columns,1,,,,\r\n"
    ",,,,,,,,,,,,\r\n"
    "Fiducial,1-X,55.08,1-Y,6.95,OverallOffsetX,0.14,OverallOffsetY,0.08,,,,\r\n"
    ",,,,,,,,,,,,\r\n"
    "NozzleChange,OFF,BeforeComponent,2,Head1,Drop,Station1,PickUp,Station3,,,,\r\n"
    "NozzleChange,OFF,BeforeComponent,3,Head1,Drop,Station3,PickUp,Station1,,,,\r\n"
    "NozzleChange,OFF,BeforeComponent,1,Head1,Drop,Station1,PickUp,Station1,,,,\r\n"
    "NozzleChange,OFF,BeforeComponent,1,Head1,Drop,Station1,PickUp,Station1,,,,\r\n"
    ",,,,,,,,,,,,\r\n"
    "Designator,Comment,Footprint,Mid X(mm),Mid Y(mm) ,Rotation,Head ,FeederNo,Mount Speed(%),Pick Height(mm),Place Height(mm),Mode,Skip\r\n";

void initDefaultFeederMatrix() {
    // Góc Dưới Trái 1..13
    g_feeder_matrix[1] = {L"100nF", L"0603", 0, 100};
    g_feeder_matrix[2] = {L"10k", L"0603", 0, 100};
    g_feeder_matrix[3] = {L"1k", L"0603", 0, 100};
    g_feeder_matrix[4] = {L"4.7k", L"0603", 0, 100};
    g_feeder_matrix[5] = {L"0R", L"0603", 0, 100};
    g_feeder_matrix[6] = {L"22pF", L"0603", 0, 100};
    g_feeder_matrix[7] = {L"1uF", L"0603", 0, 100};
    g_feeder_matrix[8] = {L"10uF", L"0805", 0, 100};
    g_feeder_matrix[9] = {L"47uF", L"0805", 0, 100};
    g_feeder_matrix[10] = {L"LED_RED", L"0603", 0, 100};
    g_feeder_matrix[11] = {L"LED_GREEN", L"0603", 0, 100};
    g_feeder_matrix[12] = {L"100k", L"0603", 0, 100};
    g_feeder_matrix[13] = {L"2.2k", L"0603", 0, 100};

    // Góc Trên Trái 14..24
    for (int i = 14; i <= 24; ++i) g_feeder_matrix[i] = {L"", L"0603", 0, 100};
    g_feeder_matrix[22] = {L"", L"SOT-23", 0, 100};
    g_feeder_matrix[23] = {L"", L"SOT-23", 0, 100};
    g_feeder_matrix[24] = {L"", L"SOD-123", 0, 100};

    // Góc Dưới Phải 30..39
    g_feeder_matrix[30] = {L"SS34", L"SMA", 0, 100};
    g_feeder_matrix[31] = {L"1N4148", L"SOD-123", 0, 100};
    g_feeder_matrix[32] = {L"S8050", L"SOT-23", 0, 100};
    g_feeder_matrix[33] = {L"S8550", L"SOT-23", 0, 100};
    g_feeder_matrix[34] = {L"AMS1117-3.3", L"SOT-223", 0, 90};
    g_feeder_matrix[35] = {L"AMS1117-5.0", L"SOT-223", 0, 90};
    g_feeder_matrix[36] = {L"BSS138", L"SOT-23", 0, 100};
    g_feeder_matrix[37] = {L"AO3400", L"SOT-23", 0, 100};
    g_feeder_matrix[38] = {L"AO3401", L"SOT-23", 0, 100};
    g_feeder_matrix[39] = {L"CH340C", L"SOP-16", 0, 90};

    // Góc Trên Phải 40..50
    for (int i = 40; i <= 50; ++i) g_feeder_matrix[i] = {L"", L"SOP-8", 0, 100};
}

std::wstring trim(const std::wstring& s) {
    size_t first = s.find_first_not_of(L" \t\r\n\"");
    if (first == std::wstring::npos) return L"";
    size_t last = s.find_last_not_of(L" \t\r\n\"");
    return s.substr(first, (last - first + 1));
}

std::wstring toLower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c){ return towlower(c); });
    return s;
}

std::vector<std::wstring> parseCsvLine(const std::wstring& line) {
    std::vector<std::wstring> fields;
    std::wstring current = L"";
    bool in_quotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        wchar_t ch = line[i];
        if (ch == L'"') {
            if (in_quotes && i + 1 < line.length() && line[i + 1] == L'"') {
                current += L'"';
                ++i;
            } else {
                in_quotes = !in_quotes;
            }
        } else if (ch == L',' && !in_quotes) {
            fields.push_back(trim(current));
            current.clear();
        } else {
            current += ch;
        }
    }
    fields.push_back(trim(current));
    return fields;
}

std::pair<std::wstring, int> extractPrefixAndNumber(const std::wstring& s) {
    std::wstring prefix = L"";
    std::wstring num_str = L"";
    bool in_num = false;

    for (wchar_t ch : s) {
        if (iswdigit(ch)) {
            in_num = true;
            num_str += ch;
        } else if (!in_num) {
            prefix += ch;
        }
    }
    int num = num_str.empty() ? 0 : _wtoi(num_str.c_str());
    return {prefix, num};
}

std::string ws2s(const std::wstring& ws) {
    if (ws.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring s2ws(const std::string& s) {
    if (s.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &s[0], (int)s.size(), NULL, 0);
    if (size_needed == 0) {
        size_needed = MultiByteToWideChar(CP_ACP, 0, &s[0], (int)s.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_ACP, 0, &s[0], (int)s.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &s[0], (int)s.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::wstring normalizeFootprint(const std::wstring& fp) {
    if (fp.empty()) return L"0603D";
    std::wstring fl = toLower(fp);
    if (fl.find(L"0603") != std::wstring::npos) return L"0603D";
    if (fl.find(L"0805") != std::wstring::npos) return L"0805D";
    if (fl.find(L"1206") != std::wstring::npos) {
        if (fl.find(L"cau_chi") != std::wstring::npos || fl.find(L"fuse") != std::wstring::npos) return L"1206_FUSE";
        return L"1206D";
    }
    if (fl.find(L"0630") != std::wstring::npos) return L"IND_0630";
    if (fl.find(L"tantalum") != std::wstring::npos || fl.find(L"7443") != std::wstring::npos || fl.find(L"7343") != std::wstring::npos) return L"TANTAL_7343";
    if (fl.find(L"button_2p") != std::wstring::npos || fl.find(L"nut_nhan_2p") != std::wstring::npos) return L"SW_2P_SMD";
    if (fl.find(L"button_4p") != std::wstring::npos || fl.find(L"nut_nhan_4p") != std::wstring::npos) return L"SW_4P_SMD";
    if (fl.find(L"header") != std::wstring::npos || fl.find(L"hdr") != std::wstring::npos) {
        if (fl.find(L"1.25") != std::wstring::npos) return L"HDR_1.25_2P_SMD";
        if (fl.find(L"4p") != std::wstring::npos) return L"HDR_2.0_4P_SMD";
        if (fl.find(L"2p") != std::wstring::npos) return L"HDR_2.0_2P_SMD";
    }
    if (fl.find(L"sma") != std::wstring::npos) return L"SMA";
    if (fl.find(L"tesdu") != std::wstring::npos) return L"SOD-323";
    if (fl.find(L"vr_") != std::wstring::npos) return L"POT_SMD";
    if (fl.find(L"via") != std::wstring::npos) return L"VIA_2.2MM";
    if (fl.find(L"sdcard") != std::wstring::npos || fl.find(L"tf3") != std::wstring::npos) return L"TF_CARD_SMD";
    if (fl.find(L"soic-16") != std::wstring::npos || fl.find(L"sop-16") != std::wstring::npos) return L"SOP-16";
    if (fl.find(L"typec") != std::wstring::npos || fl.find(L"type-c") != std::wstring::npos) return L"USB_TYPE_C";
    return fp;
}

std::wstring normalizeComment(const std::wstring& cmt) {
    if (cmt.empty()) return L"";
    std::wstring cl = toLower(cmt);
    if (cl.find(L"cau chi") != std::wstring::npos || cl.find(L"fuse") != std::wstring::npos) return L"FUSE_1206";
    if (cl.find(L"nut nhan 2p") != std::wstring::npos) return L"TACT_SW_2P";
    if (cl.find(L"nut nhan 4") != std::wstring::npos) return L"TACT_SW_4P";
    if (cl == L"nguon") return L"POWER_HDR";
    if (cl == L"bomkhi") return L"AIR_PUMP";
    if (cl == L"vankhi") return L"AIR_VALVE";
    if (cl.find(L"led 0603") != std::wstring::npos || cl == L"led") return L"LED_0603";
    if (cl.find(L"sdcard") != std::wstring::npos || cl.find(L"tf3") != std::wstring::npos) return L"MICRO_SD_TF3";
    if (cl.find(L"pressure sensor") != std::wstring::npos) return L"PRESSURE_SENSOR";
    return cmt;
}

int matchFeederSlot(const std::wstring& comment, const std::wstring& footprint) {
    std::wstring cmt = toLower(comment);
    std::wstring fp = toLower(footprint);

    for (const auto& [slot, cfg] : g_feeder_matrix) {
        if (!cfg.comment.empty() && toLower(cfg.comment) == cmt) return slot;
    }
    for (const auto& [slot, cfg] : g_feeder_matrix) {
        if (cfg.comment.empty() && !cfg.footprint.empty() && toLower(cfg.footprint) == fp) return slot;
    }
    return 1;
}

void refreshListView() {
    ListView_DeleteAllItems(g_hListView);
    const auto& list = g_showing_top ? g_top_components : g_bot_components;

    for (size_t i = 0; i < list.size(); ++i) {
        const auto& c = list[i];
        LVITEM lvItem = {0};
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = (int)i;

        std::wstring stt = std::to_wstring(i + 1);
        lvItem.pszText = (LPWSTR)stt.c_str();
        ListView_InsertItem(g_hListView, &lvItem);

        ListView_SetItemText(g_hListView, (int)i, 1, (LPWSTR)c.designator.c_str());
        ListView_SetItemText(g_hListView, (int)i, 2, (LPWSTR)c.comment.c_str());
        ListView_SetItemText(g_hListView, (int)i, 3, (LPWSTR)c.footprint.c_str());

        wchar_t buf[64];
        swprintf(buf, 64, L"%.2f", c.mid_x); ListView_SetItemText(g_hListView, (int)i, 4, buf);
        swprintf(buf, 64, L"%.2f", c.mid_y); ListView_SetItemText(g_hListView, (int)i, 5, buf);
        swprintf(buf, 64, L"%.2f", c.rotation); ListView_SetItemText(g_hListView, (int)i, 6, buf);
        swprintf(buf, 64, L"%d", c.head); ListView_SetItemText(g_hListView, (int)i, 7, buf);
        swprintf(buf, 64, L"%d", c.feeder_no); ListView_SetItemText(g_hListView, (int)i, 8, buf);
        swprintf(buf, 64, L"%d", c.mount_speed); ListView_SetItemText(g_hListView, (int)i, 9, buf);
        swprintf(buf, 64, L"%.2f", c.pick_height); ListView_SetItemText(g_hListView, (int)i, 10, buf);
        swprintf(buf, 64, L"%.2f", c.place_height); ListView_SetItemText(g_hListView, (int)i, 11, buf);
        swprintf(buf, 64, L"%d", c.mode); ListView_SetItemText(g_hListView, (int)i, 12, buf);
        ListView_SetItemText(g_hListView, (int)i, 13, (LPWSTR)L"");
    }

    wchar_t statusText[256];
    swprintf(statusText, 256, L"✔ Mặt TOP: %zu pcs  |  Mặt BOTTOM: %zu pcs  |  Đang hiển thị: %s (%zu pcs)  |  Nhấp đúp chuột để sửa",
             g_top_components.size(), g_bot_components.size(), g_showing_top ? L"TOP" : L"BOTTOM", list.size());
    SetWindowTextW(g_hStatus, statusText);
}

bool loadAltiumData(const std::wstring& filepath) {
    std::ifstream file(ws2s(filepath), std::ios::binary);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string raw = buffer.str();
    std::wstring content = s2ws(raw);

    g_top_components.clear();
    g_bot_components.clear();

    std::wistringstream stream(content);
    std::wstring line;
    bool header_found = false;
    bool is_mil = false;

    int col_des = -1, col_cmt = -1, col_layer = -1, col_fp = -1;
    int col_x = -1, col_y = -1, col_rot = -1;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        if (line.empty()) continue;

        if (toLower(line).find(L"units used: mil") != std::wstring::npos) is_mil = true;

        if (!header_found) {
            if (line.find(L"Designator") != std::wstring::npos && line.find(L"Comment") != std::wstring::npos) {
                header_found = true;
                auto cols = parseCsvLine(line);
                for (size_t i = 0; i < cols.size(); ++i) {
                    std::wstring c = toLower(cols[i]);
                    if (c == L"designator") col_des = (int)i;
                    else if (c == L"comment") col_cmt = (int)i;
                    else if (c == L"layer") col_layer = (int)i;
                    else if (c == L"footprint") col_fp = (int)i;
                    else if (c.rfind(L"center-x", 0) == 0 || c.rfind(L"mid x", 0) == 0) {
                        col_x = (int)i;
                        if (c.find(L"mil") != std::wstring::npos) is_mil = true;
                    } else if (c.rfind(L"center-y", 0) == 0 || c.rfind(L"mid y", 0) == 0) {
                        col_y = (int)i;
                    } else if (c.rfind(L"rotation", 0) == 0) {
                        col_rot = (int)i;
                    }
                }
            }
            continue;
        }

        auto fields = parseCsvLine(line);
        auto get_val = [&](int idx) -> std::wstring {
            if (idx >= 0 && idx < (int)fields.size()) return fields[idx];
            return L"";
        };

        std::wstring des = get_val(col_des);
        if (des.empty() || des[0] == L'*') continue;

        Component comp;
        comp.designator = des;
        comp.comment = normalizeComment(get_val(col_cmt));
        comp.footprint = normalizeFootprint(get_val(col_fp));
        comp.layer = get_val(col_layer);

        try {
            double rx = get_val(col_x).empty() ? 0.0 : std::stod(get_val(col_x));
            double ry = get_val(col_y).empty() ? 0.0 : std::stod(get_val(col_y));
            comp.rotation = get_val(col_rot).empty() ? 0.0 : std::stod(get_val(col_rot));
            comp.mid_x = is_mil ? rx * 0.0254 : rx;
            comp.mid_y = is_mil ? ry * 0.0254 : ry;
        } catch (...) { continue; }

        comp.feeder_no = matchFeederSlot(comp.comment, comp.footprint);
        comp.head = 0;
        comp.mount_speed = 100;
        comp.pick_height = 0.0;
        comp.place_height = 0.0;
        comp.mode = 1;
        comp.skip = 0;

        std::wstring l = toLower(comp.layer);
        if (l.find(L"bottom") != std::wstring::npos || l.find(L"bot") != std::wstring::npos || l == L"b") {
            g_bot_components.push_back(comp);
        } else {
            g_top_components.push_back(comp);
        }
    }

    auto sort_fn = [](const Component& a, const Component& b) {
        std::wstring ca = toLower(a.comment), cb = toLower(b.comment);
        if (ca != cb) return ca < cb;
        auto pa = extractPrefixAndNumber(a.designator), pb = extractPrefixAndNumber(b.designator);
        if (pa.first != pb.first) return pa.first < pb.first;
        if (pa.second != pb.second) return pa.second < pb.second;
        return a.designator < b.designator;
    };

    std::sort(g_top_components.begin(), g_top_components.end(), sort_fn);
    std::sort(g_bot_components.begin(), g_bot_components.end(), sort_fn);

    refreshListView();
    return true;
}

void doSaveAndExport() {
    if (g_top_components.empty() && g_bot_components.empty()) {
        MessageBoxW(g_hWnd, L"Chưa có dữ liệu để lưu!", L"Thông Báo", MB_ICONWARNING);
        return;
    }

    wchar_t topName[256] = {0}, botName[256] = {0};
    GetWindowTextW(g_hEditTop, topName, 256);
    GetWindowTextW(g_hEditBot, botName, 256);

    std::string headerStr = EMBEDDED_HEADER;
    std::ifstream tf("0603Demo.csv", std::ios::binary);
    if (!tf.is_open()) tf.open("../0603Demo.csv", std::ios::binary);
    if (tf.is_open()) {
        std::string t_acc, t_line;
        while (std::getline(tf, t_line)) {
            t_acc += t_line + "\r\n";
            if (t_line.find("Designator") != std::string::npos && t_line.find("Comment") != std::string::npos) {
                headerStr = t_acc;
                break;
            }
        }
    }

    // Xuất TOP
    std::ofstream outTop(ws2s(topName), std::ios::binary);
    if (outTop.is_open()) {
        outTop << headerStr;
        outTop << std::fixed << std::setprecision(2);
        for (const auto& c : g_top_components) {
            outTop << ws2s(c.designator) << ","
                   << ws2s(c.comment) << ","
                   << ws2s(c.footprint) << ","
                   << c.mid_x << ","
                   << c.mid_y << ","
                   << c.rotation << ","
                   << c.head << ","
                   << c.feeder_no << ","
                   << c.mount_speed << ","
                   << c.pick_height << ","
                   << c.place_height << ","
                   << c.mode << ","
                   << c.skip << "\r\n";
        }
        outTop.close();
    }

    // Xuất BOT
    std::ofstream outBot(ws2s(botName), std::ios::binary);
    if (outBot.is_open()) {
        outBot << headerStr;
        outBot << std::fixed << std::setprecision(2);
        for (const auto& c : g_bot_components) {
            outBot << ws2s(c.designator) << ","
                   << ws2s(c.comment) << ","
                   << ws2s(c.footprint) << ","
                   << c.mid_x << ","
                   << c.mid_y << ","
                   << c.rotation << ","
                   << c.head << ","
                   << c.feeder_no << ","
                   << c.mount_speed << ","
                   << c.pick_height << ","
                   << c.place_height << ","
                   << c.mode << ","
                   << c.skip << "\r\n";
        }
        outBot.close();
    }

    wchar_t msg[512];
    swprintf(msg, 512, 
        L"🎉 ĐÃ LƯU FILE CHỈNH SỬA CHO MÁY NEODEN YY1!\n\n"
        L"⭐ Mặt TOP:\n   • Số linh kiện: %zu\n   • File: %s\n\n"
        L"⭐ Mặt BOTTOM:\n   • Số linh kiện: %zu\n   • File: %s\n\n"
        L"Toàn bộ 13 thông số đã chỉnh sửa được lưu chính xác 100%.\nBạn có muốn mở thư mục chứa file vừa lưu?",
        g_top_components.size(), topName,
        g_bot_components.size(), botName
    );

    if (MessageBoxW(g_hWnd, msg, L"Lưu Thành Công", MB_ICONINFORMATION | MB_YESNO) == IDYES) {
        ShellExecuteW(NULL, L"open", L".", NULL, NULL, SW_SHOWNORMAL);
    }
}

// Feeder Dialog Proc
LRESULT CALLBACK FeederDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == 2001) {
            // Apply feeders back
            for (auto& c : g_top_components) c.feeder_no = matchFeederSlot(c.comment, c.footprint);
            for (auto& c : g_bot_components) c.feeder_no = matchFeederSlot(c.comment, c.footprint);
            refreshListView();
            DestroyWindow(hWnd);
        } else if (LOWORD(wParam) == IDCANCEL) {
            DestroyWindow(hWnd);
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

void openFeederMatrixDialog(HWND parent) {
    HWND hDlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"#32770", L"⚙️ Ma Trận Khay Feeder 4 Góc (NeoDen YY1)",
                               WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
                               200, 150, 800, 560, parent, NULL, g_hInst, NULL);
    if (!hDlg) return;

    SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LONG_PTR)FeederDlgProc);

    HWND hGrp1 = CreateWindowExW(0, L"BUTTON", L" 📌 Góc Trên Trái (Khay 14 → 24) ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 15, 10, 370, 220, hDlg, NULL, g_hInst, NULL);
    SendMessageW(hGrp1, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hGrp2 = CreateWindowExW(0, L"BUTTON", L" 📌 Góc Trên Phải (Khay 40 → 50) ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 400, 10, 370, 220, hDlg, NULL, g_hInst, NULL);
    SendMessageW(hGrp2, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hGrp3 = CreateWindowExW(0, L"BUTTON", L" 📌 Góc Dưới Trái (Khay 1 → 13) ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 15, 240, 370, 220, hDlg, NULL, g_hInst, NULL);
    SendMessageW(hGrp3, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hGrp4 = CreateWindowExW(0, L"BUTTON", L" 📌 Góc Dưới Phải (Khay 30 → 39) ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 400, 240, 370, 220, hDlg, NULL, g_hInst, NULL);
    SendMessageW(hGrp4, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hBtnSave = CreateWindowExW(0, L"BUTTON", L"💾 LƯU CẤU HÌNH & ÁP DỤNG", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 530, 475, 240, 35, hDlg, (HMENU)2001, g_hInst, NULL);
    SendMessageW(hBtnSave, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
}

// Edit Row Dialog
void editSelectedRow(HWND parent) {
    auto& list = g_showing_top ? g_top_components : g_bot_components;
    int sel = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
    if (sel < 0 || sel >= (int)list.size()) {
        MessageBoxW(parent, L"Vui lòng chọn một dòng linh kiện để sửa!", L"Thông Báo", MB_ICONWARNING);
        return;
    }

    auto& c = list[sel];
    wchar_t prompt[256];
    swprintf(prompt, 256, L"Nhập số khay FeederNo mới cho [%s] (Hiện tại: %d):", c.designator.c_str(), c.feeder_no);
    
    // Toggle Skip nhanh hoặc chỉnh sửa FeederNo
    c.feeder_no = (c.feeder_no % 50) + 1;
    refreshListView();
    ListView_SetItemState(g_hListView, sel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

// Splash Window Procedure
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

        HBRUSH hBgBrush = CreateSolidBrush(RGB(18, 24, 38));
        FillRect(hdc, &rect, hBgBrush);
        DeleteObject(hBgBrush);

        HPEN hBorderPen = CreatePen(PS_SOLID, 2, RGB(0, 210, 255));
        HGDIOBJ oldPen = SelectObject(hdc, hBorderPen);
        HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, 0, 0, rect.right, rect.bottom);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(hBorderPen);

        Graphics graphics(hdc);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);

        if (g_pLogoImage && g_pLogoImage->GetLastStatus() == Ok) {
            graphics.DrawImage(g_pLogoImage, (rect.right - 90) / 2, 20, 90, 90);
        }

        SetBkMode(hdc, TRANSPARENT);

        SelectObject(hdc, g_hFontSplashTitle);
        SetTextColor(hdc, RGB(248, 250, 252));
        RECT titleRect = {0, 122, rect.right, 154};
        DrawTextW(hdc, L"NeoDen YY1 SMT Converter Pro", -1, &titleRect, DT_CENTER | DT_SINGLELINE);

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

        const wchar_t* loadText = L"Đang khởi tạo hệ thống...";
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

// Main Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        g_hFontTitle = CreateFontW(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        g_hFontBold = CreateFontW(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        g_hFontNormal = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

        // Nút Mở Ma Trận Feeder 4 Góc trên Header
        HWND hBtnFeeder = CreateWindowExW(0, L"BUTTON", L"⚙️ CẤU HÌNH KHAY FEEDER 4 GÓC", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1060, 20, 275, 38, hWnd, (HMENU)301, g_hInst, NULL);
        SendMessageW(hBtnFeeder, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

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

        // Radio Chuyen doi Mat TOP / BOTTOM
        g_hRadioTop = CreateWindowExW(0, L"BUTTON", L"Mat TOP", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 35, 180, 100, 24, hWnd, (HMENU)401, g_hInst, NULL);
        SendMessageW(g_hRadioTop, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
        SendMessageW(g_hRadioTop, BM_SETCHECK, BST_CHECKED, 0);

        g_hRadioBot = CreateWindowExW(0, L"BUTTON", L"Mat BOTTOM", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 145, 180, 120, 24, hWnd, (HMENU)402, g_hInst, NULL);
        SendMessageW(g_hRadioBot, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        HWND hBtnEdit = CreateWindowExW(0, L"BUTTON", L"Doi Khay Feeder", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1170, 178, 130, 26, hWnd, (HMENU)403, g_hInst, NULL);
        SendMessageW(hBtnEdit, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        g_hStatus = CreateWindowExW(0, L"STATIC", L"Dang nap du lieu...", WS_CHILD | WS_VISIBLE, 275, 183, 880, 20, hWnd, (HMENU)104, g_hInst, NULL);
        SendMessageW(g_hStatus, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hListView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL, 35, 210, 1290, 390, hWnd, (HMENU)105, g_hInst, NULL);
        SendMessageW(g_hListView, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
        ListView_SetExtendedListViewStyle(g_hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // Du 13 cot cua NeoDen YY1 (Tong 1286px, them 2px vua khit 100% mep phai)
        LVCOLUMNW col = {0};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
        
        col.fmt = LVCFMT_CENTER; col.cx = 45;  col.pszText = (LPWSTR)L"STT";         ListView_InsertColumn(g_hListView, 0, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 100; col.pszText = (LPWSTR)L"Designator";  ListView_InsertColumn(g_hListView, 1, &col);
        col.fmt = LVCFMT_LEFT;   col.cx = 200; col.pszText = (LPWSTR)L"Comment";     ListView_InsertColumn(g_hListView, 2, &col);
        col.fmt = LVCFMT_LEFT;   col.cx = 160; col.pszText = (LPWSTR)L"Footprint";   ListView_InsertColumn(g_hListView, 3, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 92;  col.pszText = (LPWSTR)L"Mid X";       ListView_InsertColumn(g_hListView, 4, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 92;  col.pszText = (LPWSTR)L"Mid Y";       ListView_InsertColumn(g_hListView, 5, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 80;  col.pszText = (LPWSTR)L"Rotation";   ListView_InsertColumn(g_hListView, 6, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 55;  col.pszText = (LPWSTR)L"Head";       ListView_InsertColumn(g_hListView, 7, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 80;  col.pszText = (LPWSTR)L"FeederNo";   ListView_InsertColumn(g_hListView, 8, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 80;  col.pszText = (LPWSTR)L"Speed%";     ListView_InsertColumn(g_hListView, 9, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 88;  col.pszText = (LPWSTR)L"Pick(mm)";   ListView_InsertColumn(g_hListView, 10, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 88;  col.pszText = (LPWSTR)L"Place(mm)";  ListView_InsertColumn(g_hListView, 11, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 60;  col.pszText = (LPWSTR)L"Mode";       ListView_InsertColumn(g_hListView, 12, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 66;  col.pszText = (LPWSTR)L"Skip";       ListView_InsertColumn(g_hListView, 13, &col);

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

        std::vector<std::wstring> candidates = {L"Pick Place for MainPCB.csv", L"MainPCB.csv", L"Pick Place for MainPCB.txt", L"../Pick Place for MainPCB.csv"};
        for (const auto& c : candidates) {
            if (GetFileAttributesW(c.c_str()) != INVALID_FILE_ATTRIBUTES) {
                SetWindowTextW(g_hEditInput, c.c_str());
                loadAltiumData(c);
                break;
            }
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        Graphics graphics(hdc);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);

        if (g_pLogoImage && g_pLogoImage->GetLastStatus() == Ok) {
            graphics.DrawImage(g_pLogoImage, 25, 10, 56, 56);
        }

        SetBkMode(hdc, TRANSPARENT);
        SelectObject(hdc, g_hFontTitle);
        SetTextColor(hdc, RGB(20, 40, 80));
        TextOutW(hdc, 90, 10, L"NeoDen YY1 SMT Pick & Place File Converter Pro", 46);

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
                        const auto& list = g_showing_top ? g_top_components : g_bot_components;
                        if (item >= 0 && item < (int)list.size()) {
                            if (list[item].skip != 0) {
                                // BẬT: Khối Màu Đỏ Nổi Bật
                                lplvcd->clrTextBk = RGB(220, 38, 38);
                                lplvcd->clrText = RGB(220, 38, 38);
                            } else {
                                // TẮT: Khối Màu Đen
                                lplvcd->clrTextBk = RGB(20, 24, 34);
                                lplvcd->clrText = RGB(20, 24, 34);
                            }
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
                    auto& list = g_showing_top ? g_top_components : g_bot_components;
                    if (pia->iItem < (int)list.size()) {
                        list[pia->iItem].skip = (list[pia->iItem].skip == 0) ? 1 : 0;
                        refreshListView();
                        ListView_SetItemState(g_hListView, pia->iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                    }
                }
            }
            else if (pnmh->code == NM_DBLCLK) {
                LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)lParam;
                if (pia->iItem >= 0) {
                    if (pia->iSubItem == 13) {
                        auto& list = g_showing_top ? g_top_components : g_bot_components;
                        if (pia->iItem < (int)list.size()) {
                            list[pia->iItem].skip = (list[pia->iItem].skip == 0) ? 1 : 0;
                            refreshListView();
                            ListView_SetItemState(g_hListView, pia->iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                        }
                    } else {
                        editSelectedRow(hWnd);
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
                loadAltiumData(ofn.lpstrFile);
            }
            break;
        }
        case 103: {
            wchar_t path[MAX_PATH] = {0};
            GetWindowTextW(g_hEditInput, path, MAX_PATH);
            if (path[0] != L'\0') {
                loadAltiumData(path);
            }
            break;
        }
        case 201: {
            doSaveAndExport();
            break;
        }
        case 301: {
            openFeederMatrixDialog(hWnd);
            break;
        }
        case 401: { // Radio TOP
            g_showing_top = true;
            refreshListView();
            break;
        }
        case 402: { // Radio BOT
            g_showing_top = false;
            refreshListView();
            break;
        }
        case 403: { // Sửa Feeder
            editSelectedRow(hWnd);
            break;
        }
        }
        break;
    }

    case WM_DESTROY:
        if (g_hFontTitle) DeleteObject(g_hFontTitle);
        if (g_hFontBold) DeleteObject(g_hFontBold);
        if (g_hFontNormal) DeleteObject(g_hFontNormal);
        if (g_hFontSplashTitle) DeleteObject(g_hFontSplashTitle);
        if (g_hFontSplashSub) DeleteObject(g_hFontSplashSub);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    g_hInst = hInstance;
    initDefaultFeederMatrix();

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);

    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

    std::wstring logoPath = L"assets/logo.png";
    if (GetFileAttributesW(logoPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        logoPath = L"../assets/logo.png";
    }
    g_pLogoImage = new Gdiplus::Image(logoPath.c_str());

    g_hFontSplashTitle = CreateFontW(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g_hFontSplashSub = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    // Đăng ký Splash Class
    WNDCLASSEXW splashClass = {0};
    splashClass.cbSize = sizeof(WNDCLASSEXW);
    splashClass.style = CS_HREDRAW | CS_VREDRAW;
    splashClass.lpfnWndProc = SplashProc;
    splashClass.hInstance = hInstance;
    splashClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    splashClass.lpszClassName = L"NeoDenYY1Splash";
    RegisterClassExW(&splashClass);

    // Đăng ký Main Class
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcex.lpszClassName = L"NeoDenYY1GUI";

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
    g_hWnd = CreateWindowW(L"NeoDenYY1GUI", L"NeoDen YY1 SMT Pick & Place File Converter Pro - ChipXa",
                           WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
                           winX, winY, winW, winH, NULL, NULL, hInstance, NULL);

    if (g_hWnd) {
        SendMessageW(g_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
        SendMessageW(g_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);
    }

    int splashW = 500, splashH = 290;
    int splashX = (screenW - splashW) / 2, splashY = (screenH - splashH) / 2;
    g_hSplash = CreateWindowExW(WS_EX_TOPMOST, L"NeoDenYY1Splash", L"",
                                WS_POPUP | WS_VISIBLE, splashX, splashY, splashW, splashH,
                                NULL, NULL, hInstance, NULL);

    ShowWindow(g_hSplash, SW_SHOWNORMAL);
    UpdateWindow(g_hSplash);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_pLogoImage) delete g_pLogoImage;
    GdiplusShutdown(g_gdiplusToken);

    return (int)msg.wParam;
}
