#include "converter.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

// Template header chuẩn tích hợp mặc định từ 0603Demo.csv
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

// Màu sắc ANSI 24-bit TrueColor
static std::string rgb(int r, int g, int b, const std::string& text) {
    return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m" + text + "\033[0m";
}

void NeoDenConverter::printIntroBanner() {
    std::cout << "\n";
    std::cout << "      " << rgb(255, 120, 0,  "       ████████████◣           ████████████◣\n");
    std::cout << "      " << rgb(255, 80, 40,   "        ██████████████◣         ██████████████◣\n");
    std::cout << "  " << rgb(255, 140, 0, "■") << "   " << rgb(255, 40, 90,   "         ██████  ████████◣       ██████  ████████◣\n");
    std::cout << "    " << rgb(255, 20, 100, "■") << " " << rgb(180, 40, 240, "■") << " " << rgb(220, 10, 130, "        ██████    ████████◣     ██████    ████████◣\n");
    std::cout << "  " << rgb(255, 0, 120, "■") << "   " << rgb(30, 120, 255, "■") << " " << rgb(160, 30, 220, "       ██████      ████████◣   ██████      ████████◣\n");
    std::cout << "    " << rgb(40, 220, 80, "■") << "   " << rgb(120, 20, 240, "      ██████        ████████◣ ██████        ████████◣\n");
    std::cout << "      " << rgb(80, 10, 230,   "     ██████        ████████◤ ██████        ████████◤\n");
    std::cout << "      " << rgb(40, 60, 240,   "    ██████        ████████◤ ██████        ████████◤\n");
    std::cout << "      " << rgb(0, 120, 255,   "   ██████        ████████◤ ██████        ████████◤\n");
    std::cout << "      " << rgb(0, 180, 255,   "  ██████████████████████◤ ██████████████████████◤\n");
    std::cout << "      " << rgb(0, 230, 220,   " ████████████████████◤   ████████████████████◤\n\n");

    std::cout << "  ╭───────────────────────────────────────────────────────────────────────╮\n";
    std::cout << "  │     \033[48;2;20;24;40m\033[38;2;0;230;255m   NEODEN YY1 SMT PICK & PLACE AUTOMATED CONVERTER  [C++ NATIVE]   \033[0m    │\n";
    std::cout << "  │  " << rgb(180, 190, 210, "Phiên bản: 2.0 Pro • Chuẩn mẫu tự động: 0603Demo.csv") << "                     │\n";
    std::cout << "  │  " << rgb(140, 150, 170, "Tự động phân tách Layer | Tự động gán Feeder | Căn chỉnh tọa độ") << "               │\n";
    std::cout << "  ╰───────────────────────────────────────────────────────────────────────╯\n\n";
}

NeoDenConverter::NeoDenConverter() {
    loadTemplate();
}

void NeoDenConverter::loadTemplate() {
    templateHeader_ = EMBEDDED_HEADER;
    headPatterns_.clear();

    std::vector<std::string> searchPaths = {"0603Demo.csv", "../0603Demo.csv"};
    for (const auto& p : searchPaths) {
        if (fs::exists(p)) {
            std::ifstream f(p, std::ios::binary);
            if (f.is_open()) {
                std::stringstream buffer;
                buffer << f.rdbuf();
                std::string content = buffer.str();
                
                std::istringstream stream(content);
                std::string line;
                std::string headerAcc;
                bool found = false;
                while (std::getline(stream, line)) {
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    if (!found) {
                        headerAcc += line + "\r\n";
                        if (line.find("Designator") != std::string::npos && line.find("Comment") != std::string::npos) {
                            found = true;
                        }
                    } else if (!line.empty()) {
                        auto fields = parseCsvLine(line);
                        if (fields.size() >= 7) {
                            headPatterns_.push_back(fields[6]);
                        }
                    }
                }
                if (found) {
                    templateHeader_ = headerAcc;
                    break;
                }
            }
        }
    }

    if (headPatterns_.empty()) {
        headPatterns_.push_back("0");
    }
}

std::string NeoDenConverter::trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\r\n\"");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n\"");
    return s.substr(first, (last - first + 1));
}

std::string NeoDenConverter::toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

std::pair<std::string, int> NeoDenConverter::extractPrefixAndNumber(const std::string& s) {
    std::string prefix = "";
    std::string num_str = "";
    bool in_num = false;

    for (char ch : s) {
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            in_num = true;
            num_str += ch;
        } else if (!in_num) {
            prefix += ch;
        }
    }
    int num = num_str.empty() ? 0 : std::stoi(num_str);
    return {prefix, num};
}

static std::string cleanColName(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    }
    return out;
}

std::vector<std::string> NeoDenConverter::parseCsvLine(const std::string& line) {
    char delimiter = ',';
    int commas = 0, semis = 0, tabs = 0;
    bool in_q = false;
    for (char ch : line) {
        if (ch == '"') in_q = !in_q;
        else if (!in_q) {
            if (ch == ',') commas++;
            else if (ch == ';') semis++;
            else if (ch == '\t') tabs++;
        }
    }
    if (tabs > commas && tabs > semis) delimiter = '\t';
    else if (semis > commas && semis > tabs) delimiter = ';';

    std::vector<std::string> fields;
    std::string current = "";
    bool in_quotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char ch = line[i];
        if (ch == '"') {
            if (in_quotes && i + 1 < line.length() && line[i + 1] == '"') {
                current += '"';
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

bool NeoDenConverter::readAltiumFile(const std::string& filepath, std::string& errorMsg) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        errorMsg = "Không thể mở file: " + filepath;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Bỏ qua UTF-8 BOM nếu có
    if (content.size() >= 3 && (unsigned char)content[0] == 0xEF && (unsigned char)content[1] == 0xBB && (unsigned char)content[2] == 0xBF) {
        content = content.substr(3);
    }

    components_.clear();
    std::istringstream stream(content);
    std::string line;
    bool header_found = false;
    bool is_mil = false;

    int col_des = -1, col_cmt = -1, col_layer = -1, col_fp = -1;
    int col_x = -1, col_y = -1, col_rot = -1;
    int col_head = -1, col_feeder = -1, col_speed = -1, col_pick = -1, col_place = -1, col_mode = -1, col_skip = -1;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        std::string line_l = toLower(line);
        if (line_l.find("units used: mil") != std::string::npos || line_l.find("unit: mil") != std::string::npos || line_l.find("(mil)") != std::string::npos) {
            is_mil = true;
        }

        if (!header_found) {
            auto cols = parseCsvLine(line);
            int temp_des = -1, temp_cmt = -1, temp_fp = -1, temp_x = -1, temp_y = -1, temp_rot = -1, temp_layer = -1;
            int temp_head = -1, temp_feeder = -1, temp_speed = -1, temp_pick = -1, temp_place = -1, temp_mode = -1, temp_skip = -1;

            for (size_t i = 0; i < cols.size(); ++i) {
                std::string k = cleanColName(cols[i]);
                std::string raw_lower = toLower(cols[i]);

                if (k == "designator" || k == "refdes" || k == "ref" || k == "reference" || k == "part" || k == "comp" || k == "name" || k == "tag" || k == "item") {
                    if (temp_des == -1) temp_des = static_cast<int>(i);
                } else if (k == "comment" || k == "val" || k == "value" || k == "description" || k == "device" || k == "component") {
                    if (temp_cmt == -1) temp_cmt = static_cast<int>(i);
                } else if (k == "footprint" || k == "package" || k == "pattern" || k == "pkg" || k == "fp") {
                    if (temp_fp == -1) temp_fp = static_cast<int>(i);
                } else if (k == "midx" || k == "centerx" || k == "posx" || k == "x" || k == "midxmm" || k == "centerxmm" || k == "posxmm" || k == "padx" || k == "xmid") {
                    if (temp_x == -1) {
                        temp_x = static_cast<int>(i);
                        if (raw_lower.find("mil") != std::string::npos || raw_lower.find("inch") != std::string::npos) is_mil = true;
                    }
                } else if (k == "midy" || k == "centery" || k == "posy" || k == "y" || k == "midymm" || k == "centerymm" || k == "posymm" || k == "pady" || k == "ymid") {
                    if (temp_y == -1) temp_y = static_cast<int>(i);
                } else if (k == "rotation" || k == "rot" || k == "angle" || k == "orientation" || k == "rotate") {
                    if (temp_rot == -1) temp_rot = static_cast<int>(i);
                } else if (k == "layer" || k == "side" || k == "face" || k == "tb" || k == "topbottom") {
                    if (temp_layer == -1) temp_layer = static_cast<int>(i);
                } else if (k == "head" || k == "nozzle" || k == "headno") {
                    if (temp_head == -1) temp_head = static_cast<int>(i);
                } else if (k == "feederno" || k == "feeder" || k == "slot" || k == "stack") {
                    if (temp_feeder == -1) temp_feeder = static_cast<int>(i);
                } else if (k == "mountspeed" || k == "speed" || k == "velocity") {
                    if (temp_speed == -1) temp_speed = static_cast<int>(i);
                } else if (k == "pickheight" || k == "pick" || k == "pickheightmm") {
                    if (temp_pick == -1) temp_pick = static_cast<int>(i);
                } else if (k == "placeheight" || k == "place" || k == "placeheightmm") {
                    if (temp_place == -1) temp_place = static_cast<int>(i);
                } else if (k == "mode" || k == "visionmode") {
                    if (temp_mode == -1) temp_mode = static_cast<int>(i);
                } else if (k == "skip" || k == "enable" || k == "active") {
                    if (temp_skip == -1) temp_skip = static_cast<int>(i);
                }
            }

            // Tiêu chuẩn nhận diện dòng Header linh hoạt
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
        auto get_val = [&](int idx) -> std::string {
            if (idx >= 0 && idx < static_cast<int>(fields.size())) return fields[idx];
            return "";
        };

        std::string des = get_val(col_des);
        if (des.empty() || des[0] == '*' || des[0] == '#' || des[0] == ';') continue;
        if (cleanColName(des) == "designator" || cleanColName(des) == "refdes") continue;

        Component comp;
        comp.designator = des;
        comp.comment = get_val(col_cmt);
        comp.footprint = get_val(col_fp).empty() ? "0603D" : get_val(col_fp);

        std::string raw_l = toLower(get_val(col_layer));
        if (raw_l.find("bot") != std::string::npos || raw_l.find("back") != std::string::npos || raw_l == "b") {
            comp.layer = "BottomLayer";
        } else {
            comp.layer = "TopLayer";
        }

        try {
            std::string sx = get_val(col_x);
            std::string sy = get_val(col_y);
            std::string srot = get_val(col_rot);

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

        // Tự động giữ nguyên các thông số nếu file đã có sẵn định dạng NeoDen YY1
        if (col_head != -1 && !get_val(col_head).empty()) comp.head = std::atoi(get_val(col_head).c_str());
        if (col_feeder != -1 && !get_val(col_feeder).empty()) comp.feeder_no = std::atoi(get_val(col_feeder).c_str());
        if (col_speed != -1 && !get_val(col_speed).empty()) comp.mount_speed = std::atoi(get_val(col_speed).c_str());
        if (col_pick != -1 && !get_val(col_pick).empty()) comp.pick_height = std::atof(get_val(col_pick).c_str());
        if (col_place != -1 && !get_val(col_place).empty()) comp.place_height = std::atof(get_val(col_place).c_str());
        if (col_mode != -1 && !get_val(col_mode).empty()) comp.mode = std::atoi(get_val(col_mode).c_str());
        if (col_skip != -1 && !get_val(col_skip).empty()) comp.skip = std::atoi(get_val(col_skip).c_str());

        components_.push_back(comp);
    }

    if (!header_found || components_.empty()) {
        errorMsg = "Không thể đọc dữ liệu từ file! Vui lòng kiểm tra định dạng CSV/TXT.";
        return false;
    }

    return true;
}

bool NeoDenConverter::exportLayer(const std::string& targetLayer, const std::string& outputPath, LayerSummary& summary, std::string& errorMsg) {
    std::vector<Component> layer_comps;
    for (const auto& c : components_) {
        std::string l = toLower(c.layer);
        if (c.layer.empty() && targetLayer == "Top") {
            layer_comps.push_back(c);
        } else if (targetLayer == "Top" && (l.find("top") != std::string::npos || l == "t")) {
            layer_comps.push_back(c);
        } else if (targetLayer == "Bottom" && (l.find("bottom") != std::string::npos || l.find("bot") != std::string::npos || l == "b")) {
            layer_comps.push_back(c);
        }
    }

    if (layer_comps.empty()) {
        summary.total_components = 0;
        return true;
    }

    // Sắp xếp: Comment -> Designator
    std::sort(layer_comps.begin(), layer_comps.end(), [this](const Component& a, const Component& b) {
        std::string ca = toLower(a.comment);
        std::string cb = toLower(b.comment);
        if (ca != cb) return ca < cb;
        auto pa = extractPrefixAndNumber(a.designator);
        auto pb = extractPrefixAndNumber(b.designator);
        if (pa.first != pb.first) return pa.first < pb.first;
        if (pa.second != pb.second) return pa.second < pb.second;
        return a.designator < b.designator;
    });

    std::map<std::string, int> feeder_map;
    std::map<std::string, size_t> count_map;
    int next_feeder = 1;

    for (const auto& c : layer_comps) {
        if (feeder_map.find(c.comment) == feeder_map.end()) {
            feeder_map[c.comment] = next_feeder++;
        }
        count_map[c.comment]++;
    }

    std::ofstream out(outputPath, std::ios::binary);
    if (!out.is_open()) {
        errorMsg = "Không thể tạo file output: " + outputPath;
        return false;
    }

    out << templateHeader_;
    out << std::fixed << std::setprecision(2);

    for (size_t i = 0; i < layer_comps.size(); ++i) {
        const auto& c = layer_comps[i];
        int feeder_no = feeder_map[c.comment];
        std::string head = headPatterns_.empty() ? "0" : headPatterns_[i % headPatterns_.size()];

        out << c.designator << ","
            << c.comment << ","
            << c.footprint << ","
            << c.mid_x << ","
            << c.mid_y << ","
            << c.rotation << ","
            << head << ","
            << feeder_no << ","
            << "100,0,0,1,0\r\n";
    }

    summary.layer_name = targetLayer;
    summary.output_path = outputPath;
    summary.total_components = layer_comps.size();
    summary.total_feeders = feeder_map.size();
    summary.feeder_items.clear();

    for (const auto& [cmt, f_no] : feeder_map) {
        summary.feeder_items.emplace_back(cmt, count_map[cmt], f_no);
    }
    std::sort(summary.feeder_items.begin(), summary.feeder_items.end(), 
              [](const auto& a, const auto& b){ return std::get<2>(a) < std::get<2>(b); });

    return true;
}

std::string NeoDenConverter::autoDetectAltiumFile() {
    std::vector<std::string> candidates = {
        "Pick Place for MainPCB.csv", "MainPCB.csv", "Pick Place for MainPCB.txt",
        "../Pick Place for MainPCB.csv", "../MainPCB.csv"
    };
    for (const auto& c : candidates) {
        if (fs::exists(c)) return c;
    }

    if (fs::exists(".")) {
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".csv" || ext == ".txt") {
                    std::string fname = entry.path().filename().string();
                    if (fname.find("0603Demo") == std::string::npos &&
                        fname.find("top") == std::string::npos &&
                        fname.find("bot") == std::string::npos &&
                        fname.find("Output") == std::string::npos) {
                        return entry.path().string();
                    }
                }
            }
        }
    }
    return "";
}
