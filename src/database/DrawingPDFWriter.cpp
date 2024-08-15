//
// Created by matthew on 27/07/2020.
//

#include "../../include/database/DrawingPDFWriter.h"

DrawingPDFWriter::DrawingPDFWriter() {

}

bool DrawingPDFWriter::createPDF(const std::filesystem::path &pdfFilePath, const Drawing &drawing,
                                 const std::string &drawnByInitials) const {

    QPdfWriter writer(pdfFilePath.string().c_str());

    writer.setPageSize(QPageSize::A4);
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setPageMargins(QMargins(10, 5, 10, 5));

    QSvgRenderer svgTemplateRenderer(QString(":/drawing_pdf_base_template.svg"));

    QPainter painter;

    if (!painter.begin(&writer)) {
        return false;
    }
    QFontDatabase::addApplicationFont(":/fonts/Helvetica.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Arial.ttf");

    QFont font("Roboto, Arial, sans-serif");
    font.setPointSize(150); // Adjust as needed
    painter.setFont(font);
    //bool ok;
    //QFont font = QFontDialog::getFont(&ok);
    //if (ok) {
    //    painter.setFont(font);
    //}

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    drawStandardTemplate(painter, svgTemplateRenderer);

    drawTextDetails(painter, svgTemplateRenderer, drawing, drawnByInitials);

    drawRubberScreenCloth(painter, svgTemplateRenderer.boundsOnElement("drawing_target_region"), drawing);

    return true;
}

void DrawingPDFWriter::drawStandardTemplate(QPainter &painter, QSvgRenderer &svgTemplateRenderer) const {
    QRect viewport = painter.viewport();

    svgTemplateRenderer.render(&painter);

    painter.setPen(QPen(Qt::black, 5));

    painter.drawRect(viewport);

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

    QRect viewport = painter.viewport();
    painter.save();

    const double horizontalOffset = 50.0, verticalOffset = 40.0;

    QTextOption leftAlignedText;
    leftAlignedText.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QTextOption centreAlignedText;
    centreAlignedText.setAlignment(Qt::AlignCenter);

    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);

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

    if (drawing.product().productName != "Rubber Modules and Panels") {
        labelText = "Side Irons";

        std::stringstream sideIronText;
        SideIron leftSideIron = drawing.sideIron(Drawing::LEFT), rightSideIron = drawing.sideIron(Drawing::RIGHT);
        sideIronText << leftSideIron.length << "mm ";
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

        if (drawing.sideIronInverted(Drawing::LEFT)) {
            sideIronText << "Inverted ";
        }

        if (drawing.sideIronCutDown(Drawing::LEFT)) {
            sideIronText << "Cut Down ";
        }

        if (leftSideIron.type != SideIronType::None) {
            sideIronText << leftSideIron.drawingNumber << " ";
        }

        if (leftSideIron.handle() != rightSideIron.handle()) {
            sideIronText << ",\n";
            sideIronText << rightSideIron.length << "mm ";
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

            if (drawing.sideIronInverted(Drawing::RIGHT)) {
                sideIronText << "Inverted ";
            }

            if (drawing.sideIronCutDown(Drawing::RIGHT)) {
                sideIronText << "Cut Down ";
            }

            if (rightSideIron.type != SideIronType::None) {
                sideIronText << rightSideIron.drawingNumber << " ";
            }
        }

        fieldText = sideIronText.str().c_str();
        drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
            fieldText, fieldWidth, horizontalOffset, verticalOffset);

        labelText = "Overlaps";

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
                    lapString << " (" << leftOverlap->material().materialName << " " << leftOverlap->material().materialName << "mm)";
                }
                lapStrings.push_back(lapString.str());
            }
            if (rightOverlap.has_value()) {
                lapString.str(std::string());
                lapString << rightOverlap->width << "mm";
                if (rightOverlap->attachmentType == LapAttachment::BONDED) {
                    lapString << " (" << rightOverlap->material().materialName << " " << rightOverlap->material().thickness << "mm)";
                }
                lapStrings.push_back(lapString.str());
            }
            if (leftSidelap.has_value()) {
                lapString.str(std::string());
                lapString << leftSidelap->width << "mm";
                if (leftSidelap->attachmentType == LapAttachment::BONDED) {
                    lapString << " (" << leftSidelap->material().materialName << " " << leftSidelap->material().thickness << "mm)";
                }
                lapStrings.push_back(lapString.str());
            }
            if (rightSidelap.has_value()) {
                lapString.str(std::string());
                lapString << rightSidelap->width << "mm";
                if (rightSidelap->attachmentType == LapAttachment::BONDED) {
                    lapString << " (" << rightSidelap->material().materialName << " " << rightSidelap->material().thickness << "mm)";
                }
                lapStrings.push_back(lapString.str());
            }

            for (std::vector<std::string>::const_iterator it = lapStrings.begin(); it != lapStrings.end(); it++) {
                sideOverlapsText << *it;
                if (it != lapStrings.end() - 1) {
                    sideOverlapsText << ", ";
                }
            }
        }
        else {
            sideOverlapsText << "No";
        }

        fieldText = sideOverlapsText.str().c_str();
        drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
            fieldText, fieldWidth, horizontalOffset, verticalOffset);
    }
    std::string productName = drawing.product().productName;

    if (productName == "Rubber Screen Cloth") {
        if (drawing.material(Drawing::TOP)->thickness >= 15) {
            labelText = "Rebated";
            fieldText = drawing.rebated() ? "Yes" : "No";
            drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                              fieldText, fieldWidth, horizontalOffset, verticalOffset);

            if (drawing.hasBackingStrips()) {
                labelText = "Backing Strips";
                fieldText = drawing.backingStrip()->backingStripName().c_str();
                drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                                  fieldText, fieldWidth, horizontalOffset, verticalOffset);
            }
        } else {
            labelText = "Backing Strips";
            fieldText = drawing.hasBackingStrips() ? drawing.backingStrip()->backingStripName().c_str() : "No";
            drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                              fieldText, fieldWidth, horizontalOffset, verticalOffset);
        }
    } else if (productName == "Extraflex" || productName == "Polyflex") {
        if (drawing.hasBackingStrips()) {
            labelText = "Backing Strips";
            fieldText = drawing.backingStrip()->backingStripName().c_str();
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

    if (drawing.numberOfBlankSpaces()) {
        std::stringstream blankSpaceFieldText;
        std::vector<Drawing::BlankSpace> spaces = drawing.blankSpaces();
        for (unsigned i = 0; i < spaces.size(); i++) {
            Drawing::BlankSpace space = spaces[i];

            blankSpaceFieldText << space.width << "x" << space.length << " at " << "(" << space.pos.x << ", " << space.pos.y
                << ") ";

            if (i != spaces.size() - 1) {
                blankSpaceFieldText << ", ";
            }
        }

        labelText = "Blank Spaces(s)";
        fieldText = blankSpaceFieldText.str().c_str();
        drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
            fieldText, fieldWidth, horizontalOffset, verticalOffset);
    }

    if (drawing.numberOfExtraApertures()) {
        std::stringstream ExtraAperturesFieldText;
        std::vector<Drawing::ExtraAperture> apertures = drawing.extraApertures();
        for (unsigned i = 0; i < apertures.size(); i++) {
            Drawing::ExtraAperture aperture = apertures[i];

            ExtraAperturesFieldText << aperture.aperture().apertureName() << " Aperture across " << aperture.width << "x" << aperture.length << " at " << "(" << aperture.pos.x << ", " << aperture.pos.y
                << ") ";

            if (i != apertures.size() - 1) {
                ExtraAperturesFieldText << ", ";
            }
        }

        labelText = "Extra Aperture(s)";
        fieldText = ExtraAperturesFieldText.str().c_str();
        drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
            fieldText, fieldWidth, horizontalOffset, verticalOffset);
    }

    if (drawing.numberOfDamBars()) {
        std::stringstream damBarTextField;
        std::vector<Drawing::DamBar> bars = drawing.damBars();
        for (unsigned i = 0; i < bars.size(); i++) {
            Drawing::DamBar bar = bars[i];

            damBarTextField << bar.width << "x" << bar.length << " at " << "(" << bar.pos.x << ", " << bar.pos.y
                << ") ";
            damBarTextField << "Material " << bar.material().materialName;

            if (i != bars.size() - 1) {
                damBarTextField << ", ";
            }
        }

        labelText = "Dam Bar(s)";
        fieldText = damBarTextField.str().c_str();
        drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
            fieldText, fieldWidth, horizontalOffset, verticalOffset);
    }

    if (drawing.numberOfCentreHoles()) {
        std::stringstream centreHolesFieldText;
        const std::vector<Drawing::CentreHole> &holes = drawing.centreHoles();
        Aperture aperture = holes.front().aperture();
        std::string shape = DrawingComponentManager<ApertureShape>::findComponentByID(aperture.apertureShapeID).shape;

        centreHolesFieldText << aperture.apertureName();
        centreHolesFieldText << " at ";

        
        float total = 0;
        float oldY = 0;
        for (const Drawing::CentreHole &hole : holes) {
            if (hole.pos.y < oldY) {
                centreHolesFieldText << (drawing.length() - total) << "\n";
                total = 0;
            }
            oldY = hole.pos.y;
            centreHolesFieldText << hole.pos.y - total << "+";
            total = hole.pos.y;
        }
        centreHolesFieldText << (drawing.length() - total);

        labelText = "Centre Holes";
        fieldText = centreHolesFieldText.str().c_str();
        drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                          fieldText, fieldWidth, horizontalOffset, verticalOffset);
    }

    if (drawing.numberOfDeflectors()) {
        std::stringstream deflectorsFieldText;
        const std::vector<Drawing::Deflector> &deflectors = drawing.deflectors();

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
        const std::vector<Drawing::Divertor> &divertors = drawing.divertors();

        Material& mat = divertors.front().material();

        divertorsFieldText << divertors.front().width << "x" << divertors.front().length << " " << mat.thickness
            << "mm " << mat.materialName << " at ";

        std::vector<float> leftYPositions, rightYPositions;

        for (const Drawing::Divertor& divertor : divertors) {
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
        }
        else {
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
        }
        else if (leftYPositions.empty()) {
            float lastY = 0;
            for (float y : rightYPositions) {
                divertorsFieldText << (y - lastY);
                lastY = y;
                if (y != rightYPositions.back()) {
                    divertorsFieldText << "+";
                }
            }
        }
        else if (rightYPositions.empty()) {
            float lastY = 0;
            for (float y : leftYPositions) {
                divertorsFieldText << (y - lastY);
                lastY = y;
                if (y != leftYPositions.back()) {
                    divertorsFieldText << "+";
                }
            }
        }
        else {
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
    if (drawing.sideIronFixedEnd() || drawing.sideIronFeedEnd().has_value()) {
        std::stringstream markingRefText;
        labelText = "Marking Reference";
        if (drawing.sideIronFixedEnd()) {
            markingRefText << "FX - Fixed End";
            if (drawing.sideIronFeedEnd().has_value()) {
                markingRefText << "\n";
            }
        }
        if (drawing.sideIronFeedEnd().has_value()) {
            markingRefText << "FE - Feed End";
        }

        fieldText = markingRefText.str().c_str();
        drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
            fieldText, fieldWidth, horizontalOffset, verticalOffset);
    }

    labelText = "Notes";
    fieldText = drawing.notes().c_str();
    drawLabelAndField(painter, generalDetailsBox.left(), currentVPos, labelText, labelWidth,
                      fieldText, fieldWidth, horizontalOffset, verticalOffset);

    QRectF
            fieldDrawingNumberBox = DrawingPDFWriter::adjustRect(svgTemplateRenderer.boundsOnElement("field_drawing_number")),
            fieldTitleBox = DrawingPDFWriter::adjustRect(svgTemplateRenderer.boundsOnElement("field_title")),
            fieldDateBox = DrawingPDFWriter::adjustRect(svgTemplateRenderer.boundsOnElement("field_date")),
            fieldDrawnByBox = DrawingPDFWriter::adjustRect(svgTemplateRenderer.boundsOnElement("field_drawn_by")),
            fieldSCSLogoBox = DrawingPDFWriter::adjustRect(svgTemplateRenderer.boundsOnElement("field_scs_logo")),
            fieldSCSDetailsBox = DrawingPDFWriter::adjustRect(svgTemplateRenderer.boundsOnElement("field_scs_details"));

    std::stringstream title;
    title << drawing.product().productName;

    if (drawing.product().productName != "Bivitec" &&
        drawing.product().productName != "Flip Flow" &&
        drawing.product().productName != "Rubber Modules and Panels") {
        title << " " + to_str(drawing.numberOfBars()) + " Support Bar" + (drawing.numberOfBars() == 1 ? "" : "s");
    }

    painter.drawText(fieldTitleBox, title.str().c_str(), centreAlignedText);

    painter.drawText(fieldDateBox, dateText.str().c_str(), centreAlignedText);

    painter.drawText(fieldDrawnByBox, drawnByInitials.c_str(), centreAlignedText);

    fieldSCSLogoBox.adjust(30, 30, 55, 30);
    painter.drawImage(fieldSCSLogoBox, QImage(":/scs_logo.png"));

    std::stringstream details;

    details << "SCREENING CONSULTANCY & SUPPLIES, ";
    details << "42 SOMERS ROAD, RUGBY, WARWICKSHIRE, CV227DH, ";
    details << "TEL: 01788 553300 EMAIL: SALES@SCSRUGBY.CO.UK" << std::endl;
    details << "PROPRIETARY AND CONFIDENTIAL. THE INFORMATION CONTAINED IN THIS ";
    details << "DRAWING IS THE SOLE PROPERTY OF SCS LTD. ";
    details << "ANY REPRODUCTION IN PART OR AS A WHOLE WITHOUT THE WRITTEN PERMISSION ";
    details << "OF SCS LTD. IS PROHIBITED.";

    font.setPointSize(5);
    painter.setFont(font);

    fieldSCSDetailsBox.adjust(25, 25, 35, 20);
    //painter.fillRect(fieldSCSDetailsBox, Qt::GlobalColor::yellow);
    painter.drawText(fieldSCSDetailsBox, details.str().c_str(), centreAlignedText);

    font.setPointSize(18);
    painter.setFont(font);
    fieldDrawingNumberBox.adjust(20, 65, 40, 60);
    //painter.fillRect(fieldDrawingNumberBox, Qt::GlobalColor::blue);
    painter.drawText(fieldDrawingNumberBox, drawing.drawingNumber().c_str(), centreAlignedText);

    painter.restore();
}

void DrawingPDFWriter::drawRubberScreenCloth(QPainter& painter, QRectF drawingRegion, const Drawing& drawing) const {
    const double maxDimensionPercentage = 0.9;
    const double defaultHorizontalBarSize = 45;
    const double dimensionSpacingHeight = 0.7, dimensionBarHeight = 0.8, dimensionInnerSpacingHeight = 0.9;
    const double dimensionHorizontalLapOffset = 0.3, dimensionVerticalLapOffset = 0.3;
    const double shortDimensionLineSize = 0.02, longDimensionLineSize = 0.04;
    const double mainDimensionLineSize = 0.06;

    QPen dashDotPen = QPen(QBrush(Qt::black), 1, Qt::DashDotLine);
    dashDotPen.setDashPattern({ 96, 48, 4, 48 });

    QPen dashPen = QPen(QBrush(Qt::black), 1, Qt::DashLine);
    dashPen.setDashPattern({ 96, 48 });

    QPen smallDashPen = QPen(QBrush(Qt::black), 1, Qt::DashLine);
    smallDashPen.setDashPattern({ 48, 24 });

    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

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
    }
    else {
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

    if (drawing.sideIron(Drawing::LEFT).type == SideIronType::B || drawing.sideIron(Drawing::RIGHT).type == SideIronType::B) {
        drawArrow(painter, leftWidthExtender.center(), rightWidthExtender.center(),
            (to_str(drawing.width() + 60 * ((drawing.sideIron(Drawing::LEFT).type == SideIronType::B)
                + (drawing.sideIron(Drawing::RIGHT).type == SideIronType::B))) + " o/h\n" + to_str(drawing.width())).c_str(),
            DOUBLE_HEADED, false, QPen(), QPen(), 50.0, 30.0, 2);
    }
    else {
        drawArrow(painter, leftWidthExtender.center(), rightWidthExtender.center(), (to_str(drawing.width())).c_str(),
            DOUBLE_HEADED);
    }

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
    bool needed = false;
    if (drawing.sideIronFixedEnd(Drawing::LEFT).has_value() || drawing.sideIronFixedEnd(Drawing::RIGHT).has_value()) {
        needed = true;
        std::stringstream leftSideIronLowerText, rightSideIronLowerText;

        
        if (drawing.sideIronHookOrientation(Drawing::LEFT).has_value()) {
            switch (drawing.sideIronHookOrientation(Drawing::LEFT).value()) {
            case Drawing::HOOK_UP:
                leftSideIronLowerText << "Hook Up ";
                break;
            case Drawing::HOOK_DOWN:
                leftSideIronLowerText << "Hook Down ";
                break;
            }
        }
        
        if (*drawing.sideIronFixedEnd(Drawing::LEFT) == Drawing::FIXED_END) {
            leftSideIronLowerText << "FX";
            if (*drawing.sideIronFeedEnd() == Drawing::LEFT) {
                leftSideIronLowerText << ";FE";
            }
        }
        else if (*drawing.sideIronFeedEnd() == Drawing::LEFT) {
            leftSideIronLowerText << "FE";
        }

        QRectF leftSideIronLowerLabel = QFontMetricsF(painter.font()).boundingRect(leftSideIronLowerText.str().c_str());
        leftSideIronLowerLabel.moveBottomLeft(drawingRegion.bottomLeft());
        leftSideIronLowerLabel.adjust((bottomLengthExtender.center() - leftSideIronLowerLabel.center()).x(), QFontMetricsF(painter.font()).height(),
            (bottomLengthExtender.center() - leftSideIronLowerLabel.center()).x(), QFontMetricsF(painter.font()).height());

        painter.drawText(leftSideIronLowerLabel, Qt::AlignHCenter | Qt::AlignVCenter, leftSideIronLowerText.str().c_str());

        if (drawing.sideIronHookOrientation(Drawing::RIGHT).has_value()) {
            switch (drawing.sideIronHookOrientation(Drawing::RIGHT).value()) {
            case Drawing::HOOK_UP:
                rightSideIronLowerText << "Hook Up ";
                break;
            case Drawing::HOOK_DOWN:
                rightSideIronLowerText << "Hook Down ";
                break;
            }
        }
        
        if (*drawing.sideIronFixedEnd(Drawing::RIGHT) == Drawing::FIXED_END) {
            rightSideIronLowerText << "FX";
            if (*drawing.sideIronFeedEnd() == Drawing::RIGHT) {
                rightSideIronLowerText << ";FE";
            }
        }
        else if (*drawing.sideIronFeedEnd() == Drawing::RIGHT) {
            rightSideIronLowerText << "FE";
        }
        

        QRectF rightSideIronLowerLabel = QFontMetricsF(painter.font()).boundingRect(rightSideIronLowerText.str().c_str());
        rightSideIronLowerLabel.moveBottomRight(drawingRegion.bottomRight());
        rightSideIronLowerLabel.adjust(-(rightSideIronLowerLabel.center() - rightWidthExtender.center()).x(), QFontMetricsF(painter.font()).height(),
            -(rightSideIronLowerLabel.center() - rightWidthExtender.center()).x(), QFontMetricsF(painter.font()).height());


        painter.drawText(rightSideIronLowerLabel, Qt::AlignHCenter | Qt::AlignVCenter, rightSideIronLowerText.str().c_str());



    }

    if (drawing.sideIron(Drawing::LEFT).type != drawing.sideIron(Drawing::RIGHT).type ||
        drawing.sideIronInverted(Drawing::LEFT) != drawing.sideIronInverted(Drawing::RIGHT) ||
        drawing.sideIronCutDown(Drawing::LEFT) != drawing.sideIronCutDown(Drawing::RIGHT) || needed) {
        std::stringstream leftSideIronText, rightSideIronText;
        SideIron leftSideIron = drawing.sideIron(Drawing::LEFT), rightSideIron = drawing.sideIron(Drawing::RIGHT);

        switch (leftSideIron.type) {
            case SideIronType::A:
                leftSideIronText << "Type A ";
                break;
            case SideIronType::B:
                leftSideIronText << "Type B ";
                break;
            case SideIronType::C:
                leftSideIronText << "Type C ";
                break;
            case SideIronType::D:
                leftSideIronText << "Type D ";
                break;
            case SideIronType::E:
                leftSideIronText << "Type E ";
                break;
            case SideIronType::None:
                leftSideIronText << "None";
                break;
        }

        if (drawing.sideIronInverted(Drawing::LEFT)) {
            leftSideIronText << "Inverted ";
        }

        if (drawing.sideIronCutDown(Drawing::LEFT)) {
            leftSideIronText << "Cut Down ";
        }

        QRectF leftSideIronLabel = QFontMetricsF(painter.font()).boundingRect(leftSideIronText.str().c_str());
        leftSideIronLabel.moveBottomLeft(drawingRegion.bottomLeft());
        leftSideIronLabel.adjust((bottomLengthExtender.center() - leftSideIronLabel.center()).x(), 0,
                                 (bottomLengthExtender.center() - leftSideIronLabel.center()).x(), 0);

        painter.drawText(leftSideIronLabel, Qt::AlignHCenter | Qt::AlignVCenter, leftSideIronText.str().c_str());

        switch (rightSideIron.type) {
            case SideIronType::A:
                rightSideIronText << "Type A ";
                break;
            case SideIronType::B:
                rightSideIronText << "Type B ";
                break;
            case SideIronType::C:
                rightSideIronText << "Type C ";
                break;
            case SideIronType::D:
                rightSideIronText << "Type D ";
                break;
            case SideIronType::E:
                rightSideIronText << "Type E ";
                break;
            case SideIronType::None:
                rightSideIronText << "None";
                break;
        }

        if (drawing.sideIronInverted(Drawing::RIGHT)) {
            rightSideIronText << "Inverted ";
        }

        if (drawing.sideIronCutDown(Drawing::RIGHT)) {
            rightSideIronText << "Cut Down ";
        }

        QRectF rightSideIronLabel = QFontMetricsF(painter.font()).boundingRect(rightSideIronText.str().c_str());
        rightSideIronLabel.moveBottomRight(drawingRegion.bottomRight());
        rightSideIronLabel.adjust(-(rightSideIronLabel.center() - rightWidthExtender.center()).x(), 0,
                                  -(rightSideIronLabel.center() - rightWidthExtender.center()).x(), 0);
      

        painter.drawText(rightSideIronLabel, Qt::AlignHCenter | Qt::AlignVCenter, rightSideIronText.str().c_str());
    }
    
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
    int total = 0;
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
    float barWidthTotal = 0;
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
            if (spacingDimension != drawing.numberOfBars()) {
                barWidthTotal += drawing.barSpacing(spacingDimension);
                QString runningTotal = QString::number(((int)(barWidthTotal * 10)) / 10.0f);
                QPointF topCenter((nextSpacingPosition / widthDim.rCentre)* matBoundingRegion.width() +
                    matBoundingRegion.left(),
                    (0.5 * (1.0 + pLength))* regionHeight + drawingRegion.top() + longDimensionLineSize * regionHeight);
                drawText(painter, topCenter, runningTotal, 1, [](QRectF& rect, const QPointF& point) {
                    rect.moveCenter(point);
                    rect.moveTop(point.y());
                    });
            }

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

    for (const Drawing::BlankSpace& space : drawing.blankSpaces()) {
        QRectF padRegion(
            QPointF((space.pos.x / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                    (space.pos.y / lengthDim.rCentre) * matBoundingRegion.height() + matBoundingRegion.top()),
            QSizeF((space.width / widthDim.rCentre) * matBoundingRegion.width(),
                   (space.length / lengthDim.rCentre) * matBoundingRegion.height())
        );
        painter.setBrush(QColor(200, 200, 200, 127));
        painter.drawRect(padRegion);
    }

    for (const Drawing::ExtraAperture& aperture : drawing.extraApertures()) {
        QRectF apertureRegion(
            QPointF((aperture.pos.x / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                    (aperture.pos.y / lengthDim.rCentre) * matBoundingRegion.height() + matBoundingRegion.top()),
            QSizeF((aperture.width / widthDim.rCentre) * matBoundingRegion.width(),
                   (aperture.length / lengthDim.rCentre) * matBoundingRegion.height())
        );
        painter.setBrush(QColor(141, 221, 247, 127));
        painter.drawRect(apertureRegion);

        painter.setBrush(QBrush(QColor(0, 0, 0, 255)));
        painter.drawText(apertureRegion, Qt::AlignHCenter | Qt::AlignVCenter, aperture.aperture().apertureName().c_str());
    }
    for (const Drawing::DamBar& bar : drawing.damBars()) {
        QRectF padRegion(
            QPointF((bar.pos.x / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                (bar.pos.y / lengthDim.rCentre) * matBoundingRegion.height() + matBoundingRegion.top()),
            QSizeF((bar.width / widthDim.rCentre) * matBoundingRegion.width(),
                (bar.length / lengthDim.rCentre) * matBoundingRegion.height())
        );
        painter.setBrush(QColor(255, 50, 50, 127));
        painter.drawRect(padRegion);
    }

    painter.restore();

    for (const Drawing::CentreHole &hole : drawing.centreHoles()) {
        //QRectF holeBounds = QRectF(
        //        QPointF(matBoundingRegion.left() +
        //                ((hole.pos.x - DrawingComponentManager<Aperture>::findComponentByID(hole.apertureID).width / 2) / widthDim.rCentre) * matBoundingRegion.width(),
        //                matBoundingRegion.top() +
        //                ((hole.pos.y - DrawingComponentManager<Aperture>::findComponentByID(hole.apertureID).length / 2) / lengthDim.rCentre) *
        //                matBoundingRegion.height()),
        //        QSizeF((DrawingComponentManager<Aperture>::findComponentByID(hole.apertureID).width / widthDim.rCentre) * matBoundingRegion.width(),
        //               (DrawingComponentManager<Aperture>::findComponentByID(hole.apertureID).length / lengthDim.rCentre) * matBoundingRegion.height())
        //);

        //painter.setPen(Qt::red);

        //if (DrawingComponentManager<ApertureShape>::findComponentByID(DrawingComponentManager<Aperture>::findComponentByID(hole.apertureID).apertureShapeID).shape != "DIA") {
        //    painter.drawRect(holeBounds);
        //    painter.setPen(smallDashPen);
        //    painter.drawLine(QPointF(holeBounds.center().x(), holeBounds.top()),
        //                     QPointF(holeBounds.center().x(), holeBounds.bottom()));
        //    painter.drawLine(QPointF(holeBounds.left(), holeBounds.center().y()),
        //                     QPointF(holeBounds.right(), holeBounds.center().y()));
        //} else {
        //    painter.setRenderHint(QPainter::Antialiasing);
        //    QPainterPath roundedRectPath;
        //    double radius =
        //            (std::min(DrawingComponentManager<Aperture>::findComponentByID(hole.apertureID).width, DrawingComponentManager<Aperture>::findComponentByID(hole.apertureID).length) / 2.0) / widthDim.rCentre *
        //            matBoundingRegion.width();
        //    roundedRectPath.addRoundedRect(holeBounds, radius, radius);
        //    painter.drawPath(roundedRectPath);
        //    painter.setPen(smallDashPen);
        //    painter.drawLine(QPointF(holeBounds.center().x(), holeBounds.top()),
        //                     QPointF(holeBounds.center().x(), holeBounds.bottom()));
        //    painter.drawLine(QPointF(holeBounds.left(), holeBounds.center().y()),
        //                     QPointF(holeBounds.right(), holeBounds.center().y()));
        //}
        QRectF verticalHoleBounds = QRectF(
                QPointF(matBoundingRegion.left() +
                        ((hole.pos.x - hole.aperture().width / 2) / widthDim.rCentre) * matBoundingRegion.width(),
                        matBoundingRegion.top() +
                        ((hole.pos.y - hole.aperture().length / 2) / lengthDim.rCentre) * matBoundingRegion.height()),
                QSizeF((hole.aperture().width / widthDim.rCentre) * matBoundingRegion.width(),
                       (hole.aperture().length / lengthDim.rCentre) * matBoundingRegion.height())
        );
        QRectF horizontalHoleBounds = QRectF(
        QPointF(matBoundingRegion.left() +
                ((hole.pos.x - hole.aperture().length / 2) / lengthDim.rCentre) * matBoundingRegion.height(),
                matBoundingRegion.top() +
                ((hole.pos.y - hole.aperture().width / 2) / widthDim.rCentre) * matBoundingRegion.width()),
        QSizeF((hole.aperture().length / lengthDim.rCentre) * matBoundingRegion.height(),
               (hole.aperture().width / widthDim.rCentre) * matBoundingRegion.width())
        );

        painter.setPen(Qt::red);
        ApertureShape shape = DrawingComponentManager<ApertureShape>::findComponentByID(hole.aperture().apertureShapeID);
        if (shape.shape == "SQ" || shape.shape == "SL") {
            painter.drawRect(verticalHoleBounds);
            painter.setPen(Qt::DashDotLine);
            painter.drawLine(QPointF(verticalHoleBounds.center().x(), verticalHoleBounds.top()),
                              QPointF(verticalHoleBounds.center().x(), verticalHoleBounds.bottom()));
            painter.drawLine(QPointF(verticalHoleBounds.left(), verticalHoleBounds.center().y()),
                              QPointF(verticalHoleBounds.right(), verticalHoleBounds.center().y()));
        }
        else if (shape.shape == "ST") {
            painter.drawRect(horizontalHoleBounds);
            painter.setPen(Qt::DashDotLine);
            painter.drawLine(QPointF(horizontalHoleBounds.center().x(), horizontalHoleBounds.top()),
                              QPointF(horizontalHoleBounds.center().x(), horizontalHoleBounds.bottom()));
            painter.drawLine(QPointF(horizontalHoleBounds.left(), horizontalHoleBounds.center().y()),
                              QPointF(horizontalHoleBounds.right(), horizontalHoleBounds.center().y()));
        }
        else if (shape.shape == "DIA") {
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath roundedRectPath;
            double radius = (std::min(hole.aperture().width, hole.aperture().length) / 2.0) / widthDim.rCentre * matBoundingRegion.width();
            roundedRectPath.addRoundedRect(verticalHoleBounds, radius, radius);
            painter.drawPath(roundedRectPath);
            painter.setPen(Qt::DashDotLine);
            painter.drawLine(QPointF(verticalHoleBounds.center().x(), verticalHoleBounds.top()),
                              QPointF(verticalHoleBounds.center().x(), verticalHoleBounds.bottom()));
            painter.drawLine(QPointF(verticalHoleBounds.left(), verticalHoleBounds.center().y()),
                              QPointF(verticalHoleBounds.right(), verticalHoleBounds.center().y()));
        }
        else if (shape.shape == "RL") {
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath roundedRectPath;
            double radius = (std::min(hole.aperture().width, hole.aperture().length) / 2.0) / widthDim.rCentre * matBoundingRegion.width();
            roundedRectPath.addRoundedRect(verticalHoleBounds, radius, radius);
            painter.drawPath(roundedRectPath);
            painter.setPen(Qt::DashDotLine);
            painter.drawLine(QPointF(verticalHoleBounds.center().x(), verticalHoleBounds.top()),
                              QPointF(verticalHoleBounds.center().x(), verticalHoleBounds.bottom()));
            painter.drawLine(QPointF(verticalHoleBounds.left(), verticalHoleBounds.center().y()),
                              QPointF(verticalHoleBounds.right(), verticalHoleBounds.center().y()));
        }
        else if (shape.shape == "RT") {
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath roundedRectPath;
            double radius = (std::min(hole.aperture().width, hole.aperture().length) / 2.0) / widthDim.rCentre * matBoundingRegion.width();
            roundedRectPath.addRoundedRect(horizontalHoleBounds, radius, radius);
            painter.drawPath(roundedRectPath);
            painter.setPen(Qt::DashDotLine);
            painter.drawLine(QPointF(horizontalHoleBounds.center().x(), horizontalHoleBounds.top()),
                              QPointF(horizontalHoleBounds.center().x(), horizontalHoleBounds.bottom()));
            painter.drawLine(QPointF(horizontalHoleBounds.left(), horizontalHoleBounds.center().y()),
                              QPointF(horizontalHoleBounds.right(), horizontalHoleBounds.center().y()));
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
                                 bool flipLabel, QPen tailPen, QPen headPen, double headSize, double angle, unsigned lines) {
    painter.save();
    painter.setPen(tailPen);

    QLineF tail(from, to);

    painter.drawLine(tail);

    if (!label.isEmpty()) {
        const double padding = 10;

        QTextOption centreAlignedText;
        centreAlignedText.setAlignment(Qt::AlignCenter);
        if (lines == 1) {
            centreAlignedText.setWrapMode(QTextOption::NoWrap);
        }

        QRectF labelBounds = QFontMetricsF(painter.font()).boundingRect(label).adjusted(-padding, -padding, padding ,
                                                                                        padding);
        labelBounds.setHeight(labelBounds.height() * lines);
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

QRectF DrawingPDFWriter::adjustRect(const QRectF& rect) {
    return QRectF(rect);
    //return QRectF(rect.x() * 1.0504, rect.y() * 1.0143+ 30, rect.width() * 1.0504, rect.height() * 1.0143 + 30);
}

void DrawingPDFWriter::drawText(QPainter& painter, QPointF start, const QString& label, unsigned lines, std::function<void(QRectF&, const QPointF&)> move) {
	const double padding = 10;

	QTextOption centreAlignedText;
    centreAlignedText.setAlignment(Qt::AlignCenter);
	if (lines == 1) {
		centreAlignedText.setWrapMode(QTextOption::NoWrap);
	}

	QRectF labelBounds = QFontMetricsF(painter.font()).boundingRect(label).adjusted(-padding, -padding, padding ,
																					padding);
	labelBounds.setHeight(labelBounds.height() * lines);

    move(labelBounds, start);

    painter.drawText(labelBounds, label, centreAlignedText);
};