#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#pragma ide diagnostic ignored "google-default-arguments"
//
// Created by matthew on 13/07/2020.
//

#include "../../include/database/DrawingSearchResultsModel.h"

int qIntVectorID = qRegisterMetaType<QVector<int>>();

DrawingSearchResultsModel::DrawingSearchResultsModel(QObject *parent) : QAbstractTableModel(parent) {

}

int DrawingSearchResultsModel::rowCount(const QModelIndex &parent) const {
    return summaries.size();
}

int DrawingSearchResultsModel::columnCount(const QModelIndex &parent) const {
    // Drawing Number | Drawing Dimensions
    return 2;
}

QVariant DrawingSearchResultsModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        DrawingSummary summary = summaries[index.row()];

        switch (index.column()) {
            case 0:
                return QString(summary.drawingNumber.c_str());
            case 1:
                return QString(summary.summaryString().c_str());
            default:
                break;
        }
    }

    return QVariant();
}

QVariant DrawingSearchResultsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
                case 0:
                    return "Drawing Number";
                case 1:
                    return "Drawing Specifications";
                default:
                    break;
            }
        }
    }

    return QVariant();
}

void DrawingSearchResultsModel::sourceDataFromBuffer(void *buffer) {
    unsigned char *buff = (unsigned char *)buffer;
    buff += sizeof(RequestType);

    DrawingSummaryCompressionSchema schema = *((DrawingSummaryCompressionSchema *) buff);
    buff += sizeof(DrawingSummaryCompressionSchema);

    unsigned recordCount = *((unsigned *)buff);
    buff += sizeof(unsigned);

    if (summaries.size() != 0) {
        beginRemoveRows(QModelIndex(), 0, (int)summaries.size() - 1);
        removeRows(0, (int)summaries.size() - 1);
        endRemoveRows();
    }

    summaries.clear();

    for (unsigned i = 0; i < recordCount; i++) {
        unsigned size;
        DrawingSummary summary = schema.uncompressSummary(buff, size);

        buff += size;

        summaries.push_back(summary);
    }

    beginInsertRows(QModelIndex(), 0, (int)recordCount - 1);
    insertRows(0, (int)recordCount - 1);
    endInsertRows();

    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

DrawingSummary DrawingSearchResultsModel::summaryAtRow(int row) const {
    return summaries[row];
}

#pragma clang diagnostic pop