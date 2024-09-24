//
// Created by matthew on 14/07/2020.
//

#include "AddDrawingPageWidget.h"

#include "../build/ui_AddDrawingPageWidget.h"

AddDrawingPageWidget::AddDrawingPageWidget(const std::string &drawingNumber,
                                           bool automatic, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::AddDrawingPageWidget()),
      autopress(automatic) {
  ui->setupUi(this);

  inspector = new Inspector("Inspector");
  inspector->addUpdateTrigger(
      [this]() { ui->drawingSpecsVisual->setRedrawRequired(); });
  ui->drawingSpecsVisual->setInspector(inspector);

  ui->visualAndInspectorVerticalLayout->addWidget(inspector->container());

  ui->drawingNumberInput->setText(drawingNumber.c_str());

  drawing.setAsDefault();

  drawing.setDrawingNumber(drawingNumber);
  drawing.setAperture(DrawingComponentManager<Aperture>::findComponentByID(1));
  drawing.setProduct(DrawingComponentManager<Product>::findComponentByID(1));
  drawing.setMaterial(Drawing::TOP,
                      DrawingComponentManager<Material>::findComponentByID(1));

  Date today = Date::today();
  drawing.setDate(today);

  ui->dateInput->setDate(QDate(today.year, today.month, today.day));

  leftSideIronFilter = leftSideIronSource.setFilter<SideIronFilter>();
  rightSideIronFilter = rightSideIronSource.setFilter<SideIronFilter>();

  setupActivators();
  setupComboboxSources();
  setupDrawingUpdateConnections();

  visualsScene = new QGraphicsScene();
  ui->drawingSpecsVisual->setScene(visualsScene);
  ui->drawingSpecsVisual->setDrawing(drawing);

  QRegularExpressionValidator *drawingNumberValidator =
      new QRegularExpressionValidator(drawingNumberRegex);

  ui->drawingNumberInput->setValidator(drawingNumberValidator);
  connect(ui->drawingNumberInput, SIGNAL(textEdited(const QString &)), this,
          SLOT(capitaliseLineEdit(const QString &)));

  QRegularExpressionValidator *positionValidator =
      new QRegularExpressionValidator(
          QRegularExpression("(^$)|(^[0-9]+([-][0-9]+)?$)|(^[Aa][Ll]{2}$)"));

  ui->machinePositionInput->setValidator(positionValidator);
  connect(ui->machinePositionInput, SIGNAL(textEdited(const QString &)), this,
          SLOT(capitaliseLineEdit(const QString &)));

  setMode(ADD_NEW_DRAWING);
}

AddDrawingPageWidget::AddDrawingPageWidget(
    const Drawing &drawing, AddDrawingPageWidget::AddDrawingMode mode,
    bool autopress, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::AddDrawingPageWidget()),
      autopress(autopress) {
  ui->setupUi(this);

  inspector = new Inspector("Inspector");
  inspector->addUpdateTrigger(
      [this]() { ui->drawingSpecsVisual->setRedrawRequired(); });
  ui->drawingSpecsVisual->setInspector(inspector);

  ui->visualAndInspectorVerticalLayout->addWidget(inspector->container());

  this->drawing = drawing;
  // if (mode == CLONE_DRAWING) {
  //    this->drawing.setDrawingNumber("");
  //}

  leftSideIronFilter = leftSideIronSource.setFilter<SideIronFilter>();
  rightSideIronFilter = rightSideIronSource.setFilter<SideIronFilter>();

  if (drawing.sideIron(Drawing::Side::LEFT).type != SideIronType::None) {
    leftSideIronFilter->setSideIronType(
        drawing.sideIron(Drawing::Side::LEFT).type);
    ui->leftSideIronLabel->setActive();
  } else {
    leftSideIronFilter->setSideIronType(SideIronType::A);
    ui->leftSideIronLabel->setActive(false);
  }
  if (drawing.sideIron(Drawing::Side::RIGHT).type != SideIronType::None) {
    rightSideIronFilter->setSideIronType(
        drawing.sideIron(Drawing::Side::RIGHT).type);
    ui->rightSideIronLabel->setActive();
  } else {
    rightSideIronFilter->setSideIronType(SideIronType::A);
    ui->rightSideIronLabel->setActive(false);
  }

  setupActivators();
  setupComboboxSources();
  setupDrawingUpdateConnections();

  visualsScene = new QGraphicsScene();
  ui->drawingSpecsVisual->setScene(visualsScene);
  ui->drawingSpecsVisual->setDrawing(this->drawing);

  QRegularExpressionValidator *drawingNumberValidator =
      new QRegularExpressionValidator(drawingNumberRegex);

  ui->drawingNumberInput->setValidator(drawingNumberValidator);
  connect(ui->drawingNumberInput, SIGNAL(textEdited(const QString &)), this,
          SLOT(capitaliseLineEdit(const QString &)));

  QRegularExpressionValidator *positionValidator =
      new QRegularExpressionValidator(
          QRegularExpression("(^$)|(^[0-9]+([-][0-9]+)?$)|(^[Aa][Ll]{2}$)"));

  ui->machinePositionInput->setValidator(positionValidator);
  connect(ui->machinePositionInput, SIGNAL(textEdited(const QString &)), this,
          SLOT(capitaliseLineEdit(const QString &)));

  setMode(mode);

  loadDrawing();
}

AddDrawingPageWidget::~AddDrawingPageWidget() {
  delete inspector;
  delete visualsScene;
}

void AddDrawingPageWidget::setupComboboxSources() {
  std::function<bool(const SideIron &, const SideIron &)> sideIronComparator =
      [](const SideIron &a, const SideIron &b) { return a.length < b.length; };

  DrawingComponentManager<Product>::addCallback(
      [this]() { productSource.updateSource(); });
  DrawingComponentManager<Aperture>::addCallback([this]() {
    apertureSource.updateSource();
    apertureSource.sort(Aperture::apertureComparator);
  });
  DrawingComponentManager<Material>::addCallback(
      [this]() { topMaterialSource.updateSource(); });
  DrawingComponentManager<Material>::addCallback(
      [this]() { bottomMaterialSource.updateSource(); });
  DrawingComponentManager<SideIron>::addCallback([this, sideIronComparator]() {
    leftSideIronSource.updateSource();
    leftSideIronSource.sort(sideIronComparator);
  });
  DrawingComponentManager<SideIron>::addCallback([this, sideIronComparator]() {
    rightSideIronSource.updateSource();
    rightSideIronSource.sort(sideIronComparator);
  });
  DrawingComponentManager<Machine>::addCallback([this]() {
    machineManufacturerSource.updateSource();
    machineManufacturerSource.makeDistinct();
    machineModelSource.updateSource();
  });
  DrawingComponentManager<MachineDeck>::addCallback(
      [this]() { machineDeckSource.updateSource(); });
  DrawingComponentManager<BackingStrip>::addCallback(
      [this]() { backingStripSource.updateSource(); });

  DrawingComponentManager<Strap>::addCallback(
      [this]() { strapSource.updateSource(); });

  machineManufacturerSource.setMode(1);
  machineModelSource.setMode(2);

  productSource.updateSource();
  apertureSource.updateSource();
  topMaterialSource.updateSource();
  bottomMaterialSource.updateSource();
  leftSideIronSource.updateSource();
  leftSideIronSource.sort(sideIronComparator);
  rightSideIronSource.updateSource();
  rightSideIronSource.sort(sideIronComparator);
  machineManufacturerSource.updateSource();
  machineManufacturerSource.makeDistinct();
  machineModelSource.updateSource();
  machineDeckSource.updateSource();
  backingStripSource.updateSource();
  strapSource.updateSource();

  machineModelFilter = machineModelSource.setFilter<MachineModelFilter>();

  apertureSource.sort(Aperture::apertureComparator);

  ui->productInput->setDataSource(productSource);
  ui->apertureInput->setDataSource(apertureSource);
  ui->topMaterialInput->setDataSource(topMaterialSource);
  ui->bottomMaterialInput->setDataSource(bottomMaterialSource);
  ui->leftSideIronDrawingInput->setDataSource(leftSideIronSource);
  ui->rightSideIronDrawingInput->setDataSource(rightSideIronSource);
  ui->manufacturerInput->setDataSource(machineManufacturerSource);
  ui->modelInput->setDataSource(machineModelSource);
  ui->machineDeckInput->setDataSource(machineDeckSource);
  ui->backingStripsInput->setDataSource(backingStripSource);
  ui->leftStrapInput->setDataSource(strapSource);
  ui->rightStrapInput->setDataSource(strapSource);
}

void AddDrawingPageWidget::setupActivators() {
  ui->bottomMaterialLabel->addTarget(ui->bottomMaterialInput);
  ui->bottomMaterialLabel->addActivationCallback([this](bool active) {
    if (!active) {
      drawing.removeBottomLayer();
    } else {
      if (ui->bottomMaterialInput->currentIndex() != -1) {
        drawing.setMaterial(
            Drawing::BOTTOM,
            DrawingComponentManager<Material>::getComponentByHandle(
                ui->bottomMaterialInput->currentData().toInt()));
      }
    }
  });

  ui->backingStripsLabel->addTarget(ui->backingStripsInput);
  ui->backingStripsLabel->addActivationCallback([this](bool active) {
    if (!active) {
      drawing.removeBackingStrip();
    } else {
      if (ui->backingStripsInput->currentIndex() != -1) {
        drawing.setBackingStrip(
            DrawingComponentManager<BackingStrip>::getComponentByHandle(
                ui->backingStripsInput->currentData().toInt()));
      }
    }
  });
  ui->leftSideIronLabel->setActive();
  ui->backingStripsLabel->setActive(drawing.hasBackingStrips());
  ui->leftSideIronLabel->addTarget(ui->leftSideIronTypeLabel);
  ui->leftSideIronLabel->addTarget(ui->leftSideIronTypeInput);
  ui->leftSideIronLabel->addTarget(ui->leftSideIronLengthLabel);
  ui->leftSideIronLabel->addTarget(ui->leftSideIronLengthInput);
  ui->leftSideIronLabel->addTarget(ui->leftSideIronDrawingLabel);
  ui->leftSideIronLabel->addTarget(ui->leftSideIronDrawingInput);
  ui->leftSideIronLabel->addTarget(ui->leftSideIronInvertedLabel);
  ui->leftSideIronLabel->addTarget(ui->leftSideIronInvertedInput);
  ui->leftSideIronLabel->addTarget(ui->leftSideIronCutDownLabel);
  ui->leftSideIronLabel->addTarget(ui->leftSideIronCutDownInput);
  ui->leftStrapLabel->addTarget(ui->leftStrapInput);
  ui->leftStrapLabel->addActivationCallback([this](bool active) {
    if (!active) {
      drawing.removeSideIronStrap(Drawing::LEFT);
    } else {
        drawing.setSideIronStrap(
          Drawing::LEFT, DrawingComponentManager<Strap>::getComponentByHandle(
                             ui->leftStrapInput->currentData().toInt()));
    }
  });

  ui->rightSideIronLabel->addTarget(ui->rightSideIronTypeLabel);
  ui->rightSideIronLabel->addTarget(ui->rightSideIronTypeInput);
  ui->rightSideIronLabel->addTarget(ui->rightSideIronLengthLabel);
  ui->rightSideIronLabel->addTarget(ui->rightSideIronLengthInput);
  ui->rightSideIronLabel->addTarget(ui->rightSideIronDrawingLabel);
  ui->rightSideIronLabel->addTarget(ui->rightSideIronDrawingInput);
  ui->rightSideIronLabel->addTarget(ui->rightSideIronInvertedLabel);
  ui->rightSideIronLabel->addTarget(ui->rightSideIronInvertedInput);
  ui->rightSideIronLabel->addTarget(ui->rightSideIronCutDownLabel);
  ui->rightSideIronLabel->addTarget(ui->rightSideIronCutDownInput);
  ui->rightStrapLabel->addTarget(ui->rightStrapInput);
  ui->rightStrapLabel->addActivationCallback([this](bool active) {
    if (!active) {
      drawing.removeSideIronStrap(Drawing::RIGHT);
    } else {
      drawing.setSideIronStrap(
          Drawing::RIGHT, DrawingComponentManager<Strap>::getComponentByHandle(
                             ui->rightStrapInput->currentData().toInt()));
    }
  });

  ui->leftSideIronLabel->addActivationCallback([this](bool active) {
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
        drawing.setSideIron(
            Drawing::LEFT,
            DrawingComponentManager<SideIron>::getComponentByHandle(
                leftSICache));
        drawing.setSideIronInverted(Drawing::LEFT, leftSIInverted);
        drawing.setSideIron(
            Drawing::RIGHT,
            DrawingComponentManager<SideIron>::getComponentByHandle(
                leftSICache));
        drawing.setSideIronInverted(Drawing::RIGHT, leftSIInverted);
      }
    }
  });

  ui->rightSideIronLabel->addActivationCallback([this](bool active) {
    if (active) {
      ui->leftSideIronLabel->setText("Left Side Iron");
      ui->rightSideIronLabel->setText("Right Side Iron");

      if (rightSICache != 0) {
        drawing.setSideIron(
            Drawing::RIGHT,
            DrawingComponentManager<SideIron>::getComponentByHandle(
                rightSICache));
        drawing.setSideIronInverted(Drawing::RIGHT, rightSIInverted);
      }
    } else {
      ui->leftSideIronLabel->setText("Side Irons");
      ui->rightSideIronLabel->setText("Different Side Irons");

      rightSICache = drawing.sideIron(Drawing::RIGHT).handle();
      drawing.setSideIron(Drawing::RIGHT, drawing.sideIron(Drawing::LEFT));
    }
  });

  ui->manufacturerLabel->setActive();
  ui->manufacturerLabel->addTarget(ui->manufacturerInput);
  ui->manufacturerLabel->addTarget(ui->modelLabel);
  ui->manufacturerLabel->addTarget(ui->modelInput);
  ui->manufacturerLabel->addTarget(ui->quantityOnDeckLabel);
  ui->manufacturerLabel->addTarget(ui->quantityOnDeckInput);
  ui->manufacturerLabel->addTarget(ui->machinePositionLabel);
  ui->manufacturerLabel->addTarget(ui->machinePositionInput);
  ui->manufacturerLabel->addTarget(ui->machineDeckLabel);
  ui->manufacturerLabel->addTarget(ui->machineDeckInput);

  ui->manufacturerLabel->addActivationCallback([this](bool active) {
    if (!active) {
      ui->manufacturerInput->setCurrentIndex(0);
    }
  });
}

void AddDrawingPageWidget::setupDrawingUpdateConnections() {
  connect(ui->drawingNumberInput, &QLineEdit::textChanged,
          [this](const QString &text) {
            drawing.setDrawingNumber(text.toStdString());
          });
  connect(ui->leftSideIronFeedEndInput, &QCheckBox::stateChanged,
          [this](int state) {
            ui->rightSideIronFeedEndInput->blockSignals(true);
            if (state) {
              ui->rightSideIronFeedEndInput->setChecked(!state);

              drawing.setSideIronFeed(Drawing::LEFT);
            } else {
              drawing.removeSideIronFeed(Drawing::LEFT);
            }
            ui->rightSideIronFeedEndInput->blockSignals(false);
          });
  connect(ui->rightSideIronFeedEndInput, &QCheckBox::stateChanged,
          [this](int state) {
            ui->leftSideIronFeedEndInput->blockSignals(true);
            if (state) {
              ui->leftSideIronFeedEndInput->setChecked(!state);

              drawing.setSideIronFeed(Drawing::RIGHT);
            } else {
              drawing.removeSideIronFeed(Drawing::RIGHT);
            }
            ui->leftSideIronFeedEndInput->blockSignals(false);
          });
  connect(ui->leftSideIronFixedEndInput,
          qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
            ui->rightSideIronFixedEndInput->blockSignals(true);
            if (index == 0) {
              ui->rightSideIronFixedEndInput->setCurrentIndex(0);
              drawing.removeSideIronEnding(Drawing::LEFT);
              drawing.removeSideIronEnding(Drawing::RIGHT);
            } else {
              ui->rightSideIronFixedEndInput->setCurrentIndex(
                  (!(index - 1)) + 1);  // reverses 1 and 2
              boxesValues[1] = (!(index - 1)) + 1;

              drawing.setSideIronEnding(Drawing::LEFT, (Drawing::Ending)index);
              drawing.setSideIronEnding(Drawing::RIGHT,
                                        (Drawing::Ending)((!(index - 1)) + 1));
            }
            ui->rightSideIronFixedEndInput->blockSignals(false);
          });
  connect(ui->rightSideIronFixedEndInput,
          qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
            ui->leftSideIronFixedEndInput->blockSignals(true);
            if (index == 0) {
              ui->leftSideIronFixedEndInput->setCurrentIndex(0);
              drawing.removeSideIronEnding(Drawing::LEFT);
              drawing.removeSideIronEnding(Drawing::RIGHT);
            } else {
              ui->leftSideIronFixedEndInput->setCurrentIndex(
                  (!(index - 1)) + 1);  // reverses 1 and 2

              drawing.setSideIronEnding(Drawing::RIGHT, (Drawing::Ending)index);
              drawing.setSideIronEnding(Drawing::LEFT,
                                        (Drawing::Ending)((!(index - 1)) + 1));
            }
            ui->leftSideIronFixedEndInput->blockSignals(false);
          });
  connect(
      ui->leftSideIronHookOriInput,
      qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        ui->rightSideIronHookOriInput->blockSignals(true);
        if (boxesValues[2] == 0) {
          ui->rightSideIronHookOriInput->setCurrentIndex((!(index - 1)) + 1);
          boxesValues[3] = (!(index - 1)) + 1;

          drawing.setSideIronHookOrientation(Drawing::LEFT,
                                             (Drawing::HookOrientation)index);
          drawing.setSideIronHookOrientation(
              Drawing::RIGHT, (Drawing::HookOrientation)((!(index - 1)) + 1));
        }
        boxesValues[2] = index;
        if (index == 0) {
          drawing.removeSideIronHookOrientation(Drawing::LEFT);
        } else if (index == 1) {  // Cannot have 2 hook ups.
          ui->rightSideIronHookOriInput->setCurrentIndex(2);
          boxesValues[2] = 1;
          boxesValues[3] = 2;
          drawing.setSideIronHookOrientation(Drawing::LEFT, Drawing::HOOK_UP);
          drawing.setSideIronHookOrientation(Drawing::RIGHT,
                                             Drawing::HOOK_DOWN);
        }
        ui->rightSideIronHookOriInput->blockSignals(false);
      });
  connect(
      ui->rightSideIronHookOriInput,
      qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        ui->leftSideIronHookOriInput->blockSignals(true);
        if (boxesValues[3] == 0) {
          ui->leftSideIronHookOriInput->setCurrentIndex((!(index - 1)) + 1);
          boxesValues[2] = (!(index - 1)) + 1;

          drawing.setSideIronHookOrientation(Drawing::RIGHT,
                                             (Drawing::HookOrientation)index);
          drawing.setSideIronHookOrientation(
              Drawing::LEFT, (Drawing::HookOrientation)((!(index - 1)) + 1));
        }
        boxesValues[3] = index;
        if (index == 0) {
          drawing.removeSideIronHookOrientation(Drawing::RIGHT);
        } else if (index == 1) {  // Cannot have 2 hook ups.
          ui->leftSideIronHookOriInput->setCurrentIndex(2);
          boxesValues[2] = 2;
          boxesValues[3] = 1;
          drawing.setSideIronHookOrientation(Drawing::RIGHT, Drawing::HOOK_UP);
          drawing.setSideIronHookOrientation(Drawing::LEFT, Drawing::HOOK_DOWN);
        }
        ui->leftSideIronHookOriInput->blockSignals(false);
      });
  connect(
      ui->leftStrapInput, qOverload<int>(&DynamicComboBox::currentIndexChanged),
      [this](int index) {
        drawing.setSideIronStrap(
            Drawing::LEFT, DrawingComponentManager<Strap>::getComponentByHandle(
                               ui->leftStrapInput->itemData(index).toInt()));
      });
  connect(ui->rightStrapInput,
          qOverload<int>(&DynamicComboBox::currentIndexChanged),
          [this](int index) {
            drawing.setSideIronStrap(
                Drawing::RIGHT,
                DrawingComponentManager<Strap>::getComponentByHandle(
                    ui->rightStrapInput->itemData(index).toInt()));
          });
  connect(ui->productInput,
          qOverload<int>(&DynamicComboBox::currentIndexChanged),
          [this](int index) {
            Product &product =
                DrawingComponentManager<Product>::getComponentByHandle(
                    ui->productInput->itemData(index).toInt());
            if (product.handle() == drawing.product().handle()) {
              return;
            }

            if (product.productName == "Rubber Screen Cloth") {
              topMaterialSource.setFilter<RubberScreenClothMaterialFilter>();
              ui->bottomMaterialLabel->setActive(false);
              ui->bottomMaterialLabel->setEnabled(false);

              ui->drawingSpecsVisual->enableLaps();

              ui->tensionTypeInput->setEnabled(true);

              ui->numberOfBarsInput->setEnabled(true);

              leftSideIronFilter->setSideIronFilterMatType(false);

              // if (!ui->leftSideIronLabel->active()) {
              //     ui->leftSideIronLabel->setActive(true);
              //     ui->leftSideIronLabel->setDisabled(false);
              // }
            } else if (product.productName == "Extraflex") {
              topMaterialSource.setFilter<TackyBackMaterialFilter>();
              bottomMaterialSource.setFilter<FlexBottomMaterialFilter>();
              ui->bottomMaterialLabel->setActive(true);
              ui->bottomMaterialLabel->setEnabled(true);

              ui->drawingSpecsVisual->enableLaps();

              ui->tensionTypeInput->setEnabled(true);

              ui->numberOfBarsInput->setEnabled(true);
              this->leftSideIronFilter->setSideIronFilterMatType(true);

              // if (!ui->leftSideIronLabel->active()) {
              //     ui->leftSideIronLabel->setActive(true);
              //     ui->leftSideIronLabel->setDisabled(false);
              // }
            } else if (product.productName == "Polyflex") {
              topMaterialSource.setFilter<PolyurethaneMaterialFilter>();
              bottomMaterialSource.setFilter<FlexBottomMaterialFilter>();
              ui->bottomMaterialLabel->setActive(true);
              ui->bottomMaterialLabel->setEnabled(true);

              ui->drawingSpecsVisual->enableLaps();

              ui->tensionTypeInput->setEnabled(true);

              ui->numberOfBarsInput->setEnabled(true);

              leftSideIronFilter->setSideIronFilterMatType(true);

              // if (!ui->leftSideIronLabel->active()) {
              //     ui->leftSideIronLabel->setActive(true);
              //     ui->leftSideIronLabel->setDisabled(false);
              // }
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

              leftSideIronFilter->removeMatType();

              // if (ui->leftSideIronLabel->active()) {
              //     ui->leftSideIronLabel->setActive(false);
              //     ui->leftSideIronLabel->setDisabled(true);
              // }
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

              leftSideIronFilter->removeMatType();

              // if (ui->leftSideIronLabel->active()) {
              //     ui->leftSideIronLabel->setActive(false);
              //     ui->leftSideIronLabel->setDisabled(true);
              // }
            } else if (product.productName == "Rubber Modules and Panels") {
              topMaterialSource.setFilter<RubberModuleMaterialFilter>();
              ui->bottomMaterialLabel->setActive(false);
              ui->bottomMaterialLabel->setEnabled(false);

              ui->drawingSpecsVisual->enableLaps();

              ui->tensionTypeInput->setEnabled(true);

              ui->numberOfBarsInput->setEnabled(true);

              leftSideIronFilter->removeMatType();

              // if (ui->leftSideIronLabel->active()) {
              //     ui->leftSideIronLabel->setActive(false);
              //     ui->leftSideIronLabel->setDisabled(true);
              // }
            } else {
              topMaterialSource.removeFilter();
              bottomMaterialSource.removeFilter();
              ui->bottomMaterialLabel->setActive(false);
              ui->bottomMaterialLabel->setEnabled(true);

              ui->drawingSpecsVisual->enableLaps();

              ui->tensionTypeInput->setEnabled(true);

              ui->numberOfBarsInput->setEnabled(true);

              leftSideIronFilter->removeMatType();

              // if (ui->leftSideIronLabel->active()) {
              //     ui->leftSideIronLabel->setActive(false);
              //     ui->leftSideIronLabel->setDisabled(true);
              // }
            }

            drawing.setProduct(product);
          });
  connect(ui->dateInput, &QDateEdit::dateChanged, [this](const QDate &date) {
    drawing.setDate({(unsigned short)date.year(), (unsigned char)date.month(),
                     (unsigned char)date.day()});
  });

  connect(ui->widthInput, qOverload<double>(&QDoubleSpinBox::valueChanged),
          [this](double d) { drawing.setWidth((float)d); });
  connect(ui->lengthInput, qOverload<double>(&QDoubleSpinBox::valueChanged),
          [this](double d) {
            drawing.setLength((float)d);
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
  connect(ui->topMaterialInput,
          qOverload<int>(&DynamicComboBox::currentIndexChanged),
          [this](int index) {
            drawing.setMaterial(
                Drawing::MaterialLayer::TOP,
                DrawingComponentManager<Material>::getComponentByHandle(
                    ui->topMaterialInput->itemData(index).toInt()));
          });
  connect(ui->bottomMaterialInput,
          qOverload<int>(&DynamicComboBox::currentIndexChanged),
          [this](int index) {
            drawing.setMaterial(
                Drawing::MaterialLayer::BOTTOM,
                DrawingComponentManager<Material>::getComponentByHandle(
                    ui->bottomMaterialInput->itemData(index).toInt()));
          });
  connect(ui->backingStripsInput,
          qOverload<int>(&DynamicComboBox::currentIndexChanged),
          [this](int index) {
            drawing.setBackingStrip(
                DrawingComponentManager<BackingStrip>::getComponentByHandle(
                    ui->backingStripsInput->itemData(index).toInt()));
          });
  connect(
      ui->apertureInput, qOverload<int>(&DynamicComboBox::currentIndexChanged),
      [this](int index) {
        Aperture &ap = DrawingComponentManager<Aperture>::getComponentByHandle(
            ui->apertureInput->itemData(index).toInt());
        if (ap.width <= 100 || !autopress) {
          drawing.setAperture(ap);
          return;
        }
        QMessageBox::StandardButton button = QMessageBox::warning(
            this, "Incorrect Aperture",
            "The aperture you have chosen cannot be used on an autopress "
            "drawing. Are you sure you want to set this?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (button == QMessageBox::Yes) {
          drawing.setAperture(ap);
        } else {
          ui->apertureInput->setCurrentIndex(
              ui->apertureInput->findData(drawing.aperture().handle()));
        }
      });
  connect(ui->numberOfBarsInput, qOverload<int>(&QSpinBox::valueChanged),
          [this](int i) { ui->drawingSpecsVisual->setNumberOfBars(i); });
  connect(ui->tensionTypeInput, qOverload<int>(&QComboBox::currentIndexChanged),
          [this](int index) {
            switch (index) {
              case 0:
                drawing.setTensionType(Drawing::SIDE);
                if (leftSICache == 0) {
                  ui->leftSideIronTypeInput->setCurrentIndex(
                      (int)SideIronType::A - 1);
                }
                if (rightSICache == 0) {
                  ui->rightSideIronTypeInput->setCurrentIndex(
                      (int)SideIronType::A - 1);
                }
                break;
              case 1:
                drawing.setTensionType(Drawing::END);
                if (leftSICache == 0) {
                  ui->leftSideIronTypeInput->setCurrentIndex(
                      (int)SideIronType::C - 1);
                }
                if (rightSICache == 0) {
                  ui->rightSideIronTypeInput->setCurrentIndex(
                      (int)SideIronType::C - 1);
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

  connect(ui->leftSideIronTypeInput,
          qOverload<int>(&DynamicComboBox::currentIndexChanged),
          [this](int index) {
            if (leftSideIronFilter) {
              leftSideIronFilter->setSideIronType((SideIronType)(index + 1));
            }
          });

  connect(ui->leftSideIronLengthInput, qOverload<int>(&QSpinBox::valueChanged),
          [this](int value) {
            if (leftSideIronFilter) {
              leftSideIronFilter->setSideIronFilterMinimumLength(value);
            }
          });

  connect(ui->leftSideIronDrawingInput,
          qOverload<int>(&DynamicComboBox::currentIndexChanged),
          [this](int index) {
            drawing.setSideIron(
                Drawing::LEFT,
                DrawingComponentManager<SideIron>::getComponentByHandle(
                    ui->leftSideIronDrawingInput->itemData(index).toInt()));
            leftSICache = ui->leftSideIronDrawingInput->itemData(index).toInt();
            if (!ui->rightSideIronLabel->active()) {
              drawing.setSideIron(
                  Drawing::RIGHT,
                  DrawingComponentManager<SideIron>::getComponentByHandle(
                      ui->leftSideIronDrawingInput->itemData(index).toInt()));
              rightSICache = leftSICache;
            }
          });
  connect(ui->leftSideIronInvertedInput, &QCheckBox::clicked,
          [this](bool checked) {
            drawing.setSideIronInverted(Drawing::LEFT, checked);
          });

  connect(ui->leftSideIronCutDownInput, &QCheckBox::clicked,
          [this](bool checked) {
            drawing.setSideIronCutDown(Drawing::LEFT, checked);
          });

  connect(ui->rightSideIronTypeInput,
          qOverload<int>(&DynamicComboBox::currentIndexChanged),
          [this](int index) {
            if (rightSideIronFilter) {
              rightSideIronFilter->setSideIronType((SideIronType)(index + 1));
            }
          });

  connect(ui->rightSideIronLengthInput, qOverload<int>(&QSpinBox::valueChanged),
          [this](int value) {
            if (rightSideIronFilter) {
              rightSideIronFilter->setSideIronFilterMinimumLength(value);
            }
          });

  connect(
      ui->rightSideIronDrawingInput,
      qOverload<int>(&DynamicComboBox::currentIndexChanged), [this](int index) {
        drawing.setSideIron(
            Drawing::RIGHT,
            DrawingComponentManager<SideIron>::getComponentByHandle(
                ui->rightSideIronDrawingInput->itemData(index).toInt()));

        rightSICache = ui->rightSideIronDrawingInput->itemData(index).toInt();
      });
  connect(ui->rightSideIronInvertedInput, &QCheckBox::clicked,
          [this](bool checked) {
            drawing.setSideIronInverted(Drawing::RIGHT, checked);
          });

  connect(ui->rightSideIronCutDownInput, &QCheckBox::clicked,
          [this](bool checked) {
            drawing.setSideIronCutDown(Drawing::RIGHT, checked);
          });

  connect(ui->manufacturerInput, &DynamicComboBox::currentTextChanged,
          [this](const QString &text) {
            machineModelFilter->setManufacturer(text.toStdString());
            ui->modelInput->updateSourceList();
            drawing.setMachine(
                DrawingComponentManager<Machine>::getComponentByHandle(
                    ui->modelInput->itemData(0).toInt()));
          });

  connect(ui->modelInput, qOverload<int>(&DynamicComboBox::currentIndexChanged),
          [this](int index) {
            drawing.setMachine(
                DrawingComponentManager<Machine>::getComponentByHandle(
                    ui->modelInput->itemData(index).toInt()));
          });
  connect(ui->quantityOnDeckInput, qOverload<int>(&QSpinBox::valueChanged),
          [this](int newValue) { drawing.setQuantityOnDeck(newValue); });
  connect(ui->machinePositionInput, &QLineEdit::textChanged,
          [this](const QString &newPosition) {
            drawing.setMachinePosition(newPosition.toUpper().toStdString());
          });
  connect(ui->machineDeckInput,
          qOverload<int>(&DynamicComboBox::currentIndexChanged),
          [this](int index) {
            drawing.setMachineDeck(
                DrawingComponentManager<MachineDeck>::getComponentByHandle(
                    ui->machineDeckInput->itemData(index).toInt()));
          });

  connect(ui->browseDrawingPDFButton, &QPushButton::pressed, [this]() {
    QString start;
    if (drawing.hyperlink() == "") {
        if (autopress) {
            std::regex re("\w{1,2}");
            std::cmatch m;
            std::regex_search(drawing.drawingNumber().c_str(), m, re);
            std::string initial = m[0].str();
            if (initial.length() == 1)
              initial = "1" + initial;
            start = (AUTOPRESS_LOCATION + initial).c_str();
        } else {
            start = MANUAL_LOCATION;
        }
    }
    else {
        start = drawing.hyperlink().root_directory().generic_string().c_str();
    }
    const std::filesystem::path hyperlinkFile =
        QFileDialog::getOpenFileName(this, "Select a Drawing PDF File",
                                     start, "PDF (*.pdf)")
            .toStdString();
    if (!hyperlinkFile.empty()) {
      ui->hyperlinkDisplay->setText(hyperlinkFile.generic_string().c_str());
      ui->hyperlinkDisplay->setToolTip(hyperlinkFile.generic_string().c_str());

      drawing.setHyperlink(hyperlinkFile.generic_string());
    }
  });
  connect(ui->openDrawingPDFButton, &QPushButton::pressed, [this]() {
    const std::filesystem::path hyperlinkFile =
        ui->hyperlinkDisplay->displayText().toStdString();

    if (hyperlinkFile.empty()) {
      QMessageBox::about(this, "Open PDF",
                         "Cannot open PDF. No PDF file path set.");
      return;
    }

    if (!QDesktopServices::openUrl(
            QUrl::fromLocalFile(hyperlinkFile.generic_string().c_str()))) {
      QMessageBox::about(this, "Open PDF",
                         ("The PDF could not be found. (" +
                          hyperlinkFile.generic_string() + ")")
                             .c_str());
    }
  });
  connect(ui->generatePDFButton, &QPushButton::pressed, [this]() {
    if (!checkDrawing(Drawing::INVALID_HYPERLINK)) {
      return;
    }

    const QString pdfDirectory = QFileDialog::getExistingDirectory(
        this, "Open Folder to save PDF", QString());
    if (!pdfDirectory.isEmpty()) {
      std::filesystem::path pdfFile = pdfDirectory.toStdString();
      pdfFile /= (drawing.drawingNumber() + ".pdf");

      if (std::filesystem::exists(pdfFile)) {
        if (QMessageBox::question(this, "Overwrite PDF",
                                  "A PDF for this drawing already exists. "
                                  "Would you like to overwrite it?") !=
            QMessageBox::Yes) {
          return;
        }
      }

      std::stringstream initials;

      std::string userEmailCopy = userEmail;
      size_t nextDot;
      while ((nextDot = userEmailCopy.find('.')) < userEmailCopy.find('@')) {
        initials << (char)std::toupper(userEmailCopy.front());
        userEmailCopy.erase(0, nextDot + 1);
      }
      if (!userEmailCopy.empty()) {
        initials << (char)std::toupper(userEmailCopy.front());
      }

      if (pdfWriter.createPDF(pdfFile.generic_string().c_str(), drawing,
                              initials.str())) {
        QMessageBox::about(this, "PDF Generator", "Automatic PDF generated.");
      } else {
        QMessageBox::about(
            this, "PDF Generator",
            "PDF Generation failed. Is the PDF file already open?");
        return;
      }

      ui->hyperlinkDisplay->setText(pdfFile.generic_string().c_str());
      ui->hyperlinkDisplay->setToolTip(pdfFile.generic_string().c_str());

      drawing.setHyperlink(pdfFile.generic_string());
    }
  });
  connect(ui->pressDrawingHyperlinksAddButton, &QPushButton::pressed, [this]() {
    const std::filesystem::path hyperlinkFile =
        QFileDialog::getOpenFileName(this, "Select a Press Program PDF File",
                                     QString(), "PDF (*.pdf)")
            .toStdString();
    if (!hyperlinkFile.empty()) {
      ui->pressDrawingHyperlinkList->addItem(
          hyperlinkFile.generic_string().c_str());

      std::vector<std::filesystem::path> links;
      links.reserve(ui->pressDrawingHyperlinkList->count());

      for (int i = 0; i < ui->pressDrawingHyperlinkList->count(); i++) {
        std::filesystem::path p =
            ui->pressDrawingHyperlinkList->item(i)->text().toStdString();
        links.push_back(p);
      }

      drawing.setPressDrawingHyperlinks(links);
    }
  });
  connect(ui->pressDrawingHyperlinksRemoveButton, &QPushButton::pressed,
          [this]() {
            qDeleteAll(ui->pressDrawingHyperlinkList->selectedItems());

            std::vector<std::filesystem::path> links;
            links.reserve(ui->pressDrawingHyperlinkList->count());

            for (int i = 0; i < ui->pressDrawingHyperlinkList->count(); i++) {
              links.emplace_back(
                  ui->pressDrawingHyperlinkList->item(i)->text().toStdString());
            }

            drawing.setPressDrawingHyperlinks(links);
          });
  connect(ui->confirmDrawingButton, &QPushButton::pressed, this,
          &AddDrawingPageWidget::confirmDrawing);

  connect(ui->leftSideIronTypeInput,
          qOverload<int>(&QComboBox::currentIndexChanged), this,
          [=](int index) {
            switch ((SideIronType)(index + 1)) {
              case (SideIronType::B):
                ui->leftSideIronInvertedInput->setDisabled(true);
                ui->leftSideIronInvertedInput->setCheckState(
                    Qt::CheckState::Unchecked);
                break;
              default:
                ui->leftSideIronInvertedInput->setDisabled(false);
            }
          });

  connect(ui->rightSideIronTypeInput,
          qOverload<int>(&QComboBox::currentIndexChanged), this,
          [=](int index) {
            switch ((SideIronType)(index + 1)) {
              case (SideIronType::B):
                ui->rightSideIronInvertedInput->setDisabled(true);
                ui->rightSideIronInvertedInput->setCheckState(
                    Qt::CheckState::Unchecked);
                break;
              default:
                ui->rightSideIronInvertedInput->setDisabled(false);
            }
          });
}

void AddDrawingPageWidget::loadDrawing() {
  ui->drawingNumberInput->setText(drawing.drawingNumber().c_str());

  ui->dateInput->setDate(
      QDate(drawing.date().year, drawing.date().month, drawing.date().day));
  ui->widthInput->setValue(drawing.width());
  ui->lengthInput->setValue(drawing.length());
  if (!drawing.loadWarning(Drawing::MISSING_MATERIAL_DETECTED)) {
    ui->topMaterialInput->setCurrentIndex(ui->topMaterialInput->findData(
        drawing.material(Drawing::TOP)->handle()));
    if (drawing.material(Drawing::BOTTOM).has_value()) {
      ui->bottomMaterialInput->setCurrentIndex(
          ui->bottomMaterialInput->findData(
              drawing.material(Drawing::BOTTOM)->handle()));
      ui->bottomMaterialLabel->setActive();
    }
  }
  ui->productInput->setCurrentIndex(
      ui->productInput->findData(drawing.product().handle()));
  if (!drawing.loadWarning(Drawing::INVALID_BACKING_STRIP_DETECTED)) {
    ui->backingStripsInput->setCurrentIndex(
        ui->backingStripsInput->findData(drawing.backingStrip()->handle()));
  }
  if (!drawing.loadWarning(Drawing::INVALID_APERTURE_DETECTED)) {
    ui->apertureInput->setCurrentIndex(
        ui->apertureInput->findData(drawing.aperture().handle()));
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

  if (drawing.rebated()) {
    addedNotes.insert(REBATED);
  }
  if (drawing.hasBackingStrips()) {
    addedNotes.insert(HAS_BACKING_STRIPS);
  }

  ui->notesInput->setPlainText(drawing.notes().c_str());

  if (!drawing.loadWarning(Drawing::MISSING_SIDE_IRONS_DETECTED)) {
    SideIron leftSideIron = drawing.sideIron(Drawing::LEFT),
             rightSideIron = drawing.sideIron(Drawing::RIGHT);
    ui->leftSideIronDrawingInput->setCurrentIndex(
        ui->leftSideIronDrawingInput->findData(leftSideIron.handle()));
    ui->leftSideIronTypeInput->setCurrentIndex((unsigned)(leftSideIron.type) -
                                               1);
    ui->leftSideIronInvertedInput->setChecked(
        drawing.sideIronInverted(Drawing::LEFT));
    ui->leftSideIronCutDownInput->setChecked(
        drawing.sideIronCutDown(Drawing::LEFT));
    ui->rightSideIronDrawingInput->setCurrentIndex(
        ui->rightSideIronDrawingInput->findData(rightSideIron.handle()));
    ui->rightSideIronTypeInput->setCurrentIndex((unsigned)(rightSideIron.type) -
                                                1);
    ui->rightSideIronInvertedInput->setChecked(
        drawing.sideIronInverted(Drawing::RIGHT));
    ui->rightSideIronCutDownInput->setChecked(
        drawing.sideIronCutDown(Drawing::RIGHT));
    if (drawing.sideIronFeedEnd().has_value()) {
      switch (drawing.sideIronFeedEnd().value()) {
        case Drawing::LEFT:
          ui->leftSideIronFeedEndInput->setChecked(true);
          break;
        case Drawing::RIGHT:
          ui->rightSideIronFeedEndInput->setChecked(true);
          break;
      }
    }
    if (drawing.sideIronFixedEnd(Drawing::LEFT).has_value()) {
      ui->leftSideIronFixedEndInput->setCurrentIndex(
          (int)(drawing.sideIronFixedEnd(Drawing::LEFT).value()));
    }
    if (drawing.sideIronFixedEnd(Drawing::RIGHT).has_value()) {
      ui->rightSideIronFixedEndInput->setCurrentIndex(
          (int)(drawing.sideIronFixedEnd(Drawing::RIGHT).value()));
    }
    if (drawing.sideIronHookOrientation(Drawing::LEFT).has_value()) {
      ui->leftSideIronHookOriInput->setCurrentIndex(
          (int)(drawing.sideIronHookOrientation(Drawing::LEFT).value()));
    }
    if (drawing.sideIronHookOrientation(Drawing::RIGHT).has_value()) {
      ui->rightSideIronHookOriInput->setCurrentIndex(
          (int)(drawing.sideIronHookOrientation(Drawing::RIGHT).value()));
    }
    if (drawing.hasStrap(Drawing::LEFT)) {
    ui->leftStrapInput->setCurrentIndex(ui->leftStrapInput->findData(
        drawing.sideIronStrap(Drawing::LEFT).handle()));
      ui->leftStrapLabel->setActive(true);
    }
    if (drawing.hasStrap(Drawing::RIGHT)) {
    ui->rightStrapInput->setCurrentIndex(ui->rightStrapInput->findData(
        drawing.sideIronStrap(Drawing::RIGHT).handle()));
      ui->rightStrapLabel->setActive(true);
    }
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
  ui->manufacturerInput->setCurrentText(
      machineTemplate.machine().manufacturer.c_str());
  // ui->modelInput->setCurrentText(machineTemplate.machine().model.c_str());
  ui->modelInput->setCurrentIndex(
      ui->modelInput->findData(machineTemplate.machine().handle()));
  ui->quantityOnDeckInput->setValue(machineTemplate.quantityOnDeck);
  ui->machinePositionInput->setText(
      QString(machineTemplate.position.c_str()).toUpper());
  ui->machineDeckInput->setCurrentIndex(
      ui->machineDeckInput->findData(machineTemplate.deck().handle()));

  ui->hyperlinkDisplay->setText(drawing.hyperlink().generic_string().c_str());
  for (const std::filesystem::path &hyperlink :
       drawing.pressDrawingHyperlinks()) {
    ui->pressDrawingHyperlinkList->addItem(hyperlink.generic_string().c_str());
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
      QMessageBox::about(this, "Drawing Error",
                         "Invalid width. Make sure the width is positive");
      return false;
    case Drawing::INVALID_LENGTH:
      QMessageBox::about(this, "Drawing Error",
                         "Invalid length. Make sure the length is positive");
      return false;
    case Drawing::INVALID_TOP_MATERIAL:
      QMessageBox::about(this, "Drawing Error", "Invalid Top Layer Material");
      return false;
    case Drawing::INVALID_BOTTOM_MATERIAL:
      QMessageBox::about(this, "Drawing Error",
                         "Invalid Bottom Layer Material");
      return false;
    case Drawing::INVALID_APERTURE:
      QMessageBox::about(this, "Drawing Error", "Invalid Aperture");
      return false;
    case Drawing::INVALID_BAR_SPACINGS:
      QMessageBox::about(
          this, "Drawing Error",
          "Invalid Bar Spacings. Make sure they add to the width");
      return false;
    case Drawing::INVALID_BAR_WIDTHS:
      QMessageBox::about(
          this, "Drawing Error",
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
      QMessageBox::about(this, "Drawing Error",
                         "Invalid Drawing PDF Hyperlink");
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
        switch (QMessageBox::question(
            this, "Confirm Update",
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
  emit drawingAddedToDb();
}

void AddDrawingPageWidget::setConfirmationCallback(
    const std::function<void(const Drawing &, bool)> &callback) {
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
                              "Are you sure you wish to close this page "
                              "without adding your drawing "
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