#pragma once

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QCryptographicHash>
#include <QScrollBar>
#include <QResizeEvent>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QListWidgetItem>
#include <QString>

QT_BEGIN_NAMESPACE
class QListWidget;
class QLineEdit;
class QPushButton;
class QLabel;
QT_END_NAMESPACE

class ChatListDialog : public QDialog {
    Q_OBJECT

public:
    explicit ChatListDialog(const QString& currentUser, QWidget* parent = nullptr);

signals:
    void backRequested();
    void chatWith(const QString& friendName);
    void chatWithNew(const QString& friendName);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onBack();
    void onItemActivated(QListWidgetItem* item);
    void onOkClicked();

private:
    void setupUi();
    void loadExistingChats();

    QString me_;
    QVBoxLayout* mainLayout_;
    QListWidget* chatList_;
    QLineEdit* inputEdit_;
    QPushButton* okBtn_;
    QPushButton* backBtn_;
    QLabel* titleLabel_;
    QLabel* logoLabel_;
};
