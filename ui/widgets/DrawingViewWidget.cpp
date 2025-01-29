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
    pdfDocument->close();
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
    //ui->CoverStrapsTextbox->
    ui->notesTextbox->setText(drawing->notes().c_str());

    // Machine Template Page
    ui->machineTextbox->setText(drawing->machineTemplate().machine().machineName().c_str());
    ui->quantityOnDeckTextbox->setText(to_str(drawing->machineTemplate().quantityOnDeck).c_str());
    ui->positionTextbox->setText(drawing->machineTemplate().position.c_str());
    ui->deckTextbox->setText(drawing->machineTemplate().deck().deck.c_str());

    // Drawings page
    ui->drawingPDFSelectorInput->clear();
    // ui->drawingPDFSelectorInput->addItem((drawing->drawingNumber() + " PDF").c_str(), drawing->hyperlink().c_str());
    ui->drawingPDFSelectorInput->addItem(
        QString::fromStdString(drawing->drawingNumber() + " PDF"),
        QString::fromStdString(drawing->hyperlink().string())
    );

    for (const std::filesystem::path& pdf : drawing->pressDrawingHyperlinks()) {
        ui->drawingPDFSelectorInput->addItem(pdf.generic_string().c_str(), pdf.generic_string().c_str());
    }

    std::vector<std::filesystem::path> pressDrawingLocations = punchProgramPathForDrawing(drawing->drawingNumber());

    for (std::filesystem::path p : pressDrawingLocations) {
        if (std::filesystem::exists(p)) {
            ui->drawingPDFSelectorInput->addItem((p.filename().string() + " Punch Press Program").c_str(), p.generic_string().c_str());
        }
    }

    if (!drawing->loadWarning(Drawing::MISSING_SIDE_IRONS_DETECTED)) {
        SideIron leftSideIron = drawing->sideIron(Drawing::LEFT),
            rightSideIron = drawing->sideIron(Drawing::RIGHT);
        ui->drawingPDFSelectorInput->addItem(("Left Side Iron: " + leftSideIron.drawingNumber).c_str(), leftSideIron.hyperlink.c_str());
        ui->drawingPDFSelectorInput->addItem(("Right Side Iron: " + rightSideIron.drawingNumber).c_str(), rightSideIron.hyperlink.c_str());
    }

    QPdfDocumentRenderOptions renderOptions;
    renderOptions.setRenderFlags(QPdfDocumentRenderOptions::RenderFlag::Annotations);
    // pdfViewer->setDocumentRenderOptions(renderOptions);

    connect(ui->drawingPDFSelectorInput, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        pdfDocument = DrawingViewWidget::getDoc(ui->drawingPDFSelectorInput->itemData(index).toString(), sizeHint());

        pdfViewer->setDocument(pdfDocument);
        });

    ui->drawingPDFsTabFormLayout->setWidget(1, QFormLayout::FieldRole, pdfViewer);
    pdfDocument = DrawingViewWidget::getDoc(
                      drawing->hyperlink().generic_string().c_str(), sizeHint());
    pdfViewer->setDocument(pdfDocument);

    // Pricing
    QVBoxLayout *outerLayout = new QVBoxLayout();
    ui->pricesTab->setLayout(outerLayout);

    QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
    QRegularExpressionValidator* unsignedValidator = new QRegularExpressionValidator(QRegularExpression("[1-9][0-9]*"));
    float total = 0;
    QHBoxLayout* prices_layout = new QHBoxLayout();
    QWidget* prices_layout_container = new QWidget();
    prices_layout_container->setLayout(prices_layout);
    outerLayout->addWidget(prices_layout_container);

    QWidget* item_container = new QWidget();

    QFormLayout *items_layout = new QFormLayout();
    items_layout->setLabelAlignment(Qt::AlignRight);
    item_container->setLayout(items_layout);

    prices_layout->addWidget(item_container);

    std::vector<PriceItem> items = get_prices(*drawing);
    for (PriceItem item : items) {
        QHBoxLayout* vlayout = new QHBoxLayout();
        QLineEdit *second;
        if (float* flt = std::get_if<float>(&item.second)) {
            QLabel *poundLabel = new QLabel(QStringLiteral("\u00A3"));
            poundLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            poundLabel->setAlignment(Qt::AlignCenter);
            second = new QLineEdit(QString::number(*flt, 'f', 2));
            // second->setDisabled(true);
            second->setValidator(validator);
            total += *flt;
            vlayout->addWidget(poundLabel);
        }
        else {
            QLabel* spaceLabel = new QLabel(QStringLiteral("\u00A3"));
            spaceLabel->setStyleSheet("color: rgba(0, 0, 0, 0);");
            spaceLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            spaceLabel->setAlignment(Qt::AlignCenter);
            std::string err = std::get<std::string>(item.second);
            second = new QLineEdit(err.c_str());
            second->setValidator(validator);
            // second->setDisabled(true);
            vlayout->addWidget(spaceLabel);
        }
        vlayout->addWidget(second);
        subtotalEdits.push_back(second);
        connect(second, &QLineEdit::textChanged, this, &DrawingViewWidget::updateTotals);
        items_layout->addRow(item.first.c_str(), vlayout);
    }
    QHBoxLayout *extrasLayout = new QHBoxLayout();

    QLabel* poundLabel = new QLabel(QStringLiteral("\u00A3"));
    poundLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    poundLabel->setAlignment(Qt::AlignCenter);
    extraBox = new QLineEdit(QString::number(0, 'f', 2));
    extraBox->setValidator(validator);
    extrasLayout->addWidget(poundLabel);
    extrasLayout->addWidget(extraBox);
    items_layout->addRow("Extras", extrasLayout);
    connect(extraBox, &QLineEdit::textChanged, this, &DrawingViewWidget::updateTotals);

    QFormLayout* times_layout = new QFormLayout();
    times_layout->setLabelAlignment(Qt::AlignRight);
    QWidget* times_container = new QWidget();
    times_container->setLayout(times_layout);
    prices_layout->addWidget(times_container);
    float total_time = 0;
    for (TimeItem t : get_times(*drawing)) {
        total_time += t.second;
        QLineEdit* timeEdit = new QLineEdit(QString::number(t.second));
        timeEdit->setValidator(unsignedValidator);
        times_layout->addRow(t.first.c_str(), timeEdit);
        labourEdits.push_back(timeEdit);
        connect(timeEdit, &QLineEdit::textChanged, this, &DrawingViewWidget::updateTotals);
    }
    extraTimeEdit = new QLineEdit(QString::number(0));
    extraTimeEdit->setValidator(unsignedValidator);
    times_layout->addRow("Extra Time", extraTimeEdit);
    connect(extraTimeEdit, &QLineEdit::textChanged, this, &DrawingViewWidget::updateTotals);

    totalTimeEdit = new QLineEdit(QString::number(total_time));
    totalTimeEdit->setValidator(unsignedValidator);
    totalTimeEdit->setDisabled(true);
    times_layout->addRow("Total Time", totalTimeEdit);
    connect(totalTimeEdit, &QLineEdit::textChanged, this, &DrawingViewWidget::updateTotals);

    QHBoxLayout* totaling_layout = new QHBoxLayout();
    QWidget* totaling_layout_widget = new QWidget();
    totaling_layout_widget->setLayout(totaling_layout);
    outerLayout->addWidget(totaling_layout_widget);
    QWidget* left_filler = new QWidget(), * right_filler = new QWidget(), * contents = new QWidget();
    left_filler->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    contents->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    right_filler->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    totaling_layout->addWidget(left_filler);
    totaling_layout->addWidget(contents);
    totaling_layout->addWidget(right_filler);

    QFormLayout* final_layout = new QFormLayout(contents);
    final_layout->setLabelAlignment(Qt::AlignRight);
    subtotal_lineEdit = new QLineEdit(QString::number(total, 'f', 2));
    subtotal_lineEdit->setDisabled(true);
    final_layout->addRow(new QLabel("Subtotal " + QStringLiteral("\u00A3")), subtotal_lineEdit);

    std::optional<float> maybe_labour_total_cost = get_time_cost(total_time);
    float labour_total_cost =
        maybe_labour_total_cost.has_value() ? *maybe_labour_total_cost : 0;
    labour_cost_lineEdit = new QLineEdit(QString::number(labour_total_cost, 'f', 2));
    labour_cost_lineEdit->setDisabled(true);
    final_layout->addRow(new QLabel("Labour Total " + QStringLiteral("\u00A3")), labour_cost_lineEdit);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    final_layout->addRow(line);

    float total_total = total + labour_total_cost;
    total_lineEdit = new QLineEdit(QString::number(total_total, 'f', 2));
    total_lineEdit->setDisabled(true);
    final_layout->addRow(new QLabel("Total " + QStringLiteral("\u00A3")), total_lineEdit);

    sales_increase = new QLineEdit(QString::number(20, 'f', 2));
    sales_increase->setValidator(validator);
    final_layout->addRow("Sales Increase", sales_increase);
    connect(sales_increase, &QLineEdit::textChanged, this, &DrawingViewWidget::updateTotals);

    sales_total = new QLineEdit(QString::number(total_total * 1.2, 'f', 2));
    sales_total->setDisabled(true);
    final_layout->addRow("Sales Price", sales_total);
    connect(sales_total, &QLineEdit::textChanged, this, &DrawingViewWidget::updateTotals);
}

void DrawingViewWidget::setChangeDrawingCallback(const std::function<void(AddDrawingPageWidget::AddDrawingMode)> &callback) {
    changeDrawingCallback = callback;
}

std::vector<std::filesystem::path> DrawingViewWidget::punchProgramPathForDrawing(const std::string &drawingNumber) {
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
    if (folder == "M") {
        folder = "2. Manuals";
    } else if (folder.size() == 1) {
        folder = "1" + folder;
    }

    std::vector<std::filesystem::path> out;
    path /= folder;
    if (!std::filesystem::is_directory(path))
        return {};
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (std::filesystem::is_regular_file(entry.path()) && std::regex_match(entry.path().filename().string(), std::regex(drawingNumber + ".*\\.pdf"))) {
            out.push_back(entry);
        }
    }

    return out;
};


void DrawingViewWidget::updateTotals() const {
    float totalCost = 0;
    for (QLineEdit *edit : subtotalEdits) {
        totalCost += edit->text().toFloat();
    }
    totalCost += extraBox->text().toFloat();
    subtotal_lineEdit->setText(QString::number(totalCost, 'f', 2));

    unsigned total_time = 0;
    for (QLineEdit* edit : labourEdits) {
        total_time += edit->text().toUInt();
    }
    totalTimeEdit->setText(QString::number(total_time));
    std::optional<float> maybe_labour_total_cost = get_time_cost(total_time);
    float labour_total_cost =
        maybe_labour_total_cost.has_value() ? *maybe_labour_total_cost : 0;
    labour_cost_lineEdit->setText(QString::number(labour_total_cost, 'f', 2));

    total_lineEdit->setText(QString::number(totalCost + labour_total_cost, 'f', 2));

    sales_total->setText(QString::number((totalCost + labour_total_cost) * (1 + (sales_increase->text().toFloat()/100)), 'f', 2));
};

QPdfDocument *DrawingViewWidget::getDoc(const QString& path, const QSize &size) {
    QPdfDocument* oldDoc = new QPdfDocument();
    oldDoc->load(path);
  QByteArray *pdfData = new QByteArray();
  QBuffer *buffer = new QBuffer(pdfData);
  buffer->open(QIODevice::ReadWrite);
  QPdfWriter writer(buffer);
  QPageLayout layout;
  layout.setPageSize(QPageSize::A4);
  layout.setOrientation(QPageLayout::Orientation::Landscape);
  writer.setPageLayout(layout);
  writer.setResolution(300);
  QPainter painter;
  painter.begin(&writer);
  QPdfDocumentRenderOptions options;
  options.setRenderFlags(QPdfDocumentRenderOptions::RenderFlag::Annotations);
  for (int i = 0; i < oldDoc->pageCount(); i++) {
        QImage img = oldDoc->render(i, QSize(writer.width(), writer.height()),
                                    options);
    painter.drawImage(0, 0, img);
    writer.newPage();
  }
  painter.end();
  QPdfDocument *pdfDocument = new QPdfDocument();
  pdfDocument->load(buffer);
  buffer->close();
  return pdfDocument;
 }