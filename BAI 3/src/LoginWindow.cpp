//LoginWindow.cpp
#include "LoginWindow.h"
#include "AuthManager.h"

LoginWindow::LoginWindow(Mode mode, QWidget* parent)
    : QWidget(parent), mode_(mode)
{
    setFixedSize(800, 500);
    setWindowTitle(mode_ == Login ? "Đăng nhập PrivyNet" : "Đăng ký tài khoản");

    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    setupUi();
    applyStyles();

    connect(loginButton_, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(registerButton_, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);

    connect(showPasswordCheckBox_, &QCheckBox::toggled, [=](bool checked) {
        passwordEdit_->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        });

    usernameEdit_->installEventFilter(this);
    passwordEdit_->installEventFilter(this);
}
LoginWindow::~LoginWindow() {}

QString LoginWindow::getUsername() const { return usernameEdit_->text(); }

void LoginWindow::setupUi()
{
    imageLabel_ = new QLabel(this);
    QPixmap pix(":/image/loginpic.png");
    if (!pix.isNull())
        imageLabel_->setPixmap(pix.scaled(350, 400, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    else
        imageLabel_->setText("Không tìm thấy ảnh");

    imageLabel_->setFixedWidth(350);
    imageLabel_->setScaledContents(true);

    rightPanel_ = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel_);
    rightLayout->setContentsMargins(30, 30, 30, 30);
    rightLayout->setSpacing(20);

    titleLabel_ = new QLabel(mode_ == Login ? "Đăng nhập PrivyNet" : "Đăng ký tài khoản", this);
    titleLabel_->setAlignment(Qt::AlignCenter);

    QHBoxLayout* userLayout = new QHBoxLayout();
    QLabel* userIcon = new QLabel("👤", this);
    userIcon->setFixedWidth(24);
    usernameEdit_ = new QLineEdit(this);
    usernameEdit_->setPlaceholderText("Tên đăng nhập");
    userLayout->addWidget(userIcon);
    userLayout->addWidget(usernameEdit_);

    QHBoxLayout* passLayout = new QHBoxLayout();
    QLabel* passIcon = new QLabel("🔒", this);
    passIcon->setFixedWidth(24);
    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setPlaceholderText("Mật khẩu");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passLayout->addWidget(passIcon);
    passLayout->addWidget(passwordEdit_);

    showPasswordCheckBox_ = new QCheckBox("Hiển thị mật khẩu", this);

    loginButton_ = new QPushButton(mode_ == Login ? "Đăng nhập" : "Đăng ký", this);
    registerButton_ = new QPushButton(mode_ == Login ? "Đăng ký" : "Trở lại đăng nhập", this);

    rightLayout->addWidget(titleLabel_);
    rightLayout->addLayout(userLayout);
    rightLayout->addLayout(passLayout);
    rightLayout->addWidget(showPasswordCheckBox_);
    rightLayout->addStretch();
    rightLayout->addWidget(loginButton_);
    rightLayout->addWidget(registerButton_);

    QHBoxLayout* fullLayout = new QHBoxLayout();
    fullLayout->setSpacing(0);
    fullLayout->setContentsMargins(0, 0, 0, 0);
    fullLayout->addWidget(imageLabel_);
    fullLayout->addWidget(rightPanel_);

    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->addLayout(fullLayout);
    setLayout(mainLayout_);
}

void LoginWindow::applyStyles()
{
    QFont titleFont("Segoe UI", 20, QFont::Bold);
    titleLabel_->setFont(titleFont);
    titleLabel_->setStyleSheet("color: #22223B;");

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
void LoginWindow::onLoginClicked()
{
    QString u = usernameEdit_->text(), p = passwordEdit_->text();
    if(mode_==Login) 
    {
        if(AuthManager::instance().validate(u,p)) { 
            QMessageBox::information(this,"Thông báo","Đăng nhập thành công"); 
            emit loginSuccess(); 
        } 
        else QMessageBox::warning(this,"Cảnh báo","Sai tên hoặc mật khẩu");
    } else {
        if(u.isEmpty()||p.isEmpty()) { 
            QMessageBox::warning(this,"Cảnh báo","Không được để trống"); 
            return; 
        }
        if(!AuthManager::instance().isUnique(u)) { 
            QMessageBox::warning(this,"Cảnh báo","Tên đã tồn tại"); 
            return; 
        }
        AuthManager::instance().registerUser(u,p);
        QMessageBox::information(this,"Thông báo","Đăng ký thành công"); 
        emit loginSuccess();
    }
}
void LoginWindow::onRegisterClicked()
{
    // đổi mode
    mode_ = (mode_ == Login ? Register : Login);

    // cập nhật tiêu đề và nút
    titleLabel_->setText(mode_ == Login ? "Đăng nhập PrivyNet" : "Đăng ký tài khoản");
    loginButton_->setText(mode_ == Login ? "Đăng nhập" : "Đăng ký");
    registerButton_->setText(mode_ == Login ? "Đăng ký" : "Trở lại đăng nhập");

    // xóa input
    usernameEdit_->clear();
    passwordEdit_->clear();
}



bool LoginWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        if (obj == usernameEdit_) {
            if (keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                passwordEdit_->setFocus();
                return true;
            }
        }
        else if (obj == passwordEdit_) {
            if (keyEvent->key() == Qt::Key_Up) {
                usernameEdit_->setFocus();
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                onLoginClicked();
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
