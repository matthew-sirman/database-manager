//
// Created by matthew on 07/08/2020.
//

#include "DamBarGraphicsItem.h"

DamBarGraphicsItem::DamBarGraphicsItem(const QRectF& bounds, Drawing::DamBar& damBar,
	Inspector* inspector)
	: bar(damBar), inspector(inspector) {
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

QRectF DamBarGraphicsItem::boundingRect() const {
	return boundingBox;
}

void DamBarGraphicsItem::setBounds(const QRectF& bounds) {
	boundingBox = bounds;
}

void DamBarGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	painter->save();

	painter->setBrush(QBrush(QColor(255, 50, 50, 127)));
	painter->drawRect(boundingBox);

	painter->restore();
}

bool DamBarGraphicsItem::contains(const QPointF& point) const {
	return boundingBox.contains(point);
}

void DamBarGraphicsItem::setRemoveFunction(const std::function<void()>& remove) {
	removeFunction = remove;
}

void DamBarGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (boundingBox.contains(event->pos()) && event->button() == Qt::LeftButton) {
		if (inspector) {
			if (inspector->currentOwner() != inspectorAcquireID) {
				inspectorAcquireID = inspector->acquire();
				inspector->addFloatField("Width:", bar.width, 1, 0);
				inspector->addFloatField("Length:", bar.length, 1, 0);
				inspector->addFloatField("Thickness:", bar.thickness, 1, 0);
				inspector->addFloatField("X Position:", bar.pos.x, 1, 0);
				inspector->addFloatField("Y Position:", bar.pos.y, 1, 0);
			}
		}
	}

	QGraphicsItem::mousePressEvent(event);
}

void DamBarGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
	if (boundingBox.contains(event->pos())) {
		if (inspector) {
			inspector->expand();
		}
	}

	QGraphicsItem::mouseDoubleClickEvent(event);
}

void DamBarGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	if (boundingBox.contains(event->pos())) {
		QMenu* menu = new QMenu();

		menu->addAction("Remove", removeFunction, Qt::Key_Delete);

		menu->popup(event->screenPos());

		QWidget::connect(menu, &QMenu::triggered, [menu](QAction*) { menu->deleteLater(); });
	}

	QGraphicsItem::contextMenuEvent(event);
}