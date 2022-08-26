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

    if (drawing->loadWarning(Drawing::INVALID_CENTRE_HOLE_DETECTED)) {
        QMessageBox::about(this, "Invalid Centre Holes Detected", "Invalid Aperture used for one or more centre hole(s).");
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
                                          (drawing->sideIronInverted(Drawing::LEFT) ? " (inverted)" : "") +
                                          (drawing->sideIronCutDown(Drawing::LEFT) ? " (cut down)" : "")).c_str());
        ui->rightSideIronTextbox->setText((drawing->sideIron(Drawing::RIGHT).sideIronStr() +
                                           (drawing->sideIronInverted(Drawing::RIGHT) ? " (inverted)" : "") +
                                           (drawing->sideIronCutDown(Drawing::RIGHT) ? " (cut down)" : "")).c_str());
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
    if (drawing->hasBackingStrips()) {
        ui->backingStripTextbox->setText(drawing->backingStrip()->backingStripName().c_str());
    }
    else {
        ui->backingStripTextbox->setText("None");
    }
    ui->rebatedCheckbox->setChecked(drawing->rebated());
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

    // Prices not yet implimented
    // Prices page
    std::vector<std::pair<std::string, float>> prices;

    std::vector<std::string> missing;

    //Material Price
    if (drawing->material(Drawing::TOP).has_value()) {
        Material m = drawing->material(Drawing::TOP).value();
        std::tuple<float, float, float, MaterialPricingType> bestPrice(0.0, 0.0, std::numeric_limits<float>::max(), MaterialPricingType::RUNNING_M);
        for (std::tuple<float, float, float, MaterialPricingType> price : m.materialPrices) {
            if (std::get<0>(price) >= (drawing->width() + (
                (drawing->sidelap(Drawing::LEFT).has_value() ? (drawing->sidelap(Drawing::LEFT)->attachmentType == LapAttachment::INTEGRAL ? drawing->sidelap(Drawing::LEFT)->width : 0) : 0) +
                (drawing->sidelap(Drawing::RIGHT).has_value() ? (drawing->sidelap(Drawing::RIGHT)->attachmentType == LapAttachment::INTEGRAL ? drawing->sidelap(Drawing::RIGHT)->width : 0) : 0)
                                        ) && std::get<2>(price) < std::get<2>(bestPrice)))
                bestPrice = price;
        }
        if (bestPrice == std::tuple<float, float, float, MaterialPricingType>(0.0, 0.0, std::numeric_limits<float>::max(), MaterialPricingType::RUNNING_M)) {
            missing.push_back("Material");
        }
        else {
            prices.push_back({ "Material", std::get<2>(bestPrice) * (drawing->length() / 1000) });
        }
    }

    //Side Iron Price
    std::optional<std::tuple<unsigned, float, float, unsigned, bool>> lBestPrice, rBestPrice;
    std::optional<SideIron> left = std::nullopt, right = std::nullopt;
    try {
        left = drawing->sideIron(Drawing::LEFT);
    }
    catch (std::exception _) {}
    try {
        right = drawing->sideIron(Drawing::RIGHT);
    }
    catch (std::exception _) {}

    for (unsigned i : DrawingComponentManager<SideIronPrice>::dataIndexSet()) {
        SideIronPrice p = DrawingComponentManager<SideIronPrice>::getComponentByHandle(i);
        if (left.has_value() && p.type == left->type) {
            for (std::tuple<unsigned, float, float, unsigned, bool> price : p.prices) {
                //TODO: make sure length is correct
                if (std::get<1>(price) > drawing->length() && (drawing->material(Drawing::TOP)->materialName == "Extraflex") == std::get<4>(price)) {
                    if (!lBestPrice.has_value()) {
                        lBestPrice = price;
                    }
                    else if (std::get<2>(lBestPrice.value()) > std::get<2>(price)) {
                        lBestPrice = price;
                    }
                }
            }
        }
        if (right.has_value() && p.type == right->type) {
            for (std::tuple<unsigned, float, float, unsigned, bool> price : p.prices) {
                //TODO: make sure length is correct
                if (std::get<1>(price) > drawing->length() && (drawing->material(Drawing::TOP)->materialName == "Extraflex") == std::get<4>(price)) {
                    if (!rBestPrice.has_value()) {
                        rBestPrice = price;
                    }
                    else if (std::get<2>(rBestPrice.value()) > std::get<2>(price)) {
                        rBestPrice = price;
                    }
                }
            }
        }
    }
    if (lBestPrice.has_value() || rBestPrice.has_value()) {
        unsigned screws = 0;
        if (lBestPrice.has_value()) {
            prices.push_back({ "Left Side Iron", std::get<2>(lBestPrice.value()) / 2 });
            screws += std::get<3>(lBestPrice.value());
        }
        if (rBestPrice.has_value()) {
            prices.push_back({ "Right Side Iron", std::get<2>(rBestPrice.value()) / 2 });
            screws += std::get<3>(lBestPrice.value());
        }
        prices.push_back({ "Side Iron Screws", ExtraPriceManager<ExtraPriceType::SIDE_IRON_SCREWS>::getExtraPrice()->getPrice<ExtraPriceType::SIDE_IRON_SCREWS>(screws) });
        prices.push_back({ "Side Iron Nuts", ExtraPriceManager<ExtraPriceType::SIDE_IRON_NUTS>::getExtraPrice()->getPrice<ExtraPriceType::SIDE_IRON_NUTS>(screws) });
    }

    // Bonded overlaps and sidelap

    {
        float total = 0;
        for (std::optional<Drawing::Lap> lap : { drawing->overlap(Drawing::LEFT), drawing->overlap(Drawing::RIGHT) }) {
            if (!lap.has_value() || lap->attachmentType == LapAttachment::INTEGRAL)
                continue;
            float area = ((lap->width + 50) * (drawing->width() - 100) / 1000000) * LOSS_PERCENT;
            std::tuple<float, float, float, MaterialPricingType> bestPrice(0, 0, std::numeric_limits<float>::max(), MaterialPricingType::RUNNING_M);
            for (std::tuple<float, float, float, MaterialPricingType> price : lap->material().materialPrices) {
                if (std::get<2>(price) < std::get<2>(bestPrice) && std::get<0>(price) > lap->width)
                    bestPrice = price;
            }
            total += std::get<2>(bestPrice) * area;
        }
        for (std::optional<Drawing::Lap> lap : { drawing->sidelap(Drawing::LEFT), drawing->sidelap(Drawing::RIGHT) }) {
            if (!lap.has_value() || lap->attachmentType == LapAttachment::INTEGRAL)
                continue;
            float area = (((lap->width + 50) * (drawing->width() - 100)) / 1000000) * LOSS_PERCENT;
            std::tuple<float, float, float, MaterialPricingType> bestPrice(0, 0, std::numeric_limits<float>::max(), MaterialPricingType::RUNNING_M);
            for (std::tuple<float, float, float, MaterialPricingType> price : lap->material().materialPrices) {
                if (std::get<2>(price) < std::get<2>(bestPrice) && std::get<0>(price) > lap->width)
                    bestPrice = price;
            }
            if (std::get<2>(bestPrice) != std::numeric_limits<float>::max())
                total += std::get<2>(bestPrice) * area;
        }
        if (total == 0)
            missing.push_back("Material");
        else {
            prices.push_back({ "Bonded Overlaps and Sidelaps", total });
        }
    }

    // Deflectors, Divertors and Dam Bars

    ExtraPriceManager<ExtraPriceType::DEFLECTOR>::getExtraPrice()->getPrice<ExtraPriceType::DEFLECTOR>(drawing->numberOfDeflectors());

    ExtraPriceManager<ExtraPriceType::DIVERTOR>::getExtraPrice()->getPrice<ExtraPriceType::DIVERTOR>(drawing->numberOfDivertors());

    ExtraPriceManager<ExtraPriceType::DAM_BAR>::getExtraPrice()->getPrice<ExtraPriceType::DAM_BAR>(drawing->numberOfDamBars());

    // Impact Pads
    {
        float total = 0;
        float glueTotal = 0;
        for (const Drawing::ImpactPad &pad : drawing->impactPads()) {
            float area = (pad.length * pad.width) / 1000000 * LOSS_PERCENT;
            std::tuple<float, float, float, MaterialPricingType> bestPrice = std::tuple<float, float, float, MaterialPricingType>(0, 0, std::numeric_limits<float>::max(), MaterialPricingType::RUNNING_M);
            for (std::tuple<float, float, float, MaterialPricingType> price : pad.material().materialPrices) {
                if (std::get<3>(price) == MaterialPricingType::RUNNING_M) {
                    if (std::get<0>(price) > pad.width && (std::get<1>(price) > pad.length || pad.length == 0) && std::get<2>(price) < std::get<2>(bestPrice))
                        bestPrice = price;
                }
                else {
                    if (std::get<2>(price) < std::get<2>(bestPrice))
                        bestPrice = price;
                }
            }
            if (std::get<2>(bestPrice) != std::numeric_limits<float>::max())
                total += std::get<2>(bestPrice) * area;
            glueTotal += ExtraPriceManager<ExtraPriceType::TACKYBACK_GLUE>::getExtraPrice()->getPrice<ExtraPriceType::TACKYBACK_GLUE>(area) * (pad.material().materialName == "Tacky Back" ? 1.5 : 2);
        }
        if (total != 0) {
            prices.push_back({ "Impact Pad(s)", total });
            prices.push_back({ "Impact pad glue", glueTotal });
        }
    }
    
    // Primer
    


    // glue price
    if (drawing->hasBackingStrips()) {
        float totalWidth = 0;
        for (std::vector<float>::const_iterator it = (drawing->allBarWidths().begin() + 1); it < (drawing->allBarWidths().end()-1); it++) {
            totalWidth += *it;
        }
        float area = (totalWidth / 1000)* (drawing->length() / 1000);
        std::string materialName = DrawingComponentManager<Material>::findComponentByID(drawing->backingStrip()->materialID).materialName;
        if (materialName == "Rubber Screen Cloth")
            prices.push_back({ "Tackyback Glue Price: ",
                ExtraPriceManager<ExtraPriceType::TACKYBACK_GLUE>::getExtraPrice()->getPrice<ExtraPriceType::TACKYBACK_GLUE>(area) * 2});
        else if (materialName == "Tacky Back")
            prices.push_back({ "Tackyback Glue Price: ",
                ExtraPriceManager<ExtraPriceType::TACKYBACK_GLUE>::getExtraPrice()->getPrice<ExtraPriceType::TACKYBACK_GLUE>(area) * 1.5 });
        else if (materialName == "Polyurethane")
            prices.push_back({ "Tackyback Glue Price: ",
                ExtraPriceManager<ExtraPriceType::TACKYBACK_GLUE>::getExtraPrice()->getPrice<ExtraPriceType::TACKYBACK_GLUE>(area) * 2 });
        else if (materialName == "Moulded Polyurethane")
            prices.push_back({ "Tackyback Glue Price: ",
                ExtraPriceManager<ExtraPriceType::TACKYBACK_GLUE>::getExtraPrice()->getPrice<ExtraPriceType::TACKYBACK_GLUE>(area) * 2 });
        else if (materialName == "Rubber x60")
            prices.push_back({ "Tackyback Glue Price: ",
                ExtraPriceManager<ExtraPriceType::TACKYBACK_GLUE>::getExtraPrice()->getPrice<ExtraPriceType::TACKYBACK_GLUE>(area) * 1.5 });
    }
    if (true /*straps*/) {

    }


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
        // box->exec();
    }

    //Impact Pads


    QVBoxLayout* main = new QVBoxLayout();
    QHBoxLayout* formLayout = new QHBoxLayout();
    QFormLayout* leftForm = new QFormLayout();
    QFormLayout* rightForm = new QFormLayout();
    QFormLayout* bottomForm = new QFormLayout();
    main->setMargin(0);
    main->setSpacing(5);
    formLayout->setMargin(5);

    main->insertLayout(0, formLayout);
    main->insertLayout(1, bottomForm);

    formLayout->insertLayout(0, leftForm);
    formLayout->insertLayout(1, rightForm);

    QWidget* costLabelContainer = new QWidget();
    QLabel* costLabel = new QLabel("Material costs", costLabelContainer);
    leftForm->addRow(costLabelContainer);
    float total = 0;
    for (std::pair<std::string, float> priced : prices) {
        QLineEdit* priceTextbox = new QLineEdit(QString::number(priced.second, 'f', 2));
        total += priced.second;
        priceTextbox->setReadOnly(true);
        leftForm->addRow(priced.first.c_str(), priceTextbox);
    }
    QLineEdit* priceTextbox = new QLineEdit(QString::number(total, 'f', 2));
    priceTextbox->setReadOnly(true);
    leftForm->addRow("Total: ", priceTextbox);


    leftForm->addRow(new QLabel("left"));
    rightForm->addRow(new QLabel("right"));

    QLineEdit* grandTotal = new QLineEdit(QString::number(100, 'f', 2));
    bottomForm->addRow("Grand Total: ", grandTotal);

    ui->pricesTab->setLayout(main);
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
