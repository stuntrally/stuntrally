#include "AddPropertyDialog.hpp"
#include "ui_addpropertydialog.h"

AddPropertyDialog::AddPropertyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddPropertyDialog)
{
    ui->setupUi(this);
}

AddPropertyDialog::~AddPropertyDialog()
{
    delete ui;
}
