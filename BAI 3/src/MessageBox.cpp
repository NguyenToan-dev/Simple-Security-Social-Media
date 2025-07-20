// MessageBox.cpp
#include "MessageBox.h"


MessageBox::MessageBox(const QString& currentUser,
    const QString& friendName,
    const QString& chatKey,
    QWidget* parent)
    : QWidget(parent),
    me_(currentUser),
    friend_(friendName),
    chatKey_(chatKey)
{
    QPalette pal;
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);

    QDir().mkpath("data");
    QString f1 = QString("data/%1_chat_%2.txt").arg(me_).arg(friend_);
    QString f2 = QString("data/%1_chat_%2.txt").arg(friend_).arg(me_);
    chatFilePath_ = QFile::exists(f1) ? f1 : (QFile::exists(f2) ? f2 : f1);

    setupUi();
    setWindowTitle(friend_);
    resize(400, 600);

    loadHistory();
}


void MessageBox::setupUi()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);

    // Header
    headerWidget_ = new QWidget(this);
    headerWidget_->setFixedHeight(50);
    headerWidget_->setStyleSheet("background-color:#1e1e1e;");
    headerLayout_ = new QHBoxLayout(headerWidget_);
    headerLayout_->setContentsMargins(10, 0, 10, 0);

    // Back button
    backButton_ = new QPushButton("< Back", headerWidget_);
    backButton_->setStyleSheet("color:white; background:transparent; border:none; font-weight:bold;");
    connect(backButton_, &QPushButton::clicked, this, &MessageBox::onBackClicked);
    headerLayout_->addWidget(backButton_);

    // Title
    QLabel* name = new QLabel(friend_, headerWidget_);
    name->setStyleSheet("color:white; font-size:16px;");
    headerLayout_->addStretch();
    headerLayout_->addWidget(name);
    headerLayout_->addStretch();

    // Logo
    logoLabel_ = new QLabel(headerWidget_);
    QPixmap pix(":/image/logo.png");
    logoLabel_->setPixmap(pix.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    headerLayout_->addWidget(logoLabel_);

    mainLayout_->addWidget(headerWidget_);

    // Scroll area for messages
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

    // Input bar
    QWidget* inputWidget = new QWidget(this);
    inputWidget->setFixedHeight(50);
    inputWidget->setStyleSheet("background-color:#1e1e1e;");
    inputLayout_ = new QHBoxLayout(inputWidget);
    inputLayout_->setContentsMargins(10, 5, 10, 5);
    inputEdit_ = new QLineEdit(inputWidget);
    inputEdit_->setStyleSheet(
        "background-color:#2e2e2e; color:white; border-radius:10px; padding:5px;"
    );
    // Enter key triggers send
    connect(inputEdit_, &QLineEdit::returnPressed, this, &MessageBox::onSendClicked);

    sendButton_ = new QPushButton("Send", inputWidget);
    sendButton_->setStyleSheet(
        "background-color:#3778c2; color:white; padding:5px 15px; border:none; border-radius:10px;"
    );
    connect(sendButton_, &QPushButton::clicked, this, &MessageBox::onSendClicked);

    inputLayout_->addWidget(inputEdit_);
    inputLayout_->addWidget(sendButton_);
    mainLayout_->addWidget(inputWidget);
}

void MessageBox::loadHistory()
{
    QFile f(chatFilePath_);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&f);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        // 1) Tách sender và phần cipher qua dấu ':' đầu tiên
        int sep = line.indexOf(':');
        QString sender = line.left(sep);
        QString cipher = line.mid(sep + 1);

        // Lấy tên file làm key, bỏ đuôi ".txt"
        QString baseName = QFileInfo(chatFilePath_).fileName().split('.').first();
        QString plain = AuthManager::instance().decryptMessage(baseName, cipher);


        // 4) Lấy nội dung message
        int idx = plain.indexOf("] ");
        QString textPart = (idx >= 0 ? plain.mid(idx + 2) : plain);
        int colon = textPart.indexOf(": ");
        QString content = (colon >= 0 ? textPart.mid(colon + 2) : textPart);

        // 5) Xác định ai gửi
        bool sentByMe = (sender == me_);

        // 6) Hiển thị
        addMessage(content, sentByMe);
    }
    f.close();
}



void MessageBox::addMessage(const QString& text, bool sentByMe)
{
    QWidget* bubble = new QWidget(scrollContent_);
    QHBoxLayout* hl = new QHBoxLayout(bubble);
    hl->setContentsMargins(5, 2, 5, 2);

    QLabel* lbl = new QLabel(text, bubble);
    lbl->setWordWrap(true);
    lbl->setMaximumWidth(250);
    if (sentByMe) {
        lbl->setStyleSheet("background-color:#3778c2; color:white; padding:8px; border-radius:10px;");
        hl->addStretch();
        hl->addWidget(lbl);
    }
    else {
        lbl->setStyleSheet("background-color:#444; color:white; padding:8px; border-radius:10px;");
        hl->addWidget(lbl);
        hl->addStretch();
    }
    scrollLayout_->addWidget(bubble);

    QTimer::singleShot(50, [this]() {
        scrollArea_->verticalScrollBar()->setValue(scrollArea_->verticalScrollBar()->maximum());
        });
}

void MessageBox::onSendClicked()
{
    QString msg = inputEdit_->text().trimmed();
    if (msg.isEmpty()) return;

    addMessage(msg, true);

    // Chuẩn bị
    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString plain = QString("[%1] %2: %3").arg(ts, me_, msg);

    // Lấy tên file (vd "toan_chat_alice") làm key
    QString baseName = QFileInfo(chatFilePath_).fileName().split('.').first();
    QString cipher = AuthManager::instance().encryptMessage(baseName, plain);



    QFile f(chatFilePath_);
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&f);
        // -- Ghi thêm tên người gửi trước dấu ':'
        out << me_ << ":" << cipher << "\n";
        f.close();
    }

    inputEdit_->clear();
}



void MessageBox::onBackClicked()
{
    emit backRequested();
    close();
}
