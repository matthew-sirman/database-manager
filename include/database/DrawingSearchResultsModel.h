//
// Created by matthew on 13/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGSEARCHRESULTSMODEL_H
#define DATABASE_MANAGER_DRAWINGSEARCHRESULTSMODEL_H

#include <QAbstractTableModel>

#include <functional>

#include "Drawing.h"

class DrawingSearchResultsModel : public QAbstractTableModel {
    Q_OBJECT

public:
    DrawingSearchResultsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void sourceDataFromBuffer(void* buffer);

    DrawingSummary summaryAtRow(int row) const;

protected:
    std::vector<DrawingSummary> summaries;
};


#endif //DATABASE_MANAGER_DRAWINGSEARCHRESULTSMODEL_H
