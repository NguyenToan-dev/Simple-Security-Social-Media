// MessageBox.cpp
#include "MessageBox.h"

// Constructor của MessageBox
MessageBox::MessageBox(const QString& currentUser,
    const QString& friendName,
    const QString& chatKey,
    QWidget* parent)
    : QWidget(parent),
    me_(currentUser),
    friend_(friendName),
    chatKey_(chatKey)
{
    // Thiết lập màu nền đen cho toàn bộ cửa sổ
    QPalette pal;
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);

    // Tạo thư mục lưu trữ nếu chưa có
    QDir().mkpath("data");

    // Xác định đường dẫn file chat theo thứ tự ưu tiên:
    // 1. userA_chat_userB.txt
    // 2. userB_chat_userA.txt
    QString f1 = QString("data/%1_chat_%2.txt").arg(me_).arg(friend_);
    QString f2 = QString("data/%1_chat_%2.txt").arg(friend_).arg(me_);
    chatFilePath_ = QFile::exists(f1) ? f1 : (QFile::exists(f2) ? f2 : f1);

    setupUi();  // Tạo giao diện
    setWindowTitle(friend_);  // Đặt tiêu đề là tên người bạn
    resize(400, 600);  // Đặt kích thước mặc định

    loadHistory();  // Tải tin nhắn cũ lên giao diện
}

// Thiết lập giao diện người dùng
void MessageBox::setupUi()
{
    // Layout chính chứa toàn bộ các thành phần
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);

    // ===== HEADER =====
    headerWidget_ = new QWidget(this);
    headerWidget_->setFixedHeight(50);
    headerWidget_->setStyleSheet("background-color:#1e1e1e;");
    headerLayout_ = new QHBoxLayout(headerWidget_);
    headerLayout_->setContentsMargins(10, 0, 10, 0);

    // Nút quay lại
    backButton_ = new QPushButton("< Back", headerWidget_);
    backButton_->setStyleSheet("color:white; background:transparent; border:none; font-weight:bold;");
    connect(backButton_, &QPushButton::clicked, this, &MessageBox::onBackClicked);
    headerLayout_->addWidget(backButton_);

    // Tên bạn bè (ở giữa header)
    QLabel* name = new QLabel(friend_, headerWidget_);
    name->setStyleSheet("color:white; font-size:16px;");
    headerLayout_->addStretch();
    headerLayout_->addWidget(name);
    headerLayout_->addStretch();

    // Logo bên phải
    logoLabel_ = new QLabel(headerWidget_);
    QPixmap pix(":/image/logo.png");
    logoLabel_->setPixmap(pix.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    headerLayout_->addWidget(logoLabel_);

    mainLayout_->addWidget(headerWidget_);

    // ===== KHU VỰC TIN NHẮN (CUỘN ĐƯỢC) =====
    scrollArea_ = new QScrollArea(this);
    scrollArea_->setFrameShape(QFrame::NoFrame);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setStyleSheet("background-color:black;");

    scrollContent_ = new QWidget(scrollArea_);
    scrollLayout_ = new QVBoxLayout(scrollContent_);
    scrollLayout_->setAlignment(Qt::AlignTop);
    scrollContent_->setLayout(scrollLayout_);
    scrollArea_->setWidget(scrollContent_);

    mainLayout_->addWidget(scrollArea_);

    // ===== THANH NHẬP TIN NHẮN =====
    QWidget* inputWidget = new QWidget(this);
    inputWidget->setFixedHeight(50);
    inputWidget->setStyleSheet("background-color:#1e1e1e;");
    inputLayout_ = new QHBoxLayout(inputWidget);
    inputLayout_->setContentsMargins(10, 5, 10, 5);

    // Ô nhập liệu
    inputEdit_ = new QLineEdit(inputWidget);
    inputEdit_->setStyleSheet("background-color:#2e2e2e; color:white; border-radius:10px; padding:5px;");
    connect(inputEdit_, &QLineEdit::returnPressed, this, &MessageBox::onSendClicked);  // Enter = Gửi

    // Nút gửi
    sendButton_ = new QPushButton("Send", inputWidget);
    sendButton_->setStyleSheet("background-color:#3778c2; color:white; padding:5px 15px; border:none; border-radius:10px;");
    connect(sendButton_, &QPushButton::clicked, this, &MessageBox::onSendClicked);

    inputLayout_->addWidget(inputEdit_);
    inputLayout_->addWidget(sendButton_);
    mainLayout_->addWidget(inputWidget);
}

// Đọc file lịch sử tin nhắn và hiển thị ra màn hình
void MessageBox::loadHistory()
{
    QFile f(chatFilePath_);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&f);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        // 1) Tách sender và cipher qua dấu ':' đầu tiên
        int sep = line.indexOf(':');
        QString sender = line.left(sep);
        QString cipher = line.mid(sep + 1);

        // 2) Lấy tên file không đuôi .txt làm key cho giải mã
        QString baseName = QFileInfo(chatFilePath_).fileName().split('.').first();
        QString plain = AuthManager::instance().decryptMessage(baseName, cipher);

        // 3) Tách phần nội dung thật sự của tin nhắn sau "] ..."
        int idx = plain.indexOf("] ");
        QString textPart = (idx >= 0 ? plain.mid(idx + 2) : plain);

        // 4) Lấy nội dung sau ": "
        int colon = textPart.indexOf(": ");
        QString content = (colon >= 0 ? textPart.mid(colon + 2) : textPart);

        // 5) Xác định xem tin nhắn do mình gửi hay bạn gửi
        bool sentByMe = (sender == me_);

        // 6) Thêm vào giao diện
        addMessage(content, sentByMe);
    }

    f.close();
}

// Thêm 1 tin nhắn mới vào giao diện
void MessageBox::addMessage(const QString& text, bool sentByMe)
{
    QWidget* bubble = new QWidget(scrollContent_);
    QHBoxLayout* hl = new QHBoxLayout(bubble);
    hl->setContentsMargins(5, 2, 5, 2);

    QLabel* lbl = new QLabel(text, bubble);
    lbl->setWordWrap(true);
    lbl->setMaximumWidth(250);

    // Nếu mình gửi: hiển thị bên phải, màu xanh
    if (sentByMe) {
        lbl->setStyleSheet("background-color:#3778c2; color:white; padding:8px; border-radius:10px;");
        hl->addStretch();
        hl->addWidget(lbl);
    }
    // Nếu người khác gửi: hiển thị bên trái, màu xám
    else {
        lbl->setStyleSheet("background-color:#444; color:white; padding:8px; border-radius:10px;");
        hl->addWidget(lbl);
        hl->addStretch();
    }

    scrollLayout_->addWidget(bubble);

    // Tự động cuộn xuống cuối cùng sau 50ms
    QTimer::singleShot(50, [this]() {
        scrollArea_->verticalScrollBar()->setValue(scrollArea_->verticalScrollBar()->maximum());
        });
}

// Xử lý sự kiện khi người dùng nhấn nút gửi hoặc Enter
void MessageBox::onSendClicked()
{
    QString msg = inputEdit_->text().trimmed();
    if (msg.isEmpty()) return;  // Không gửi nếu rỗng

    addMessage(msg, true);  // Hiển thị ngay

    // Tạo tin nhắn định dạng "[thời gian] người gửi: nội dung"
    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString plain = QString("[%1] %2: %3").arg(ts, me_, msg);

    // Mã hóa tin nhắn với key là tên file (không đuôi)
    QString baseName = QFileInfo(chatFilePath_).fileName().split('.').first();
    QString cipher = AuthManager::instance().encryptMessage(baseName, plain);

    // Ghi tin nhắn vào file, định dạng: người gửi : ciphertext
    QFile f(chatFilePath_);
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&f);
        out << me_ << ":" << cipher << "\n";
        f.close();
    }

    inputEdit_->clear();  // Xóa ô nhập
}

// Gửi tín hiệu quay lại khi người dùng nhấn nút Back
void MessageBox::onBackClicked()
{
    emit backRequested();  // Emit tín hiệu để bên ngoài biết
    close();  // Đóng cửa sổ
}
