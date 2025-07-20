// MessageBox.h
#pragma once
#include "AuthManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QPalette>
#include <QScrollBar>
#include <QTimer>
#include <QDir>
#include <QWidget>
#include <QString>
#include <QFileInfo>


QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QWidget;
class QLabel;
class QLineEdit;
class QPushButton;
QT_END_NAMESPACE

class MessageBox : public QWidget
{
    Q_OBJECT
public:
    explicit MessageBox(const QString& currentUser,
        const QString& friendName,
        const QString& chatKey,
        QWidget* parent = nullptr);

signals:
    void backRequested();

private slots:
    void onSendClicked();
    void onBackClicked();

private:
    void setupUi();
    void addMessage(const QString& text, bool sentByMe);
    void loadHistory();

    QString chatKey_;
    QString me_, friend_;
    QString chatFilePath_;
    QVBoxLayout* mainLayout_;
    QHBoxLayout* headerLayout_;
    QWidget* headerWidget_;
    QPushButton* backButton_;
    QLabel* logoLabel_;
    QScrollArea* scrollArea_;
    QWidget* scrollContent_;
    QVBoxLayout* scrollLayout_;
    QHBoxLayout* inputLayout_;
    QLineEdit* inputEdit_;
    QPushButton* sendButton_;
};