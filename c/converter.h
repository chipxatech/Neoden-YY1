#ifndef CONVERTER_H
#define CONVERTER_H

#include <stddef.h>
#include <stdbool.h>

#define MAX_STR 128

typedef struct {
    char designator[MAX_STR];
    char comment[MAX_STR];
    char footprint[MAX_STR];
    double mid_x;
    double mid_y;
    double rotation;
    int head;
    int feeder_no;
    int raw_feeder_no;
    int mount_speed;
    double pick_height;
    double place_height;
    int mode;
    int skip;
    char layer[MAX_STR];
} Component;

typedef struct {
    char comment[MAX_STR];
    int count;
    int feeder_no;
} FeederSummaryItem;

typedef struct {
    char layer_name[32];
    char output_path[256];
    size_t total_components;
    size_t total_feeders;
    FeederSummaryItem* feeder_items;
    size_t feeder_items_count;
} LayerSummary;

typedef struct {
    Component* items;
    size_t count;
    size_t capacity;
} ComponentList;

// Khởi tạo và giải phóng danh sách linh kiện
void component_list_init(ComponentList* list);
void component_list_free(ComponentList* list);
void component_list_add(ComponentList* list, const Component* comp);

// Đọc file Altium (.csv / .txt)
bool read_altium_file(const char* filepath, ComponentList* out_list, char* error_msg, size_t error_msg_size);

// Xuất file CSV mặt TOP / BOTTOM
bool export_layer(const ComponentList* all_comps, const char* target_layer, const char* output_path, 
                  LayerSummary* summary, char* error_msg, size_t error_msg_size);

// Tự động tìm file Altium trong thư mục
bool auto_detect_altium_file(char* out_path, size_t out_path_size);

// In Banner Intro với Logo màu sắc
void print_intro_banner(void);

// Kích hoạt ANSI trên Windows
void enable_console_ansi(void);

#endif // CONVERTER_H
