//
// Created by matthew on 13/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGSEARCHRESULTSMODEL_H
#define DATABASE_MANAGER_DRAWINGSEARCHRESULTSMODEL_H

#include <QAbstractTableModel>

#include <functional>

#include "../../include/database/Drawing.h"

/// <summary>
/// DrawingSearchResultsModel
/// Displays the results of a search query to the database.
/// </summary>
class DrawingSearchResultsModel : public QAbstractTableModel {
    Q_OBJECT

public:
    /// <summary>
    /// Creates an empty search result table.
    /// </summary>
    /// <param name="parent"></param>
    DrawingSearchResultsModel(QObject *parent = nullptr);

    /// <summary>
    /// Getter for the row count.
    /// </summary>
    /// <param name="parent"></param>
    /// <returns></returns>
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /// <summary>
    /// Getter for the column count, which is always 3.
    /// </summary>
    /// <param name="parent"></param>
    /// <returns>3</returns>
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /// <summary>
    /// Gets the data selected by the user.
    /// </summary>
    /// <param name="index">The index of the data selected by the user.</param>
    /// <param name="role">The type of text selected (editible, visible etc)</param>
    /// <returns></returns>
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /// <summary>
    /// Returns the header's data selected by the user.
    /// </summary>
    /// <param name="section">The column or row selected</param>
    /// <param name="orientation">Whether section refers to column or now</param>
    /// <param name="role">The type of text selected (editible, visible etc)</param>
    /// <returns></returns>
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /// <summary>
    /// Populates the table with drawing summaries from a given buffer.
    /// </summary>
    /// <param name="buffer">The buffer to source summaries from, as a rvalue reference
    /// to indicate gaining ownership over this buffer.</param>
    void sourceDataFromBuffer(void*&& buffer);

    /// <summary>
    /// Getter for the drawing summary in a specific row.
    /// </summary>
    /// <param name="row">The row to source the summary.</param>
    /// <returns>The sourced summary.</returns>
    DrawingSummary summaryAtRow(int row) const;

protected:
    /// <summary>
    /// A vector of all the summaries, to populate the search results table,
    /// and have information be read off them as required.
    /// </summary>
    std::vector<DrawingSummary> summaries;
};


#endif //DATABASE_MANAGER_DRAWINGSEARCHRESULTSMODEL_H
