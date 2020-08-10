//
// Created by matthew on 09/08/2020.
//

#include "Inspector.h"

Inspector::Inspector(const QString &title) {
	contents = new QFormLayout();
	expandingContainer = new ExpandingWidget(title, 200);
	expandingContainer->setContent(contents);
}

Inspector::~Inspector() {
	delete expandingContainer;
	delete contents;
}

QFormLayout *Inspector::layout() const {
	return contents;
}

ExpandingWidget *Inspector::container() const {
	return expandingContainer;
}

unsigned Inspector::currentOwner() const {
	return owner;
}

unsigned Inspector::acquire() {
	while (contents->count() != 0) {
		QLayoutItem *toDelete = contents->takeAt(0);
		delete toDelete->widget();
		delete toDelete;
	}
	currentRow = 0;
	return ++owner;
}

void Inspector::addIntegerField(const QString &label, int &field, int min, int max) {
	QSpinBox *fieldInput = new QSpinBox();
	fieldInput->setMinimum(min);
	fieldInput->setMaximum(max);
	fieldInput->setValue(field);
	QWidget::connect(fieldInput, &QSpinBox::editingFinished, [this, &field, fieldInput]() {
		field = fieldInput->value();
		std::for_each(updateTriggers.begin(), updateTriggers.end(), [](const std::function<void()> &trigger) { trigger(); });
	});

	contents->addRow(label, fieldInput);
}

void Inspector::addFloatField(const QString &label, float &field, unsigned precision, float min, float max) {
	QDoubleSpinBox *fieldInput = new QDoubleSpinBox();
	fieldInput->setMinimum(min);
	fieldInput->setMaximum(max);
	fieldInput->setDecimals(precision);
	fieldInput->setValue(field);
	QWidget::connect(fieldInput, &QDoubleSpinBox::editingFinished, [this, &field, fieldInput]() {
		field = fieldInput->value();
		std::for_each(updateTriggers.begin(), updateTriggers.end(), [](const std::function<void()> &trigger) { trigger(); });
	});

	contents->addRow(label, fieldInput);
}

void Inspector::addDoubleField(const QString &label, double &field, unsigned precision, double min, double max) {
	QDoubleSpinBox *fieldInput = new QDoubleSpinBox();
	fieldInput->setMinimum(min);
	fieldInput->setMaximum(max);
	fieldInput->setDecimals(precision);
	fieldInput->setValue(field);
	QWidget::connect(fieldInput, &QDoubleSpinBox::editingFinished, [this, &field, fieldInput]() {
		field = fieldInput->value();
		std::for_each(updateTriggers.begin(), updateTriggers.end(), [](const std::function<void()> &trigger) { trigger(); });
	});

	contents->addRow(label, fieldInput);
}

void Inspector::expand() {
	expandingContainer->expand();
}

void Inspector::collapse() {
	expandingContainer->collapse();
}

void Inspector::addUpdateTrigger(const std::function<void()> &trigger) {
	updateTriggers.push_back(trigger);
}
