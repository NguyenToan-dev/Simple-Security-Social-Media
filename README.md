# PrivyNet

PrivyNet là một ứng dụng mạng xã hội đơn giản tập trung vào bảo mật và mã hoá dữ liệu. Dự án được xây dựng bằng Qt/C++, với các thành phần chính quản lý việc đăng ký, đăng nhập, danh sách chat và hộp thoại nhắn tin.

## 🚀 Tính năng chính

* Đăng ký & Đăng nhập

* Lưu trữ thông tin người dùng an toàn với hash mật khẩu kèm salt.

* Danh sách trò chuyện

* Hiển thị các cuộc trò chuyện hiện có của người dùng.

* Cho phép tạo mới và chuyển đến cuộc trò chuyện với bạn bè.

* Hộp thoại nhắn tin

* Mã hoá nội dung trước khi lưu (AES hoặc tương đương).

* Hiển thị lịch sử tin nhắn dưới dạng cuộn.

* Singleton AuthManager

* Quản lý tập trung các chức năng đăng ký, xác thực và mã hoá.
---

## 🏗️ Kiến trúc và thành phần

###  Sơ đồ UML
![UML Diagram](UML%20DIAGRAM.png)

###  Các lớp chính

1. AuthManager (Singleton)

- Quản lý đăng ký, xác thực người dùng.

- Hàm instance() đảm bảo chỉ có một thể hiện duy nhất.

- registerUser(), validate(), isUnique().

- Mã hoá và giải mã tin nhắn qua encryptMessage() / decryptMessage().

2. LoginWindow (UI)

- Giao diện đăng nhập/đăng ký.

- Thu thập tên đăng nhập và mật khẩu.

- Bắt sự kiện click và xử lý thành công bất đồng bộ.

3. ChatListDialog (UI)

- Hiển thị danh sách các cuộc trò chuyện (QListWidget).

- Cho phép chọn hoặc tạo cuộc trò chuyện mới.

- Phát tín hiệu chatWith và chatWithNew.

4. MessageBox (UI)

- Hiển thị khung chat với bạn bè.

- Các tin nhắn được load từ file (đã mã hoá) và giải mã khi hiển thị.

- Cho phép gửi tin nhắn mới, tự động cuộn.

5. MainWindow (Coordinator)

- Chứa và điều phối các widget: LoginWindow, ChatListDialog, MessageBox.

- Sử dụng QStackedWidget để chuyển đổi giữa các màn hình.

- Kết nối tín hiệu/slot giữa các thành phần giao diện.
---

## ⚙️ Cài đặt và chạy


---

## 🔐 Bảo mật


---

## 📂 Cấu trúc thư mục

PrivyNet/
├── src/
│   ├── MainWindow.h/cpp
│   ├── LoginWindow.h/cpp
│   ├── ChatListDialog.h/cpp
│   ├── MessageBox.h/cpp
│   └── AuthManager.h/cpp
├── resources/
│   └── images, icons...
├── CMakeLists.txt hoặc PRIVYNET.pro
└── README.md


