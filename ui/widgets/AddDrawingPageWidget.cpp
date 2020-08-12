//
// Created by matthew on 14/07/2020.
//

#include "AddDrawingPageWidget.h"
#include "../build/ui_AddDrawingPageWidget.h"

AddDrawingPageWidget::AddDrawingPageWidget(const std::string &drawingNumber, QWidget *parent)
        : QWidget(parent), ui(new Ui::AddDrawingPageWidget()) {
    ui->setupUi(this);

    inspector = new Inspector("Inspector");
    inspector->addUpdateTrigger([this]() { ui->drawingSpecsVisual->setRedrawRequired(); });
    ui->drawingSpecsVisual->setInspector(inspector);

    ui->visualAndInspectorVerticalLayout->addWidget(inspector->container());

    ui->drawingNumberInput->setText(drawingNumber.c_str());

    drawing.setAsDefault();

    drawing.setDrawingNumber(drawingNumber);
    drawing.setAperture(DrawingComponentManager<Aperture>::findComponentByID(1));
    drawing.setProduct(DrawingComponentManager<Product>::findComponentByID(1));
    drawing.setMaterial(Drawing::TOP, DrawingComponentManager<Material>::findComponentByID(1));

    Date today = Date::today();
    drawing.setDate(today);

    ui->dateInput->setDate(QDate(today.year, today.month, today.day));

    setupActivators();
    setupComboboxSources();
    setupDrawingUpdateConnections();

    visualsScene = new QGraphicsScene();
    ui->drawingSpecsVisual->setScene(visualsScene);
    ui->drawingSpecsVisual->setDrawing(drawing);

    QRegExpValidator *drawingNumberValidator = new QRegExpValidator(drawingNumberRegex);

    ui->drawingNumberInput->setValidator(drawingNumberValidator);
    connect(ui->drawingNumberInput, SIGNAL(textEdited(const QString &)), this,
            SLOT(capitaliseLineEdit(const QString &)));

    QRegExpValidator *positionValidator = new QRegExpValidator(QRegExp("(^$)|(^[0-9]+([-][0-9]+)?$)|(^[Aa][Ll]{2}$)"));

    ui->machinePositionInput->setValidator(positionValidator);
    connect(ui->machinePositionInput, SIGNAL(textEdited(const QString &)), this,
            SLOT(capitaliseLineEdit(const QString &)));

    leftSideIronFilter = leftSideIronSource.setFilter<SideIronFilter>();
    rightSideIronFilter = rightSideIronSource.setFilter<SideIronFilter>();

    setMode(ADD_NEW_DRAWING);
}

AddDrawingPageWidget::AddDrawingPageWidget(const Drawing &drawing, AddDrawingPageWidget::AddDrawingMode mode,
                                           QWidget *parent)
        : QWidget(parent), ui(new Ui::AddDrawingPageWidget()) {
    ui->setupUi(this);

    inspector = new Inspector("Inspector");
    inspector->addUpdateTrigger([this]() { ui->drawingSpecsVisual->setRedrawRequired(); });
    ui->drawingSpecsVisual->setInspector(inspector);

    ui->visualAndInspectorVerticalLayout->addWidget(inspector->container());

    this->drawing = drawing;
    if (mode == CLONE_DRAWING) {
        this->drawing.setDrawingNumber("");
    }

    setupActivators();
    setupComboboxSources();
    setupDrawingUpdateConnections();

    visualsScene = new QGraphicsScene();
    ui->drawingSpecsVisual->setScene(visualsScene);
    ui->drawingSpecsVisual->setDrawing(this->drawing);

    QRegExpValidator *drawingNumberValidator = new QRegExpValidator(drawingNumberRegex);

    ui->drawingNumberInput->setValidator(drawingNumberValidator);
    connect(ui->drawingNumberInput, SIGNAL(textEdited(const QString &)), this,
            SLOT(capitaliseLineEdit(const QString &)));

    QRegExpValidator *positionValidator = new QRegExpValidator(QRegExp("(^$)|(^[0-9]+([-][0-9]+)?$)|(^[Aa][Ll]{2}$)"));

    ui->machinePositionInput->setValidator(positionValidator);
    connect(ui->machinePositionInput, SIGNAL(textEdited(const QString &)), this,
            SLOT(capitaliseLineEdit(const QString &)));

    setMode(mode);

    loadDrawing();

    leftSideIronFilter = leftSideIronSource.setFilter<SideIronFilter>();
    rightSideIronFilter = rightSideIronSource.setFilter<SideIronFilter>();

    if (drawing.sideIron(Drawing::Side::LEFT).type != SideIronType::None) {
        leftSideIronFilter->setSideIronType(drawing.sideIron(Drawing::Side::LEFT).type);
    } else {
        leftSideIronFilter->setSideIronType(SideIronType::A);
    }
    if (drawing.sideIron(Drawing::Side::RIGHT).type != SideIronType::None) {
        rightSideIronFilter->setSideIronType(drawing.sideIron(Drawing::Side::RIGHT).type);
    } else {
        leftSideIronFilter->setSideIronType(SideIronType::A);
    }
}

AddDrawingPageWidget::~AddDrawingPageWidget() {

}

void AddDrawingPageWidget::setupComboboxSources() {
    static std::unordered_map<unsigned char, unsigned char> shapeOrder = {
            {1, 1},
            {2, 4},
            {3, 2},
            {4, 3},
            {5, 5},
            {6, 0}
    };
    std::function<bool(const Aperture &, const Aperture &)> apertureComparator = [](const Aperture &a,
                                                                                    const Aperture &b) {
        if (a.apertureShapeID != b.apertureShapeID) {
            return shapeOrder[a.apertureShapeID] < shapeOrder[b.apertureShapeID];
        }
        return a.width < b.width;
    };

    DrawingComponentManager<Product>::addCallback([this]() { productSource.updateSource(); });
    DrawingComponentManager<Aperture>::addCallback([this, apertureComparator]() {
        apertureSource.updateSource();
        apertureSource.sort(apertureComparator);
    });
    DrawingComponentManager<Material>::addCallback([this]() { topMaterialSource.updateSource(); });
    DrawingComponentManager<Material>::addCallback([this]() { bottomMaterialSource.updateSource(); });
    DrawingComponentManager<SideIron>::addCallback([this]() { leftSideIronSource.updateSource(); });
    DrawingComponentManager<SideIron>::addCallback([this]() { rightSideIronSource.updateSource(); });
    DrawingComponentManager<Machine>::addCallback([this]() {
        machineManufacturerSource.updateSource();
        machineManufacturerSource.makeDistinct();
        machineModelSource.updateSource();
    });
    DrawingComponentManager<MachineDeck>::addCallback([this]() { machineDeckSource.updateSource(); });

    machineManufacturerSource.setMode(1);
    machineModelSource.setMode(2);

    productSource.updateSource();
    apertureSource.updateSource();
    topMaterialSource.updateSource();
    bottomMaterialSource.updateSource();
    leftSideIronSource.updateSource();
    rightSideIronSource.updateSource();
    machineManufacturerSource.updateSource();
    machineManufacturerSource.makeDistinct();
    machineModelSource.updateSource();
    machineDeckSource.updateSource();

    machineModelFilter = machineModelSource.setFilter<MachineModelFilter>();

    apertureSource.sort(apertureComparator);

    ui->productInput->setDataSource(productSource);
    ui->apertureInput->setDataSource(apertureSource);
    ui->topMaterialInput->setDataSource(topMaterialSource);
    ui->bottomMaterialInput->setDataSource(bottomMaterialSource);
    ui->leftSideIronDrawingInput->setDataSource(leftSideIronSource);
    ui->rightSideIronDrawingInput->setDataSource(rightSideIronSource);
    ui->manufacturerInput->setDataSource(machineManufacturerSource);
    ui->modelInput->setDataSource(machineModelSource);
    ui->machineDeckInput->setDataSource(machineDeckSource);
}

void AddDrawingPageWidget::setupActivators() {
    ui->bottomMaterialLabel->addTarget(ui->bottomMaterialInput);
    ui->bottomMaterialLabel->addActivationCallback(
            [this](bool active) {
                if (!active) {
                    drawing.removeBottomLayer();
                } else {
                    if (ui->bottomMaterialInput->currentIndex() != -1) {
                        drawing.setMaterial(Drawing::TOP, DrawingComponentManager<Material>::getComponentByHandle(
                                ui->bottomMaterialInput->currentData().toInt()));
                    }
                }
            }
    );

    ui->leftSideIronLabel->setActive();
    ui->leftSideIronLabel->addTarget(ui->leftSideIronTypeLabel);
    ui->leftSideIronLabel->addTarget(ui->leftSideIronTypeInput);
    ui->leftSideIronLabel->addTarget(ui->leftSideIronLengthLabel);
    ui->leftSideIronLabel->addTarget(ui->leftSideIronLengthInput);
    ui->leftSideIronLabel->addTarget(ui->leftSideIronDrawingLabel);
    ui->leftSideIronLabel->addTarget(ui->leftSideIronDrawingInput);
    ui->leftSideIronLabel->addTarget(ui->leftSideIronInvertedLabel);
    ui->leftSideIronLabel->addTarget(ui->leftSideIronInvertedInput);

    ui->rightSideIronLabel->addTarget(ui->rightSideIronTypeLabel);
    ui->rightSideIronLabel->addTarget(ui->rightSideIronTypeInput);
    ui->rightSideIronLabel->addTarget(ui->rightSideIronLengthLabel);
    ui->rightSideIronLabel->addTarget(ui->rightSideIronLengthInput);
    ui->rightSideIronLabel->addTarget(ui->rightSideIronDrawingLabel);
    ui->rightSideIronLabel->addTarget(ui->rightSideIronDrawingInput);
    ui->rightSideIronLabel->addTarget(ui->rightSideIronInvertedLabel);
    ui->rightSideIronLabel->addTarget(ui->rightSideIronInvertedInput);

    ui->leftSideIronLabel->addActivationCallback(
            [this](bool active) {
                if (!active) {
                    ui->rightSideIronLabel->setActive(false);
                    ui->rightSideIronLabel->setEnabled(false);
                    ui->leftSideIronLabel->setText("No Side Irons");
                    ui->rightSideIronLabel->setText("No Side Irons");

                    leftSICache = drawing.sideIron(Drawing::LEFT).handle();
                    leftSIInverted = drawing.sideIronInverted(Drawing::LEFT);
                    rightSICache = drawing.sideIron(Drawing::RIGHT).handle();
                    rightSIInverted = drawing.sideIronInverted(Drawing::RIGHT);

                    drawing.removeSideIron(Drawing::LEFT);
                    drawing.removeSideIron(Drawing::RIGHT);
                } else {
                    ui->rightSideIronLabel->setEnabled(true);
                    ui->leftSideIronLabel->setText("Side Irons");
                    ui->rightSideIronLabel->setText("Different Side Irons");

                    if (leftSICache != 0) {
                        drawing.setSideIron(Drawing::LEFT,
                                            DrawingComponentManager<SideIron>::getComponentByHandle(leftSICache));
                        drawing.setSideIronInverted(Drawing::LEFT, leftSIInverted);
                        drawing.setSideIron(Drawing::RIGHT,
                                            DrawingComponentManager<SideIron>::getComponentByHandle(leftSICache));
                        drawing.setSideIronInverted(Drawing::RIGHT, leftSIInverted);
                    }
                }
            }
    );

    ui->rightSideIronLabel->addActivationCallback(
            [this](bool active) {
                if (active) {
                    ui->leftSideIronLabel->setText("Left Side Iron");
                    ui->rightSideIronLabel->setText("Right Side Iron");

                    if (rightSICache != 0) {
                        drawing.setSideIron(Drawing::RIGHT,
                                            DrawingComponentManager<SideIron>::getComponentByHandle(rightSICache));
                        drawing.setSideIronInverted(Drawing::RIGHT, rightSIInverted);
                    }
                } else {
                    ui->leftSideIronLabel->setText("Side Irons");
                    ui->rightSideIronLabel->setText("Different Side Irons");

                    rightSICache = drawing.sideIron(Drawing::RIGHT).handle();
                    drawing.setSideIron(Drawing::RIGHT, drawing.sideIron(Drawing::LEFT));
                }
            }
    );
}

void AddDrawingPageWidget::setupDrawingUpdateConnections() {
    connect(ui->drawingNumberInput, &QLineEdit::textChanged, [this](const QString &text) {
        if (drawingNumberRegex.exactMatch(text)) {
            drawing.setDrawingNumber(text.toStdString());
        }
    });
    connect(ui->productInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        Product &product = DrawingComponentManager<Product>::getComponentByHandle(
                ui->productInput->itemData(index).toInt());

        if (product.productName == "Rubber Screen Cloth") {
            topMaterialSource.setFilter<RubberScreenClothMaterialFilter>();
            ui->bottomMaterialLabel->setActive(false);
            ui->bottomMaterialLabel->setEnabled(false);

            ui->drawingSpecsVisual->enableLaps();

            ui->tensionTypeInput->setEnabled(true);

            ui->numberOfBarsInput->setEnabled(true);
        } else if (product.productName == "Extraflex") {
            topMaterialSource.setFilter<TackyBackMaterialFilter>();
            bottomMaterialSource.setFilter<FlexBottomMaterialFilter>();
            ui->bottomMaterialLabel->setActive(true);
            ui->bottomMaterialLabel->setEnabled(true);

            ui->drawingSpecsVisual->enableLaps();

            ui->tensionTypeInput->setEnabled(true);

            ui->numberOfBarsInput->setEnabled(true);
        } else if (product.productName == "Polyflex") {
            topMaterialSource.setFilter<PolyurethaneMaterialFilter>();
            bottomMaterialSource.setFilter<FlexBottomMaterialFilter>();
            ui->bottomMaterialLabel->setActive(true);
            ui->bottomMaterialLabel->setEnabled(true);

            ui->drawingSpecsVisual->enableLaps();

            ui->tensionTypeInput->setEnabled(true);

            ui->numberOfBarsInput->setEnabled(true);
        } else if (product.productName == "Bivitec") {
            topMaterialSource.setFilter<BivitecMaterialFilter>();
            ui->bottomMaterialLabel->setActive(false);
            ui->bottomMaterialLabel->setEnabled(false);

            ui->drawingSpecsVisual->disableLaps();

            ui->tensionTypeInput->setEnabled(false);
            ui->tensionTypeInput->setCurrentIndex(0);
            drawing.setTensionType(Drawing::SIDE);

            ui->numberOfBarsInput->setEnabled(false);
            ui->numberOfBarsInput->setValue(0);
            ui->drawingSpecsVisual->setNumberOfBars(0);
        } else if (product.productName == "Flip Flow") {
            topMaterialSource.setFilter<PolyurethaneMaterialFilter>();
            ui->bottomMaterialLabel->setActive(false);
            ui->bottomMaterialLabel->setEnabled(false);

            ui->drawingSpecsVisual->disableLaps();

            ui->tensionTypeInput->setEnabled(false);
            ui->tensionTypeInput->setCurrentIndex(0);
            drawing.setTensionType(Drawing::SIDE);

            ui->numberOfBarsInput->setEnabled(false);
            ui->numberOfBarsInput->setValue(0);
            ui->drawingSpecsVisual->setNumberOfBars(0);
        } else if (product.productName == "Rubber Modules and Panels") {
            topMaterialSource.setFilter<RubberModuleMaterialFilter>();
            ui->bottomMaterialLabel->setActive(false);
            ui->bottomMaterialLabel->setEnabled(false);

            ui->drawingSpecsVisual->enableLaps();

            ui->tensionTypeInput->setEnabled(true);

            ui->numberOfBarsInput->setEnabled(true);
        } else {
            topMaterialSource.removeFilter();
            bottomMaterialSource.removeFilter();
            ui->bottomMaterialLabel->setActive(false);
            ui->bottomMaterialLabel->setEnabled(true);

            ui->drawingSpecsVisual->enableLaps();

            ui->tensionTypeInput->setEnabled(true);

            ui->numberOfBarsInput->setEnabled(true);
        }

        drawing.setProduct(product);
    });
    connect(ui->dateInput, &QDateEdit::dateChanged, [this](const QDate &date) {
        drawing.setDate({(unsigned short) date.year(), (unsigned char) date.month(), (unsigned char) date.day()});
    });

    connect(ui->widthInput, qOverload<double>(&QDoubleSpinBox::valueChanged),
            [this](double d) { drawing.setWidth((float) d); });
    connect(ui->lengthInput, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double d) {
        drawing.setLength((float) d);
        switch (drawing.tensionType()) {
            case Drawing::SIDE:
                if (leftSICache == 0) {
                    ui->leftSideIronLengthInput->setValue(d - 10);
                }
                if (rightSICache == 0) {
                    ui->rightSideIronLengthInput->setValue(d - 10);
                }
                break;
            case Drawing::END:
                if (leftSICache == 0) {
                    ui->leftSideIronLengthInput->setValue(d - 20);
                }
                if (rightSICache == 0) {
                    ui->rightSideIronLengthInput->setValue(d - 20);
                }
                break;
        }
    });
    connect(ui->topMaterialInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setMaterial(Drawing::MaterialLayer::TOP, DrawingComponentManager<Material>::getComponentByHandle(
                ui->topMaterialInput->itemData(index).toInt()));
    });
    connect(ui->bottomMaterialInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setMaterial(Drawing::MaterialLayer::BOTTOM, DrawingComponentManager<Material>::getComponentByHandle(
                ui->bottomMaterialInput->itemData(index).toInt()));
    });
    connect(ui->apertureInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setAperture(
                DrawingComponentManager<Aperture>::getComponentByHandle(ui->apertureInput->itemData(index).toInt()));
    });
    connect(ui->numberOfBarsInput, qOverload<int>(&QSpinBox::valueChanged),
            [this](int i) { ui->drawingSpecsVisual->setNumberOfBars(i); });
    connect(ui->tensionTypeInput, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        switch (index) {
            case 0:
                drawing.setTensionType(Drawing::SIDE);
                if (leftSICache == 0) {
                    ui->leftSideIronTypeInput->setCurrentIndex((int) SideIronType::A - 1);
                }
                if (rightSICache == 0) {
                    ui->rightSideIronTypeInput->setCurrentIndex((int) SideIronType::A - 1);
                }
                break;
            case 1:
                drawing.setTensionType(Drawing::END);
                if (leftSICache == 0) {
                    ui->leftSideIronTypeInput->setCurrentIndex((int) SideIronType::C - 1);
                }
                if (rightSICache == 0) {
                    ui->rightSideIronTypeInput->setCurrentIndex((int) SideIronType::C - 1);
                }
                break;
            default:
                break;
        }
    });
    connect(ui->rebatedInput, &QCheckBox::clicked, [this](bool checked) {
        drawing.setRebated(checked);
        if (checked && addedNotes.find(REBATED) == addedNotes.end()) {
            std::stringstream updatedNotes;
            updatedNotes << drawing.notes() << REBATED_NOTE << std::endl;
            ui->notesInput->setText(updatedNotes.str().c_str());
            addedNotes.insert(REBATED);
        }
    });
    connect(ui->backingStripsInput, &QCheckBox::clicked, [this](bool checked) {
        drawing.setHasBackingStrips(checked);
        if (checked && addedNotes.find(HAS_BACKING_STRIPS) == addedNotes.end()) {
            std::stringstream updatedNotes;
            updatedNotes << drawing.notes() << BACKING_STRIPS_NOTE << std::endl;
            ui->notesInput->setText(updatedNotes.str().c_str());
            addedNotes.insert(HAS_BACKING_STRIPS);
        }
    });
    connect(ui->rubberCoverStrapsAddNoteButton, &QPushButton::clicked, [this]() {
        if (addedNotes.find(RUBBER_COVER_STRAPS) == addedNotes.end()) {
            std::stringstream updatedNotes;
            updatedNotes << drawing.notes() << RUBBER_COVER_STRAPS_NOTES << std::endl;
            ui->notesInput->setText(updatedNotes.str().c_str());
            addedNotes.insert(RUBBER_COVER_STRAPS);
        }
    });
    connect(ui->addDrawingWTLStrapNoteButton, &QPushButton::clicked, [this]() {
        if (addedNotes.find(WEAR_TILE_LINER_STRAPS) == addedNotes.end()) {
            std::stringstream updatedNotes;
            updatedNotes << drawing.notes() << DRAWING_WTL_STRAPS_NOTE << std::endl;
            ui->notesInput->setText(updatedNotes.str().c_str());
            addedNotes.insert(WEAR_TILE_LINER_STRAPS);
        }
    });
    connect(ui->addSideIronWTLStrapNoteButton, &QPushButton::clicked, [this]() {
        if (addedNotes.find(WEAR_TILE_LINER_STRAPS) == addedNotes.end()) {
            std::stringstream updatedNotes;
            updatedNotes << drawing.notes() << SIDE_IRON_WTL_STRAPS_NOTE << std::endl;
            ui->notesInput->setText(updatedNotes.str().c_str());
            addedNotes.insert(WEAR_TILE_LINER_STRAPS);
        }
    });
    connect(ui->notesInput, &QTextEdit::textChanged, [this]() {
        drawing.setNotes(ui->notesInput->toPlainText().toStdString());
    });

    connect(ui->leftSideIronTypeInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        if (leftSideIronFilter) {
            leftSideIronFilter->setSideIronType((SideIronType) (index + 1));
        }
    });

    connect(ui->leftSideIronLengthInput, qOverload<int>(&QSpinBox::valueChanged), [this](int value) {
        if (leftSideIronFilter) {
            leftSideIronFilter->setSideIronFilterMinimumLength(value);
        }
    });

    connect(ui->leftSideIronDrawingInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setSideIron(Drawing::LEFT, DrawingComponentManager<SideIron>::getComponentByHandle(
                ui->leftSideIronDrawingInput->itemData(index).toInt()));
        leftSICache = ui->leftSideIronDrawingInput->itemData(index).toInt();
        if (!ui->rightSideIronLabel->active()) {
            drawing.setSideIron(Drawing::RIGHT, DrawingComponentManager<SideIron>::getComponentByHandle(
                    ui->leftSideIronDrawingInput->itemData(index).toInt()));
            rightSICache = leftSICache;
        }
    });
    connect(ui->leftSideIronInvertedInput, &QCheckBox::clicked, [this](bool checked) {
        drawing.setSideIronInverted(Drawing::LEFT, checked);
    });

    connect(ui->rightSideIronTypeInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        if (rightSideIronFilter) {
            rightSideIronFilter->setSideIronType((SideIronType) (index + 1));
        }
    });

    connect(ui->rightSideIronLengthInput, qOverload<int>(&QSpinBox::valueChanged), [this](int value) {
        if (rightSideIronFilter) {
            rightSideIronFilter->setSideIronFilterMinimumLength(value);
        }
    });

    connect(ui->rightSideIronDrawingInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setSideIron(Drawing::RIGHT, DrawingComponentManager<SideIron>::getComponentByHandle(
                ui->leftSideIronDrawingInput->itemData(index).toInt()));
        rightSICache = ui->rightSideIronDrawingInput->itemData(index).toInt();
    });
    connect(ui->rightSideIronInvertedInput, &QCheckBox::clicked, [this](bool checked) {
        drawing.setSideIronInverted(Drawing::RIGHT, checked);
    });

    connect(ui->manufacturerInput, &DynamicComboBox::currentTextChanged, [this](const QString &text) {
        machineModelFilter->setManufacturer(text.toStdString());
        ui->modelInput->updateSourceList();
        drawing.setMachine(
                DrawingComponentManager<Machine>::getComponentByHandle(ui->modelInput->itemData(0).toInt()));
    });

    connect(ui->modelInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setMachine(
                DrawingComponentManager<Machine>::getComponentByHandle(ui->modelInput->itemData(index).toInt()));
    });
    connect(ui->quantityOnDeckInput, qOverload<int>(&QSpinBox::valueChanged), [this](int newValue) {
        drawing.setQuantityOnDeck(newValue);
    });
    connect(ui->machinePositionInput, &QLineEdit::textChanged, [this](const QString &newPosition) {
        drawing.setMachinePosition(newPosition.toStdString());
    });
    connect(ui->machineDeckInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setMachineDeck(DrawingComponentManager<MachineDeck>::getComponentByHandle(
                ui->machineDeckInput->itemData(index).toInt()));
    });

    connect(ui->browseDrawingPDFButton, &QPushButton::pressed, [this]() {
        const std::filesystem::path hyperlinkFile = QFileDialog::getOpenFileName(this, "Select a Drawing PDF File",
                                                                                 QString(),
                                                                                 "PDF (*.pdf)").toStdString();
        if (!hyperlinkFile.empty()) {
            ui->hyperlinkDisplay->setText(hyperlinkFile.generic_string().c_str());
            ui->hyperlinkDisplay->setToolTip(hyperlinkFile.generic_string().c_str());

            drawing.setHyperlink(hyperlinkFile.generic_string());
        }
    });
    connect(ui->openDrawingPDFButton, &QPushButton::pressed, [this]() {
        const std::filesystem::path hyperlinkFile = ui->hyperlinkDisplay->displayText().toStdString();

        if (hyperlinkFile.empty()) {
            QMessageBox::about(this, "Open PDF", "Cannot open PDF. No PDF file path set.");
            return;
        }

        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(hyperlinkFile.generic_string().c_str()))) {
            QMessageBox::about(this, "Open PDF", ("The PDF could not be found. (" + hyperlinkFile.generic_string() + ")").c_str());
        }
    });
    connect(ui->generatePDFButton, &QPushButton::pressed, [this]() {
        if (!checkDrawing(Drawing::INVALID_HYPERLINK)) {
            return;
        }

        const QString pdfDirectory = QFileDialog::getExistingDirectory(this, "Open Folder to save PDF", QString());
        if (!pdfDirectory.isEmpty()) {
            std::filesystem::path pdfFile = pdfDirectory.toStdString();
            pdfFile /= (drawing.drawingNumber() + ".pdf");

            if (std::filesystem::exists(pdfFile)) {
                if (QMessageBox::question(this, "Overwrite PDF",
                                          "A PDF for this drawing already exists. Would you like to overwrite it?") !=
                    QMessageBox::Yes) {
                    return;
                }
            }

            std::stringstream initials;

            std::string userEmailCopy = userEmail;
            size_t nextDot;
            while ((nextDot = userEmailCopy.find('.')) < userEmailCopy.find('@')) {
                initials << (char) std::toupper(userEmailCopy.front());
                userEmailCopy.erase(0, nextDot + 1);
            }
            if (!userEmailCopy.empty()) {
                initials << (char) std::toupper(userEmailCopy.front());
            }

            if (pdfWriter.createPDF(pdfFile.generic_string().c_str(), drawing, initials.str())) {
                QMessageBox::about(this, "PDF Generator", "Automatic PDF generated.");
            } else {
                QMessageBox::about(this, "PDF Generator", "PDF Generation failed. Is the PDF file already open?");
                return;
            }

            ui->hyperlinkDisplay->setText(pdfFile.generic_string().c_str());
            ui->hyperlinkDisplay->setToolTip(pdfFile.generic_string().c_str());

            drawing.setHyperlink(pdfFile.generic_string());
        }
    });
    connect(ui->pressDrawingHyperlinksAddButton, &QPushButton::pressed, [this]() {
        const std::filesystem::path hyperlinkFile = QFileDialog::getOpenFileName(this,
                                                                                 "Select a Press Program PDF File",
                                                                                 QString(),
                                                                                 "PDF (*.pdf)").toStdString();
        if (!hyperlinkFile.empty()) {
            ui->pressDrawingHyperlinkList->addItem(hyperlinkFile.generic_string().c_str());

            std::vector<std::string> links;
            links.reserve(ui->pressDrawingHyperlinkList->count());

            for (int i = 0; i < ui->pressDrawingHyperlinkList->count(); i++) {
                std::filesystem::path p = ui->pressDrawingHyperlinkList->item(i)->text().toStdString();
                links.push_back(p.generic_string());
            }

            drawing.setPressDrawingHyperlinks(links);
        }
    });
    connect(ui->pressDrawingHyperlinksRemoveButton, &QPushButton::pressed, [this]() {
        qDeleteAll(ui->pressDrawingHyperlinkList->selectedItems());

        std::vector<std::string> links;
        links.reserve(ui->pressDrawingHyperlinkList->count());

        for (int i = 0; i < ui->pressDrawingHyperlinkList->count(); i++) {
            links.push_back(ui->pressDrawingHyperlinkList->item(i)->text().toStdString());
        }

        drawing.setPressDrawingHyperlinks(links);
    });

    connect(ui->confirmDrawingButton, &QPushButton::pressed, this, &AddDrawingPageWidget::confirmDrawing);
}

void AddDrawingPageWidget::loadDrawing() {
    ui->drawingNumberInput->setText(drawing.drawingNumber().c_str());
    ui->productInput->setCurrentIndex(ui->productInput->findData(drawing.product().handle()));
    ui->dateInput->setDate(QDate(drawing.date().year, drawing.date().month, drawing.date().day));
    ui->widthInput->setValue(drawing.width());
    ui->lengthInput->setValue(drawing.length());
    if (!drawing.loadWarning(Drawing::MISSING_MATERIAL_DETECTED)) {
        ui->topMaterialInput->setCurrentIndex(ui->topMaterialInput->findData(drawing.material(Drawing::TOP)->handle()));
        if (drawing.material(Drawing::BOTTOM).has_value()) {
            ui->bottomMaterialLabel->setActive();
            ui->bottomMaterialInput->setCurrentIndex(
                    ui->bottomMaterialInput->findData(drawing.material(Drawing::BOTTOM)->handle()));
        }
    }
    if (!drawing.loadWarning(Drawing::INVALID_APERTURE_DETECTED)) {
        ui->apertureInput->setCurrentIndex(ui->apertureInput->findData(drawing.aperture().handle()));
    }
    ui->numberOfBarsInput->setValue(drawing.numberOfBars());
    switch (drawing.tensionType()) {
        case Drawing::TensionType::SIDE:
            ui->tensionTypeInput->setCurrentText("Side");
            break;
        case Drawing::TensionType::END:
            ui->tensionTypeInput->setCurrentText("End");
            break;
    }
    ui->rebatedInput->setChecked(drawing.rebated());
    ui->backingStripsInput->setChecked(drawing.hasBackingStrips());

    if (drawing.rebated()) {
        addedNotes.insert(REBATED);
    }
    if (drawing.hasBackingStrips()) {
        addedNotes.insert(HAS_BACKING_STRIPS);
    }

    ui->notesInput->setPlainText(drawing.notes().c_str());

    if (!drawing.loadWarning(Drawing::MISSING_SIDE_IRONS_DETECTED)) {
        SideIron leftSideIron = drawing.sideIron(Drawing::LEFT), rightSideIron = drawing.sideIron(Drawing::RIGHT);
        ui->leftSideIronDrawingInput->setCurrentIndex(ui->leftSideIronDrawingInput->findData(leftSideIron.handle()));
        ui->leftSideIronInvertedInput->setChecked(drawing.sideIronInverted(Drawing::LEFT));
        ui->rightSideIronDrawingInput->setCurrentIndex(ui->rightSideIronDrawingInput->findData(rightSideIron.handle()));
        ui->rightSideIronInvertedInput->setChecked(drawing.sideIronInverted(Drawing::RIGHT));
        if (leftSideIron.handle() == rightSideIron.handle()) {
            ui->rightSideIronLabel->setActive(false);

            if (leftSideIron.componentID() == 1) {
                ui->leftSideIronLabel->setActive(false);
            } else {
                ui->leftSideIronLabel->setActive(true);
            }
        } else {
            ui->rightSideIronLabel->setActive(true);
        }
    }

    Drawing::MachineTemplate machineTemplate = drawing.machineTemplate();
    ui->manufacturerInput->setCurrentText(machineTemplate.machine().manufacturer.c_str());
    ui->modelInput->setCurrentText(machineTemplate.machine().model.c_str());
    ui->quantityOnDeckInput->setValue(machineTemplate.quantityOnDeck);
    ui->machinePositionInput->setText(machineTemplate.position.c_str());
    ui->machineDeckInput->setCurrentIndex(ui->machineDeckInput->findData(machineTemplate.deck().handle()));

    ui->hyperlinkDisplay->setText(drawing.hyperlink().c_str());
    for (const std::string &hyperlink : drawing.pressDrawingHyperlinks()) {
        ui->pressDrawingHyperlinkList->addItem(hyperlink.c_str());
    }
}

bool AddDrawingPageWidget::checkDrawing(unsigned exclusions) {
    switch (drawing.checkDrawingValidity(exclusions)) {
        case Drawing::SUCCESS:
            return true;
        case Drawing::INVALID_DRAWING_NUMBER:
            QMessageBox::about(this, "Drawing Error", "Invalid Drawing Number");
            return false;
        case Drawing::INVALID_PRODUCT:
            QMessageBox::about(this, "Drawing Error", "Invalid Product selected");
            return false;
        case Drawing::INVALID_WIDTH:
            QMessageBox::about(this, "Drawing Error", "Invalid width. Make sure the width is positive");
            return false;
        case Drawing::INVALID_LENGTH:
            QMessageBox::about(this, "Drawing Error", "Invalid length. Make sure the length is positive");
            return false;
        case Drawing::INVALID_TOP_MATERIAL:
            QMessageBox::about(this, "Drawing Error", "Invalid Top Layer Material");
            return false;
        case Drawing::INVALID_BOTTOM_MATERIAL:
            QMessageBox::about(this, "Drawing Error", "Invalid Bottom Layer Material");
            return false;
        case Drawing::INVALID_APERTURE:
            QMessageBox::about(this, "Drawing Error", "Invalid Aperture");
            return false;
        case Drawing::INVALID_BAR_SPACINGS:
            QMessageBox::about(this, "Drawing Error", "Invalid Bar Spacings. Make sure they add to the width");
            return false;
        case Drawing::INVALID_BAR_WIDTHS:
            QMessageBox::about(this, "Drawing Error",
                               "Invalid Bar Widths. Make sure they are all positive and non-zero");
            return false;
        case Drawing::INVALID_SIDE_IRONS:
            QMessageBox::about(this, "Drawing Error", "Invalid Side Irons");
            return false;
        case Drawing::INVALID_MACHINE:
            QMessageBox::about(this, "Drawing Error", "Invalid Machine");
            return false;
        case Drawing::INVALID_MACHINE_POSITION:
            QMessageBox::about(this, "Drawing Error", "Invalid Machine position");
            return false;
        case Drawing::INVALID_MACHINE_DECK:
            QMessageBox::about(this, "Drawing Error", "Invalid Machine Deck");
            return false;
        case Drawing::INVALID_HYPERLINK:
            QMessageBox::about(this, "Drawing Error", "Invalid Drawing PDF Hyperlink");
            return false;
        default:
            QMessageBox::about(this, "Drawing Error", "Unknown error");
            return false;
    }
}

void AddDrawingPageWidget::confirmDrawing() {
    if (checkDrawing()) {
        switch (addMode) {
            case ADD_NEW_DRAWING:
            case CLONE_DRAWING:
                confirmationCallback(drawing, false);
                drawingAdded = true;
                break;
            case EDIT_DRAWING:
                switch (QMessageBox::question(this, "Confirm Update",
                                              "Are you sure you wish to update this drawing?")) {
                    case QMessageBox::Yes:
                        confirmationCallback(drawing, true);
                        drawingAdded = true;
                        break;
                    default:
                        break;
                }
                break;
        }
    }
}

void AddDrawingPageWidget::setConfirmationCallback(const std::function<void(const Drawing &, bool)> &callback) {
    confirmationCallback = callback;
}

void AddDrawingPageWidget::setMode(AddDrawingPageWidget::AddDrawingMode mode) {
    addMode = mode;

    switch (addMode) {
        case ADD_NEW_DRAWING:
        case CLONE_DRAWING:
            ui->confirmDrawingButton->setText("Add Drawing to Database");
            break;
        case EDIT_DRAWING:
            ui->confirmDrawingButton->setText("Update Drawing");
            break;
    }
}

void AddDrawingPageWidget::setUserEmail(const std::string &email) {
    userEmail = email;
}

void AddDrawingPageWidget::closeEvent(QCloseEvent *event) {
    if (!drawingAdded) {
        if (QMessageBox::question(this, "Close Page",
                                  "Are you sure you wish to close this page without adding your drawing "
                                  "to the database?") == QMessageBox::No) {
            event->ignore();
            return;
        }
    }

    QWidget::closeEvent(event);
}

void AddDrawingPageWidget::capitaliseLineEdit(const QString &text) {
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (!lineEdit) {
        return;
    }
    lineEdit->setText(text.toUpper());
}