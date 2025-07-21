# PrivyNet

PrivyNet là một dự án mạng xã hội demo đơn giản, tập trung vào quyền riêng tư và bảo mật thông tin người dùng. Dự án được phát triển bằng Qt/C++, bao gồm các chức năng chính như đăng ký, đăng nhập, danh sách bạn bè và hộp thoại nhắn tin riêng tư.
Điểm nổi bật của PrivyNet là sử dụng thư viện mã hóa hiện đại [Libsodium], giúp đảm bảo:
- Mật khẩu được băm an toàn với salt
- Tin nhắn được mã hóa end-to-end bằng XSalsa20-Poly1305
- Toàn vẹn và xác thực dữ liệu qua MAC

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
- Có thể chạy thẳng từ file PrivyNet.exe trong thư mục RUN HERE hoặc coi video để cài đặt trong VS 2022.
---

## 🔐 Bảo mật
Hệ thống này áp dụng các kỹ thuật mã hóa hiện đại sử dụng chủ yếu là thư viện sodium để mã hoá nhằm bảo vệ thông tin người dùng và nội dung tin nhắn:
### ✅ Bảo vệ mật khẩu
- Mật khẩu không bao giờ được lưu trực tiếp.
- Khi đăng ký, mật khẩu được kết hợp với chuỗi muối (salt) ngẫu nhiên, sau đó băm nhiều vòng bằng thuật toán SHA‑256.
- Cú pháp lưu trữ:
```sh
username : salt : hashed_password
```
- Khi người dùng đăng nhập, mật khẩu nhập vào sẽ được băm lại và so sánh với bản lưu trữ để xác minh.
> 📌 Sử dụng kỹ thuật salt và nhiều vòng hash giúp chống lại các tấn công từ điển (dictionary attack) và rainbow table.
### ✉️ Mã hóa tin nhắn
- Tin nhắn giữa người dùng được mã hóa bằng Libsodium với thuật toán XSalsa20-Poly1305, sử dụng API crypto_secretbox_easy.
- Mỗi tin nhắn được mã hóa bằng:
+ Khóa bí mật 32 byte: được dẫn xuất từ chatKey + server secret bằng SHA‑256.
+ Nonce (số ngẫu nhiên duy nhất): 24 byte được tạo mới mỗi lần gửi tin.
- Sau khi mã hóa, dữ liệu được đóng gói thành:
```sh
base64( nonce || ciphertext )
```
> 📌 Mỗi ciphertext đều bao gồm MAC (Message Authentication Code) 16 byte, giúp đảm bảo tính toàn vẹn và xác thực.
### 🛡️ Chống sửa đổi và tiêm dữ liệu
- Giải mã chỉ thành công khi dữ liệu chưa bị sửa đổi.
- Nếu attacker cố tình thay đổi ciphertext (bit flipping), MAC sẽ không khớp và hệ thống sẽ từ chối giải mã, trả về chuỗi rỗng.
- Điều này ngăn chặn hoàn toàn các cuộc tấn công dạng tiêm mã độc vào ciphertext hoặc giả mạo tin nhắn.
### 🔒 Kết luận
Nhờ kết hợp:
- Hash mật khẩu với salt + lặp nhiều vòng.
- Mã hóa tin nhắn với nonce và MAC.
- Dẫn xuất khóa an toàn từ chatKey.
→ Hệ thống đảm bảo an toàn mật khẩu, bảo vệ nội dung truyền thông, và ngăn chặn các cuộc tấn công phổ biến trong thực tế như:
- Replay attack
- Tampering (chỉnh sửa dữ liệu)
- Ciphertext injection
---

## 📂 Cấu trúc thư mục
```plaintext
PrivyNet/
|── data/
├── src/
|   ├── image/
│   ├── MainWindow.h/cpp
│   ├── LoginWindow.h/cpp
│   ├── ChatListDialog.h/cpp
│   ├── MessageBox.h/cpp
│   └── AuthManager.h/cpp
├── Username.txt
└── README.md

```
