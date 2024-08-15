
#pragma once

//
// Created by alistair on 09/08/2024.
//

#ifndef DATABASE_MANAGER_AREAGRAPHICSITEM_H
#define DATABASE_MANAGER_AREAGRAPHICSITEM_H

#include <concepts>
#include <type_traits>
#include <QGraphicsItem>
#include <QPainter>
#include <QDoubleSpinBox>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

#include "../Inspector.h"
#include "../DimensionLine.h"
#include "../../../include/database/Drawing.h"

/// <summary>
/// AreaGraphicsItem inherits QGraphicsItem \n
/// AreaGraphicsItem displays and populates the Inspector for any item that would be displayed as an area on the drawing.
/// Addons must implement both a static brush attribute and static populateInspector function in AreaGraphicsItem.cpp.
/// If either of these don't exist, a linker error will occour, naming what is missing from which AreaGraphicsIte<T, Ds...>.
/// </summary>
/// <typeparam name="T">The addon that is to be displayed.</typeparam>
/// <typeparam name="...Ds">Any drawing components the addon relies upon, such as an aperture or material.</typeparam>
template<typename T, DrawingComponentConcept ...Ds>
class AreaGraphicsItem : public QGraphicsItem {
public:
	explicit AreaGraphicsItem(const QRectF& bounds, T& item, Inspector* inspector = nullptr);

	[[nodiscard]] QRectF boundingRect() const override;

	void setBounds(const QRectF& bounds);

	void paint(QPainter* painter, const QStyleOptionGraphicsItem* options, QWidget* widget) override;

	[[nodiscard]] bool contains(const QPointF& point) const override;

	void setRemoveFunction(const std::function<void()>& remove);

	const T& get() const;

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

	void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
private:
	static void populateInspector(T& item, Inspector* inspector, ComboboxComponentDataSource<Ds>*... sources);
	T& item;
	Inspector* inspector = nullptr;

	unsigned inspectorAcquireID = -1;

	QRectF boundingBox;

	std::function<void()> removeFunction = nullptr;

	std::tuple<ComboboxComponentDataSource<Ds>*...> componentSources;

	static const QBrush brush;
};

template<typename T, DrawingComponentConcept ...Ds>
AreaGraphicsItem<T, Ds...>::AreaGraphicsItem(const QRectF& bounds, T& item, Inspector* inspector)
	:item(item), inspector(inspector), boundingBox(bounds) {
	componentSources = std::make_tuple((new ComboboxComponentDataSource<Ds>())...);
}

template<typename T, DrawingComponentConcept ...Ds>
QRectF AreaGraphicsItem<T, Ds...>::boundingRect() const {
	return boundingBox;
}

template<typename T, DrawingComponentConcept ...Ds>
void AreaGraphicsItem<T, Ds...>::setBounds(const QRectF& bounds) {
	boundingBox = bounds;
}

template<typename T, DrawingComponentConcept ...Ds>
void AreaGraphicsItem<T, Ds...>::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	painter->save();

	painter->setBrush(AreaGraphicsItem<T, Ds...>::brush);
	painter->drawRect(boundingBox);

	painter->restore();
}

template<typename T, DrawingComponentConcept ...Ds>
bool AreaGraphicsItem<T, Ds...>::contains(const QPointF& point) const {
	return boundingBox.contains(point);
}

template<typename T, DrawingComponentConcept ...Ds>
void AreaGraphicsItem<T, Ds...>::setRemoveFunction(const std::function<void()>& remove) {
	removeFunction = remove;
}

template<typename T, DrawingComponentConcept ...Ds>
const T& AreaGraphicsItem<T, Ds...>::get() const {
	return item;
}

template<typename T, DrawingComponentConcept ...Ds>
void AreaGraphicsItem<T, Ds...>::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (inspector) {
		if (inspector->currentOwner() != inspectorAcquireID) {
			inspectorAcquireID = inspector->acquire();
			std::apply([this](ComboboxComponentDataSource<Ds>*...comboboxSources) {
				AreaGraphicsItem<T, Ds...>::populateInspector(item, inspector, comboboxSources...);
				}, componentSources);
		}
	}
	QGraphicsItem::mousePressEvent(event);
}

template<typename T, DrawingComponentConcept ...Ds>
void AreaGraphicsItem<T, Ds...>::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
	if (boundingBox.contains(event->pos())) {
		if (inspector) {
			inspector->expand();
		}
	}

	QGraphicsItem::mouseDoubleClickEvent(event);
}

template<typename T, DrawingComponentConcept ...Ds>
void AreaGraphicsItem<T, Ds...>::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	if (boundingBox.contains(event->pos())) {
		QMenu* menu = new QMenu();

		menu->addAction("Remove", [this]() {
			if (inspectorAcquireID == inspector->currentOwner()) {
				inspector->clear();
			}
			removeFunction();
			}, Qt::Key_Delete);
		menu->popup(event->screenPos());

		QWidget::connect(menu, &QMenu::triggered, [menu](QAction*) { menu->deleteLater(); });
	}

	QGraphicsItem::contextMenuEvent(event);
}
#endif // DATABASE_MANAGER_AREAGRAPHICSITEM_H
