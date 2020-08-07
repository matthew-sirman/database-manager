//
// Created by matthew on 27/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGPDFWRITER_H
#define DATABASE_MANAGER_DRAWINGPDFWRITER_H

#include <QPdfWriter>
#include <QPainter>
#include <QSvgRenderer>
#include <QImage>
#include <QPainterPath>
#include <QtMath>
#include <QMessageBox>

#include <filesystem>

#include "Drawing.h"

class DrawingPDFWriter {
public:
	DrawingPDFWriter();

	bool createPDF(const std::filesystem::path &pdfFilePath, const Drawing &drawing) const;

private:
	void drawStandardTemplate(QPainter &painter, QSvgRenderer &svgTemplateRenderer) const;

	void drawLabelAndField(QPainter &painter, double left, double &top, const QString &label, double labelWidth, const QString &field, 
						   double fieldWidth, double hOffset = 0, double vOffset = 0) const;

	void drawTextDetails(QPainter &painter, QSvgRenderer &svgTemplateRenderer, const Drawing &drawing) const;

	void drawRubberScreenCloth(QPainter &painter, QRectF drawingRegion, const Drawing &drawing) const;

	enum ArrowMode {
		NO_HEAD,
		SINGLE_HEADED,
		DOUBLE_HEADED
	};

	static void drawArrow(QPainter &painter, QPointF from, QPointF to, const QString &label = QString(), ArrowMode mode = SINGLE_HEADED,
		bool flipLabel = false, QPen tailPen = QPen(), QPen headPen = QPen(), double headSize = 50, double angle = 30);
};

#endif //DATABASE_MANAGER_DRAWINGPDFWRITER_H