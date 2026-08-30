import tkinter as tk
from tkinter import ttk, filedialog, messagebox, simpledialog
import os
import sys
import re
import json
from PIL import Image, ImageTk

# Quản lý cấu hình 4 góc khay Feeder NeoDen YY1
DEFAULT_FEEDER_DATA = {
    "1": {"comment": "1K-0603", "footprint": "0603", "head": 0, "speed": 100},
    "2": {"comment": "100uF-0603", "footprint": "0603", "head": 0, "speed": 100},
    "3": {"comment": "10K-0805", "footprint": "0805", "head": 0, "speed": 100},
    "4": {"comment": "100uF-0805", "footprint": "0805", "head": 0, "speed": 100},
    "5": {"comment": "1K-0805", "footprint": "0805", "head": 0, "speed": 100},
    "6": {"comment": "10K-0603", "footprint": "0603", "head": 0, "speed": 100},
    "7": {"comment": "10uF-0805", "footprint": "0805", "head": 0, "speed": 100},
    "8": {"comment": "2SC1805", "footprint": "SOT-23", "head": 0, "speed": 100},
    "9": {"comment": "4.7K-0805", "footprint": "0805", "head": 0, "speed": 100},
    "10": {"comment": "", "footprint": "0603", "head": 0, "speed": 100},
    "11": {"comment": "", "footprint": "0603", "head": 0, "speed": 100},
    "12": {"comment": "", "footprint": "0603", "head": 0, "speed": 100},
    "13": {"comment": "", "footprint": "0603", "head": 0, "speed": 100},
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
    "30": {"comment": "Red-0805", "footprint": "0805", "head": 0, "speed": 100},
    "31": {"comment": "Green-0805", "footprint": "0805", "head": 0, "speed": 100},
    "32": {"comment": "Yellow-0805", "footprint": "0805", "head": 0, "speed": 100},
    "33": {"comment": "Blue-0805", "footprint": "0805", "head": 0, "speed": 100},
    "34": {"comment": "Red-0603", "footprint": "0603", "head": 0, "speed": 100},
    "35": {"comment": "Blue-0603", "footprint": "0603", "head": 0, "speed": 100},
    "36": {"comment": "", "footprint": "SOT-23", "head": 0, "speed": 100},
    "37": {"comment": "", "footprint": "SOT-23", "head": 0, "speed": 100},
    "38": {"comment": "", "footprint": "SOT-23", "head": 0, "speed": 100},
    "39": {"comment": "", "footprint": "SOP-16", "head": 0, "speed": 90},
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
    """Hộp thoại cấu hình 50 Khay Feeder 4 Góc NeoDen YY1 với Quản Lý Đa Cấu Hình Hiện Đại"""
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app
        self.title("⚙️ Quản Lý Cấu Hình 50 Khay Feeder 4 Góc - Máy NeoDen YY1")
        self.geometry("700x780")
        self.resizable(False, False)
        self.configure(bg="#F1F5F9")
        self.transient(parent)
        self.grab_set()
        
        self.profiles_dir = os.path.join(self.app.base_dir, "feeder_profiles")
        os.makedirs(self.profiles_dir, exist_ok=True)
        
        self.profile_var = tk.StringVar(value=self.app.active_profile if self.app.active_profile else "Mac_Dinh")
        self.entries = {}
        self.setup_ui()
        self.load_profile_list()
        
    def setup_ui(self):
        # Header banner
        hdr = tk.Frame(self, bg="#F1F5F9", padx=15, pady=8)
        hdr.pack(fill=tk.X)
        tk.Label(hdr, text="⚙️ CẤU HÌNH 50 KHAY FEEDER 4 GÓC (NEODEN YY1)", font=("Segoe UI", 12, "bold"), fg="#0284C7", bg="#F1F5F9").pack(anchor=tk.W)
        tk.Label(hdr, text="Quản lý nhiều cấu hình linh kiện mặc định cho từng dự án mạch. Số 1, 14, 30, 40 nằm ở phía dưới.", font=("Segoe UI", 8), fg="#475569", bg="#F1F5F9").pack(anchor=tk.W)
        
        # Profile Management Bar
        prof_bar = tk.Frame(self, bg="#FFFFFF", padx=10, pady=6, bd=1, relief="solid")
        prof_bar.pack(fill=tk.X, padx=10, pady=(2, 6))
        
        tk.Label(prof_bar, text="📂 Cấu Hình (Profile):", font=("Segoe UI", 9, "bold"), fg="#0284C7", bg="#FFFFFF").pack(side=tk.LEFT, padx=(0, 6))
        
        self.cb_profile = ttk.Combobox(prof_bar, textvariable=self.profile_var, state="readonly", width=22, font=("Segoe UI", 9, "bold"))
        self.cb_profile.pack(side=tk.LEFT, padx=(0, 8))
        self.cb_profile.bind("<<ComboboxSelected>>", self.on_profile_selected)
        
        tk.Button(prof_bar, text="➕ Tạo Mới", font=("Segoe UI", 8, "bold"), bg="#0284C7", fg="white", padx=8, pady=3, bd=0, command=self.create_new_profile).pack(side=tk.LEFT, padx=3)
        tk.Button(prof_bar, text="💾 Lưu", font=("Segoe UI", 8, "bold"), bg="#10B981", fg="white", padx=8, pady=3, bd=0, command=self.save_current_profile).pack(side=tk.LEFT, padx=3)
        tk.Button(prof_bar, text="📁 Lưu Thành...", font=("Segoe UI", 8), bg="#475569", fg="white", padx=8, pady=3, bd=0, command=self.save_as_profile).pack(side=tk.LEFT, padx=3)
        tk.Button(prof_bar, text="🗑️ Xóa", font=("Segoe UI", 8), bg="#EF4444", fg="white", padx=8, pady=3, bd=0, command=self.delete_profile).pack(side=tk.LEFT, padx=3)
        
        # Container 4 góc
        grid_frame = tk.Frame(self, bg="#F1F5F9", padx=8, pady=0)
        grid_frame.pack(fill=tk.BOTH, expand=True)
        grid_frame.columnconfigure(0, weight=1)
        grid_frame.columnconfigure(1, weight=1)
        
        # 4 Quadrants (Mỗi góc 1 cột duy nhất, số 1, 14, 30, 40 ở phía dưới)
        self.build_quadrant(grid_frame, 0, 0, "📌 Góc Trên Trái (14 → 24)", range(24, 13, -1), "#0284C7")
        self.build_quadrant(grid_frame, 0, 1, "📌 Góc Trên Phải (40 → 50)", range(50, 39, -1), "#059669")
        self.build_quadrant(grid_frame, 1, 0, "📌 Góc Dưới Trái (1 → 13)", range(13, 0, -1), "#0284C7")
        self.build_quadrant(grid_frame, 1, 1, "📌 Góc Dưới Phải (30 → 39)", range(39, 29, -1), "#D97706")
        
        # Footer buttons
        btn_bar = tk.Frame(self, bg="#FFFFFF", padx=15, pady=8, bd=1, relief="solid")
        btn_bar.pack(fill=tk.X)
        
        tk.Button(btn_bar, text="💾 LƯU VÀ ÁP DỤNG NGAY", font=("Segoe UI", 9, "bold"), bg="#10B981", fg="white", padx=18, pady=6, bd=0, command=self.save_and_apply).pack(side=tk.RIGHT, padx=5)
        tk.Button(btn_bar, text="✖ Đóng", font=("Segoe UI", 9), bg="#475569", fg="white", padx=14, pady=6, bd=0, command=self.destroy).pack(side=tk.LEFT)
        
    def build_quadrant(self, parent, row, col, title, feeder_range, border_color):
        frame = tk.LabelFrame(parent, text=f"  {title}  ", font=("Segoe UI", 9, "bold"), fg=border_color, bg="#FFFFFF", bd=1, relief="solid", padx=6, pady=4)
        frame.grid(row=row, column=col, sticky="nsew", padx=4, pady=2)
        
        for f_no in feeder_range:
            f_str = str(f_no)
            cur = self.app.feeder_matrix.get(f_str, {"comment": "", "footprint": "0603", "head": 0, "speed": 100})
            
            r = tk.Frame(frame, bg="#FFFFFF", pady=1)
            r.pack(fill=tk.X)
            
            lbl_no = tk.Label(r, text=f"#{f_no:02d}:", width=5, font=("Segoe UI", 8, "bold"), fg="#475569", bg="#FFFFFF", anchor=tk.E)
            lbl_no.pack(side=tk.LEFT, padx=(2, 4))
            
            e_cmt = tk.Entry(r, font=("Segoe UI", 8), bg="#FFFFFF", fg="#0F172A", insertbackground="black", bd=1, relief="solid")
            e_cmt.insert(0, cur.get("comment", "") if isinstance(cur, dict) else str(cur))
            e_cmt.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 4))
            
            self.entries[f_str] = e_cmt
            
    def load_profile_list(self):
        mac_dinh_file = os.path.join(self.profiles_dir, "Mac_Dinh.json")
        if not os.path.exists(mac_dinh_file):
            self.save_profile_to_disk("Mac_Dinh", {str(k): v["comment"] for k, v in DEFAULT_FEEDER_DATA.items()})
            
        profiles = []
        if os.path.exists(self.profiles_dir):
            for fname in os.listdir(self.profiles_dir):
                if fname.endswith(".json"):
                    stem = os.path.splitext(fname)[0]
                    if stem != "Mac_Dinh":
                        profiles.append(stem)
        profiles.sort()
        profiles.insert(0, "Mac_Dinh")
        
        self.cb_profile["values"] = profiles
        cur_prof = self.app.active_profile if getattr(self.app, "active_profile", None) else "Mac_Dinh"
        if self.profile_var.get() not in profiles:
            self.profile_var.set(cur_prof if cur_prof in profiles else "Mac_Dinh")
        self.on_profile_selected()
            
    def on_profile_selected(self, event=None):
        prof_name = self.profile_var.get().strip()
        if not prof_name: prof_name = "Mac_Dinh"
        filepath = os.path.join(self.profiles_dir, f"{prof_name}.json")
        if os.path.exists(filepath):
            try:
                with open(filepath, "r", encoding="utf-8") as f:
                    data = json.load(f)
                    feeders = data.get("feeders", data)
                    for slot_str, entry in self.entries.items():
                        val = feeders.get(slot_str, "")
                        if isinstance(val, dict): val = val.get("comment", "")
                        entry.delete(0, tk.END)
                        entry.insert(0, str(val))
            except Exception as e:
                messagebox.showerror("Lỗi Nạp Cấu Hình", f"Không thể đọc file {prof_name}.json: {e}")
                
    def get_current_ui_feeders(self):
        res = {}
        for f_str, entry in self.entries.items():
            res[f_str] = entry.get().strip()
        return res
        
    def save_profile_to_disk(self, prof_name, feeders_dict):
        filepath = os.path.join(self.profiles_dir, f"{prof_name}.json")
        data = {
            "profile_name": prof_name,
            "feeders": feeders_dict
        }
        with open(filepath, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
            
    def save_current_profile(self):
        prof_name = self.profile_var.get().strip()
        if not prof_name: prof_name = "Mac_Dinh"
        feeders = self.get_current_ui_feeders()
        self.save_profile_to_disk(prof_name, feeders)
        messagebox.showinfo("Thành Công", f"Đã lưu cấu hình [{prof_name}] thành công!", parent=self)
        
    def create_new_profile(self):
        new_name = simpledialog.askstring("Tạo Cấu Hình Mới", "Nhập tên cấu hình mới (VD: Bo_Nguon, Du_An_1):", parent=self)
        if new_name:
            new_name = "".join(c for c in new_name.strip() if c.isalnum() or c in ("_", "-"))
            if not new_name:
                messagebox.showwarning("Tên Không Hợp Lệ", "Tên cấu hình không được để trống!", parent=self)
                return
            feeders = self.get_current_ui_feeders()
            self.save_profile_to_disk(new_name, feeders)
            self.profile_var.set(new_name)
            self.load_profile_list()
            messagebox.showinfo("Thành Công", f"Đã tạo mới cấu hình [{new_name}] thành công!", parent=self)
            
    def save_as_profile(self):
        new_name = simpledialog.askstring("Lưu Thành Cấu Hình Khác", "Nhập tên cấu hình mới:", parent=self)
        if new_name:
            new_name = "".join(c for c in new_name.strip() if c.isalnum() or c in ("_", "-"))
            if not new_name: return
            feeders = self.get_current_ui_feeders()
            self.save_profile_to_disk(new_name, feeders)
            self.profile_var.set(new_name)
            self.load_profile_list()
            messagebox.showinfo("Thành Công", f"Đã lưu thành cấu hình [{new_name}] thành công!", parent=self)
            
    def delete_profile(self):
        prof_name = self.profile_var.get().strip()
        if prof_name in ("Mac_Dinh", "Mặc Định", ""):
            messagebox.showwarning("Không Thể Xóa", "Cấu hình mặc định [Mac_Dinh] là cấu hình gốc của máy và không thể xóa!", parent=self)
            return
        if messagebox.askyesno("Xác Nhận Xóa", f"Bạn có chắc muốn xóa vĩnh viễn cấu hình [{prof_name}]?", parent=self):
            filepath = os.path.join(self.profiles_dir, f"{prof_name}.json")
            if os.path.exists(filepath):
                try:
                    os.remove(filepath)
                except Exception as e:
                    messagebox.showerror("Lỗi", f"Không thể xóa file: {e}", parent=self)
                    return
            self.profile_var.set("Mac_Dinh")
            self.load_profile_list()
            self.on_profile_selected()
            messagebox.showinfo("Thành Công", f"Đã xóa cấu hình [{prof_name}]! Đã tự động chuyển về [Mac_Dinh].", parent=self)
            
    def save_and_apply(self):
        prof_name = self.profile_var.get().strip()
        if not prof_name: prof_name = "Mac_Dinh"
        self.save_current_profile()
        for f_str, entry in self.entries.items():
            val = entry.get().strip()
            if f_str not in self.app.feeder_matrix:
                self.app.feeder_matrix[f_str] = {"comment": val, "footprint": "0603", "head": 0, "speed": 100}
            else:
                self.app.feeder_matrix[f_str]["comment"] = val
            
        self.app.active_profile = prof_name
        self.app.save_feeder_matrix_file()
        self.app.update_active_profile_label()
        self.app.apply_feeder_assignments_to_components()
        messagebox.showinfo("Thành Công", f"🎉 Đã lưu cấu hình [{prof_name}] và tự động gán lại số khay Feeder trên toàn bộ bảng linh kiện!", parent=self)
        self.destroy()


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
        self.origin_type = "BOTTOM_LEFT"
        
        # Nạp ma trận Feeder
        self.active_profile = "Mac_Dinh"
        self.feeder_matrix = {}
        self.load_feeder_matrix_file()
        
        self.setup_ui()
        
        SplashScreen(root, self.show_main_window)
        
    def show_main_window(self):
        self.root.deiconify()
        self.root.update()
        
    def update_active_profile_label(self):
        if hasattr(self, "lbl_active_profile"):
            self.lbl_active_profile.config(text=f"⚙️ Quy tắc đang áp dụng: [{self.active_profile}]")
            
    def load_feeder_matrix_file(self):
        if os.path.exists(self.matrix_file):
            try:
                with open(self.matrix_file, "r", encoding="utf-8") as f:
                    data = json.load(f)
                    if "profile_name" in data:
                        self.active_profile = data.get("profile_name", "Mac_Dinh")
                    if "quadrants" in data:
                        # Parse structured JSON
                        self.feeder_matrix = {}
                        for q_key, q_info in data["quadrants"].items():
                            for f_id, f_val in q_info.get("feeders", {}).items():
                                self.feeder_matrix[str(f_id)] = f_val
                    elif "feeders" in data:
                        self.feeder_matrix = {str(k): v for k, v in data["feeders"].items()}
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
        
        # Nút Cấu Hình Feeder 4 Góc & Nhãn Profile Đang Áp Dụng (Nằm Dưới Nút)
        feeder_box = tk.Frame(header_frame, bg=bg_dark)
        feeder_box.pack(side=tk.RIGHT, padx=5, pady=2)
        
        btn_feeder = ttk.Button(feeder_box, text="⚙️ CẤU HÌNH KHAY FEEDER 4 GÓC", style="Feeder.TButton", command=self.open_feeder_matrix_dialog)
        btn_feeder.pack(side=tk.TOP, fill=tk.X, pady=(0, 3))
        
        self.lbl_active_profile = tk.Label(
            feeder_box,
            text=f"⚙️ Quy tắc đang áp dụng: [{self.active_profile}]",
            font=("Segoe UI", 9, "bold"),
            fg="#38BDF8",
            bg="#1E293B",
            padx=8,
            pady=3,
            bd=1,
            relief="ridge"
        )
        self.lbl_active_profile.pack(side=tk.TOP, fill=tk.X)
        
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
        
        self.stats_label = tk.Label(tb_bar, text="📊 Chưa nạp file nào. Vui lòng bấm 'Chọn File...' để nạp dữ liệu.", font=("Segoe UI", 9, "bold"), fg="#38BDF8", bg=card_bg)
        self.stats_label.pack(side=tk.LEFT)
        
        self.auto_match_var = tk.BooleanVar(value=True)
        chk_match = ttk.Checkbutton(
            tb_bar,
            text="Tự động nhận diện Feeder theo Cấu hình",
            variable=self.auto_match_var,
            command=self.on_auto_match_toggled
        )
        chk_match.pack(side=tk.LEFT, padx=15)
        
        self.board_width_var = tk.StringVar(value="")
        self.board_width_var.trace_add("write", lambda *args: self.on_board_width_changed())
        tk.Label(tb_bar, text="Chiều rộng bo X (mm):", font=("Segoe UI", 9, "bold"), fg="#F8FAFC", bg=card_bg).pack(side=tk.LEFT, padx=(10, 2))
        self.entry_bw = tk.Entry(tb_bar, textvariable=self.board_width_var, width=8, font=("Segoe UI", 9, "bold"), justify=tk.CENTER)
        self.entry_bw.pack(side=tk.LEFT, padx=(0, 10))
        
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
        
        self.row_export = ttk.Frame(cfg_frame, style="Card.TFrame")
        self.row_export.pack(fill=tk.X, pady=2)
        
        self.lbl_top = ttk.Label(self.row_export, text="Tên file TOP:")
        self.lbl_top.pack(side=tk.LEFT, padx=(0, 5))
        self.entry_top = ttk.Entry(self.row_export, textvariable=self.top_output_name, width=20, font=("Segoe UI", 9))
        self.entry_top.pack(side=tk.LEFT, padx=(0, 25))
        
        self.lbl_bot = ttk.Label(self.row_export, text="Tên file BOTTOM:")
        self.lbl_bot.pack(side=tk.LEFT, padx=(0, 5))
        self.entry_bot = ttk.Entry(self.row_export, textvariable=self.bot_output_name, width=20, font=("Segoe UI", 9))
        self.entry_bot.pack(side=tk.LEFT)
        
        self.btn_convert = ttk.Button(cfg_frame, text="💾 LƯU FILE ĐÃ CHỈNH SỬA CHO MÁY NEODEN YY1", style="Convert.TButton", command=self.save_and_export)
        self.btn_convert.pack(fill=tk.X, pady=(8, 2))
        
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
            "STT": 42, "Designator": 96, "Comment": 208, "Footprint": 162,
            "Mid X(mm)": 90, "Mid Y(mm)": 90, "Rotation": 80, "Head": 55,
            "FeederNo": 78, "Mount Speed(%)": 78, "Pick Height(mm)": 88,
            "Place Height(mm)": 88, "Mode": 56, "Skip": 48
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

        def on_table_double_click(event):
            region = tree.identify_region(event.x, event.y)
            if region in ("cell", "tree"):
                col = tree.identify_column(event.x)
                item_id = tree.identify_row(event.y)
                if not item_id:
                    return
                try:
                    col_idx = int(col.replace("#", "")) - 1
                except Exception:
                    return
                if col_idx == 0:
                    return
                try:
                    idx = int(item_id.split("_")[1])
                except Exception:
                    return
                _, comp_list, _ = self.get_active_tree_and_list()
                if idx >= len(comp_list):
                    return
                if col_idx == 13:
                    comp_list[idx]["skip"] = 0 if comp_list[idx].get("skip", 0) == 1 else 1
                    self.refresh_tables()
                    return

                bbox = tree.bbox(item_id, col)
                if not bbox:
                    return
                x, y, w, h = bbox
                key_map = {
                    1: "designator", 2: "comment", 3: "footprint",
                    4: "mid_x", 5: "mid_y", 6: "rotation",
                    7: "head", 8: "feeder_no", 9: "mount_speed",
                    10: "pick_height", 11: "place_height", 12: "mode"
                }
                key = key_map.get(col_idx)
                if not key:
                    return

                cur_val = str(comp_list[idx].get(key, ""))
                entry = tk.Entry(tree, font=("Segoe UI", 9), bg="#1E293B", fg="#FFFFFF", insertbackground="white", bd=1, relief="solid")
                entry.insert(0, cur_val)
                entry.select_range(0, tk.END)
                entry.place(x=x, y=y, width=w, height=h)
                entry.focus_set()

                def save_cell(e=None):
                    if not entry.winfo_exists():
                        return
                    new_val = entry.get().strip()
                    try:
                        if key in ("mid_x", "mid_y", "rotation", "pick_height", "place_height"):
                            comp_list[idx][key] = float(new_val)
                        elif key in ("head", "feeder_no", "mount_speed", "mode"):
                            comp_list[idx][key] = int(new_val)
                        else:
                            comp_list[idx][key] = new_val
                    except Exception:
                        comp_list[idx][key] = new_val
                    entry.destroy()
                    self.refresh_tables()

                def cancel_cell(e=None):
                    if entry.winfo_exists():
                        entry.destroy()

                entry.bind("<Return>", save_cell)
                entry.bind("<FocusOut>", save_cell)
                entry.bind("<Escape>", cancel_cell)
                            
        tree.bind("<ButtonRelease-1>", on_table_click)
        tree.bind("<Double-1>", on_table_double_click)
        return tree
        
    def on_tab_changed(self, event):
        selected_tab = self.notebook.index(self.notebook.select())
        self.current_layer = "TOP" if selected_tab == 0 else "BOTTOM"
        
    def recalc_bottom_coordinates(self):
        if not hasattr(self, 'board_width_var') or not hasattr(self, 'bot_components'):
            return
        val_str = self.board_width_var.get().strip()
        bw = 0.0
        if val_str:
            try:
                bw = float(val_str)
            except Exception:
                bw = 0.0
            
        for c in self.bot_components:
            raw_x = c.get("raw_mid_x", c.get("mid_x", 0.0))
            if bw > 0.0:
                c["mid_x"] = bw - raw_x
            else:
                c["mid_x"] = raw_x
            c["mid_y"] = c.get("raw_mid_y", c.get("mid_y", 0.0))
            
    def on_board_width_changed(self):
        if not hasattr(self, 'tree_bot') or not hasattr(self, 'bot_components'):
            return
        self.recalc_bottom_coordinates()
        self.refresh_tables()

    def on_auto_match_toggled(self):
        is_auto = self.auto_match_var.get()
        if is_auto:
            for c in self.top_components:
                c["feeder_no"], c["head"], c["mount_speed"] = self.find_feeder_no(c["comment"], c["footprint"])
            for c in self.bot_components:
                c["feeder_no"], c["head"], c["mount_speed"] = self.find_feeder_no(c["comment"], c["footprint"])
        else:
            for c in self.top_components:
                c["feeder_no"] = c.get("raw_feeder_no", 0)
            for c in self.bot_components:
                c["feeder_no"] = c.get("raw_feeder_no", 0)
        self.refresh_tables()

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
        if "sdcard" in cl or "tf3" in cl: return "MICRO_SD_TF3"
        if "pressure sensor" in cl: return "PRESSURE_SENSOR"
        return c

    def remove_diacritics(self, text):
        s = text.lower()
        replacements = {
            'á': 'a', 'à': 'a', 'ả': 'a', 'ã': 'a', 'ạ': 'a', 'ă': 'a', 'ắ': 'a', 'ằ': 'a', 'ẳ': 'a', 'ẵ': 'a', 'ặ': 'a', 'â': 'a', 'ấ': 'a', 'ầ': 'a', 'ẩ': 'a', 'ẫ': 'a', 'ậ': 'a',
            'é': 'e', 'è': 'e', 'ẻ': 'e', 'ẽ': 'e', 'ẹ': 'e', 'ê': 'e', 'ế': 'e', 'ề': 'e', 'ể': 'e', 'ễ': 'e', 'ệ': 'e',
            'í': 'i', 'ì': 'i', 'ỉ': 'i', 'ĩ': 'i', 'ị': 'i',
            'ó': 'o', 'ò': 'o', 'ỏ': 'o', 'õ': 'o', 'ọ': 'o', 'ô': 'o', 'ố': 'o', 'ồ': 'o', 'ổ': 'o', 'ỗ': 'o', 'ộ': 'o', 'ơ': 'o', 'ớ': 'o', 'ờ': 'o', 'ở': 'o', 'ỡ': 'o', 'ợ': 'o',
            'ú': 'u', 'ù': 'u', 'ủ': 'u', 'ũ': 'u', 'ụ': 'u', 'ư': 'u', 'ứ': 'u', 'ừ': 'u', 'ử': 'u', 'ữ': 'u', 'ự': 'u',
            'ý': 'y', 'ỳ': 'y', 'ỷ': 'y', 'ỹ': 'y', 'ỵ': 'y',
            'đ': 'd'
        }
        for k, v in replacements.items():
            s = s.replace(k, v)
        return s

    def canonicalize_feeder_keywords(self, text):
        s = self.remove_diacritics(text)
        s = s.replace("xanh la", "green").replace("xanh cay", "green")
        s = s.replace("xanh duong", "blue").replace("xanh bien", "blue")
        s = s.replace("xanh", "green")
        s = s.replace("vang", "yellow").replace("trang", "white").replace("cam", "orange")
        words = s.split()
        words = ["red" if w == "do" else w for w in words]
        return " ".join(words)

    def natural_sort_key(self, comp):
        cmt = str(comp.get("comment", "")).lower()
        des = str(comp.get("designator", ""))
        numbers = re.findall(r'\d+', des)
        num = int(numbers[-1]) if numbers else 0
        prefix = re.sub(r'\d+', '', des)
        return (cmt, prefix, num, des)

    def is_valid_component(self, des, cmt):
        if not des or des.startswith("*") or des.startswith("#") or des.startswith(";"):
            return False
        d_clean = self.clean_col_name(des)
        if d_clean in ["designator", "refdes", "pattern", "footprint"]:
            return False
            
        d = des.lower()
        if any(bad in d for bad in ["http:", "https:", "www.", "snapeda", "://", ".com", ".org", ".net", "copyright", "all rights", "license"]):
            return False
        if len(des) > 30 or "/" in des or "\\" in des:
            return False
            
        c = str(cmt).lower()
        if any(bad in c for bad in ["snapeda", "view-part", "http://", "https://", "www."]):
            return False
            
        return True

    def extract_tokens(self, text):
        can = self.canonicalize_feeder_keywords(str(text))
        tokens = []
        cur = ""
        for c in can:
            if c.isalnum() or c == '.':
                cur += c
            else:
                if cur:
                    tokens.append(cur)
                    cur = ""
        if cur:
            tokens.append(cur)
            
        extra = []
        for t in tokens:
            if t.startswith("0402") and t != "0402": extra.append("0402")
            elif t.startswith("0603") and t != "0603": extra.append("0603")
            elif t.startswith("0805") and t != "0805": extra.append("0805")
            elif t.startswith("1206") and t != "1206": extra.append("1206")
            elif t.startswith("1210") and t != "1210": extra.append("1210")
            elif "sot23" in t: extra.extend(["sot23", "sot-23"])
            elif "sod323" in t: extra.extend(["sod323", "sod-323"])
            elif "sod123" in t: extra.extend(["sod123", "sod-123"])
            elif "sop16" in t or "soic16" in t: extra.extend(["sop16", "sop-16"])
        tokens.extend(extra)
        return tokens

    def find_feeder_no(self, comment, footprint):
        """Khớp linh kiện thông minh vào khay Feeder đã thiết lập theo 4 góc"""
        comp_tokens = self.extract_tokens(comment)
        fp_tokens = self.extract_tokens(footprint)
        comp_tokens.extend(fp_tokens)
        if not comp_tokens:
            return 0, 0, 100

        best_slot = 0
        best_head = 0
        best_speed = 100
        max_matched = 0

        for f_id, f_cfg in self.feeder_matrix.items():
            cfg_raw = f_cfg.get("comment", "") if isinstance(f_cfg, dict) else str(f_cfg)
            if not str(cfg_raw).strip(): continue
            f_tokens = self.extract_tokens(cfg_raw)
            if not f_tokens: continue

            all_matched = True
            match_count = 0

            for ft in f_tokens:
                if ft in ("smd", "chip"): continue
                if ft in comp_tokens:
                    match_count += 1
                else:
                    all_matched = False
                    break

            if all_matched and match_count > max_matched:
                max_matched = match_count
                best_slot = int(f_id)
                best_head = f_cfg.get("head", 0) if isinstance(f_cfg, dict) else 0
                best_speed = f_cfg.get("speed", 100) if isinstance(f_cfg, dict) else 100

        return best_slot, best_head, best_speed
        
    def clean_col_name(self, s):
        return "".join(c.lower() for c in s if c.isalnum())

    def parse_csv_line(self, line):
        commas = line.count(",")
        semis = line.count(";")
        tabs = line.count("\t")
        delimiter = ","
        if tabs > commas and tabs > semis: delimiter = "\t"
        elif semis > commas and semis > tabs: delimiter = ";"

        try:
            reader = csv.reader([line], delimiter=delimiter)
            for row in reader:
                return [c.strip() for c in row]
        except Exception:
            pass
        return [c.strip().strip('"') for c in line.split(delimiter)]

    def load_altium_data(self):
        filepath = self.input_file.get().strip()
        if not filepath or not os.path.exists(filepath):
            return
            
        try:
            with open(filepath, "rb") as f:
                raw_bytes = f.read()
            
            content = ""
            for enc in ["utf-8-sig", "utf-8", "cp1252", "latin-1"]:
                try:
                    content = raw_bytes.decode(enc)
                    break
                except Exception:
                    continue
            if not content:
                content = raw_bytes.decode("utf-8", errors="ignore")
                
            lines = content.splitlines()
            header_idx = None
            is_mil = False
            col_map = {}
            
            for i, line in enumerate(lines):
                line_l = line.lower()
                if "units used: mil" in line_l or "unit: mil" in line_l or "(mil)" in line_l:
                    is_mil = True
                    
                cols = self.parse_csv_line(line)
                temp_map = {}
                for idx, c in enumerate(cols):
                    k = self.clean_col_name(c)
                    raw_lower = c.lower()
                    if k in ["designator", "refdes", "ref", "reference", "part", "comp", "name", "tag", "item"]:
                        if "designator" not in temp_map: temp_map["designator"] = idx
                    elif k in ["comment", "val", "value", "description", "device", "component"]:
                        if "comment" not in temp_map: temp_map["comment"] = idx
                    elif k in ["footprint", "package", "pattern", "pkg", "fp"]:
                        if "footprint" not in temp_map: temp_map["footprint"] = idx
                    elif k in ["midx", "centerx", "posx", "x", "midxmm", "centerxmm", "posxmm", "padx", "xmid"]:
                        if "x" not in temp_map:
                            temp_map["x"] = idx
                            if "mil" in raw_lower or "inch" in raw_lower: is_mil = True
                    elif k in ["midy", "centery", "posy", "y", "midymm", "centerymm", "posymm", "pady", "ymid"]:
                        if "y" not in temp_map: temp_map["y"] = idx
                    elif k in ["rotation", "rot", "angle", "orientation", "rotate"]:
                        if "rot" not in temp_map: temp_map["rot"] = idx
                    elif k in ["layer", "side", "face", "tb", "topbottom"]:
                        if "layer" not in temp_map: temp_map["layer"] = idx
                    elif k in ["head", "nozzle", "headno"]:
                        if "head" not in temp_map: temp_map["head"] = idx
                    elif k in ["feederno", "feeder", "slot", "stack"]:
                        if "feeder" not in temp_map: temp_map["feeder"] = idx
                    elif k in ["mountspeed", "speed", "velocity"]:
                        if "speed" not in temp_map: temp_map["speed"] = idx
                    elif k in ["pickheight", "pick", "pickheightmm"]:
                        if "pick" not in temp_map: temp_map["pick"] = idx
                    elif k in ["placeheight", "place", "placeheightmm"]:
                        if "place" not in temp_map: temp_map["place"] = idx
                    elif k in ["mode", "visionmode"]:
                        if "mode" not in temp_map: temp_map["mode"] = idx
                    elif k in ["skip", "enable", "active"]:
                        if "skip" not in temp_map: temp_map["skip"] = idx

                if "designator" in temp_map and ("x" in temp_map or "comment" in temp_map or "footprint" in temp_map):
                    header_idx = i
                    col_map = temp_map
                    break
                    
            if header_idx is None:
                messagebox.showerror("Lỗi Định Dạng", "Không tìm thấy dòng Header chứa thông tin linh kiện trong file!")
                return
                
            raw_top = []
            raw_bot = []
            
            for line in lines[header_idx + 1:]:
                line = line.strip()
                if not line: continue
                
                parts = self.parse_csv_line(line)
                if "designator" not in col_map or col_map["designator"] >= len(parts):
                    continue
                    
                des = parts[col_map["designator"]].strip()
                cmt_raw = parts[col_map["comment"]].strip() if "comment" in col_map and col_map["comment"] < len(parts) else ""
                if not self.is_valid_component(des, cmt_raw):
                    continue
                    
                fp_raw = parts[col_map["footprint"]].strip() if "footprint" in col_map and col_map["footprint"] < len(parts) else "0603D"
                layer_raw = parts[col_map["layer"]].strip().lower() if "layer" in col_map and col_map["layer"] < len(parts) else "toplayer"
                
                layer = "BottomLayer" if ("bot" in layer_raw or "back" in layer_raw or layer_raw == "b") else "TopLayer"
                
                cmt = self.normalize_comment(cmt_raw)
                fp = self.normalize_footprint(fp_raw)
                
                try:
                    raw_x = float(parts[col_map["x"]]) if "x" in col_map and col_map["x"] < len(parts) and parts[col_map["x"]] != "" else 0.0
                    raw_y = float(parts[col_map["y"]]) if "y" in col_map and col_map["y"] < len(parts) and parts[col_map["y"]] != "" else 0.0
                    rot = float(parts[col_map["rot"]]) if "rot" in col_map and col_map["rot"] < len(parts) and parts[col_map["rot"]] != "" else 0.0
                except:
                    raw_x, raw_y, rot = 0.0, 0.0, 0.0
                    
                raw_mid_x = raw_x * 0.0254 if is_mil else raw_x
                raw_mid_y = raw_y * 0.0254 if is_mil else raw_y
                mid_x = raw_mid_x
                mid_y = raw_mid_y
                
                def_fno, def_head, def_spd = self.find_feeder_no(cmt, fp)
                
                try:
                    head = int(parts[col_map["head"]]) if "head" in col_map and col_map["head"] < len(parts) and parts[col_map["head"]] != "" else def_head
                except: head = def_head
                
                try:
                    raw_feeder_no = int(parts[col_map["feeder"]]) if "feeder" in col_map and col_map["feeder"] < len(parts) and parts[col_map["feeder"]] != "" else 0
                except: raw_feeder_no = 0
                
                feeder_no = def_fno if self.auto_match_var.get() else raw_feeder_no
                
                try:
                    mount_speed = int(parts[col_map["speed"]]) if "speed" in col_map and col_map["speed"] < len(parts) and parts[col_map["speed"]] != "" else def_spd
                except: mount_speed = def_spd
                
                try:
                    pick_height = float(parts[col_map["pick"]]) if "pick" in col_map and col_map["pick"] < len(parts) and parts[col_map["pick"]] != "" else 0.0
                except: pick_height = 0.0
                
                try:
                    place_height = float(parts[col_map["place"]]) if "place" in col_map and col_map["place"] < len(parts) and parts[col_map["place"]] != "" else 0.0
                except: place_height = 0.0
                
                try:
                    mode = int(parts[col_map["mode"]]) if "mode" in col_map and col_map["mode"] < len(parts) and parts[col_map["mode"]] != "" else 1
                except: mode = 1
                
                try:
                    skip = int(parts[col_map["skip"]]) if "skip" in col_map and col_map["skip"] < len(parts) and parts[col_map["skip"]] != "" else 0
                except: skip = 0
                
                comp = {
                    "designator": des,
                    "comment": cmt,
                    "footprint": fp,
                    "mid_x": mid_x,
                    "mid_y": mid_y,
                    "raw_mid_x": raw_mid_x,
                    "raw_mid_y": raw_mid_y,
                    "rotation": rot,
                    "head": head,
                    "feeder_no": feeder_no,
                    "raw_feeder_no": raw_feeder_no,
                    "mount_speed": mount_speed,
                    "pick_height": pick_height,
                    "place_height": place_height,
                    "mode": mode,
                    "skip": skip,
                    "layer": layer
                }
                
                if "bot" in layer.lower():
                    raw_bot.append(comp)
                else:
                    raw_top.append(comp)
                    
            if not raw_top and not raw_bot:
                messagebox.showerror("Lỗi Đọc File", "Không tìm thấy linh kiện hợp lệ trong file CSV/TXT!")
                return

            raw_top.sort(key=self.natural_sort_key)
            raw_bot.sort(key=self.natural_sort_key)
            
            self.top_components = raw_top
            self.bot_components = raw_bot
            
            if "feeder" not in col_map:
                self.assign_dynamic_feeders(self.top_components)
                self.assign_dynamic_feeders(self.bot_components)
            
            self.origin_type = self.detect_origin_type()
            self.recalc_coordinates()
            self.update_layer_and_origin_ui()
            self.refresh_tables()
            
        except Exception as e:
            messagebox.showerror("Lỗi Đọc File", f"Chi tiết: {str(e)}")

    def detect_origin_type(self):
        all_comps = self.top_components + self.bot_components
        if not all_comps:
            return "UNKNOWN"
        min_x = min(c["raw_mid_x"] for c in all_comps)
        max_x = max(c["raw_mid_x"] for c in all_comps)
        min_y = min(c["raw_mid_y"] for c in all_comps)
        
        if min_y < -0.1:
            return "INVALID"
            
        if min_x >= -0.1:
            return "BOTTOM_LEFT"
        elif max_x <= 0.1:
            return "BOTTOM_RIGHT"
        else:
            return "INVALID"

    def recalc_coordinates(self):
        bw_str = self.board_width_var.get().strip()
        try:
            bw = float(bw_str) if bw_str else 0.0
        except ValueError:
            bw = 0.0

        if self.origin_type == "BOTTOM_LEFT":
            for c in self.top_components:
                c["mid_x"] = c["raw_mid_x"]
                c["mid_y"] = c["raw_mid_y"]
            for c in self.bot_components:
                c["mid_x"] = (bw - c["raw_mid_x"]) if bw > 0.0 else c["raw_mid_x"]
                c["mid_y"] = c["raw_mid_y"]
        elif self.origin_type == "BOTTOM_RIGHT":
            for c in self.bot_components:
                c["mid_x"] = abs(c["raw_mid_x"])
                c["mid_y"] = c["raw_mid_y"]
            for c in self.top_components:
                c["mid_x"] = (bw + c["raw_mid_x"]) if bw > 0.0 else abs(c["raw_mid_x"])
                c["mid_y"] = c["raw_mid_y"]
        else:
            for c in self.top_components:
                c["mid_x"] = c["raw_mid_x"]
                c["mid_y"] = c["raw_mid_y"]
            for c in self.bot_components:
                c["mid_x"] = c["raw_mid_x"]
                c["mid_y"] = c["raw_mid_y"]

    def update_layer_and_origin_ui(self):
        has_top = bool(self.top_components)
        has_bot = bool(self.bot_components)
        
        if hasattr(self, "notebook"):
            if has_top and not has_bot:
                self.notebook.tab(0, state="normal")
                self.notebook.tab(1, state="disabled")
                self.notebook.select(0)
                if hasattr(self, "lbl_top"):
                    self.lbl_top.pack(side=tk.LEFT, padx=(0, 5))
                    self.entry_top.pack(side=tk.LEFT, padx=(0, 25))
                    self.lbl_bot.pack_forget()
                    self.entry_bot.pack_forget()
                if hasattr(self, "btn_convert"):
                    self.btn_convert.config(text="💾 LƯU FILE TOP_OUTPUT.CSV CHO MÁY NEODEN YY1")
            elif not has_top and has_bot:
                self.notebook.tab(0, state="disabled")
                self.notebook.tab(1, state="normal")
                self.notebook.select(1)
                if hasattr(self, "lbl_top"):
                    self.lbl_top.pack_forget()
                    self.entry_top.pack_forget()
                    self.lbl_bot.pack(side=tk.LEFT, padx=(0, 5))
                    self.entry_bot.pack(side=tk.LEFT)
                if hasattr(self, "btn_convert"):
                    self.btn_convert.config(text="💾 LƯU FILE BOT_OUTPUT.CSV CHO MÁY NEODEN YY1")
            else:
                self.notebook.tab(0, state="normal")
                self.notebook.tab(1, state="normal")
                if hasattr(self, "lbl_top"):
                    self.lbl_top.pack(side=tk.LEFT, padx=(0, 5))
                    self.entry_top.pack(side=tk.LEFT, padx=(0, 25))
                    self.lbl_bot.pack(side=tk.LEFT, padx=(0, 5))
                    self.entry_bot.pack(side=tk.LEFT)
                if hasattr(self, "btn_convert"):
                    self.btn_convert.config(text="💾 LƯU CẢ 2 FILE (TOP + BOTTOM) CHO MÁY NEODEN YY1")

        if self.origin_type == "BOTTOM_LEFT":
            origin_str = "📍 Gốc: GÓC DƯỚI BÊN TRÁI (X>=0, Y>=0)"
        elif self.origin_type == "BOTTOM_RIGHT":
            origin_str = "📍 Gốc: GÓC DƯỚI BÊN PHẢI (X<=0, Y>=0)"
        else:
            origin_str = "⚠️ CẢNH BÁO: GỐC TỌA ĐỘ KHÔNG HỢP LỆ (ở giữa/trong/trên mạch)!"

        if has_top and has_bot:
            layer_str = f"📦 2 Mặt (TOP: {len(self.top_components)} LK, BOT: {len(self.bot_components)} LK)"
        elif has_top:
            layer_str = f"📦 Chỉ có Mặt TOP ({len(self.top_components)} LK)"
        elif has_bot:
            layer_str = f"📦 Chỉ có Mặt BOTTOM ({len(self.bot_components)} LK)"
        else:
            layer_str = "📦 Chưa có linh kiện nào"

        if hasattr(self, "stats_label"):
            self.stats_label.config(text=f"{origin_str}  |  {layer_str}")

        if self.origin_type == "INVALID":
            messagebox.showwarning(
                "Cảnh Báo Gốc Tọa Độ Không Hợp Lệ",
                "⚠️ CẢNH BÁO FILE KHÔNG HỢP LỆ:\n\n"
                "Gốc tọa độ của file hiện tại đang được đặt ở GIỮA MẠCH, TRÊN MẠCH hoặc TRONG MẠCH!\n\n"
                "📌 Quy chuẩn máy NeoDen YY1:\n"
                "- Gốc hợp lệ 1: Góc Dưới Bên Trái (toàn bộ X >= 0, Y >= 0)\n"
                "- Gốc hợp lệ 2: Góc Dưới Bên Phải (toàn bộ X <= 0, Y >= 0)\n\n"
                "Vui lòng kiểm tra và đặt lại gốc tọa độ chuẩn trong Altium Designer trước khi xuất Pick & Place!"
            )
            
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
        
    def on_auto_match_toggled(self):
        if self.auto_match_var.get():
            self.apply_feeder_assignments_to_components()
        else:
            self.refresh_tables()

    def on_board_width_changed(self):
        self.recalc_coordinates()
        self.refresh_tables()

    def apply_feeder_assignments_to_components(self):
        if self.auto_match_var.get():
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
            is_missing = (c.get("feeder_no", 0) == 0)
            
            tags = []
            if is_missing:
                tags.append("feeder_missing")
            elif is_skip:
                tags.append("skip_on")
            else:
                tags.append("skip_off")

            self.tree_top.insert("", tk.END, iid=f"top_{idx}", values=(
                idx + 1, c["designator"], c["comment"], c["footprint"],
                f"{c['mid_x']:.2f}", f"{c['mid_y']:.2f}", f"{c['rotation']:.2f}",
                c["head"], c["feeder_no"], c["mount_speed"],
                f"{c['pick_height']:.2f}", f"{c['place_height']:.2f}",
                c["mode"], skip_text
            ), tags=tuple(tags))
            
        # Refresh BOT
        self.tree_bot.delete(*self.tree_bot.get_children())
        for idx, c in enumerate(self.bot_components):
            is_skip = (c.get("skip", 0) == 1)
            skip_text = str(c.get("skip", 0))
            is_missing = (c.get("feeder_no", 0) == 0)
            
            tags = []
            if is_missing:
                tags.append("feeder_missing")
            elif is_skip:
                tags.append("skip_on")
            else:
                tags.append("skip_off")

            self.tree_bot.insert("", tk.END, iid=f"bot_{idx}", values=(
                idx + 1, c["designator"], c["comment"], c["footprint"],
                f"{c['mid_x']:.2f}", f"{c['mid_y']:.2f}", f"{c['rotation']:.2f}",
                c["head"], c["feeder_no"], c["mount_speed"],
                f"{c['pick_height']:.2f}", f"{c['place_height']:.2f}",
                c["mode"], skip_text
            ), tags=tuple(tags))
            
        for tree in (self.tree_top, self.tree_bot):
            tree.tag_configure("feeder_missing", background="#7F1D1D", foreground="#FCA5A5", font=("Segoe UI", 9, "bold"))
            tree.tag_configure("skip_on", foreground="#DC2626", font=("Segoe UI", 9, "bold"))
            tree.tag_configure("skip_off", foreground="#16A34A", font=("Segoe UI", 9, "bold"))
        
        self.notebook.tab(0, text=f"  Mặt TOP ({len(self.top_components)} linh kiện)  ")
        self.notebook.tab(1, text=f"  Mặt BOTTOM ({len(self.bot_components)} linh kiện)  ")

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
        has_top = bool(self.top_components)
        has_bot = bool(self.bot_components)
        if not has_top and not has_bot:
            messagebox.showwarning("Cảnh Báo", "Chưa có dữ liệu để lưu!")
            return
            
        self.recalc_coordinates()
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
                            
            report_items = []
            # Lưu TOP nếu có
            if has_top:
                top_out_path = os.path.join(self.base_dir, self.top_output_name.get().strip())
                with open(top_out_path, "w", encoding="utf-8", newline="") as f:
                    f.write(header_str)
                    for c in self.top_components:
                        line = f"{c['designator']},{c['comment']},{c['footprint']},{c['mid_x']:.2f},{c['mid_y']:.2f},{c['rotation']:.2f},{c['head']},{c['feeder_no']},{c['mount_speed']},{c['pick_height']:.2f},{c['place_height']:.2f},{c['mode']},{c['skip']}\r\n"
                        f.write(line)
                report_items.append(f"⭐ Mặt TOP:\n   • Số linh kiện: {len(self.top_components)}\n   • File: {top_out_path}")
                    
            # Lưu BOT nếu có
            if has_bot:
                bot_out_path = os.path.join(self.base_dir, self.bot_output_name.get().strip())
                with open(bot_out_path, "w", encoding="utf-8", newline="") as f:
                    f.write(header_str)
                    for c in self.bot_components:
                        line = f"{c['designator']},{c['comment']},{c['footprint']},{c['mid_x']:.2f},{c['mid_y']:.2f},{c['rotation']:.2f},{c['head']},{c['feeder_no']},{c['mount_speed']},{c['pick_height']:.2f},{c['place_height']:.2f},{c['mode']},{c['skip']}\r\n"
                        f.write(line)
                report_items.append(f"⭐ Mặt BOTTOM:\n   • Số linh kiện: {len(self.bot_components)}\n   • File: {bot_out_path}")
                    
            msg = (
                f"🎉 ĐÃ LƯU BẢN ĐÃ CHỈNH SỬA CHO MÁY NEODEN YY1!\n\n"
                + "\n\n".join(report_items)
                + "\n\nToàn bộ 13 thông số đã chỉnh sửa được lưu chính xác 100%!"
            )
            messagebox.showinfo("Lưu Thành Công", msg)
            
        except Exception as e:
            messagebox.showerror("Lỗi Xuất File", f"Chi tiết: {str(e)}")

if __name__ == "__main__":
    root = tk.Tk()
    app = NeoDenYY1App(root)
    root.mainloop()
