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

    setupActivators();
    setupComboboxSources();

    visualsScene = new QGraphicsScene();
    ui->drawingSpecsVisual->setScene(visualsScene);
    ui->drawingSpecsVisual->setDrawing(drawing);

    // TODO: Machine position regex

    QRegExpValidator *drawingNumberValidator = new QRegExpValidator(drawingNumberRegex);

    ui->drawingNumberInput->setValidator(drawingNumberValidator);
    connect(ui->drawingNumberInput, SIGNAL(textEdited(const QString &)), SLOT(capitaliseLineEdit(const QString &)));

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
                                ui->bottomMaterialInput->currentData().toInt()));
                    }
                }
            }
    );

    ui->overlapsLabel->addTarget(ui->overlapTypeInput);
    ui->sidelapsLabel->addTarget(ui->sidelapTypeInput);

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
                } else {
                    ui->rightSideIronLabel->setEnabled(true);
                    ui->leftSideIronLabel->setText("Side Irons");
                    ui->rightSideIronLabel->setText("Different Side Irons");
                }
            }
    );

    ui->rightSideIronLabel->addActivationCallback(
            [this](bool active) {
                if (active) {
                    ui->leftSideIronLabel->setText("Left Side Iron");
                    ui->rightSideIronLabel->setText("Right Side Iron");
                } else {
                    ui->leftSideIronLabel->setText("Side Irons");
                    ui->rightSideIronLabel->setText("Different Side Irons");
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
        drawing.setProduct(DrawingComponentManager<Product>::getComponentByID(ui->productInput->itemData(index).toInt()));
    });

    connect(ui->widthInput, qOverload<double>(&QDoubleSpinBox::valueChanged),
            [this](double d) { drawing.setWidth((float) d); });
    connect(ui->lengthInput, qOverload<double>(&QDoubleSpinBox::valueChanged),
            [this](double d) { drawing.setLength((float) d); });
    connect(ui->topMaterialInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setMaterial(Drawing::MaterialLayer::TOP, DrawingComponentManager<Material>::getComponentByID(
                ui->topMaterialInput->itemData(index).toInt()));
    });
    connect(ui->bottomMaterialInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setMaterial(Drawing::MaterialLayer::BOTTOM, DrawingComponentManager<Material>::getComponentByID(
                ui->bottomMaterialInput->itemData(index).toInt()));
    });
    connect(ui->apertureInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setAperture(DrawingComponentManager<Aperture>::getComponentByID(ui->apertureInput->itemData(index).toInt()));
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
}

void AddDrawingPageWidget::capitaliseLineEdit(const QString &text) {
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (!lineEdit) {
        return;
    }
    lineEdit->setText(text.toUpper());
}

#pragma clang diagnostic pop