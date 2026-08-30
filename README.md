# NeoDen YY1 SMT - Altium Pick & Place File Converter (GUI Pro Suite)

**Đơn vị phát triển & Bản quyền**: **CÔNG TY TNHH CÔNG NGHỆ CHIPXA**

Phần mềm giao diện đồ họa (**Desktop GUI**) chuyên nghiệp dùng để chuyển đổi và chuẩn hóa dữ liệu **Pick & Place** xuất từ **Altium Designer** sang định dạng file **CSV** tương thích chuẩn cho máy gắp linh kiện tự động **NeoDen YY1** (SMT Pick and Place Machine).

Dự án được xây dựng 100% dạng ứng dụng cửa sổ đồ họa Desktop (**Native Win32 GUI & Python GUI**) với Logo thương hiệu, bảng xem trước danh sách linh kiện và tính năng tự động gán khay Feeder.

---

## 🖥️ CÁC BẢN GIAO DIỆN ĐỒ HỌA DESKTOP (DOUBLE-CLICK ĐỂ CHẠY NGAY)

Tất cả các bản GUI đều có **Màn hình Intro mở đầu (Splash Screen)** kèm **Logo ảnh đa sắc**, hiệu ứng thanh tiến trình (Loading Bar) trước khi mở giao diện làm việc chính:

| Ứng dụng GUI | Ngôn ngữ | File chạy trực tiếp (Double Click) | Đặc điểm giao diện & tính năng |
| :--- | :---: | :--- | :--- |
| ⚡ **C++ GUI Pro** *(Khuyên dùng)* | **C++ Win32** | 👉 **[`neoden_converter_gui.exe`](file:///c:/Users/Admin/Desktop/chipxa/Neoden-YY1/neoden_converter_gui.exe)** | • **Intro Splash Screen mở đầu** với Logo ảnh GDI+ & thanh loading mượt mà<br>• Bảng ListView xem trước danh sách linh kiện chuẩn Windows<br>• Nút duyệt file, thống kê và chuyển đổi 1-click |
| 🎨 **Python GUI** | **Python Tkinter** | 👉 **[`CHAY_GIAO_DIEN_GUI.bat`](file:///c:/Users/Admin/Desktop/chipxa/Neoden-YY1/CHAY_GIAO_DIEN_GUI.bat)** | • **Intro Splash Screen hiện đại** với Logo ảnh lớn<br>• Giao diện Dark Mode Tkinter trực quan |
| 🦀 **Rust Native GUI** | **Rust Win32** | 👉 **[`neoden_converter_rust_gui.exe`](file:///c:/Users/Admin/Desktop/chipxa/Neoden-YY1/neoden_converter_rust_gui.exe)** | • Màn hình Intro mở đầu bằng Rust thuần siêu nhẹ<br>• Không cần cài thêm bất kỳ thư viện ngoài nào |
| 🚀 **C Native GUI** | **Pure C Win32** | 👉 **[`neoden_converter_c_gui.exe`](file:///c:/Users/Admin/Desktop/chipxa/Neoden-YY1/neoden_converter_c_gui.exe)** | • Màn hình Intro mở đầu C Native<br>• File thực thi siêu gọn chỉ **32 KB** |

---

## 📁 Cấu trúc thư mục dự án

```text
Neoden-YY1/
├── assets/
│   └── logo.png                   # File ảnh Logo gốc hiển thị trên giao diện
│
├── rust/                          # 🦀 DỰ ÁN RUST GUI
│   ├── src/main.rs                # Mã nguồn Rust Desktop Win32 GUI
│   ├── Cargo.toml
│   └── neoden_converter_rust_gui.exe
│
├── cpp/                           # ⚡ DỰ ÁN C++ GUI
│   ├── gui_main.cpp               # Mã nguồn C++ Win32 GUI + GDI+ Logo
│   ├── converter.hpp / .cpp       # Bộ xử lý dữ liệu P&P
│   ├── CMakeLists.txt
│   └── neoden_converter_gui.exe
│
├── c/                             # 🚀 DỰ ÁN C GUI
│   ├── gui_main.c                 # Mã nguồn C thuần Win32 Desktop GUI
│   ├── converter.h / .c           # Thuật toán phân tích dữ liệu và sắp xếp
│   ├── CMakeLists.txt
│   └── neoden_converter_c_gui.exe
│
├── neoden_converter_gui.exe       # Bản C++ GUI chính ở thư mục gốc
├── neoden_converter_rust_gui.exe  # Bản Rust GUI chính ở thư mục gốc
├── neoden_converter_c_gui.exe     # Bản C GUI chính ở thư mục gốc
├── CHAY_GIAO_DIEN_GUI.bat         # Khởi chạy bản Python GUI
├── 0603Demo.csv                   # File mẫu chuẩn gốc từ nhà sản xuất NeoDen YY1
├── Pick Place for MainPCB.csv     # File dữ liệu Pick & Place mẫu từ Altium
├── Top_Output.csv / Bot_Output.csv# File CSV kết quả chuẩn 13 cột của máy YY1
├── SMT NEODEN/                    # Sách hướng dẫn sử dụng thiết bị SMT (YY1, FP2636, IN6)
└── README.md
```

---

## 🌟 Tính Năng Nổi Bật Vượt Trội

1. **Chuẩn Hóa 100% Theo Mẫu Gốc Nhà Sản Xuất ([`0603Demo.csv`](file:///c:/Users/Admin/Desktop/chipxa/Neoden-YY1/0603Demo.csv))**:
   - Tự động nạp và hiển thị đầy đủ **13 cột thông số**:
     `Designator`, `Comment`, `Footprint`, `Mid X(mm)`, `Mid Y(mm)`, `Rotation`, `Head`, `FeederNo`, `Mount Speed(%)`, `Pick Height(mm)`, `Place Height(mm)`, `Mode`, `Skip`.
   - Giữ nguyên vẹn toàn bộ phần Header máy (`PanelizedPCB`, `Fiducial`, `NozzleChange`).

2. **Chỉnh Sửa Toàn Bộ Thông Số Trực Tiếp Trên Màn Hình**:
   - Tự động nạp và chuyển đổi file Altium ngay khi mở.
   - Hỗ trợ **Tab xem riêng biệt Mặt TOP & Mặt BOTTOM**.
   - **Nhấp đúp chuột vào bất kỳ dòng nào** để chỉnh sửa mọi thông số: Tọa độ X/Y, Góc quay, Khay Feeder, Đầu hút Head, Tốc độ, Chiều cao gắp/đặt, Chế độ Vision, Bỏ qua linh kiện (Skip).
   - Nút **"💾 LƯU FILE ĐÃ CHỈNH SỬA"** lưu chính xác bản dữ liệu sau khi chỉnh sửa.

3. **Bảng Cấu Hình Khay Feeder 4 Góc (Feeder Setup Matrix)**:
   - Mô phỏng chính xác bố cục 4 góc vật lý trên máy **NeoDen YY1**:
     - 📍 **Góc Trên Bên Trái**: Khay **14 đến 24** (11 khay)
     - 📍 **Góc Trên Bên Phải**: Khay **40 đến 50** (11 khay)
     - 📍 **Góc Dưới Bên Trái**: Khay **1 đến 13** (13 khay)
     - 📍 **Góc Dưới Bên Phải**: Khay **30 đến 39** (10 khay)
   - Lưu cấu hình vào [`feeder_matrix.json`](file:///c:/Users/Admin/Desktop/chipxa/Neoden-YY1/feeder_matrix.json) để ghi nhớ vĩnh viễn giữa các phiên làm việc.
   - Tự động đối chiếu Comment/Footprint để gán số khay Feeder chính xác khi nạp file Altium mới.

4. **Trải Nghiệm Đồ Họa Cao Cấp & Logo Thương Hiệu**:
   - Tích hợp **Intro Màn Hình Chào (Splash Screen)** với Logo đồ họa GDI+ sắc nét và thanh tải hệ thống mượt mà.
   - Hiển thị bản quyền tác giả **CÔNG TY TNHH CÔNG NGHỆ CHIPXA**.
   - Biểu tượng Icon tùy chỉnh được nhúng thẳng vào file `.exe` và thanh tiêu đề cửa sổ.
