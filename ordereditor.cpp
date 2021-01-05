#include "ordereditor.h"
#include "ui_ordereditor.h"

#include <QDebug>
#include <QMessageBox>

OrderEditor::OrderEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OrderEditor)
{
    ui->setupUi(this);
    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
}

OrderEditor::~OrderEditor()
{
    delete ui;
}

void OrderEditor::setItemList(QStringList items)
{
    ui->item_list->clear();
    ui->item_list->addItems(items);
}

void OrderEditor::setMaxCount(int max)
{
    ui->count->setMaximum(max);
    ui->count->setMinimum(1);
}

void OrderEditor::setModel(QAbstractItemModel *model, bool isEdit, int current_count, QString old_name)
{
    this->current_count = current_count;
    this->isEdit = isEdit;
    this->old_name = old_name;

    ui->client->clear();
    ui->date->setMaximumDate(QDate::currentDate());

    mapper->clearMapping();
    mapper->setModel(model);

    mapper->addMapping(ui->item_list, 1);
    mapper->addMapping(ui->date, 2);
    mapper->addMapping(ui->count, 3);
    mapper->addMapping(ui->client, 4);
}

void OrderEditor::setModel(QStandardItemModel *model, bool isEdit)
{
    this->isEdit = isEdit;

    ui->client->clear();
    ui->date->setDate(QDate::currentDate());
    ui->date->setMaximumDate(QDate::currentDate());

    mapper->clearMapping();
    mapper->setModel(model);
}

void OrderEditor::on_item_list_currentTextChanged(const QString &arg1)
{
    if (!arg1.contains("Не выбрано"))
    {
        int reminder = emit getReminder(arg1.right(8));

        ui->date->setMinimumDate(QDate::fromString(emit getDate(arg1.right(8)), "yyyy-MM-dd"));

        if (isEdit && old_name.contains(arg1))
            setMaxCount(current_count + reminder);
        else
            setMaxCount(reminder);
    }
    else
    {
        ui->date->setMinimumDate(QDate::currentDate());
        setMaxCount(1);
    }
}

void OrderEditor::on_btn_accept_clicked()
{
    QString error = "";
    QRegExp client_reg("([А-ЯЁ]+\\s'[А-ЯЁ][а-яё-]+')");

    if (ui->item_list->currentText().contains("Не выбрано"))
        error += "Выберите товар!\n";

    if (!client_reg.exactMatch(ui->client->text()))
        error += "Корректно введите заказчика!\n";

    if (error.length())
    {
        QMessageBox *msg = new QMessageBox();
        msg->setIcon(QMessageBox::Warning);
        msg->setWindowTitle("Ошибка");
        msg->setInformativeText(error);
        msg->addButton("Понял", QMessageBox::AcceptRole);
        msg->setAttribute(Qt::WA_QuitOnClose, false);
        msg->exec();
    }
    else
    {
        if (isEdit)
        {
            mapper->submit();
            emit changesReminders(old_name.right(8), ui->item_list->currentText().right(8), current_count, ui->count->value());
            close();
        }
        else
        {
            mapper->model()->insertRow(mapper->model()->rowCount());
            mapper->setCurrentModelIndex(mapper->model()->index(mapper->model()->rowCount() - 1, 0));

            mapper->addMapping(ui->item_list, 1);
            mapper->addMapping(ui->date, 2);
            mapper->addMapping(ui->count, 3);
            mapper->addMapping(ui->client, 4);

            mapper->submit();

            emit updateReminder(ui->item_list->currentText().right(8), ui->count->value());
            emit setOrderNumber();

            close();
        }
    }
}

void OrderEditor::on_btn_cancle_clicked()
{
    close();
}
