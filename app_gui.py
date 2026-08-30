import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import os
import sys
import re
import json
from PIL import Image, ImageTk

# Quản lý cấu hình 4 góc khay Feeder NeoDen YY1
DEFAULT_FEEDER_DATA = {
    "1": {"comment": "100nF", "footprint": "0603", "head": 0, "speed": 100},
    "2": {"comment": "10k", "footprint": "0603", "head": 0, "speed": 100},
    "3": {"comment": "1k", "footprint": "0603", "head": 0, "speed": 100},
    "4": {"comment": "4.7k", "footprint": "0603", "head": 0, "speed": 100},
    "5": {"comment": "0R", "footprint": "0603", "head": 0, "speed": 100},
    "6": {"comment": "22pF", "footprint": "0603", "head": 0, "speed": 100},
    "7": {"comment": "1uF", "footprint": "0603", "head": 0, "speed": 100},
    "8": {"comment": "10uF", "footprint": "0805", "head": 0, "speed": 100},
    "9": {"comment": "47uF", "footprint": "0805", "head": 0, "speed": 100},
    "10": {"comment": "LED_RED", "footprint": "0603", "head": 0, "speed": 100},
    "11": {"comment": "LED_GREEN", "footprint": "0603", "head": 0, "speed": 100},
    "12": {"comment": "100k", "footprint": "0603", "head": 0, "speed": 100},
    "13": {"comment": "2.2k", "footprint": "0603", "head": 0, "speed": 100},
    # Top-Left 14..24
    "14": {"comment": "", "footprint": "0603", "head": 0, "speed": 100},
    "15": {"comment": "", "footprint": "0603", "head": 0, "speed": 100},
    "16": {"comment": "", "footprint": "0603", "head": 0, "speed": 100},
    "17": {"comment": "", "footprint": "0805", "head": 0, "speed": 100},
    "18": {"comment": "", "footprint": "0805", "head": 0, "speed": 100},
    "19": {"comment": "", "footprint": "0805", "head": 0, "speed": 100},
    "20": {"comment": "", "footprint": "1206", "head": 0, "speed": 100},
    "21": {"comment": "", "footprint": "1206", "head": 0, "speed": 100},
    "22": {"comment": "", "footprint": "SOT-23", "head": 0, "speed": 100},
    "23": {"comment": "", "footprint": "SOT-23", "head": 0, "speed": 100},
    "24": {"comment": "", "footprint": "SOD-123", "head": 0, "speed": 100},
    # Bottom-Right 30..39
    "30": {"comment": "SS34", "footprint": "SMA", "head": 0, "speed": 100},
    "31": {"comment": "1N4148", "footprint": "SOD-123", "head": 0, "speed": 100},
    "32": {"comment": "S8050", "footprint": "SOT-23", "head": 0, "speed": 100},
    "33": {"comment": "S8550", "footprint": "SOT-23", "head": 0, "speed": 100},
    "34": {"comment": "AMS1117-3.3", "footprint": "SOT-223", "head": 0, "speed": 90},
    "35": {"comment": "AMS1117-5.0", "footprint": "SOT-223", "head": 0, "speed": 90},
    "36": {"comment": "BSS138", "footprint": "SOT-23", "head": 0, "speed": 100},
    "37": {"comment": "AO3400", "footprint": "SOT-23", "head": 0, "speed": 100},
    "38": {"comment": "AO3401", "footprint": "SOT-23", "head": 0, "speed": 100},
    "39": {"comment": "CH340C", "footprint": "SOP-16", "head": 0, "speed": 90},
    # Top-Right 40..50
    "40": {"comment": "", "footprint": "SOP-8", "head": 0, "speed": 100},
    "41": {"comment": "", "footprint": "SOP-8", "head": 0, "speed": 100},
    "42": {"comment": "", "footprint": "SOP-14", "head": 0, "speed": 100},
    "43": {"comment": "", "footprint": "SOP-16", "head": 0, "speed": 100},
    "44": {"comment": "", "footprint": "QFN-20", "head": 0, "speed": 90},
    "45": {"comment": "", "footprint": "QFN-32", "head": 0, "speed": 90},
    "46": {"comment": "", "footprint": "LQFP-48", "head": 0, "speed": 80},
    "47": {"comment": "", "footprint": "SMA", "head": 0, "speed": 100},
    "48": {"comment": "", "footprint": "SMB", "head": 0, "speed": 100},
    "49": {"comment": "", "footprint": "SMC", "head": 0, "speed": 100},
    "50": {"comment": "", "footprint": "ELECTRO", "head": 0, "speed": 80},
}

class SplashScreen:
    def __init__(self, root, callback):
        self.root = root
        self.callback = callback
        
        self.splash = tk.Toplevel(root)
        self.splash.overrideredirect(True)
        
        w, h = 540, 360
        sw = root.winfo_screenwidth()
        sh = root.winfo_screenheight()
        x = (sw - w) // 2
        y = (sh - h) // 2
        self.splash.geometry(f"{w}x{h}+{x}+{y}")
        self.splash.configure(bg="#121826")
        
        base_dir = os.path.dirname(os.path.abspath(__file__)) if not getattr(sys, 'frozen', False) else os.path.dirname(sys.executable)
        logo_path = os.path.join(base_dir, "assets", "logo.png")
        
        frame = tk.Frame(self.splash, bg="#121826", highlightbackground="#00D2FF", highlightthickness=2)
        frame.pack(fill=tk.BOTH, expand=True, padx=2, pady=2)
        
        self.logo_img = None
        if os.path.exists(logo_path):
            try:
                pil_img = Image.open(logo_path).resize((90, 90), Image.Resampling.LANCZOS)
                self.logo_img = ImageTk.PhotoImage(pil_img)
                tk.Label(frame, image=self.logo_img, bg="#121826").pack(pady=(20, 8))
            except Exception as e:
                print("Lỗi tải logo splash:", e)
                
        tk.Label(frame, text="NeoDen YY1 SMT Converter Pro", font=("Segoe UI", 16, "bold"), fg="#F8FAFC", bg="#121826").pack()
        tk.Label(frame, text="Phát triển bởi: CÔNG TY TNHH CÔNG NGHỆ CHIPXA", font=("Segoe UI", 9, "bold"), fg="#00D2FF", bg="#121826").pack(pady=(2, 2))
        tk.Label(frame, text="Automated Pick & Place Processor • 0603Demo.csv Embedded", font=("Segoe UI", 9), fg="#94A3B8", bg="#121826").pack(pady=(0, 16))
        
        style = ttk.Style()
        style.theme_use('clam')
        style.configure("Splash.Horizontal.TProgressbar", foreground="#00D2FF", background="#00D2FF", troughcolor="#1E293B", bordercolor="#121826", lightcolor="#00D2FF", darkcolor="#00D2FF")
        
        self.progress = ttk.Progressbar(frame, style="Splash.Horizontal.TProgressbar", orient="horizontal", length=390, mode="determinate")
        self.progress.pack(pady=(0, 8))
        
        self.status_lbl = tk.Label(frame, text="Đang khởi tạo hệ thống...", font=("Segoe UI", 9, "italic"), fg="#00D2FF", bg="#121826")
        self.status_lbl.pack()
        
        self.progress_val = 0
        self.update_progress()
        
    def update_progress(self):
        self.progress_val += 3
        self.progress["value"] = self.progress_val
        
        if self.progress_val < 35:
            self.status_lbl.config(text="Đang khởi tạo hệ thống...")
        elif self.progress_val < 70:
            self.status_lbl.config(text="Tích hợp file mẫu chuẩn 0603Demo.csv...")
        elif self.progress_val < 95:
            self.status_lbl.config(text="Chuẩn bị ma trận Feeder 4 góc & bộ nạp Altium...")
        else:
            self.status_lbl.config(text="Sẵn sàng!")
            
        if self.progress_val >= 100:
            self.splash.destroy()
            self.callback()
        else:
            self.splash.after(25, self.update_progress)


class FeederMatrixDialog(tk.Toplevel):
    """Hộp thoại cấu hình Ma trận 4 Góc Khay Feeder NeoDen YY1"""
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app
        self.title("⚙️ Cấu Hình Ma Trận 4 Góc Khay Feeder - Máy NeoDen YY1")
        self.geometry("1100x720")
        self.minsize(980, 640)
        self.configure(bg="#0F172A")
        self.transient(parent)
        self.grab_set()
        
        self.entries = {}
        self.setup_ui()
        
    def setup_ui(self):
        # Tiêu đề
        hdr = tk.Frame(self, bg="#0F172A", padx=15, pady=10)
        hdr.pack(fill=tk.X)
        tk.Label(hdr, text="BẢNG THIẾT LẬP 4 GÓC KHAY FEEDER (NEODEN YY1 FEEDER MATRIX)", font=("Segoe UI", 13, "bold"), fg="#38BDF8", bg="#0F172A").pack(anchor=tk.W)
        tk.Label(hdr, text="Cấu hình trước linh kiện được gắn trên từng khay Feeder. Khi nạp file Altium, phần mềm tự động gán chính xác số khay.", font=("Segoe UI", 9), fg="#94A3B8", bg="#0F172A").pack(anchor=tk.W)
        
        # Container 4 góc
        grid_frame = tk.Frame(self, bg="#0F172A", padx=10, pady=5)
        grid_frame.pack(fill=tk.BOTH, expand=True)
        grid_frame.rowconfigure(0, weight=1)
        grid_frame.rowconfigure(1, weight=1)
        grid_frame.columnconfigure(0, weight=1)
        grid_frame.columnconfigure(1, weight=1)
        
        # 4 Quadrants
        self.build_quadrant(grid_frame, 0, 0, "GÓC TRÊN BÊN TRÁI (Khay 14 → 24)", range(14, 25), "#38BDF8", "#0369A1")
        self.build_quadrant(grid_frame, 0, 1, "GÓC TRÊN BÊN PHẢI (Khay 40 → 50)", range(40, 51), "#F472B6", "#BE185D")
        self.build_quadrant(grid_frame, 1, 0, "GÓC DƯỚI BÊN TRÁI (Khay 1 → 13)", range(1, 14), "#4ADE80", "#15803D")
        self.build_quadrant(grid_frame, 1, 1, "GÓC DƯỚI BÊN PHẢI (Khay 30 → 39)", range(30, 40), "#FBBF24", "#B45309")
        
        # Footer buttons
        btn_bar = tk.Frame(self, bg="#1E293B", padx=15, pady=10)
        btn_bar.pack(fill=tk.X)
        
        tk.Button(btn_bar, text="💾 LƯU CẤU HÌNH & ÁP DỤNG NGAY", font=("Segoe UI", 10, "bold"), bg="#10B981", fg="white", padx=15, pady=6, bd=0, command=self.save_and_apply).pack(side=tk.RIGHT, padx=5)
        tk.Button(btn_bar, text="🔄 Khôi Phục Mặc Định", font=("Segoe UI", 9), bg="#475569", fg="white", padx=10, pady=6, bd=0, command=self.reset_default).pack(side=tk.RIGHT, padx=5)
        tk.Button(btn_bar, text="✖ Đóng", font=("Segoe UI", 9), bg="#334155", fg="white", padx=10, pady=6, bd=0, command=self.destroy).pack(side=tk.LEFT)
        
    def build_quadrant(self, parent, row, col, title, feeder_range, border_color, tag_bg):
        frame = tk.LabelFrame(parent, text=f"  {title}  ", font=("Segoe UI", 9, "bold"), fg=border_color, bg="#1E293B", bd=2, relief="groove", padx=5, pady=5)
        frame.grid(row=row, column=col, sticky="nsew", padx=5, pady=5)
        
        # Canvas scrollable
        canvas = tk.Canvas(frame, bg="#1E293B", highlightthickness=0)
        scrollbar = ttk.Scrollbar(frame, orient="vertical", command=canvas.yview)
        scrollable_frame = tk.Frame(canvas, bg="#1E293B")
        
        scrollable_frame.bind("<Configure>", lambda e: canvas.configure(scrollregion=canvas.bbox("all")))
        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)
        
        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        
        # Header bảng
        h_row = tk.Frame(scrollable_frame, bg="#334155", pady=2)
        h_row.pack(fill=tk.X, pady=(0, 2))
        tk.Label(h_row, text="Khay", width=5, font=("Segoe UI", 8, "bold"), fg="white", bg="#334155").pack(side=tk.LEFT, padx=1)
        tk.Label(h_row, text="Linh Kiện (Comment)", width=15, font=("Segoe UI", 8, "bold"), fg="white", bg="#334155").pack(side=tk.LEFT, padx=1)
        tk.Label(h_row, text="Footprint", width=10, font=("Segoe UI", 8, "bold"), fg="white", bg="#334155").pack(side=tk.LEFT, padx=1)
        tk.Label(h_row, text="Head", width=5, font=("Segoe UI", 8, "bold"), fg="white", bg="#334155").pack(side=tk.LEFT, padx=1)
        tk.Label(h_row, text="Tốc độ%", width=6, font=("Segoe UI", 8, "bold"), fg="white", bg="#334155").pack(side=tk.LEFT, padx=1)
        
        for f_no in feeder_range:
            f_str = str(f_no)
            cur = self.app.feeder_matrix.get(f_str, {"comment": "", "footprint": "0603", "head": 0, "speed": 100})
            
            r = tk.Frame(scrollable_frame, bg="#1E293B", pady=1)
            r.pack(fill=tk.X)
            
            # Badge số khay
            lbl_no = tk.Label(r, text=f"#{f_no}", width=5, font=("Segoe UI", 8, "bold"), fg="white", bg=tag_bg)
            lbl_no.pack(side=tk.LEFT, padx=1)
            
            e_cmt = tk.Entry(r, width=15, font=("Segoe UI", 8), bg="#0F172A", fg="#F8FAFC", insertbackground="white")
            e_cmt.insert(0, cur.get("comment", ""))
            e_cmt.pack(side=tk.LEFT, padx=1)
            
            e_fp = tk.Entry(r, width=10, font=("Segoe UI", 8), bg="#0F172A", fg="#F8FAFC", insertbackground="white")
            e_fp.insert(0, cur.get("footprint", ""))
            e_fp.pack(side=tk.LEFT, padx=1)
            
            e_head = ttk.Combobox(r, values=["0", "1"], width=3, font=("Segoe UI", 8), state="readonly")
            e_head.set(str(cur.get("head", 0)))
            e_head.pack(side=tk.LEFT, padx=1)
            
            e_spd = tk.Entry(r, width=6, font=("Segoe UI", 8), bg="#0F172A", fg="#F8FAFC", insertbackground="white")
            e_spd.insert(0, str(cur.get("speed", 100)))
            e_spd.pack(side=tk.LEFT, padx=1)
            
            self.entries[f_str] = {
                "comment": e_cmt,
                "footprint": e_fp,
                "head": e_head,
                "speed": e_spd
            }
            
    def save_and_apply(self):
        new_matrix = {}
        for f_str, widgets in self.entries.items():
            try: spd = int(widgets["speed"].get().strip())
            except: spd = 100
            try: hd = int(widgets["head"].get().strip())
            except: hd = 0
            
            new_matrix[f_str] = {
                "comment": widgets["comment"].get().strip(),
                "footprint": widgets["footprint"].get().strip(),
                "head": hd,
                "speed": spd
            }
            
        self.app.feeder_matrix = new_matrix
        self.app.save_feeder_matrix_file()
        self.app.apply_feeder_assignments_to_components()
        messagebox.showinfo("Thành Công", "Đã lưu ma trận 4 góc khay Feeder và cập nhật toàn bộ bảng dữ liệu!", parent=self)
        self.destroy()
        
    def reset_default(self):
        if messagebox.askyesno("Xác nhận", "Khôi phục toàn bộ bảng Feeder về mặc định ban đầu?", parent=self):
            self.app.feeder_matrix = json.loads(json.dumps(DEFAULT_FEEDER_DATA))
            for f_str, widgets in self.entries.items():
                cur = self.app.feeder_matrix.get(f_str, {"comment": "", "footprint": "0603", "head": 0, "speed": 100})
                widgets["comment"].delete(0, tk.END); widgets["comment"].insert(0, cur.get("comment", ""))
                widgets["footprint"].delete(0, tk.END); widgets["footprint"].insert(0, cur.get("footprint", ""))
                widgets["head"].set(str(cur.get("head", 0)))
                widgets["speed"].delete(0, tk.END); widgets["speed"].insert(0, str(cur.get("speed", 100)))


class RowEditDialog(tk.Toplevel):
    """Hộp thoại chỉnh sửa chi tiết toàn bộ 13 thông số của 1 linh kiện"""
    def __init__(self, parent, comp_data, on_save_callback):
        super().__init__(parent)
        self.comp_data = comp_data
        self.on_save_callback = on_save_callback
        self.title(f"✏️ Chỉnh Sửa Linh Kiện [{comp_data.get('designator', '')}]")
        self.geometry("540x580")
        self.resizable(False, False)
        self.configure(bg="#0F172A")
        self.transient(parent)
        self.grab_set()
        
        self.fields = {}
        self.setup_ui()
        
    def setup_ui(self):
        tk.Label(self, text="CHỈNH SỬA TOÀN BỘ 13 THÔNG SỐ CHUẨN NEODEN YY1", font=("Segoe UI", 11, "bold"), fg="#38BDF8", bg="#0F172A", pady=10).pack()
        
        form_frame = tk.Frame(self, bg="#1E293B", padx=15, pady=10, bd=1, relief="ridge")
        form_frame.pack(fill=tk.BOTH, expand=True, padx=15, pady=5)
        
        param_defs = [
            ("1. Designator (Tên LK):", "designator", str),
            ("2. Comment (Giá trị):", "comment", str),
            ("3. Footprint (Kiểu chân):", "footprint", str),
            ("4. Mid X (mm):", "mid_x", float),
            ("5. Mid Y (mm):", "mid_y", float),
            ("6. Rotation (Góc quay °):", "rotation", float),
            ("7. Head (Đầu hút 0/1):", "head", int),
            ("8. FeederNo (Số khay 1..50):", "feeder_no", int),
            ("9. Mount Speed (% Tốc độ):", "mount_speed", int),
            ("10. Pick Height (mm Cao gắp):", "pick_height", float),
            ("11. Place Height (mm Cao đặt):", "place_height", float),
            ("12. Mode (Chế độ Vision 1/0):", "mode", int),
            ("13. Skip (Bỏ qua 0=Gắp, 1=Bỏ):", "skip", int),
        ]
        
        for idx, (label_txt, key, field_type) in enumerate(param_defs):
            row = tk.Frame(form_frame, bg="#1E293B", pady=2)
            row.pack(fill=tk.X)
            
            tk.Label(row, text=label_txt, width=24, font=("Segoe UI", 9), fg="#E2E8F0", bg="#1E293B", anchor="w").pack(side=tk.LEFT)
            val = self.comp_data.get(key, 0 if field_type in (int, float) else "")
            
            e = tk.Entry(row, font=("Segoe UI", 9), bg="#0F172A", fg="#F8FAFC", insertbackground="white")
            if field_type == float:
                e.insert(0, f"{float(val):.2f}")
            else:
                e.insert(0, str(val))
            e.pack(side=tk.RIGHT, fill=tk.X, expand=True)
            self.fields[key] = (e, field_type)
            
        btn_bar = tk.Frame(self, bg="#0F172A", pady=10)
        btn_bar.pack(fill=tk.X, padx=15)
        
        tk.Button(btn_bar, text="💾 LƯU THAY ĐỔI", font=("Segoe UI", 10, "bold"), bg="#10B981", fg="white", padx=20, pady=6, bd=0, command=self.save).pack(side=tk.RIGHT, padx=5)
        tk.Button(btn_bar, text="Hủy", font=("Segoe UI", 9), bg="#475569", fg="white", padx=12, pady=6, bd=0, command=self.destroy).pack(side=tk.RIGHT)
        
    def save(self):
        updated = {}
        for key, (entry, field_type) in self.fields.items():
            raw = entry.get().strip()
            try:
                if field_type == float: updated[key] = float(raw)
                elif field_type == int: updated[key] = int(raw)
                else: updated[key] = raw
            except:
                updated[key] = self.comp_data.get(key, "")
                
        self.on_save_callback(updated)
        self.destroy()


class NeoDenYY1App:
    def __init__(self, root):
        self.root = root
        self.root.withdraw()
        
        self.root.title("NeoDen YY1 SMT Pick & Place File Converter Pro - CÔNG TY TNHH CÔNG NGHỆ CHIPXA")
        self.root.geometry("1380x820")
        self.root.minsize(1200, 750)
        self.root.configure(bg="#121824")
        
        # Center window on screen
        self.root.update_idletasks()
        w = 1380
        h = 820
        x = (self.root.winfo_screenwidth() // 2) - (w // 2)
        y = (self.root.winfo_screenheight() // 2) - (h // 2)
        self.root.geometry(f"{w}x{h}+{x}+{y}")
        
        self.base_dir = os.path.dirname(os.path.abspath(__file__)) if not getattr(sys, 'frozen', False) else os.path.dirname(sys.executable)
        self.logo_path = os.path.join(self.base_dir, "assets", "logo.png")
        self.ico_path = os.path.join(self.base_dir, "assets", "app_icon.ico")
        self.template_path = os.path.join(self.base_dir, "0603Demo.csv")
        self.matrix_file = os.path.join(self.base_dir, "feeder_matrix.json")
        
        if os.path.exists("app_icon.ico"):
            try: self.root.iconbitmap("app_icon.ico")
            except: pass
        elif os.path.exists(self.ico_path):
            try: self.root.iconbitmap(self.ico_path)
            except: pass
            
        self.input_file = tk.StringVar()
        self.top_output_name = tk.StringVar(value="Top_Output.csv")
        self.bot_output_name = tk.StringVar(value="Bot_Output.csv")
        
        # Danh sách linh kiện tách 2 mặt đầy đủ 13 cột
        self.top_components = []
        self.bot_components = []
        self.current_layer = "TOP"
        
        # Nạp ma trận Feeder
        self.feeder_matrix = {}
        self.load_feeder_matrix_file()
        
        self.setup_ui()
        self.auto_detect_file()
        
        SplashScreen(root, self.show_main_window)
        
    def show_main_window(self):
        self.root.deiconify()
        self.root.update()
        
    def load_feeder_matrix_file(self):
        if os.path.exists(self.matrix_file):
            try:
                with open(self.matrix_file, "r", encoding="utf-8") as f:
                    data = json.load(f)
                    if "quadrants" in data:
                        # Parse structured JSON
                        self.feeder_matrix = {}
                        for q_key, q_info in data["quadrants"].items():
                            for f_id, f_val in q_info.get("feeders", {}).items():
                                self.feeder_matrix[str(f_id)] = f_val
                    else:
                        self.feeder_matrix = data
                    return
            except Exception as e:
                print("Lỗi đọc feeder_matrix.json:", e)
        self.feeder_matrix = json.loads(json.dumps(DEFAULT_FEEDER_DATA))
        
    def save_feeder_matrix_file(self):
        try:
            structured = {
                "description": "NeoDen YY1 Feeder Bank 4-Quadrant Configuration",
                "author": "CÔNG TY TNHH CÔNG NGHỆ CHIPXA",
                "quadrants": {
                    "top_left": {"name": "Góc Trên Bên Trái (14..24)", "range": [14, 24], "feeders": {str(k): v for k, v in self.feeder_matrix.items() if 14 <= int(k) <= 24}},
                    "top_right": {"name": "Góc Trên Bên Phải (40..50)", "range": [40, 50], "feeders": {str(k): v for k, v in self.feeder_matrix.items() if 40 <= int(k) <= 50}},
                    "bottom_left": {"name": "Góc Dưới Bên Trái (1..13)", "range": [1, 13], "feeders": {str(k): v for k, v in self.feeder_matrix.items() if 1 <= int(k) <= 13}},
                    "bottom_right": {"name": "Góc Dưới Bên Phải (30..39)", "range": [30, 39], "feeders": {str(k): v for k, v in self.feeder_matrix.items() if 30 <= int(k) <= 39}},
                }
            }
            with open(self.matrix_file, "w", encoding="utf-8") as f:
                json.dump(structured, f, indent=2, ensure_ascii=False)
        except Exception as e:
            print("Lỗi ghi feeder_matrix.json:", e)
            
    def setup_ui(self):
        style = ttk.Style()
        style.theme_use('clam')
        
        bg_dark = "#0F172A"
        card_bg = "#1E293B"
        accent_blue = "#38BDF8"
        text_light = "#F8FAFC"
        
        self.root.configure(bg=bg_dark)
        
        style.configure("Main.TFrame", background=bg_dark)
        style.configure("Card.TFrame", background=card_bg)
        style.configure("Card.TLabelframe", background=card_bg, foreground=accent_blue, font=("Segoe UI", 10, "bold"))
        style.configure("Card.TLabelframe.Label", background=card_bg, foreground=accent_blue, font=("Segoe UI", 10, "bold"))
        
        style.configure("TLabel", background=card_bg, foreground=text_light, font=("Segoe UI", 9))
        style.configure("Header.TLabel", background=bg_dark, foreground=text_light, font=("Segoe UI", 15, "bold"))
        style.configure("Author.TLabel", background=bg_dark, foreground=accent_blue, font=("Segoe UI", 9, "bold"))
        style.configure("SubHeader.TLabel", background=bg_dark, foreground="#94A3B8", font=("Segoe UI", 9))
        
        # Tabs Style
        style.configure("TNotebook", background=card_bg, borderwidth=0)
        style.configure("TNotebook.Tab", font=("Segoe UI", 10, "bold"), padding=[16, 6], background="#334155", foreground="white")
        style.map("TNotebook.Tab", background=[("selected", "#0284C7"), ("active", "#0369A1")])
        
        # Buttons
        style.configure("Action.TButton", font=("Segoe UI", 9, "bold"), background="#0284C7", foreground="white", padding=6)
        style.map("Action.TButton", background=[("active", "#0369A1")])
        
        style.configure("Feeder.TButton", font=("Segoe UI", 10, "bold"), background="#8B5CF6", foreground="white", padding=8)
        style.map("Feeder.TButton", background=[("active", "#7C3AED")])
        
        style.configure("Convert.TButton", font=("Segoe UI", 11, "bold"), background="#10B981", foreground="white", padding=10)
        style.map("Convert.TButton", background=[("active", "#059669")])

        main_container = ttk.Frame(self.root, style="Main.TFrame")
        main_container.pack(fill=tk.BOTH, expand=True, padx=15, pady=10)
        
        # --- HEADER VỚI LOGO ---
        header_frame = ttk.Frame(main_container, style="Main.TFrame")
        header_frame.pack(fill=tk.X, pady=(0, 10))
        
        self.logo_img = None
        if os.path.exists(self.logo_path):
            try:
                pil_img = Image.open(self.logo_path).resize((60, 60), Image.Resampling.LANCZOS)
                self.logo_img = ImageTk.PhotoImage(pil_img)
                logo_label = tk.Label(header_frame, image=self.logo_img, bg=bg_dark)
                logo_label.pack(side=tk.LEFT, padx=(0, 12))
            except Exception as e:
                print("Lỗi tải logo:", e)
                
        title_box = ttk.Frame(header_frame, style="Main.TFrame")
        title_box.pack(side=tk.LEFT, fill=tk.Y)
        
        title_lbl = ttk.Label(title_box, text="NeoDen YY1 SMT Pick & Place File Converter Pro", style="Header.TLabel")
        title_lbl.pack(anchor=tk.W)

        author_lbl = ttk.Label(title_box, text="Bản quyền & Phát triển: CÔNG TY TNHH CÔNG NGHỆ CHIPXA", style="Author.TLabel")
        author_lbl.pack(anchor=tk.W, pady=(1, 0))
        
        sub_lbl = ttk.Label(title_box, text="⚡ Tự động nạp 13 cột • Chỉnh sửa thông số trực tiếp • Thiết lập ma trận khay Feeder 4 góc (1..13, 14..24, 30..39, 40..50)", style="SubHeader.TLabel")
        sub_lbl.pack(anchor=tk.W, pady=(1, 0))
        
        # Nút Cấu Hình Feeder 4 Góc
        btn_feeder = ttk.Button(header_frame, text="⚙️ CẤU HÌNH KHAY FEEDER 4 GÓC", style="Feeder.TButton", command=self.open_feeder_matrix_dialog)
        btn_feeder.pack(side=tk.RIGHT, padx=5, pady=5)
        
        # --- KHỐI 1: CHỌN FILE ALTIUM ---
        input_frame = ttk.LabelFrame(main_container, text="  1. File Altium Pick & Place Đầu Vào  ", style="Card.TLabelframe", padding=10)
        input_frame.pack(fill=tk.X, pady=(0, 8))
        
        entry_box = ttk.Entry(input_frame, textvariable=self.input_file, font=("Segoe UI", 10))
        entry_box.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 10))
        
        btn_browse = ttk.Button(input_frame, text="📁 Chọn File...", style="Action.TButton", command=self.browse_file)
        btn_browse.pack(side=tk.RIGHT, padx=4)
        
        btn_reload = ttk.Button(input_frame, text="🔄 Tải & Chuyển Đổi Lại", style="Action.TButton", command=self.load_altium_data)
        btn_reload.pack(side=tk.RIGHT)
        
        # --- KHỐI 2: TABS TOP/BOTTOM & BẢNG 13 CỘT CHỈNH SỬA TRỰC TIẾP ---
        table_frame = ttk.LabelFrame(main_container, text="  2. Bảng Thông Số 13 Cột Chuẩn NeoDen YY1 (Nhấp đúp vào dòng để sửa)  ", style="Card.TLabelframe", padding=8)
        table_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 8))
        
        # Thanh công cụ bảng
        tb_bar = tk.Frame(table_frame, bg=card_bg, pady=4)
        tb_bar.pack(fill=tk.X)
        
        self.stats_label = tk.Label(tb_bar, text="📊 Đang nạp dữ liệu...", font=("Segoe UI", 9, "bold"), fg="#38BDF8", bg=card_bg)
        self.stats_label.pack(side=tk.LEFT)
        
        tk.Button(tb_bar, text="✏️ Sửa dòng đã chọn", font=("Segoe UI", 8, "bold"), bg="#0284C7", fg="white", bd=0, padx=8, pady=3, command=self.edit_selected_row).pack(side=tk.RIGHT, padx=3)
        tk.Button(tb_bar, text="➕ Thêm linh kiện", font=("Segoe UI", 8), bg="#334155", fg="white", bd=0, padx=8, pady=3, command=self.add_component).pack(side=tk.RIGHT, padx=3)
        tk.Button(tb_bar, text="🗑️ Xóa dòng", font=("Segoe UI", 8), bg="#DC2626", fg="white", bd=0, padx=8, pady=3, command=self.delete_selected_row).pack(side=tk.RIGHT, padx=3)
        
        # Notebook Tabs TOP / BOTTOM
        self.notebook = ttk.Notebook(table_frame)
        self.notebook.pack(fill=tk.BOTH, expand=True, pady=4)
        
        self.tab_top = ttk.Frame(self.notebook, style="Card.TFrame")
        self.tab_bot = ttk.Frame(self.notebook, style="Card.TFrame")
        
        self.notebook.add(self.tab_top, text="  Mặt TOP  ")
        self.notebook.add(self.tab_bot, text="  Mặt BOTTOM  ")
        self.notebook.bind("<<NotebookTabChanged>>", self.on_tab_changed)
        
        # 13 Cột chuẩn NeoDen YY1
        self.cols = (
            "STT", "Designator", "Comment", "Footprint", "Mid X(mm)", "Mid Y(mm)", 
            "Rotation", "Head", "FeederNo", "Mount Speed(%)", "Pick Height(mm)", 
            "Place Height(mm)", "Mode", "Skip"
        )
        
        # Treeview cho TOP
        self.tree_top = self.create_table(self.tab_top)
        # Treeview cho BOTTOM
        self.tree_bot = self.create_table(self.tab_bot)
        
        # --- KHỐI 3: XUẤT FILE ĐÃ CHỈNH SỬA ---
        cfg_frame = ttk.LabelFrame(main_container, text="  3. Lưu / Xuất File Sau Khi Đã Chỉnh Sửa  ", style="Card.TLabelframe", padding=10)
        cfg_frame.pack(fill=tk.X)
        
        row1 = ttk.Frame(cfg_frame, style="Card.TFrame")
        row1.pack(fill=tk.X, pady=2)
        
        ttk.Label(row1, text="Tên file TOP:").pack(side=tk.LEFT, padx=(0, 5))
        ttk.Entry(row1, textvariable=self.top_output_name, width=20, font=("Segoe UI", 9)).pack(side=tk.LEFT, padx=(0, 25))
        
        ttk.Label(row1, text="Tên file BOTTOM:").pack(side=tk.LEFT, padx=(0, 5))
        ttk.Entry(row1, textvariable=self.bot_output_name, width=20, font=("Segoe UI", 9)).pack(side=tk.LEFT)
        
        btn_convert = ttk.Button(cfg_frame, text="💾 LƯU FILE ĐÃ CHỈNH SỬA CHO MÁY NEODEN YY1", style="Convert.TButton", command=self.save_and_export)
        btn_convert.pack(fill=tk.X, pady=(8, 2))
        
    def create_table(self, parent_tab):
        container = ttk.Frame(parent_tab, style="Card.TFrame")
        container.pack(fill=tk.BOTH, expand=True)
        
        tree_scroll_y = ttk.Scrollbar(container, orient="vertical")
        tree_scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
        
        tree_scroll_x = ttk.Scrollbar(container, orient="horizontal")
        tree_scroll_x.pack(side=tk.BOTTOM, fill=tk.X)
        
        tree = ttk.Treeview(container, columns=self.cols, show="headings", 
                            yscrollcommand=tree_scroll_y.set, xscrollcommand=tree_scroll_x.set, height=10)
        tree_scroll_y.config(command=tree.yview)
        tree_scroll_x.config(command=tree.xview)
        
        col_widths = {
            "STT": 45, "Designator": 100, "Comment": 210, "Footprint": 166,
            "Mid X(mm)": 92, "Mid Y(mm)": 92, "Rotation": 80, "Head": 55,
            "FeederNo": 80, "Mount Speed(%)": 80, "Pick Height(mm)": 88,
            "Place Height(mm)": 88, "Mode": 60, "Skip": 50
        }
        
        for col in self.cols:
            tree.heading(col, text=col)
            tree.column(col, width=col_widths.get(col, 80), anchor=tk.CENTER)
        tree.column("Comment", anchor=tk.W)
        tree.column("Footprint", anchor=tk.W)
        
        tree.pack(fill=tk.BOTH, expand=True)
        
        def on_table_click(event):
            region = tree.identify_region(event.x, event.y)
            if region == "cell":
                col = tree.identify_column(event.x)
                if col == "#14":  # Cột 14 là Skip
                    item_id = tree.identify_row(event.y)
                    if item_id:
                        idx = int(item_id.split("_")[1])
                        _, comp_list, _ = self.get_active_tree_and_list()
                        if idx < len(comp_list):
                            comp_list[idx]["skip"] = 0 if comp_list[idx].get("skip", 0) == 1 else 1
                            self.refresh_tables()
                            active_tree, _, _ = self.get_active_tree_and_list()
                            active_tree.selection_remove(active_tree.selection())
                            
        tree.bind("<ButtonRelease-1>", on_table_click)
        tree.bind("<Double-1>", lambda e: self.edit_selected_row())
        return tree
        
    def on_tab_changed(self, event):
        selected_tab = self.notebook.index(self.notebook.select())
        self.current_layer = "TOP" if selected_tab == 0 else "BOTTOM"
        
    def open_feeder_matrix_dialog(self):
        FeederMatrixDialog(self.root, self)
        
    def auto_detect_file(self):
        candidates = ["Pick Place for MainPCB.csv", "MainPCB.csv", "Pick Place for MainPCB.txt", "../Pick Place for MainPCB.csv"]
        for c in candidates:
            p = os.path.join(self.base_dir, c)
            if os.path.exists(p):
                self.input_file.set(p)
                self.load_altium_data()
                break
                
    def browse_file(self):
        filename = filedialog.askopenfilename(
            title="Chọn file Altium Pick and Place",
            filetypes=[("Pick & Place Files", "*.csv;*.txt"), ("CSV Files", "*.csv"), ("All Files", "*.*")]
        )
        if filename:
            self.input_file.set(filename)
            self.load_altium_data()
            
    def normalize_footprint(self, fp):
        """Chuẩn hóa Footprint tiếng Việt / ký hiệu tùy ý sang định dạng tiếng Anh chuẩn máy YY1"""
        if not fp: return "0603D"
        f = fp.strip()
        fl = f.lower()
        
        if "0603" in fl: return "0603D"
        if "0805" in fl: return "0805D"
        if "1206" in fl:
            if "cau_chi" in fl or "fuse" in fl: return "1206_FUSE"
            return "1206D"
        if "0630" in fl: return "IND_0630"
        if "tantalum" in fl or "7443" in fl or "7343" in fl: return "TANTAL_7343"
        if "button_2p" in fl or "nut_nhan_2p" in fl: return "SW_2P_SMD"
        if "button_4p" in fl or "nut_nhan_4p" in fl: return "SW_4P_SMD"
        if "header" in fl or "hdr" in fl:
            if "1.25" in fl: return "HDR_1.25_2P_SMD"
            if "4p" in fl: return "HDR_2.0_4P_SMD"
            if "2p" in fl: return "HDR_2.0_2P_SMD"
        if "sma" in fl: return "SMA"
        if "tesdu" in fl: return "SOD-323"
        if "vr_" in fl: return "POT_SMD"
        if "via" in fl: return "VIA_2.2MM"
        if "sdcard" in fl or "tf3" in fl: return "TF_CARD_SMD"
        if "soic-16" in fl or "sop-16" in fl: return "SOP-16"
        if "typec" in fl or "type-c" in fl: return "USB_TYPE_C"
        return f

    def normalize_comment(self, cmt):
        """Chuẩn hóa giá trị linh kiện sang chuẩn tiếng Anh kỹ thuật không dấu"""
        if not cmt: return ""
        c = cmt.strip()
        cl = c.lower()
        if "cau chi" in cl or "fuse" in cl: return "FUSE_1206"
        if "nut nhan 2p" in cl: return "TACT_SW_2P"
        if "nut nhan 4" in cl: return "TACT_SW_4P"
        if cl == "nguon": return "POWER_HDR"
        if cl == "bomkhi": return "AIR_PUMP"
        if cl == "vankhi": return "AIR_VALVE"
        if "led 0603" in cl or cl == "led": return "LED_0603"
        if "sdcard" in cl or "tf3" in cl: return "MICRO_SD_TF3"
        if "pressure sensor" in cl: return "PRESSURE_SENSOR"
        return c

    def natural_sort_key(self, comp):
        cmt = str(comp.get("comment", "")).lower()
        des = str(comp.get("designator", ""))
        numbers = re.findall(r'\d+', des)
        num = int(numbers[-1]) if numbers else 0
        prefix = re.sub(r'\d+', '', des)
        return (cmt, prefix, num, des)

    def find_feeder_no(self, comment, footprint):
        """Khớp linh kiện vào khay Feeder đã thiết lập theo 4 góc"""
        cmt_clean = comment.strip().lower()
        fp_clean = footprint.strip().lower()
        
        # 1. Tìm chính xác theo Comment
        for f_id, f_cfg in self.feeder_matrix.items():
            if f_cfg.get("comment", "").strip().lower() == cmt_clean and cmt_clean != "":
                return int(f_id), f_cfg.get("head", 0), f_cfg.get("speed", 100)
                
        # 2. Tìm theo Footprint nếu chưa khớp Comment
        for f_id, f_cfg in self.feeder_matrix.items():
            if f_cfg.get("footprint", "").strip().lower() == fp_clean and f_cfg.get("comment", "") == "":
                return int(f_id), f_cfg.get("head", 0), f_cfg.get("speed", 100)
                
        return 1, 0, 100
        
    def load_altium_data(self):
        filepath = self.input_file.get().strip()
        if not filepath or not os.path.exists(filepath):
            return
            
        try:
            with open(filepath, "rb") as f:
                content = f.read().decode("utf-8-sig", errors="ignore")
                
            lines = content.splitlines()
            header_idx = None
            is_mil = False
            
            for i, line in enumerate(lines):
                if "units used: mil" in line.lower():
                    is_mil = True
                if "designator" in line.lower() and "comment" in line.lower():
                    header_idx = i
                    break
                    
            if header_idx is None:
                messagebox.showerror("Lỗi Định Dạng", "Không tìm thấy dòng Header chứa 'Designator' và 'Comment' trong file!")
                return
                
            header_cols = [c.strip().strip('"') for c in lines[header_idx].split(",")]
            col_map = {}
            for idx, c in enumerate(header_cols):
                cl = c.lower()
                if "designator" in cl: col_map["designator"] = idx
                elif "comment" in cl: col_map["comment"] = idx
                elif "layer" in cl: col_map["layer"] = idx
                elif "footprint" in cl: col_map["footprint"] = idx
                elif cl.startswith("center-x") or cl.startswith("mid x"):
                    col_map["x"] = idx
                    if "mil" in cl: is_mil = True
                elif cl.startswith("center-y") or cl.startswith("mid y"):
                    col_map["y"] = idx
                elif cl.startswith("rotation"): col_map["rot"] = idx
                
            raw_top = []
            raw_bot = []
            
            for line in lines[header_idx + 1:]:
                line = line.strip()
                if not line: continue
                
                parts = self.parse_csv_line(line)
                if "designator" not in col_map or col_map["designator"] >= len(parts):
                    continue
                    
                des = parts[col_map["designator"]].strip()
                if not des or des.startswith("*"):
                    continue
                    
                cmt_raw = parts[col_map["comment"]].strip() if "comment" in col_map and col_map["comment"] < len(parts) else ""
                fp_raw = parts[col_map["footprint"]].strip() if "footprint" in col_map and col_map["footprint"] < len(parts) else "0603D"
                layer = parts[col_map["layer"]].strip() if "layer" in col_map and col_map["layer"] < len(parts) else "TopLayer"
                
                cmt = self.normalize_comment(cmt_raw)
                fp = self.normalize_footprint(fp_raw)
                
                try:
                    raw_x = float(parts[col_map["x"]]) if "x" in col_map and col_map["x"] < len(parts) else 0.0
                    raw_y = float(parts[col_map["y"]]) if "y" in col_map and col_map["y"] < len(parts) else 0.0
                    rot = float(parts[col_map["rot"]]) if "rot" in col_map and col_map["rot"] < len(parts) else 0.0
                except:
                    raw_x, raw_y, rot = 0.0, 0.0, 0.0
                    
                mid_x = raw_x * 0.0254 if is_mil else raw_x
                mid_y = raw_y * 0.0254 if is_mil else raw_y
                
                f_no, head, spd = self.find_feeder_no(cmt, fp)
                
                comp = {
                    "designator": des,
                    "comment": cmt,
                    "footprint": fp,
                    "mid_x": mid_x,
                    "mid_y": mid_y,
                    "rotation": rot,
                    "head": head,
                    "feeder_no": f_no,
                    "mount_speed": spd,
                    "pick_height": 0.0,
                    "place_height": 0.0,
                    "mode": 1,
                    "skip": 0,
                    "layer": layer
                }
                
                if "bot" in layer.lower():
                    raw_bot.append(comp)
                else:
                    raw_top.append(comp)
                    
            raw_top.sort(key=self.natural_sort_key)
            raw_bot.sort(key=self.natural_sort_key)
            
            self.top_components = raw_top
            self.bot_components = raw_bot
            
            # Phân phối Feeder tự động nếu chưa có trong matrix
            self.assign_dynamic_feeders(self.top_components)
            self.assign_dynamic_feeders(self.bot_components)
            
            self.refresh_tables()
            
        except Exception as e:
            messagebox.showerror("Lỗi Đọc File", f"Chi tiết: {str(e)}")
            
    def assign_dynamic_feeders(self, comp_list):
        """Gán số khay Feeder tuần tự cho các linh kiện chưa được định trước"""
        assigned_map = {}
        # Các khay ưu tiên góc dưới trái (1..13), trên trái (14..24), dưới phải (30..39), trên phải (40..50)
        available_slots = list(range(1, 14)) + list(range(14, 25)) + list(range(30, 40)) + list(range(40, 51))
        used_slots = set()
        
        for c in comp_list:
            cmt = c["comment"]
            f_no, head, spd = self.find_feeder_no(cmt, c["footprint"])
            if cmt in self.get_configured_comments():
                c["feeder_no"] = f_no
                c["head"] = head
                c["mount_speed"] = spd
                used_slots.add(f_no)
                assigned_map[cmt] = f_no
                
        slot_idx = 0
        for c in comp_list:
            cmt = c["comment"]
            if cmt in assigned_map:
                c["feeder_no"] = assigned_map[cmt]
            else:
                while slot_idx < len(available_slots) and available_slots[slot_idx] in used_slots:
                    slot_idx += 1
                if slot_idx < len(available_slots):
                    chosen = available_slots[slot_idx]
                    slot_idx += 1
                else:
                    chosen = 1
                assigned_map[cmt] = chosen
                used_slots.add(chosen)
                c["feeder_no"] = chosen
                
    def get_configured_comments(self):
        return set(v["comment"] for v in self.feeder_matrix.values() if v.get("comment", "").strip())
        
    def apply_feeder_assignments_to_components(self):
        for c in self.top_components:
            f_no, head, spd = self.find_feeder_no(c["comment"], c["footprint"])
            c["feeder_no"] = f_no
            c["head"] = head
            c["mount_speed"] = spd
        for c in self.bot_components:
            f_no, head, spd = self.find_feeder_no(c["comment"], c["footprint"])
            c["feeder_no"] = f_no
            c["head"] = head
            c["mount_speed"] = spd
        self.refresh_tables()
        
    def refresh_tables(self):
        # Refresh TOP
        self.tree_top.delete(*self.tree_top.get_children())
        for idx, c in enumerate(self.top_components):
            is_skip = (c.get("skip", 0) == 1)
            skip_text = str(c.get("skip", 0))
            tag = "skip_on" if is_skip else "skip_off"
            self.tree_top.insert("", tk.END, iid=f"top_{idx}", values=(
                idx + 1, c["designator"], c["comment"], c["footprint"],
                f"{c['mid_x']:.2f}", f"{c['mid_y']:.2f}", f"{c['rotation']:.2f}",
                c["head"], c["feeder_no"], c["mount_speed"],
                f"{c['pick_height']:.2f}", f"{c['place_height']:.2f}",
                c["mode"], skip_text
            ), tags=(tag,))
            
        # Refresh BOT
        self.tree_bot.delete(*self.tree_bot.get_children())
        for idx, c in enumerate(self.bot_components):
            is_skip = (c.get("skip", 0) == 1)
            skip_text = str(c.get("skip", 0))
            tag = "skip_on" if is_skip else "skip_off"
            self.tree_bot.insert("", tk.END, iid=f"bot_{idx}", values=(
                idx + 1, c["designator"], c["comment"], c["footprint"],
                f"{c['mid_x']:.2f}", f"{c['mid_y']:.2f}", f"{c['rotation']:.2f}",
                c["head"], c["feeder_no"], c["mount_speed"],
                f"{c['pick_height']:.2f}", f"{c['place_height']:.2f}",
                c["mode"], skip_text
            ), tags=(tag,))
            
        self.tree_top.tag_configure("skip_on", foreground="#DC2626", font=("Segoe UI", 9, "bold"))
        self.tree_top.tag_configure("skip_off", foreground="#94A3B8", font=("Segoe UI", 9, "bold"))
        self.tree_bot.tag_configure("skip_on", foreground="#DC2626", font=("Segoe UI", 9, "bold"))
        self.tree_bot.tag_configure("skip_off", foreground="#94A3B8", font=("Segoe UI", 9, "bold"))
        
        self.notebook.tab(0, text=f"  Mặt TOP ({len(self.top_components)} linh kiện)  ")
        self.notebook.tab(1, text=f"  Mặt BOTTOM ({len(self.bot_components)} linh kiện)  ")
        
        top_u = len(set(c["comment"] for c in self.top_components))
        bot_u = len(set(c["comment"] for c in self.bot_components))
        self.stats_label.config(text=f"✔ Đã nạp: TOP {len(self.top_components)} pcs ({top_u} loại)  |  BOTTOM {len(self.bot_components)} pcs ({bot_u} loại)  |  Nhấp đúp chuột để sửa 13 thông số")

    def get_active_tree_and_list(self):
        if self.notebook.index(self.notebook.select()) == 0:
            return self.tree_top, self.top_components, "top"
        else:
            return self.tree_bot, self.bot_components, "bot"

    def edit_selected_row(self):
        tree, comp_list, prefix = self.get_active_tree_and_list()
        selected = tree.selection()
        if not selected:
            messagebox.showwarning("Thông Báo", "Vui lòng nhấp chọn một dòng linh kiện để chỉnh sửa!")
            return
            
        item_id = selected[0]
        idx = int(item_id.split("_")[1])
        comp = comp_list[idx]
        
        def on_save(updated_data):
            comp_list[idx] = updated_data
            self.refresh_tables()
            
        RowEditDialog(self.root, comp, on_save)
        
    def add_component(self):
        tree, comp_list, prefix = self.get_active_tree_and_list()
        new_comp = {
            "designator": f"R{len(comp_list)+1}",
            "comment": "10k",
            "footprint": "0603D",
            "mid_x": 0.0,
            "mid_y": 0.0,
            "rotation": 0.0,
            "head": 0,
            "feeder_no": 1,
            "mount_speed": 100,
            "pick_height": 0.0,
            "place_height": 0.0,
            "mode": 1,
            "skip": 0,
            "layer": "TopLayer" if prefix == "top" else "BottomLayer"
        }
        
        def on_save(data):
            comp_list.append(data)
            comp_list.sort(key=self.natural_sort_key)
            self.refresh_tables()
            
        RowEditDialog(self.root, new_comp, on_save)
        
    def delete_selected_row(self):
        tree, comp_list, prefix = self.get_active_tree_and_list()
        selected = tree.selection()
        if not selected: return
        
        if messagebox.askyesno("Xác nhận", "Bạn có chắc chắn muốn xóa dòng linh kiện này?"):
            idx = int(selected[0].split("_")[1])
            comp_list.pop(idx)
            self.refresh_tables()
            
    def toggle_skip(self):
        tree, comp_list, prefix = self.get_active_tree_and_list()
        selected = tree.selection()
        if not selected: return
        
        for s in selected:
            idx = int(s.split("_")[1])
            comp_list[idx]["skip"] = 0 if comp_list[idx].get("skip", 0) == 1 else 1
        self.refresh_tables()

    def parse_csv_line(self, line):
        fields = []
        current = []
        in_quotes = False
        for ch in line:
            if ch == '"':
                in_quotes = not in_quotes
            elif ch == ',' and not in_quotes:
                fields.append("".join(current).strip().strip('"'))
                current = []
                continue
            current.append(ch)
        fields.append("".join(current).strip().strip('"'))
        return fields

    def save_and_export(self):
        if not self.top_components and not self.bot_components:
            messagebox.showwarning("Cảnh Báo", "Chưa có dữ liệu để lưu!")
            return
            
        try:
            header_str = (
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
                "Designator,Comment,Footprint,Mid X(mm),Mid Y(mm) ,Rotation,Head ,FeederNo,Mount Speed(%),Pick Height(mm),Place Height(mm),Mode,Skip\r\n"
            )
            
            if os.path.exists(self.template_path):
                with open(self.template_path, "r", encoding="utf-8-sig", errors="ignore") as tf:
                    t_lines = []
                    for line in tf:
                        t_lines.append(line.rstrip("\r\n") + "\r\n")
                        if "Designator" in line and "Comment" in line:
                            header_str = "".join(t_lines)
                            break
                            
            # Lưu TOP
            top_out_path = os.path.join(self.base_dir, self.top_output_name.get().strip())
            with open(top_out_path, "w", encoding="utf-8", newline="") as f:
                f.write(header_str)
                for c in self.top_components:
                    line = f"{c['designator']},{c['comment']},{c['footprint']},{c['mid_x']:.2f},{c['mid_y']:.2f},{c['rotation']:.2f},{c['head']},{c['feeder_no']},{c['mount_speed']},{c['pick_height']:.2f},{c['place_height']:.2f},{c['mode']},{c['skip']}\r\n"
                    f.write(line)
                    
            # Lưu BOT
            bot_out_path = os.path.join(self.base_dir, self.bot_output_name.get().strip())
            with open(bot_out_path, "w", encoding="utf-8", newline="") as f:
                f.write(header_str)
                for c in self.bot_components:
                    line = f"{c['designator']},{c['comment']},{c['footprint']},{c['mid_x']:.2f},{c['mid_y']:.2f},{c['rotation']:.2f},{c['head']},{c['feeder_no']},{c['mount_speed']},{c['pick_height']:.2f},{c['place_height']:.2f},{c['mode']},{c['skip']}\r\n"
                    f.write(line)
                    
            msg = (
                f"🎉 ĐÃ LƯU BẢN ĐÃ CHỈNH SỬA CHO MÁY NEODEN YY1!\n\n"
                f"• Mặt TOP: {len(self.top_components)} linh kiện\n"
                f"  File: {top_out_path}\n\n"
                f"• Mặt BOTTOM: {len(self.bot_components)} linh kiện\n"
                f"  File: {bot_out_path}\n\n"
                f"Tất cả các thông số chỉnh sửa (Tọa độ, Feeder, Head, Góc, Skip, Tốc độ) đã được lưu chính xác 100%!"
            )
            messagebox.showinfo("Lưu Thành Công", msg)
            
        except Exception as e:
            messagebox.showerror("Lỗi Xuất File", f"Chi tiết: {str(e)}")

if __name__ == "__main__":
    root = tk.Tk()
    app = NeoDenYY1App(root)
    root.mainloop()
