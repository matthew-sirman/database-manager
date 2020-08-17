//
// Created by matthew on 27/07/2020.
//

#include "../../include/database/DrawingPDFWriter.h"

DrawingPDFWriter::DrawingPDFWriter() {

}

bool DrawingPDFWriter::createPDF(const std::filesystem::path &pdfFilePath, const Drawing &drawing,
                                 const std::string &drawnByInitials) const {
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

    drawTextDetails(painter, svgTemplateRenderer, drawing, drawnByInitials);

    drawRubberScreenCloth(painter, svgTemplateRenderer.boundsOnElement("drawing_target_region"), drawing);

    return true;
}

void DrawingPDFWriter::drawStandardTemplate(QPainter &painter, QSvgRenderer &svgTemplateRenderer) const {
    QRect viewport = painter.viewport();

    painter.setPen(QPen(Qt::black, 5));

    painter.drawRect(viewport);

    svgTemplateRenderer.render(&painter);
}

void DrawingPDFWriter::drawLabelAndField(QPainter &painter, double left, double &top, const QString &label,
                                         double labelWidth, const QString &field,
                                         double fieldWidth, double hOffset, double vOffset) const {
    QTextOption topLeftTextOption;
    topLeftTextOption.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    topLeftTextOption.setWrapMode(QTextOption::WordWrap);

    QRectF labelBounds = QFontMetrics(painter.font()).boundingRect(
            QRect(left, top, labelWidth, 0)
                    .adjusted(hOffset, vOffset, -hOffset, -vOffset),
            Qt::TextWordWrap, label
    );
    QRectF fieldBounds = QFontMetrics(painter.font()).boundingRect(
            QRect(left + labelWidth, top, fieldWidth, 0)
                    .adjusted(hOffset, vOffset, -hOffset, -vOffset),
            Qt::TextWordWrap, field
    );

    double height = MAX(labelBounds.height(), fieldBounds.height()) + 2 * vOffset;

    painter.drawRect(QRectF(left, top, labelWidth, height));
    painter.drawRect(QRectF(left + labelWidth, top, fieldWidth, height));
    painter.drawText(labelBounds, label, topLeftTextOption);
    painter.drawText(fieldBounds, field, topLeftTextOption);

    top += height;
}

void
DrawingPDFWriter::drawTextDetails(QPainter &painter, QSvgRenderer &svgTemplateRenderer, const Drawing &drawing,
                                  const std::string &drawnByInitials) const {
    painter.save();

    const double horizontalOffset = 50.0, verticalOffset = 40.0;

    QTextOption leftAlignedText;
    leftAlignedText.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QTextOption centreAlignedText;
    centreAlignedText.setAlignment(Qt::AlignCenter);

    QRectF
            generalDetailsBox = svgTemplateRenderer.boundsOnElement("general_details_target_region"),
            manufacturingDetailsBox = svgTemplateRenderer.boundsOnElement("manufacturing_details_target_region");

    const double labelWidth = 1500, fieldWidth = 2300;
    double currentVPos = generalDetailsBox.top();

    QString labelText, fieldText;

    Drawing::MachineTemplate machineTemplate = drawing.machineTemplate();

    labelText = "Manufacturer";
    fieldText = machineTemplate.machine().manufacturer.c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    labelText = "Model";
    fieldText = machineTemplate.machine().model.c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    labelText = "Deck";
    fieldText = machineTemplate.deck().deck.c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    labelText = "Quantity On Deck";
    fieldText = to_str(machineTemplate.quantityOnDeck).c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    labelText = "Position";
    fieldText = machineTemplate.position.c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    labelText = "Date";

    std::tm date;
    date.tm_year = drawing.date().year - 1900;
    date.tm_mon = drawing.date().month - 1;
    date.tm_mday = drawing.date().day;

    std::stringstream dateText;
    dateText << std::put_time(&date, "%d/%m/%Y");

    fieldText = dateText.str().c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    currentVPos = manufacturingDetailsBox.top();

    labelText = "Width";
    fieldText = (to_str(drawing.width()) + "mm").c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    labelText = "Length";
    fieldText = (to_str(drawing.length()) + "mm").c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    labelText = "Thickness";
    std::stringstream thicknessText;
    thicknessText << drawing.material(Drawing::TOP)->thickness;

    std::optional<Material> bottomLayer = drawing.material(Drawing::BOTTOM);
    if (bottomLayer.has_value()) {
        thicknessText << "+" << bottomLayer->thickness;
    }
    fieldText = thicknessText.str().c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    labelText = "Aperture";
    fieldText = drawing.aperture().apertureName().c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    labelText = "Side Irons";

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

    fieldText = sideIronText.str().c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    labelText = "Side Overlaps";

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
                lapString << " (bonded " << leftOverlap->material().thickness << "mm)";
            }
            lapStrings.push_back(lapString.str());
        }
        if (rightOverlap.has_value()) {
            lapString.str(std::string());
            lapString << rightOverlap->width << "mm";
            if (rightOverlap->attachmentType == LapAttachment::BONDED) {
                lapString << " (bonded " << rightOverlap->material().thickness << "mm)";
            }
            lapStrings.push_back(lapString.str());
        }
        if (leftSidelap.has_value()) {
            lapString.str(std::string());
            lapString << leftSidelap->width << "mm";
            if (leftSidelap->attachmentType == LapAttachment::BONDED) {
                lapString << " (bonded " << leftSidelap->material().thickness << "mm)";
            }
            lapStrings.push_back(lapString.str());
        }
        if (rightSidelap.has_value()) {
            lapString.str(std::string());
            lapString << rightSidelap->width << "mm";
            if (rightSidelap->attachmentType == LapAttachment::BONDED) {
                lapString << " (bonded " << rightSidelap->material().thickness << "mm)";
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

    fieldText = sideOverlapsText.str().c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    std::string productName = drawing.product().productName;

    if (productName == "Rubber Screen Cloth") {
        if (drawing.material(Drawing::TOP)->thickness >= 15) {
            labelText = "Rebated";
            fieldText = drawing.rebated() ? "Yes" : "No";
            drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                              fieldText, fieldWidth, horizontalOffset, verticalOffset);

            if (drawing.hasBackingStrips()) {
                labelText = "Backing Strips";
                fieldText = "Yes";
                drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                                  fieldText, fieldWidth, horizontalOffset, verticalOffset);
            }
        } else {
            labelText = "Backing Strips";
            fieldText = drawing.hasBackingStrips() ? "Yes" : "No";
            drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                              fieldText, fieldWidth, horizontalOffset, verticalOffset);
        }
    } else if (productName == "Extraflex" || productName == "Polyflex") {
        if (drawing.hasBackingStrips()) {
            labelText = "Backing Strips";
            fieldText = "Yes";
            drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                              fieldText, fieldWidth, horizontalOffset, verticalOffset);
        }
    }

    if (drawing.numberOfImpactPads()) {
        std::stringstream impactPadFieldText;
        std::vector<Drawing::ImpactPad> pads = drawing.impactPads();
        for (unsigned i = 0; i < pads.size(); i++) {
            Drawing::ImpactPad pad = pads[i];

            Material &mat = pad.material();

            impactPadFieldText << pad.width << "x" << pad.length << " at " << "(" << pad.pos.x << ", " << pad.pos.y
                               << ") ";
            impactPadFieldText << mat.thickness << "mm " << mat.materialName << ", " << pad.aperture().apertureName();

            if (i != pads.size() - 1) {
                impactPadFieldText << ", ";
            }
        }

        labelText = "Impact Pad(s)";
        fieldText = impactPadFieldText.str().c_str();
        drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                          fieldText, fieldWidth, horizontalOffset, verticalOffset);
    }

    if (drawing.numberOfCentreHoles()) {
        std::stringstream centreHolesFieldText;
        std::vector<Drawing::CentreHole> holes = drawing.centreHoles();

        Drawing::CentreHole::Shape shape = holes.front().centreHoleShape;

        centreHolesFieldText << shape.width << "x" << shape.length << " ";
        if (shape.rounded) {
            centreHolesFieldText << "(rounded) ";
        }
        centreHolesFieldText << "at ";

        std::vector<float> positions;
        positions.reserve(holes.size());
        for (const Drawing::CentreHole &hole : holes) {
            positions.push_back(hole.pos.y);
        }

        std::sort(positions.begin(), positions.end());
        std::vector<float>::iterator last = std::unique(positions.begin(), positions.end());
        positions.erase(last, positions.end());

        float lastY = 0;
        for (float y : positions) {
            centreHolesFieldText << (y - lastY);
            lastY = y;
            if (y != positions.back()) {
                centreHolesFieldText << "+";
            }
        }

        labelText = "Centre Holes";
        fieldText = centreHolesFieldText.str().c_str();
        drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                          fieldText, fieldWidth, horizontalOffset, verticalOffset);
    }

    if (drawing.numberOfDeflectors()) {
        std::stringstream deflectorsFieldText;
        std::vector<Drawing::Deflector> deflectors = drawing.deflectors();

        Material &mat = deflectors.front().material();

        deflectorsFieldText << deflectors.front().size << "mm " << mat.thickness << "mm " << mat.materialName << " at ";

        for (unsigned i = 0; i < deflectors.size(); i++) {
            deflectorsFieldText << "(" << deflectors[i].pos.x << ", " << deflectors[i].pos.y << ")";
            if (i != deflectors.size() - 1) {
                deflectorsFieldText << ", ";
            }
        }

        labelText = "Deflectors";
        fieldText = deflectorsFieldText.str().c_str();
        drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                          fieldText, fieldWidth, horizontalOffset, verticalOffset);
    }

    if (drawing.numberOfDivertors()) {
        std::stringstream divertorsFieldText;
        std::vector<Drawing::Divertor> divertors = drawing.divertors();

        Material &mat = divertors.front().material();

        divertorsFieldText << divertors.front().width << "x" << divertors.front().length << " " << mat.thickness
                           << "mm " << mat.materialName << " at ";

        std::vector<float> leftYPositions, rightYPositions;

        for (const Drawing::Divertor &divertor : divertors) {
            switch (divertor.side) {
                case Drawing::LEFT:
                    leftYPositions.push_back(divertor.verticalPosition);
                    break;
                case Drawing::RIGHT:
                    rightYPositions.push_back(divertor.verticalPosition);
                    break;
            }
        }

        std::sort(leftYPositions.begin(), leftYPositions.end());
        std::sort(rightYPositions.begin(), rightYPositions.end());

        bool matchingSides = true;

        if (leftYPositions.size() == rightYPositions.size()) {
            for (unsigned i = 0; i < leftYPositions.size(); i++) {
                if (leftYPositions[i] != rightYPositions[i]) {
                    matchingSides = false;
                    break;
                }
            }
        } else {
            matchingSides = false;
        }

        if (matchingSides) {
            float lastY = 0;
            for (float y : leftYPositions) {
                divertorsFieldText << (y - lastY);
                lastY = y;
                if (y != leftYPositions.back()) {
                    divertorsFieldText << "+";
                }
            }
        } else if (leftYPositions.empty()) {
            float lastY = 0;
            for (float y : rightYPositions) {
                divertorsFieldText << (y - lastY);
                lastY = y;
                if (y != rightYPositions.back()) {
                    divertorsFieldText << "+";
                }
            }
        } else if (rightYPositions.empty()) {
            float lastY = 0;
            for (float y : leftYPositions) {
                divertorsFieldText << (y - lastY);
                lastY = y;
                if (y != leftYPositions.back()) {
                    divertorsFieldText << "+";
                }
            }
        } else {
            divertorsFieldText << "Left: ";
            float lastY = 0;
            for (float y : leftYPositions) {
                divertorsFieldText << (y - lastY);
                lastY = y;
                if (y != leftYPositions.back()) {
                    divertorsFieldText << "+";
                }
            }
            divertorsFieldText << ", Right: ";
            lastY = 0;
            for (float y : rightYPositions) {
                divertorsFieldText << (y - lastY);
                lastY = y;
                if (y != rightYPositions.back()) {
                    divertorsFieldText << "+";
                }
            }
        }

        labelText = "Divertors";
        fieldText = divertorsFieldText.str().c_str();
        drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                          fieldText, fieldWidth, horizontalOffset, verticalOffset);
    }

    labelText = "Notes";
    fieldText = drawing.notes().c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    QRectF
            fieldDrawingNumberBox = svgTemplateRenderer.boundsOnElement("field_drawing_number"),
            fieldTitleBox = svgTemplateRenderer.boundsOnElement("field_title"),
            fieldDateBox = svgTemplateRenderer.boundsOnElement("field_date"),
            fieldDrawnByBox = svgTemplateRenderer.boundsOnElement("field_drawn_by"),
            fieldSCSLogoBox = svgTemplateRenderer.boundsOnElement("field_scs_logo"),
            fieldSCSDetailsBox = svgTemplateRenderer.boundsOnElement("field_scs_details");

    std::stringstream title;
    title << drawing.product().productName;

    if (drawing.product().productName != "Bivitec" && drawing.product().productName != "Flip Flow") {
        title << " " + to_str(drawing.numberOfBars()) + " Support Bar" + (drawing.numberOfBars() == 1 ? "" : "s");
    }

    painter.drawText(fieldTitleBox, title.str().c_str(), centreAlignedText);

    painter.drawText(fieldDateBox, dateText.str().c_str(), centreAlignedText);

    painter.drawText(fieldDrawnByBox, drawnByInitials.c_str(), centreAlignedText);

    painter.drawImage(fieldSCSLogoBox, QImage(":/scs_logo.png"));

    std::stringstream details;

    details << "SCREENING CONSULTANCY & SUPPLIES, ";
    details << "42 SOMERS ROAD, RUGBY, WARWICKSHIRE, CV227DH, ";
    details << "TEL: 01788 553300 EMAIL: SALES@SCSRUGBY.CO.UK" << std::endl;
    details << "PROPRIETARY AND CONFIDENTIAL. THE INFORMATION CONTAINED IN THIS ";
    details << "DRAWING IS THE SOLE PROPERTY OF SCS LTD. ";
    details << "ANY REPRODUCTION IN PART OR AS A WHOLE WITHOUT THE WRITTEN PERMISSION ";
    details << "OF SCS LTD. IS PROHIBITED.";

    QFont font = painter.font();
    font.setPointSize(5);
    painter.setFont(font);

    painter.drawText(fieldSCSDetailsBox, details.str().c_str(), centreAlignedText);

    font.setPointSize(18);
    painter.setFont(font);
    painter.drawText(fieldDrawingNumberBox, drawing.drawingNumber().c_str(), centreAlignedText);

    painter.restore();
}

void DrawingPDFWriter::drawRubberScreenCloth(QPainter &painter, QRectF drawingRegion, const Drawing &drawing) const {
    const double maxDimensionPercentage = 0.8;
    const double defaultHorizontalBarSize = 45;
    const double dimensionSpacingHeight = 0.7, dimensionBarHeight = 0.8, dimensionInnerSpacingHeight = 0.9;
    const double dimensionHorizontalLapOffset = 0.3, dimensionVerticalLapOffset = 0.3;
    const double shortDimensionLineSize = 0.02, longDimensionLineSize = 0.04;
    const double mainDimensionLineSize = 0.06;

    QPen dashDotPen = QPen(QBrush(Qt::black), 1, Qt::DashDotLine);
    dashDotPen.setDashPattern({96, 48, 4, 48});

    QPen dashPen = QPen(QBrush(Qt::black), 1, Qt::DashLine);
    dashPen.setDashPattern({96, 48});

    QPen smallDashPen = QPen(QBrush(Qt::black), 1, Qt::DashLine);
    smallDashPen.setDashPattern({48, 24});

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
            QPointF(matBoundingRegion.left(), (0.5 * (1.0 - pLength)) * regionHeight + drawingRegion.top()),
            QPointF(matBoundingRegion.left(),
                    (0.5 * (1.0 - pLength) - mainDimensionLineSize) * regionHeight + drawingRegion.top()));
    QLineF rightWidthExtender(
            QPointF(matBoundingRegion.right(), (0.5 * (1.0 - pLength)) * regionHeight + drawingRegion.top()),
            QPointF(matBoundingRegion.right(),
                    (0.5 * (1.0 - pLength) - mainDimensionLineSize) * regionHeight + drawingRegion.top()));

    painter.setPen(dashPen);
    painter.drawLine(leftWidthExtender);
    painter.drawLine(rightWidthExtender);
    painter.setPen(Qt::black);

    drawArrow(painter, leftWidthExtender.center(), rightWidthExtender.center(), to_str(drawing.width()).c_str(),
              DOUBLE_HEADED);

    QLineF topLengthExtender(
            QPointF((0.5 * (1.0 - pWidth)) * regionWidth + drawingRegion.left(), matBoundingRegion.top()),
            QPointF((0.5 * (1.0 - pWidth) - mainDimensionLineSize) * regionWidth + drawingRegion.left(),
                    matBoundingRegion.top()));
    QLineF bottomLengthExtender(
            QPointF((0.5 * (1.0 - pWidth)) * regionWidth + drawingRegion.left(), matBoundingRegion.bottom()),
            QPointF((0.5 * (1.0 - pWidth) - mainDimensionLineSize) * regionWidth + drawingRegion.left(),
                    matBoundingRegion.bottom()));

    painter.setPen(dashPen);
    painter.drawLine(topLengthExtender);
    painter.drawLine(bottomLengthExtender);
    painter.setPen(Qt::black);

    drawArrow(painter, topLengthExtender.center(), bottomLengthExtender.center(), to_str(drawing.length()).c_str(),
              DOUBLE_HEADED, true);

    if (leftLap.has_value()) {
        QRectF leftLapRegion;
        leftLapRegion.setLeft(
                (0.5 * (1.0 - pWidth)) * regionWidth + drawingRegion.left()
        );
        leftLapRegion.setTopRight(matBoundingRegion.topLeft());
        leftLapRegion.setBottomRight(matBoundingRegion.bottomLeft());

        painter.drawRect(leftLapRegion);

        QPointF lapLeft(leftLapRegion.left(),
                        matBoundingRegion.height() * dimensionVerticalLapOffset + matBoundingRegion.top());
        QPointF lapRight(leftLapRegion.right(),
                         matBoundingRegion.height() * dimensionVerticalLapOffset + matBoundingRegion.top());

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

        QPointF lapLeft(rightLapRegion.left(),
                        matBoundingRegion.height() * dimensionVerticalLapOffset + matBoundingRegion.top());
        QPointF lapRight(rightLapRegion.right(),
                         matBoundingRegion.height() * dimensionVerticalLapOffset + matBoundingRegion.top());

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

        QPointF lapTop(matBoundingRegion.width() * dimensionHorizontalLapOffset + matBoundingRegion.left(),
                       topLapRegion.top());
        QPointF lapBottom(matBoundingRegion.width() * dimensionHorizontalLapOffset + matBoundingRegion.left(),
                          topLapRegion.bottom());

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

        QPointF lapTop(matBoundingRegion.width() * dimensionHorizontalLapOffset + matBoundingRegion.left(),
                       bottomLapRegion.top());
        QPointF lapBottom(matBoundingRegion.width() * dimensionHorizontalLapOffset + matBoundingRegion.left(),
                          bottomLapRegion.bottom());

        std::string lapString = to_str(bottomLap->width) + "mm";
        if (bottomLap->attachmentType == LapAttachment::BONDED) {
            lapString += " Bonded";
        }

        drawArrow(painter, QPointF(lapTop.x(), lapTop.y() - shortDimensionLineSize * regionHeight), lapTop);
        drawArrow(painter, QPointF(lapBottom.x(), lapBottom.y() + longDimensionLineSize * regionHeight), lapBottom,
                  lapString.c_str());
    }

    std::vector<double> apertureRegionEndpoints;
    apertureRegionEndpoints.push_back(drawing.leftMargin());

    double currentMatPosition = 0;

    painter.setPen(dashDotPen);

    for (unsigned bar = 0; bar < drawing.numberOfBars(); bar++) {
        currentMatPosition += drawing.barSpacing(bar);

        QLineF barCentreDividerLine;
        barCentreDividerLine.setP1(QPointF(
                (currentMatPosition / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                (defaultHorizontalBarSize / lengthDim.rCentre) * matBoundingRegion.height() + matBoundingRegion.top()
        ));
        barCentreDividerLine.setP2(QPointF(
                (currentMatPosition / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                (1 - (defaultHorizontalBarSize / lengthDim.rCentre)) * matBoundingRegion.height() +
                matBoundingRegion.top()
        ));

        painter.drawLine(barCentreDividerLine);

        double barWidth = drawing.barWidth(bar + 1);
        apertureRegionEndpoints.push_back(currentMatPosition - barWidth / 2);
        apertureRegionEndpoints.push_back(currentMatPosition + barWidth / 2);
    }

    apertureRegionEndpoints.push_back(widthDim.rCentre - drawing.rightMargin());

    painter.setPen(Qt::black);

    double apertureRegionTop =
            (defaultHorizontalBarSize / lengthDim.rCentre) * matBoundingRegion.height() + matBoundingRegion.top(),
            apertureRegionBottom =
            (1 - (defaultHorizontalBarSize / lengthDim.rCentre)) * matBoundingRegion.height() + matBoundingRegion.top();
    double apertureRegionHeight = apertureRegionBottom - apertureRegionTop;

    for (unsigned apertureRegion = 0; apertureRegion < apertureRegionEndpoints.size() / 2; apertureRegion++) {
        double start = apertureRegionEndpoints[2 * apertureRegion];
        double end = apertureRegionEndpoints[2 * apertureRegion + 1];

        QRectF region;
        region.setTopLeft(QPointF(
                (start / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                apertureRegionTop
        ));
        region.setBottomRight(QPointF(
                (end / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                apertureRegionBottom
        ));

        painter.drawRect(region);
    }

    double spacingPosition = 0;

    for (unsigned spacingDimension = 0; spacingDimension <= drawing.numberOfBars(); spacingDimension++) {
        double nextSpacingPosition = spacingPosition + drawing.barSpacing(spacingDimension);

        if (drawing.numberOfBars() != 0) {
            drawArrow(
                    painter,
                    QPointF(
                            (spacingPosition / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                            matBoundingRegion.height() * dimensionSpacingHeight + matBoundingRegion.top()),
                    QPointF(
                            (nextSpacingPosition / widthDim.rCentre) * matBoundingRegion.width() +
                            matBoundingRegion.left(),
                            matBoundingRegion.height() * dimensionSpacingHeight + matBoundingRegion.top()),
                    to_str(drawing.barSpacing(spacingDimension)).c_str(), DOUBLE_HEADED
            );
        }

        double leftInnerSpacing = apertureRegionEndpoints[2 *
                                                          spacingDimension], rightInnerSpacing = apertureRegionEndpoints[
                2 * spacingDimension + 1];

        drawArrow(
                painter,
                QPointF(
                        (leftInnerSpacing / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                        apertureRegionHeight * dimensionInnerSpacingHeight + apertureRegionTop),
                QPointF(
                        (rightInnerSpacing / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                        apertureRegionHeight * dimensionInnerSpacingHeight + apertureRegionTop),
                to_str(rightInnerSpacing - leftInnerSpacing).c_str(), DOUBLE_HEADED
        );

        spacingPosition = nextSpacingPosition;
    }

    std::vector<double> barEndpoints = apertureRegionEndpoints;
    barEndpoints.insert(barEndpoints.begin(), 0);
    barEndpoints.push_back(widthDim.rCentre);

    for (unsigned bar = 0; bar < drawing.numberOfBars() + 2; bar++) {
        QPointF barLeft(
                (barEndpoints[2 * bar] / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                apertureRegionHeight * dimensionBarHeight + apertureRegionTop);
        QPointF barRight(
                (barEndpoints[2 * bar + 1] / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                apertureRegionHeight * dimensionBarHeight + apertureRegionTop);

        drawArrow(painter, QPointF(barLeft.x() - shortDimensionLineSize * regionWidth, barLeft.y()), barLeft);
        drawArrow(painter, QPointF(barRight.x() + longDimensionLineSize * regionWidth, barRight.y()), barRight,
                  to_str(drawing.barWidth(bar)).c_str(), SINGLE_HEADED, true);
    }

    painter.save();

    for (const Drawing::ImpactPad &pad : drawing.impactPads()) {
        QRectF padRegion(
                QPointF((pad.pos.x / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                        (pad.pos.y / lengthDim.rCentre) * matBoundingRegion.height() + matBoundingRegion.top()),
                QSizeF((pad.width / widthDim.rCentre) * matBoundingRegion.width(),
                       (pad.length / lengthDim.rCentre) * matBoundingRegion.height())
        );
        painter.setBrush(QColor(255, 255, 0, 127));
        painter.drawRect(padRegion);
    }

    painter.restore();

    for (const Drawing::CentreHole &hole : drawing.centreHoles()) {
        QRectF holeBounds = QRectF(
                QPointF(matBoundingRegion.left() +
                        ((hole.pos.x - hole.centreHoleShape.width / 2) / widthDim.rCentre) * matBoundingRegion.width(),
                        matBoundingRegion.top() +
                        ((hole.pos.y - hole.centreHoleShape.length / 2) / lengthDim.rCentre) *
                        matBoundingRegion.height()),
                QSizeF((hole.centreHoleShape.width / widthDim.rCentre) * matBoundingRegion.width(),
                       (hole.centreHoleShape.length / lengthDim.rCentre) * matBoundingRegion.height())
        );

        painter.setPen(Qt::red);

        if (!hole.centreHoleShape.rounded) {
            painter.drawRect(holeBounds);
            painter.setPen(smallDashPen);
            painter.drawLine(QPointF(holeBounds.center().x(), holeBounds.top()),
                             QPointF(holeBounds.center().x(), holeBounds.bottom()));
            painter.drawLine(QPointF(holeBounds.left(), holeBounds.center().y()),
                             QPointF(holeBounds.right(), holeBounds.center().y()));
        } else {
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath roundedRectPath;
            double radius =
                    (std::min(hole.centreHoleShape.width, hole.centreHoleShape.length) / 2.0) / widthDim.rCentre *
                    matBoundingRegion.width();
            roundedRectPath.addRoundedRect(holeBounds, radius, radius);
            painter.drawPath(roundedRectPath);
            painter.setPen(smallDashPen);
            painter.drawLine(QPointF(holeBounds.center().x(), holeBounds.top()),
                             QPointF(holeBounds.center().x(), holeBounds.bottom()));
            painter.drawLine(QPointF(holeBounds.left(), holeBounds.center().y()),
                             QPointF(holeBounds.right(), holeBounds.center().y()));
        }
    }

    painter.restore();

    double root2 = std::sqrt(2);

    for (const Drawing::Deflector &deflector : drawing.deflectors()) {
        QPainterPath deflectorBounds;

        deflectorBounds.moveTo(
                matBoundingRegion.left() + (deflector.pos.x / widthDim.rCentre) * matBoundingRegion.width(),
                matBoundingRegion.top() +
                (deflector.pos.y - deflector.size / root2) / lengthDim.rCentre * matBoundingRegion.height());
        deflectorBounds.lineTo(matBoundingRegion.left() +
                               (deflector.pos.x + deflector.size / root2) / widthDim.rCentre *
                               matBoundingRegion.width(), matBoundingRegion.top() +
                                                          (deflector.pos.y / lengthDim.rCentre) *
                                                          matBoundingRegion.height());
        deflectorBounds.lineTo(
                matBoundingRegion.left() + (deflector.pos.x / widthDim.rCentre) * matBoundingRegion.width(),
                matBoundingRegion.top() +
                (deflector.pos.y + deflector.size / root2) / lengthDim.rCentre * matBoundingRegion.height());
        deflectorBounds.lineTo(matBoundingRegion.left() +
                               (deflector.pos.x - deflector.size / root2) / widthDim.rCentre *
                               matBoundingRegion.width(), matBoundingRegion.top() +
                                                          (deflector.pos.y / lengthDim.rCentre) *
                                                          matBoundingRegion.height());
        deflectorBounds.lineTo(
                matBoundingRegion.left() + (deflector.pos.x / widthDim.rCentre) * matBoundingRegion.width(),
                matBoundingRegion.top() +
                (deflector.pos.y - deflector.size / root2) / lengthDim.rCentre * matBoundingRegion.height());

        painter.setPen(Qt::red);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawPath(deflectorBounds);
    }

    painter.restore();

    for (const Drawing::Divertor &divertor : drawing.divertors()) {
        QPainterPath divertorBounds;

        switch (divertor.side) {
            case Drawing::LEFT:
                divertorBounds.moveTo(matBoundingRegion.left(), matBoundingRegion.top() +
                                                                (divertor.verticalPosition -
                                                                 divertor.width / (2 * root2)) / lengthDim.rCentre *
                                                                matBoundingRegion.height());
                divertorBounds.lineTo(matBoundingRegion.left() +
                                      (divertor.length / root2) / widthDim.rCentre * matBoundingRegion.width(),
                                      matBoundingRegion.top() +
                                      (divertor.verticalPosition - divertor.width / (2 * root2) +
                                       divertor.length / root2) / lengthDim.rCentre * matBoundingRegion.height());
                divertorBounds.lineTo(matBoundingRegion.left() +
                                      (divertor.length / root2) / widthDim.rCentre * matBoundingRegion.width(),
                                      matBoundingRegion.top() +
                                      (divertor.verticalPosition + divertor.width / (2 * root2) +
                                       divertor.length / root2) / lengthDim.rCentre * matBoundingRegion.height());
                divertorBounds.lineTo(matBoundingRegion.left(), matBoundingRegion.top() +
                                                                (divertor.verticalPosition +
                                                                 divertor.width / (2 * root2)) / lengthDim.rCentre *
                                                                matBoundingRegion.height());
                divertorBounds.lineTo(matBoundingRegion.left(), matBoundingRegion.top() +
                                                                (divertor.verticalPosition -
                                                                 divertor.width / (2 * root2)) / lengthDim.rCentre *
                                                                matBoundingRegion.height());
                break;
            case Drawing::RIGHT:
                divertorBounds.moveTo(matBoundingRegion.right(), matBoundingRegion.top() +
                                                                 (divertor.verticalPosition -
                                                                  divertor.width / (2 * root2)) / lengthDim.rCentre *
                                                                 matBoundingRegion.height());
                divertorBounds.lineTo(matBoundingRegion.right() -
                                      (divertor.length / root2) / widthDim.rCentre * matBoundingRegion.width(),
                                      matBoundingRegion.top() +
                                      (divertor.verticalPosition - divertor.width / (2 * root2) +
                                       divertor.length / root2) / lengthDim.rCentre * matBoundingRegion.height());
                divertorBounds.lineTo(matBoundingRegion.right() -
                                      (divertor.length / root2) / widthDim.rCentre * matBoundingRegion.width(),
                                      matBoundingRegion.top() +
                                      (divertor.verticalPosition + divertor.width / (2 * root2) +
                                       divertor.length / root2) / lengthDim.rCentre * matBoundingRegion.height());
                divertorBounds.lineTo(matBoundingRegion.right(), matBoundingRegion.top() +
                                                                 (divertor.verticalPosition +
                                                                  divertor.width / (2 * root2)) / lengthDim.rCentre *
                                                                 matBoundingRegion.height());
                divertorBounds.lineTo(matBoundingRegion.right(), matBoundingRegion.top() +
                                                                 (divertor.verticalPosition -
                                                                  divertor.width / (2 * root2)) / lengthDim.rCentre *
                                                                 matBoundingRegion.height());
                break;
        }

        painter.setPen(Qt::red);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawPath(divertorBounds);
    }

    painter.restore();
}

void DrawingPDFWriter::drawArrow(QPainter &painter, QPointF from, QPointF to, const QString &label, ArrowMode mode,
                                 bool flipLabel, QPen tailPen, QPen headPen, double headSize, double angle) {
    painter.save();
    painter.setPen(tailPen);

    QLineF tail(from, to);

    painter.drawLine(tail);

    if (!label.isEmpty()) {
        const double padding = 10;

        QTextOption centreAlignedText;
        centreAlignedText.setAlignment(Qt::AlignCenter);

        QRectF labelBounds = QFontMetricsF(painter.font()).boundingRect(label).adjusted(-padding, -padding, padding,
                                                                                        padding);
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