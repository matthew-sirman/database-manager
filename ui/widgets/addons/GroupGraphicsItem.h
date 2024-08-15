
#pragma once

//
// Created by alistair on 09/08/2024.
//

#ifndef DATABASE_MANAGER_GROUPGRAPHICSITEM_H
#define DATABASE_MANAGER_GROUPGRAPHICSITEM_H
#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <vector>
#include <unordered_map>

#include "../Inspector.h"
#include "../DimensionLine.h"
#include "../../../include/database/Drawing.h"

template<typename T, DrawingComponentConcept ...Ds>
class GroupGraphicsItem : public QGraphicsItem {
public:
	explicit GroupGraphicsItem(std::vector<T> &items, const QRectF &matBounds, float matWidth, float matLength, Inspector *inspector = nullptr);

	[[nodiscard]] QRectF boundingRect() const override;

	void setBounds(const QRectF &bounds, float matWidth, float matLength);

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

	void addItem(T &&item);

	void clearItems();

	bool contains(const QPointF &point) const override;

	bool empty() const;

    void setRemoveFunction(const std::function<void(const T&)> &remove);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
	QPainterPath calculateBounds(const T& item) const;

	void populateInspector(T& item);// , Inspector* inspector, std::vector<T*>&, ComboboxComponentDataSource<Ds>*...);

	std::vector<T> &items;
	std::vector<unsigned> itemAcquireIDs;
	Inspector *inspector = nullptr;

	float currentWidth = 30, currentLength = 300;

	QRectF matBounds;
	float matWidth, matLength;

    std::function<void(const T &)> removeFunction = nullptr;
	
	std::tuple<ComboboxComponentDataSource<Ds>*...> componentSources;

	static void sync(const T&, T&);
};

template<typename T, DrawingComponentConcept ...Ds>
GroupGraphicsItem<T, Ds...>::GroupGraphicsItem(std::vector<T> &items, const QRectF& matBounds, float matWidth, float matLength, Inspector* inspector) 
 : items(items), matBounds(matBounds), matWidth(matWidth), matLength(matLength), inspector(inspector) {
	componentSources = std::make_tuple((new ComboboxComponentDataSource<Ds>)...);
};

template<typename T, DrawingComponentConcept ...Ds>
QRectF GroupGraphicsItem<T, Ds...>::boundingRect() const {
	return matBounds;
}

template<typename T, DrawingComponentConcept ...Ds>
void GroupGraphicsItem<T, Ds...>::setBounds(const QRectF& rect, float matWidth, float matHeight) {
	matBounds = rect;
	this->matWidth = matWidth;
	this->matLength = matLength;
}

template<typename T, DrawingComponentConcept ...Ds>
void GroupGraphicsItem<T, Ds...>::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	painter->save();

	for (T &item: items) {
		painter->setPen(Qt::red);
		painter->setRenderHint(QPainter::Antialiasing);
		painter->drawPath(calculateBounds(item));
	}

	painter->restore();
}

template<typename T, DrawingComponentConcept ...Ds>
void GroupGraphicsItem<T, Ds...>::addItem(T &&item) {
	if (!items.empty()) {
		GroupGraphicsItem<T, Ds...>::sync(items.front(), item);
	}
	items.push_back(std::move(item));
    itemAcquireIDs.push_back(-1);
	//itemAcquireIDs.emplace(items.back(), - 1);
}

template<typename T, DrawingComponentConcept ...Ds>
void GroupGraphicsItem<T, Ds...>::clearItems() {
	items.clear();
	itemAcquireIDs.clear();
}

template<typename T, DrawingComponentConcept ...Ds>
bool GroupGraphicsItem<T, Ds...>::contains(const QPointF& point) const {
	for (T &item: items) {
		if (calculateBounds(item).contains(point)) {
			return true;
		}
	}
	return false;
}

template<typename T, DrawingComponentConcept ...Ds>
void GroupGraphicsItem<T, Ds...>::setRemoveFunction(const std::function<void(const T&)> &removeFunction) {
	this->removeFunction = removeFunction;
};

template<typename T, DrawingComponentConcept ...Ds>
void GroupGraphicsItem<T, Ds...>::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		for (unsigned i = 0; i < items.size(); i ++) {
			T& item = items[i];
			if (calculateBounds(item).contains(event->pos())) {
				if (inspector) {
					if (inspector->currentOwner() != itemAcquireIDs[i]) {
                        itemAcquireIDs[i] = inspector->acquire();
						GroupGraphicsItem<T, Ds...>::populateInspector(item);
					}
				}
				break;
			}
		}
	}
	QGraphicsItem::mousePressEvent(event);
};

template<typename T, DrawingComponentConcept ...Ds>
void GroupGraphicsItem<T, Ds...>::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
	for (T &item : items) {
		if (calculateBounds(item).contains(event->pos())) {
			if (inspector) {
				inspector->expand();
			}
			break;
		}
	}
	QGraphicsItem::mouseDoubleClickEvent(event);
}

template<typename T, DrawingComponentConcept ...Ds>
void GroupGraphicsItem<T, Ds...>::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	for (unsigned i = 0; i < items.size(); i++) {
		T &item = items[i];
        if (calculateBounds(item).contains(event->pos())) {
            QMenu *menu = new QMenu();

            menu->addAction("Remove", [this, item]() {
				auto it = std::find(items.begin(), items.end(), item);
				int index = it - items.begin();
                if (removeFunction) {
					if (itemAcquireIDs[index] == inspector->currentOwner()) {
						inspector->clear();
					}
					itemAcquireIDs.erase(std::next(itemAcquireIDs.begin(), index));
					std::erase(items, item);
                }
            }, Qt::Key_Delete);

            menu->popup(event->screenPos());

            QWidget::connect(menu, &QMenu::triggered, [menu](QAction *) { menu->deleteLater(); });
            break;
        }
    }
    QGraphicsItem::contextMenuEvent(event);
}

template<typename T, DrawingComponentConcept ...Ds>
bool GroupGraphicsItem<T, Ds...>::empty() const {
	return items.empty();
}
#endif
