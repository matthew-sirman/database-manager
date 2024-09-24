//
// Created by matthew on 27/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGPDFWRITER_H
#define DATABASE_MANAGER_DRAWINGPDFWRITER_H

#include <QPdfWriter>
#include <QPainter>
#include <QSvgWidget>
#include <QSvgRenderer>
#include <QImage>
#include <QPainterPath>
#include <QtMath>
#include <QMessageBox>
#include <QFontDialog>
#include <QFontDatabase>
#include <QEventLoop>

#include <filesystem>

#include "../../../include/database/Drawing.h"


/// <summary>
/// DrawingPDFWriter
/// A class designed to help draw a drawing to a pdf.
/// </summary>
class DrawingPDFWriter {
public:
	/// <summary>
	/// Creates a new DrawingPDFWriter object.
	/// </summary>
	DrawingPDFWriter();

	/// <summary>
	/// Creates a pdf representing the drawing.
	/// </summary>
	/// <param name="pdfFilePath">The file path to store the drawing at.</param>
	/// <param name="drawing">The drawing to base the pdf off.</param>
	/// <param name="drawnByInitials">The author of the drawing.</param>
	/// <returns>True if the pdf drawing and creation was successful, false otherwise.</returns>
	bool createPDF(const std::filesystem::path &pdfFilePath, const Drawing &drawing, const std::string &drawnByInitials = std::string()) const;

private:
	void drawStandardTemplate(QPainter &painter, QSvgRenderer &svgTemplateRenderer) const;

	void drawLabelAndField(QPainter &painter, double left, double &top, const QString &label, double labelWidth, const QString &field, 
						   double fieldWidth, double hOffset = 0, double vOffset = 0) const;

	void drawTextDetails(QPainter &painter, QSvgRenderer &svgTemplateRenderer, const Drawing &drawing, const std::string &drawnByInitials) const;

	void drawRubberScreenCloth(QPainter &painter, QRectF drawingRegion, const Drawing &drawing) const;

	enum ArrowMode {
		NO_HEAD,
		SINGLE_HEADED,
		DOUBLE_HEADED
	};

	static void drawArrow(QPainter &painter, QPointF from, QPointF to, const QString &label = QString(), ArrowMode mode = SINGLE_HEADED,
		bool flipLabel = false, QPen tailPen = QPen(), QPen headPen = QPen(), double headSize = 50, double angle = 30, unsigned lines = 1);

	static void drawText(QPainter& painter, QPointF start, const QString& label, unsigned lines = 1,
		std::function<void(QRectF&, const QPointF&)> = std::bind(&QRectF::moveCenter, std::placeholders::_1, std::placeholders::_2));

	static QRectF adjustRect(const QRectF&);
};

#endif //DATABASE_MANAGER_DRAWINGPDFWRITER_H