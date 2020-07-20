#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 14/07/2020.
//

#include "AddDrawingPageWidget.h"
#include "../build/ui_AddDrawingPageWidget.h"

AddDrawingPageWidget::AddDrawingPageWidget(QWidget *parent)
        : QWidget(parent), ui(new Ui::AddDrawingPageWidget) {
    ui->setupUi(this);

    // TODO: Text selecting line edits / spin boxes

    setupActivators();
    setupComboboxSources();

    drawing.setWidth(0);
    drawing.setLength(0);
    drawing.setTensionType(Drawing::SIDE);

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

    setupDrawingUpdateConnections();
}

AddDrawingPageWidget::~AddDrawingPageWidget() {

}

void AddDrawingPageWidget::setupComboboxSources() {
    DrawingComponentManager<Product>::addCallback([this]() { productSource.updateSource(); });
    DrawingComponentManager<Aperture>::addCallback([this]() { apertureSource.updateSource(); });
    DrawingComponentManager<Material>::addCallback([this]() { materialSource.updateSource(); });
    DrawingComponentManager<SideIron>::addCallback([this]() { sideIronSource.updateSource(); });
    DrawingComponentManager<Machine>::addCallback([this]() { machineSource.updateSource(); });
    DrawingComponentManager<MachineDeck>::addCallback([this]() { machineDeckSource.updateSource(); });

    productSource.updateSource();
    apertureSource.updateSource();
    materialSource.updateSource();
    sideIronSource.updateSource();
    machineSource.updateSource();
    machineDeckSource.updateSource();

    ui->productInput->setDataSource(productSource);
    ui->apertureInput->setDataSource(apertureSource);
    ui->topMaterialInput->setDataSource(materialSource);
    ui->bottomMaterialInput->setDataSource(materialSource);
    ui->leftSideIronDrawingInput->setDataSource(sideIronSource);
    ui->rightSideIronDrawingInput->setDataSource(sideIronSource);
    ui->machineInput->setDataSource(machineSource);
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
                        drawing.setMaterial(Drawing::TOP, DrawingComponentManager<Material>::getComponentByID(
                                ui->bottomMaterialInput->currentData().value<ElementIndex>()));
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

                    leftSICache = drawing.sideIron(Drawing::LEFT).componentID;
                    leftSIInverted = drawing.sideIronInverted(Drawing::LEFT);
                    rightSICache = drawing.sideIron(Drawing::RIGHT).componentID;
                    rightSIInverted = drawing.sideIronInverted(Drawing::RIGHT);

                    drawing.removeSideIron(Drawing::LEFT);
                    drawing.removeSideIron(Drawing::RIGHT);
                } else {
                    ui->rightSideIronLabel->setEnabled(true);
                    ui->leftSideIronLabel->setText("Side Irons");
                    ui->rightSideIronLabel->setText("Different Side Irons");

                    if (leftSICache != 0) {
                        drawing.setSideIron(Drawing::LEFT,
                                            DrawingComponentManager<SideIron>::getComponentByID(leftSICache));
                        drawing.setSideIronInverted(Drawing::LEFT, leftSIInverted);
                        drawing.setSideIron(Drawing::RIGHT,
                                            DrawingComponentManager<SideIron>::getComponentByID(leftSICache));
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
                                            DrawingComponentManager<SideIron>::getComponentByID(rightSICache));
                        drawing.setSideIronInverted(Drawing::RIGHT, rightSIInverted);
                    }
                } else {
                    ui->leftSideIronLabel->setText("Side Irons");
                    ui->rightSideIronLabel->setText("Different Side Irons");

                    rightSICache = drawing.sideIron(Drawing::RIGHT).componentID;
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
        drawing.setProduct(
                DrawingComponentManager<Product>::getComponentByID(ui->productInput->itemData(index).value<ElementIndex>()));
    });
    connect(ui->dateInput, &QDateEdit::dateChanged, [this](const QDate &date) {
        drawing.setDate({ (unsigned short) date.year(), (unsigned char) date.month(), (unsigned char) date.day() });
    });

    connect(ui->widthInput, qOverload<double>(&QDoubleSpinBox::valueChanged),
            [this](double d) { drawing.setWidth((float) d); });
    connect(ui->lengthInput, qOverload<double>(&QDoubleSpinBox::valueChanged),
            [this](double d) { drawing.setLength((float) d); });
    connect(ui->topMaterialInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setMaterial(Drawing::MaterialLayer::TOP, DrawingComponentManager<Material>::getComponentByID(
                ui->topMaterialInput->itemData(index).value<ElementIndex>()));
    });
    connect(ui->bottomMaterialInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setMaterial(Drawing::MaterialLayer::BOTTOM, DrawingComponentManager<Material>::getComponentByID(
                ui->bottomMaterialInput->itemData(index).value<ElementIndex>()));
    });
    connect(ui->apertureInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setAperture(
                DrawingComponentManager<Aperture>::getComponentByID(ui->apertureInput->itemData(index).value<ElementIndex>()));
    });
    connect(ui->numberOfBarsInput, qOverload<int>(&QSpinBox::valueChanged),
            [this](int i) { ui->drawingSpecsVisual->setNumberOfBars(i); });
    connect(ui->tensionTypeInput, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        switch (index) {
            case 0:
                drawing.setTensionType(Drawing::SIDE);
                break;
            case 1:
                drawing.setTensionType(Drawing::END);
                break;
            default:
                break;
        }
    });
    connect(ui->notesInput, &QTextEdit::textChanged, [this]() {
        drawing.setNotes(ui->notesInput->toPlainText().toStdString());
    });

    connect(ui->leftSideIronDrawingInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setSideIron(Drawing::LEFT, DrawingComponentManager<SideIron>::getComponentByID(
                ui->leftSideIronDrawingInput->itemData(index).value<ElementIndex>()));
        if (!ui->rightSideIronLabel->active()) {
            drawing.setSideIron(Drawing::RIGHT, DrawingComponentManager<SideIron>::getComponentByID(
                    ui->leftSideIronDrawingInput->itemData(index).value<ElementIndex>()));
        }
    });
    connect(ui->leftSideIronInvertedInput, &QCheckBox::clicked, [this](bool checked) {
        drawing.setSideIronInverted(Drawing::LEFT, checked);
    });

    connect(ui->rightSideIronDrawingInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setSideIron(Drawing::RIGHT, DrawingComponentManager<SideIron>::getComponentByID(
                ui->leftSideIronDrawingInput->itemData(index).value<ElementIndex>()));
    });
    connect(ui->rightSideIronInvertedInput, &QCheckBox::clicked, [this](bool checked) {
        drawing.setSideIronInverted(Drawing::RIGHT, checked);
    });

    connect(ui->machineInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setMachine(DrawingComponentManager<Machine>::getComponentByID(ui->machineInput->itemData(index).value<ElementIndex>()));
    });
    connect(ui->quantityOnDeckInput, qOverload<int>(&QSpinBox::valueChanged), [this](int newValue) {
        drawing.setQuantityOnDeck(newValue);
    });
    connect(ui->machinePositionInput, &QLineEdit::textChanged, [this](const QString &newPosition) {
        drawing.setMachinePosition(newPosition.toStdString());
    });
    connect(ui->machineDeckInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setMachineDeck(DrawingComponentManager<MachineDeck>::getComponentByID(ui->machineDeckInput->itemData(index).value<ElementIndex>()));
    });

    connect(ui->openDrawingPDFButton, &QPushButton::pressed, [this]() {
        const QString hyperlinkFile = QFileDialog::getOpenFileName(this, "Select a Drawing PDF File", QString(), "PDF (*.pdf)");
        if (!hyperlinkFile.isEmpty()) {
            ui->hyperlinkDisplay->setText(hyperlinkFile);
            ui->hyperlinkDisplay->setToolTip(hyperlinkFile);

            drawing.setHyperlink(hyperlinkFile.toStdString());
        }
    });
    connect(ui->pressDrawingHyperlinksAddButton, &QPushButton::pressed, [this]() {
        const QString hyperlinkFile = QFileDialog::getOpenFileName(this, "Select a Press Program PDF File",
                                                                   QString(), "PDF (*.pdf)");
        if (!hyperlinkFile.isEmpty()) {
            ui->pressDrawingHyperlinkList->addItem(hyperlinkFile);

            std::vector<std::string> links;
            links.reserve(ui->pressDrawingHyperlinkList->count());

            for (int i = 0; i < ui->pressDrawingHyperlinkList->count(); i++) {
                links.push_back(ui->pressDrawingHyperlinkList->item(i)->text().toStdString());
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

void AddDrawingPageWidget::setDrawing(const Drawing &newDrawing) {
    drawing = newDrawing;
}

void AddDrawingPageWidget::confirmDrawing() {
    switch (drawing.checkDrawingValidity()) {
        case Drawing::SUCCESS:
            switch (addMode) {
                case ADD_NEW_DRAWING:
                    confirmationCallback(drawing, false);
                    break;
                case EDIT_DRAWING:
                    switch (QMessageBox::question(this, "Confirm Update", "Are you sure you wish to update this drawing?")) {
                        case QMessageBox::Yes:
                            confirmationCallback(drawing, true);
                            break;
                        default:
                            break;
                    }
                    break;
            }
            break;
        case Drawing::INVALID_DRAWING_NUMBER:
            QMessageBox::about(this, "Drawing Error", "Invalid Drawing Number");
            break;
        case Drawing::INVALID_PRODUCT:
            QMessageBox::about(this, "Drawing Error", "Invalid Product selected");
            break;
        case Drawing::INVALID_WIDTH:
            QMessageBox::about(this, "Drawing Error", "Invalid width. Make sure the width is positive");
            break;
        case Drawing::INVALID_LENGTH:
            QMessageBox::about(this, "Drawing Error", "Invalid length. Make sure the length is positive");
            break;
        case Drawing::INVALID_TOP_MATERIAL:
            QMessageBox::about(this, "Drawing Error", "Invalid Top Layer Material");
            break;
        case Drawing::INVALID_BOTTOM_MATERIAL:
            QMessageBox::about(this, "Drawing Error", "Invalid Bottom Layer Material");
            break;
        case Drawing::INVALID_APERTURE:
            QMessageBox::about(this, "Drawing Error", "Invalid Aperture");
            break;
        case Drawing::INVALID_BAR_SPACINGS:
            QMessageBox::about(this, "Drawing Error", "Invalid Bar Spacings. Make sure they add to the width");
            break;
        case Drawing::INVALID_BAR_WIDTHS:
            QMessageBox::about(this, "Drawing Error", "Invalid Bar Widths. Make sure they are all positive and non-zero");
            break;
        case Drawing::INVALID_SIDE_IRONS:
            QMessageBox::about(this, "Drawing Error", "Invalid Side Irons");
            break;
        case Drawing::INVALID_MACHINE:
            QMessageBox::about(this, "Drawing Error", "Invalid Machine");
            break;
        case Drawing::INVALID_MACHINE_POSITION:
            QMessageBox::about(this, "Drawing Error", "Invalid Machine position");
            break;
        case Drawing::INVALID_MACHINE_DECK:
            QMessageBox::about(this, "Drawing Error", "Invalid Machine Deck");
            break;
        case Drawing::INVALID_HYPERLINK:
            QMessageBox::about(this, "Drawing Error", "Invalid Drawing PDF Hyperlink");
            break;
    }
}

void AddDrawingPageWidget::setConfirmationCallback(const std::function<void(const Drawing &, bool)> &callback) {
    confirmationCallback = callback;
}

void AddDrawingPageWidget::setMode(AddDrawingPageWidget::AddDrawingMode mode) {
    addMode = mode;

    switch (addMode) {
        case ADD_NEW_DRAWING:
            ui->confirmDrawingButton->setText("Add Drawing to Database");
            break;
        case EDIT_DRAWING:
            ui->confirmDrawingButton->setText("Update Drawing");
            break;
    }
}

void AddDrawingPageWidget::capitaliseLineEdit(const QString &text) {
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (!lineEdit) {
        return;
    }
    lineEdit->setText(text.toUpper());
}

#pragma clang diagnostic pop