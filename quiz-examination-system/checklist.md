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















Dựa trên tài liệu SRS, nguyên lý thiết kế giao diện (UI Design Principles) từ nguồn, và trạng thái hiện tại của dự án (Backend đã xong, UI chưa tương tác), đây là **Checklist hướng dẫn triển khai UI (No-code)** để biến giao diện tĩnh thành một ứng dụng tương tác hoàn chỉnh.

Mục tiêu là kết nối các RPC bạn vừa viết với các sự kiện (Events) trên màn hình XAML.

---

### PHẦN 1: TEACHER DASHBOARD (Ưu tiên cao nhất - Vì cần dữ liệu để Student test)

Màn hình này thực hiện **UC-02 (Question Bank)** và **UC-03 (Create Quiz)**.

#### 1. Tab "Question Bank" (Ngân hàng câu hỏi)
- [ ] **Hiển thị danh sách (Data Binding):**
    - Tạo một `ListView` hoặc `DataGrid` trong XAML.
    - **Sự kiện `Page_Loaded`:** Gọi hàm `SupabaseClient::GetQuestions()`.
    - **UI Update:** Khi dữ liệu về (callback), gán vào `ItemsSource` của ListView.
- [ ] **Chức năng Thêm mới (Add Question):**
    - Tạo nút "Add Question".
    - **Sự kiện `Click`:** Mở một `ContentDialog` chứa các TextBox (Câu hỏi, 4 đáp án) và ComboBox (Đáp án đúng, Độ khó).
    - **Nút "Save" trong Dialog:** Gọi `SupabaseClient::CreateQuestionValidated()`.
    - **Phản hồi:** Nếu thành công $\rightarrow$ Đóng Dialog, hiện thông báo xanh (TeachingTip), reload lại danh sách.
- [ ] **Chức năng Xóa (Delete Question):**
    - Thêm nút "Delete" vào mỗi dòng (hoặc nút trên Toolbar xử lý dòng đang chọn).
    - **Sự kiện `Click`:** Gọi `SupabaseClient::DeleteQuestionSafe()`.
    - **Xử lý lỗi (Quan trọng):** Nếu server trả về `blocked` (do câu hỏi đang nằm trong Quiz active) $\rightarrow$ Hiện thông báo lỗi đỏ: "Không thể xóa câu hỏi đang được sử dụng".

#### 2. Tab "Manage Quizzes" (Quản lý đề thi)
- [ ] **Hiển thị danh sách Quiz:**
    - Tương tự Question Bank, load danh sách Quiz do giáo viên tạo.
- [ ] **Tạo đề thi (Create Quiz Flow):**
    - **Bước 1 (Info):** Nhập Tên đề, Thời gian, Số lần làm bài (Max Attempts).
    - **Bước 2 (Select Questions):** Hiện danh sách câu hỏi với `CheckBox` để giáo viên tích chọn.
    - **Bước 3 (Submit):** Gọi hàm tạo Quiz, gửi kèm danh sách ID các câu hỏi đã chọn.
- [ ] **Xem Báo cáo (View Report - UC-05):**
    - Chọn một Quiz từ danh sách $\rightarrow$ Hiện nút "View Report".
    - **Sự kiện:** Gọi `GetQuizAttemptsReport`.
    - **UI:** Đổ dữ liệu vào bảng thống kê (Tên HS, Điểm, Thời gian nộp).
    - **Nút "Export CSV":** Gọi hàm download CSV bạn đã cấu hình header `Accept: text/csv`.

---

### PHẦN 2: STUDENT DASHBOARD (Luồng cốt lõi UC-04)

Đây là phần quan trọng nhất để Demo. Theo **NFR-04**, sinh viên phải vào thi được trong vòng 3 bước.

#### 1. Màn hình chính (Dashboard)
- [ ] **Danh sách đề thi (Available Quizzes):**
    - **Sự kiện `Page_Loaded`:** Gọi `GetStudentQuizzes()`.
    - **UI:** Sử dụng `GridView` (dạng thẻ bài) hiện Tên đề, Thời gian, Trạng thái (Chưa làm/Đã làm).
    - **Tương tác:** Click vào thẻ $\rightarrow$ Điều hướng sang trang `ExamPage`.

#### 2. Màn hình làm bài (Exam Page) - Quan trọng
*Lưu ý: Màn hình này cần thiết kế cẩn thận để tránh gian lận.*

- [ ] **Load đề thi:**
    - Gọi `GetQuizQuestions()`.
    - **Lưu ý:** Dữ liệu trả về **KHÔNG** được chứa trường `is_correct` (đáp án đúng) để đảm bảo bảo mật NFR-02.
- [ ] **Giao diện thi:**
    - Chia màn hình: Bên trái là danh sách số câu (Navigation), Bên phải là Nội dung câu hỏi hiện tại.
    - Dùng `RadioButton` cho 4 đáp án.
- [ ] **Điều hướng & Auto-save:**
    - Khi bấm nút "Next" hoặc "Previous" $\rightarrow$ Lưu tạm đáp án vào biến cục bộ (hoặc gọi API save nháp nếu muốn xịn).
- [ ] **Đồng hồ đếm ngược (Timer):**
    - Sử dụng `DispatcherTimer` của WinUI.
    - **Logic:** Khi timer = 0 $\rightarrow$ Tự động gọi hàm `SubmitAttempt`.
- [ ] **Nộp bài (Submit):**
    - Nút "Submit" cần hiện `ContentDialog` xác nhận: "Bạn có chắc muốn nộp bài?".
    - **Hành động:** Gọi RPC `submit_quiz_attempt` gửi kèm danh sách câu trả lời.
    - **Kết quả:** Server trả về điểm số ngay lập tức $\rightarrow$ Hiện Dialog thông báo điểm số.

---

### PHẦN 3: ADMIN DASHBOARD (Quản trị hệ thống)

#### 1. Tab "User Management"
- [ ] **Quản lý trạng thái:**
    - Danh sách User có `ToggleSwitch` cho cột "Active/Disabled".
    - **Sự kiện `Toggled`:** Gọi RPC update status.
- [ ] **Reset Mật khẩu:**
    - Nút "Reset Password" (UC-12).
    - **Logic:** Gọi API reset về mật khẩu mặc định (ví dụ: "123456") và hiện thông báo copy cho Admin.

#### 2. Tab "System Audit"
- [ ] **Xem Log:**
    - Chỉ cần `DataGrid` read-only hiển thị bảng `audit_logs` từ Supabase.
    - Giúp Admin theo dõi ai vừa login sai 5 lần (tính năng bảo mật bạn vừa làm).

---

### PHẦN 4: UI FEEDBACK & UX (Trải nghiệm người dùng)

Theo nguồn về thiết kế UI, bạn cần đảm bảo:

- [ ] **Feedback:** Mọi hành động gọi xuống Supabase (Async) đều phải hiện `ProgressRing` (vòng xoay loading) và làm mờ màn hình để người dùng biết hệ thống đang xử lý.
- [ ] **Consistency:** Các nút "Xóa" nên luôn có màu đỏ, nút "Lưu/Thêm" màu xanh (Accent Color).
- [ ] **Error Handling:** Khi RPC trả về lỗi (mất mạng, lỗi logic), phải hiện `TeachingTip` hoặc `ContentDialog` báo lỗi rõ ràng, không để app bị crash.

### Bước tiếp theo bạn cần làm:
Hãy mở file **`StudentDashboardPage.xaml`** trước.
1.  Vẽ một `GridView` có tên là `QuizListGrid`.
2.  Trong file `.cpp`, viết hàm `LoadQuizzes` gọi RPC `get_student_quizzes`.
3.  Khi có data, loop qua và thêm item vào `QuizListGrid`.

Bạn có muốn tôi viết mẫu đoạn XAML cho cái `GridView` hiển thị bài thi đẹp mắt không?