#ifndef ORDEREDITOR_H
#define ORDEREDITOR_H

#include <QAbstractItemModel>
#include <QDataWidgetMapper>
#include <QMainWindow>
#include <QStandardItemModel>

namespace Ui {
class OrderEditor;
}

class OrderEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit OrderEditor(QWidget *parent = nullptr);
    ~OrderEditor();

    void setItemList(QStringList items);
    void setMaxCount(int max);
    void setModel(QAbstractItemModel *model, bool isEdit, int current_count, QString old_name);
    void setModel(QStandardItemModel *model, bool isEdit);
    QDataWidgetMapper *mapper;

signals:
    int getReminder(QString code);
    QString getDate(QString code);
    void updateReminder(QString code, int count);
    void setOrderNumber();
    void changesReminders(QString old_code, QString new_code, int old_count, int new_count);

private slots:
    void on_item_list_currentTextChanged(const QString &arg1);

    void on_btn_accept_clicked();

    void on_btn_cancle_clicked();

private:
    Ui::OrderEditor *ui;
    bool isEdit;
    int current_count;
    QString old_name;
};

#endif // ORDEREDITOR_H
