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
    // Drawing Number | Drawing Dimensions | Bar Spacings
    return 3;
}

QVariant DrawingSearchResultsModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        DrawingSummary summary = summaries[index.row()];

        switch (index.column()) {
            case 0:
                return QString(summary.drawingNumber.c_str());
            case 1:
                return QString(summary.summaryString().c_str());
            case 2: {
                if (summary.barSpacingCount() == 1) {
                    return QString("No bars");
                }
                std::stringstream spacingsString;
                std::vector<float> spacings = summary.barSpacings();
                for (std::vector<float>::const_iterator it = spacings.begin(); it != spacings.end(); it++) {
                    spacingsString << *it;
                    if (it != spacings.end() - 1) {
                        spacingsString << "+";
                    }
                }
                return QString(spacingsString.str().c_str());
            }
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
                case 2:
                    return "Bar Spacings";
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

    if (summaries.size() != 0) {
        beginInsertRows(QModelIndex(), 0, (int)recordCount - 1);
        insertRows(0, (int)recordCount - 1);
        endInsertRows();
    }

    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

DrawingSummary DrawingSearchResultsModel::summaryAtRow(int row) const {
    return summaries[row];
}