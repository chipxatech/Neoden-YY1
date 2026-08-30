#include "converter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
void enable_console_ansi(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
}
#else
void enable_console_ansi(void) {}
#endif

// Header chuẩn tích hợp từ 0603Demo.csv
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

void print_intro_banner(void) {
    printf("\n");
    printf("      \033[38;2;255;120;0m       ████████████◣           ████████████◣\033[0m\n");
    printf("      \033[38;2;255;80;40m        ██████████████◣         ██████████████◣\033[0m\n");
    printf("  \033[38;2;255;140;0m■\033[0m   \033[38;2;255;40;90m         ██████  ████████◣       ██████  ████████◣\033[0m\n");
    printf("    \033[38;2;255;20;100m■\033[0m \033[38;2;180;40;240m■\033[0m \033[38;2;220;10;130m        ██████    ████████◣     ██████    ████████◣\033[0m\n");
    printf("  \033[38;2;255;0;120m■\033[0m   \033[38;2;30;120;255m■\033[0m \033[38;2;160;30;220m       ██████      ████████◣   ██████      ████████◣\033[0m\n");
    printf("    \033[38;2;40;220;80m■\033[0m   \033[38;2;120;20;240m      ██████        ████████◣ ██████        ████████◣\033[0m\n");
    printf("      \033[38;2;80;10;230m     ██████        ████████◤ ██████        ████████◤\033[0m\n");
    printf("      \033[38;2;40;60;240m    ██████        ████████◤ ██████        ████████◤\033[0m\n");
    printf("      \033[38;2;0;120;255m   ██████        ████████◤ ██████        ████████◤\033[0m\n");
    printf("      \033[38;2;0;180;255m  ██████████████████████◤ ██████████████████████◤\033[0m\n");
    printf("      \033[38;2;0;230;220m ████████████████████◤   ████████████████████◤\033[0m\n\n");

    printf("  ╭───────────────────────────────────────────────────────────────────────╮\n");
    printf("  │     \033[48;2;20;24;40m\033[38;2;0;230;255m   NEODEN YY1 SMT PICK & PLACE AUTOMATED CONVERTER  [C NATIVE]     \033[0m    │\n");
    printf("  │  \033[38;2;180;190;210mPhiên bản: 2.0 Pro • Chuẩn mẫu tự động: 0603Demo.csv\033[0m                     │\n");
    printf("  │  \033[38;2;140;150;170mTự động phân tách Layer | Tự động gán Feeder | Căn chỉnh tọa độ\033[0m               │\n");
    printf("  ╰───────────────────────────────────────────────────────────────────────╯\n\n");
}

void component_list_init(ComponentList* list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void component_list_free(ComponentList* list) {
    if (list->items) {
        free(list->items);
        list->items = NULL;
    }
    list->count = 0;
    list->capacity = 0;
}

void component_list_add(ComponentList* list, const Component* comp) {
    if (list->count >= list->capacity) {
        size_t new_cap = list->capacity == 0 ? 32 : list->capacity * 2;
        Component* new_items = (Component*)realloc(list->items, new_cap * sizeof(Component));
        if (!new_items) return;
        list->items = new_items;
        list->capacity = new_cap;
    }
    list->items[list->count++] = *comp;
}

static void trim_str(char* str) {
    if (!str) return;
    char* start = str;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n' || *start == '"') {
        start++;
    }
    char* end = start + strlen(start);
    while (end > start && (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '\r' || *(end - 1) == '\n' || *(end - 1) == '"')) {
        end--;
    }
    size_t len = (size_t)(end - start);
    memmove(str, start, len);
    str[len] = '\0';
}

static void str_to_lower(char* str) {
    for (; *str; ++str) *str = (char)tolower((unsigned char)*str);
}

static size_t parse_csv_line(const char* line, char fields[][MAX_STR], size_t max_fields) {
    size_t field_idx = 0;
    size_t char_idx = 0;
    bool in_quotes = false;

    for (size_t i = 0; line[i] != '\0' && field_idx < max_fields; ++i) {
        char ch = line[i];
        if (ch == '"') {
            if (in_quotes && line[i + 1] == '"') {
                if (char_idx < MAX_STR - 1) fields[field_idx][char_idx++] = '"';
                ++i;
            } else {
                in_quotes = !in_quotes;
            }
        } else if (ch == ',' && !in_quotes) {
            fields[field_idx][char_idx] = '\0';
            trim_str(fields[field_idx]);
            field_idx++;
            char_idx = 0;
        } else {
            if (char_idx < MAX_STR - 1) fields[field_idx][char_idx++] = ch;
        }
    }

    if (field_idx < max_fields) {
        fields[field_idx][char_idx] = '\0';
        trim_str(fields[field_idx]);
        field_idx++;
    }
    return field_idx;
}

static void extract_prefix_and_number(const char* s, char* prefix, int* number) {
    prefix[0] = '\0';
    *number = 0;
    char num_str[32] = {0};
    int p_idx = 0, n_idx = 0;
    bool in_num = false;

    for (int i = 0; s[i] != '\0'; ++i) {
        if (isdigit((unsigned char)s[i])) {
            in_num = true;
            if (n_idx < 30) num_str[n_idx++] = s[i];
        } else if (!in_num) {
            if (p_idx < MAX_STR - 1) prefix[p_idx++] = s[i];
        }
    }
    prefix[p_idx] = '\0';
    num_str[n_idx] = '\0';
    if (n_idx > 0) *number = atoi(num_str);
}

static void normalize_footprint(const char* in_fp, char* out_fp, size_t out_size) {
    if (!in_fp || in_fp[0] == '\0') {
        strncpy(out_fp, "0603D", out_size);
        return;
    }
    char fl[MAX_STR];
    strncpy(fl, in_fp, sizeof(fl) - 1);
    fl[sizeof(fl) - 1] = '\0';
    str_to_lower(fl);

    if (strstr(fl, "0603")) { strncpy(out_fp, "0603D", out_size); return; }
    if (strstr(fl, "0805")) { strncpy(out_fp, "0805D", out_size); return; }
    if (strstr(fl, "1206")) {
        if (strstr(fl, "cau_chi") || strstr(fl, "fuse")) { strncpy(out_fp, "1206_FUSE", out_size); return; }
        strncpy(out_fp, "1206D", out_size); return;
    }
    if (strstr(fl, "0630")) { strncpy(out_fp, "IND_0630", out_size); return; }
    if (strstr(fl, "tantalum") || strstr(fl, "7443") || strstr(fl, "7343")) { strncpy(out_fp, "TANTAL_7343", out_size); return; }
    if (strstr(fl, "button_2p") || strstr(fl, "nut_nhan_2p")) { strncpy(out_fp, "SW_2P_SMD", out_size); return; }
    if (strstr(fl, "button_4p") || strstr(fl, "nut_nhan_4p")) { strncpy(out_fp, "SW_4P_SMD", out_size); return; }
    if (strstr(fl, "header") || strstr(fl, "hdr")) {
        if (strstr(fl, "1.25")) { strncpy(out_fp, "HDR_1.25_2P_SMD", out_size); return; }
        if (strstr(fl, "4p")) { strncpy(out_fp, "HDR_2.0_4P_SMD", out_size); return; }
        if (strstr(fl, "2p")) { strncpy(out_fp, "HDR_2.0_2P_SMD", out_size); return; }
    }
    if (strstr(fl, "sma")) { strncpy(out_fp, "SMA", out_size); return; }
    if (strstr(fl, "tesdu")) { strncpy(out_fp, "SOD-323", out_size); return; }
    if (strstr(fl, "vr_")) { strncpy(out_fp, "POT_SMD", out_size); return; }
    if (strstr(fl, "via")) { strncpy(out_fp, "VIA_2.2MM", out_size); return; }
    if (strstr(fl, "sdcard") || strstr(fl, "tf3")) { strncpy(out_fp, "TF_CARD_SMD", out_size); return; }
    if (strstr(fl, "soic-16") || strstr(fl, "sop-16")) { strncpy(out_fp, "SOP-16", out_size); return; }
    if (strstr(fl, "typec") || strstr(fl, "type-c")) { strncpy(out_fp, "USB_TYPE_C", out_size); return; }

    strncpy(out_fp, in_fp, out_size);
}

static void normalize_comment(const char* in_cmt, char* out_cmt, size_t out_size) {
    if (!in_cmt || in_cmt[0] == '\0') {
        out_cmt[0] = '\0';
        return;
    }
    char cl[MAX_STR];
    strncpy(cl, in_cmt, sizeof(cl) - 1);
    cl[sizeof(cl) - 1] = '\0';
    str_to_lower(cl);

    if (strstr(cl, "cau chi") || strstr(cl, "fuse")) { strncpy(out_cmt, "FUSE_1206", out_size); return; }
    if (strstr(cl, "nut nhan 2p")) { strncpy(out_cmt, "TACT_SW_2P", out_size); return; }
    if (strstr(cl, "nut nhan 4")) { strncpy(out_cmt, "TACT_SW_4P", out_size); return; }
    if (strcmp(cl, "nguon") == 0) { strncpy(out_cmt, "POWER_HDR", out_size); return; }
    if (strcmp(cl, "bomkhi") == 0) { strncpy(out_cmt, "AIR_PUMP", out_size); return; }
    if (strcmp(cl, "vankhi") == 0) { strncpy(out_cmt, "AIR_VALVE", out_size); return; }
    if (strstr(cl, "led 0603") || strcmp(cl, "led") == 0) { strncpy(out_cmt, "LED_0603", out_size); return; }
    if (strstr(cl, "sdcard") || strstr(cl, "tf3")) { strncpy(out_cmt, "MICRO_SD_TF3", out_size); return; }
    if (strstr(cl, "pressure sensor")) { strncpy(out_cmt, "PRESSURE_SENSOR", out_size); return; }

    strncpy(out_cmt, in_cmt, out_size);
}

static int compare_components(const void* a, const void* b) {
    const Component* ca = (const Component*)a;
    const Component* cb = (const Component*)b;

    char cmt_a[MAX_STR], cmt_b[MAX_STR];
    strncpy(cmt_a, ca->comment, sizeof(cmt_a));
    strncpy(cmt_b, cb->comment, sizeof(cmt_b));
    str_to_lower(cmt_a);
    str_to_lower(cmt_b);

    int cmt_cmp = strcmp(cmt_a, cmt_b);
    if (cmt_cmp != 0) return cmt_cmp;

    char pref_a[MAX_STR], pref_b[MAX_STR];
    int num_a = 0, num_b = 0;
    extract_prefix_and_number(ca->designator, pref_a, &num_a);
    extract_prefix_and_number(cb->designator, pref_b, &num_b);

    int pref_cmp = strcmp(pref_a, pref_b);
    if (pref_cmp != 0) return pref_cmp;

    if (num_a != num_b) return num_a - num_b;

    return strcmp(ca->designator, cb->designator);
}

bool read_altium_file(const char* filepath, ComponentList* out_list, char* error_msg, size_t error_msg_size) {
    FILE* f = fopen(filepath, "rb");
    if (!f) {
        snprintf(error_msg, error_msg_size, "Không thể mở file: %s", filepath);
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(f);
        snprintf(error_msg, error_msg_size, "Không đủ bộ nhớ!");
        return false;
    }

    size_t read_bytes = fread(buffer, 1, size, f);
    buffer[read_bytes] = '\0';
    fclose(f);

    component_list_free(out_list);
    component_list_init(out_list);

    bool header_found = false;
    bool is_mil = false;
    int col_des = -1, col_cmt = -1, col_layer = -1, col_fp = -1;
    int col_x = -1, col_y = -1, col_rot = -1;

    char* line = strtok(buffer, "\r\n");
    char fields[32][MAX_STR];

    while (line != NULL) {
        char line_lower[1024];
        strncpy(line_lower, line, sizeof(line_lower));
        line_lower[sizeof(line_lower) - 1] = '\0';
        str_to_lower(line_lower);

        if (strstr(line_lower, "units used: mil") != NULL) {
            is_mil = true;
        }

        if (!header_found) {
            if (strstr(line, "Designator") != NULL && strstr(line, "Comment") != NULL) {
                header_found = true;
                size_t num_cols = parse_csv_line(line, fields, 32);
                for (size_t i = 0; i < num_cols; ++i) {
                    char col_name[MAX_STR];
                    strncpy(col_name, fields[i], sizeof(col_name));
                    str_to_lower(col_name);

                    if (strcmp(col_name, "designator") == 0) col_des = (int)i;
                    else if (strcmp(col_name, "comment") == 0) col_cmt = (int)i;
                    else if (strcmp(col_name, "layer") == 0) col_layer = (int)i;
                    else if (strcmp(col_name, "footprint") == 0) col_fp = (int)i;
                    else if (strncmp(col_name, "center-x", 8) == 0 || strncmp(col_name, "mid x", 5) == 0) {
                        col_x = (int)i;
                        if (strstr(col_name, "mil") != NULL) is_mil = true;
                    }
                    else if (strncmp(col_name, "center-y", 8) == 0 || strncmp(col_name, "mid y", 5) == 0) {
                        col_y = (int)i;
                    }
                    else if (strncmp(col_name, "rotation", 8) == 0) {
                        col_rot = (int)i;
                    }
                }
            }
            line = strtok(NULL, "\r\n");
            continue;
        }

        size_t num_fields = parse_csv_line(line, fields, 32);
        if (col_des >= 0 && col_des < (int)num_fields) {
            const char* des = fields[col_des];
            if (des[0] != '\0' && des[0] != '*') {
                Component comp;
                memset(&comp, 0, sizeof(Component));
                strncpy(comp.designator, des, sizeof(comp.designator) - 1);

                if (col_cmt >= 0 && col_cmt < (int)num_fields) {
                    normalize_comment(fields[col_cmt], comp.comment, sizeof(comp.comment) - 1);
                }

                if (col_fp >= 0 && col_fp < (int)num_fields && fields[col_fp][0] != '\0') {
                    normalize_footprint(fields[col_fp], comp.footprint, sizeof(comp.footprint) - 1);
                } else {
                    strcpy(comp.footprint, "0603D");
                }

                if (col_layer >= 0 && col_layer < (int)num_fields)
                    strncpy(comp.layer, fields[col_layer], sizeof(comp.layer) - 1);

                double rx = (col_x >= 0 && col_x < (int)num_fields) ? atof(fields[col_x]) : 0.0;
                double ry = (col_y >= 0 && col_y < (int)num_fields) ? atof(fields[col_y]) : 0.0;
                comp.rotation = (col_rot >= 0 && col_rot < (int)num_fields) ? atof(fields[col_rot]) : 0.0;

                comp.mid_x = is_mil ? rx * 0.0254 : rx;
                comp.mid_y = is_mil ? ry * 0.0254 : ry;
                comp.head = 0;
                comp.feeder_no = 1;
                comp.mount_speed = 100;
                comp.pick_height = 0.0;
                comp.place_height = 0.0;
                comp.mode = 1;
                comp.skip = 0;

                component_list_add(out_list, &comp);
            }
        }

        line = strtok(NULL, "\r\n");
    }

    free(buffer);

    if (!header_found) {
        snprintf(error_msg, error_msg_size, "Không tìm thấy dòng Header (Designator, Comment) trong file Altium!");
        return false;
    }

    return true;
}

bool export_layer(const ComponentList* all_comps, const char* target_layer, const char* output_path, 
                  LayerSummary* summary, char* error_msg, size_t error_msg_size) {
    ComponentList layer_comps;
    component_list_init(&layer_comps);

    for (size_t i = 0; i < all_comps->count; ++i) {
        const Component* c = &all_comps->items[i];
        char l[MAX_STR];
        strncpy(l, c->layer, sizeof(l));
        str_to_lower(l);

        if (c->layer[0] == '\0' && strcmp(target_layer, "Top") == 0) {
            component_list_add(&layer_comps, c);
        } else if (strcmp(target_layer, "Top") == 0 && (strstr(l, "top") != NULL || strcmp(l, "t") == 0)) {
            component_list_add(&layer_comps, c);
        } else if (strcmp(target_layer, "Bottom") == 0 && (strstr(l, "bottom") != NULL || strstr(l, "bot") != NULL || strcmp(l, "b") == 0)) {
            component_list_add(&layer_comps, c);
        }
    }

    if (layer_comps.count == 0) {
        summary->total_components = 0;
        summary->total_feeders = 0;
        summary->feeder_items = NULL;
        summary->feeder_items_count = 0;
        component_list_free(&layer_comps);
        return true;
    }

    // Sắp xếp tự nhiên
    qsort(layer_comps.items, layer_comps.count, sizeof(Component), compare_components);

    // Gán Feeder
    FeederSummaryItem* feeders = (FeederSummaryItem*)malloc(layer_comps.count * sizeof(FeederSummaryItem));
    size_t num_feeders = 0;
    int next_feeder = 1;

    int* comp_feeder_map = (int*)malloc(layer_comps.count * sizeof(int));

    for (size_t i = 0; i < layer_comps.count; ++i) {
        const char* cmt = layer_comps.items[i].comment;
        int found_feeder = -1;

        for (size_t j = 0; j < num_feeders; ++j) {
            if (strcmp(feeders[j].comment, cmt) == 0) {
                found_feeder = feeders[j].feeder_no;
                feeders[j].count++;
                break;
            }
        }

        if (found_feeder == -1) {
            found_feeder = next_feeder++;
            strncpy(feeders[num_feeders].comment, cmt, sizeof(feeders[num_feeders].comment) - 1);
            feeders[num_feeders].count = 1;
            feeders[num_feeders].feeder_no = found_feeder;
            num_feeders++;
        }

        comp_feeder_map[i] = found_feeder;
        if (layer_comps.items[i].feeder_no <= 1) {
            layer_comps.items[i].feeder_no = found_feeder;
        }
    }

    FILE* out = fopen(output_path, "wb");
    if (!out) {
        snprintf(error_msg, error_msg_size, "Không thể tạo file: %s", output_path);
        free(feeders);
        free(comp_feeder_map);
        component_list_free(&layer_comps);
        return false;
    }

    // Ghi Header từ file mẫu hoặc template nhúng
    FILE* t_file = fopen("0603Demo.csv", "rb");
    if (!t_file) t_file = fopen("../0603Demo.csv", "rb");

    if (t_file) {
        char t_buf[256];
        bool found = false;
        while (fgets(t_buf, sizeof(t_buf), t_file)) {
            fputs(t_buf, out);
            if (strstr(t_buf, "Designator") && strstr(t_buf, "Comment")) {
                found = true;
                break;
            }
        }
        fclose(t_file);
        if (!found) {
            fputs(EMBEDDED_HEADER, out);
        }
    } else {
        fputs(EMBEDDED_HEADER, out);
    }

    for (size_t i = 0; i < layer_comps.count; ++i) {
        const Component* c = &layer_comps.items[i];
        int fno = (c->feeder_no > 0) ? c->feeder_no : comp_feeder_map[i];
        fprintf(out, "%s,%s,%s,%.2f,%.2f,%.2f,%d,%d,%d,%.2f,%.2f,%d,%d\r\n",
                c->designator, c->comment, c->footprint,
                c->mid_x, c->mid_y, c->rotation,
                c->head, fno, c->mount_speed,
                c->pick_height, c->place_height, c->mode, c->skip);
    }
    fclose(out);

    strncpy(summary->layer_name, target_layer, sizeof(summary->layer_name));
    strncpy(summary->output_path, output_path, sizeof(summary->output_path));
    summary->total_components = layer_comps.count;
    summary->total_feeders = num_feeders;
    summary->feeder_items = feeders;
    summary->feeder_items_count = num_feeders;

    free(comp_feeder_map);
    component_list_free(&layer_comps);
    return true;
}

bool auto_detect_altium_file(char* out_path, size_t out_path_size) {
    const char* candidates[] = {
        "Pick Place for MainPCB.csv", "MainPCB.csv", "Pick Place for MainPCB.txt",
        "../Pick Place for MainPCB.csv", "../MainPCB.csv"
    };
    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        FILE* f = fopen(candidates[i], "r");
        if (f) {
            fclose(f);
            snprintf(out_path, out_path_size, "%s", candidates[i]);
            return true;
        }
    }
    return false;
}
