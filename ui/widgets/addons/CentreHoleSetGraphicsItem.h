//
// Created by matthew on 10/08/2020.
//

#ifndef DATABASE_MANAGER_CENTREHOLESETGRAPHICSITEM_H
#define DATABASE_MANAGER_CENTREHOLESETGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>

#include <vector>

#include "../Inspector.h"
#include "../DimensionLine.h"
#include "../../../include/database/Drawing.h"

class CentreHoleSetGraphicsItem : public QGraphicsItem {
public:
	explicit CentreHoleSetGraphicsItem(const QRectF &matBounds, float matWidth, float matLength, Inspector *inspector = nullptr);

	[[nodiscard]] QRectF boundingRect() const override;

	void setBounds(const QRectF &bounds, float width, float length);

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

	void addCentreHole(Drawing::CentreHole &hole);

	void clearCentreHoles();

	
	Aperture currentAperture() const;

	bool contains(const QPointF &point) const override;

	void setRemoveFunction(const std::function<void(const Drawing::CentreHole &)> &remove);

	bool empty() const;

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
	std::vector<Drawing::CentreHole *> holes;
	std::vector<unsigned> holeAcquireIDs;
	Inspector *inspector = nullptr;

	unsigned apertureID;

	QRectF matBounds;
	float matWidth, matLength;

    std::function<void(const Drawing::CentreHole &)> removeFunction = nullptr;

	ComboboxComponentDataSource<Aperture> apertureSource;
};

#endif //DATABASE_MANAGER_CENTREHOLESETGRAPHICSITEM_H