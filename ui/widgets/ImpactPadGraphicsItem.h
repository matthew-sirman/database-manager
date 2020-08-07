//
// Created by matthew on 07/08/2020.
//

#ifndef DATABASE_MANAGER_IMPACTPADGRAPHICSITEM_H
#define DATABASE_MANAGER_IMPACTPADGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QDoubleSpinBox>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "DimensionLine.h"
#include "../../include/database/Drawing.h"

class ImpactPadGraphicsItem : public QGraphicsItem {
public:
	explicit ImpactPadGraphicsItem(QGraphicsScene *scene, const QRectF &bounds, Drawing::ImpactPad &impactPad);

	[[nodiscard]] QRectF boundingRect() const override;

	void setBounds(const QRectF &bounds);

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
	Drawing::ImpactPad &pad;

	QDoubleSpinBox *widthInputBox = nullptr, *lengthInputBox = nullptr;
	QGraphicsProxyWidget *widthDimensionInput = nullptr, *lengthDimensionInput = nullptr;
	WidgetDimensionLine *widthDimensionLine = nullptr, *lengthDimensionLine = nullptr;

	const double dimensionOffset = 0.02;

	QRectF boundingBox;

	bool editModeActive = false;

};

#endif //DATABASE_MANAGER_IMPACTPADGRAPHICSITEM_H