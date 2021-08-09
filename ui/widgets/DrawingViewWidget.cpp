//
// Created by matthew on 14/07/2020.
//

#include "DrawingViewWidget.h"
#include "../build/ui_DrawingViewWidget.h"

DrawingViewWidget::DrawingViewWidget(const Drawing &drawing, QWidget *parent)
        : QWidget(parent), ui(new Ui::DrawingViewWidget()) {
    ui->setupUi(this);

    ui->rebatedCheckbox->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->rebatedCheckbox->setFocusPolicy(Qt::NoFocus);
    ui->backingStripsCheckbox->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->backingStripsCheckbox->setFocusPolicy(Qt::NoFocus);

    this->drawing = &drawing;

    this->pdfDocument = new QPdfDocument();
    this->pdfViewer = new QPdfView(this);

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

    connect(ui->openPDFButton, &QPushButton::clicked, [this]() {
        std::filesystem::path pdfPath = ui->drawingPDFSelectorInput->currentData().toString().toStdString();
        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(pdfPath.generic_string().c_str()))) {
            QMessageBox::about(this, "Open PDF", ("The PDF could not be found. (" + pdfPath.generic_string() + ")").c_str());
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
    }
    else {
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
        }
        else if (left.has_value()) {
            ui->sidelapsTextbox->setText(left->strAsSidelap().c_str());
        }
        else if (right.has_value()) {
            ui->sidelapsTextbox->setText(right->strAsSidelap().c_str());
        }
    }
    else {
        ui->sidelapsTextbox->setText("None");
    }
    if (drawing->hasOverlaps()) {
        std::optional<Drawing::Lap> left = drawing->overlap(Drawing::Side::LEFT);
        std::optional<Drawing::Lap> right = drawing->overlap(Drawing::Side::RIGHT);

        if (left.has_value() && right.has_value()) {
            ui->overlapsTextbox->setText((left->strAsOverlap() + ", " + right->strAsOverlap()).c_str());
        }
        else if (left.has_value()) {
            ui->overlapsTextbox->setText(left->strAsOverlap().c_str());
        }
        else if (right.has_value()) {
            ui->overlapsTextbox->setText(right->strAsOverlap().c_str());
        }
    }
    else {
        ui->overlapsTextbox->setText("None");
    }
    if (drawing->loadWarning(Drawing::MISSING_SIDE_IRONS_DETECTED)) {
        QMessageBox::about(this, "Missing Side Irons Detected", "The side irons are missing from this drawing");
    }
    else {
        ui->leftSideIronTextbox->setText((drawing->sideIron(Drawing::LEFT).sideIronStr() +
            (drawing->sideIronInverted(Drawing::LEFT) ? " (inverted)" : "")).c_str());
        ui->rightSideIronTextbox->setText((drawing->sideIron(Drawing::RIGHT).sideIronStr() +
            (drawing->sideIronInverted(Drawing::RIGHT) ? " (inverted)" : "")).c_str());
    }
    if (drawing->loadWarning(Drawing::MISSING_MATERIAL_DETECTED)) {
        QMessageBox::about(this, "Missing Material Detected", "The material(s) are missing from this drawing");
    }
    else {
        Material topLayer = drawing->material(Drawing::MaterialLayer::TOP).value();
        std::optional<Material> bottomLayer = drawing->material(Drawing::MaterialLayer::BOTTOM);
        ui->thicknessTextbox->setText((to_str(topLayer.thickness) +
            (bottomLayer.has_value() ? "+" + to_str(bottomLayer->thickness)
                : "")).c_str());
        ui->topMaterialTextbox->setText(topLayer.material().c_str());
        if (bottomLayer.has_value()) {
            ui->bottomMaterialTextbox->setText(bottomLayer->material().c_str());
        }
        else {
            ui->bottomMaterialTextbox->setText("None");
        }
    }
    ui->rebatedCheckbox->setChecked(drawing->rebated());
    ui->backingStripsCheckbox->setChecked(drawing->hasBackingStrips());
    ui->notesTextbox->setText(drawing->notes().c_str());

    // Machine Template Page
    ui->machineTextbox->setText(drawing->machineTemplate().machine().machineName().c_str());
    ui->quantityOnDeckTextbox->setText(to_str(drawing->machineTemplate().quantityOnDeck).c_str());
    ui->positionTextbox->setText(drawing->machineTemplate().position.c_str());
    ui->deckTextbox->setText(drawing->machineTemplate().deck().deck.c_str());

    // Drawings page
    ui->drawingPDFSelectorInput->clear();
    ui->drawingPDFSelectorInput->addItem((drawing->drawingNumber() + " PDF").c_str(), drawing->hyperlink().c_str());

    for (const std::filesystem::path& pdf : drawing->pressDrawingHyperlinks()) {
        ui->drawingPDFSelectorInput->addItem(pdf.generic_string().c_str(), pdf.generic_string().c_str());
    }

    std::filesystem::path pressDrawingLocation = punchProgramPathForDrawing(drawing->drawingNumber());

    if (std::filesystem::exists(pressDrawingLocation)) {
        ui->drawingPDFSelectorInput->addItem("Punch Program (auto search)", pressDrawingLocation.generic_string().c_str());
    }

    if (!drawing->loadWarning(Drawing::MISSING_SIDE_IRONS_DETECTED)) {
        SideIron leftSideIron = drawing->sideIron(Drawing::LEFT),
            rightSideIron = drawing->sideIron(Drawing::RIGHT);
        ui->drawingPDFSelectorInput->addItem(("Left Side Iron: " + leftSideIron.drawingNumber).c_str(), leftSideIron.hyperlink.c_str());
        ui->drawingPDFSelectorInput->addItem(("Right Side Iron: " + rightSideIron.drawingNumber).c_str(), rightSideIron.hyperlink.c_str());
    }

    QPdfDocumentRenderOptions renderOptions;
    renderOptions.setRenderFlags(QPdf::RenderAnnotations);
    pdfViewer->setDocumentRenderOptions(renderOptions);

    connect(ui->drawingPDFSelectorInput, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        pdfDocument->load(ui->drawingPDFSelectorInput->itemData(index).toString());

        pdfViewer->setDocument(pdfDocument);
        });

    ui->drawingPDFsTabFormLayout->setWidget(1, QFormLayout::FieldRole, pdfViewer);
    pdfDocument->load(drawing->hyperlink().generic_string().c_str());
    pdfViewer->setDocument(pdfDocument);

    // Prices page
    std::vector<std::pair<std::string, float>> prices;

    std::vector<std::string> missing;

    // material pricing
    Material material = *drawing->material(Drawing::MaterialLayer::TOP);
    float materialPrice = std::numeric_limits<float>::max();
    for (std::vector<std::tuple<float, float, float, MaterialPricingType>>::iterator element = material.materialPrices.begin(); element != material.materialPrices.end(); element++) {
        // needs to account for pressing 2 700mm out of 1600mm instead of 1 700mm from 1070mm
        if (std::get<3>(*element) == MaterialPricingType::SHEET) {
            if (std::get<0>(*element) >= drawing->width() && std::get<1>(*element) && std::get<1>(*element) >= drawing->length()) {
                int tempPrice = std::get<2>(*element);
                if (tempPrice < materialPrice) {
                    materialPrice = tempPrice;
                }
            }
        }
        else if (std::get<3>(*element) == MaterialPricingType::RUNNING_M) {
            if (std::get<0>(*element) >= drawing->width()) {
                float tempPrice = std::get<2>(*element) * (drawing->length())/1000;
                if (tempPrice < materialPrice) {
                    materialPrice = tempPrice;
                }
            }
        }
        else if (std::get<3>(*element) == MaterialPricingType::SQUARE_M) {
            if (std::get<0>(*element) >= drawing->width()) {
                int tempPrice = std::get<2>(*element) * (drawing->length() * drawing->width()/1000)/1000;
                if (tempPrice < materialPrice) {
                    materialPrice = tempPrice;
                }
            }
        }
    }
    if (materialPrice == std::numeric_limits<float>::max()) {
        missing.push_back("Material");
        materialPrice = 0;
    }
    prices.push_back({ "Material Price: ", materialPrice });

    // Side Iron pricing
    float sideIronPrice = std::numeric_limits<float>::max();
    float sideIronScrewsPrice = std::numeric_limits<float>::max();
    float sideIronNutsPrice = std::numeric_limits<float>::max();
    for (unsigned index : DrawingComponentManager<SideIronPrice>::dataIndexSet()) {
        SideIronPrice& sideIronPriceObj = DrawingComponentManager<SideIronPrice>::findComponentByID(index);
        // assumes that 1 mat always has the same side iron set, and therefore grabs an arbitrary one to fetch the type
        if (sideIronPriceObj.type == drawing->sideIron(Drawing::Side::LEFT).type) {
            for (std::tuple<unsigned, float, float, unsigned, bool>price : sideIronPriceObj.prices) {
      
                if (std::get<1>(price) >= drawing->sideIron(Drawing::Side::LEFT).length && std::get<2>(price) < sideIronPrice) {
                    if (drawing->product().productName != "Extraflex" && !std::get<4>(price)) {
                        sideIronPrice = std::get<2>(price);
                        sideIronNutsPrice = ExtraPriceManager<ExtraPriceType::SIDE_IRON_NUTS>::getExtraPrice()->getPrice<ExtraPriceType::SIDE_IRON_NUTS>(std::get<3>(price));
                        sideIronScrewsPrice = ExtraPriceManager<ExtraPriceType::SIDE_IRON_SCREWS>::getExtraPrice()->getPrice<ExtraPriceType::SIDE_IRON_SCREWS>(std::get<3>(price));
                    }
                    else if (drawing->product().productName == "Extraflex" && std::get<4>(price)) {
                        prices.push_back({ "Side Iron Screws Price: ",
                                        trunc(1.3 * ExtraPriceManager<ExtraPriceType::SIDE_IRON_SCREWS>::getExtraPrice()->getPrice<ExtraPriceType::SIDE_IRON_SCREWS>(std::get<3>(price))) });
                        prices.push_back({ "Side Iron Nuts Price: ",
                                        trunc(1.3 * ExtraPriceManager<ExtraPriceType::SIDE_IRON_NUTS>::getExtraPrice()->getPrice<ExtraPriceType::SIDE_IRON_NUTS>(std::get<3>(price))) });
                    }
                }
            }
        }
    }
    if (sideIronPrice < std::numeric_limits<float>::max()) {
        prices.push_back({ "Side Irons Price: ", sideIronPrice });
        prices.push_back({ "Side Iron Screws Price: ", sideIronScrewsPrice });
        prices.push_back({ "Side Iron Nuts Price: ", sideIronNutsPrice });
    }
    else {
        missing.push_back("Side Iron");
    }

    // Extras
    // Glue and Labour
    // prices.push_back({ "Labour: ",
    //        ExtraPriceManager<ExtraPriceType::LABOUR>::getExtraPrice()->getPrice<ExtraPriceType::LABOUR>(5.0) });
    // prices.push_back({ "Tackyback Glue Price: ",
    //    ExtraPriceManager<ExtraPriceType::TACKYBACK_GLUE>::getExtraPrice()->getPrice<ExtraPriceType::TACKYBACK_GLUE>(0.0) });

    if (missing.size() != 0) {
        QMessageBox* box = new QMessageBox(this);
        std::stringstream ss;
        ss << "Couldn't find";
        for (std::vector<std::string>::iterator it = missing.begin(); it < missing.end(); it++) {
            if (it != missing.end() - 1)
                ss << " " << *it << " and";
            else
                ss << " " << *it << " price(s)";


        }
        box->setText(ss.str().c_str());
        box->exec();
    }

    QFormLayout* layout = new QFormLayout();
    float total = 0;
    for (std::pair<std::string, float> priced : prices) {
        QLineEdit* priceTextbox = new QLineEdit(QString::number(priced.second, 'f', 2));
        total += priced.second;
        priceTextbox->setReadOnly(true);
        layout->addRow(priced.first.c_str(), priceTextbox);
    }
    QLineEdit* priceTextbox = new QLineEdit(QString::number(total, 'f', 2));
    priceTextbox->setReadOnly(true);
    layout->addRow("Total: ", priceTextbox);
    ui->pricesTab->setLayout(layout);
}

void DrawingViewWidget::setChangeDrawingCallback(const std::function<void(AddDrawingPageWidget::AddDrawingMode)> &callback) {
    changeDrawingCallback = callback;
}

std::filesystem::path DrawingViewWidget::punchProgramPathForDrawing(const std::string &drawingNumber) {
    std::filesystem::path path = PUNCH_PDF_LOCATION;
    std::string folder;

    size_t index = 0;
    while (index < drawingNumber.size()) {
        if (std::isalpha(drawingNumber.at(index))) {
            folder += drawingNumber.at(index++);
        } else {
            break;
        }
    }

    if (folder.size() == 1) {
        folder = "1" + folder;
    }

    path /= folder;
    path /= drawingNumber + ".pdf";

    return path;
}
