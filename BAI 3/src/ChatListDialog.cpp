#include "ChatListDialog.h"
#include "AuthManager.h"
#include "MessageBox.h"


ChatListDialog::ChatListDialog(const QString& currentUser, QWidget* parent)
    : QDialog(parent), me_(currentUser)
{
    setWindowTitle("PrivyNet");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setupUi();
    loadExistingChats();
}

void ChatListDialog::setupUi()
{
    // Header
    QWidget* header = new QWidget(this);
    header->setFixedHeight(50);
    header->setStyleSheet("background-color:white;");
    auto* hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(10, 0, 10, 0);

    backBtn_ = new QPushButton("< Back", header);
    backBtn_->setFlat(true);
    backBtn_->setStyleSheet("font-weight:bold;color:#333;");
    connect(backBtn_, &QPushButton::clicked, this, &ChatListDialog::onBack);
    hlay->addWidget(backBtn_);

    titleLabel_ = new QLabel("CHATS", header);
    titleLabel_->setStyleSheet("font-size:18pt; font-weight:bold; color:#222;");
    titleLabel_->setAlignment(Qt::AlignCenter);
    hlay->addWidget(titleLabel_, 1);

    logoLabel_ = new QLabel(header);
    QPixmap pix(":/image/logo.png");
    logoLabel_->setPixmap(pix.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    hlay->addWidget(logoLabel_);

    // List
    chatList_ = new QListWidget(this);
    chatList_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chatList_->setStyleSheet(R"(
        QListWidget{background:white;border:none;}
        QListWidget::item{padding:0px;}
        QListWidget::item:hover{background:#f0f0f0;}
    )");
    chatList_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    connect(chatList_, &QListWidget::itemActivated, this, &ChatListDialog::onItemActivated);

    // Input bar
    QWidget* inputBar = new QWidget(this);
    inputBar->setFixedHeight(50);
    auto* ilay = new QHBoxLayout(inputBar);
    ilay->setContentsMargins(10, 5, 10, 5);

    inputEdit_ = new QLineEdit(inputBar);
    inputEdit_->setPlaceholderText("Enter friend's username");
    // Thêm dòng này để Enter = OK
    connect(inputEdit_, &QLineEdit::returnPressed, this, &ChatListDialog::onOkClicked);

    ilay->addWidget(inputEdit_, 1);

    okBtn_ = new QPushButton("OK", inputBar);
    okBtn_->setFixedWidth(60);
    connect(okBtn_, &QPushButton::clicked, this, &ChatListDialog::onOkClicked);
    ilay->addWidget(okBtn_);

    // Main layout
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);
    mainLayout_->addWidget(header);
    mainLayout_->addWidget(chatList_);
    mainLayout_->addWidget(inputBar);
    mainLayout_->setStretch(0, 0);
    mainLayout_->setStretch(1, 1);
    mainLayout_->setStretch(2, 0);
}

void ChatListDialog::loadExistingChats()
{
    QDir dir("data");
    // 1) Lấy cả hai pattern: "*_chat_me.txt" và "me_chat_*.txt"
    QStringList patterns;
    patterns << QString("*_chat_%1.txt").arg(me_)
        << QString("%1_chat_*.txt").arg(me_);
    QStringList files = dir.entryList(patterns, QDir::Files);

    // 2) Dùng set để tránh hiện duplicate nếu cả 2 file đều có
    QSet<QString> seenFriends;

    for (const QString& fname : files) {
        QStringList parts = fname.split("_chat_");
        if (parts.size() != 2) continue;

        // parts[0] = sender, parts[1] = receiver.txt
        QString a = parts[0];
        QString b = parts[1];                // e.g. "toan.txt" hoặc "alice.txt"
        QString friendName;
        if (a == me_) {
            // mẫu "me_chat_friend.txt"
            friendName = QFileInfo(b).completeBaseName();
        }
        else {
            // mẫu "friend_chat_me.txt"
            friendName = a;
        }

        if (seenFriends.contains(friendName))
            continue;
        seenFriends.insert(friendName);

        // baseName chính là tên file không có .txt
        QString baseName = QFileInfo(fname).completeBaseName();

        // Đọc dòng cuối của file fname
        QFile f(dir.filePath(fname));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        QTextStream in(&f);
        QString line, lastLine;
        while (!in.atEnd()) {
            line = in.readLine().trimmed();
            if (!line.isEmpty())
                lastLine = line;
        }
        f.close();

        // Tách cipher (bỏ "sender:")
        int sep = lastLine.indexOf(':');
        QString cipher = (sep >= 0 ? lastLine.mid(sep + 1) : lastLine);

        // Giải mã với key = baseName
        QString dec = AuthManager::instance().decryptMessage(baseName, cipher);

        // Lấy phần nội dung để preview
        int idx = dec.indexOf("] ");
        QString textPart = (idx >= 0 ? dec.mid(idx + 2) : dec);
        int col = textPart.indexOf(": ");
        QString preview = (col >= 0 ? textPart.mid(col + 2) : textPart);

        // Tạo QListWidgetItem
        QListWidgetItem* item = new QListWidgetItem(chatList_);
        QWidget* w = new QWidget;
        auto* vlay = new QVBoxLayout(w);
        vlay->setContentsMargins(10, 6, 10, 6);
        vlay->setSpacing(2);

        QLabel* nameLbl = new QLabel(friendName, w);
        nameLbl->setStyleSheet("font-weight:bold; font-size:12pt; color:#000;");
        nameLbl->setWordWrap(true);
        vlay->addWidget(nameLbl);

        QLabel* preLbl = new QLabel(preview, w);
        preLbl->setStyleSheet("color:#666; font-size:9pt;");
        preLbl->setWordWrap(true);
        vlay->addWidget(preLbl);

        w->setLayout(vlay);

        int availW = chatList_->viewport()->width()
            - chatList_->verticalScrollBar()->width()
            - chatList_->frameWidth() * 2;
        w->setMinimumWidth(availW);

        item->setSizeHint(w->sizeHint());
        chatList_->setItemWidget(item, w);

        // Lưu friendName và chatKey (baseName)
        item->setData(Qt::UserRole, friendName);
        item->setData(Qt::UserRole + 1, baseName);
    }
}



void ChatListDialog::resizeEvent(QResizeEvent* ev)
{
    QDialog::resizeEvent(ev);
    int availW = chatList_->viewport()->width()
        - chatList_->verticalScrollBar()->width()
        - chatList_->frameWidth() * 2;
    for (int i = 0; i < chatList_->count(); ++i) {
        auto* item = chatList_->item(i);
        if (auto* w = chatList_->itemWidget(item)) {
            w->setMinimumWidth(availW);
            item->setSizeHint(w->sizeHint());
        }
    }
}

void ChatListDialog::onBack()
{
    emit backRequested();
    reject();
}

void ChatListDialog::onItemActivated(QListWidgetItem* item)
{
    QString friendName = item->data(Qt::UserRole).toString();
    QString chatKey = item->data(Qt::UserRole + 1).toString();

    // Tạo MessageBox như cửa sổ riêng
    MessageBox* box = new MessageBox(me_, friendName, chatKey, nullptr);
    box->setAttribute(Qt::WA_DeleteOnClose);  // tự delete khi đóng
    // Đảm bảo nó là Window (có titlebar, không nằm trong layout ChatListDialog)
    box->setWindowFlags(
        Qt::Window
        | Qt::WindowTitleHint
        | Qt::WindowCloseButtonHint
    );
    // Show và chắc chắn đưa lên top
    box->show();
    box->raise();
    box->activateWindow();

    // Khi bấm Back trong MessageBox, chỉ cần reload preview
    connect(box, &MessageBox::backRequested, this, [this]() {
        chatList_->clear();
        loadExistingChats();
        });
}

void ChatListDialog::onOkClicked()
{
    QString f = inputEdit_->text().trimmed();
    if (f.isEmpty()) return;
    emit chatWithNew(f);
    accept();
}
