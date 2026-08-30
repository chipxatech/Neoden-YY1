use std::process::Command;
use std::env;
use std::path::Path;

fn main() {
    let out_dir = env::var("OUT_DIR").unwrap();
    let res_path = format!("{}/app.res", out_dir);
    
    // Tìm rc.exe từ Windows Kits hoặc PATH
    let rc_paths = [
        r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\rc.exe",
        r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\rc.exe",
        r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64\rc.exe",
        "rc.exe",
    ];

    let mut compiled = false;
    for rc in &rc_paths {
        let status = Command::new(rc)
            .args(&["/fo", &res_path, "app.rc"])
            .status();
            
        if let Ok(s) = status {
            if s.success() {
                compiled = true;
                break;
            }
        }
    }

    if compiled {
        println!("cargo:rustc-link-arg={}", res_path);
    }
    
    println!("cargo:rerun-if-changed=app.rc");
    println!("cargo:rerun-if-changed=app_icon.ico");
}
