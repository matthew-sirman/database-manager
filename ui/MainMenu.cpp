#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 02/07/2020.
//

#include "MainMenu.h"
#include "../build/ui_MainMenu.h"

MainMenu::MainMenu(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainMenu) {
    ui->setupUi(this);

    // TODO: Make this dynamic plus add keygen if needed
    // TODO: Add option to save login details (aka get repeat key which last x days/months)

    RSAKeyPair clientKey;
    clientKey.privateKey = unlockPrivateKey("/home/matthew/database-manager/test_client_keys/client_key.pri", sha256(TEMP_pw.c_str(), TEMP_pw.size()));
    clientKey.publicKey = readPublicKey("/home/matthew/database-manager/test_client_keys/client_key.pub");

    PublicKey serverSignature = readPublicKey("/home/matthew/database-manager/keys/signature/signature.pub");

    client = new Client(16, clientKey, serverSignature);
    handler = new DatabaseResponseHandler();

    client->initialiseClient();
    client->setResponseHandler(*handler);

    // TODO: Make connection interface and make non blocking (?)
    if (!client->connectToServer("192.168.68.120", 10000, [](const std::string &url) { QDesktopServices::openUrl(QUrl(url.c_str())); })) {
        QMessageBox::about(this, "Connection to Server Failed", "Failed to connect to the server. "
                                                                "The application cannot be used without a connection to the server. "
                                                                "Is the server running?");
        exit(0);
    }

    client->startClientLoop();

    sendSourceTableRequests();

    connect(ui->searchButton, SIGNAL(clicked()), this, SLOT(searchButtonPressed()));
    connect(ui->searchResultsTable, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(handleSearchElementContextMenu(const QPoint &)));
    ui->searchResultsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->mainTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(ui->drawingMenu_addDrawingAction, SIGNAL(triggered()), this, SLOT(openAddDrawingTab()));
//    connect(ui->fileMenu_exitAction, SIGNAL(triggered()), this, SLOT(close()));

    ui->mainTabs->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    ui->mainTabs->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);

    setupComboboxSources();
    setupValidators();
    setupActivators();
    setupSearchResultsTable();
}

MainMenu::~MainMenu() = default;

void MainMenu::closeEvent(QCloseEvent *event) {
    client->stopClientLoop();
}

void MainMenu::sendSourceTableRequests() const {
    // Source each type of drawing component
    sourceTable(RequestType::SOURCE_PRODUCT_TABLE);
    sourceTable(RequestType::SOURCE_APERTURE_TABLE);
    sourceTable(RequestType::SOURCE_APERTURE_SHAPE_TABLE);
    sourceTable(RequestType::SOURCE_MATERIAL_TABLE);
    sourceTable(RequestType::SOURCE_SIDE_IRON_TABLE);
    sourceTable(RequestType::SOURCE_MACHINE_TABLE);
    sourceTable(RequestType::SOURCE_MACHINE_DECK_TABLE);
}

void MainMenu::sourceTable(RequestType requestType) const {
    client->addMessageToSendQueue(&requestType, sizeof(RequestType));
}

void MainMenu::setupComboboxSources() {
    DrawingComponentManager<Product>::addCallback([this]() { productSource.updateSource(); });
    DrawingComponentManager<Aperture>::addCallback([this]() { apertureSource.updateSource(); });
    DrawingComponentManager<ApertureShape>::addCallback([this]() { apertureShapeSource.updateSource(); });
    DrawingComponentManager<Material>::addCallback([this]() { materialSource.updateSource(); });
    DrawingComponentManager<SideIron>::addCallback([this]() { sideIronSource.updateSource(); });
    DrawingComponentManager<Machine>::addCallback([this]() { machineSource.updateSource(); });
    DrawingComponentManager<MachineDeck>::addCallback([this]() { machineDeckSource.updateSource(); });

    ui->productTypeSearchInput->setDataSource(productSource);
    ui->productTypeSearchInput->addExtraSourceItem({ "Any", std::nullopt });
    ui->apertureSearchInput->setDataSource(apertureSource);
    ui->apertureSearchInput->addExtraSourceItem({ "Any", std::nullopt });
    ui->thickness1SearchInput->setDataSource(materialSource);
    ui->thickness1SearchInput->addExtraSourceItem({ "Any", std::nullopt });
    ui->thickness2SearchInput->setDataSource(materialSource);
    ui->thickness2SearchInput->addExtraSourceItem({ "Any", std::nullopt });
    ui->machineSearchInput->setDataSource(machineSource);
    ui->machineSearchInput->addExtraSourceItem({ "Any", std::nullopt });
    ui->deckSearchInput->setDataSource(machineDeckSource);
    ui->deckSearchInput->addExtraSourceItem({ "Any", std::nullopt });
}

void MainMenu::setupValidators() {
    QRegExpValidator *drawingNumberValidator = new QRegExpValidator(QRegExp("^([a-zA-Z]{1,2}[0-9]{2}[a-zA-Z]?|M[0-9]{3}[a-zA-Z]?)$"));

    ui->drawingNumberSearchInput->setValidator(drawingNumberValidator);
    connect(ui->drawingNumberSearchInput, SIGNAL(textEdited(const QString &)), SLOT(capitaliseLineEdit(const QString &)));
}

void MainMenu::setupActivators() {
    ui->drawingNumberLabel->addTarget(ui->drawingNumberSearchInput);
    ui->drawingNumberLabel->setActive();

    ui->widthLabel->addTarget(ui->widthSearchInput);
    ui->widthLabel->addTarget(ui->widthToleranceLabel);
    ui->widthLabel->addTarget(ui->widthToleranceSearchInput);

    ui->lengthLabel->addTarget(ui->lengthSearchInput);
    ui->lengthLabel->addTarget(ui->lengthToleranceLabel);
    ui->lengthLabel->addTarget(ui->lengthToleranceSearchInput);

    ui->numberOfBarsLabel->addTarget(ui->numberOfBarsSearchInput);

    ui->startDateLabel->addTarget(ui->startDateSearchInput);

    ui->endDateLabel->addTarget(ui->endDateSearchInput);

    ui->sideIronLengthLabel->addTarget(ui->sideIronLengthSearchInput);

    ui->sidelapWidthLabel->addTarget(ui->sidelapWidthSearchInput);
    ui->sidelapWidthLabel->addTarget(ui->sidelapWidthToleranceLabel);
    ui->sidelapWidthLabel->addTarget(ui->sidelapWidthToleranceSearchInput);

    ui->overlapWidthLabel->addTarget(ui->overlapWidthSearchInput);
    ui->overlapWidthLabel->addTarget(ui->overlapWidthToleranceLabel);
    ui->overlapWidthLabel->addTarget(ui->overlapWidthToleranceSearchInput);

    ui->quantityOnDeckLabel->addTarget(ui->quantityOnDeckSearchInput);

    ui->positionLabel->addTarget(ui->positionSearchInput);
}

void MainMenu::setupSearchResultsTable() {
    searchResultsModel = new DrawingSearchResultsModel();
    searchResultsModel->setHeaderData(0, Qt::Horizontal, "Drawing Number");
    searchResultsModel->setHeaderData(1, Qt::Horizontal, "Drawing Dimensions");

    handler->setSearchResultsModel(searchResultsModel);
    ui->searchResultsTable->setModel(searchResultsModel);

    ui->searchResultsTable->horizontalHeader()->setStretchLastSection(true);
    ui->searchResultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
}

void MainMenu::searchButtonPressed() {
    // TODO: Finish sourcing from UI
    DatabaseSearchQuery query;

    if (ui->drawingNumberLabel->active()) {
        query.drawingNumber = ui->drawingNumberSearchInput->text().toStdString();
    }
    if (ui->widthLabel->active()) {
        unsigned width = ui->widthSearchInput->value();
        unsigned tolerance = ui->widthToleranceSearchInput->value();
        query.width = { width - tolerance, width + tolerance };
    }
    if (ui->lengthLabel->active()) {
        unsigned length = ui->lengthSearchInput->value();
        unsigned tolerance = ui->lengthToleranceSearchInput->value();
        query.length = { length - tolerance, length + tolerance };
    }
    if (ui->productTypeSearchInput->currentText() != "Any") {
        query.productType = DrawingComponentManager<Product>::getComponentByID(ui->productTypeSearchInput->currentData().toInt());
    }
    if (ui->numberOfBarsLabel->active()) {
        query.numberOfBars = ui->numberOfBarsSearchInput->value();
    }
    if (ui->apertureSearchInput->currentText() != "Any") {
        query.aperture = DrawingComponentManager<Aperture>::getComponentByID(ui->apertureSearchInput->currentData().toInt());
    }
    if (ui->thickness1SearchInput->currentText() != "Any") {
        query.topThickness = DrawingComponentManager<Material>::getComponentByID(ui->thickness1SearchInput->currentData().toInt());
    }
    if (ui->thickness2SearchInput->currentText() != "Any") {
        query.bottomThickness = DrawingComponentManager<Material>::getComponentByID(ui->thickness2SearchInput->currentData().toInt());
    }
    if (ui->startDateLabel->active()) {
        QDate date = ui->startDateSearchInput->date();
        query.dateRange = { { (unsigned short) date.year(), (unsigned char) date.month(), (unsigned char) date.day() },
                            { 65535, 12, 31 } };
    }
    if (ui->endDateLabel->active()) {
        QDate date = ui->endDateSearchInput->date();
        if (query.dateRange == std::nullopt) {
            query.dateRange = { { 0, 1, 1 },
                                { (unsigned short) date.year(), (unsigned char) date.month(), (unsigned char) date.day() } };
        } else {
            query.dateRange->upperBound = { (unsigned short) date.year(), (unsigned char) date.month(), (unsigned char) date.day() };
        }
    }
    if (ui->sideIronTypeSearchInput->currentText() != "Any") {
        query.sideIronType = (SideIronType) (ui->sideIronTypeSearchInput->currentIndex() - 1);
    }
    if (ui->sideIronLengthLabel->active()) {
        query.sideIronLength = ui->sideIronLengthSearchInput->value();
    }
    if (ui->sidelapsSearchInput->currentText() != "Any") {
        query.sidelapMode = (LapSetting) (ui->sidelapsSearchInput->currentIndex() - 1);
    }
    if (ui->overlapsSearchInput->currentText() != "Any") {
        query.overlapMode = (LapSetting) (ui->overlapsSearchInput->currentIndex() - 1);
    }
    if (ui->sidelapWidthLabel->active()) {
        unsigned width = ui->sidelapWidthSearchInput->value();
        unsigned tolerance = ui->sidelapWidthToleranceSearchInput->value();
        query.sidelapWidth = { width - tolerance, width + tolerance };
    }
    if (ui->overlapWidthLabel->active()) {
        unsigned width = ui->overlapWidthSearchInput->value();
        unsigned tolerance = ui->overlapWidthToleranceSearchInput->value();
        query.overlapWidth = { width - tolerance, width + tolerance };
    }
    if (ui->sidelapAttachmentSearchInput->currentText() != "Any") {
        query.sidelapAttachment = (LapAttachment) (ui->sidelapAttachmentSearchInput->currentIndex() - 1);
    }
    if (ui->overlapAttachmentSearchInput->currentText() != "Any") {
        query.overlapAttachment = (LapAttachment) (ui->overlapAttachmentSearchInput->currentIndex() - 1);
    }
    if (ui->machineSearchInput->currentText() != "Any") {
        query.machine = DrawingComponentManager<Machine>::getComponentByID(ui->machineSearchInput->currentData().toInt());
    }
    if (ui->quantityOnDeckLabel->active()) {
        query.quantityOnDeck = ui->quantityOnDeckSearchInput->value();
    }
    if (ui->positionLabel->active()) {
        query.position = ui->positionSearchInput->text().toStdString();
    }
    if (ui->deckSearchInput->currentText() != "Any") {
        query.machineDeck = DrawingComponentManager<MachineDeck>::getComponentByID(ui->deckSearchInput->currentData().toInt());
    }

    unsigned bufferSize;
    void *queryBuffer = query.createBuffer(bufferSize);
    client->addMessageToSendQueue(queryBuffer, bufferSize);
    free(queryBuffer);
}

void MainMenu::capitaliseLineEdit(const QString &text) {
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (!lineEdit) {
        return;
    }
    lineEdit->setText(text.toUpper());
}

void MainMenu::handleSearchElementContextMenu(const QPoint &pos) {
    QModelIndex item = ui->searchResultsTable->indexAt(pos);
    if (item.isValid()) {
        if (item.column() == 0) {
            QMenu *menu = new QMenu(this);
            std::string drawingNumber = searchResultsModel->summaryAtRow(item.row()).drawingNumber;
            menu->addAction(("Open drawing " + drawingNumber + " in new tab").c_str(), this, SLOT(openDrawingViewTab()), Qt::Key_Enter);
            menu->popup(ui->searchResultsTable->viewport()->mapToGlobal(pos));
        }
    }
}

void MainMenu::openDrawingViewTab() {
    ui->mainTabs->addTab(new DrawingViewWidget(ui->mainTabs), tr("New Tab"));
}

void MainMenu::closeTab(int index) {
    ui->mainTabs->removeTab(index);
}

void MainMenu::openAddDrawingTab() {
    ui->mainTabs->addTab(new AddDrawingPageWidget(ui->mainTabs), tr("Add Drawing"));
}

#pragma clang diagnostic pop