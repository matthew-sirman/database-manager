//
// Created by matthew on 07/08/2020.
//

#include "ExtraApertureGraphicsItem.h"

ExtraApertureGraphicsItem::ExtraApertureGraphicsItem(const QRectF& bounds, Drawing::ExtraAperture& extraAperture,
	Inspector* inspector)
	: extraAperture(extraAperture), inspector(inspector) {
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

	std::function<bool(const Aperture&, const Aperture&)> apertureComparator = [](const Aperture& a, const Aperture& b) {
		if (a.apertureShapeID != b.apertureShapeID) {
			return shapeOrder[a.apertureShapeID] < shapeOrder[b.apertureShapeID];
		}
		return a.width < b.width;
	};

	DrawingComponentManager<Aperture>::addCallback([this, apertureComparator]() {
		apertureSource.updateSource();
		apertureSource.sort(apertureComparator);
	});

	apertureSource.updateSource();
	apertureSource.sort(apertureComparator);
}

QRectF ExtraApertureGraphicsItem::boundingRect() const {
	return boundingBox;
}

void ExtraApertureGraphicsItem::setBounds(const QRectF& bounds) {
	boundingBox = bounds;
}

void ExtraApertureGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	painter->save();

	painter->setBrush(QBrush(QColor(141, 221, 247, 127)));
	painter->drawRect(boundingBox);

	painter->setBrush(QBrush(QColor(0, 0, 0, 255)));
	if (DrawingComponentManager<Aperture>::validComponentID(extraAperture.apertureID))
		painter->drawText(boundingBox, Qt::AlignHCenter | Qt::AlignVCenter, DrawingComponentManager<Aperture>::findComponentByID(extraAperture.apertureID).apertureName().c_str());

	painter->restore();
}

bool ExtraApertureGraphicsItem::contains(const QPointF& point) const {
	return boundingBox.contains(point);
}

void ExtraApertureGraphicsItem::setRemoveFunction(const std::function<void()>& remove) {
	removeFunction = remove;
}

void ExtraApertureGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (boundingBox.contains(event->pos()) && event->button() == Qt::LeftButton) {
		if (inspector) {
			if (inspector->currentOwner() != inspectorAcquireID) {
				inspectorAcquireID = inspector->acquire();
				inspector->addFloatField("Width:", extraAperture.width, 1, 0);
				inspector->addFloatField("Length:", extraAperture.length, 1, 0);
				inspector->addFloatField("X Position:", extraAperture.pos.x, 1, 0);
				inspector->addFloatField("Y Position:", extraAperture.pos.y, 1, 0);
				inspector->addComponentField<Aperture>("Aperture:", [this](const Aperture& aperture) {
					extraAperture.apertureID = aperture.componentID();
				}, apertureSource, DrawingComponentManager<Aperture>::validComponentID(extraAperture.apertureID) ? DrawingComponentManager<Aperture>::findComponentByID(extraAperture.apertureID).handle() : -1);
			}
		}
	}

	QGraphicsItem::mousePressEvent(event);
}

void ExtraApertureGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
	if (boundingBox.contains(event->pos())) {
		if (inspector) {
			inspector->expand();
		}
	}

	QGraphicsItem::mouseDoubleClickEvent(event);
}

void ExtraApertureGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	if (boundingBox.contains(event->pos())) {
		QMenu* menu = new QMenu();

		menu->addAction("Remove", removeFunction, Qt::Key_Delete);

		menu->popup(event->screenPos());

		QWidget::connect(menu, &QMenu::triggered, [menu](QAction*) { menu->deleteLater(); });
	}

	QGraphicsItem::contextMenuEvent(event);
}