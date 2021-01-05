#ifndef ITEMEDITOR_H
#define ITEMEDITOR_H

#include <QAbstractItemModel>
#include <QDataWidgetMapper>
#include <QMainWindow>
#include <QStandardItemModel>

namespace Ui {
class ItemEditor;
}

class ItemEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit ItemEditor(QWidget *parent = nullptr);
    ~ItemEditor();

    void setListNumbers(QStringList numbers);
    bool checkNumber(QStringList numbers, QString number);
    void setModel(QAbstractItemModel *model, bool isEdit, QString old_code, int min_count);
    void setModel(QStandardItemModel *model, bool isEdit);
    QDataWidgetMapper *mapper;

signals:
    void remainderNewItem(QString value);
    void setRemainder(QString code, QString reminder);
    void changeOrderItem(QString code, QString new_name);

private slots:
    void on_btn_accept_clicked();

    void on_btn_cancle_clicked();

private:
    Ui::ItemEditor *ui;
    bool isEdit;
    QString old_code;
    QStringList numbers;
    int reminder;
};

#endif // ITEMEDITOR_H
