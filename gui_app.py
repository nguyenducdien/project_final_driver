import customtkinter as ctk
from tkinter import messagebox
import subprocess
import os

class BaseConverterApp(ctk.CTk):
    def __init__(self):
        super().__init__()

        # --- CẤU HÌNH GIAO DIỆN CHUNG ---
        ctk.set_appearance_mode("light")
        ctk.set_default_color_theme("green")
        
        self.title("Base Converter Pro - USB Integrated")
        self.geometry("600x650")
        self.resizable(False, False)
        
        self.log_content = []
        
        if not os.path.exists("./cli_app"):
            messagebox.showwarning("Cảnh báo", "Không tìm thấy './cli_app'. Hãy bấm Terminal gõ 'make' trước khi thử nhé!")
        
        # Cấp quyền ẩn
        subprocess.run(["chmod", "+x", "./save_usb_driver.sh"], stderr=subprocess.DEVNULL)
        subprocess.run(["chmod", "+x", "./read_usb_driver.sh"], stderr=subprocess.DEVNULL)

        self.setup_ui()

    def setup_ui(self):
        # Tiêu đề
        self.title_label = ctk.CTkLabel(self, text="HỆ THỐNG CHUYỂN ĐỔI CƠ SỐ", 
                                        font=ctk.CTkFont(family="Inter", size=24, weight="bold"),
                                        text_color="#4fa94d")
        self.title_label.pack(pady=(30, 20))

        # --- KHUNG NHẬP LIỆU ---
        self.frame_inputs = ctk.CTkFrame(self, fg_color="transparent")
        self.frame_inputs.pack(pady=10)

        # 1. Từ Hệ
        self.lbl_from = ctk.CTkLabel(self.frame_inputs, text="Hệ Cơ Số Nguồn", font=ctk.CTkFont(size=15))
        self.lbl_from.grid(row=0, column=0, padx=15, pady=15, sticky="e")
        
        self.entry_from = ctk.CTkEntry(self.frame_inputs, width=220, placeholder_text="Ví dụ: 10", 
                                       font=ctk.CTkFont(size=14), corner_radius=8, border_width=1)
        self.entry_from.grid(row=0, column=1, padx=10, pady=15)

        # 2. Sang Hệ
        self.lbl_to = ctk.CTkLabel(self.frame_inputs, text="Hệ Cơ Số Đích", font=ctk.CTkFont(size=15))
        self.lbl_to.grid(row=1, column=0, padx=15, pady=15, sticky="e")
        
        self.entry_to = ctk.CTkEntry(self.frame_inputs, width=220, placeholder_text="Ví dụ: 2", 
                                     font=ctk.CTkFont(size=14), corner_radius=8, border_width=1)
        self.entry_to.grid(row=1, column=1, padx=10, pady=15)

        # 3. Số Cần Chuyển
        self.lbl_num = ctk.CTkLabel(self.frame_inputs, text="Số Cần Chuyển Đổi:", font=ctk.CTkFont(size=15))
        self.lbl_num.grid(row=2, column=0, padx=15, pady=15, sticky="e")
        
        self.entry_num = ctk.CTkEntry(self.frame_inputs, width=220, placeholder_text="Ví dụ: 255", 
                                      font=ctk.CTkFont(size=14), corner_radius=8, border_width=1)
        self.entry_num.grid(row=2, column=1, padx=10, pady=15)

        # --- KHUNG NÚT BẤM CÁC CHỨC NĂNG ---
        self.frame_btns = ctk.CTkFrame(self, fg_color="transparent")
        self.frame_btns.pack(pady=(15, 20))

        self.btn_convert = ctk.CTkButton(self.frame_btns, text="CHUYỂN ĐỔI", 
                                         font=ctk.CTkFont(size=14, weight="bold"),
                                         hover_color="#3e8e41",
                                         corner_radius=8, width=150, height=45,
                                         command=self.convert)
        self.btn_convert.grid(row=0, column=0, padx=10)

        self.btn_save = ctk.CTkButton(self.frame_btns, text="LƯU VÀO USB", 
                                      font=ctk.CTkFont(size=14, weight="bold"),
                                      fg_color="#d9534f", hover_color="#c9302c", 
                                      corner_radius=8, width=150, height=45,
                                      command=self.save_to_usb)
        self.btn_save.grid(row=0, column=1, padx=10)

        self.btn_read = ctk.CTkButton(self.frame_btns, text="ĐỌC TỪ USB", 
                                      font=ctk.CTkFont(size=14, weight="bold"),
                                      fg_color="#f0ad4e", hover_color="#ec971f", 
                                      corner_radius=8, width=150, height=45,
                                      command=self.read_from_usb)
        self.btn_read.grid(row=0, column=2, padx=10)

        # --- BẢNG HIỂN THỊ LOGS / KẾT QUẢ ---
        self.lbl_log = ctk.CTkLabel(self, text="Nhật Ký Kết Quả Từ Kernel:", font=ctk.CTkFont(size=14, weight="bold"))
        self.lbl_log.pack(anchor="w", padx=40)

        self.textbox = ctk.CTkTextbox(self, width=520, height=180, font=ctk.CTkFont(family="Consolas", size=13),
                                      corner_radius=10, border_width=1)
        self.textbox.pack(pady=5)
        self.textbox.configure(state="disabled")

    def append_log(self, text):
        self.textbox.configure(state="normal")
        self.textbox.insert("end", text + "\n")
        self.textbox.yview("end")
        self.textbox.configure(state="disabled")
        self.log_content.append(text)

    def convert(self):
        f = self.entry_from.get().strip()
        t = self.entry_to.get().strip()
        n = self.entry_num.get().strip()
        
        if not f or not t or not n:
            messagebox.showerror("Thiếu thông tin", "Vui lòng nhập đầy đủ Từ hệ, Sang hệ và Số cần chuyển!")
            return
            
        try:
            res = subprocess.run(["./cli_app", f, t, n], capture_output=True, text=True, errors="replace")
            if res.returncode != 0:
                self.append_log(f"[LỖI] {res.stdout.strip() or 'Giao tiếp Driver Kernel thất bại!'}")
                return
            
            output = res.stdout.strip()
            msg = f"[TÍNH TOÁN] {n} (Hệ {f})  ➜  {output} (Hệ {t})"
            self.append_log(msg)
            
        except Exception as e:
            messagebox.showerror("Lỗi Thực Thi", f"Không thể gọi cli_app. Chi tiết lỗi: {str(e)}")

    def save_to_usb(self):
        if not self.log_content:
            messagebox.showinfo("Trống", "Chưa có kết quả tính toán nào hiện trên màn hình để lưu cả!")
            return
            
        full_text ="\n".join([line for line in self.log_content if not line.startswith(">>>")])
        
        self.append_log("\n>>> TIẾN TRÌNH: Mã hóa AES văn bản thông qua Kernel Driver...")
        self.update()
        
        # Băm nhỏ văn bản làm các khối 120 ký tự để ném xuống bộ đệm 128 bytes của Kernel
        ciphertext_hex = ""
        chunk_size = 120
        for i in range(0, len(full_text), chunk_size):
            chunk = full_text[i:i+chunk_size]
            res = subprocess.run(["./crypto_cli", "1", chunk], capture_output=True, text=True, errors="replace")
            if res.returncode == 0:
                ciphertext_hex += res.stdout.strip()
            else:
                messagebox.showerror("Lỗi Mã Hóa", "Giao tiếp mã hóa lỗi! Bạn đã make lại chưa?")
                return
        
        self.append_log(">>> TIẾN TRÌNH: Đang ngắt Module Driver (bc_usb) ...")
        self.update()
        
        try:
            p = subprocess.Popen(["sudo", "bash", "./save_usb_driver.sh"], 
                                 stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, errors="replace")
            out, err = p.communicate(input=ciphertext_hex)
            
            if p.returncode == 0:
                self.append_log(">>> LƯU FILE: Ghi đè ketqua_chuyendoi.txt lên chính USB đó!")
                self.append_log(">>> KẾT NỐI: Trả lại phần cứng cho Kernel Driver C.")
                self.append_log(">>> HOÀN TẤT! Có thể chuyển đổi số tiếp theo bình thường.")
                messagebox.showinfo("Thành công Tuyệt Đối", "USB đã ghi đè dữ liệu siêu tốc và cài lại Driver như cũ!")
            else:
                self.append_log(f">>> [LỖI THÁO RÁP USB]: {out}\n{err}")
                messagebox.showerror("Thất Bại Dây Chuyền", f"Quá trình tháo ráp lỗi:\n{err}")
        except Exception as e:
            messagebox.showerror("Lỗi Cấp Quyền", str(e))

    def read_from_usb(self):
        self.append_log("\n>>> TIẾN TRÌNH: Đang ngắt Module Driver để tiến hành ĐỌC FILE từ lưu trữ...")
        self.update()
        
        try:
            p = subprocess.Popen(["sudo", "bash", "./read_usb_driver.sh"], 
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, errors="replace")
            out, err = p.communicate()
            
            if p.returncode == 0:
                enc_hex = out.replace('\n', '').replace('\r', '').strip()
                self.append_log(">>> KẾT QUẢ TRÍCH XUẤT TỪ USB (ĐANG GIẢI MÃ...):")
                if enc_hex:
                    decrypted_text = ""
                    # Cipher HEX size for BUF_SIZE 128 is 256 chars
                    hex_chunk_size = 256
                    for i in range(0, len(enc_hex), hex_chunk_size):
                        h_chunk = enc_hex[i:i+hex_chunk_size]
                        res = subprocess.run(["./crypto_cli", "2", h_chunk], capture_output=True, text=True, errors="replace")
                        if res.returncode == 0:
                            # Strip NULL padding added during encryption
                            decrypted_text += res.stdout.strip("\x00").strip() + "\n"
                    
                    if decrypted_text.strip():
                        self.append_log(decrypted_text.strip())
                    else:
                        self.append_log(enc_hex) # Fallback raw print if not encrypted
                
                self.append_log(">>> KẾT NỐI: Đã trả lại phần cứng cho Kernel Driver C tính toán.")
            else:
                self.append_log(f">>> [LỖI TRÍCH XUẤT USB]: {err.strip()}")
                messagebox.showerror("Thất Bại Quá Trình Đọc", f"Quá trình tháo ráp lỗi:\n{err.strip()}")
        except Exception as e:
            messagebox.showerror("Lỗi Cấp Quyền", str(e))

if __name__ == "__main__":
    app = BaseConverterApp()
    app.mainloop()
