#include "itemeditor.h"
#include "ui_itemeditor.h"

#include <QMessageBox>
#include <QDebug>

ItemEditor::ItemEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ItemEditor)
{
    ui->setupUi(this);
    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
}

ItemEditor::~ItemEditor()
{
    delete ui;
}

void ItemEditor::setListNumbers(QStringList numbers)
{
    this->numbers = numbers;
}

bool ItemEditor::checkNumber(QStringList numbers, QString number)
{
    for (QString val : numbers)
    {
        if (val.contains(number))
            return true;
    }
    return false;
}

void ItemEditor::setModel(QAbstractItemModel *model, bool isEdit, QString old_code, int reminder)
{
    this->old_code = old_code;
    this->isEdit = isEdit;
    this->reminder = reminder;

    ui->count->setMinimum((reminder == 0) ? 1 : reminder);
    ui->date->setMaximumDate(QDate::currentDate());

    mapper->clearMapping();
    mapper->setModel(model);

    mapper->addMapping(ui->number, 0);
    mapper->addMapping(ui->name, 1);
    mapper->addMapping(ui->date, 2);
    mapper->addMapping(ui->count, 3);
    mapper->addMapping(ui->category_list, 5);
}

void ItemEditor::setModel(QStandardItemModel *model, bool isEdit)
{
    ui->date->setDate(QDate::currentDate());
    ui->date->setMaximumDate(QDate::currentDate());
    ui->name->clear();
    ui->number->clear();
    ui->count->setMinimum(1);
    ui->count->setValue(1);
    ui->category_list->setCurrentText("Не выбрано");

    this->isEdit = isEdit;
    mapper->clearMapping();
    mapper->setModel(model);
}

void ItemEditor::on_btn_accept_clicked()
{
    QString error = "";
    QRegExp name_reg("([А-ЯЁ][а-яё-]+\\s'[А-ЯЁ][а-яё-]+')");
    QRegExp number_reg("([0-9]{8,8})");
    if (!name_reg.exactMatch(ui->name->text()))
        error += "Корректно введите название!\n";

    if(!number_reg.exactMatch(ui->number->text()))
        error += "Корректно введите код товара!\n";
    else if (checkNumber(numbers, ui->number->text()))
        if (!old_code.contains(ui->number->text()))
            error += "Товар с данным кодом существует!\n";

    if (ui->category_list->currentText().contains("Не выбрано"))
        error += "Выберите категорию!\n";

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
            emit setRemainder(ui->number->text(), QString::number(ui->count->value() - reminder));
            emit changeOrderItem(old_code, ui->name->text() + "|" + ui->number->text());
            close();
        }
        else
        {
            mapper->model()->insertRow(mapper->model()->rowCount());
            mapper->setCurrentModelIndex(mapper->model()->index(mapper->model()->rowCount() - 1, 0));

            mapper->addMapping(ui->number, 0);
            mapper->addMapping(ui->name, 1);
            mapper->addMapping(ui->date, 2);
            mapper->addMapping(ui->count, 3);
            mapper->addMapping(ui->category_list, 5);

            mapper->submit();

            emit remainderNewItem(ui->count->text());

            close();
        }
    }
}

void ItemEditor::on_btn_cancle_clicked()
{
    close();
}
