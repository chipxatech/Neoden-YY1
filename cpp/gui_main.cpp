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
HWND g_hChkAutoMatch = NULL;
bool g_auto_match_feeder = true;
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
    // 50 khay mặc định theo cấu hình máy
    for (int i = 1; i <= 50; ++i) g_feeder_matrix[i] = {L"", L"0603", 0, 100};

    // Góc Dưới Trái 1..13 theo ảnh mẫu
    g_feeder_matrix[1] = {L"1K-0603", L"0603", 0, 100};
    g_feeder_matrix[2] = {L"100uF-0603", L"0603", 0, 100};
    g_feeder_matrix[3] = {L"10K-0805", L"0805", 0, 100};
    g_feeder_matrix[4] = {L"100uF-0805", L"0805", 0, 100};
    g_feeder_matrix[5] = {L"1K-0805", L"0805", 0, 100};
    g_feeder_matrix[6] = {L"10K-0603", L"0603", 0, 100};
    g_feeder_matrix[7] = {L"10uF-0805", L"0805", 0, 100};
    g_feeder_matrix[8] = {L"2SC1805", L"SOT-23", 0, 100};
    g_feeder_matrix[9] = {L"4.7K-0805", L"0805", 0, 100};

    // Góc Dưới Phải 30..39 theo ảnh mẫu
    g_feeder_matrix[30] = {L"Red-0805", L"0805", 0, 100};
    g_feeder_matrix[31] = {L"Green-0805", L"0805", 0, 100};
    g_feeder_matrix[32] = {L"Yellow-0805", L"0805", 0, 100};
    g_feeder_matrix[33] = {L"Blue-0805", L"0805", 0, 100};
    g_feeder_matrix[34] = {L"Red-0603", L"0603", 0, 100};
    g_feeder_matrix[35] = {L"Blue-0603", L"0603", 0, 100};
}

std::wstring trim(const std::wstring& s) {
    size_t first = s.find_first_not_of(L" \t\r\n\"");
    if (first == std::wstring::npos) return L"";
    size_t last = s.find_last_not_of(L" \t\r\n\"");
    return s.substr(first, (last - first + 1));
}

std::wstring toLower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c){ return (wchar_t)towlower(c); });
    return s;
}

static std::wstring cleanColName(const std::wstring& s) {
    std::wstring out;
    for (wchar_t c : s) {
        if (iswalnum(c)) {
            out += (wchar_t)towlower(c);
        }
    }
    return out;
}

std::vector<std::wstring> parseCsvLine(const std::wstring& line) {
    wchar_t delimiter = L',';
    int commas = 0, semis = 0, tabs = 0;
    bool in_q = false;
    for (wchar_t ch : line) {
        if (ch == L'"') in_q = !in_q;
        else if (!in_q) {
            if (ch == L',') commas++;
            else if (ch == L';') semis++;
            else if (ch == L'\t') tabs++;
        }
    }
    if (tabs > commas && tabs > semis) delimiter = L'\t';
    else if (semis > commas && semis > tabs) delimiter = L';';

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
        } else if (ch == delimiter && !in_quotes) {
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
    if (cl.find(L"sdcard") != std::wstring::npos || cl.find(L"tf3") != std::wstring::npos) return L"MICRO_SD_TF3";
    if (cl.find(L"pressure sensor") != std::wstring::npos) return L"PRESSURE_SENSOR";
    return cmt;
}

static std::wstring removeDiacriticsCpp(const std::wstring& str) {
    std::wstring s = toLower(str);
    for (size_t i = 0; i < s.length(); ++i) {
        wchar_t c = s[i];
        if ((c >= 0x00E0 && c <= 0x00E5) || c == 0x0103 || c == 0x1EA1 || c == 0x1EA3 || c == 0x1EA5 || c == 0x1EA7 || c == 0x1EA9 || c == 0x1EAB || c == 0x1EAD || c == 0x1EAF || c == 0x1EB1 || c == 0x1EB3 || c == 0x1EB5 || c == 0x1EB7 || c == 0x00E2) s[i] = L'a';
        else if ((c >= 0x00E8 && c <= 0x00EB) || c == 0x00EA || c == 0x1EB9 || c == 0x1EBB || c == 0x1EBD || c == 0x1EBF || c == 0x1EC1 || c == 0x1EC3 || c == 0x1EC5 || c == 0x1EC7) s[i] = L'e';
        else if ((c >= 0x00EC && c <= 0x00EF) || c == 0x1EC9 || c == 0x1ECB) s[i] = L'i';
        else if ((c >= 0x00F2 && c <= 0x00F6) || c == 0x00F4 || c == 0x01A1 || c == 0x1ECD || c == 0x1ECF || c == 0x1ED1 || c == 0x1ED3 || c == 0x1ED5 || c == 0x1ED7 || c == 0x1ED9 || c == 0x1EDB || c == 0x1EDD || c == 0x1EDF || c == 0x1EE1 || c == 0x1EE3) s[i] = L'o';
        else if ((c >= 0x00F9 && c <= 0x00FC) || c == 0x01B0 || c == 0x1EE5 || c == 0x1EE7 || c == 0x1EE9 || c == 0x1EEB || c == 0x1EED || c == 0x1EEF || c == 0x1EF1) s[i] = L'u';
        else if (c == 0x00FD || c == 0x1EF3 || c == 0x1EF5 || c == 0x1EF7 || c == 0x1EF9) s[i] = L'y';
        else if (c == 0x0111 || c == 0x0110) s[i] = L'd';
    }
    return s;
}

static std::wstring canonicalizeFeederKeywordsCpp(const std::wstring& input) {
    std::wstring s = removeDiacriticsCpp(input);
    size_t pos = 0;
    while ((pos = s.find(L"xanh la", pos)) != std::wstring::npos) { s.replace(pos, 7, L"green"); pos += 5; }
    pos = 0;
    while ((pos = s.find(L"xanh cay", pos)) != std::wstring::npos) { s.replace(pos, 8, L"green"); pos += 5; }
    pos = 0;
    while ((pos = s.find(L"xanh duong", pos)) != std::wstring::npos) { s.replace(pos, 10, L"blue"); pos += 4; }
    pos = 0;
    while ((pos = s.find(L"xanh bien", pos)) != std::wstring::npos) { s.replace(pos, 9, L"blue"); pos += 4; }
    pos = 0;
    while ((pos = s.find(L"xanh", pos)) != std::wstring::npos) { s.replace(pos, 4, L"green"); pos += 5; }
    pos = 0;
    while ((pos = s.find(L"do", pos)) != std::wstring::npos) {
        bool before_ok = (pos == 0 || !iswalnum(s[pos - 1]));
        bool after_ok = (pos + 2 >= s.length() || !iswalnum(s[pos + 2]));
        if (before_ok && after_ok) { s.replace(pos, 2, L"red"); pos += 3; }
        else pos += 2;
    }
    pos = 0;
    while ((pos = s.find(L"vang", pos)) != std::wstring::npos) { s.replace(pos, 4, L"yellow"); pos += 6; }
    pos = 0;
    while ((pos = s.find(L"trang", pos)) != std::wstring::npos) { s.replace(pos, 5, L"white"); pos += 5; }
    pos = 0;
    while ((pos = s.find(L"cam", pos)) != std::wstring::npos) {
        bool before_ok = (pos == 0 || !iswalnum(s[pos - 1]));
        bool after_ok = (pos + 3 >= s.length() || !iswalnum(s[pos + 3]));
        if (before_ok && after_ok) { s.replace(pos, 3, L"orange"); pos += 6; }
        else pos += 3;
    }
    return s;
}

int matchFeederSlot(const std::wstring& comment, const std::wstring& footprint) {
    if (comment.empty() && footprint.empty()) return 0;

    std::wstring cmt_can = canonicalizeFeederKeywordsCpp(comment);
    std::wstring fp_can = canonicalizeFeederKeywordsCpp(footprint);

    std::vector<std::wstring> packages = {L"0402", L"0603", L"0805", L"1206", L"1210", L"sod-123", L"sod-323", L"sot-23", L"sop-8", L"sop-16", L"sma", L"smb", L"smc"};
    std::wstring comp_pkg = L"";
    for (const auto& pkg : packages) {
        if (cmt_can.find(pkg) != std::wstring::npos || fp_can.find(pkg) != std::wstring::npos) {
            comp_pkg = pkg;
            break;
        }
    }

    std::vector<std::wstring> colors = {L"red", L"green", L"blue", L"yellow", L"white", L"orange"};
    std::wstring comp_color = L"";
    for (const auto& clr : colors) {
        if (cmt_can.find(clr) != std::wstring::npos) {
            comp_color = clr;
            break;
        }
    }

    // 1. Nếu là LED hoặc linh kiện có màu và kích thước (VD: led do 0603 -> red + 0603 -> slot 34)
    if (!comp_color.empty()) {
        for (const auto& [slot, cfg] : g_feeder_matrix) {
            if (cfg.comment.empty()) continue;
            std::wstring f_can = canonicalizeFeederKeywordsCpp(cfg.comment);
            if (f_can.find(comp_color) != std::wstring::npos) {
                if (!comp_pkg.empty()) {
                    if (f_can.find(comp_pkg) != std::wstring::npos) return slot;
                } else {
                    return slot;
                }
            }
        }
    }

    // 2. Khớp tuyệt đối cả Comment & Footprint dạng "Value-Footprint" (VD: 1K-0603, 10K-0805)
    std::wstring full_pair = cmt_can + L"-" + fp_can;
    for (const auto& [slot, cfg] : g_feeder_matrix) {
        if (cfg.comment.empty()) continue;
        std::wstring f_can = canonicalizeFeederKeywordsCpp(cfg.comment);
        if (f_can == full_pair) return slot;

        size_t dash = f_can.find(L"-");
        if (dash != std::wstring::npos) {
            std::wstring val_p = f_can.substr(0, dash);
            std::wstring fp_p = f_can.substr(dash + 1);
            if ((val_p == cmt_can || (!val_p.empty() && cmt_can.find(val_p) != std::wstring::npos) || (!cmt_can.empty() && val_p.find(cmt_can) != std::wstring::npos)) &&
                (fp_p == fp_can || (!fp_p.empty() && fp_can.find(fp_p) != std::wstring::npos) || (!fp_can.empty() && fp_p.find(fp_can) != std::wstring::npos))) {
                return slot;
            }
        }
    }

    // 3. Khớp chính xác Comment
    for (const auto& [slot, cfg] : g_feeder_matrix) {
        if (cfg.comment.empty()) continue;
        std::wstring f_can = canonicalizeFeederKeywordsCpp(cfg.comment);
        if (f_can == cmt_can) return slot;
    }

    // 4. Khớp mờ / chứa Comment
    for (const auto& [slot, cfg] : g_feeder_matrix) {
        if (cfg.comment.empty()) continue;
        std::wstring f_can = canonicalizeFeederKeywordsCpp(cfg.comment);
        if (f_can.find(cmt_can) != std::wstring::npos || cmt_can.find(f_can) != std::wstring::npos) return slot;
    }
    return 0; // Trả về 0 nếu chưa có cấu hình khay Feeder phù hợp
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
        ListView_SetItemText(g_hListView, (int)i, 13, (LPWSTR)(c.skip ? L"1" : L"0"));
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

    // Bỏ qua UTF-8 BOM nếu có
    if (raw.size() >= 3 && (unsigned char)raw[0] == 0xEF && (unsigned char)raw[1] == 0xBB && (unsigned char)raw[2] == 0xBF) {
        raw = raw.substr(3);
    }

    std::wstring content = s2ws(raw);

    g_top_components.clear();
    g_bot_components.clear();

    std::wistringstream stream(content);
    std::wstring line;
    bool header_found = false;
    bool is_mil = false;

    int col_des = -1, col_cmt = -1, col_layer = -1, col_fp = -1;
    int col_x = -1, col_y = -1, col_rot = -1;
    int col_head = -1, col_feeder = -1, col_speed = -1, col_pick = -1, col_place = -1, col_mode = -1, col_skip = -1;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        if (line.empty()) continue;

        std::wstring line_l = toLower(line);
        if (line_l.find(L"units used: mil") != std::wstring::npos || line_l.find(L"unit: mil") != std::wstring::npos || line_l.find(L"(mil)") != std::wstring::npos) {
            is_mil = true;
        }

        if (!header_found) {
            auto cols = parseCsvLine(line);
            int temp_des = -1, temp_cmt = -1, temp_fp = -1, temp_x = -1, temp_y = -1, temp_rot = -1, temp_layer = -1;
            int temp_head = -1, temp_feeder = -1, temp_speed = -1, temp_pick = -1, temp_place = -1, temp_mode = -1, temp_skip = -1;

            for (size_t i = 0; i < cols.size(); ++i) {
                std::wstring k = cleanColName(cols[i]);
                std::wstring raw_lower = toLower(cols[i]);

                if (k == L"designator" || k == L"refdes" || k == L"ref" || k == L"reference" || k == L"part" || k == L"comp" || k == L"name" || k == L"tag" || k == L"item") {
                    if (temp_des == -1) temp_des = static_cast<int>(i);
                } else if (k == L"comment" || k == L"val" || k == L"value" || k == L"description" || k == L"device" || k == L"component") {
                    if (temp_cmt == -1) temp_cmt = static_cast<int>(i);
                } else if (k == L"footprint" || k == L"package" || k == L"pattern" || k == L"pkg" || k == L"fp") {
                    if (temp_fp == -1) temp_fp = static_cast<int>(i);
                } else if (k == L"midx" || k == L"centerx" || k == L"posx" || k == L"x" || k == L"midxmm" || k == L"centerxmm" || k == L"posxmm" || k == L"padx" || k == L"xmid") {
                    if (temp_x == -1) {
                        temp_x = static_cast<int>(i);
                        if (raw_lower.find(L"mil") != std::wstring::npos || raw_lower.find(L"inch") != std::wstring::npos) is_mil = true;
                    }
                } else if (k == L"midy" || k == L"centery" || k == L"posy" || k == L"y" || k == L"midymm" || k == L"centerymm" || k == L"posymm" || k == L"pady" || k == L"ymid") {
                    if (temp_y == -1) temp_y = static_cast<int>(i);
                } else if (k == L"rotation" || k == L"rot" || k == L"angle" || k == L"orientation" || k == L"rotate") {
                    if (temp_rot == -1) temp_rot = static_cast<int>(i);
                } else if (k == L"layer" || k == L"side" || k == L"face" || k == L"tb" || k == L"topbottom") {
                    if (temp_layer == -1) temp_layer = static_cast<int>(i);
                } else if (k == L"head" || k == L"nozzle" || k == L"headno") {
                    if (temp_head == -1) temp_head = static_cast<int>(i);
                } else if (k == L"feederno" || k == L"feeder" || k == L"slot" || k == L"stack") {
                    if (temp_feeder == -1) temp_feeder = static_cast<int>(i);
                } else if (k == L"mountspeed" || k == L"speed" || k == L"velocity") {
                    if (temp_speed == -1) temp_speed = static_cast<int>(i);
                } else if (k == L"pickheight" || k == L"pick" || k == L"pickheightmm") {
                    if (temp_pick == -1) temp_pick = static_cast<int>(i);
                } else if (k == L"placeheight" || k == L"place" || k == L"placeheightmm") {
                    if (temp_place == -1) temp_place = static_cast<int>(i);
                } else if (k == L"mode" || k == L"visionmode") {
                    if (temp_mode == -1) temp_mode = static_cast<int>(i);
                } else if (k == L"skip" || k == L"enable" || k == L"active") {
                    if (temp_skip == -1) temp_skip = static_cast<int>(i);
                }
            }

            if (temp_des != -1 && (temp_x != -1 || temp_cmt != -1 || temp_fp != -1)) {
                header_found = true;
                col_des = temp_des; col_cmt = temp_cmt; col_fp = temp_fp;
                col_x = temp_x; col_y = temp_y; col_rot = temp_rot; col_layer = temp_layer;
                col_head = temp_head; col_feeder = temp_feeder; col_speed = temp_speed;
                col_pick = temp_pick; col_place = temp_place; col_mode = temp_mode; col_skip = temp_skip;
            }
            continue;
        }

        auto fields = parseCsvLine(line);
        auto get_val = [&](int idx) -> std::wstring {
            if (idx >= 0 && idx < (int)fields.size()) return fields[idx];
            return L"";
        };

        std::wstring des = get_val(col_des);
        if (des.empty() || des[0] == L'*' || des[0] == L'#' || des[0] == L';') continue;
        if (cleanColName(des) == L"designator" || cleanColName(des) == L"refdes") continue;

        Component comp;
        comp.designator = des;
        comp.comment = normalizeComment(get_val(col_cmt));
        comp.footprint = normalizeFootprint(get_val(col_fp));

        std::wstring raw_layer = toLower(get_val(col_layer));
        if (raw_layer.find(L"bottom") != std::wstring::npos || raw_layer.find(L"bot") != std::wstring::npos || raw_layer == L"b" || raw_layer.find(L"back") != std::wstring::npos) {
            comp.layer = L"BottomLayer";
        } else {
            comp.layer = L"TopLayer";
        }

        try {
            std::wstring sx = get_val(col_x);
            std::wstring sy = get_val(col_y);
            std::wstring srot = get_val(col_rot);

            double rx = sx.empty() ? 0.0 : std::stod(sx);
            double ry = sy.empty() ? 0.0 : std::stod(sy);
            comp.rotation = srot.empty() ? 0.0 : std::stod(srot);
            comp.mid_x = is_mil ? rx * 0.0254 : rx;
            comp.mid_y = is_mil ? ry * 0.0254 : ry;
        } catch (...) {
            comp.mid_x = 0.0;
            comp.mid_y = 0.0;
            comp.rotation = 0.0;
        }

        if (col_feeder != -1 && !get_val(col_feeder).empty()) comp.feeder_no = _wtoi(get_val(col_feeder).c_str());
        else comp.feeder_no = g_auto_match_feeder ? matchFeederSlot(comp.comment, comp.footprint) : 0;

        if (col_head != -1 && !get_val(col_head).empty()) comp.head = _wtoi(get_val(col_head).c_str());
        else comp.head = 0;

        if (col_speed != -1 && !get_val(col_speed).empty()) comp.mount_speed = _wtoi(get_val(col_speed).c_str());
        else comp.mount_speed = 100;

        if (col_pick != -1 && !get_val(col_pick).empty()) comp.pick_height = _wtof(get_val(col_pick).c_str());
        else comp.pick_height = 0.0;

        if (col_place != -1 && !get_val(col_place).empty()) comp.place_height = _wtof(get_val(col_place).c_str());
        else comp.place_height = 0.0;

        if (col_mode != -1 && !get_val(col_mode).empty()) comp.mode = _wtoi(get_val(col_mode).c_str());
        else comp.mode = 1;

        if (col_skip != -1 && !get_val(col_skip).empty()) comp.skip = _wtoi(get_val(col_skip).c_str());
        else comp.skip = 0;

        if (comp.layer == L"BottomLayer") {
            g_bot_components.push_back(comp);
        } else {
            g_top_components.push_back(comp);
        }
    }

    if (!header_found || (g_top_components.empty() && g_bot_components.empty())) {
        MessageBoxW(g_hWnd, L"Không thể đọc linh kiện từ file! Vui lòng kiểm tra định dạng CSV/TXT.", L"Lỗi Định Dạng", MB_ICONERROR);
        return false;
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


// Quản lý Profile Cấu hình Feeder
static std::wstring g_active_profile = L"Mac_Dinh";
static wchar_t g_inputDlgBuffer[128] = {0};
static const wchar_t* g_inputDlgTitle = L"";
static const wchar_t* g_inputDlgPrompt = L"";

static LRESULT CALLBACK InputDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        if (wmId == IDOK) {
            GetDlgItemTextW(hWnd, 101, g_inputDlgBuffer, 128);
            DestroyWindow(hWnd);
        } else if (wmId == IDCANCEL) {
            g_inputDlgBuffer[0] = 0;
            DestroyWindow(hWnd);
        }
        break;
    }
    case WM_CLOSE:
        g_inputDlgBuffer[0] = 0;
        DestroyWindow(hWnd);
        break;
    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

static bool showInputBoxCpp(HWND parent, const wchar_t* title, const wchar_t* prompt, std::wstring& outStr) {
    g_inputDlgTitle = title;
    g_inputDlgPrompt = prompt;
    g_inputDlgBuffer[0] = 0;
    RECT pr;
    GetWindowRect(parent, &pr);
    int dlgW = 360, dlgH = 150;
    int dlgX = pr.left + (pr.right - pr.left - dlgW) / 2;
    int dlgY = pr.top + (pr.bottom - pr.top - dlgH) / 2;

    HWND hDlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"#32770", title, WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, dlgX, dlgY, dlgW, dlgH, parent, NULL, g_hInst, NULL);
    if (!hDlg) return false;
    SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LONG_PTR)InputDlgProc);

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

    if (g_inputDlgBuffer[0] != 0) {
        outStr = trim(g_inputDlgBuffer);
        return !outStr.empty();
    }
    return false;
}

static std::wstring getProfilesDirectoryCpp() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring s(exePath);
    size_t pos = s.find_last_of(L"\\/");
    std::wstring dir = (pos != std::wstring::npos) ? s.substr(0, pos) : L".";
    
    std::wstring p1 = dir + L"\\feeder_profiles";
    std::wstring p2 = dir + L"\\..\\feeder_profiles";
    
    DWORD dw1 = GetFileAttributesW(p1.c_str());
    if (dw1 != INVALID_FILE_ATTRIBUTES && (dw1 & FILE_ATTRIBUTE_DIRECTORY)) return p1;
    
    DWORD dw2 = GetFileAttributesW(p2.c_str());
    if (dw2 != INVALID_FILE_ATTRIBUTES && (dw2 & FILE_ATTRIBUTE_DIRECTORY)) return p2;
    
    CreateDirectoryW(p1.c_str(), NULL);
    return p1;
}

static void saveProfileToDiskCpp(const std::wstring& profName, const std::map<int, std::wstring>& feeders) {
    std::wstring pDir = getProfilesDirectoryCpp();
    std::wstring path = pDir + L"\\" + profName + L".json";
    std::ofstream out(ws2s(path));
    if (out.is_open()) {
        out << "{\n  \"profile_name\": \"" << ws2s(profName) << "\",\n  \"feeders\": {\n";
        bool first = true;
        for (int i = 1; i <= 50; ++i) {
            auto it = feeders.find(i);
            std::string val = (it != feeders.end()) ? ws2s(it->second) : "";
            if (!first) out << ",\n";
            out << "    \"" << i << "\": \"" << val << "\"";
            first = false;
        }
        out << "\n  }\n}\n";
        out.close();
    }

    // Ghi đồng bộ feeder_matrix.json
    std::ofstream outM("feeder_matrix.json");
    if (outM.is_open()) {
        outM << "{\n  \"profile_name\": \"" << ws2s(profName) << "\",\n  \"feeders\": {\n";
        bool first = true;
        for (int i = 1; i <= 50; ++i) {
            auto it = feeders.find(i);
            std::string val = (it != feeders.end()) ? ws2s(it->second) : "";
            if (!first) outM << ",\n";
            outM << "    \"" << i << "\": \"" << val << "\"";
            first = false;
        }
        outM << "\n  }\n}\n";
        outM.close();
    }
}

static std::vector<std::wstring> listFeederProfilesCpp() {
    std::wstring pDir = getProfilesDirectoryCpp();
    
    // Đảm bảo Mac_Dinh.json luôn luôn tồn tại
    std::wstring macDinhPath = pDir + L"\\Mac_Dinh.json";
    DWORD dw = GetFileAttributesW(macDinhPath.c_str());
    if (dw == INVALID_FILE_ATTRIBUTES) {
        std::map<int, std::wstring> defFd;
        for (int i = 1; i <= 50; ++i) defFd[i] = g_feeder_matrix[i].comment;
        saveProfileToDiskCpp(L"Mac_Dinh", defFd);
    }

    std::vector<std::wstring> list;
    WIN32_FIND_DATAW ffd;
    std::wstring searchPattern = pDir + L"\\*.json";
    HANDLE hFind = FindFirstFileW(searchPattern.c_str(), &ffd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::wstring fn = ffd.cFileName;
                size_t p = fn.rfind(L".json");
                if (p != std::wstring::npos) {
                    std::wstring stem = fn.substr(0, p);
                    if (stem != L"Mac_Dinh") {
                        list.push_back(stem);
                    }
                }
            }
        } while (FindNextFileW(hFind, &ffd) != 0);
        FindClose(hFind);
    }
    std::sort(list.begin(), list.end());
    list.insert(list.begin(), L"Mac_Dinh");
    return list;
}

static void loadProfileFromDiskCpp(const std::wstring& profName, std::map<int, std::wstring>& feeders) {
    for (int i = 1; i <= 50; ++i) feeders[i] = L"";
    std::wstring path = getProfilesDirectoryCpp() + L"\\" + profName + L".json";
    std::ifstream in(ws2s(path));
    if (!in.is_open()) return;

    std::string line;
    while (std::getline(in, line)) {
        size_t q1 = line.find("\"");
        if (q1 == std::string::npos) continue;
        size_t q2 = line.find("\"", q1 + 1);
        if (q2 == std::string::npos) continue;
        std::string key = line.substr(q1 + 1, q2 - q1 - 1);
        int slot = atoi(key.c_str());
        if (slot >= 1 && slot <= 50) {
            size_t colon = line.find(":", q2);
            if (colon != std::string::npos) {
                size_t vq1 = line.find("\"", colon);
                if (vq1 != std::string::npos) {
                    size_t vq2 = line.find("\"", vq1 + 1);
                    if (vq2 != std::string::npos) {
                        std::string val = line.substr(vq1 + 1, vq2 - vq1 - 1);
                        feeders[slot] = s2ws(val);
                    }
                }
            }
        }
    }
}

// Helper vẽ 1 hàng Khay Feeder gồm Label Số Khay và Edit Box Nhập Trị Số gọn gàng
static void createFeederSlotControl(HWND hParent, int slot, int x, int y, const std::wstring& val) {
    wchar_t lblText[32];
    swprintf(lblText, 32, L"#%02d:", slot);
    HWND hLbl = CreateWindowExW(0, L"STATIC", lblText, WS_CHILD | WS_VISIBLE | SS_RIGHT, x, y + 2, 45, 18, hParent, NULL, g_hInst, NULL);
    SendMessageW(hLbl, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hEd = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", val.c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, x + 50, y, 220, 21, hParent, (HMENU)(INT_PTR)(5000 + slot), g_hInst, NULL);
    SendMessageW(hEd, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
}

static void updateDialogFromMapCpp(HWND hWnd, const std::map<int, std::wstring>& feeders) {
    for (int slot = 1; slot <= 50; ++slot) {
        HWND hEd = GetDlgItem(hWnd, 5000 + slot);
        if (hEd) {
            auto it = feeders.find(slot);
            SetWindowTextW(hEd, (it != feeders.end()) ? it->second.c_str() : L"");
        }
    }
}

static std::map<int, std::wstring> getDialogFeedersCpp(HWND hWnd) {
    std::map<int, std::wstring> res;
    for (int slot = 1; slot <= 50; ++slot) {
        HWND hEd = GetDlgItem(hWnd, 5000 + slot);
        if (hEd) {
            wchar_t buf[128] = {0};
            GetWindowTextW(hEd, buf, 128);
            res[slot] = buf;
        } else {
            res[slot] = L"";
        }
    }
    return res;
}

static HBRUSH g_hBrushDarkDlg = CreateSolidBrush(RGB(15, 23, 42)); // #0F172A
static HBRUSH g_hBrushEditDark = CreateSolidBrush(RGB(15, 23, 42)); // #0F172A
static HWND g_hLblActiveProfile = NULL;

static std::wstring g_dialog_profile = L"Mac_Dinh";

void refreshActiveProfileLabelCpp() {
    if (g_hLblActiveProfile) {
        std::wstring profVal = g_active_profile.empty() ? L"Mac_Dinh" : g_active_profile;
        std::wstring text = L"⚙️ Quy tắc đang áp dụng: [" + profVal + L"]";
        SetWindowTextW(g_hLblActiveProfile, text.c_str());
        InvalidateRect(g_hLblActiveProfile, NULL, TRUE);
        UpdateWindow(g_hLblActiveProfile);
    }
}

// Feeder Dialog Proc
LRESULT CALLBACK FeederDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CTLCOLORDLG:
        return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, TRANSPARENT);
        HWND hCtl = (HWND)lParam;
        int id = GetDlgCtrlID(hCtl);
        if (id >= 5001 && id <= 5050) {
            SetTextColor(hdc, RGB(30, 41, 59)); // #1E293B (chữ tối rõ nét, nền trong suốt)
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

        if (wmId == 3001 && wmEvent == CBN_SELCHANGE) { // Chọn profile để xem/chỉnh sửa trong hộp thoại (chưa áp dụng ra ngoài)
            HWND hCb = GetDlgItem(hWnd, 3001);
            int idx = (int)SendMessageW(hCb, CB_GETCURSEL, 0, 0);
            if (idx != CB_ERR) {
                wchar_t pName[128] = {0};
                SendMessageW(hCb, CB_GETLBTEXT, idx, (LPARAM)pName);
                g_dialog_profile = pName;
                std::map<int, std::wstring> fd;
                loadProfileFromDiskCpp(g_dialog_profile, fd);
                updateDialogFromMapCpp(hWnd, fd);
            }
        } else if (wmId == 3002) { // + Tạo Mới Profile
            std::wstring newName;
            if (showInputBoxCpp(hWnd, L"Tạo Cấu Hình Mới", L"Nhập tên cấu hình mới (VD: Bo_Mach_A):", newName)) {
                auto fd = getDialogFeedersCpp(hWnd);
                saveProfileToDiskCpp(newName, fd);
                g_dialog_profile = newName;
                
                HWND hCb = GetDlgItem(hWnd, 3001);
                SendMessageW(hCb, CB_RESETCONTENT, 0, 0);
                auto plist = listFeederProfilesCpp();
                int selIdx = 0;
                for (size_t i = 0; i < plist.size(); ++i) {
                    SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)plist[i].c_str());
                    if (plist[i] == g_dialog_profile) selIdx = (int)i;
                }
                SendMessageW(hCb, CB_SETCURSEL, selIdx, 0);
                MessageBoxW(hWnd, (L"🎉 Đã tạo cấu hình mới [" + newName + L"] thành công! Nhấn 'LƯU VÀ ÁP DỤNG NGAY' nếu muốn áp dụng cho mạch.").c_str(), L"Thành Công", MB_ICONINFORMATION);
            }
        } else if (wmId == 3003) { // Lưu Profile
            auto fd = getDialogFeedersCpp(hWnd);
            saveProfileToDiskCpp(g_dialog_profile, fd);
            MessageBoxW(hWnd, (L"💾 Đã lưu cấu hình [" + g_dialog_profile + L"] vào bộ nhớ thành công!").c_str(), L"Thành Công", MB_ICONINFORMATION);
        } else if (wmId == 3004) { // Lưu Thành (Save As)
            std::wstring newName;
            if (showInputBoxCpp(hWnd, L"Lưu Thành Cấu Hình Khác", L"Nhập tên cấu hình mới:", newName)) {
                auto fd = getDialogFeedersCpp(hWnd);
                saveProfileToDiskCpp(newName, fd);
                g_dialog_profile = newName;
                
                HWND hCb = GetDlgItem(hWnd, 3001);
                SendMessageW(hCb, CB_RESETCONTENT, 0, 0);
                auto plist = listFeederProfilesCpp();
                int selIdx = 0;
                for (size_t i = 0; i < plist.size(); ++i) {
                    SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)plist[i].c_str());
                    if (plist[i] == g_dialog_profile) selIdx = (int)i;
                }
                SendMessageW(hCb, CB_SETCURSEL, selIdx, 0);
                MessageBoxW(hWnd, (L"🎉 Đã lưu thành cấu hình [" + newName + L"] thành công!").c_str(), L"Thành Công", MB_ICONINFORMATION);
            }
        } else if (wmId == 3005) { // Xóa Profile
            if (g_dialog_profile == L"Mac_Dinh" || g_dialog_profile == L"Mặc Định" || g_dialog_profile.empty()) {
                MessageBoxW(hWnd, L"Cấu hình mặc định [Mac_Dinh] là cấu hình gốc của máy và không thể xóa!", L"Thông Báo", MB_ICONWARNING);
                break;
            }
            if (MessageBoxW(hWnd, (L"Bạn có chắc muốn xóa vĩnh viễn cấu hình [" + g_dialog_profile + L"]?").c_str(), L"Xác Nhận Xóa", MB_ICONQUESTION | MB_YESNO) == IDYES) {
                std::wstring delPath = getProfilesDirectoryCpp() + L"\\" + g_dialog_profile + L".json";
                DeleteFileW(delPath.c_str());
                g_dialog_profile = L"Mac_Dinh";
                HWND hCb = GetDlgItem(hWnd, 3001);
                SendMessageW(hCb, CB_RESETCONTENT, 0, 0);
                auto plist = listFeederProfilesCpp();
                int selIdx = 0;
                for (size_t i = 0; i < plist.size(); ++i) {
                    SendMessageW(hCb, CB_ADDSTRING, 0, (LPARAM)plist[i].c_str());
                    if (plist[i] == g_dialog_profile) selIdx = (int)i;
                }
                SendMessageW(hCb, CB_SETCURSEL, selIdx, 0);
                std::map<int, std::wstring> fd;
                loadProfileFromDiskCpp(g_dialog_profile, fd);
                updateDialogFromMapCpp(hWnd, fd);
                MessageBoxW(hWnd, L"Đã xóa cấu hình! Đã chuyển về xem [Mac_Dinh].", L"Thành Công", MB_ICONINFORMATION);
            }
        } else if (wmId == IDOK || wmId == 2001) { // LƯU & ÁP DỤNG
            g_active_profile = g_dialog_profile;
            auto fd = getDialogFeedersCpp(hWnd);
            saveProfileToDiskCpp(g_active_profile, fd);

            for (int slot = 1; slot <= 50; ++slot) {
                g_feeder_matrix[slot].comment = fd[slot];
            }

            // Áp dụng lại số khay cho toàn bộ linh kiện trên bảng nếu đang bật tự động nhận diện
            if (g_auto_match_feeder) {
                for (auto& c : g_top_components) c.feeder_no = matchFeederSlot(c.comment, c.footprint);
                for (auto& c : g_bot_components) c.feeder_no = matchFeederSlot(c.comment, c.footprint);
            }
            refreshListView();
            refreshActiveProfileLabelCpp();

            MessageBoxW(hWnd, (L"🎉 Đã áp dụng bộ quy tắc [" + g_active_profile + L"] cho toàn bộ bảng linh kiện!").c_str(), L"Thành Công", MB_ICONINFORMATION);
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

void openFeederMatrixDialog(HWND parent) {
    g_dialog_profile = g_active_profile.empty() ? L"Mac_Dinh" : g_active_profile;

    RECT pr;
    GetWindowRect(parent, &pr);
    int dlgW = 675, dlgH = 765;
    int dlgX = pr.left + (pr.right - pr.left - dlgW) / 2;
    int dlgY = pr.top + (pr.bottom - pr.top - dlgH) / 2;

    HWND hDlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"#32770", L"Cấu Hình 50 Khay Feeder 4 Góc (NeoDen YY1)",
                               WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
                               dlgX, dlgY, dlgW, dlgH, parent, NULL, g_hInst, NULL);
    if (!hDlg) return;

    SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LONG_PTR)FeederDlgProc);

    // Profile Management Bar ở trên cùng
    HWND hLblProf = CreateWindowExW(0, L"STATIC", L"Cấu Hình:", WS_CHILD | WS_VISIBLE | SS_RIGHT, 15, 14, 85, 20, hDlg, NULL, g_hInst, NULL);
    SendMessageW(hLblProf, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hCbProf = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 105, 11, 165, 350, hDlg, (HMENU)3001, g_hInst, NULL);
    SendMessageW(hCbProf, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

    auto plist = listFeederProfilesCpp();
    int selIdx = 0;
    for (size_t i = 0; i < plist.size(); ++i) {
        SendMessageW(hCbProf, CB_ADDSTRING, 0, (LPARAM)plist[i].c_str());
        if (plist[i] == g_active_profile) selIdx = (int)i;
    }
    SendMessageW(hCbProf, CB_SETCURSEL, selIdx, 0);

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
        createFeederSlotControl(hDlg, slot, 25, 68 + i * 22, g_feeder_matrix[slot].comment);
    }

    // 2. Góc Trên Phải: Khay 40..50 (#40 ở dưới cùng, #50 ở trên cùng)
    for (int i = 0; i < 11; ++i) {
        int slot = 50 - i;
        createFeederSlotControl(hDlg, slot, 345, 68 + i * 22, g_feeder_matrix[slot].comment);
    }

    // 3. Góc Dưới Trái: Khay 1..13 (#01 ở dưới cùng, #13 ở trên cùng)
    for (int i = 0; i < 13; ++i) {
        int slot = 13 - i;
        createFeederSlotControl(hDlg, slot, 25, 352 + i * 23, g_feeder_matrix[slot].comment);
    }

    // 4. Góc Dưới Phải: Khay 30..39 (#30 ở dưới cùng, #39 ở trên cùng)
    for (int i = 0; i < 10; ++i) {
        int slot = 39 - i;
        createFeederSlotControl(hDlg, slot, 345, 352 + i * 23, g_feeder_matrix[slot].comment);
    }

    // Nút Lưu / Đóng ở dưới (Đã bỏ nút Mặc Định)
    HWND hBtnSave = CreateWindowExW(0, L"BUTTON", L"LƯU VÀ ÁP DỤNG NGAY", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 440, 675, 200, 36, hDlg, (HMENU)2001, g_hInst, NULL);
    SendMessageW(hBtnSave, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

    HWND hBtnCancel = CreateWindowExW(0, L"BUTTON", L"Đóng", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 25, 675, 90, 36, hDlg, (HMENU)IDCANCEL, g_hInst, NULL);
    SendMessageW(hBtnCancel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
}

// In-Place Cell Editing trực tiếp trên từng ô
static WNDPROC g_oldEditProc = NULL;
static int g_inPlaceItem = -1;
static int g_inPlaceSubItem = -1;
static HWND g_hInPlaceEdit = NULL;

void commitInPlaceEdit(bool save) {
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
        std::wstring s = buf;
        auto& list = g_showing_top ? g_top_components : g_bot_components;
        if (item >= 0 && item < (int)list.size()) {
            auto& c = list[item];
            switch (subItem) {
                case 1: c.designator = s; break;
                case 2: c.comment = s; break;
                case 3: c.footprint = s; break;
                case 4: try { c.mid_x = std::stod(s); } catch(...) {} break;
                case 5: try { c.mid_y = std::stod(s); } catch(...) {} break;
                case 6: try { c.rotation = std::stod(s); } catch(...) {} break;
                case 7: try { c.head = std::stoi(s); } catch(...) {} break;
                case 8: try { c.feeder_no = std::stoi(s); } catch(...) {} break;
                case 9: try { c.mount_speed = std::stoi(s); } catch(...) {} break;
                case 10: try { c.pick_height = std::stod(s); } catch(...) {} break;
                case 11: try { c.place_height = std::stod(s); } catch(...) {} break;
                case 12: try { c.mode = std::stoi(s); } catch(...) {} break;
            }
        }
        refreshListView();
    }
    DestroyWindow(hEdit);
}

LRESULT CALLBACK InPlaceEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_KILLFOCUS:
        commitInPlaceEdit(true);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_RETURN) {
            commitInPlaceEdit(true);
            return 0;
        } else if (wParam == VK_ESCAPE) {
            commitInPlaceEdit(false);
            return 0;
        }
        break;
    case WM_CHAR:
        if (wParam == VK_RETURN || wParam == VK_ESCAPE) return 0;
        break;
    }
    return CallWindowProc(g_oldEditProc, hWnd, message, wParam, lParam);
}

void startInPlaceEdit(int item, int subItem) {
    if (item < 0 || subItem < 1 || subItem > 12) return;
    commitInPlaceEdit(true);

    RECT rc;
    ListView_GetSubItemRect(g_hListView, item, subItem, LVIR_BOUNDS, &rc);

    wchar_t cur[256] = {0};
    ListView_GetItemText(g_hListView, item, subItem, cur, 256);

    g_inPlaceItem = item;
    g_inPlaceSubItem = subItem;

    g_hInPlaceEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", cur,
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

        g_hWnd = hWnd;

        // Nút Mở Ma Trận Feeder 4 Góc & Nhãn Profile Nằm Ngay Dưới Nút
        HWND hBtnFeeder = CreateWindowExW(0, L"BUTTON", L"⚙️ CẤU HÌNH KHAY FEEDER 4 GÓC", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1050, 8, 290, 32, hWnd, (HMENU)301, g_hInst, NULL);
        SendMessageW(hBtnFeeder, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        std::wstring initProfText = L"⚙️ Quy tắc đang áp dụng: [" + (g_active_profile.empty() ? L"Mac_Dinh" : g_active_profile) + L"]";
        g_hLblActiveProfile = CreateWindowExW(0, L"STATIC", initProfText.c_str(), WS_CHILD | WS_VISIBLE | SS_CENTER, 1050, 42, 290, 22, hWnd, (HMENU)404, g_hInst, NULL);
        SendMessageW(g_hLblActiveProfile, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

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
        g_hRadioTop = CreateWindowExW(0, L"BUTTON", L"Mat TOP", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 35, 180, 95, 24, hWnd, (HMENU)401, g_hInst, NULL);
        SendMessageW(g_hRadioTop, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
        SendMessageW(g_hRadioTop, BM_SETCHECK, BST_CHECKED, 0);

        g_hRadioBot = CreateWindowExW(0, L"BUTTON", L"Mat BOTTOM", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 135, 180, 110, 24, hWnd, (HMENU)402, g_hInst, NULL);
        SendMessageW(g_hRadioBot, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        // Checkbox Tự động nhận diện Feeder theo cấu hình
        g_hChkAutoMatch = CreateWindowExW(0, L"BUTTON", L"☑ Tự động nhận diện Feeder theo Cấu hình", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 255, 180, 310, 24, hWnd, (HMENU)302, g_hInst, NULL);
        SendMessageW(g_hChkAutoMatch, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
        SendMessageW(g_hChkAutoMatch, BM_SETCHECK, g_auto_match_feeder ? BST_CHECKED : BST_UNCHECKED, 0);

        g_hStatus = CreateWindowExW(0, L"STATIC", L"Chua chon file CAD nao", WS_CHILD | WS_VISIBLE, 575, 183, 750, 20, hWnd, (HMENU)104, g_hInst, NULL);
        SendMessageW(g_hStatus, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hListView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL, 35, 210, 1290, 390, hWnd, (HMENU)105, g_hInst, NULL);
        SendMessageW(g_hListView, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
        ListView_SetExtendedListViewStyle(g_hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // Du 13 cot cua NeoDen YY1 (Tong 1286px, them 2px vua khit 100% mep phai)
        LVCOLUMNW col = {0};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
        
        col.fmt = LVCFMT_CENTER; col.cx = 42;  col.pszText = (LPWSTR)L"STT";         ListView_InsertColumn(g_hListView, 0, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 96;  col.pszText = (LPWSTR)L"Designator";  ListView_InsertColumn(g_hListView, 1, &col);
        col.fmt = LVCFMT_LEFT;   col.cx = 208; col.pszText = (LPWSTR)L"Comment";     ListView_InsertColumn(g_hListView, 2, &col);
        col.fmt = LVCFMT_LEFT;   col.cx = 162; col.pszText = (LPWSTR)L"Footprint";   ListView_InsertColumn(g_hListView, 3, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 90;  col.pszText = (LPWSTR)L"Mid X";       ListView_InsertColumn(g_hListView, 4, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 90;  col.pszText = (LPWSTR)L"Mid Y";       ListView_InsertColumn(g_hListView, 5, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 80;  col.pszText = (LPWSTR)L"Rotation";   ListView_InsertColumn(g_hListView, 6, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 55;  col.pszText = (LPWSTR)L"Head";       ListView_InsertColumn(g_hListView, 7, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 78;  col.pszText = (LPWSTR)L"FeederNo";   ListView_InsertColumn(g_hListView, 8, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 78;  col.pszText = (LPWSTR)L"Speed%";     ListView_InsertColumn(g_hListView, 9, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 88;  col.pszText = (LPWSTR)L"Pick(mm)";   ListView_InsertColumn(g_hListView, 10, &col);
        col.fmt = LVCFMT_RIGHT;  col.cx = 88;  col.pszText = (LPWSTR)L"Place(mm)";  ListView_InsertColumn(g_hListView, 11, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 56;  col.pszText = (LPWSTR)L"Mode";       ListView_InsertColumn(g_hListView, 12, &col);
        col.fmt = LVCFMT_CENTER; col.cx = 48;  col.pszText = (LPWSTR)L"Skip";       ListView_InsertColumn(g_hListView, 13, &col);

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
            if (g_hLblActiveProfile) {
                SetWindowPos(g_hLblActiveProfile, NULL, rightX, 42, 290, 22, SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
        break;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        HWND hwndStatic = (HWND)lParam;
        if (hwndStatic == g_hLblActiveProfile) {
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
                    const auto& list = g_showing_top ? g_top_components : g_bot_components;
                    if (item >= 0 && item < (int)list.size()) {
                        if (subItem == 8) { // Cột FeederNo
                            if (list[item].feeder_no == 0) {
                                // CHƯA CÓ FEEDER / CHƯA NHẬN DIỆN ĐƯỢC (0): Khối Màu Đỏ Nổi Bật Cảnh Báo, Chữ Trắng
                                lplvcd->nmcd.uItemState &= ~(CDIS_SELECTED | CDIS_FOCUS | CDIS_HOT);
                                lplvcd->clrTextBk = RGB(220, 38, 38);
                                lplvcd->clrText = RGB(255, 255, 255);
                            }
                        } else if (subItem == 13) { // Cột Skip
                            lplvcd->nmcd.uItemState &= ~(CDIS_SELECTED | CDIS_FOCUS | CDIS_HOT);
                            if (list[item].skip != 0) {
                                // BẬT SKIP (1): Khối Màu Đỏ Nổi Bật, Chữ Trắng (Bỏ Qua)
                                lplvcd->clrTextBk = RGB(220, 38, 38);
                                lplvcd->clrText = RGB(255, 255, 255);
                            } else {
                                // MẶC ĐỊNH (0): Khối Màu Xanh Lá Cây Đẹp (Gắp Linh Kiện)
                                lplvcd->clrTextBk = RGB(22, 163, 74); // #16A34A (Màu Xanh Lá Cây)
                                lplvcd->clrText = RGB(255, 255, 255);
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
                        ListView_SetItemState(g_hListView, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
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
                            ListView_SetItemState(g_hListView, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
                        }
                    } else if (pia->iSubItem >= 1 && pia->iSubItem <= 12) {
                        startInPlaceEdit(pia->iItem, pia->iSubItem);
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
        case 302: { // Checkbox Tự động nhận diện Feeder
            g_auto_match_feeder = (SendMessageW(g_hChkAutoMatch, BM_GETCHECK, 0, 0) == BST_CHECKED);
            if (g_auto_match_feeder) {
                for (auto& c : g_top_components) c.feeder_no = matchFeederSlot(c.comment, c.footprint);
                for (auto& c : g_bot_components) c.feeder_no = matchFeederSlot(c.comment, c.footprint);
                refreshListView();
            }
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
            int sel = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
            if (sel >= 0) {
                startInPlaceEdit(sel, 8);
            }
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
