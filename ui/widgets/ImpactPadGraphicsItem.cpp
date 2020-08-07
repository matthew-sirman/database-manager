//
// Created by matthew on 07/08/2020.
//

#include "ImpactPadGraphicsItem.h"

ImpactPadGraphicsItem::ImpactPadGraphicsItem(QGraphicsScene *scene, const QRectF &bounds, Drawing::ImpactPad &impactPad)
	: pad(impactPad) {
	boundingBox = bounds;

	setAcceptHoverEvents(true);

	widthInputBox = new QDoubleSpinBox();
	widthInputBox->setDecimals(1);
	widthInputBox->setMinimum(0);
	widthInputBox->setMaximum(32767);
	widthInputBox->setValue(pad.width);

	widthDimensionInput = scene->addWidget(widthInputBox);
	widthDimensionLine = new WidgetDimensionLine(QRectF(), DimensionLine::HORIZONTAL, widthDimensionInput);
	scene->addItem(widthDimensionLine);

	lengthInputBox = new QDoubleSpinBox();
	lengthInputBox->setDecimals(1);
	lengthInputBox->setMinimum(0);
	lengthInputBox->setMaximum(32767);
	lengthInputBox->setValue(pad.length);

	lengthDimensionInput = scene->addWidget(lengthInputBox);
	lengthDimensionLine = new WidgetDimensionLine(QRectF(), DimensionLine::VERTICAL, lengthDimensionInput);
	scene->addItem(lengthDimensionLine);
}

QRectF ImpactPadGraphicsItem::boundingRect() const {
	return boundingBox;
}

void ImpactPadGraphicsItem::setBounds(const QRectF &bounds) {
	boundingBox = bounds;
}

void ImpactPadGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	painter->save();

	painter->setBrush(QBrush(QColor(255, 255, 0, 127)));
	painter->drawRect(boundingBox);

	if (editModeActive) {
		QRectF widthDimensionBounds(
			QPointF(boundingBox.left(), boundingBox.top() - dimensionOffset * painter->viewport().width()),
			boundingBox.topRight()
		);
		widthDimensionLine->setBounds(widthDimensionBounds);
		widthDimensionLine->paint(painter, option, widget);
		QRectF lengthDimensionBounds(
			QPointF(boundingBox.left() - dimensionOffset * painter->viewport().width(), boundingBox.top()),
			boundingBox.bottomLeft()
		);
		lengthDimensionLine->setBounds(lengthDimensionBounds);
		lengthDimensionLine->paint(painter, option, widget);
	} else {
		widthDimensionLine->setVisible(false);
		widthDimensionInput->setVisible(false);
		lengthDimensionLine->setVisible(false);
		lengthDimensionInput->setVisible(false);
	}

	painter->restore();
}

void ImpactPadGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
	if (boundingBox.contains(event->pos())) {
		editModeActive = !editModeActive;
	}

	QGraphicsItem::mouseDoubleClickEvent(event);
}
