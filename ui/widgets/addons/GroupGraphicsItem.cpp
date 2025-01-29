#include "GroupGraphicsItem.h"

template<>
QPainterPath GroupGraphicsItem<Drawing::CentreHole, Aperture>::calculateBounds(const Drawing::CentreHole &hole) const {
	Aperture ap = hole.aperture();
	bool longitudinal, rounded;
	if (ap.getShape().shape == "ST" || ap.getShape().shape == "SQ") {
		rounded = false;
		longitudinal = false;
	}
	else if (ap.getShape().shape == "SL") {
		rounded = false;
		longitudinal = true;
	}
	else if (ap.getShape().shape == "DIA" || ap.getShape().shape == "RT") {
		rounded = true;
		longitudinal = false;
	}
	else if (ap.getShape().shape == "RL") {
		rounded = true;
		longitudinal = true;
	}
	QPointF holePoint;
	QPointF start(matBounds.left(), matBounds.top());
	QPointF h(hole.pos.x, hole.pos.y); 
	QPointF apPoint = longitudinal ? QPointF(ap.width, ap.length) : QPointF(ap.length, ap.width);
	float ratio = longitudinal ? matBounds.width() / matWidth : matBounds.height() / matLength;
	QPointF boxTL = start + ratio * (h - 0.5 * apPoint);
	QSizeF size((ap.width / matWidth) * matBounds.width(),
		(ap.length / matLength) * matBounds.height());
	if (!longitudinal) {
		size = QSizeF(size.height(), size.width());
	}
	QRectF holeBounds(boxTL, size);
	QPainterPath path;
	path.addRect(holeBounds);
	return path;
};

template<>
void GroupGraphicsItem<Drawing::CentreHole, Aperture>::populateInspector(Drawing::CentreHole &hole){
	inspector->addFloatField("X Position:", hole.pos.x, 1, 0);
	inspector->addFloatField("Y Position:", hole.pos.y, 1, 0);

	ComboboxComponentDataSource<Aperture>* apertureSource = std::get<ComboboxComponentDataSource<Aperture>*>(componentSources);
	DrawingComponentManager<Aperture>::addCallback([apertureSource]() {
		apertureSource->updateSource();
		apertureSource->sort(Aperture::apertureComparator);
	});

	apertureSource->updateSource();
	apertureSource->sort(Aperture::apertureComparator);

    inspector->addComponentField<Aperture>("Aperture:", [this](const Aperture& aperture) {
        for (Drawing::CentreHole &hole : this->items) {
			hole.setAperture(aperture);
        }
    }, *apertureSource, hole.aperture().handle());
}

template<>
void GroupGraphicsItem<Drawing::CentreHole, Aperture>::sync(const Drawing::CentreHole &a, Drawing::CentreHole &b) {
	b.apertureID = a.apertureID;
}

template<>
QPainterPath GroupGraphicsItem<Drawing::Deflector, Material>::calculateBounds(const Drawing::Deflector &deflector) const {
	QPainterPath deflectorBounds;

	double root2 = std::sqrt(2);

	deflectorBounds.moveTo(matBounds.left() + (deflector.pos.x / matWidth) * matBounds.width(), matBounds.top() +
						   (deflector.pos.y - deflector.size / root2) / matLength * matBounds.height());
	deflectorBounds.lineTo(matBounds.left() + (deflector.pos.x + deflector.size / root2) / matWidth * matBounds.width(), matBounds.top() +
						   (deflector.pos.y / matLength) * matBounds.height());
	deflectorBounds.lineTo(matBounds.left() + (deflector.pos.x / matWidth) * matBounds.width(), matBounds.top() +
						   (deflector.pos.y + deflector.size / root2) / matLength * matBounds.height());
	deflectorBounds.lineTo(matBounds.left() + (deflector.pos.x - deflector.size / root2) / matWidth * matBounds.width(), matBounds.top() +
						   (deflector.pos.y / matLength) * matBounds.height());
	deflectorBounds.lineTo(matBounds.left() + (deflector.pos.x / matWidth) * matBounds.width(), matBounds.top() +
						   (deflector.pos.y - deflector.size / root2) / matLength * matBounds.height());

	return deflectorBounds;
}

template<>
void GroupGraphicsItem<Drawing::Deflector, Material>::populateInspector(Drawing::Deflector &deflector) {
	inspector->addFloatField("Size:", [this](float value) {
		std::for_each(this->items.begin(), this->items.end(), [value](Drawing::Deflector &deflector) { deflector.size = value; });
	}, deflector.size, 1, 0);
	inspector->addFloatField("X Position:", deflector.pos.x, 1, 0);
	inspector->addFloatField("Y Position:", deflector.pos.y, 1, 0);

	ComboboxComponentDataSource<Material>* materialSource = std::get<ComboboxComponentDataSource<Material>*>(componentSources);
	
	DrawingComponentManager<Aperture>::addCallback([materialSource]() {
		materialSource->updateSource();
	});

	materialSource->updateSource();

	inspector->addComponentField<Material>("Material:", [this](const Material &material) {
		std::for_each(this->items.begin(), this->items.end(), [&material](Drawing::Deflector &deflector) { deflector.setMaterial(material); });
	}, *materialSource, deflector.materialHandle == 0 ? -1 : deflector.materialHandle);
};

template<>
void GroupGraphicsItem<Drawing::Deflector, Material>::sync(const Drawing::Deflector &a, Drawing::Deflector &b) {
	b.materialHandle = a.materialHandle;
	b.size = a.size;
}

template<>
QPainterPath GroupGraphicsItem<Drawing::Divertor, Material>::calculateBounds(const Drawing::Divertor &divertor) const {
	QPainterPath divertorBounds;

	double root2 = std::sqrt(2);

	switch (divertor.side) {
		case Drawing::LEFT:
			divertorBounds.moveTo(matBounds.left(), matBounds.top() +
								  (divertor.verticalPosition - divertor.width / (2 * root2)) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.left() + (divertor.length / root2) / matWidth * matBounds.width(),
								  matBounds.top() + (divertor.verticalPosition - divertor.width / (2 * root2) +
													 divertor.length / root2) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.left() + (divertor.length / root2) / matWidth * matBounds.width(),
								  matBounds.top() + (divertor.verticalPosition + divertor.width / (2 * root2) +
													 divertor.length / root2) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.left(), matBounds.top() +
								  (divertor.verticalPosition + divertor.width / (2 * root2)) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.left(), matBounds.top() +
								  (divertor.verticalPosition - divertor.width / (2 * root2)) / matLength * matBounds.height());
			break;
		case Drawing::RIGHT:
			divertorBounds.moveTo(matBounds.right(), matBounds.top() +
								  (divertor.verticalPosition - divertor.width / (2 * root2)) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.right() - (divertor.length / root2) / matWidth * matBounds.width(),
								  matBounds.top() + (divertor.verticalPosition - divertor.width / (2 * root2) +
													 divertor.length / root2) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.right() - (divertor.length / root2) / matWidth * matBounds.width(),
								  matBounds.top() + (divertor.verticalPosition + divertor.width / (2 * root2) +
													 divertor.length / root2) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.right(), matBounds.top() +
								  (divertor.verticalPosition + divertor.width / (2 * root2)) / matLength * matBounds.height());
			divertorBounds.lineTo(matBounds.right(), matBounds.top() +
								  (divertor.verticalPosition - divertor.width / (2 * root2)) / matLength * matBounds.height());
			break;
	}

	return divertorBounds;
};

template<>
void GroupGraphicsItem<Drawing::Divertor, Material>::populateInspector(Drawing::Divertor &divertor) {
	inspector->addFloatField("Width:", [this](float value) {
		std::for_each(items.begin(), items.end(), [value](Drawing::Divertor &divertor) { divertor.width = value; });
	}, currentWidth, 1, 0);
	inspector->addFloatField("Length:", [this](float value) {
		std::for_each(items.begin(), items.end(), [value](Drawing::Divertor &divertor) { divertor.length = value; });
	}, currentLength, 1, 0);
	inspector->addFloatField("Y Position:", divertor.verticalPosition, 1, 0);
	ComboboxComponentDataSource<Material>* materialSource = std::get<ComboboxComponentDataSource<Material>*>(componentSources);

	DrawingComponentManager<Aperture>::addCallback([materialSource]() {
		materialSource->updateSource();
	});

	materialSource->updateSource();

	inspector->addComponentField<Material>("Material:", [this](const Material &material) {
		std::for_each(items.begin(), items.end(), [&material](Drawing::Divertor &divertor) { divertor.setMaterial(material); });
	}, *materialSource, divertor.materialHandle == 0 ? -1 : divertor.materialHandle);
}

template<>
void GroupGraphicsItem<Drawing::Divertor, Material>::sync(const Drawing::Divertor &a, Drawing::Divertor &b) {
	b.width = a.width;
	b.length = a.length;
}
