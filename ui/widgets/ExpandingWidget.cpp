//
// Created by matthew on 09/08/2020.
//

#include "ExpandingWidget.h"

ExpandingWidget::ExpandingWidget(const QString &title, unsigned expandTime, QWidget *parent)
	: QWidget(parent), expandTime(expandTime) {
	expandButton = new QToolButton(this);
	expandButton->setStyleSheet("QToolButton { border: none; }");
	expandButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	expandButton->setArrowType(Qt::ArrowType::UpArrow);
	expandButton->setText(title);
	expandButton->setCheckable(true);
	expandButton->setChecked(false);

	headerFrame = new QFrame(this);
	headerFrame->setFrameShape(QFrame::HLine);
	headerFrame->setFrameShadow(QFrame::Sunken);
	headerFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	contentArea = new QScrollArea(this);
	contentArea->setStyleSheet("QScrollArea { background-color: white; border: none; }");
	contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	contentArea->setMaximumHeight(0);
	contentArea->setMinimumHeight(0);

	mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	unsigned row = 0;
	mainLayout->addWidget(expandButton, row, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(headerFrame, row++, 2, 1, 1);
	mainLayout->addWidget(contentArea, row, 0, 1, 3);
	setLayout(mainLayout);

	connect(expandButton, &QToolButton::clicked, [this](bool checked) {
		expandButton->setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::UpArrow);

		const unsigned collapsedHeight = expandButton->sizeHint().height();
		unsigned contentHeight = contentArea->layout()->sizeHint().height();

		if (checked) {
			setMinimumHeight(contentHeight + collapsedHeight);
			setMaximumHeight(contentHeight + collapsedHeight);
			contentArea->setMaximumHeight(contentHeight);
		} else {
			setMinimumHeight(collapsedHeight);
			setMaximumHeight(collapsedHeight);
			contentArea->setMaximumHeight(0);
		}
	});
}

ExpandingWidget::~ExpandingWidget() {
	delete expandButton;
	delete mainLayout;
	delete headerFrame;
	delete contentArea;
}

void ExpandingWidget::setContent(QLayout *layout) {
	delete contentArea->layout();
	contentArea->setLayout(layout);
}

void ExpandingWidget::expand() {
	if (!expandButton->isChecked()) {
		expandButton->click();
	}
}

void ExpandingWidget::collapse() {
	if (expandButton->isChecked()) {
		expandButton->click();
	}
}
