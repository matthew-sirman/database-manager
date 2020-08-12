//
// Created by matthew on 07/08/2020.
//

#ifndef DATABASE_MANAGER_IMPACTPADGRAPHICSITEM_H
#define DATABASE_MANAGER_IMPACTPADGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QDoubleSpinBox>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

#include "../Inspector.h"
#include "../DimensionLine.h"
#include "../../../include/database/Drawing.h"

class ImpactPadGraphicsItem : public QGraphicsItem {
public:
	explicit ImpactPadGraphicsItem(const QRectF &bounds, Drawing::ImpactPad &impactPad, 
		Inspector *inspector = nullptr);

	[[nodiscard]] QRectF boundingRect() const override;

	void setBounds(const QRectF &bounds);

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

	[[nodiscard]] bool contains(const QPointF &point) const override;

	void setRemoveFunction(const std::function<void()> &remove);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
	Drawing::ImpactPad &pad;
	Inspector *inspector = nullptr;

	unsigned inspectorAcquireID = -1;

	QRectF boundingBox;

	ComboboxComponentDataSource<Material> materialSource;
	ComboboxComponentDataSource<Aperture> apertureSource;

	std::function<void()> removeFunction = nullptr;
};

#endif //DATABASE_MANAGER_IMPACTPADGRAPHICSITEM_H