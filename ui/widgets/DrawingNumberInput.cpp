#include "drawingNumberInput.h"
#include "../build/ui_DrawingNumberinput.h"


DrawingNumberInput::DrawingNumberInput(QWidget* parent) : QLineEdit(parent), ui(new Ui::DrawingNumberInput()) {
	ui->setupUi(this);
}

DrawingNumberInput::~DrawingNumberInput() {

}

void DrawingNumberInput::focusOutEvent(QFocusEvent* e) {
	std::cout << "here" << std::endl;
	QTLineEdit::focusOutEvent(e);
}