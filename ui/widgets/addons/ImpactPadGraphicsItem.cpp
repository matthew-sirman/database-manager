//
// Created by matthew on 07/08/2020.
//

#include "ImpactPadGraphicsItem.h"

ImpactPadGraphicsItem::ImpactPadGraphicsItem(const QRectF &bounds, Drawing::ImpactPad &impactPad,
	Inspector *inspector)
	: pad(impactPad), inspector(inspector) {
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
	std::function<bool(const Aperture &, const Aperture &)> apertureComparator = [](const Aperture &a, const Aperture &b) {
		if (a.apertureShapeID != b.apertureShapeID) {
			return shapeOrder[a.apertureShapeID] < shapeOrder[b.apertureShapeID];
		}
		return a.width < b.width;
	};

	DrawingComponentManager<Material>::addCallback([this]() { materialSource.updateSource(); });
	materialSource.updateSource();
	DrawingComponentManager<Aperture>::addCallback([this, apertureComparator]() { 
		apertureSource.updateSource(); 
		apertureSource.sort(apertureComparator); 
	});
	apertureSource.updateSource();
	apertureSource.sort(apertureComparator);
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

bool ImpactPadGraphicsItem::contains(const QPointF &point) const {
	return boundingBox.contains(point);
}

void ImpactPadGraphicsItem::setRemoveFunction(const std::function<void()> &remove) {
    removeFunction = remove;
}

void ImpactPadGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if (boundingBox.contains(event->pos()) && event->button() == Qt::LeftButton) {
		if (inspector) {
			if (inspector->currentOwner() != inspectorAcquireID) {
				Material &currentMaterial = pad.material();
				Aperture &currentAperture = pad.aperture();

				inspectorAcquireID = inspector->acquire();
				inspector->addFloatField("Width:", pad.width, 1, 0);
				inspector->addFloatField("Length:", pad.length, 1, 0);
				inspector->addFloatField("X Position:", pad.pos.x, 1, 0);
				inspector->addFloatField("Y Position:", pad.pos.y, 1, 0);
				inspector->addComponentField<Material>("Material:", [this](const Material &material) {
					pad.setMaterial(material);
				}, materialSource, currentMaterial.handle() == 0 ? -1 : currentMaterial.handle());
				inspector->addComponentField<Aperture>("Aperture:", [this](const Aperture &aperture) {
					pad.setAperture(aperture);
				}, apertureSource, currentAperture.handle() == 0 ? -1 : currentAperture.handle());
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

void ImpactPadGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
	if (boundingBox.contains(event->pos())) {
		QMenu *menu = new QMenu();

		menu->addAction("Remove", removeFunction, Qt::Key_Delete);

		menu->popup(event->screenPos());

        QWidget::connect(menu, &QMenu::triggered, [menu](QAction *) { menu->deleteLater(); });
	}

	QGraphicsItem::contextMenuEvent(event);
}