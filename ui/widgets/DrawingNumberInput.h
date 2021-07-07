#pragma once

#ifndef DATABASE_MANAGER_DRAWINGNUMBERINPUT_H
#define DATABASE_MANAGER_DRAWINGNUMBERINPUT_H

#include <QtWidgets/QLineEdit>

#include <QEvent>
#include <QStyle>
#include <QVariant>
#include <QFocusEvent>
#include <QWidget>

namespace Ui {
    class DrawingNumberInput;
}

class DrawingNumberInput : public QLineEdit {
    Q_OBJECT

public:
    explicit DrawingNumberInput(QWidget* parent = nullptr);

    ~DrawingNumberInput() override;

protected:
	void focusOutEvent(QFocusEvent* e) override;

    Ui::DrawingNumberInput* ui = nullptr;

private:

};

#endif //DATABASE_MANAGER_DRAWINGNUMBERINPUT_H