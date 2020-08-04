//
// Created by matthew on 14/07/2020.
//

#include "DrawingViewWidget.h"
#include "../build/ui_DrawingViewWidget.h"

DrawingViewWidget::DrawingViewWidget(const Drawing &drawing, QWidget *parent)
        : QWidget(parent), ui(new Ui::DrawingViewWidget()) {
    ui->setupUi(this);

    this->drawing = &drawing;

    this->pdfDocument = new QPdfDocument();

    updateFields();

    connect(ui->editDrawingButton, &QPushButton::clicked, [this]() {
        if (changeDrawingCallback) {
            changeDrawingCallback(AddDrawingPageWidget::EDIT_DRAWING);
        }
    });

    connect(ui->cloneDrawingButton, &QPushButton::clicked, [this]() {
        if (changeDrawingCallback) {
            changeDrawingCallback(AddDrawingPageWidget::CLONE_DRAWING);
        }
    });
}

DrawingViewWidget::~DrawingViewWidget() {

}

void DrawingViewWidget::updateFields() {
    ui->drawingNumberHeaderLabel->setText(drawing->drawingNumber().c_str());

    // Drawing Specs page
    ui->drawingNumberTextbox->setText(drawing->drawingNumber().c_str());
    ui->productTextbox->setText(drawing->product().productName.c_str());
    ui->widthTextbox->setText(to_str(drawing->width()).c_str());
    ui->lengthTextbox->setText(to_str(drawing->length()).c_str());

    if (drawing->loadWarning(Drawing::INVALID_APERTURE_DETECTED)) {
        QMessageBox::about(this, "Invalid Aperture Detected", "When loading this drawing from the database, the aperture "
            "was found to be invalid or broken.");
    } else {
        ui->apertureTextbox->setText(drawing->aperture().apertureName().c_str());
    }

    if (drawing->loadWarning(Drawing::INVALID_LAPS_DETECTED)) {
        QMessageBox::about(this, "Invalid Sidelaps/Overlaps Detected", "When loading this drawing from the database, "
                                                          "invalid sidelaps or overlaps were found.");
    }

    if (drawing->hasSidelaps()) {
        std::optional<Drawing::Lap> left = drawing->sidelap(Drawing::Side::LEFT);
        std::optional<Drawing::Lap> right = drawing->sidelap(Drawing::Side::RIGHT);

        if (left.has_value() && right.has_value()) {
            ui->sidelapsTextbox->setText((left->strAsSidelap() + ", " + right->strAsSidelap()).c_str());
        } else if (left.has_value()) {
            ui->sidelapsTextbox->setText(left->strAsSidelap().c_str());
        } else if (right.has_value()) {
            ui->sidelapsTextbox->setText(right->strAsSidelap().c_str());
        }
    } else {
        ui->sidelapsTextbox->setText("None");
    }
    if (drawing->hasOverlaps()) {
        std::optional<Drawing::Lap> left = drawing->overlap(Drawing::Side::LEFT);
        std::optional<Drawing::Lap> right = drawing->overlap(Drawing::Side::RIGHT);

        if (left.has_value() && right.has_value()) {
            ui->overlapsTextbox->setText((left->strAsOverlap() + ", " + right->strAsOverlap()).c_str());
        } else if (left.has_value()) {
            ui->overlapsTextbox->setText(left->strAsOverlap().c_str());
        } else if (right.has_value()) {
            ui->overlapsTextbox->setText(right->strAsOverlap().c_str());
        }
    } else {
        ui->overlapsTextbox->setText("None");
    }
    if (drawing->loadWarning(Drawing::MISSING_SIDE_IRONS_DETECTED)) {
        QMessageBox::about(this, "Missing Side Irons Detected", "The side irons are missing from this drawing");
    } else {
        ui->leftSideIronTextbox->setText((drawing->sideIron(Drawing::LEFT).sideIronStr() +
                                          (drawing->sideIronInverted(Drawing::LEFT) ? " (inverted)" : "")).c_str());
        ui->rightSideIronTextbox->setText((drawing->sideIron(Drawing::RIGHT).sideIronStr() +
                                          (drawing->sideIronInverted(Drawing::RIGHT) ? " (inverted)" : "")).c_str());
    }
    if (drawing->loadWarning(Drawing::MISSING_MATERIAL_DETECTED)) {
        QMessageBox::about(this, "Missing Material Detected", "The material(s) are missing from this drawing");
    } else {
        Material topLayer = drawing->material(Drawing::MaterialLayer::TOP).value();
        std::optional<Material> bottomLayer = drawing->material(Drawing::MaterialLayer::BOTTOM);
        ui->thicknessTextbox->setText((to_str(topLayer.thickness) +
                                       (bottomLayer.has_value() ? "+" + to_str(bottomLayer->thickness)
                                                                : "")).c_str());
        ui->topMaterialTextbox->setText(topLayer.material().c_str());
        if (bottomLayer.has_value()) {
            ui->bottomMaterialTextbox->setText(bottomLayer->material().c_str());
        } else {
            ui->bottomMaterialTextbox->setText("None");
        }
    }
    ui->notesTextbox->setText(drawing->notes().c_str());

    // Machine Template Page
    ui->machineTextbox->setText(drawing->machineTemplate().machine().machineName().c_str());
    ui->quantityOnDeckTextbox->setText(to_str(drawing->machineTemplate().quantityOnDeck).c_str());
    ui->positionTextbox->setText(drawing->machineTemplate().position.c_str());
    ui->deckTextbox->setText(drawing->machineTemplate().deck().deck.c_str());

    // Drawings page
    ui->drawingPDFSelectorInput->clear();
    ui->drawingPDFSelectorInput->addItem((drawing->drawingNumber() + " PDF").c_str(), drawing->hyperlink().c_str());

    for (const std::string &pdf : drawing->pressDrawingHyperlinks()) {
        ui->drawingPDFSelectorInput->addItem(pdf.c_str(), pdf.c_str());
    }

    std::filesystem::path pressDrawingLocation = punchProgramPathForDrawing(drawing->drawingNumber());

    if (std::filesystem::exists(pressDrawingLocation)) {
        ui->drawingPDFSelectorInput->addItem(pressDrawingLocation.string().c_str(), pressDrawingLocation.string().c_str());
    }

    if (!drawing->loadWarning(Drawing::MISSING_SIDE_IRONS_DETECTED)) {
        SideIron leftSideIron = drawing->sideIron(Drawing::LEFT),
            rightSideIron = drawing->sideIron(Drawing::RIGHT);
        ui->drawingPDFSelectorInput->addItem(("Left Side Iron: " + leftSideIron.drawingNumber).c_str(), leftSideIron.hyperlink.c_str());
        ui->drawingPDFSelectorInput->addItem(("Right Side Iron: " + rightSideIron.drawingNumber).c_str(), rightSideIron.hyperlink.c_str());
    }

    pdfViewer = new QPdfView(this);
    QPdfDocumentRenderOptions renderOptions;
    renderOptions.setRenderFlags(QPdf::RenderAnnotations);
    pdfViewer->setDocumentRenderOptions(renderOptions);

    connect(ui->drawingPDFSelectorInput, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        pdfDocument->load(ui->drawingPDFSelectorInput->itemData(index).toString());

        pdfViewer->setDocument(pdfDocument);
    });

    ui->drawingPDFsTabFormLayout->setWidget(1, QFormLayout::FieldRole, pdfViewer);
    pdfDocument->load(drawing->hyperlink().c_str());
    pdfViewer->setDocument(pdfDocument);
}

void DrawingViewWidget::setChangeDrawingCallback(const std::function<void(AddDrawingPageWidget::AddDrawingMode)> &callback) {
    changeDrawingCallback = callback;
}

std::filesystem::path DrawingViewWidget::punchProgramPathForDrawing(const std::string &drawingNumber) {
    std::filesystem::path path = PUNCH_PDF_LOCATION;
    std::string folder = "" + drawingNumber.at(0);

    if (std::isdigit(drawingNumber.at(1))) {
        folder = "1" + folder;
    } else {
        folder += drawingNumber.at(1);
    }

    path /= folder;
    path /= drawingNumber + ".pdf";

    return path;
}
