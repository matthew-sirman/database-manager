//
// Created by matthew on 07/08/2020.
//

#include "ImpactPadGraphicsItem.h"

ImpactPadGraphicsItem::ImpactPadGraphicsItem(QGraphicsScene *scene, const QRectF &bounds, Drawing::ImpactPad &impactPad,
	Inspector *inspector)
	: pad(impactPad), inspector(inspector) {
	boundingBox = bounds;

	setAcceptHoverEvents(true);

	/*widthInputBox = new QDoubleSpinBox();
	widthInputBox->setDecimals(1);
	widthInputBox->setMinimum(0);
	widthInputBox->setMaximum(32767);
	widthInputBox->setValue(pad.width);

	QWidget::connect(widthInputBox, &QDoubleSpinBox::editingFinished, [this]() {
		pad.width = (float)widthInputBox->value();
		if (updateCallback) {
			updateCallback();
		}
	});

	widthDimensionInput = scene->addWidget(widthInputBox);
	widthDimensionInput->setZValue(1);
	widthDimensionLine = new WidgetDimensionLine(QRectF(), DimensionLine::HORIZONTAL, widthDimensionInput);
	scene->addItem(widthDimensionLine);

	lengthInputBox = new QDoubleSpinBox();
	lengthInputBox->setDecimals(1);
	lengthInputBox->setMinimum(0);
	lengthInputBox->setMaximum(32767);
	lengthInputBox->setValue(pad.length);

	QWidget::connect(lengthInputBox, &QDoubleSpinBox::editingFinished, [this]() {
		pad.length = (float)lengthInputBox->value();
		if (updateCallback) {
			updateCallback();
		}
	});

	lengthDimensionInput = scene->addWidget(lengthInputBox);
	lengthDimensionInput->setZValue(1);
	lengthDimensionLine = new WidgetDimensionLine(QRectF(), DimensionLine::VERTICAL, lengthDimensionInput);
	scene->addItem(lengthDimensionLine);*/
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

	painter->restore();
}

void ImpactPadGraphicsItem::setUpdateCallback(const std::function<void()> &callback) {
	updateCallback = callback;
}

void ImpactPadGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if (boundingBox.contains(event->pos())) {
		if (inspector) {
			if (inspector->currentOwner() != inspectorAcquireID) {
				inspectorAcquireID = inspector->acquire();
				inspector->addFloatField("Width:", pad.width, 1, 0);
				inspector->addFloatField("Length:", pad.length, 1, 0);
				inspector->addFloatField("X Position:", pad.pos.x, 1, 0);
				inspector->addFloatField("Y Position:", pad.pos.y, 1, 0);
			}
		}
	}

	QGraphicsItem::mousePressEvent(event);
}

void ImpactPadGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
	if (boundingBox.contains(event->pos())) {
		if (inspector) {
			inspector->expand();
		}
	}

	QGraphicsItem::mouseDoubleClickEvent(event);
}
