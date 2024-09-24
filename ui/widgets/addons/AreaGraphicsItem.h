
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
	/// <summary>
	/// Constructs a new item ready to be displayed.
	/// </summary>
	/// <param name="bounds">The are this item occupies (in screen coords).</param>
	/// <param name="item">A reference to the item this object displays.</param>
	/// <param name="inspector">The inspector to display and update the item.</param>
	explicit AreaGraphicsItem(const QRectF& bounds, T& item, Inspector* inspector = nullptr);

	/// <summary>
	/// Getter for the bounding rect.
	/// </summary>
	/// <returns>The bounding rect.</returns>
	[[nodiscard]] QRectF boundingRect() const override;

	/// <summary>
	/// Updates the bounds this object is drawn to.
	/// </summary>
	/// <param name="bounds"></param>
	void setBounds(const QRectF& bounds);

	/// <summary>
	/// Paints this object to a canvas through the provided painter.
	/// </summary>
	/// <param name="painter">The painter to drawn to.</param>
	/// <param name="options">Unused.</param>
	/// <param name="widget">Unused.</param>
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* options, QWidget* widget) override;

	/// <summary>
	/// Checks whether the provided point is within the bounding rect.
	/// </summary>
	/// <param name="point">Point to check.</param>
	/// <returns>True if the point is in the bounds, false otherwise.</returns>
	[[nodiscard]] bool contains(const QPointF& point) const override;

	/// <summary>
	/// Sets a function to be called upon the removal of this object.
	/// </summary>
	/// <param name="remove"></param>
	void setRemoveFunction(const std::function<void()>& remove);

	/// <summary>
	/// Getter for the item this object is based off.
	/// </summary>
	/// <typeparam name="T">The type this object is based upon.</typeparam>
	/// <returns>A refernce to the item this object is based upon.</returns>
	const T& get() const;

protected:
	/// <summary>
	/// Overriden mouse press event for populating the inspector. Propogates event.
	/// </summary>
	/// <param name="event">The triggering event.</param>
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

	/// <summary>
	/// Overriden mouse double click event for expanding the inspector. Propogates event.
	/// </summary>
	/// <param name="event">The triggering event.</param>
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

	/// <summary>
	/// Override context menu event display custom context menu. Propogates event.
	/// </summary>
	/// <param name="event">The tiggering event.</param>
	void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
private:
	/// <summary>
	/// Populates the inspector with the relevant information. This must be specifically implemented
	/// for all templates of this function, as seen in \ref AreaGraphicsItem.cpp.
	/// </summary>
	/// <param name="item">The item to be displayed.</param>
	/// <param name="inspector">The inspector to populate.</param>
	/// <param name="...sources">All the component sources for use in the inspector.</param>
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
