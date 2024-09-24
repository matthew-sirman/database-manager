
#pragma once

//
// Created by alistair on 09/08/2024.
//

#ifndef DATABASE_MANAGER_GROUPGRAPHICSITEM_H
#define DATABASE_MANAGER_GROUPGRAPHICSITEM_H
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>
#include <unordered_map>
#include <vector>

#include "../../../include/database/Drawing.h"
#include "../DimensionLine.h"
#include "../Inspector.h"

/// <summary>
/// GroupGraphicsItem inherits QGraphicsItem
/// This class is designed to link to a drawing and display any graphics items
/// which are considered groups, such as centre holes. Synced attributes can be
/// set up so that anything that should be the same for all can be set.
/// </summary>
/// <typeparam name="T">The type that is to be shown.</typeparam>
/// <typeparam name="...Ds">Any DrawingComponents that the underlying item
/// replies upon.</typeparam>
template <typename T, DrawingComponentConcept... Ds>
class GroupGraphicsItem : public QGraphicsItem {
   public:
    /// <summary>
    /// Constructs a new GroupGraphicsItem, binding it to the drawings items by
    /// reference.
    /// </summary>
    /// <param name="items">A reference to the drawings items.</param>
    /// <param name="matBounds">The bounds of the mat on the screen.</param>
    /// <param name="matWidth">The physical width of the mat.</param>
    /// <param name="matLength">The physical length of the mat.</param>
    /// <param name="inspector">The inspector each item should be link
    /// in.</param>
    explicit GroupGraphicsItem(std::vector<T> &items, const QRectF &matBounds,
                               float matWidth, float matLength,
                               Inspector *inspector = nullptr);

    /// <summary>
    /// Getter for the bounding rect of the mat.
    /// </summary>
    /// <returns>The bounding rect.</returns>
    [[nodiscard]] QRectF boundingRect() const override;

    /// <summary>
    /// Sets the bounding rect and the mats width and length.
    /// </summary>
    /// <param name="bounds">The new bounding rect.</param>
    /// <param name="matWidth">The new mat width.</param>
    /// <param name="matLength">The new mat length.</param>
    void setBounds(const QRectF &bounds, float matWidth, float matLength);

    /// <summary>
    /// Paints all the items to the provided painter.
    /// </summary>
    /// <param name="painter">The painter to paint the items to.</param>
    /// <param name="option">Unused.</param>
    /// <param name="widget">Unused.</param>
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    /// <summary>
    /// Adds a new item to the drawing and sets it up for use in this graphics
    /// item.
    /// </summary>
    /// <param name="item">An r-value reference to the new item.</param>
    void addItem(T &&item);

    /// <summary>
    /// Clears all the items from both the drawing and this object.
    /// </summary>
    void clearItems();

    /// <summary>
    /// Checks whether or not a point is within any of the individual items.
    /// </summary>
    /// <param name="point">The point to check against.</param>
    /// <returns>True if the point is contained in \a any of the items, false
    /// otherwise.</returns>
    bool contains(const QPointF &point) const override;

    /// <summary>
    /// Checks whether or not there are any items attached to the drawing.
    /// </summary>
    /// <returns>True if there are no items, false otherwise.</returns>
    bool empty() const;

    /// <summary>
    /// Binds to the respective remove function from the drawing to this, to
    /// remove the item according to the drawing.
    /// </summary>
    /// <param name="remove">The drawing's remove function.</param>
    void setRemoveFunction(const std::function<void(const T &)> &remove);

   protected:
    /// <summary>
    /// Overriden mouse press event for populating the inspector. Propogates
    /// event.
    /// </summary>
    /// <param name="event">The event that triggered this function.</param>
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /// <summary>
    /// Overriden mouse double click event for expanding the inspector widget.
    /// Propogates event.
    /// </summary>
    /// <param name="event"></param>
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    /// <summary>
    /// Overriden context menu event for showing a custom context menu.
    /// Propogates event.
    /// </summary>
    /// <param name="event"></param>
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

   private:
    QPainterPath calculateBounds(const T &item) const;

    void populateInspector(
        T &item);  // , Inspector* inspector, std::vector<T*>&,
                   // ComboboxComponentDataSource<Ds>*...);

    std::vector<T> &items;
    std::vector<unsigned> itemAcquireIDs;
    Inspector *inspector = nullptr;

    float currentWidth = 30, currentLength = 300;

    QRectF matBounds;
    float matWidth, matLength;

    std::function<void(const T &)> removeFunction = nullptr;

    std::tuple<ComboboxComponentDataSource<Ds> *...> componentSources;

    static void sync(const T &, T &);
};

template <typename T, DrawingComponentConcept... Ds>
GroupGraphicsItem<T, Ds...>::GroupGraphicsItem(std::vector<T> &items,
                                               const QRectF &matBounds,
                                               float matWidth, float matLength,
                                               Inspector *inspector)
    : items(items),
      matBounds(matBounds),
      matWidth(matWidth),
      matLength(matLength),
      inspector(inspector) {
    componentSources =
        std::make_tuple((new ComboboxComponentDataSource<Ds>)...);
    itemAcquireIDs = std::vector<unsigned>(items.size(), -1);
};

template <typename T, DrawingComponentConcept... Ds>
QRectF GroupGraphicsItem<T, Ds...>::boundingRect() const {
    return matBounds;
}

template <typename T, DrawingComponentConcept... Ds>
void GroupGraphicsItem<T, Ds...>::setBounds(const QRectF &rect, float matWidth,
                                            float matHeight) {
    matBounds = rect;
    this->matWidth = matWidth;
    this->matLength = matLength;
}

template <typename T, DrawingComponentConcept... Ds>
void GroupGraphicsItem<T, Ds...>::paint(QPainter *painter,
                                        const QStyleOptionGraphicsItem *option,
                                        QWidget *widget) {
    painter->save();

    for (T &item : items) {
        painter->setPen(Qt::red);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawPath(calculateBounds(item));
    }

    painter->restore();
}

template <typename T, DrawingComponentConcept... Ds>
void GroupGraphicsItem<T, Ds...>::addItem(T &&item) {
    if (!items.empty()) {
        GroupGraphicsItem<T, Ds...>::sync(items.front(), item);
    }
    items.push_back(std::move(item));
    itemAcquireIDs.push_back(-1);
    // itemAcquireIDs.emplace(items.back(), - 1);
}

template <typename T, DrawingComponentConcept... Ds>
void GroupGraphicsItem<T, Ds...>::clearItems() {
    items.clear();
    itemAcquireIDs.clear();
}

template <typename T, DrawingComponentConcept... Ds>
bool GroupGraphicsItem<T, Ds...>::contains(const QPointF &point) const {
    for (T &item : items) {
        if (calculateBounds(item).contains(point)) {
            return true;
        }
    }
    return false;
}

template <typename T, DrawingComponentConcept... Ds>
void GroupGraphicsItem<T, Ds...>::setRemoveFunction(
    const std::function<void(const T &)> &removeFunction) {
    this->removeFunction = removeFunction;
};

template <typename T, DrawingComponentConcept... Ds>
void GroupGraphicsItem<T, Ds...>::mousePressEvent(
    QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        for (unsigned i = 0; i < items.size(); i++) {
            T &item = items[i];
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

template <typename T, DrawingComponentConcept... Ds>
void GroupGraphicsItem<T, Ds...>::mouseDoubleClickEvent(
    QGraphicsSceneMouseEvent *event) {
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

template <typename T, DrawingComponentConcept... Ds>
void GroupGraphicsItem<T, Ds...>::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event) {
    for (unsigned i = 0; i < items.size(); i++) {
        T &item = items[i];
        if (calculateBounds(item).contains(event->pos())) {
            QMenu *menu = new QMenu();

            menu->addAction(
                "Remove",
                [this, item]() {
                    auto it = std::find(items.begin(), items.end(), item);
                    int index = it - items.begin();
                    if (removeFunction) {
                        if (itemAcquireIDs[index] ==
                            inspector->currentOwner()) {
                            inspector->clear();
                        }
                        itemAcquireIDs.erase(
                            std::next(itemAcquireIDs.begin(), index));
                        std::erase(items, item);
                    }
                },
                Qt::Key_Delete);

            menu->popup(event->screenPos());

            QWidget::connect(menu, &QMenu::triggered,
                             [menu](QAction *) { menu->deleteLater(); });
            break;
        }
    }
    QGraphicsItem::contextMenuEvent(event);
}

template <typename T, DrawingComponentConcept... Ds>
bool GroupGraphicsItem<T, Ds...>::empty() const {
    return items.empty();
}
#endif
