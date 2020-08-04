//
// Created by matthew on 27/07/2020.
//

#include "../../include/database/DrawingPDFWriter.h"

DrawingPDFWriter::DrawingPDFWriter() {

}

bool DrawingPDFWriter::createPDF(const std::filesystem::path &pdfFilePath, const Drawing &drawing) const {
	std::string productName = drawing.product().productName;

	if (productName == "Rubber Modules and Panels") {
		QMessageBox::about(nullptr, "Unsupported Type", "Rubber Modules and Panels PDF generation is not supported.");
		return false;
	}

	QPdfWriter writer(pdfFilePath.string().c_str());

	writer.setPageSize(QPagedPaintDevice::A4);
	writer.setPageOrientation(QPageLayout::Landscape);
	writer.setPageMargins(QMargins(30, 15, 30, 15));

	QSvgRenderer svgTemplateRenderer(QString(":/drawing_pdf_base_template.svg"));

	QPainter painter;

	if (!painter.begin(&writer)) {
		return false; 
	}

	drawStandardTemplate(painter, svgTemplateRenderer);

	drawTextDetails(painter, svgTemplateRenderer, drawing);

	drawRubberScreenCloth(painter, svgTemplateRenderer.boundsOnElement("drawing_target_region"), drawing);

	return true;
}

void DrawingPDFWriter::drawStandardTemplate(QPainter &painter, QSvgRenderer &svgTemplateRenderer) const {
	QRect viewport = painter.viewport();

	painter.setPen(QPen(Qt::black, 5));

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
		sideIronText << ", ";
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
	font.setPointSize(18);
	painter.setFont(font);
	painter.drawText(fieldDrawingNumberBox, drawing.drawingNumber().c_str(), centreAlignedText);

	painter.restore();
}

void DrawingPDFWriter::drawRubberScreenCloth(QPainter &painter, QRectF drawingRegion, const Drawing &drawing) const {
	const double maxDimensionPercentage = 0.8;
	const double horizontalBarSizePercentage = 0.03;
	const double dimensionSpacingHeight = 0.7, dimensionBarHeight = 0.8, dimensionInnerSpacingHeight = 0.9;
	const double dimensionHorizontalLapOffset = 0.3, dimensionVerticalLapOffset = 0.3;
	const double shortDimensionLineSize = 0.02, longDimensionLineSize = 0.04;
	const double mainDimensionLineSize = 0.06;

	QPen dashDotPen = QPen(QBrush(Qt::black), 1, Qt::DashDotLine);
	dashDotPen.setDashPattern({ 96, 48, 4, 48 });

	QPen dashPen = QPen(QBrush(Qt::black), 1, Qt::DashLine);
	dashPen.setDashPattern({ 96, 48 });

	double regionWidth = drawingRegion.width(), regionHeight = drawingRegion.height();

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

	struct {
		double rLeft = 0;
		double rCentre = 0;
		double rRight = 0;

		inline double total() const {
			return rLeft + rCentre + rRight;
		}
	} widthDim, lengthDim;

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

	double pWidth, pLength;

	if (widthDim.total() / regionWidth > lengthDim.total() / regionHeight) {
		pWidth = maxDimensionPercentage;
		pLength = maxDimensionPercentage * ((lengthDim.total() / regionHeight) / (widthDim.total() / regionWidth));
	} else {
		pLength = maxDimensionPercentage;
		pWidth = maxDimensionPercentage * ((widthDim.total() / regionWidth) / (lengthDim.total() / regionHeight));
	}

	QRectF matBoundingRegion;
	matBoundingRegion.setTopLeft(QPointF(
		(0.5 - pWidth * (0.5 - widthDim.rLeft / widthDim.total())) * regionWidth + drawingRegion.left(),
		(0.5 - pLength * (0.5 - lengthDim.rLeft / lengthDim.total())) * regionHeight + drawingRegion.top()
	));
	matBoundingRegion.setBottomRight(QPointF(
		(0.5 + pWidth * (0.5 - widthDim.rRight / widthDim.total())) * regionWidth + drawingRegion.left(),
		(0.5 + pLength * (0.5 - lengthDim.rRight / lengthDim.total())) * regionHeight + drawingRegion.top()
	));

	painter.drawRect(matBoundingRegion);

	QLineF leftWidthExtender(
		QPointF(matBoundingRegion.left(), (0.5 * (1.0 - pLength)) *regionHeight + drawingRegion.top()),
		QPointF(matBoundingRegion.left(), (0.5 * (1.0 - pLength) - mainDimensionLineSize) *regionHeight + drawingRegion.top()));
	QLineF rightWidthExtender(
		QPointF(matBoundingRegion.right(), (0.5 * (1.0 - pLength)) *regionHeight + drawingRegion.top()),
		QPointF(matBoundingRegion.right(), (0.5 * (1.0 - pLength) - mainDimensionLineSize) *regionHeight + drawingRegion.top()));

	painter.setPen(dashPen);
	painter.drawLine(leftWidthExtender);
	painter.drawLine(rightWidthExtender);
	painter.setPen(Qt::black);

	drawArrow(painter, leftWidthExtender.center(), rightWidthExtender.center(), to_str(drawing.width()).c_str(), DOUBLE_HEADED);

	QLineF topLengthExtender(
		QPointF((0.5 * (1.0 - pWidth)) *regionWidth + drawingRegion.left(), matBoundingRegion.top()),
		QPointF((0.5 * (1.0 - pWidth) - mainDimensionLineSize) *regionWidth + drawingRegion.left(), matBoundingRegion.top()));
	QLineF bottomLengthExtender(
		QPointF((0.5 * (1.0 - pWidth)) *regionWidth + drawingRegion.left(), matBoundingRegion.bottom()),
		QPointF((0.5 * (1.0 - pWidth) - mainDimensionLineSize) *regionWidth + drawingRegion.left(), matBoundingRegion.bottom()));

	painter.setPen(dashPen);
	painter.drawLine(topLengthExtender);
	painter.drawLine(bottomLengthExtender);
	painter.setPen(Qt::black);

	drawArrow(painter, topLengthExtender.center(), bottomLengthExtender.center(), to_str(drawing.length()).c_str(), DOUBLE_HEADED, true);

	if (leftLap.has_value()) {
		QRectF leftLapRegion;
		leftLapRegion.setLeft(
			(0.5 * (1.0 - pWidth)) * regionWidth + drawingRegion.left()
		);
		leftLapRegion.setTopRight(matBoundingRegion.topLeft());
		leftLapRegion.setBottomRight(matBoundingRegion.bottomLeft());

		painter.drawRect(leftLapRegion);

		QPointF lapLeft(leftLapRegion.left(), matBoundingRegion.height() *dimensionVerticalLapOffset + matBoundingRegion.top());
		QPointF lapRight(leftLapRegion.right(), matBoundingRegion.height() *dimensionVerticalLapOffset + matBoundingRegion.top());

		std::string lapString = to_str(leftLap->width) + "mm";
		if (leftLap->attachmentType == LapAttachment::BONDED) {
			lapString += " Bonded";
		}

		drawArrow(painter, QPointF(lapLeft.x() - longDimensionLineSize * regionWidth, lapLeft.y()), lapLeft,
			lapString.c_str());
		drawArrow(painter, QPointF(lapRight.x() + shortDimensionLineSize * regionWidth, lapRight.y()), lapRight);
	}
	if (rightLap.has_value()) {
		QRectF rightLapRegion;
		rightLapRegion.setRight(
			(0.5 * (1.0 + pWidth)) * regionWidth + drawingRegion.left()
		);
		rightLapRegion.setTopLeft(matBoundingRegion.topRight());
		rightLapRegion.setBottomLeft(matBoundingRegion.bottomRight());

		painter.drawRect(rightLapRegion);

		QPointF lapLeft(rightLapRegion.left(), matBoundingRegion.height() *dimensionVerticalLapOffset + matBoundingRegion.top());
		QPointF lapRight(rightLapRegion.right(), matBoundingRegion.height() *dimensionVerticalLapOffset + matBoundingRegion.top());

		std::string lapString = to_str(rightLap->width) + "mm";
		if (rightLap->attachmentType == LapAttachment::BONDED) {
			lapString += " Bonded";
		}

		drawArrow(painter, QPointF(lapLeft.x() - shortDimensionLineSize * regionWidth, lapLeft.y()), lapLeft);
		drawArrow(painter, QPointF(lapRight.x() + longDimensionLineSize * regionWidth, lapRight.y()), lapRight,
			lapString.c_str(), SINGLE_HEADED, true);
	}
	if (topLap.has_value()) {
		QRectF topLapRegion;
		topLapRegion.setTop(
			(0.5 * (1.0 - pLength)) * regionHeight + drawingRegion.top()
		);
		topLapRegion.setBottomLeft(matBoundingRegion.topLeft());
		topLapRegion.setBottomRight(matBoundingRegion.topRight());

		painter.drawRect(topLapRegion);

		QPointF lapTop(matBoundingRegion.width() *dimensionHorizontalLapOffset + matBoundingRegion.left(), topLapRegion.top());
		QPointF lapBottom(matBoundingRegion.width() *dimensionHorizontalLapOffset + matBoundingRegion.left(), topLapRegion.bottom());

		std::string lapString = to_str(topLap->width) + "mm";
		if (topLap->attachmentType == LapAttachment::BONDED) {
			lapString += " Bonded";
		}

		drawArrow(painter, QPointF(lapTop.x(), lapTop.y() - longDimensionLineSize * regionHeight), lapTop,
			lapString.c_str());
		drawArrow(painter, QPointF(lapBottom.x(), lapBottom.y() + shortDimensionLineSize * regionHeight), lapBottom);
	}
	if (bottomLap.has_value()) {
		QRectF bottomLapRegion;
		bottomLapRegion.setBottom(
			(0.5 * (1.0 + pLength)) * regionHeight + drawingRegion.top()
		);
		bottomLapRegion.setTopLeft(matBoundingRegion.bottomLeft());
		bottomLapRegion.setTopRight(matBoundingRegion.bottomRight());

		painter.drawRect(bottomLapRegion);

		QPointF lapTop(matBoundingRegion.width() * dimensionHorizontalLapOffset + matBoundingRegion.left(), bottomLapRegion.top());
		QPointF lapBottom(matBoundingRegion.width() * dimensionHorizontalLapOffset + matBoundingRegion.left(), bottomLapRegion.bottom());

		std::string lapString = to_str(bottomLap->width) + "mm";
		if (bottomLap->attachmentType == LapAttachment::BONDED) {
			lapString += " Bonded";
		}

		drawArrow(painter, QPointF(lapTop.x(), lapTop.y() - shortDimensionLineSize * regionHeight), lapTop);
		drawArrow(painter, QPointF(lapBottom.x(), lapBottom.y() + longDimensionLineSize * regionHeight), lapBottom,
			lapString.c_str());
	}

	std::vector<double> apertureRegionEndpoints;
	apertureRegionEndpoints.push_back(drawing.leftBar());

	double currentMatPosition = 0;

	painter.setPen(dashDotPen);

	for (unsigned bar = 0; bar < drawing.numberOfBars(); bar++) {
		currentMatPosition += drawing.barSpacing(bar);

		QLineF barCentreDividerLine;
		barCentreDividerLine.setP1(QPointF(
			(currentMatPosition / widthDim.rCentre) *matBoundingRegion.width() + matBoundingRegion.left(),
			horizontalBarSizePercentage *matBoundingRegion.height() + matBoundingRegion.top()
		));
		barCentreDividerLine.setP2(QPointF(
			(currentMatPosition / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
			(1 - horizontalBarSizePercentage) * matBoundingRegion.height() + matBoundingRegion.top()
		));

		painter.drawLine(barCentreDividerLine);

		double barWidth = drawing.barWidth(bar + 1);
		apertureRegionEndpoints.push_back(currentMatPosition - barWidth / 2);
		apertureRegionEndpoints.push_back(currentMatPosition + barWidth / 2);
	}

	apertureRegionEndpoints.push_back(widthDim.rCentre - drawing.rightBar());

	painter.setPen(Qt::black);

	for (unsigned apertureRegion = 0; apertureRegion < apertureRegionEndpoints.size() / 2; apertureRegion++) {
		double start = apertureRegionEndpoints[2 * apertureRegion];
		double end = apertureRegionEndpoints[2 * apertureRegion + 1];

		QRectF region;
		region.setTopLeft(QPointF(
			(start / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
			horizontalBarSizePercentage * matBoundingRegion.height() + matBoundingRegion.top()
		));
		region.setBottomRight(QPointF(
			(end / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
			(1 - horizontalBarSizePercentage) * matBoundingRegion.height() + matBoundingRegion.top()
		));

		painter.drawRect(region);
	}

	double spacingPosition = 0;

	for (unsigned spacingDimension = 0; spacingDimension <= drawing.numberOfBars(); spacingDimension++) {
		double nextSpacingPosition = spacingPosition + drawing.barSpacing(spacingDimension);

		drawArrow(
			painter,
			QPointF(
				(spacingPosition / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
				matBoundingRegion.height() * dimensionSpacingHeight + matBoundingRegion.top()),
			QPointF(
				(nextSpacingPosition / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
				matBoundingRegion.height() * dimensionSpacingHeight + matBoundingRegion.top()),
			to_str(drawing.barSpacing(spacingDimension)).c_str(), DOUBLE_HEADED
		);

		double leftInnerSpacing = apertureRegionEndpoints[2 * spacingDimension], rightInnerSpacing = apertureRegionEndpoints[2 * spacingDimension + 1];

		drawArrow(
			painter,
			QPointF(
				(leftInnerSpacing / widthDim.rCentre) *matBoundingRegion.width() + matBoundingRegion.left(),
				matBoundingRegion.height() *dimensionInnerSpacingHeight + matBoundingRegion.top()),
			QPointF(
				(rightInnerSpacing / widthDim.rCentre) *matBoundingRegion.width() + matBoundingRegion.left(),
				matBoundingRegion.height() *dimensionInnerSpacingHeight + matBoundingRegion.top()),
			to_str(rightInnerSpacing - leftInnerSpacing).c_str(), DOUBLE_HEADED
		);

		spacingPosition = nextSpacingPosition;
	}

	std::vector<double> barEndpoints = apertureRegionEndpoints;
	barEndpoints.insert(barEndpoints.begin(), 0);
	barEndpoints.push_back(widthDim.rCentre);

	for (unsigned bar = 0; bar < drawing.numberOfBars() + 2; bar++) {
		QPointF barLeft((barEndpoints[2 * bar] / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
			matBoundingRegion.height() * dimensionBarHeight + matBoundingRegion.top());
		QPointF barRight((barEndpoints[2 * bar + 1] / widthDim.rCentre) *matBoundingRegion.width() + matBoundingRegion.left(),
			matBoundingRegion.height() *dimensionBarHeight + matBoundingRegion.top());

		drawArrow(painter, QPointF(barLeft.x() - shortDimensionLineSize * regionWidth, barLeft.y()), barLeft);
		drawArrow(painter, QPointF(barRight.x() + longDimensionLineSize * regionWidth, barRight.y()), barRight,
			to_str(drawing.barWidth(bar)).c_str(), SINGLE_HEADED, true);
	}
}

void DrawingPDFWriter::drawArrow(QPainter &painter, QPointF from, QPointF to, const QString &label, ArrowMode mode, bool flipLabel, QPen tailPen, QPen headPen, double headSize, double angle) {
	painter.save();
	painter.setPen(tailPen);

	QLineF tail(from, to);

	painter.drawLine(tail);

	if (!label.isEmpty()) {
		const double padding = 10;

		QTextOption centreAlignedText;
		centreAlignedText.setAlignment(Qt::AlignCenter);

		QRectF labelBounds = QFontMetricsF(painter.font()).boundingRect(label).adjusted(-padding, -padding, padding, padding);
		tail.center();

		QLineF boundsDiameter(labelBounds.center(), labelBounds.topLeft());

		double l = boundsDiameter.length();
		double alpha = boundsDiameter.angle();
		double theta = tail.angle();

		QLineF arrowToTextCentre = QLineF(tail.center(), to).normalVector();
		if (!flipLabel) {
			arrowToTextCentre.setLength(l * cos(qDegreesToRadians(abs(90 - theta) - alpha)));
		} else {
			arrowToTextCentre.setLength(-l * cos(qDegreesToRadians(abs(90 - theta) - alpha)));
		}

		labelBounds.moveCenter(arrowToTextCentre.p2());

		painter.drawText(labelBounds, label, centreAlignedText);
	}

	painter.setPen(headPen);

	switch (mode) {
		case DOUBLE_HEADED: {
			QPainterPath path;
			QLineF h1, h2;
			h1.setP1(from);
			h2.setP1(from);
			h1.setAngle(tail.angle() + angle);
			h1.setLength(headSize);
			h2.setAngle(tail.angle() - angle);
			h2.setLength(headSize);
			path.moveTo(h1.p2());
			path.lineTo(from);
			path.lineTo(h2.p2());

			painter.drawPath(path);
		}
		case SINGLE_HEADED: {
			QPainterPath path;
			QLineF h1, h2;
			h1.setP1(to);
			h2.setP1(to);
			h1.setAngle(tail.angle() + 180 + angle);
			h1.setLength(headSize);
			h2.setAngle(tail.angle() + 180 - angle);
			h2.setLength(headSize);
			path.moveTo(h1.p2());
			path.lineTo(to);
			path.lineTo(h2.p2());

			painter.drawPath(path);
		}
		case NO_HEAD:
			break;
	}

	painter.restore();
}