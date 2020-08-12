//
// Created by matthew on 10/08/2020.
//

#include "DeflectorSetGraphicsItem.h"

DeflectorSetGraphicsItem::DeflectorSetGraphicsItem(const QRectF &bounds, float matWidth, float matLength, Inspector *inspector)
	: inspector(inspector) {
	this->matBounds = bounds;
	this->matWidth = matWidth;
	this->matLength = matLength;

	setAcceptHoverEvents(true);

	DrawingComponentManager<Material>::addCallback([this]() { materialSource.updateSource(); });
	materialSource.updateSource();

	currentMaterialHandle = DrawingComponentManager<Material>::findComponentByID(1).handle();
}

QRectF DeflectorSetGraphicsItem::boundingRect() const {
	return matBounds;
}

void DeflectorSetGraphicsItem::setBounds(const QRectF &bounds, float width, float length) {
	matBounds = bounds;
	matWidth = width;
	matLength = length;
}

void DeflectorSetGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	painter->save();

	for (Drawing::Deflector *deflector : deflectors) {
		painter->setPen(Qt::red);
		painter->setRenderHint(QPainter::Antialiasing);
		painter->drawPath(calculateDeflectorBounds(deflector));
	}

	painter->restore();
}

void DeflectorSetGraphicsItem::addDeflector(Drawing::Deflector &deflector) {
	deflectors.push_back(&deflector);
	deflectorAcquireIDs.push_back(-1);
}

void DeflectorSetGraphicsItem::clearDeflectors() {
	deflectors.clear();
	deflectorAcquireIDs.clear();
}

float DeflectorSetGraphicsItem::deflectorSize() const {
	return currentSize;
}

Material &DeflectorSetGraphicsItem::deflectorMaterial() const {
	return DrawingComponentManager<Material>::getComponentByHandle(currentMaterialHandle);
}

bool DeflectorSetGraphicsItem::contains(const QPointF &point) const {
	for (Drawing::Deflector *deflector : deflectors) {
		if (calculateDeflectorBounds(deflector).contains(point)) {
			return true;
		}
	}
	return false;
}

void DeflectorSetGraphicsItem::setRemoveFunction(const std::function<void(const Drawing::Deflector &)> &remove) {
    removeFunction = remove;
}

void DeflectorSetGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		for (unsigned i = 0; i < deflectors.size(); i++) {
			Drawing::Deflector *deflector = deflectors[i];
			if (calculateDeflectorBounds(deflector).contains(event->pos())) {
				if (inspector) {
					if (inspector->currentOwner() != deflectorAcquireIDs[i]) {
						deflectorAcquireIDs[i] = inspector->acquire();
						inspector->addFloatField("Size:", [this](float value) {
							std::for_each(deflectors.begin(), deflectors.end(), [value](Drawing::Deflector *deflector) { deflector->size = value; });
							currentSize = value;
						}, currentSize, 1, 0);
						inspector->addFloatField("X Position:", deflector->pos.x, 1, 0);
						inspector->addFloatField("Y Position:", deflector->pos.y, 1, 0);
						inspector->addComponentField<Material>("Material:", [this](const Material &material) {
							std::for_each(deflectors.begin(), deflectors.end(), [&material](Drawing::Deflector *deflector) { deflector->setMaterial(material); });
							currentMaterialHandle = material.handle();
						}, materialSource, currentMaterialHandle == 0 ? -1 : currentMaterialHandle);
					}
				}
				break;
			}
		}
	}

	QGraphicsItem::mousePressEvent(event);
}

void DeflectorSetGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
	for (Drawing::Deflector *deflector : deflectors) {
		if (calculateDeflectorBounds(deflector).contains(event->pos())) {
			if (inspector) {
				inspector->expand();
			}
			break;
		}
	}

	QGraphicsItem::mouseDoubleClickEvent(event);
}

void DeflectorSetGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    for (Drawing::Deflector *deflector : deflectors) {
        if (calculateDeflectorBounds(deflector).contains(event->pos())) {
            QMenu *menu = new QMenu();

            menu->addAction("Remove", [this, deflector]() {
                if (removeFunction) {
                    removeFunction(*deflector);
                }
            }, Qt::Key_Delete);

            menu->popup(event->screenPos());
            break;
        }
    }
    QGraphicsItem::contextMenuEvent(event);
}

QPainterPath DeflectorSetGraphicsItem::calculateDeflectorBounds(const Drawing::Deflector *deflector) const {
	QPainterPath deflectorBounds;

	double root2 = std::sqrt(2);

	deflectorBounds.moveTo(matBounds.left() + (deflector->pos.x / matWidth) * matBounds.width(), matBounds.top() +
						   (deflector->pos.y - deflector->size / root2) / matLength * matBounds.height());
	deflectorBounds.lineTo(matBounds.left() + (deflector->pos.x + deflector->size / root2) / matWidth * matBounds.width(), matBounds.top() +
						   (deflector->pos.y / matLength) * matBounds.height());
	deflectorBounds.lineTo(matBounds.left() + (deflector->pos.x / matWidth) * matBounds.width(), matBounds.top() +
						   (deflector->pos.y + deflector->size / root2) / matLength * matBounds.height());
	deflectorBounds.lineTo(matBounds.left() + (deflector->pos.x - deflector->size / root2) / matWidth * matBounds.width(), matBounds.top() +
						   (deflector->pos.y / matLength) * matBounds.height());
	deflectorBounds.lineTo(matBounds.left() + (deflector->pos.x / matWidth) * matBounds.width(), matBounds.top() +
						   (deflector->pos.y - deflector->size / root2) / matLength * matBounds.height());

	return deflectorBounds;
}
