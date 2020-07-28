//
// Created by matthew on 27/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGPDFWRITER_H
#define DATABASE_MANAGER_DRAWINGPDFWRITER_H

#include <QPdfWriter>
#include <QPainter>
#include <QSvgRenderer>
#include <QImage>
#include <QPaintEngine>

#include <filesystem>

#include "Drawing.h"

class DrawingPDFWriter {
public:
	DrawingPDFWriter();

	void createPDF(const std::filesystem::path &pdfFilePath, const Drawing &drawing) const;

private:
	void drawStandardTemplate(QPainter &painter, QSvgRenderer &svgTemplateRenderer) const;

	void drawTextDetails(QPainter &painter, QSvgRenderer &svgTemplateRenderer, const Drawing &drawing) const;

	void drawMat(QPainter &painter, QRectF drawingRegion, const Drawing &drawing) const;

	void drawVerticallyCentredText(QPainter &painter, QPointF leftCentre, const QString &text) const;
};

#endif //DATABASE_MANAGER_DRAWINGPDFWRITER_H