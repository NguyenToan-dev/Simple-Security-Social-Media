#include "ChatListDialog.h"
#include "AuthManager.h"
#include "MessageBox.h"

// Constructor của ChatListDialog, khởi tạo giao diện và nạp các cuộc trò chuyện đã lưu
ChatListDialog::ChatListDialog(const QString& currentUser, QWidget* parent)
    : QDialog(parent), me_(currentUser)
{
    setWindowTitle("PrivyNet");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setupUi();            // Thiết lập UI
    loadExistingChats();  // Nạp danh sách cuộc trò chuyện
}

// Hàm thiết lập giao diện người dùng
void ChatListDialog::setupUi()
{
    // === HEADER ===
    QWidget* header = new QWidget(this);
    header->setFixedHeight(50);
    header->setStyleSheet("background-color:white;");
    auto* hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(10, 0, 10, 0);

    // Nút quay lại
    backBtn_ = new QPushButton("< Back", header);
    backBtn_->setFlat(true);
    backBtn_->setStyleSheet("font-weight:bold;color:#333;");
    connect(backBtn_, &QPushButton::clicked, this, &ChatListDialog::onBack);
    hlay->addWidget(backBtn_);

    // Tiêu đề CHATS
    titleLabel_ = new QLabel("CHATS", header);
    titleLabel_->setStyleSheet("font-size:18pt; font-weight:bold; color:#222;");
    titleLabel_->setAlignment(Qt::AlignCenter);
    hlay->addWidget(titleLabel_, 1);

    // Logo nhỏ bên phải
    logoLabel_ = new QLabel(header);
    QPixmap pix(":/image/logo.png");
    logoLabel_->setPixmap(pix.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    hlay->addWidget(logoLabel_);

    // === DANH SÁCH CHAT ===
    chatList_ = new QListWidget(this);
    chatList_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chatList_->setStyleSheet(R"(
        QListWidget{background:white;border:none;}
        QListWidget::item{padding:0px;}
        QListWidget::item:hover{background:#f0f0f0;}
    )");
    chatList_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    connect(chatList_, &QListWidget::itemActivated, this, &ChatListDialog::onItemActivated);

    // === THANH NHẬP TÊN NGƯỜI DÙNG ===
    QWidget* inputBar = new QWidget(this);
    inputBar->setFixedHeight(50);
    auto* ilay = new QHBoxLayout(inputBar);
    ilay->setContentsMargins(10, 5, 10, 5);

    inputEdit_ = new QLineEdit(inputBar);
    inputEdit_->setPlaceholderText("Enter friend's username");
    connect(inputEdit_, &QLineEdit::returnPressed, this, &ChatListDialog::onOkClicked);  // Nhấn Enter để xác nhận

    ilay->addWidget(inputEdit_, 1);

    // Nút OK để mở chat với bạn mới
    okBtn_ = new QPushButton("OK", inputBar);
    okBtn_->setFixedWidth(60);
    connect(okBtn_, &QPushButton::clicked, this, &ChatListDialog::onOkClicked);
    ilay->addWidget(okBtn_);

    // === LAYOUT CHÍNH ===
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);
    mainLayout_->addWidget(header);
    mainLayout_->addWidget(chatList_);
    mainLayout_->addWidget(inputBar);
    mainLayout_->setStretch(0, 0);  // header không co giãn
    mainLayout_->setStretch(1, 1);  // chatList chiếm phần lớn
    mainLayout_->setStretch(2, 0);  // input không co giãn
}

// Hàm nạp danh sách cuộc trò chuyện từ thư mục data/
void ChatListDialog::loadExistingChats()
{
    QDir dir("data");
    QStringList patterns;
    patterns << QString("*_chat_%1.txt").arg(me_)
        << QString("%1_chat_*.txt").arg(me_);  // file chat có tên dạng a_chat_b.txt
    QStringList files = dir.entryList(patterns, QDir::Files);

    QSet<QString> seenFriends;  // dùng để tránh trùng bạn bè

    for (const QString& fname : files) {
        QStringList parts = fname.split("_chat_");
        if (parts.size() != 2) continue;

        QString a = parts[0];
        QString b = parts[1];
        QString friendName;
        if (a == me_) {
            friendName = QFileInfo(b).completeBaseName();  // nếu mình là a -> friend là b
        }
        else {
            friendName = a;  // nếu mình là b -> friend là a
        }

        if (seenFriends.contains(friendName))
            continue;
        seenFriends.insert(friendName);

        QString baseName = QFileInfo(fname).completeBaseName();

        // Đọc dòng cuối cùng trong file để lấy tin nhắn gần nhất
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

        // Giải mã tin nhắn cuối cùng
        int sep = lastLine.indexOf(':');
        QString cipher = (sep >= 0 ? lastLine.mid(sep + 1) : lastLine);
        QString dec = AuthManager::instance().decryptMessage(baseName, cipher);

        // Cắt phần nội dung sau dấu "] " (bỏ timestamp)
        int idx = dec.indexOf("] ");
        QString textPart = (idx >= 0 ? dec.mid(idx + 2) : dec);

        // Cắt phần sau dấu ": " (bỏ tên người gửi)
        int col = textPart.indexOf(": ");
        QString preview = (col >= 0 ? textPart.mid(col + 2) : textPart);

        // Rút gọn tin nhắn nếu quá dài
        if (preview.length() > 32) {
            preview = preview.left(32) + "...";
        }

        // Tạo một item trong danh sách với tên bạn và tin nhắn gần nhất
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

        // Tính toán chiều rộng phù hợp để không bị cắt
        int availW = chatList_->viewport()->width()
            - chatList_->verticalScrollBar()->width()
            - chatList_->frameWidth() * 2;
        w->setMinimumWidth(availW);

        item->setSizeHint(w->sizeHint());
        chatList_->setItemWidget(item, w);

        item->setData(Qt::UserRole, friendName);     // dùng để lấy lại tên bạn khi bấm
        item->setData(Qt::UserRole + 1, baseName);   // dùng làm chatKey
    }
}

// Khi thay đổi kích thước cửa sổ, cập nhật chiều rộng widget của từng item
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

// Khi bấm nút quay lại, gửi tín hiệu và đóng hộp thoại
void ChatListDialog::onBack()
{
    emit backRequested();  // tín hiệu cho lớp cha biết đã bấm Back
    reject();              // đóng hộp thoại
}

// Khi người dùng chọn một cuộc trò chuyện
void ChatListDialog::onItemActivated(QListWidgetItem* item)
{
    QString friendName = item->data(Qt::UserRole).toString();
    QString chatKey = item->data(Qt::UserRole + 1).toString();

    // Tạo cửa sổ MessageBox cho cuộc trò chuyện này
    MessageBox* box = new MessageBox(me_, friendName, chatKey, nullptr);
    box->setAttribute(Qt::WA_DeleteOnClose);  // tự xoá khi đóng

    // Hiển thị như một cửa sổ riêng biệt
    box->setWindowFlags(
        Qt::Window
        | Qt::WindowTitleHint
        | Qt::WindowCloseButtonHint
    );
    box->show();
    box->raise();
    box->activateWindow();

    // Khi đóng MessageBox, reload lại danh sách chat
    connect(box, &MessageBox::backRequested, this, [this]() {
        chatList_->clear();
        loadExistingChats();
        });
}

// Khi nhấn OK hoặc Enter để mở chat với người dùng mới
void ChatListDialog::onOkClicked()
{
    QString f = inputEdit_->text().trimmed();
    if (f.isEmpty()) return;
    emit chatWithNew(f);  // gửi tín hiệu tạo cuộc trò chuyện mới
    accept();             // đóng hộp thoại
}
