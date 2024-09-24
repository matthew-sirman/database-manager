//
// Created by alistair on 27/08/2024.
//

#include "AddComponentWindow.h"
#include "../build/ui_AddComponentWindow.h"

AddCompWindow::AddCompWindow(Client* client, std::string name,
    QWidget* parent) : QDialog(parent), ui(new Ui::AddComponentWindow()) {
	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);
    ui->titleLabel->setText(("Add new " + name).c_str());
    this->setWindowTitle(("Add new " + name).c_str());
};

QWidget *AddCompWindow::getLayoutContainer() {
    return ui->layoutContainer;
}

void AddCompWindow::setAcceptCallback(std::function<void(int)> &&callback) {
    acceptCallback = callback;
    connect(this, &QDialog::finished, acceptCallback);
}