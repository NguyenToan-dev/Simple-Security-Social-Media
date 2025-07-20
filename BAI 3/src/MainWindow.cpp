// MainWindow.cpp
#include "MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    stackedWidget(new QStackedWidget(this)),
    loginWindow(new LoginWindow),
    friendDialog(nullptr),
    messageBox(nullptr)
{
    // Chỉ thêm loginWindow ngay từ đầu
    stackedWidget->addWidget(loginWindow); // index 0
    setCentralWidget(stackedWidget);
    stackedWidget->setCurrentWidget(loginWindow);

    setupConnections();
}

void MainWindow::setupConnections()
{
    // Khi đăng nhập thành công
    connect(loginWindow, &LoginWindow::loginSuccess, this, [this]() {
        // Lưu username và tạo ChatListDialog
        currentUser = loginWindow->getUsername();
        if (friendDialog) {
            // nếu đã tồn tại, xoá và tạo lại để load data mới
            stackedWidget->removeWidget(friendDialog);
            delete friendDialog;
        }
        friendDialog = new ChatListDialog(currentUser);
        stackedWidget->addWidget(friendDialog); // index 1
        stackedWidget->setCurrentWidget(friendDialog);

        // Kết nối back từ ChatListDialog
        connect(friendDialog, &ChatListDialog::backRequested, this, [this]() {
            stackedWidget->setCurrentWidget(loginWindow);
            });
        // Khi chọn bạn cũ hoặc nhập mới
        auto openChat = [this](const QString& friendName) {
            selectedFriend = friendName;
            if (messageBox) {
                stackedWidget->removeWidget(messageBox);
                delete messageBox;
            }

            // compute the key exactly the same way MessageBox expects it:
            QString chatKey = QString("%1_chat_%2")
                .arg(currentUser)
                .arg(selectedFriend);

            messageBox = new MessageBox(currentUser, selectedFriend, chatKey);

            stackedWidget->addWidget(messageBox); // index 2...
            stackedWidget->setCurrentWidget(messageBox);
            connect(messageBox, &MessageBox::backRequested, this, [this]() {
                stackedWidget->setCurrentWidget(friendDialog);
                });
            };
        connect(friendDialog, &ChatListDialog::chatWith, this, openChat);
        connect(friendDialog, &ChatListDialog::chatWithNew, this, openChat);
        });
}
