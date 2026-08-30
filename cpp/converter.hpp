#ifndef CONVERTER_HPP
#define CONVERTER_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

struct Component {
    std::string designator;
    std::string comment;
    std::string footprint;
    double mid_x = 0.0;
    double mid_y = 0.0;
    double rotation = 0.0;
    std::string layer;
};

struct LayerSummary {
    std::string layer_name;
    std::string output_path;
    size_t total_components = 0;
    size_t total_feeders = 0;
    std::vector<std::tuple<std::string, size_t, int>> feeder_items; // comment, count, feeder_no
};

class NeoDenConverter {
public:
    NeoDenConverter();
    
    // Đọc file Altium Pick & Place (.csv hoặc .txt)
    bool readAltiumFile(const std::string& filepath, std::string& errorMsg);
    
    // Xuất dữ liệu ra file NeoDen YY1 CSV
    bool exportLayer(const std::string& targetLayer, const std::string& outputPath, LayerSummary& summary, std::string& errorMsg);
    
    // Tự động tìm file Altium trong thư mục
    static std::string autoDetectAltiumFile();
    
    // In Banner Intro với Logo màu sắc chuyên nghiệp
    static void printIntroBanner();
    
    size_t getComponentCount() const { return components_.size(); }

private:
    std::vector<Component> components_;
    std::string templateHeader_;
    std::vector<std::string> headPatterns_;
    
    void loadTemplate();
    static std::vector<std::string> parseCsvLine(const std::string& line);
    static std::pair<std::string, int> extractPrefixAndNumber(const std::string& s);
    static std::string trim(const std::string& s);
    static std::string toLower(std::string s);
};

#endif // CONVERTER_HPP
