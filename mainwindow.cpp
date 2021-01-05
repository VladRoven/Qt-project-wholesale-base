#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "xlsxdocument.h"
#include "xlsxformat.h"

#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSortFilterProxyModel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    item_editor = new ItemEditor(this);
    order_editor = new OrderEditor(this);

    connect(item_editor, &ItemEditor::remainderNewItem, this, &MainWindow::remainderNewItem);
    connect(item_editor, &ItemEditor::setRemainder, this, &MainWindow::setRemainder);
    connect(item_editor, &ItemEditor::changeOrderItem, this, &MainWindow::changeOrderItem);

    connect(order_editor, &OrderEditor::getReminder, this, &MainWindow::getReminder);
    connect(order_editor, &OrderEditor::getDate, this, &MainWindow::getDate);
    connect(order_editor, &OrderEditor::updateReminder, this, &MainWindow::updateReminder);
    connect(order_editor, &OrderEditor::setOrderNumber, this, &MainWindow::setOrderNumber);
    connect(order_editor, &OrderEditor::changesReminders, this, &MainWindow::changesReminders);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QStringList MainWindow::getNumberList()
{
    QStringList list;
    for (int i = 0; i < model_items->rowCount(); i++)
    {
        list.append(model_items->item(i, 0)->text());
    }

    return list;
}

int MainWindow::getReminder(QString code)
{
    for (int i = 0; i < model_items->rowCount(); i++)
    {
        if (model_items->item(i, 0)->text().contains(code))
            return model_items->item(i, 4)->text().toInt();
    }
    return 1;
}

QString MainWindow::getDate(QString code)
{
    for (int i = 0; i < model_items->rowCount(); i++)
    {
        if (model_items->item(i, 0)->text().contains(code))
            return model_items->item(i, 2)->text();
    }
    return QDate::currentDate().toString();
}

void MainWindow::setRemainder(QString code, QString reminder)
{
    for (int i = 0; i < model_items->rowCount(); i++)
    {
        if (model_items->item(i, 0)->text().contains(code))
        {
            model_items->setData(model_items->item(i, 4)->index(), reminder);
            break;
        }
    }
}

void MainWindow::updateReminder(QString code, int count)
{
    for (int i = 0; i < model_items->rowCount(); i++)
    {
        if (model_items->item(i, 0)->text().contains(code))
        {
            model_items->setData(model_items->item(i, 4)->index(), QString::number(model_items->item(i, 4)->text().toInt() - count));
            break;
        }
    }
}

void MainWindow::changesReminders(QString old_code, QString new_code, int old_count, int new_count)
{
    for (int i = 0; i < model_items->rowCount(); i++)
    {
        if (old_code.contains(new_code))
        {
            if (model_items->item(i, 0)->text().contains(new_code))
            {
                if (old_count != new_count)
                {
                    if (old_count > new_count)
                        model_items->setData(model_items->item(i, 4)->index(), QString::number(model_items->item(i, 4)->text().toInt() + (old_count - new_count)));
                    else
                        model_items->setData(model_items->item(i, 4)->index(), QString::number(model_items->item(i, 4)->text().toInt() - (new_count - old_count)));
                }
            }
        }
        else
        {
            if (model_items->item(i, 0)->text().contains(old_code))
                model_items->setData(model_items->item(i, 4)->index(), QString::number(model_items->item(i, 4)->text().toInt() + old_count));
            if (model_items->item(i, 0)->text().contains(new_code))
                model_items->setData(model_items->item(i, 4)->index(), QString::number(model_items->item(i, 4)->text().toInt() - new_count));
        }
    }
}

void MainWindow::setOrderNumber()
{
    if (model_orders->rowCount() > 1)
        model_orders->setData(model_orders->index(model_orders->rowCount() - 1, 0), QString::number(model_orders->index(model_orders->rowCount() - 2, 0).data().toInt() + 1));
    else if (model_orders->rowCount() > 0)
        model_orders->setData(model_orders->index(model_orders->rowCount() - 1, 0), "1");
}

void MainWindow::changeOrderItem(QString code, QString new_name)
{
    for (int i = 0; i < model_orders->rowCount(); i++)
    {
        if (model_orders->item(i, 1)->text().right(8).contains(code))
            model_orders->setData(model_orders->item(i, 1)->index(), new_name);
    }
}

void MainWindow::remainderNewItem(QString value)
{
    model_items->setData(model_items->index(model_items->rowCount() - 1, 4), value);
}


void MainWindow::on_open_file_triggered()
{
    QString openFileName = QFileDialog::getOpenFileName(this, tr("Открыть файл"), QString(), tr("JSON (*.json)"));
    QFile jsonFile(openFileName);

    if (!jsonFile.open(QIODevice::ReadOnly))
    {
        return;
    }
    file_info = openFileName;

    model_items->clear();
    model_orders->clear();
    QByteArray saveData = jsonFile.readAll();

    QJsonDocument json_doc = QJsonDocument::fromJson(saveData);

    QStringList horizontalHeader;
    horizontalHeader.append({"Код", "Название", "Дата поступления", "Кол-во товара", "Остаток", "Категория"});
    model_items->setHorizontalHeaderLabels(horizontalHeader);

    horizontalHeader.clear();
    horizontalHeader.append({"Номер заказа", "Товар", "Дата заказа", "Кол-во товара", "Заказчик"});
    model_orders->setHorizontalHeaderLabels(horizontalHeader);

    ui->table_item->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->table_order->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QJsonArray json_items = json_doc.object()["items"].toArray();
    QJsonArray json_orders = json_doc.object()["orders"].toArray();

    QJsonArray temp_array;
    QList<QStandardItem *> temp_list;

    for (int i = 0; i < json_items.count(); i++)
    {
        temp_list.clear();
        temp_array = json_items[i].toArray();
        for (int j = 0; j < temp_array.count(); j++)
        {
             temp_list.append(new QStandardItem(temp_array[j].toString()));
        }
        model_items->insertRow(i, temp_list);
        model_items->setData(model_items->index(i, 2), QDate::fromString(model_items->data(model_items->index(i, 2)).toString(), "yyyy-MM-dd"));
    }

    for (int i = 0; i < json_orders.count(); i++)
    {
        temp_list.clear();
        temp_array = json_orders[i].toArray();
        for (int j = 0; j < temp_array.count(); j++)
        {
           temp_list.append(new QStandardItem(temp_array[j].toString()));
        }
        model_orders->insertRow(i, temp_list);
        model_orders->setData(model_orders->index(i, 2), QDate::fromString(model_orders->data(model_orders->index(i, 2)).toString(), "yyyy-MM-dd"));
    }

    ui->table_item->setModel(model_items);
    ui->table_order->setModel(model_orders);
    ui->status_line->showMessage("Открыт файл: " + file_info.baseName() + " (" + file_info.absoluteFilePath() + ")");
}

void MainWindow::on_new_file_triggered()
{
    QString saveFileName = QFileDialog::getSaveFileName(this,
                                                            tr("Новый файл"),
                                                            QString(),
                                                            tr("JSON (*.json)"));
    QFileInfo fileInfo(saveFileName);
    QDir::setCurrent(fileInfo.path());
    QFile json_file(saveFileName);

    if (!json_file.open(QIODevice::WriteOnly))
    {
        return;
    }
    file_info = saveFileName;
    json_file.close();

    model_items->clear();
    model_orders->clear();

    QStringList horizontalHeader;
    horizontalHeader.append({"Код", "Название", "Дата поступления", "Кол-во товара", "Остаток", "Категория"});
    model_items->setHorizontalHeaderLabels(horizontalHeader);

    horizontalHeader.clear();
    horizontalHeader.append({"Номер заказа", "Товар", "Дата заказа", "Кол-во товара", "Заказчик"});
    model_orders->setHorizontalHeaderLabels(horizontalHeader);

    ui->table_item->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->table_order->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->table_item->setModel(model_items);
    ui->table_order->setModel(model_orders);
    ui->status_line->showMessage("Открыт файл: " + fileInfo.baseName() + " (" + fileInfo.absoluteFilePath() + ")");
}

void MainWindow::on_search_on_name_textChanged(const QString &arg1)
{
    if (file_info.absoluteFilePath() != "")
    {
        if (arg1.length())
        {
            QString str = arg1;
            str[ 0 ] = str[ 0 ].toUpper();
            for (int i = 1; i < str.length(); i++)
            {
                if (str[i - 1] == "'")
                    str[i] = str[i].toUpper();
                else
                    str[i] = str[i].toLower();
            }
            QSortFilterProxyModel *proxy_model = new QSortFilterProxyModel();
            proxy_model->setSourceModel(model_items);
            ui->table_item->setModel(proxy_model);
            proxy_model->setFilterKeyColumn(1);
            proxy_model->setFilterRegExp(str);
            ui->status_line->showMessage("Найдено записей: " + QString::number(proxy_model->rowCount()));
        }
        else
        {
            ui->table_item->setModel(model_items);
            ui->status_line->showMessage("Всего записей: " + QString::number(model_items->rowCount()));
        }
    }
}

void MainWindow::on_search_on_code_textChanged(const QString &arg1)
{
    if (file_info.absoluteFilePath() != "")
    {
        if (arg1.length())
        {
            QSortFilterProxyModel *proxy_model = new QSortFilterProxyModel();
            proxy_model->setSourceModel(model_items);
            ui->table_item->setModel(proxy_model);
            proxy_model->setFilterKeyColumn(0);
            proxy_model->setFilterRegExp(arg1);
            ui->status_line->showMessage("Найдено записей: " + QString::number(proxy_model->rowCount()));
        }
        else
        {
            ui->table_item->setModel(model_items);
            ui->status_line->showMessage("Всего записей: " + QString::number(model_items->rowCount()));
        }
    }
}

void MainWindow::on_category_list_currentTextChanged(const QString &arg1)
{
    if (file_info.absoluteFilePath() != "")
    {
        if (arg1.contains("Всё"))
        {
            ui->table_item->setModel(model_items);
            ui->status_line->showMessage("Всего записей: " + QString::number(model_items->rowCount()));
        }
        else if (arg1.length())
        {
            QSortFilterProxyModel *proxy_model = new QSortFilterProxyModel();
            proxy_model->setSourceModel(model_items);
            ui->table_item->setModel(proxy_model);
            proxy_model->setFilterKeyColumn(5);
            proxy_model->setFilterRegExp(arg1);
            ui->status_line->showMessage("Найдено записей: " + QString::number(proxy_model->rowCount()));
        }
    }
}

void MainWindow::on_search_on_number_textChanged(const QString &arg1)
{
    if (file_info.absoluteFilePath() != "")
    {
        if (arg1.length())
        {
            QSortFilterProxyModel *proxy_model = new QSortFilterProxyModel();
            proxy_model->setSourceModel(model_orders);
            ui->table_order->setModel(proxy_model);
            proxy_model->setFilterKeyColumn(0);
            proxy_model->setFilterRegExp(arg1);
            ui->status_line->showMessage("Найдено записей: " + QString::number(proxy_model->rowCount()));
        }
        else
        {
            ui->table_order->setModel(model_orders);
            ui->status_line->showMessage("Всего записей: " + QString::number(model_orders->rowCount()));
        }
    }
}

void MainWindow::on_save_triggered()
{
    if (file_info.absoluteFilePath() != "")
    {
        QFile json_file(file_info.absoluteFilePath());
        QDir::setCurrent(file_info.path());

        if (!json_file.open(QIODevice::WriteOnly))
        {
            return;
        }

        QJsonObject json;
        QJsonArray data;
        for (int i = 0; i < model_items->rowCount(); i++)
        {
            QJsonArray row;

            for (int j = 0; j < model_items->columnCount(); j++)
            {
                row.append(QJsonValue(model_items->item(i, j)->text()));
            }
            data.append(row);
        }

        json["items"] = data;
        data = {};

        for (int i = 0; i < model_orders->rowCount(); i++)
        {
            QJsonArray row;

            for (int j = 0; j < model_orders->columnCount(); j++)
            {
                row.append(QJsonValue(model_orders->item(i, j)->text()));
            }
            data.append(row);
        }
        json["orders"] = data;
        data = {};

        QJsonDocument saveDoc(json);
        json_file.write(saveDoc.toJson());
        json_file.close();
        ui->status_line->showMessage("Файл \"" + file_info.baseName() + "\" сохранён! Путь: " + file_info.absoluteFilePath());
    }
    else
        ui->status_line->showMessage("Файл не открыт!");
}

void MainWindow::on_save_as_triggered()
{
    if (file_info.absoluteFilePath() != "")
    {
        QString saveFileName = QFileDialog::getSaveFileName(this,
                                                                tr("Сохранить файл как"),
                                                                QString(),
                                                                tr("JSON (*.json)"));
        QFileInfo fileInfo(saveFileName);
        QDir::setCurrent(fileInfo.path());
        QFile json_file(saveFileName);

        if (!json_file.open(QIODevice::WriteOnly))
        {
            return;
        }

        QJsonObject json;
        QJsonArray data;

        for (int i = 0; i < model_items->rowCount(); i++)
        {
            QJsonArray row;

            for (int j = 0; j < model_items->columnCount(); j++)
            {
                row.append(QJsonValue(model_items->item(i, j)->text()));
            }
            data.append(row);
        }

        json["items"] = data;
        data = {};

        for (int i = 0; i < model_orders->rowCount(); i++)
        {
            QJsonArray row;

            for (int j = 0; j < model_orders->columnCount(); j++)
            {
                row.append(QJsonValue(model_orders->item(i, j)->text()));
            }
            data.append(row);
        }
        json["orders"] = data;
        data = {};

        QJsonDocument saveDoc(json);
        json_file.write(saveDoc.toJson());
        json_file.close();
        ui->status_line->showMessage("Файл \"" + fileInfo.baseName() + "\" сохранён! Путь: " + fileInfo.absoluteFilePath());
    }
    else
        ui->status_line->showMessage("Файл не открыт!");
}

void MainWindow::on_del_item_clicked()
{
    if (file_info.absoluteFilePath() != "")
    {
        QModelIndex index = ui->table_item->currentIndex();
        auto model = ui->table_item->model();

        if (index.row() >= 0)
        {
            QMessageBox *msg = new QMessageBox();
            msg->setIcon(QMessageBox::Information);
            msg->setWindowTitle("Подтверждение удаления");
            msg->setText("Вы действительно хотите удалить выбранный товар?");
            msg->setInformativeText("При удалении товара будет удалена иформация о заказах, закреплённые за ним!");
            QPushButton *btn_ok = msg->addButton("Да", QMessageBox::AcceptRole);
            msg->addButton("Отмена", QMessageBox::RejectRole);
            msg->exec();

            if (msg->clickedButton() == btn_ok)
            {
                for (int i = model_orders->rowCount() - 1; i >= 0; i--)
                {
                    if (model_orders->item(i, 1)->text().contains(model->data(model->index(index.row(), 1)).toString() + "|" + model->data(model->index(index.row(), 0)).toString()))
                    {
                        model_orders->removeRow(i);
                    }
                }

                model->removeRow(index.row());
                ui->table_item->setModel(model_items);
                ui->table_order->setModel(model_orders);
            }
        }
        else
        {
            ui->status_line->showMessage("Выберите товар!");
        }
    }
}

void MainWindow::on_del_order_clicked()
{
    if (file_info.absoluteFilePath() != "")
    {
        QModelIndex index = ui->table_order->currentIndex();
        auto model = ui->table_order->model();

        if (index.row() >= 0)
        {
            QMessageBox *msg = new QMessageBox();
            msg->setIcon(QMessageBox::Information);
            msg->setWindowTitle("Подтверждение удаления");
            msg->setText("Вы действительно хотите удалить выбранный заказ?");
            msg->setInformativeText("При удалении заказа, значение остатка у товара будет изменено!");
            QPushButton *btn_ok = msg->addButton("Да", QMessageBox::AcceptRole);
            msg->addButton("Отмена", QMessageBox::RejectRole);
            msg->exec();

            if (msg->clickedButton() == btn_ok)
            {
                for (int i = 0; i < model_items->rowCount(); i++)
                {
                    if (model_items->item(i, 0)->text().contains(model->index(index.row(), 1).data().toString().right(8)))
                    {
                        model_items->setData(model_items->item(i, 4)->index(), model_items->item(i, 4)->text().toInt() + model->index(index.row(), 3).data().toInt());
                        break;
                    }
                }
                model->removeRow(index.row());
                ui->table_order->setModel(model_orders);
            }
        }
        else
        {
            ui->status_line->showMessage("Выберите заказ!");
        }
    }
}

void MainWindow::on_add_item_clicked()
{
    if (file_info.absoluteFilePath() != "")
    {
        item_editor->setModel(model_items, false);
        item_editor->setListNumbers(getNumberList());
        item_editor->setWindowModality(Qt::ApplicationModal);
        item_editor->show();
    }
}

void MainWindow::on_table_item_doubleClicked(const QModelIndex &index)
{
    if (file_info.absoluteFilePath() != "")
    {
        int count = 0;
        for (int i = 0; i < model_orders->rowCount(); i++)
        {
            if (model_orders->item(i, 1)->text().right(8).contains(ui->table_item->model()->index(index.row(), 0).data().toString()))
                count += model_orders->item(i, 3)->text().toInt();
        }

        item_editor->setModel(ui->table_item->model(), true, ui->table_item->model()->index(index.row(), 0).data().toString(), count);
        item_editor->mapper->setCurrentModelIndex(index);
        item_editor->setListNumbers(getNumberList());
        item_editor->setWindowModality(Qt::ApplicationModal);
        item_editor->show();
    }
}

void MainWindow::on_add_oreder_clicked()
{
    if (file_info.absoluteFilePath() != "")
    {
        QStringList list;
        list.append("Не выбрано");

        for (int i = 0; i < model_items->rowCount(); i++)
        {
            if (model_items->item(i, 4)->text().toInt() > 0)
                list.append(model_items->item(i, 1)->text() + "|" + model_items->item(i, 0)->text());
        }

        order_editor->setModel(model_orders, false);
        order_editor->setItemList(list);
        order_editor->setWindowModality(Qt::ApplicationModal);
        order_editor->show();
    }
}

void MainWindow::on_table_order_doubleClicked(const QModelIndex &index)
{
    if (file_info.absoluteFilePath() != "")
    {
        QStringList list;
        list.append("Не выбрано");

        for (int i = 0; i < model_items->rowCount(); i++)
        {
            list.append(model_items->item(i, 1)->text() + "|" + model_items->item(i, 0)->text());
        }

        order_editor->setItemList(list);
        order_editor->setModel(ui->table_order->model(), true, ui->table_order->model()->index(index.row(), 3).data().toInt(), ui->table_order->model()->index(index.row(), 1).data().toString());
        order_editor->mapper->setCurrentModelIndex(index);
        order_editor->setWindowModality(Qt::ApplicationModal);
        order_editor->show();
    }
}

void MainWindow::on_to_excel_for_category_triggered()
{
    if (file_info.absoluteFilePath() != "")
    {
        QXlsx::Document excel;
        QXlsx::Format excel_format;
        excel_format.setFontBold(true);
        excel.setColumnWidth(1, 6, 23);
        excel.setRowFormat(1, excel_format);
        excel.setRowFormat(4, excel_format);
        excel.write("A1", "Категория");
        excel.write("A2", ui->category_list->currentText());
        excel.write("A4", "Код");
        excel.write("B4", "Название");
        excel.write("C4", "Дата поступления");
        excel.write("D4", "Кол-во товара");
        excel.write("E4", "Остаток");
        excel.write("F4", "Категория");

        int itr = 5;
        for (int i = 0; i < model_items->rowCount(); i++)
        {
            if (ui->category_list->currentText().contains("Всё"))
            {
                excel.write("A" + QString::number(itr), model_items->item(i, 0)->text());
                excel.write("B" + QString::number(itr), model_items->item(i, 1)->text());
                excel.write("C" + QString::number(itr), QDate::fromString(model_items->item(i, 2)->text(), "yyyy-MM-dd").toString("dd.MM.yyyy"));
                excel.write("D" + QString::number(itr), model_items->item(i, 3)->text());
                excel.write("E" + QString::number(itr), model_items->item(i, 4)->text());
                excel.write("F" + QString::number(itr), model_items->item(i, 5)->text());
                ++itr;
            }
            else if (model_items->item(i, 5)->text().contains(ui->category_list->currentText()))
            {
                excel.write("A" + QString::number(itr), model_items->item(i, 0)->text());
                excel.write("B" + QString::number(itr), model_items->item(i, 1)->text());
                excel.write("C" + QString::number(itr), QDate::fromString(model_items->item(i, 2)->text(), "yyyy-MM-dd").toString("dd.MM.yyyy"));
                excel.write("D" + QString::number(itr), model_items->item(i, 3)->text());
                excel.write("E" + QString::number(itr), model_items->item(i, 4)->text());
                excel.write("F" + QString::number(itr), model_items->item(i, 5)->text());
                ++itr;
            }
        }

        QString saveFileName = QFileDialog::getSaveFileName(this,
                                                                tr("Новый файл"),
                                                                QString("Категория " + ui->category_list->currentText()),
                                                                tr("Excel (*.xlsx)"));
        excel.saveAs(saveFileName);
    }
}

void MainWindow::on_to_excel_all_item_triggered()
{
    if (file_info.absoluteFilePath() != "")
    {
        QXlsx::Document excel;
        QXlsx::Format excel_format;
        excel_format.setFontBold(true);
        excel.setColumnWidth(1, 6, 23);
        excel.setRowFormat(1, excel_format);
        excel.write("A1", "Код");
        excel.write("B1", "Название");
        excel.write("C1", "Дата поступления");
        excel.write("D1", "Кол-во товара");
        excel.write("E1", "Остаток");
        excel.write("F1", "Категория");

        int itr = 2;
        for (int i = 0; i < model_items->rowCount(); i++)
        {
            excel.write("A" + QString::number(itr), model_items->item(i, 0)->text());
            excel.write("B" + QString::number(itr), model_items->item(i, 1)->text());
            excel.write("C" + QString::number(itr), QDate::fromString(model_items->item(i, 2)->text(), "yyyy-MM-dd").toString("dd.MM.yyyy"));
            excel.write("D" + QString::number(itr), model_items->item(i, 3)->text());
            excel.write("E" + QString::number(itr), model_items->item(i, 4)->text());
            excel.write("F" + QString::number(itr), model_items->item(i, 5)->text());
            ++itr;
        }

        QString saveFileName = QFileDialog::getSaveFileName(this,
                                                                tr("Новый файл"),
                                                                QString("Все товары"),
                                                                tr("Excel (*.xlsx)"));
        excel.saveAs(saveFileName);
    }
}

void MainWindow::on_to_excel_all_order_triggered()
{
    if (file_info.absoluteFilePath() != "")
    {
        QXlsx::Document excel;
        QXlsx::Format excel_format;
        excel_format.setFontBold(true);
        excel.setColumnWidth(1, 5, 23);
        excel.setRowFormat(1, excel_format);
        excel.write("A1", "Номер заказа");
        excel.write("B1", "Товар");
        excel.write("C1", "Дата заказа");
        excel.write("D1", "Кол-во товара");
        excel.write("E1", "Заказчик");

        int itr = 2;
        for (int i = 0; i < model_orders->rowCount(); i++)
        {
            excel.write("A" + QString::number(itr), model_orders->item(i, 0)->text());
            excel.write("B" + QString::number(itr), model_orders->item(i, 1)->text());
            excel.write("C" + QString::number(itr), QDate::fromString(model_orders->item(i, 2)->text(), "yyyy-MM-dd").toString("dd.MM.yyyy"));
            excel.write("D" + QString::number(itr), model_orders->item(i, 3)->text());
            excel.write("E" + QString::number(itr), model_orders->item(i, 4)->text());
            ++itr;
        }

        QString saveFileName = QFileDialog::getSaveFileName(this,
                                                                tr("Новый файл"),
                                                                QString("Все заказы"),
                                                                tr("Excel (*.xlsx)"));
        excel.saveAs(saveFileName);
    }
}
