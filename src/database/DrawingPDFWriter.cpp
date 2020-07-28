//
// Created by matthew on 27/07/2020.
//

#include "../../include/database/DrawingPDFWriter.h"

DrawingPDFWriter::DrawingPDFWriter() {

}

void DrawingPDFWriter::createPDF(const std::filesystem::path &pdfFilePath, const Drawing &drawing) const {
	QPdfWriter writer(pdfFilePath.string().c_str());
	writer.setPageSize(QPagedPaintDevice::A4);
	writer.setPageOrientation(QPageLayout::Landscape);
	writer.setPageMargins(QMargins(30, 15, 30, 15));

	QSvgRenderer svgTemplateRenderer(QString(":/drawing_pdf_base_template.svg"));

	QPainter painter(&writer);

	drawStandardTemplate(painter, svgTemplateRenderer);

	drawTextDetails(painter, svgTemplateRenderer, drawing);

	drawMat(painter, svgTemplateRenderer.boundsOnElement("drawing_target_region"), drawing);
}

void DrawingPDFWriter::drawStandardTemplate(QPainter &painter, QSvgRenderer &svgTemplateRenderer) const {
	QRect viewport = painter.viewport();

	painter.setPen(QPen(Qt::black, 10));

	painter.drawRect(viewport);

	svgTemplateRenderer.render(&painter);
}

void DrawingPDFWriter::drawTextDetails(QPainter &painter, QSvgRenderer &svgTemplateRenderer, const Drawing &drawing) const {
	painter.save();

	const float horizontalOffset = 20.0f, verticalOffset = 10.0f;

	QTextOption leftAlignedText;
	leftAlignedText.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	QRectF
		fieldManufacturerBox = svgTemplateRenderer.boundsOnElement("field_manufacturer").adjusted(horizontalOffset, 0, -horizontalOffset, 0),
		fieldModelBox = svgTemplateRenderer.boundsOnElement("field_model").adjusted(horizontalOffset, 0, -horizontalOffset, 0),
		fieldDeckBox = svgTemplateRenderer.boundsOnElement("field_deck").adjusted(horizontalOffset, 0, -horizontalOffset, 0),
		fieldQuantityOnDeckBox = svgTemplateRenderer.boundsOnElement("field_quantity_on_deck").adjusted(horizontalOffset, 0, -horizontalOffset, 0),
		fieldPositionBox = svgTemplateRenderer.boundsOnElement("field_position").adjusted(horizontalOffset, 0, -horizontalOffset, 0),
		fieldDateBox = svgTemplateRenderer.boundsOnElement("field_date").adjusted(horizontalOffset, 0, -horizontalOffset, 0);

	Drawing::MachineTemplate &machineTemplate = drawing.machineTemplate();

	painter.drawText(fieldManufacturerBox, machineTemplate.machine().manufacturer.c_str(), leftAlignedText);
	painter.drawText(fieldModelBox, machineTemplate.machine().model.c_str(), leftAlignedText);
	painter.drawText(fieldDeckBox, machineTemplate.deck().deck.c_str(), leftAlignedText);
	painter.drawText(fieldQuantityOnDeckBox, to_str(machineTemplate.quantityOnDeck).c_str(), leftAlignedText);
	painter.drawText(fieldPositionBox, machineTemplate.position.c_str(), leftAlignedText);

	std::tm date;
	date.tm_year = drawing.date().year - 1900;
	date.tm_mon = drawing.date().month - 1;
	date.tm_mday = drawing.date().day;

	std::stringstream dateText;
	dateText << std::put_time(&date, "%d/%m/%Y");

	painter.drawText(fieldDateBox, dateText.str().c_str(), leftAlignedText);

	QRectF
		fieldWidthBox = svgTemplateRenderer.boundsOnElement("field_width").adjusted(horizontalOffset, 0, -horizontalOffset, 0),
		fieldLengthBox = svgTemplateRenderer.boundsOnElement("field_length").adjusted(horizontalOffset, 0, -horizontalOffset, 0),
		fieldThicknessBox = svgTemplateRenderer.boundsOnElement("field_thickness").adjusted(horizontalOffset, 0, -horizontalOffset, 0),
		fieldApertureBox = svgTemplateRenderer.boundsOnElement("field_aperture").adjusted(horizontalOffset, 0, -horizontalOffset, 0),
		fieldSideIronsBox = svgTemplateRenderer.boundsOnElement("field_side_irons").adjusted(horizontalOffset, 0, -horizontalOffset, 0),
		fieldSideOverlapsBox = svgTemplateRenderer.boundsOnElement("field_side_overlaps").adjusted(horizontalOffset, 0, -horizontalOffset, 0),
		fieldNotesBox = svgTemplateRenderer.boundsOnElement("field_notes").adjusted(horizontalOffset, verticalOffset, -horizontalOffset, -verticalOffset);

	painter.drawText(fieldWidthBox, (to_str(drawing.width()) + "mm").c_str(), leftAlignedText);
	painter.drawText(fieldLengthBox, (to_str(drawing.length()) + "mm").c_str(), leftAlignedText);

	std::stringstream thicknessText;
	thicknessText << drawing.material(Drawing::TOP)->thickness;
	
	std::optional<Material> bottomLayer = drawing.material(Drawing::BOTTOM);
	if (bottomLayer.has_value()) {
		thicknessText << "+" << bottomLayer->thickness;
	}

	painter.drawText(fieldThicknessBox, thicknessText.str().c_str(), leftAlignedText);
	painter.drawText(fieldApertureBox, drawing.aperture().apertureName().c_str(), leftAlignedText);

	std::stringstream sideIronText;
	SideIron leftSideIron = drawing.sideIron(Drawing::LEFT), rightSideIron = drawing.sideIron(Drawing::RIGHT);

	sideIronText << "Type ";
	switch (leftSideIron.type) {
		case SideIronType::A:
			sideIronText << "Type A ";
			break;
		case SideIronType::B:
			sideIronText << "Type B ";
			break;
		case SideIronType::C:
			sideIronText << "Type C ";
			break;
		case SideIronType::D:
			sideIronText << "Type D ";
			break;
		case SideIronType::E:
			sideIronText << "Type E ";
			break;
		case SideIronType::None:
			sideIronText << "None";
			break;
	}
	if (leftSideIron.type != SideIronType::None) {
		sideIronText << "(" << leftSideIron.drawingNumber << ")";
	}

	if (leftSideIron.handle() != rightSideIron.handle()) {
		sideIronText << ", Type ";
		switch (rightSideIron.type) {
			case SideIronType::A:
				sideIronText << "Type A ";
				break;
			case SideIronType::B:
				sideIronText << "Type B ";
				break;
			case SideIronType::C:
				sideIronText << "Type C ";
				break;
			case SideIronType::D:
				sideIronText << "Type D ";
				break;
			case SideIronType::E:
				sideIronText << "Type E ";
				break;
			case SideIronType::None:
				sideIronText << "None";
				break;
		}
		if (rightSideIron.type != SideIronType::None) {
			sideIronText << "(" << rightSideIron.drawingNumber << ")";
		}
	}

	painter.drawText(fieldSideIronsBox, sideIronText.str().c_str(), leftAlignedText);

	std::stringstream sideOverlapsText;

	if (drawing.hasOverlaps() || drawing.hasSidelaps()) {
		std::optional<Drawing::Lap>
			leftOverlap = drawing.overlap(Drawing::LEFT),
			rightOverlap = drawing.overlap(Drawing::RIGHT),
			leftSidelap = drawing.sidelap(Drawing::LEFT),
			rightSidelap = drawing.sidelap(Drawing::RIGHT);

		std::vector<std::string> lapStrings;
		std::stringstream lapString;

		if (leftOverlap.has_value()) {
			lapString.str(std::string());
			lapString << leftOverlap->width << "mm";
			if (leftOverlap->attachmentType == LapAttachment::BONDED) {
				lapString << " (bonded)";
			}
			lapStrings.push_back(lapString.str());
		}
		if (rightOverlap.has_value()) {
			lapString.str(std::string());
			lapString << rightOverlap->width << "mm";
			if (rightOverlap->attachmentType == LapAttachment::BONDED) {
				lapString << " (bonded)";
			}
			lapStrings.push_back(lapString.str());
		}
		if (leftSidelap.has_value()) {
			lapString.str(std::string());
			lapString << leftSidelap->width << "mm";
			if (leftSidelap->attachmentType == LapAttachment::BONDED) {
				lapString << " (bonded)";
			}
			lapStrings.push_back(lapString.str());
		}
		if (rightSidelap.has_value()) {
			lapString.str(std::string());
			lapString << rightSidelap->width << "mm";
			if (rightSidelap->attachmentType == LapAttachment::BONDED) {
				lapString << " (bonded)";
			}
			lapStrings.push_back(lapString.str());
		}

		for (std::vector<std::string>::const_iterator it = lapStrings.begin(); it != lapStrings.end(); it++) {
			sideOverlapsText << *it;
			if (it != lapStrings.end() - 1) {
				sideOverlapsText << ", ";
			}
		}
	} else {
		sideOverlapsText << "No";
	}
	painter.drawText(fieldSideOverlapsBox, sideOverlapsText.str().c_str(), leftAlignedText);

	QTextOption topLeftAlignedText;
	topLeftAlignedText.setWrapMode(QTextOption::WordWrap);
	topLeftAlignedText.setAlignment(Qt::AlignLeft | Qt::AlignTop);
	painter.drawText(fieldNotesBox, drawing.notes().c_str(), topLeftAlignedText);

	QRectF
		fieldDrawingNumberBox = svgTemplateRenderer.boundsOnElement("field_drawing_number"),
		fieldNumberOfBarsBox = svgTemplateRenderer.boundsOnElement("field_number_of_bars");

	QTextOption centreAlignedText;
	centreAlignedText.setAlignment(Qt::AlignCenter);

	painter.drawText(fieldNumberOfBarsBox, (to_str(drawing.numberOfBars()) + " SUPPORT BARS").c_str(), centreAlignedText);

	QFont font = painter.font();
	font.setPointSize(20);
	painter.setFont(font);
	painter.drawText(fieldDrawingNumberBox, drawing.drawingNumber().c_str(), centreAlignedText);

	painter.restore();
}

void DrawingPDFWriter::drawMat(QPainter &painter, QRectF drawingRegion, const Drawing &drawing) const {
	const float maxDimensionPercentage = 0.8;

	float regionWidth = drawingRegion.width(), regionHeight = drawingRegion.height();
	float pWidth = 0, pLength = 0;

	std::optional<Drawing::Lap> leftLap, rightLap, topLap, bottomLap;

	switch (drawing.tensionType()) {
		case Drawing::SIDE:
			leftLap = drawing.sidelap(Drawing::LEFT);
			rightLap = drawing.sidelap(Drawing::RIGHT);
			topLap = drawing.overlap(Drawing::LEFT);
			bottomLap = drawing.overlap(Drawing::RIGHT);
			break;
		case Drawing::END:
			leftLap = drawing.overlap(Drawing::LEFT);
			rightLap = drawing.overlap(Drawing::RIGHT);
			topLap = drawing.sidelap(Drawing::LEFT);
			bottomLap = drawing.sidelap(Drawing::RIGHT);
			break;
	}

	struct CompositeDimension {
		float rLeft = 0;
		float rCentre = 0;
		float rRight = 0;

		inline float total() const {
			return rLeft + rCentre + rRight;
		}
	};

	CompositeDimension widthDim, lengthDim;
	widthDim.rCentre = drawing.width();
	lengthDim.rCentre = drawing.length();

	if (leftLap.has_value()) {
		widthDim.rLeft = leftLap->width;
	}
	if (rightLap.has_value()) {
		widthDim.rRight = rightLap->width;
	}
	if (topLap.has_value()) {
		lengthDim.rLeft = topLap->width;
	}
	if (bottomLap.has_value()) {
		lengthDim.rRight = bottomLap->width;
	}

	if (widthDim.total() / regionWidth > lengthDim.total() / regionHeight) {
		
	} else {

	}
}

void DrawingPDFWriter::drawVerticallyCentredText(QPainter &painter, QPointF leftCentre, const QString &text) const {
	leftCentre += QPointF(0, QFontMetrics(painter.font()).size(Qt::TextSingleLine, text).height() / 2.0f);
	painter.drawText(leftCentre, text);
}
