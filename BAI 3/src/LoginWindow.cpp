// LoginWindow.cpp
#include "LoginWindow.h"
#include "AuthManager.h"

// Constructor của LoginWindow, khởi tạo UI và kết nối các tín hiệu
LoginWindow::LoginWindow(Mode mode, QWidget* parent)
    : QWidget(parent), mode_(mode)
{
    // Thiết lập kích thước và tiêu đề cửa sổ
    setFixedSize(800, 500);
    setWindowTitle(mode_ == Login ? "Đăng nhập PrivyNet" : "Đăng ký tài khoản");

    // Đặt màu nền trắng
    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    setupUi();         // Thiết lập giao diện
    applyStyles();     // Thiết lập kiểu giao diện

    // Kết nối sự kiện nhấn nút với xử lý tương ứng
    connect(loginButton_, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(registerButton_, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);

    // Toggle hiện/ẩn mật khẩu khi checkbox thay đổi
    connect(showPasswordCheckBox_, &QCheckBox::toggled, [=](bool checked) {
        passwordEdit_->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        });

    // Lọc sự kiện bàn phím cho input
    usernameEdit_->installEventFilter(this);
    passwordEdit_->installEventFilter(this);
}

// Destructor
LoginWindow::~LoginWindow() {}

// Trả về tên đăng nhập đang nhập trong ô input
QString LoginWindow::getUsername() const { return usernameEdit_->text(); }

// Thiết lập toàn bộ layout và widget
void LoginWindow::setupUi()
{
    // Ảnh minh họa bên trái
    imageLabel_ = new QLabel(this);
    QPixmap pix(":/image/loginpic.png");
    if (!pix.isNull())
        imageLabel_->setPixmap(pix.scaled(350, 400, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    else
        imageLabel_->setText("Không tìm thấy ảnh");

    imageLabel_->setFixedWidth(350);
    imageLabel_->setScaledContents(true);

    // Panel bên phải chứa các input và nút
    rightPanel_ = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel_);
    rightLayout->setContentsMargins(30, 30, 30, 30);
    rightLayout->setSpacing(20);

    // Tiêu đề chính
    titleLabel_ = new QLabel(mode_ == Login ? "Đăng nhập PrivyNet" : "Đăng ký tài khoản", this);
    titleLabel_->setAlignment(Qt::AlignCenter);

    // Dòng nhập tên người dùng
    QHBoxLayout* userLayout = new QHBoxLayout();
    QLabel* userIcon = new QLabel("👤", this);
    userIcon->setFixedWidth(24);
    usernameEdit_ = new QLineEdit(this);
    usernameEdit_->setPlaceholderText("Tên đăng nhập");
    userLayout->addWidget(userIcon);
    userLayout->addWidget(usernameEdit_);

    // Dòng nhập mật khẩu
    QHBoxLayout* passLayout = new QHBoxLayout();
    QLabel* passIcon = new QLabel("🔒", this);
    passIcon->setFixedWidth(24);
    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setPlaceholderText("Mật khẩu");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passLayout->addWidget(passIcon);
    passLayout->addWidget(passwordEdit_);

    // Checkbox hiện mật khẩu
    showPasswordCheckBox_ = new QCheckBox("Hiển thị mật khẩu", this);

    // Nút đăng nhập hoặc đăng ký (tùy mode)
    loginButton_ = new QPushButton(mode_ == Login ? "Đăng nhập" : "Đăng ký", this);
    // Nút chuyển chế độ (đăng ký hoặc trở lại)
    registerButton_ = new QPushButton(mode_ == Login ? "Đăng ký" : "Trở lại đăng nhập", this);

    // Thêm các widget vào layout bên phải
    rightLayout->addWidget(titleLabel_);
    rightLayout->addLayout(userLayout);
    rightLayout->addLayout(passLayout);
    rightLayout->addWidget(showPasswordCheckBox_);
    rightLayout->addStretch();
    rightLayout->addWidget(loginButton_);
    rightLayout->addWidget(registerButton_);

    // Layout chính chia đôi trái/phải
    QHBoxLayout* fullLayout = new QHBoxLayout();
    fullLayout->setSpacing(0);
    fullLayout->setContentsMargins(0, 0, 0, 0);
    fullLayout->addWidget(imageLabel_);
    fullLayout->addWidget(rightPanel_);

    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->addLayout(fullLayout);
    setLayout(mainLayout_);
}

// Thiết lập font, màu sắc và kiểu hiển thị cho các widget
void LoginWindow::applyStyles()
{
    // Tiêu đề
    QFont titleFont("Segoe UI", 20, QFont::Bold);
    titleLabel_->setFont(titleFont);
    titleLabel_->setStyleSheet("color: #22223B;");

    // Ô nhập
    QFont inputFont("Segoe UI", 12);
    QString inputStyle = R"(
        QLineEdit {
            padding: 8px;
            border: 1.5px solid #4A4E69;
            border-radius: 6px;
            background-color: #F8F8FF;
            color: #22223B;
        }
        QLineEdit:focus {
            border: 1.5px solid #0078D7;
            background-color: #fff;
        }
    )";
    usernameEdit_->setFont(inputFont);
    usernameEdit_->setStyleSheet(inputStyle);
    passwordEdit_->setFont(inputFont);
    passwordEdit_->setStyleSheet(inputStyle);

    // Nút đăng nhập
    loginButton_->setFixedHeight(38);
    loginButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #0078D7;
            color: white;
            border: none;
            border-radius: 6px;
            font-weight: bold;
            font-size: 15px;
        }
        QPushButton:hover {
            background-color: #005A9E;
        }
    )");

    // Nút chuyển sang đăng ký / quay lại đăng nhập
    registerButton_->setFixedHeight(32);
    registerButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #A8DADC;
            color: #1D3557;
            border: none;
            border-radius: 6px;
            font-weight: bold;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #89C2D9;
        }
    )");

    // Checkbox hiển thị mật khẩu
    showPasswordCheckBox_->setFont(QFont("Segoe UI", 11));
    showPasswordCheckBox_->setStyleSheet(R"(
        QCheckBox {
            spacing: 10px;
            color: #22223B;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
        }
    )");
}

// Xử lý khi nhấn nút đăng nhập / đăng ký
void LoginWindow::onLoginClicked()
{
    QString u = usernameEdit_->text(), p = passwordEdit_->text();
    if (mode_ == Login)
    {
        // Nếu đang ở chế độ đăng nhập
        if (AuthManager::instance().validate(u, p)) {
            QMessageBox::information(this, "Thông báo", "Đăng nhập thành công");
            emit loginSuccess();  // Phát tín hiệu đăng nhập thành công
        }
        else {
            QMessageBox::warning(this, "Cảnh báo", "Sai tên hoặc mật khẩu");
        }
    }
    else {
        // Nếu đang ở chế độ đăng ký
        if (u.isEmpty() || p.isEmpty()) {
            QMessageBox::warning(this, "Cảnh báo", "Không được để trống");
            return;
        }
        if (!AuthManager::instance().isUnique(u)) {
            QMessageBox::warning(this, "Cảnh báo", "Tên đã tồn tại");
            return;
        }
        AuthManager::instance().registerUser(u, p);
        QMessageBox::information(this, "Thông báo", "Đăng ký thành công");
        emit loginSuccess();
    }
}

// Xử lý khi nhấn nút "Đăng ký" hoặc "Trở lại đăng nhập"
void LoginWindow::onRegisterClicked()
{
    // Chuyển đổi chế độ
    mode_ = (mode_ == Login ? Register : Login);

    // Cập nhật tiêu đề và nhãn nút tương ứng
    titleLabel_->setText(mode_ == Login ? "Đăng nhập PrivyNet" : "Đăng ký tài khoản");
    loginButton_->setText(mode_ == Login ? "Đăng nhập" : "Đăng ký");
    registerButton_->setText(mode_ == Login ? "Đăng ký" : "Trở lại đăng nhập");

    // Xóa các input
    usernameEdit_->clear();
    passwordEdit_->clear();
}

// Lọc sự kiện bàn phím cho các ô nhập liệu
bool LoginWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        if (obj == usernameEdit_) {
            // Nhấn xuống hoặc enter sẽ chuyển focus xuống ô mật khẩu
            if (keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                passwordEdit_->setFocus();
                return true;
            }
        }
        else if (obj == passwordEdit_) {
            if (keyEvent->key() == Qt::Key_Up) {
                // Nhấn lên sẽ chuyển lại ô tên đăng nhập
                usernameEdit_->setFocus();
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                // Nhấn enter ở ô mật khẩu thì tiến hành đăng nhập/đăng ký
                onLoginClicked();
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
