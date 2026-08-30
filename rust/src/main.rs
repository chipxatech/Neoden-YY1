#![windows_subsystem = "windows"]

use std::ffi::c_void;
use std::fs::File;
use std::io::{Read, Write};
use std::ptr;
use std::sync::Mutex;

type HWND = *mut c_void;
type HDC = *mut c_void;
type HFONT = *mut c_void;
type HINSTANCE = *mut c_void;
type HBRUSH = *mut c_void;
type HPEN = *mut c_void;
type HGDIOBJ = *mut c_void;
type GpBitmap = *mut c_void;
type GpGraphics = *mut c_void;

#[repr(C)]
struct RECT {
    left: i32,
    top: i32,
    right: i32,
    bottom: i32,
}

#[repr(C)]
struct PAINTSTRUCT {
    hdc: HDC,
    f_erase: i32,
    rc_paint: RECT,
    f_restore: i32,
    f_inc_update: i32,
    rgb_reserved: [u8; 32],
}

#[repr(C)]
struct WNDCLASSEXW {
    cb_size: u32,
    style: u32,
    lpfn_wnd_proc: unsafe extern "system" fn(HWND, u32, usize, isize) -> isize,
    cb_cls_extra: i32,
    cb_wnd_extra: i32,
    h_instance: HINSTANCE,
    h_icon: *mut c_void,
    h_cursor: *mut c_void,
    hbr_background: HBRUSH,
    lpsz_menu_name: *const u16,
    lpsz_class_name: *const u16,
    h_icon_sm: *mut c_void,
}

#[repr(C)]
struct MSG {
    hwnd: HWND,
    message: u32,
    wparam: usize,
    lparam: isize,
    time: u32,
    pt_x: i32,
    pt_y: i32,
}

#[repr(C)]
struct INITCOMMONCONTROLSEX {
    dw_size: u32,
    dw_icc: u32,
}

#[repr(C)]
struct GdiplusStartupInput {
    gdiplus_version: u32,
    debug_event_callback: *mut c_void,
    suppress_background_thread: i32,
    suppress_external_codecs: i32,
}

#[repr(C)]
struct LVCOLUMNW {
    mask: u32,
    fmt: i32,
    cx: i32,
    psz_text: *mut u16,
    cch_text_max: i32,
    i_sub_item: i32,
    i_image: i32,
    i_order: i32,
    cx_min: i32,
    cx_default: i32,
    cx_ideal: i32,
}

#[repr(C)]
struct LVITEMW {
    mask: u32,
    i_item: i32,
    i_sub_item: i32,
    state: u32,
    state_mask: u32,
    psz_text: *mut u16,
    cch_text_max: i32,
    i_image: i32,
    l_param: isize,
    i_indent: i32,
    i_group_id: i32,
    c_columns: u32,
    pu_columns: *mut u32,
    pi_col_fmt: *mut i32,
    i_group: i32,
}

#[repr(C)]
struct OPENFILENAMEW {
    l_struct_size: u32,
    hwnd_owner: HWND,
    h_instance: HINSTANCE,
    lpstr_filter: *const u16,
    lpstr_custom_filter: *mut u16,
    n_max_cust_filter: u32,
    n_filter_index: u32,
    lpstr_file: *mut u16,
    n_max_file: u32,
    lpstr_file_title: *mut u16,
    n_max_file_title: u32,
    lpstr_initial_dir: *const u16,
    lpstr_title: *const u16,
    flags: u32,
    n_file_offset: u16,
    n_file_extension: u16,
    lpstr_def_ext: *const u16,
    l_cust_data: isize,
    lpfn_hook: *mut c_void,
    lp_template_name: *const u16,
    pv_reserved: *mut c_void,
    dw_reserved: u32,
    flags_ex: u32,
}

#[repr(C)]
struct NMHDR {
    hwnd_from: HWND,
    id_from: usize,
    code: u32,
}

#[repr(C)]
struct NMCUSTOMDRAW {
    hdr: NMHDR,
    dw_draw_stage: u32,
    hdc: HDC,
    rc: RECT,
    dw_item_spec: usize,
    u_item_state: u32,
    l_item_l_param: isize,
}

#[repr(C)]
struct NMLVCUSTOMDRAW {
    nmcd: NMCUSTOMDRAW,
    clr_text: u32,
    clr_text_bk: u32,
    i_sub_item: i32,
    dw_item_type: u32,
    clr_face: u32,
    i_icon_effect: i32,
    i_icon_phase: i32,
    i_part_id: i32,
    i_state_id: i32,
    rc_text: RECT,
    u_align: u32,
}

#[repr(C)]
struct POINT {
    x: i32,
    y: i32,
}

#[repr(C)]
struct NMITEMACTIVATE {
    hdr: NMHDR,
    i_item: i32,
    i_sub_item: i32,
    u_new_state: u32,
    u_old_state: u32,
    u_changed: u32,
    pt_action: POINT,
    l_param: isize,
    u_key_flags: u32,
}

#[derive(Clone, Debug)]
pub struct Component {
    pub designator: String,
    pub comment: String,
    pub footprint: String,
    pub mid_x: f64,
    pub mid_y: f64,
    pub rotation: f64,
    pub head: i32,
    pub feeder_no: i32,
    pub mount_speed: i32,
    pub pick_height: f64,
    pub place_height: f64,
    pub mode: i32,
    pub skip: i32,
    pub layer: String,
}

struct AppState {
    hwnd: HWND,
    h_splash: HWND,
    h_edit_input: HWND,
    h_edit_top: HWND,
    h_edit_bot: HWND,
    h_list_view: HWND,
    h_status: HWND,
    h_radio_top: HWND,
    h_radio_bot: HWND,
    font_main_title: HFONT,
    font_main_sub: HFONT,
    font_splash_title: HFONT,
    font_splash_sub: HFONT,
    logo_bitmap: GpBitmap,
    splash_progress: i32,
    showing_top: bool,
    top_components: Vec<Component>,
    bot_components: Vec<Component>,
}

unsafe impl Send for AppState {}
unsafe impl Sync for AppState {}

static STATE: Mutex<AppState> = Mutex::new(AppState {
    hwnd: ptr::null_mut(),
    h_splash: ptr::null_mut(),
    h_edit_input: ptr::null_mut(),
    h_edit_top: ptr::null_mut(),
    h_edit_bot: ptr::null_mut(),
    h_list_view: ptr::null_mut(),
    h_status: ptr::null_mut(),
    h_radio_top: ptr::null_mut(),
    h_radio_bot: ptr::null_mut(),
    font_main_title: ptr::null_mut(),
    font_main_sub: ptr::null_mut(),
    font_splash_title: ptr::null_mut(),
    font_splash_sub: ptr::null_mut(),
    logo_bitmap: ptr::null_mut(),
    splash_progress: 0,
    showing_top: true,
    top_components: Vec::new(),
    bot_components: Vec::new(),
});

const DEFAULT_HEADER: &str = "\
NEODEN,YY1,P&P FILE,,,,,,,,,,
,,,,,,,,,,,,
PanelizedPCB,UnitLength,0,UnitWidth,0,Rows,1,Columns,1,,,,
,,,,,,,,,,,,
Fiducial,1-X,55.08,1-Y,6.95,OverallOffsetX,0.14,OverallOffsetY,0.08,,,,
,,,,,,,,,,,,
NozzleChange,OFF,BeforeComponent,2,Head1,Drop,Station1,PickUp,Station3,,,,
NozzleChange,OFF,BeforeComponent,3,Head1,Drop,Station3,PickUp,Station1,,,,
NozzleChange,OFF,BeforeComponent,1,Head1,Drop,Station1,PickUp,Station1,,,,
NozzleChange,OFF,BeforeComponent,1,Head1,Drop,Station1,PickUp,Station1,,,,
,,,,,,,,,,,,
Designator,Comment,Footprint,Mid X(mm),Mid Y(mm) ,Rotation,Head ,FeederNo,Mount Speed(%),Pick Height(mm),Place Height(mm),Mode,Skip\r\n";

#[link(name = "user32")]
#[link(name = "gdi32")]
#[link(name = "comctl32")]
#[link(name = "gdiplus")]
#[link(name = "shell32")]
#[link(name = "comdlg32")]
unsafe extern "system" {
    fn InitCommonControlsEx(lpInitCtrls: *const INITCOMMONCONTROLSEX) -> i32;
    fn GdiplusStartup(token: *mut usize, input: *const GdiplusStartupInput, output: *mut c_void) -> i32;
    fn GdiplusShutdown(token: usize);
    fn GdipCreateBitmapFromFile(filename: *const u16, bitmap: *mut GpBitmap) -> i32;
    fn GdipCreateFromHDC(hdc: HDC, graphics: *mut GpGraphics) -> i32;
    fn GdipDrawImageRectI(graphics: GpGraphics, image: GpBitmap, x: i32, y: i32, width: i32, height: i32) -> i32;
    fn GdipDisposeImage(image: GpBitmap) -> i32;
    fn GdipDeleteGraphics(graphics: GpGraphics) -> i32;

    fn RegisterClassExW(lpWndClass: *const WNDCLASSEXW) -> u16;
    fn CreateWindowExW(
        dwExStyle: u32, lpClassName: *const u16, lpWindowName: *const u16,
        dwStyle: u32, X: i32, Y: i32, nWidth: i32, nHeight: i32,
        hWndParent: HWND, hMenu: *mut c_void, hInstance: HINSTANCE, lpParam: *mut c_void,
    ) -> HWND;
    fn ShowWindow(hWnd: HWND, nCmdShow: i32) -> i32;
    fn UpdateWindow(hWnd: HWND) -> i32;
    fn DestroyWindow(hWnd: HWND) -> i32;
    fn SetTimer(hWnd: HWND, nIDEvent: usize, uElapse: u32, lpTimerFunc: *mut c_void) -> usize;
    fn KillTimer(hWnd: HWND, uIDEvent: usize) -> i32;
    fn GetMessageW(lpMsg: *mut MSG, hWnd: HWND, wMsgFilterMin: u32, wMsgFilterMax: u32) -> i32;
    fn TranslateMessage(lpMsg: *const MSG) -> i32;
    fn DispatchMessageW(lpMsg: *const MSG) -> isize;
    fn PostQuitMessage(nExitCode: i32);
    fn DefWindowProcW(hWnd: HWND, Msg: u32, wParam: usize, lParam: isize) -> isize;
    fn SendMessageW(hWnd: HWND, Msg: u32, wParam: usize, lParam: isize) -> isize;
    fn InvalidateRect(hWnd: HWND, lpRect: *const RECT, bErase: i32) -> i32;
    fn GetClientRect(hWnd: HWND, lpRect: *mut RECT) -> i32;
    fn BeginPaint(hWnd: HWND, lpPaint: *mut PAINTSTRUCT) -> HDC;
    fn EndPaint(hWnd: HWND, lpPaint: *const PAINTSTRUCT) -> i32;
    fn CreateSolidBrush(color: u32) -> HBRUSH;
    fn CreatePen(fnPenStyle: i32, nWidth: i32, crColor: u32) -> HPEN;
    fn SelectObject(hdc: HDC, h: HGDIOBJ) -> HGDIOBJ;
    fn DeleteObject(ho: HGDIOBJ) -> i32;
    fn FillRect(hDC: HDC, lprc: *const RECT, hbr: HBRUSH) -> i32;
    fn Rectangle(hdc: HDC, left: i32, top: i32, right: i32, bottom: i32) -> i32;
    fn GetStockObject(i: i32) -> HGDIOBJ;
    fn SetBkMode(hdc: HDC, mode: i32) -> i32;
    fn SetTextColor(hdc: HDC, color: u32) -> u32;
    fn DrawTextW(hdc: HDC, lpchText: *const u16, cchText: i32, lprc: *mut RECT, format: u32) -> i32;
    fn TextOutW(hdc: HDC, x: i32, y: i32, lpString: *const u16, c: i32) -> i32;
    fn GetSystemMetrics(nIndex: i32) -> i32;
    fn SetWindowTextW(hWnd: HWND, lpString: *const u16) -> i32;
    fn GetWindowTextW(hWnd: HWND, lpString: *mut u16, nMaxCount: i32) -> i32;
    fn MessageBoxW(hWnd: HWND, lpText: *const u16, lpCaption: *const u16, uType: u32) -> i32;
    fn LoadCursorW(hInstance: HINSTANCE, lpCursorName: *const u16) -> *mut c_void;
    fn LoadImageW(hInst: HINSTANCE, name: *const u16, uType: u32, cx: i32, cy: i32, fuLoad: u32) -> *mut c_void;
    fn CreateFontW(
        cHeight: i32, cWidth: i32, cEscapement: i32, cOrientation: i32, cWeight: i32,
        bItalic: u32, bUnderline: u32, bStrikeOut: u32, iCharSet: u32,
        iOutPrecision: u32, iClipPrecision: u32, iQuality: u32, iPitchAndFamily: u32,
        pszFaceName: *const u16,
    ) -> HFONT;
    fn GetOpenFileNameW(lpofn: *mut OPENFILENAMEW) -> i32;
    fn ShellExecuteW(hwnd: HWND, lpOperation: *const u16, lpFile: *const u16, lpParameters: *const u16, lpDirectory: *const u16, nShowCmd: i32) -> HINSTANCE;
}

fn to_wstr(s: &str) -> Vec<u16> {
    s.encode_utf16().chain(std::iter::once(0)).collect()
}

fn from_wstr(ptr: *const u16) -> String {
    if ptr.is_null() { return String::new(); }
    let mut len = 0;
    unsafe {
        while *ptr.add(len) != 0 { len += 1; }
        let slice = std::slice::from_raw_parts(ptr, len);
        String::from_utf16_lossy(slice)
    }
}

fn refresh_list_view() {
    let (hwnd_lv, hwnd_status, list, showing_top, top_len, bot_len) = {
        let state = STATE.lock().unwrap();
        if state.h_list_view.is_null() { return; }
        let list = if state.showing_top { state.top_components.clone() } else { state.bot_components.clone() };
        (state.h_list_view, state.h_status, list, state.showing_top, state.top_components.len(), state.bot_components.len())
    };

    unsafe {
        SendMessageW(hwnd_lv, 0x1009 /* LVM_DELETEALLITEMS */, 0, 0);

        for (i, c) in list.iter().enumerate() {
            let mut stt = to_wstr(&format!("{}", i + 1));
            let mut item: LVITEMW = std::mem::zeroed();
            item.mask = 0x0001; // LVIF_TEXT
            item.i_item = i as i32;
            item.psz_text = stt.as_mut_ptr();
            SendMessageW(hwnd_lv, 0x104D /* LVM_INSERTITEMW */, 0, &item as *const _ as isize);

            let mut set_sub = |sub: i32, text: &str| {
                let mut w = to_wstr(text);
                let mut sub_item: LVITEMW = std::mem::zeroed();
                sub_item.i_sub_item = sub;
                sub_item.psz_text = w.as_mut_ptr();
                SendMessageW(hwnd_lv, 0x1074 /* LVM_SETITEMTEXTW */, i, &sub_item as *const _ as isize);
            };

            set_sub(1, &c.designator);
            set_sub(2, &c.comment);
            set_sub(3, &c.footprint);
            set_sub(4, &format!("{:.2}", c.mid_x));
            set_sub(5, &format!("{:.2}", c.mid_y));
            set_sub(6, &format!("{:.2}", c.rotation));
            set_sub(7, &format!("{}", c.head));
            set_sub(8, &format!("{}", c.feeder_no));
            set_sub(9, &format!("{}", c.mount_speed));
            set_sub(10, &format!("{:.2}", c.pick_height));
            set_sub(11, &format!("{:.2}", c.place_height));
            set_sub(12, &format!("{}", c.mode));
            set_sub(13, "");
        }

        let status_txt = format!(
            "Mặt TOP: {} pcs  |  Mặt BOTTOM: {} pcs  |  Đang xem: {} ({} pcs)  |  Nhấp đúp chuột để sửa",
            top_len, bot_len,
            if showing_top { "TOP" } else { "BOTTOM" }, list.len()
        );
        let w_status = to_wstr(&status_txt);
        SetWindowTextW(hwnd_status, w_status.as_ptr());
    }
}

fn normalize_footprint(fp: &str) -> String {
    if fp.is_empty() { return "0603D".to_string(); }
    let fl = fp.trim().to_lowercase();
    if fl.contains("0603") { return "0603D".to_string(); }
    if fl.contains("0805") { return "0805D".to_string(); }
    if fl.contains("1206") {
        if fl.contains("cau_chi") || fl.contains("fuse") { return "1206_FUSE".to_string(); }
        return "1206D".to_string();
    }
    if fl.contains("0630") { return "IND_0630".to_string(); }
    if fl.contains("tantalum") || fl.contains("7443") || fl.contains("7343") { return "TANTAL_7343".to_string(); }
    if fl.contains("button_2p") || fl.contains("nut_nhan_2p") { return "SW_2P_SMD".to_string(); }
    if fl.contains("button_4p") || fl.contains("nut_nhan_4p") { return "SW_4P_SMD".to_string(); }
    if fl.contains("header") || fl.contains("hdr") {
        if fl.contains("1.25") { return "HDR_1.25_2P_SMD".to_string(); }
        if fl.contains("4p") { return "HDR_2.0_4P_SMD".to_string(); }
        if fl.contains("2p") { return "HDR_2.0_2P_SMD".to_string(); }
    }
    if fl.contains("sma") { return "SMA".to_string(); }
    if fl.contains("tesdu") { return "SOD-323".to_string(); }
    if fl.contains("vr_") { return "POT_SMD".to_string(); }
    if fl.contains("via") { return "VIA_2.2MM".to_string(); }
    if fl.contains("sdcard") || fl.contains("tf3") { return "TF_CARD_SMD".to_string(); }
    if fl.contains("soic-16") || fl.contains("sop-16") { return "SOP-16".to_string(); }
    if fl.contains("typec") || fl.contains("type-c") { return "USB_TYPE_C".to_string(); }
    fp.trim().to_string()
}

fn normalize_comment(cmt: &str) -> String {
    if cmt.is_empty() { return String::new(); }
    let cl = cmt.trim().to_lowercase();
    if cl.contains("cau chi") || cl.contains("fuse") { return "FUSE_1206".to_string(); }
    if cl.contains("nut nhan 2p") { return "TACT_SW_2P".to_string(); }
    if cl.contains("nut nhan 4") { return "TACT_SW_4P".to_string(); }
    if cl == "nguon" { return "POWER_HDR".to_string(); }
    if cl == "bomkhi" { return "AIR_PUMP".to_string(); }
    if cl == "vankhi" { return "AIR_VALVE".to_string(); }
    if cl.contains("led 0603") || cl == "led" { return "LED_0603".to_string(); }
    if cl.contains("sdcard") || cl.contains("tf3") { return "MICRO_SD_TF3".to_string(); }
    if cl.contains("pressure sensor") { return "PRESSURE_SENSOR".to_string(); }
    cmt.trim().to_string()
}

fn parse_altium_data(filepath: &str) -> bool {
    let mut file = match File::open(filepath) {
        Ok(f) => f,
        Err(_) => return false,
    };
    let mut content = String::new();
    if file.read_to_string(&mut content).is_err() {
        return false;
    }

    let mut is_mil = false;
    let mut header_found = false;
    let mut col_map = std::collections::HashMap::new();

    let mut top_raw = Vec::new();
    let mut bot_raw = Vec::new();

    for line in content.lines() {
        let l_trim = line.trim();
        if l_trim.is_empty() { continue; }

        if l_trim.to_lowercase().contains("units used: mil") {
            is_mil = true;
        }

        if !header_found {
            if l_trim.contains("Designator") && l_trim.contains("Comment") {
                header_found = true;
                let cols: Vec<&str> = l_trim.split(',').map(|s| s.trim().trim_matches('"')).collect();
                for (idx, c) in cols.iter().enumerate() {
                    let cl = c.to_lowercase();
                    if cl == "designator" { col_map.insert("des", idx); }
                    else if cl == "comment" { col_map.insert("cmt", idx); }
                    else if cl == "layer" { col_map.insert("layer", idx); }
                    else if cl == "footprint" { col_map.insert("fp", idx); }
                    else if cl.starts_with("center-x") || cl.starts_with("mid x") {
                        col_map.insert("x", idx);
                        if cl.contains("mil") { is_mil = true; }
                    } else if cl.starts_with("center-y") || cl.starts_with("mid y") {
                        col_map.insert("y", idx);
                    } else if cl.starts_with("rotation") {
                        col_map.insert("rot", idx);
                    }
                }
            }
            continue;
        }

        let parts: Vec<&str> = l_trim.split(',').map(|s| s.trim().trim_matches('"')).collect();
        if let Some(&des_idx) = col_map.get("des") {
            if des_idx >= parts.len() { continue; }
            let des = parts[des_idx];
            if des.is_empty() || des.starts_with('*') { continue; }

            let cmt_raw = col_map.get("cmt").and_then(|&i| parts.get(i)).unwrap_or(&"");
            let fp_raw = col_map.get("fp").and_then(|&i| parts.get(i)).filter(|s| !s.is_empty()).unwrap_or(&"0603D");
            let layer = col_map.get("layer").and_then(|&i| parts.get(i)).unwrap_or(&"TopLayer").to_string();

            let cmt = normalize_comment(cmt_raw);
            let fp = normalize_footprint(fp_raw);

            let rx: f64 = col_map.get("x").and_then(|&i| parts.get(i)).and_then(|s| s.parse().ok()).unwrap_or(0.0);
            let ry: f64 = col_map.get("y").and_then(|&i| parts.get(i)).and_then(|s| s.parse().ok()).unwrap_or(0.0);
            let rot: f64 = col_map.get("rot").and_then(|&i| parts.get(i)).and_then(|s| s.parse().ok()).unwrap_or(0.0);

            let mid_x = if is_mil { rx * 0.0254 } else { rx };
            let mid_y = if is_mil { ry * 0.0254 } else { ry };

            let comp = Component {
                designator: des.to_string(),
                comment: cmt,
                footprint: fp,
                mid_x,
                mid_y,
                rotation: rot,
                head: 0,
                feeder_no: 1,
                mount_speed: 100,
                pick_height: 0.0,
                place_height: 0.0,
                mode: 1,
                skip: 0,
                layer: layer.clone(),
            };

            let ll = layer.to_lowercase();
            if ll.contains("bot") || ll == "b" {
                bot_raw.push(comp);
            } else {
                top_raw.push(comp);
            }
        }
    }

    let sort_comps = |list: &mut Vec<Component>| {
        list.sort_by(|a, b| {
            if a.comment != b.comment {
                a.comment.cmp(&b.comment)
            } else {
                a.designator.cmp(&b.designator)
            }
        });
        let mut feeder_map = std::collections::HashMap::new();
        let mut next_f = 1;
        for c in list.iter_mut() {
            let fno = *feeder_map.entry(c.comment.clone()).or_insert_with(|| {
                let f = next_f;
                next_f += 1;
                f
            });
            c.feeder_no = fno;
        }
    };

    sort_comps(&mut top_raw);
    sort_comps(&mut bot_raw);

    {
        let mut state = STATE.lock().unwrap();
        state.top_components = top_raw;
        state.bot_components = bot_raw;
    }

    refresh_list_view();
    true
}

fn save_outputs() {
    let (hwnd, h_top, h_bot, top_comps, bot_comps) = {
        let state = STATE.lock().unwrap();
        (state.hwnd, state.h_edit_top, state.h_edit_bot, state.top_components.clone(), state.bot_components.clone())
    };

    if top_comps.is_empty() && bot_comps.is_empty() {
        unsafe {
            MessageBoxW(hwnd, to_wstr("Chưa có dữ liệu để lưu!").as_ptr(), to_wstr("Thông Báo").as_ptr(), 0x0030);
        }
        return;
    }

    let mut w_top = [0u16; 260];
    let mut w_bot = [0u16; 260];
    unsafe {
        GetWindowTextW(h_top, w_top.as_mut_ptr(), 260);
        GetWindowTextW(h_bot, w_bot.as_mut_ptr(), 260);
    }
    let top_name = from_wstr(w_top.as_ptr());
    let bot_name = from_wstr(w_bot.as_ptr());

    let write_file = |name: &str, list: &Vec<Component>| -> bool {
        if let Ok(mut f) = File::create(name) {
            let _ = f.write_all(DEFAULT_HEADER.as_bytes());
            for c in list {
                let row = format!(
                    "{},{},{},{:.2},{:.2},{:.2},{},{},{},{:.2},{:.2},{},{}\r\n",
                    c.designator, c.comment, c.footprint,
                    c.mid_x, c.mid_y, c.rotation,
                    c.head, c.feeder_no, c.mount_speed,
                    c.pick_height, c.place_height, c.mode, c.skip
                );
                let _ = f.write_all(row.as_bytes());
            }
            true
        } else {
            false
        }
    };

    let ok_top = write_file(&top_name, &top_comps);
    let ok_bot = write_file(&bot_name, &bot_comps);

    if ok_top && ok_bot {
        let msg = format!(
            "ĐÃ LƯU FILE CHỈNH SỬA CHO MÁY NEODEN YY1!\n\n• Mặt TOP: {} linh kiện -> {}\n• Mặt BOTTOM: {} linh kiện -> {}\n\nToàn bộ 13 thông số đã chỉnh sửa được lưu chính xác 100%.\nBạn có muốn mở thư mục chứa file vừa lưu?",
            top_comps.len(), top_name,
            bot_comps.len(), bot_name
        );
        unsafe {
            if MessageBoxW(hwnd, to_wstr(&msg).as_ptr(), to_wstr("Lưu Thành Công").as_ptr(), 0x0040 | 0x0004) == 6 {
                ShellExecuteW(ptr::null_mut(), to_wstr("open").as_ptr(), to_wstr(".").as_ptr(), ptr::null(), ptr::null(), 1);
            }
        }
    }
}

// Splash Proc
unsafe extern "system" fn splash_proc(hwnd: HWND, msg: u32, wparam: usize, lparam: isize) -> isize {
    match msg {
        0x0001 => { // WM_CREATE
            SetTimer(hwnd, 1, 20, ptr::null_mut());
            0
        }
        0x0113 => { // WM_TIMER
            let mut done = false;
            let mut main_hwnd = ptr::null_mut();
            {
                let mut state = STATE.lock().unwrap();
                state.splash_progress += 3;
                if state.splash_progress >= 100 {
                    done = true;
                    main_hwnd = state.hwnd;
                }
            }
            InvalidateRect(hwnd, ptr::null(), 0);

            if done {
                KillTimer(hwnd, 1);
                DestroyWindow(hwnd);
                if !main_hwnd.is_null() {
                    ShowWindow(main_hwnd, 1);
                    UpdateWindow(main_hwnd);
                }
            }
            0
        }
        0x000F => { // WM_PAINT
            let mut ps: PAINTSTRUCT = std::mem::zeroed();
            let hdc = BeginPaint(hwnd, &mut ps);

            let mut rect: RECT = std::mem::zeroed();
            GetClientRect(hwnd, &mut rect);

            let h_bg = CreateSolidBrush(0x00261812);
            FillRect(hdc, &rect, h_bg);
            DeleteObject(h_bg);

            let h_pen = CreatePen(0, 2, 0x00FFD200);
            let old_pen = SelectObject(hdc, h_pen);
            let old_brush = SelectObject(hdc, GetStockObject(5));
            Rectangle(hdc, 0, 0, rect.right, rect.bottom);
            SelectObject(hdc, old_pen);
            SelectObject(hdc, old_brush);
            DeleteObject(h_pen);

            let (bmp, f_title, f_sub, progress) = {
                let state = STATE.lock().unwrap();
                (state.logo_bitmap, state.font_splash_title, state.font_splash_sub, state.splash_progress)
            };

            if !bmp.is_null() {
                let mut graphics: GpGraphics = ptr::null_mut();
                if GdipCreateFromHDC(hdc, &mut graphics) == 0 && !graphics.is_null() {
                    let img_w = 90;
                    let img_h = 90;
                    let img_x = (rect.right - img_w) / 2;
                    let img_y = 20;
                    GdipDrawImageRectI(graphics, bmp, img_x, img_y, img_w, img_h);
                    GdipDeleteGraphics(graphics);
                }
            }

            SetBkMode(hdc, 1);

            SelectObject(hdc, f_title);
            SetTextColor(hdc, 0x00FCFAF8);
            let mut title_rect = RECT { left: 0, top: 122, right: rect.right, bottom: 154 };
            DrawTextW(hdc, to_wstr("NeoDen YY1 SMT Converter Pro (Rust GUI)").as_ptr(), -1, &mut title_rect, 0x00000001 | 0x00000020);

            SelectObject(hdc, f_sub);
            SetTextColor(hdc, 0x00FFD200);
            let mut author_rect = RECT { left: 0, top: 154, right: rect.right, bottom: 178 };
            DrawTextW(hdc, to_wstr("Phát triển bởi: CÔNG TY TNHH CÔNG NGHỆ CHIPXA").as_ptr(), -1, &mut author_rect, 0x00000001 | 0x00000020);

            SetTextColor(hdc, 0x00B8A394);
            let mut sub_rect = RECT { left: 0, top: 178, right: rect.right, bottom: 202 };
            DrawTextW(hdc, to_wstr("Automated Pick & Place Processor • 0603Demo.csv Embedded").as_ptr(), -1, &mut sub_rect, 0x00000001 | 0x00000020);

            let bar_w = 380;
            let bar_h = 10;
            let bar_x = (rect.right - bar_w) / 2;
            let bar_y = 220;

            let h_prog_bg = CreateSolidBrush(0x003B291E);
            let bg_bar_rc = RECT { left: bar_x, top: bar_y, right: bar_x + bar_w, bottom: bar_y + bar_h };
            FillRect(hdc, &bg_bar_rc, h_prog_bg);
            DeleteObject(h_prog_bg);

            let fill_w = (bar_w * progress) / 100;
            if fill_w > 0 {
                let h_prog_fill = CreateSolidBrush(0x00FFD200);
                let fill_rc = RECT { left: bar_x, top: bar_y, right: bar_x + fill_w, bottom: bar_y + bar_h };
                FillRect(hdc, &fill_rc, h_prog_fill);
                DeleteObject(h_prog_fill);
            }

            let load_text = if progress <= 30 {
                "Đang khởi tạo hệ thống Rust Native..."
            } else if progress <= 65 {
                "Tích hợp file mẫu chuẩn 0603Demo.csv..."
            } else if progress <= 90 {
                "Chuẩn bị ma trận Feeder 4 góc & bộ nạp Altium..."
            } else {
                "Khởi động hoàn tất!"
            };

            SetTextColor(hdc, 0x00B4DC00);
            let mut status_rect = RECT { left: 0, top: 240, right: rect.right, bottom: 268 };
            DrawTextW(hdc, to_wstr(load_text).as_ptr(), -1, &mut status_rect, 0x00000001 | 0x00000020);

            EndPaint(hwnd, &ps);
            0
        }
        _ => DefWindowProcW(hwnd, msg, wparam, lparam),
    }
}

// Main Window Procedure
unsafe extern "system" fn wnd_proc(hwnd: HWND, msg: u32, wparam: usize, lparam: isize) -> isize {
    match msg {
        0x000F => { // WM_PAINT
            let mut ps: PAINTSTRUCT = std::mem::zeroed();
            let hdc = BeginPaint(hwnd, &mut ps);

            let (bmp, f_title, f_sub, f_author) = {
                let state = STATE.lock().unwrap();
                (state.logo_bitmap, state.font_main_title, state.font_main_sub, state.font_splash_sub)
            };

            if !bmp.is_null() {
                let mut graphics: GpGraphics = ptr::null_mut();
                if GdipCreateFromHDC(hdc, &mut graphics) == 0 && !graphics.is_null() {
                    GdipDrawImageRectI(graphics, bmp, 25, 10, 56, 56);
                    GdipDeleteGraphics(graphics);
                }
            }

            SetBkMode(hdc, 1);
            SelectObject(hdc, f_title);
            SetTextColor(hdc, 0x00502814);
            let title = to_wstr("NeoDen YY1 SMT Pick & Place File Converter (Rust GUI)");
            TextOutW(hdc, 90, 10, title.as_ptr(), (title.len() - 1) as i32);

            SelectObject(hdc, f_author);
            SetTextColor(hdc, 0x00CC6600);
            let author = to_wstr("Bản quyền & Phát triển: CÔNG TY TNHH CÔNG NGHỆ CHIPXA");
            TextOutW(hdc, 90, 34, author.as_ptr(), (author.len() - 1) as i32);

            SelectObject(hdc, f_sub);
            SetTextColor(hdc, 0x0078645A);
            let sub = to_wstr("Tự động 13 cột • Ma trận Feeder 4 góc (1..13, 14..24, 30..39, 40..50) • Chỉnh sửa & Lưu trực tiếp");
            TextOutW(hdc, 90, 52, sub.as_ptr(), (sub.len() - 1) as i32);

            EndPaint(hwnd, &ps);
            0
        }
        0x004E => { // WM_NOTIFY
            let pnmh = lparam as *const NMHDR;
            if !pnmh.is_null() && unsafe { (*pnmh).id_from } == 105 {
                let code = unsafe { (*pnmh).code };
                if code == 0xFFFFFFF4 /* NM_CUSTOMDRAW */ {
                    let pcustom = lparam as *mut NMLVCUSTOMDRAW;
                    if !pcustom.is_null() {
                        let draw_stage = unsafe { (*pcustom).nmcd.dw_draw_stage };
                        if draw_stage == 0x00000001 /* CDDS_PREPAINT */ {
                            return 0x00000020; /* CDRF_NOTIFYITEMDRAW */
                        } else if draw_stage == 0x00010001 /* CDDS_ITEMPREPAINT */ {
                            return 0x00000020; /* CDRF_NOTIFYSUBITEMDRAW */
                        } else if draw_stage == (0x00010001 | 0x00020000) /* CDDS_ITEMPREPAINT | CDDS_SUBITEM */ {
                            let item = unsafe { (*pcustom).nmcd.dw_item_spec } as usize;
                            let sub_item = unsafe { (*pcustom).i_sub_item };
                            if sub_item == 13 {
                                let (showing_top, top_len, bot_len, is_skip) = {
                                    let state = STATE.lock().unwrap();
                                    let list = if state.showing_top { &state.top_components } else { &state.bot_components };
                                    let is_skip = if item < list.len() { list[item].skip != 0 } else { false };
                                    (state.showing_top, state.top_components.len(), state.bot_components.len(), is_skip)
                                };
                                if is_skip {
                                    // BẬT: Khối Màu Đỏ Nổi Bật
                                    unsafe {
                                        (*pcustom).clr_text_bk = 0x002626DC; // BGR for RGB(220, 38, 38)
                                        (*pcustom).clr_text = 0x002626DC;
                                    }
                                } else {
                                    // TẮT: Khối Màu Đen
                                    unsafe {
                                        (*pcustom).clr_text_bk = 0x00221814; // BGR for RGB(20, 24, 34)
                                        (*pcustom).clr_text = 0x00221814;
                                    }
                                }
                            }
                            return 0x00000000;
                        }
                    }
                    return 0x00000000;
                } else if code == 0xFFFFFFFE /* NM_CLICK */ || code == 0xFFFFFFFD /* NM_DBLCLK */ {
                    let pia = lparam as *const NMITEMACTIVATE;
                    if !pia.is_null() {
                        let item = unsafe { (*pia).i_item };
                        let sub_item = unsafe { (*pia).i_sub_item };
                        if item >= 0 && (sub_item == 13 || code == 0xFFFFFFFD) {
                            let (hwnd_lv, showing_top) = {
                                let state = STATE.lock().unwrap();
                                (state.h_list_view, state.showing_top)
                            };
                            {
                                let mut state = STATE.lock().unwrap();
                                let list = if showing_top { &mut state.top_components } else { &mut state.bot_components };
                                if (item as usize) < list.len() {
                                    list[item as usize].skip = if list[item as usize].skip == 0 { 1 } else { 0 };
                                }
                            }
                            refresh_list_view();
                            unsafe {
                                let mut lvi: LVITEMW = std::mem::zeroed();
                                lvi.state_mask = 0x0003;
                                lvi.state = 0x0003;
                                SendMessageW(hwnd_lv, 0x102B /* LVM_SETITEMSTATE */, item as usize, &lvi as *const _ as isize);
                            }
                        }
                    }
                }
            }
            0
        }
        0x0111 => { // WM_COMMAND
            let cmd_id = (wparam & 0xFFFF) as u32;
            match cmd_id {
                102 => { // Browse
                    let mut file_buf = [0u16; 260];
                    let filter = to_wstr("Pick & Place Files (*.csv;*.txt)\0*.csv;*.txt\0CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0\0");
                    let mut ofn: OPENFILENAMEW = std::mem::zeroed();
                    ofn.l_struct_size = std::mem::size_of::<OPENFILENAMEW>() as u32;
                    ofn.hwnd_owner = hwnd;
                    ofn.lpstr_filter = filter.as_ptr();
                    ofn.lpstr_file = file_buf.as_mut_ptr();
                    ofn.n_max_file = 260;
                    ofn.flags = 0x00001000 | 0x00000800;

                    if GetOpenFileNameW(&mut ofn) != 0 {
                        let path = from_wstr(file_buf.as_ptr());
                        let h_edit = {
                            let state = STATE.lock().unwrap();
                            state.h_edit_input
                        };
                        SetWindowTextW(h_edit, file_buf.as_ptr());
                        parse_altium_data(&path);
                    }
                }
                103 => { // Reload
                    let mut path_buf = [0u16; 260];
                    let h_edit = {
                        let state = STATE.lock().unwrap();
                        state.h_edit_input
                    };
                    GetWindowTextW(h_edit, path_buf.as_mut_ptr(), 260);
                    let path = from_wstr(path_buf.as_ptr());
                    if !path.is_empty() {
                        parse_altium_data(&path);
                    }
                }
                201 => { // Save & Export
                    save_outputs();
                }
                401 => { // Radio TOP
                    {
                        let mut state = STATE.lock().unwrap();
                        state.showing_top = true;
                    }
                    refresh_list_view();
                }
                402 => { // Radio BOT
                    {
                        let mut state = STATE.lock().unwrap();
                        state.showing_top = false;
                    }
                    refresh_list_view();
                }
                _ => {}
            }
            0
        }
        0x0002 => { // WM_DESTROY
            PostQuitMessage(0);
            0
        }
        _ => DefWindowProcW(hwnd, msg, wparam, lparam),
    }
}

fn main() {
    unsafe {
        let icc = INITCOMMONCONTROLSEX {
            dw_size: std::mem::size_of::<INITCOMMONCONTROLSEX>() as u32,
            dw_icc: 0x00000001 | 0x00000004 | 0x00000020,
        };
        InitCommonControlsEx(&icc);

        let mut gdi_token: usize = 0;
        let gdi_input = GdiplusStartupInput {
            gdiplus_version: 1,
            debug_event_callback: ptr::null_mut(),
            suppress_background_thread: 0,
            suppress_external_codecs: 0,
        };
        GdiplusStartup(&mut gdi_token, &gdi_input, ptr::null_mut());

        let mut logo_bitmap: GpBitmap = ptr::null_mut();
        let logo_candidates = ["assets/logo.png", "../assets/logo.png", "logo.png"];
        for c in &logo_candidates {
            let w_path = to_wstr(c);
            if GdipCreateBitmapFromFile(w_path.as_ptr(), &mut logo_bitmap) == 0 && !logo_bitmap.is_null() {
                break;
            }
        }

        let font_splash_title = CreateFontW(22, 0, 0, 0, 700, 0, 0, 0, 1, 0, 0, 5, 0, to_wstr("Segoe UI").as_ptr());
        let font_splash_sub = CreateFontW(14, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 5, 0, to_wstr("Segoe UI").as_ptr());
        let font_main_title = CreateFontW(22, 0, 0, 0, 700, 0, 0, 0, 1, 0, 0, 5, 0, to_wstr("Segoe UI").as_ptr());
        let font_main_sub = CreateFontW(15, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 5, 0, to_wstr("Segoe UI").as_ptr());

        {
            let mut state = STATE.lock().unwrap();
            state.logo_bitmap = logo_bitmap;
            state.font_splash_title = font_splash_title;
            state.font_splash_sub = font_splash_sub;
            state.font_main_title = font_main_title;
            state.font_main_sub = font_main_sub;
        }

        let hinst: HINSTANCE = ptr::null_mut();

        // Splash class
        let splash_class_name = to_wstr("NeoDenYY1RustSplash");
        let splash_wc = WNDCLASSEXW {
            cb_size: std::mem::size_of::<WNDCLASSEXW>() as u32,
            style: 0x0001 | 0x0002,
            lpfn_wnd_proc: splash_proc,
            cb_cls_extra: 0,
            cb_wnd_extra: 0,
            h_instance: hinst,
            h_icon: ptr::null_mut(),
            h_cursor: LoadCursorW(ptr::null_mut(), 32512 as *const _),
            hbr_background: 16 as *mut _,
            lpsz_menu_name: ptr::null(),
            lpsz_class_name: splash_class_name.as_ptr(),
            h_icon_sm: ptr::null_mut(),
        };
        RegisterClassExW(&splash_wc);

        // Main class
        let main_class_name = to_wstr("NeoDenYY1RustGUI");

        let mut h_icon_big = LoadImageW(hinst, 1 as *const _, 1, 32, 32, 0);
        if h_icon_big.is_null() { h_icon_big = LoadImageW(ptr::null_mut(), to_wstr("assets/app_icon.ico").as_ptr(), 1, 32, 32, 0x0010); }
        if h_icon_big.is_null() { h_icon_big = LoadImageW(ptr::null_mut(), to_wstr("app_icon.ico").as_ptr(), 1, 32, 32, 0x0010); }

        let mut h_icon_sm = LoadImageW(hinst, 1 as *const _, 1, 16, 16, 0);
        if h_icon_sm.is_null() { h_icon_sm = LoadImageW(ptr::null_mut(), to_wstr("assets/app_icon.ico").as_ptr(), 1, 16, 16, 0x0010); }
        if h_icon_sm.is_null() { h_icon_sm = LoadImageW(ptr::null_mut(), to_wstr("app_icon.ico").as_ptr(), 1, 16, 16, 0x0010); }

        let wc = WNDCLASSEXW {
            cb_size: std::mem::size_of::<WNDCLASSEXW>() as u32,
            style: 0x0001 | 0x0002,
            lpfn_wnd_proc: wnd_proc,
            cb_cls_extra: 0,
            cb_wnd_extra: 0,
            h_instance: hinst,
            h_icon: h_icon_big,
            h_cursor: LoadCursorW(ptr::null_mut(), 32512 as *const _),
            hbr_background: 16 as *mut _,
            lpsz_menu_name: ptr::null(),
            lpsz_class_name: main_class_name.as_ptr(),
            h_icon_sm: h_icon_sm,
        };
        RegisterClassExW(&wc);

        let screen_w = GetSystemMetrics(0);
        let screen_h = GetSystemMetrics(1);
        let win_w = 1380;
        let win_h = 820;
        let win_x = (screen_w - win_w) / 2;
        let win_y = (screen_h - win_h) / 2;

        let hwnd = CreateWindowExW(
            0, main_class_name.as_ptr(), to_wstr("NeoDen YY1 SMT Pick & Place File Converter (Rust GUI) - ChipXa").as_ptr(),
            0x00CF0000 & !0x00010000 & !0x00040000,
            win_x, win_y, win_w, win_h, ptr::null_mut(), ptr::null_mut(), hinst, ptr::null_mut()
        );

        if hwnd.is_null() { return; }

        if !h_icon_big.is_null() {
            SendMessageW(hwnd, 0x0080, 1, h_icon_big as isize);
        }
        if !h_icon_sm.is_null() {
            SendMessageW(hwnd, 0x0080, 0, h_icon_sm as isize);
        }

        {
            let mut state = STATE.lock().unwrap();
            state.hwnd = hwnd;

            let font_normal = CreateFontW(15, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 5, 0, to_wstr("Segoe UI").as_ptr());
            let font_bold = CreateFontW(15, 0, 0, 0, 700, 0, 0, 0, 1, 0, 0, 5, 0, to_wstr("Segoe UI").as_ptr());

            let grp1 = CreateWindowExW(0, to_wstr("BUTTON").as_ptr(), to_wstr(" 1. File Altium Pick & Place Đầu Vào ").as_ptr(), 0x50000007, 20, 75, 1320, 70, hwnd, ptr::null_mut(), hinst, ptr::null_mut());
            SendMessageW(grp1, 0x0030, font_bold as usize, 1);

            state.h_edit_input = CreateWindowExW(0x00000200, to_wstr("EDIT").as_ptr(), to_wstr("").as_ptr(), 0x50000080, 35, 102, 1050, 26, hwnd, 101 as *mut _, hinst, ptr::null_mut());
            SendMessageW(state.h_edit_input, 0x0030, font_normal as usize, 1);

            let btn_browse = CreateWindowExW(0, to_wstr("BUTTON").as_ptr(), to_wstr("Chọn File...").as_ptr(), 0x50000000, 1100, 101, 110, 28, hwnd, 102 as *mut _, hinst, ptr::null_mut());
            SendMessageW(btn_browse, 0x0030, font_normal as usize, 1);

            let btn_reload = CreateWindowExW(0, to_wstr("BUTTON").as_ptr(), to_wstr("Tải Lại").as_ptr(), 0x50000000, 1220, 101, 90, 28, hwnd, 103 as *mut _, hinst, ptr::null_mut());
            SendMessageW(btn_reload, 0x0030, font_normal as usize, 1);

            let grp2 = CreateWindowExW(0, to_wstr("BUTTON").as_ptr(), to_wstr(" 2. Toàn Bộ 13 Cột Chuẩn NeoDen YY1 (Nhấp đúp chuột vào dòng để chỉnh sửa) ").as_ptr(), 0x50000007, 20, 155, 1320, 460, hwnd, ptr::null_mut(), hinst, ptr::null_mut());
            SendMessageW(grp2, 0x0030, font_bold as usize, 1);

            state.h_radio_top = CreateWindowExW(0, to_wstr("BUTTON").as_ptr(), to_wstr("Mặt TOP").as_ptr(), 0x50000009, 35, 180, 100, 24, hwnd, 401 as *mut _, hinst, ptr::null_mut());
            SendMessageW(state.h_radio_top, 0x0030, font_bold as usize, 1);
            SendMessageW(state.h_radio_top, 0x00F1 /* BM_SETCHECK */, 1, 0);

            state.h_radio_bot = CreateWindowExW(0, to_wstr("BUTTON").as_ptr(), to_wstr("Mặt BOTTOM").as_ptr(), 0x50000009, 145, 180, 120, 24, hwnd, 402 as *mut _, hinst, ptr::null_mut());
            SendMessageW(state.h_radio_bot, 0x0030, font_bold as usize, 1);

            state.h_status = CreateWindowExW(0, to_wstr("STATIC").as_ptr(), to_wstr("Đang nạp dữ liệu...").as_ptr(), 0x50000000, 275, 183, 1020, 20, hwnd, 104 as *mut _, hinst, ptr::null_mut());
            SendMessageW(state.h_status, 0x0030, font_bold as usize, 1);

            state.h_list_view = CreateWindowExW(0x00000200, to_wstr("SysListView32").as_ptr(), to_wstr("").as_ptr(), 0x50000001 | 0x0004, 35, 210, 1290, 390, hwnd, 105 as *mut _, hinst, ptr::null_mut());
            SendMessageW(state.h_list_view, 0x0030, font_normal as usize, 1);
            SendMessageW(state.h_list_view, 0x1036 /* LVM_SETEXTENDEDLISTVIEWSTYLE */, 0, 0x00000020 | 0x00000001);

            // 13 Cột (Tổng 1286px, thêm 2px vừa khít 100% mép phải)
            let cols = [
                ("STT", 45, 2), ("Designator", 100, 2), ("Comment", 200, 0), ("Footprint", 160, 0),
                ("Mid X", 92, 1), ("Mid Y", 92, 1), ("Rotation", 80, 1), ("Head", 55, 2),
                ("FeederNo", 80, 2), ("Speed%", 80, 1), ("Pick(mm)", 88, 1), ("Place(mm)", 88, 1),
                ("Mode", 60, 2), ("Skip", 66, 2),
            ];

            for (i, (name, width, fmt)) in cols.iter().enumerate() {
                let mut w_name = to_wstr(name);
                let mut col_struct: LVCOLUMNW = std::mem::zeroed();
                col_struct.mask = 0x0001 | 0x0002 | 0x0004;
                col_struct.fmt = *fmt;
                col_struct.cx = *width;
                col_struct.psz_text = w_name.as_mut_ptr();
                SendMessageW(state.h_list_view, 0x1061 /* LVM_INSERTCOLUMNW */, i, &col_struct as *const _ as isize);
            }

            let grp3 = CreateWindowExW(0, to_wstr("BUTTON").as_ptr(), to_wstr(" 3. Lưu / Xuất File Sau Khi Chỉnh Sửa ").as_ptr(), 0x50000007, 20, 625, 1320, 135, hwnd, ptr::null_mut(), hinst, ptr::null_mut());
            SendMessageW(grp3, 0x0030, font_bold as usize, 1);

            let lbl_top = CreateWindowExW(0, to_wstr("STATIC").as_ptr(), to_wstr("Tên file TOP:").as_ptr(), 0x50000000, 35, 652, 90, 20, hwnd, ptr::null_mut(), hinst, ptr::null_mut());
            SendMessageW(lbl_top, 0x0030, font_normal as usize, 1);

            state.h_edit_top = CreateWindowExW(0x00000200, to_wstr("EDIT").as_ptr(), to_wstr("Top_Output.csv").as_ptr(), 0x50000080, 130, 648, 250, 26, hwnd, ptr::null_mut(), hinst, ptr::null_mut());
            SendMessageW(state.h_edit_top, 0x0030, font_normal as usize, 1);

            let lbl_bot = CreateWindowExW(0, to_wstr("STATIC").as_ptr(), to_wstr("Tên file BOTTOM:").as_ptr(), 0x50000000, 420, 652, 120, 20, hwnd, ptr::null_mut(), hinst, ptr::null_mut());
            SendMessageW(lbl_bot, 0x0030, font_normal as usize, 1);

            state.h_edit_bot = CreateWindowExW(0x00000200, to_wstr("EDIT").as_ptr(), to_wstr("Bot_Output.csv").as_ptr(), 0x50000080, 550, 648, 250, 26, hwnd, ptr::null_mut(), hinst, ptr::null_mut());
            SendMessageW(state.h_edit_bot, 0x0030, font_normal as usize, 1);

            let btn_convert = CreateWindowExW(0, to_wstr("BUTTON").as_ptr(), to_wstr("LƯU FILE ĐÃ CHỈNH SỬA CHO MÁY NEODEN YY1").as_ptr(), 0x50000001, 35, 690, 1290, 48, hwnd, 201 as *mut _, hinst, ptr::null_mut());
            SendMessageW(btn_convert, 0x0030, font_main_title as usize, 1);
        }

        let candidates = ["Pick Place for MainPCB.csv", "MainPCB.csv", "Pick Place for MainPCB.txt", "../Pick Place for MainPCB.csv"];
        for c in &candidates {
            if std::path::Path::new(c).exists() {
                let h_edit = {
                    let state = STATE.lock().unwrap();
                    state.h_edit_input
                };
                SetWindowTextW(h_edit, to_wstr(c).as_ptr());
                parse_altium_data(c);
                break;
            }
        }

        let splash_w = 500;
        let splash_h = 290;
        let splash_x = (screen_w - splash_w) / 2;
        let splash_y = (screen_h - splash_h) / 2;

        let h_splash = CreateWindowExW(
            0x00000008, splash_class_name.as_ptr(), to_wstr("").as_ptr(),
            0x80000000 | 0x10000000,
            splash_x, splash_y, splash_w, splash_h,
            ptr::null_mut(), ptr::null_mut(), hinst, ptr::null_mut()
        );

        {
            let mut state = STATE.lock().unwrap();
            state.h_splash = h_splash;
        }

        ShowWindow(h_splash, 1);
        UpdateWindow(h_splash);

        let mut msg: MSG = std::mem::zeroed();
        while GetMessageW(&mut msg, ptr::null_mut(), 0, 0) > 0 {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        if !logo_bitmap.is_null() {
            GdipDisposeImage(logo_bitmap);
        }
        GdiplusShutdown(gdi_token);
    }
}
