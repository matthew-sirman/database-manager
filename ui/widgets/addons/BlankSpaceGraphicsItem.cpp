//
// Created by matthew on 07/08/2020.
//

#include "BlankSpaceGraphicsItem.h"

BlankSpaceGraphicsItem::BlankSpaceGraphicsItem(const QRectF& bounds, Drawing::BlankSpace& blankSpace,
	Inspector* inspector)
	: space(blankSpace), inspector(inspector) {
	boundingBox = bounds;

	setAcceptHoverEvents(true);

	static std::unordered_map<unsigned char, unsigned char> shapeOrder = {
		{1, 1},
		{2, 4},
		{3, 2},
		{4, 3},
		{5, 5},
		{6, 0}
	};
}

QRectF BlankSpaceGraphicsItem::boundingRect() const {
	return boundingBox;
}

void BlankSpaceGraphicsItem::setBounds(const QRectF& bounds) {
	boundingBox = bounds;
}

void BlankSpaceGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	painter->save();

	painter->setBrush(QBrush(QColor(200, 200, 200, 127)));
	painter->drawRect(boundingBox);

	painter->restore();
}

bool BlankSpaceGraphicsItem::contains(const QPointF& point) const {
	return boundingBox.contains(point);
}

void BlankSpaceGraphicsItem::setRemoveFunction(const std::function<void()>& remove) {
	removeFunction = remove;
}

void BlankSpaceGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (boundingBox.contains(event->pos()) && event->button() == Qt::LeftButton) {
		if (inspector) {
			if (inspector->currentOwner() != inspectorAcquireID) {
				inspectorAcquireID = inspector->acquire();
				inspector->addFloatField("Width:", space.width, 1, 0);
				inspector->addFloatField("Length:", space.length, 1, 0);
				inspector->addFloatField("X Position:", space.pos.x, 1, 0);
				inspector->addFloatField("Y Position:", space.pos.y, 1, 0);
			}
		}
	}

	QGraphicsItem::mousePressEvent(event);
}

void BlankSpaceGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
	if (boundingBox.contains(event->pos())) {
		if (inspector) {
			inspector->expand();
		}
	}

	QGraphicsItem::mouseDoubleClickEvent(event);
}

void BlankSpaceGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	if (boundingBox.contains(event->pos())) {
		QMenu* menu = new QMenu();

		menu->addAction("Remove", removeFunction, Qt::Key_Delete);

		menu->popup(event->screenPos());

		QWidget::connect(menu, &QMenu::triggered, [menu](QAction*) { menu->deleteLater(); });
	}

	QGraphicsItem::contextMenuEvent(event);
}