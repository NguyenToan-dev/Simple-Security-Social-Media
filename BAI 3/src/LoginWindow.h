//LoginWindow.h
#pragma once

#include <QTextStream>
#include <QDir>
#include <QFile>
#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QPixmap>
#include <QMessageBox>
#include <QKeyEvent>

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    QString getUsername() const;
    
    enum Mode { Login, Register };
    explicit LoginWindow(Mode mode = Login, QWidget* parent = nullptr);
    ~LoginWindow();

signals:
    void loginSuccess();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    void setupUi();
    void applyStyles();

    QLabel* imageLabel_;
    QLabel* titleLabel_;
    QLineEdit* usernameEdit_;
    QLineEdit* passwordEdit_;
    QPushButton* loginButton_;
    QPushButton* registerButton_;
    QWidget* rightPanel_;
    QVBoxLayout* mainLayout_;
    QCheckBox* showPasswordCheckBox_;
    Mode mode_;
};
