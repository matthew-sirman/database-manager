#include "Helper.h"

QWidget *Helper<bool>::addComponentToLayout(QFormLayout* layout,
                                            std::string name) {
    QCheckBox* box = new QCheckBox();
    layout->addRow(name.c_str(), box);
    return box;
}

QWidget* Helper<float>::addComponentToLayout(QFormLayout* layout, std::string name) {
    QDoubleSpinBox* box = new QDoubleSpinBox();
    layout->addRow(name.c_str(), box);
    return box;
};

QWidget* Helper<std::optional<float>>::addComponentToLayout(QFormLayout* layout, std::string name) {
    QDoubleSpinBox* box = new QDoubleSpinBox();
    ActivatorLabel* label = new ActivatorLabel();
    label->addTarget(box);
    label->setText(name.c_str());
    layout->addRow(label, box);
    return box;
};

QWidget* Helper<unsigned short>::addComponentToLayout(QFormLayout* layout,
                                              std::string name) {
    QSpinBox* box = new QSpinBox();
    layout->addRow(name.c_str(), box);
    return box;
}


QWidget* Helper<unsigned>::addComponentToLayout(QFormLayout* layout,
                                                std::string name) {
    QSpinBox* box = new QSpinBox();
    layout->addRow(name.c_str(), box);
    return box;
};

QWidget* Helper<std::optional<unsigned>>::addComponentToLayout(
    QFormLayout* layout, std::string name) {QSpinBox* box = new QSpinBox();
    ActivatorLabel* label = new ActivatorLabel();
    label->addTarget(box);
    label->setText(name.c_str());
    layout->addRow(label, box);
    return box;
    }

QWidget* Helper<std::string>::addComponentToLayout(QFormLayout* layout,
                                           std::string name) {
    QLineEdit* edit = new QLineEdit();
    layout->addRow(name.c_str(), edit);
    return edit;
};

QWidget* Helper<std::optional<std::string>>::addComponentToLayout(
    QFormLayout* layout,
                                                          std::string name) {
    QLineEdit* edit = new QLineEdit();
    ActivatorLabel* label = new ActivatorLabel();
    label->addTarget(edit);
    label->setText(name.c_str());
    layout->addRow(label, edit);
    return edit;
};

QWidget* Helper<std::filesystem::path>::addComponentToLayout(
    QFormLayout* layout, std::string name) {
    QHBoxLayout *l = new QHBoxLayout();
    QLineEdit* edit = new QLineEdit();
    edit->setReadOnly(true);
    QPushButton* btn = new QPushButton("Open...");
    QObject::connect(btn, &QPushButton::clicked,
            [edit, name](bool checked) { edit->setText(QFileDialog::getOpenFileName(nullptr, ("Pick " + name).c_str()));
    });
    l->addWidget(edit);
    l->addWidget(btn);
    layout->addRow(name.c_str(), l);
    return edit;
}

unsigned Helper<unsigned>::getComponent(QWidget* widget, bool isComponent) {
    if (isComponent) {
      DynamicComboBox* w = (DynamicComboBox*) widget;
        return w->itemData(w->currentIndex()).toInt();
    }
    return ((QSpinBox*)widget)->value();
}


std::optional<unsigned> Helper<std::optional<unsigned>>::getComponent(
    QWidget* widget, bool isComponent) {
    if (widget->isEnabled()) {
        return std::make_optional(Helper<unsigned>::getComponent(widget, isComponent));
    };
    return std::nullopt;
    }

float Helper<float>::getComponent(QWidget* widget, bool isComponent) {
    return (float)(((QDoubleSpinBox*)widget)->value());
}

bool Helper<bool>::getComponent(QWidget* widget, bool isComponent) {
    return ((QCheckBox*)widget)->isChecked();
}

std::string Helper<std::string>::getComponent(QWidget* widget, bool isComponent) {
    return ((QLineEdit*)widget)->text().toStdString();
};

unsigned short Helper<unsigned short>::getComponent(QWidget* widget, bool isComponent) {
    QSpinBox* b = (QSpinBox*)widget;
    return b->value();
}

std::optional<float> Helper<std::optional<float>>::getComponent(
    QWidget* widget, bool isComponent) {
    if (widget->isEnabled()) {
        return std::make_optional(Helper<float>::getComponent(widget, isComponent));
    };
    return std::nullopt;
    }

std::filesystem::path Helper<std::filesystem::path>::getComponent(
    QWidget* widget, bool isComponent) {
    QLineEdit* edit = (QLineEdit*)widget;
    return std::filesystem::path::path(edit->text().toStdString());
   }
