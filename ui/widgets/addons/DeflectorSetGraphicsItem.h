//
// Created by matthew on 10/08/2020.
//

#ifndef DATABASE_MANAGER_DEFLECTORSETGRAPHICSITEM_H
#define DATABASE_MANAGER_DEFLECTORSETGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>

#include <vector>

#include "../Inspector.h"
#include "../DimensionLine.h"
#include "../../../include/database/Drawing.h"

class DeflectorSetGraphicsItem : public QGraphicsItem {
public:
	explicit DeflectorSetGraphicsItem(const QRectF &matBounds, float matWidth, float matLength, Inspector *inspector = nullptr);

	[[nodiscard]] QRectF boundingRect() const override;

	void setBounds(const QRectF &bounds, float width, float length);

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

	void addDeflector(Drawing::Deflector &deflector);

	void clearDeflectors();

	float deflectorSize() const;

	Material &deflectorMaterial() const;

	bool contains(const QPointF &point) const override;

	void setRemoveFunction(const std::function<void(const Drawing::Deflector &)> &remove);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
	QPainterPath calculateDeflectorBounds(const Drawing::Deflector *deflector) const;

	std::vector<Drawing::Deflector *> deflectors;
	std::vector<unsigned> deflectorAcquireIDs;
	Inspector *inspector = nullptr;

	float currentSize = 50;
	unsigned currentMaterialHandle = 0;

	QRectF matBounds;
	float matWidth, matLength;

	ComboboxComponentDataSource<Material> materialSource;

	std::function<void(const Drawing::Deflector &)> removeFunction = nullptr;
};

#endif //DATABASE_MANAGER_DEFLECTORSETGRAPHICSITEM_H