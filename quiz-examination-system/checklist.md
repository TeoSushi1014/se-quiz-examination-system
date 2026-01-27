Dựa trên tài liệu **SRS v1.5** (Project: Quiz Examination System, Group 05) và yêu cầu nộp bài của môn học Công nghệ phần mềm, dưới đây là **Checklist kiểm tra chi tiết** để bạn rà soát toàn bộ dự án trước khi nộp.

Checklist này được chia theo các hạng mục đánh giá của môn học: **Program (5đ)**, **Tools (2đ)**, **Test cases (2đ)** và **Quy cách nộp bài**.

---

### PHẦN 1: KIỂM TRA CHỨC NĂNG (FUNCTIONAL REQUIREMENTS)
*Mục tiêu: Đảm bảo code chạy đúng logic đã viết trong SRS v1.5.*

#### 1. Quản lý xác thực (Authentication - UC01, UC10, UC11, UC12)
- [ ] **Đăng nhập:** Hệ thống có chặn đăng nhập nếu sai mật khẩu quá 5 lần không? (Yêu cầu khóa 30 phút).
- [ ] **Phân quyền:**
    - [ ] Student đăng nhập có bị chặn truy cập màn hình Teacher/Admin không?.
    - [ ] Admin có quyền reset mật khẩu cho user khác không?.
- [ ] **Đổi mật khẩu:** Mật khẩu mới có bị từ chối nếu dài quá 72 bytes không?.
- [ ] **Quên mật khẩu:** Màn hình Login **KHÔNG** được có nút "Forgot Password" (chức năng này chỉ dành cho Admin thực hiện thủ công).

#### 2. Ngân hàng câu hỏi (Question Bank - UC02)
- [ ] **Validation:** Khi tạo câu hỏi, hệ thống có bắt buộc phải chọn **chính xác 1 đáp án đúng** không?.
- [ ] **Ràng buộc xóa:** Khi Teacher xóa câu hỏi đang nằm trong một Quiz đang hoạt động (Active), hệ thống có hiện cảnh báo và chặn xóa không?.

#### 3. Quản lý bài thi (Quiz Management - UC03, UC06, UC08)
- [ ] **Cấu hình Quiz:**
    - [ ] Có cho phép set `Max Attempts` (số lần làm bài tối đa) là số cụ thể hoặc "Unlimited" không?.
    - [ ] Có chặn nếu `Time Limit` hoặc `Total Points` là số âm không?.
- [ ] **Shuffle (Trộn):** Tính năng trộn câu hỏi và trộn đáp án có hoạt động thực tế khi Student làm bài không?.
- [ ] **Phân quyền xóa:**
    - [ ] Teacher: Xóa Quiz có bài làm của HS $\rightarrow$ **Hệ thống phải CHẶN**.
    - [ ] Admin: Chọn "Purge Permanently" $\rightarrow$ **Hệ thống phải XÓA** cả Quiz và toàn bộ kết quả liên quan.

#### 4. Làm bài thi (Take Quiz - UC04)
- [ ] **Quy trình 3 bước:** Từ lúc đăng nhập đến lúc bắt đầu làm bài có $\le$ 3 bước không? (Login $\rightarrow$ Dashboard $\rightarrow$ Take Quiz).
- [ ] **Auto-save:** Khi chuyển câu hỏi (Next/Prev), đáp án cũ có được lưu lại không?.
- [ ] **Timer:** Khi hết giờ, hệ thống có tự động nộp bài (Auto-submit) không?.
- [ ] **Chặn làm lại:** Nếu sinh viên đã hết lượt làm bài (Max attempts reached), nút "Take Quiz" có bị mờ hoặc chặn không?.

#### 5. Chấm điểm & Báo cáo (Grading & Reporting - UC05)
- [ ] **Tính điểm:** Logic tính điểm có chính xác là: `Tổng điểm = Tổng (Điểm câu đúng)`, câu sai/bỏ trống = 0 điểm không?.
- [ ] **Hiển thị kết quả:** Nếu Quiz cài đặt `Result Visibility` là "After Quiz End", sinh viên nộp bài xong có bị ẩn điểm số cho đến khi hết hạn bài thi không?.
- [ ] **Export CSV:** File CSV xuất ra có đủ các cột: Username, Attempt Number, Score, Time Spent không?.

---

### PHẦN 2: KIỂM TRA YÊU CẦU PHI CHỨC NĂNG (NFR)
*Mục tiêu: Đạt điểm tối đa phần kỹ thuật và triển khai.*

- [ ] **NFR-02 (Bảo mật):**
    - [ ] Kiểm tra Database (Supabase): Cột `password` trong bảng `users` có phải là chuỗi mã hóa (hash) không? Tuyệt đối **KHÔNG** lưu plain text.
    - [ ] Thuật toán mã hóa có phải là **bcrypt** (work factor $\ge$ 10) như trong file `PasswordHasher.h` không?.
- [ ] **NFR-06 (Docker):**
    - [ ] Trong thư mục nộp bài có file `Dockerfile` không?.
    - [ ] Dockerfile có build được ứng dụng hoặc backend service không?.
- [ ] **Cơ sở dữ liệu:** Code C++ có đang kết nối thực sự tới **Supabase (PostgreSQL)** không? (Lưu ý: Đoạn code mẫu `main.cpp` dòng đang dùng `vector` giả lập trên RAM $\rightarrow$ Cần thay thế bằng gọi API/Database thật để đúng yêu cầu SRS).

---

### PHẦN 3: KIỂM TRA TÀI LIỆU & CÔNG CỤ (DOCS & TOOLS)
*Mục tiêu: Đạt 2 điểm Tools + 2 điểm Test cases.*

#### 1. Cấu trúc thư mục nộp bài
Tên thư mục: `ProgAndTest_Group05` (hoặc GroupXY), bên trong gồm:
- [ ] **Source code:** `.cpp`, `.h`, `.xaml`...
- [ ] **Testing document:** File Excel/Word chứa Test Cases.
- [ ] **Docker file:** File cấu hình Docker.
- [ ] **Evidence of tools:** File (Word/PDF) chứa ảnh chụp màn hình minh chứng việc nhóm đã sử dụng các công cụ:
    - [ ] Quản lý source: Ảnh chụp **GitHub/GitLab** (commit history).
    - [ ] Vẽ biểu đồ: Ảnh chụp **Diagrams.net** (hoặc công cụ tương đương).
    - [ ] Quản lý lỗi (Bug tracking): Ảnh chụp **Jira / Excel / Trello** theo dõi bug.
    - [ ] IDE: Ảnh chụp Visual Studio/VS Code.

#### 2. Tài liệu kiểm thử (Testing Document)
Phải có các Test Case khớp với Use Case trong SRS. Ví dụ mẫu:
- [ ] **TC_Auth_01 (Login Success):** Input đúng $\rightarrow$ Vào Dashboard.
- [ ] **TC_Auth_02 (Login Fail):** Input sai pass $\rightarrow$ Báo lỗi.
- [ ] **TC_Quiz_01 (Max Attempts):** Cố làm bài lần 2 khi max=1 $\rightarrow$ Hệ thống chặn.
- [ ] **TC_Admin_01 (Purge):** Admin xóa Quiz có attempt $\rightarrow$ Xóa thành công & Log ghi nhận.

---

### PHẦN 4: CHUẨN BỊ BÁO CÁO (DEMO)
*Mục tiêu: Đạt 1 điểm thuyết trình.*

- [ ] **Kịch bản Demo:** Chuẩn bị sẵn 1 kịch bản chạy mượt mà:
    1.  Đăng nhập Admin $\rightarrow$ Tạo User Teacher & Student.
    2.  Đăng nhập Teacher $\rightarrow$ Tạo Câu hỏi $\rightarrow$ Tạo Quiz (set time ngắn để test).
    3.  Đăng nhập Student $\rightarrow$ Làm bài $\rightarrow$ Kiểm tra Auto-submit khi hết giờ.
    4.  Quay lại Teacher $\rightarrow$ Xem Report $\rightarrow$ Xuất CSV.
    5.  Admin xóa Quiz để chứng minh chức năng Purge.
- [ ] **Mở sẵn công cụ:** Trước khi lên bảng, mở sẵn Git, Supabase Dashboard, và file Excel Test Case để show cho giáo viên xem khi được hỏi.

**Lưu ý quan trọng nhất:** Hãy sửa lại code phần kết nối Database nếu bạn vẫn đang dùng dữ liệu giả (`vector users` trong `main.cpp`). SRS yêu cầu rõ ràng database phải host trên Supabase Cloud.