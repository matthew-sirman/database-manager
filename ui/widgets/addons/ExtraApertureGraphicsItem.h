#pragma once

//
// Created by matthew on 07/08/2020.
//

#ifndef DATABASE_MANAGER_EXTRAAPERTUREGRAPHICSITEM_H
#define DATABASE_MANAGER_EXTRAAPERTUREGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QDoubleSpinBox>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

#include "../Inspector.h"
#include "../DimensionLine.h"
#include "../../../include/database/Drawing.h"

class ExtraApertureGraphicsItem : public QGraphicsItem {
public:
	explicit ExtraApertureGraphicsItem(const QRectF& bounds, Drawing::ExtraAperture& extraAperture,
		Inspector* inspector = nullptr);

	[[nodiscard]] QRectF boundingRect() const override;

	void setBounds(const QRectF& bounds);

	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	[[nodiscard]] bool contains(const QPointF& point) const override;

	void setRemoveFunction(const std::function<void()>& remove);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

	void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
	Drawing::ExtraAperture& extraAperture;
	Inspector* inspector = nullptr;

	unsigned inspectorAcquireID = -1;

	QRectF boundingBox;

	std::function<void()> removeFunction = nullptr;

	ComboboxComponentDataSource<Aperture> apertureSource;
};

#endif //DATABASE_MANAGER_EXTRAAPERTUREGRAPHICSITEM_H