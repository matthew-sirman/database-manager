#include "AreaGraphicsItem.h"

const QBrush AreaGraphicsItem<Drawing::BlankSpace>::brush(QColor(200, 200, 200, 127));

void  AreaGraphicsItem<Drawing::BlankSpace>::populateInspector(Drawing::BlankSpace& b, Inspector* inspector) {
	inspector->addFloatField("Width:", b.width, 1, 0);
	inspector->addFloatField("Length:", b.length, 1, 0);
	inspector->addFloatField("X Position:", b.pos.x, 1, 0);
	inspector->addFloatField("Y Position:", b.pos.y, 1, 0);
};

const QBrush AreaGraphicsItem<Drawing::ImpactPad, Material, Aperture>::brush(QColor(255, 255, 0, 127));

void AreaGraphicsItem<Drawing::ImpactPad, Material, Aperture>::populateInspector(Drawing::ImpactPad& p, Inspector* inspector,
	ComboboxComponentDataSource<Material>* materialSource, ComboboxComponentDataSource<Aperture>* apertureSource) {

	Material &currentMaterial = p.material();
	Aperture &currentAperture = p.aperture();

	inspector->addFloatField("Width:", p.width, 1, 0);
	inspector->addFloatField("Length:", p.length, 1, 0);
	inspector->addFloatField("X Position:", p.pos.x, 1, 0);
	inspector->addFloatField("Y Position:", p.pos.y, 1, 0);

	DrawingComponentManager<Material>::addCallback([materialSource]() {
		materialSource->updateSource();
	});

	materialSource->updateSource();

	inspector->addComponentField<Material>("Material:", [&p](const Material &material) mutable {
		p.setMaterial(material);
	}, *materialSource, currentMaterial.handle() == 0 ? -1 : currentMaterial.handle());

	DrawingComponentManager<Aperture>::addCallback([apertureSource]() {
		apertureSource->updateSource();
		apertureSource->sort(Aperture::apertureComparator);
	});

	apertureSource->updateSource();
	apertureSource->sort(Aperture::apertureComparator);

	inspector->addComponentField<Aperture>("Aperture:", [&p](const Aperture &aperture) mutable {
		p.setAperture(aperture);
	}, *apertureSource, currentAperture.handle() == 0 ? -1 : currentAperture.handle());
};

const QBrush AreaGraphicsItem<Drawing::ExtraAperture, Aperture>::brush(QColor(141, 221, 247, 127));

void AreaGraphicsItem<Drawing::ExtraAperture, Aperture>::populateInspector(Drawing::ExtraAperture& e,
	Inspector* inspector, ComboboxComponentDataSource<Aperture>* apertureSource) {
			inspector->addFloatField("Width:", e.width, 1, 0);
			inspector->addFloatField("Length:", e.length, 1, 0);
			inspector->addFloatField("X Position:", e.pos.x, 1, 0);
			inspector->addFloatField("Y Position:", e.pos.y, 1, 0);

	        DrawingComponentManager<Aperture>::addCallback([apertureSource]() {
		        apertureSource->updateSource();
		        apertureSource->sort(Aperture::apertureComparator);
	        });

        	apertureSource->updateSource();
	        apertureSource->sort(Aperture::apertureComparator);
			inspector->addComponentField<Aperture>("Aperture:",
                [&e](const Aperture& aperture) mutable {
					e.setAperture(aperture);
			    },
                *apertureSource,
                DrawingComponentManager<Aperture>::validComponentID(e.aperture().componentID())
                    ? e.aperture().handle()
                    : -1);
};

const QBrush AreaGraphicsItem<Drawing::DamBar, Material>::brush(QColor(255, 50, 50, 127));

void AreaGraphicsItem<Drawing::DamBar, Material>::populateInspector(Drawing::DamBar& d,
	Inspector* inspector, ComboboxComponentDataSource<Material>* materialSource) {
	inspector->addFloatField("Width:", d.width, 1, 0);
	inspector->addFloatField("Length:", d.length, 1, 0);
	inspector->addFloatField("X Position:", d.pos.x, 1, 0);
	inspector->addFloatField("Y Position:", d.pos.y, 1, 0);

	Material currentMaterial = d.material();
	DrawingComponentManager<Material>::addCallback([materialSource]() {
		materialSource->updateSource();
	});

	materialSource->updateSource();

	inspector->addComponentField<Material>("Material:", [&d](const Material &material) mutable {
		d.setMaterial(material);
	}, *materialSource, currentMaterial.handle() == 0 ? -1 : currentMaterial.handle());
};
