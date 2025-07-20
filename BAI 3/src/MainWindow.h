// MainWindow.h
#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include "LoginWindow.h"
#include "ChatListDialog.h"
#include "MessageBox.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void setupConnections();

    QStackedWidget* stackedWidget;
    LoginWindow* loginWindow;
    ChatListDialog* friendDialog;
    MessageBox* messageBox;

    QString currentUser;
    QString selectedFriend;
};