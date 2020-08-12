//
// Created by matthew on 10/08/2020.
//

#ifndef DATABASE_MANAGER_DIVERTORSETGRAPHICSITEM_H
#define DATABASE_MANAGER_DIVERTORSETGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>

#include <vector>

#include "../Inspector.h"
#include "../DimensionLine.h"
#include "../../../include/database/Drawing.h"

class DivertorSetGraphicsItem : public QGraphicsItem {
public:
	explicit DivertorSetGraphicsItem(const QRectF &matBounds, float matWidth, float matLength, Inspector *inspector = nullptr);

	[[nodiscard]] QRectF boundingRect() const override;

	void setBounds(const QRectF &bounds, float width, float length);

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

	void addDivertor(Drawing::Divertor &divertor);

	void clearDivertors();

	float divertorWidth() const;

	float divertorLength() const;

	Material &divertorMaterial() const;

	bool contains(const QPointF &point) const override;

    void setRemoveFunction(const std::function<void(const Drawing::Divertor &)> &remove);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
	QPainterPath calculateDivertorBounds(const Drawing::Divertor *divertor) const;

	std::vector<Drawing::Divertor *> divertors;
	std::vector<unsigned> divertorAcquireIDs;
	Inspector *inspector = nullptr;

	float currentWidth = 30, currentLength = 300;
	unsigned currentMaterialHandle = 0;

	QRectF matBounds;
	float matWidth, matLength;

	ComboboxComponentDataSource<Material> materialSource;

    std::function<void(const Drawing::Divertor &)> removeFunction = nullptr;
};

#endif //DATABASE_MANAGER_DIVERTORSETGRAPHICSITEM_H