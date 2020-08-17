//
// Created by matthew on 10/08/2020.
//

#include "DivertorSetGraphicsItem.h"

DivertorSetGraphicsItem::DivertorSetGraphicsItem(const QRectF &bounds, float matWidth, float matLength, Inspector *inspector)
	: inspector(inspector) {
	this->matBounds = bounds;
	this->matWidth = matWidth;
	this->matLength = matLength;

	setAcceptHoverEvents(true);

	DrawingComponentManager<Material>::addCallback([this]() { materialSource.updateSource(); });
	materialSource.updateSource();

	currentMaterialHandle = DrawingComponentManager<Material>::findComponentByID(1).handle();
}

QRectF DivertorSetGraphicsItem::boundingRect() const {
	return matBounds;
}

void DivertorSetGraphicsItem::setBounds(const QRectF &bounds, float width, float length) {
	matBounds = bounds;
	matWidth = width;
	matLength = length;
}

void DivertorSetGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	painter->save();

	for (Drawing::Divertor *divertor : divertors) {
		painter->setPen(Qt::red);
		painter->setRenderHint(QPainter::Antialiasing);
		painter->drawPath(calculateDivertorBounds(divertor));
	}

	painter->restore();
}

void DivertorSetGraphicsItem::addDivertor(Drawing::Divertor &divertor) {
	divertors.push_back(&divertor);
	divertorAcquireIDs.push_back(-1);
}

void DivertorSetGraphicsItem::clearDivertors() {
	divertors.clear();
	divertorAcquireIDs.clear();
}

float DivertorSetGraphicsItem::divertorWidth() const {
	return currentWidth;
}

float DivertorSetGraphicsItem::divertorLength() const {
	return currentLength;
}

Material &DivertorSetGraphicsItem::divertorMaterial() const {
	return DrawingComponentManager<Material>::getComponentByHandle(currentMaterialHandle);
}

bool DivertorSetGraphicsItem::contains(const QPointF &point) const {
	for (Drawing::Divertor *divertor : divertors) {
		if (calculateDivertorBounds(divertor).contains(point)) {
			return true;
		}
	}
	return false;
}

void DivertorSetGraphicsItem::setRemoveFunction(const std::function<void(const Drawing::Divertor &)> &remove) {
    removeFunction = remove;
}

void DivertorSetGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		for (unsigned i = 0; i < divertors.size(); i++) {
			Drawing::Divertor *divertor = divertors[i];
			if (calculateDivertorBounds(divertor).contains(event->pos())) {
				if (inspector) {
					if (inspector->currentOwner() != divertorAcquireIDs[i]) {
						divertorAcquireIDs[i] = inspector->acquire();
						inspector->addFloatField("Width:", [this](float value) {
							std::for_each(divertors.begin(), divertors.end(), [value](Drawing::Divertor *divertor) { divertor->width = value; });
							currentWidth = value;
						}, currentWidth, 1, 0);
						inspector->addFloatField("Length:", [this](float value) {
							std::for_each(divertors.begin(), divertors.end(), [value](Drawing::Divertor *divertor) { divertor->length = value; });
							currentLength = value;
						}, currentLength, 1, 0);
						inspector->addFloatField("Y Position:", divertor->verticalPosition, 1, 0);
						inspector->addComponentField<Material>("Material:", [this](const Material &material) {
							std::for_each(divertors.begin(), divertors.end(), [&material](Drawing::Divertor *divertor) { divertor->setMaterial(material); });
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

void DivertorSetGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
	for (Drawing::Divertor *divertor : divertors) {
		if (calculateDivertorBounds(divertor).contains(event->pos())) {
			if (inspector) {
				inspector->expand();
			}
			break;
		}
	}

	QGraphicsItem::mouseDoubleClickEvent(event);
}

void DivertorSetGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    for (Drawing::Divertor *divertor : divertors) {
        if (calculateDivertorBounds(divertor).contains(event->pos())) {
            QMenu *menu = new QMenu();

            menu->addAction("Remove", [this, divertor]() {
                if (removeFunction) {
                    removeFunction(*divertor);
                }
            }, Qt::Key_Delete);

            menu->popup(event->screenPos());

            QWidget::connect(menu, &QMenu::triggered, [menu](QAction *) { menu->deleteLater(); });
            break;
        }
    }
    QGraphicsItem::contextMenuEvent(event);
}

QPainterPath DivertorSetGraphicsItem::calculateDivertorBounds(const Drawing::Divertor *divertor) const {
	QPainterPath divertorBounds;

	double root2 = std::sqrt(2);

	switch (divertor->side) {
		case Drawing::LEFT:
			divertorBounds.moveTo(matBounds.left(), matBounds.top() +
								  (divertor->verticalPosition - divertor->width / (2 * root2)) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.left() + (divertor->length / root2) / matWidth * matBounds.width(),
								  matBounds.top() + (divertor->verticalPosition - divertor->width / (2 * root2) +
													 divertor->length / root2) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.left() + (divertor->length / root2) / matWidth * matBounds.width(),
								  matBounds.top() + (divertor->verticalPosition + divertor->width / (2 * root2) +
													 divertor->length / root2) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.left(), matBounds.top() +
								  (divertor->verticalPosition + divertor->width / (2 * root2)) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.left(), matBounds.top() +
								  (divertor->verticalPosition - divertor->width / (2 * root2)) / matLength * matBounds.height());
			break;
		case Drawing::RIGHT:
			divertorBounds.moveTo(matBounds.right(), matBounds.top() +
								  (divertor->verticalPosition - divertor->width / (2 * root2)) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.right() - (divertor->length / root2) / matWidth * matBounds.width(),
								  matBounds.top() + (divertor->verticalPosition - divertor->width / (2 * root2) +
													 divertor->length / root2) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.right() - (divertor->length / root2) / matWidth * matBounds.width(),
								  matBounds.top() + (divertor->verticalPosition + divertor->width / (2 * root2) +
													 divertor->length / root2) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.right(), matBounds.top() +
								  (divertor->verticalPosition + divertor->width / (2 * root2)) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.right(), matBounds.top() +
								  (divertor->verticalPosition - divertor->width / (2 * root2)) / matLength * matBounds.height());
			break;
	}

	return divertorBounds;
}