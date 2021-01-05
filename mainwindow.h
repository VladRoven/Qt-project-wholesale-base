#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QMainWindow>
#include <QStandardItemModel>
#include "itemeditor.h"
#include "ordereditor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    QStringList getNumberList();

    int getReminder(QString code);

    QString getDate(QString code);

    void setRemainder(QString code, QString reminder);

    void updateReminder(QString code, int count);

    void changesReminders(QString old_code, QString new_code, int old_count, int new_count);

    void setOrderNumber();

    void changeOrderItem(QString code, QString new_name);

    void remainderNewItem(QString value);

    void on_open_file_triggered();

    void on_new_file_triggered();

    void on_search_on_name_textChanged(const QString &arg1);

    void on_search_on_code_textChanged(const QString &arg1);

    void on_category_list_currentTextChanged(const QString &arg1);

    void on_search_on_number_textChanged(const QString &arg1);

    void on_save_triggered();

    void on_save_as_triggered();

    void on_del_item_clicked();

    void on_del_order_clicked();

    void on_add_item_clicked();

    void on_table_item_doubleClicked(const QModelIndex &index);

    void on_add_oreder_clicked();

    void on_table_order_doubleClicked(const QModelIndex &index);

    void on_to_excel_for_category_triggered();

    void on_to_excel_all_item_triggered();

    void on_to_excel_all_order_triggered();

private:
    Ui::MainWindow *ui;
    QFileInfo file_info;
    QStandardItemModel *model_items = new QStandardItemModel;
    QStandardItemModel *model_orders = new QStandardItemModel;
    ItemEditor *item_editor;
    OrderEditor *order_editor;
};
#endif // MAINWINDOW_H
