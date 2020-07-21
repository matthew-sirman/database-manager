#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 02/07/2020.
//

#include "MainMenu.h"
#include "../build/ui_MainMenu.h"

MainMenu::MainMenu(const std::filesystem::path &clientMetaFilePath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainMenu) {
    ui->setupUi(this);

    // TODO: Make this dynamic plus add keygen if needed
    // TODO: Add option to save login details (aka get repeat key which last x days/months)

    std::ifstream clientMetaFile;
    clientMetaFile.open(clientMetaFilePath);

    nlohmann::json clientMeta;

    try {
        clientMetaFile >> clientMeta;
    } catch (nlohmann::json::parse_error &) {
        QMessageBox::about(this, "Invalid Client Meta File", "Failed to load the client meta file. "
                                                             "Make sure the path is valid and that the file is correctly "
                                                             "formatted JSON.");

        clientMetaFile.close();
        exit(0);
    }

    if (clientMeta.find("keyPath") == clientMeta.end()) {
        QMessageBox::about(this, "Invalid Client Meta File", "Ensure that the client meta file contains "
                                                             "the field 'keyPath' with the location of the client keys.");
        clientMetaFile.close();
        exit(0);
    }
    if (clientMeta.find("serverSignaturePath") == clientMeta.end()) {
        QMessageBox::about(this, "Invalid Client Meta File", "Ensure that the client meta file contains "
                                                             "the field 'serverSignaturePath' with the location of the server's "
                                                             "digital signature.");
        clientMetaFile.close();
        exit(0);
    }
    if (clientMeta.find("serverAddress") == clientMeta.end()) {
        QMessageBox::about(this, "Invalid Client Meta File", "Ensure that the client meta file contains "
                                                             "the field 'serverAddress' with the server's IP address.");
        clientMetaFile.close();
        exit(0);
    }
    if (clientMeta.find("serverPort") == clientMeta.end()) {
        QMessageBox::about(this, "Invalid Client Meta File", "Ensure that the client meta file contains "
                                                             "the field 'serverPort' with the server's port for the Database "
                                                             "Manager application.");
        clientMetaFile.close();
        exit(0);
    }

    std::filesystem::path keyPath = clientMeta["keyPath"];
    std::filesystem::path serverSignaturePath = clientMeta["serverSignaturePath"];
    std::string serverIP = clientMeta["serverAddress"];
    unsigned serverPort = clientMeta["serverPort"];

    RSAKeyPair clientKey;

    if (!std::filesystem::exists(keyPath / "client_key.pri") || !std::filesystem::exists(keyPath / "client_key.pub")) {
        QMessageBox::about(this, "Generating Keys", "Your client will shortly begin generating keys "
                                                    "for connection to the server. "
                                                    "This may take up to a minute.");

        clientKey = generateRSAKeyPair();
        writePlaintextPrivateKey(clientKey.privateKey, keyPath / "client_key.pri");
        writePublicKey(clientKey.publicKey, keyPath / "client_key.pub");

        QMessageBox::about(this, "Key Generation Complete", "Your client will now open a web browser with "
                                                            "an Outlook login. Please login to your Outlook account to use the "
                                                            "database.");
    } else {
        clientKey.privateKey = readPlaintextPrivateKey(keyPath / "client_key.pri");
        clientKey.publicKey = readPublicKey(keyPath / "client_key.pub");
    }

    PublicKey serverSignature = readPublicKey(serverSignaturePath / "signature.pub");

    float refreshRate;

    nlohmann::json::iterator refreshRateIter = clientMeta.find("refreshRate");

    if (refreshRateIter != clientMeta.end()) {
        refreshRate = *refreshRateIter;
    } else {
        refreshRate = DEFAULT_REFRESH_RATE;
    }

    client = new Client(refreshRate, clientKey, serverSignature);
    handler = new DatabaseResponseHandler();

    client->initialiseClient();
    client->setResponseHandler(*handler);

    nlohmann::json::iterator repeatTokenPathIter = clientMeta.find("repeatTokenPath");

    if (repeatTokenPathIter != clientMeta.end()) {
        std::filesystem::path repeatTokenPath = *repeatTokenPathIter;

        uint256 repeatToken;

        std::ifstream repeatTokenFile;
        repeatTokenFile.read((char *) &repeatToken, sizeof(uint256));

        if (!client->connectWithToken(serverIP, serverPort, repeatToken)) {
            QMessageBox::about(this, "Connection to Server Failed", "Failed to connect to the server (with repeat token). "
                                                                    "The application cannot be used without a connection to the server. "
                                                                    "Is the server running?");
            clientMetaFile.close();
            exit(0);
        }
    } else {
        // TODO: Make connection interface and make non blocking (?)
        if (!client->connectToServer(serverIP, serverPort,
                                     [](const std::string &url) { QDesktopServices::openUrl(QUrl(url.c_str())); })) {
            QMessageBox::about(this, "Connection to Server Failed", "Failed to connect to the server. "
                                                                    "The application cannot be used without a connection to the server. "
                                                                    "Is the server running?");
            clientMetaFile.close();
            exit(0);
        }
    }

    clientMetaFile.close();

    client->startClientLoop();

    sendSourceTableRequests();

    connect(ui->searchButton, SIGNAL(clicked()), this, SLOT(searchButtonPressed()));
    connect(ui->searchResultsTable, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(handleSearchElementContextMenu(const QPoint &)));
    ui->searchResultsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->mainTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(ui->drawingMenu_addDrawingAction, SIGNAL(triggered()), this, SLOT(openAddDrawingTab()));
    connect(this, SIGNAL(itemAddedToDrawingQueue()), this, SLOT(processDrawings()));

    ui->mainTabs->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    ui->mainTabs->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);

    handler->setDrawingReceivedHandler([this](DrawingRequest &drawingRequest) { onReceiveDrawing(drawingRequest); });

    handler->setDrawingInsertResponseHandler([this](DrawingInsert::InsertResponseType responseType, unsigned responseCode) {
        emit insertDrawingResponseReceived(responseType, responseCode);
    });

    connect(this, SIGNAL(insertDrawingResponseReceived(unsigned, unsigned)),
            this, SLOT(insertDrawingResponse(unsigned, unsigned)));

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
    ui->productTypeSearchInput->addExtraSourceItem({ "Any" });
    ui->apertureSearchInput->setDataSource(apertureSource);
    ui->apertureSearchInput->addExtraSourceItem({ "Any" });
    ui->thickness1SearchInput->setDataSource(materialSource);
    ui->thickness1SearchInput->addExtraSourceItem({ "Any" });
    ui->thickness2SearchInput->setDataSource(materialSource);
    ui->thickness2SearchInput->addExtraSourceItem({ "Any" });
    ui->machineSearchInput->setDataSource(machineSource);
    ui->machineSearchInput->addExtraSourceItem({ "Any" });
    ui->deckSearchInput->setDataSource(machineDeckSource);
    ui->deckSearchInput->addExtraSourceItem({ "Any" });
}

void MainMenu::setupValidators() {
    QRegExpValidator *drawingNumberValidator = new QRegExpValidator(QRegExp("^([a-zA-Z]{1,2}[0-9]{2}[a-zA-Z]?|M[0-9]{3}[a-zA-Z]?)$"));

    ui->drawingNumberSearchInput->setValidator(drawingNumberValidator);
    connect(ui->drawingNumberSearchInput, SIGNAL(textEdited(const QString &)), this, SLOT(capitaliseLineEdit(const QString &)));

    QRegExpValidator *positionSearchValidator = new QRegExpValidator(QRegExp("(^$)|(^[0-9]+([-][0-9]+)?$)|(^[Aa][Ll]{2}$)"));

    ui->positionSearchInput->setValidator(positionSearchValidator);
    connect(ui->positionSearchInput, SIGNAL(textEdited(const QString &)), this, SLOT(capitaliseLineEdit(const QString &)));
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

void MainMenu::onReceiveDrawing(DrawingRequest &drawingRequest) {
    drawingReceivedQueueMutex.lock();
    drawingReceivedQueue.push(&drawingRequest);
    drawingReceivedQueueMutex.unlock();

    emit itemAddedToDrawingQueue();
}

unsigned MainMenu::getValidRequestCode() const {
    std::vector<unsigned> codes;

    for (std::pair<unsigned, DrawingResponseMode> code : drawingResponseActions) {
        codes.push_back(code.first);
    }
    for (unsigned i = 0; i < codes.size(); i++) {
        unsigned target = codes[i];
        while (target < codes.size() && target != codes[target]) {
            unsigned temp = codes[target];
            codes[target] = target;
            target = temp;
        }
    }
    for (unsigned i = 0; i < codes.size(); i++) {
        if (codes[i] != i) {
            return i;
        }
    }
    return codes.size();
}

unsigned MainMenu::getValidInsertCode() const {
    std::vector<unsigned> codes;

    for (std::pair<unsigned, const Drawing *> code : drawingInserts) {
        codes.push_back(code.first);
    }
    for (unsigned i = 0; i < codes.size(); i++) {
        unsigned target = codes[i];
        while (target < codes.size() && target != codes[target]) {
            unsigned temp = codes[target];
            codes[target] = target;
            target = temp;
        }
    }
    for (unsigned i = 0; i < codes.size(); i++) {
        if (codes[i] != i) {
            return i;
        }
    }
    return codes.size();
}

void MainMenu::searchButtonPressed() {
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
        query.productType = DrawingComponentManager<Product>::getComponentByID(ui->productTypeSearchInput->currentData().value<ElementIndex>());
    }
    if (ui->numberOfBarsLabel->active()) {
        query.numberOfBars = ui->numberOfBarsSearchInput->value();
    }
    if (ui->apertureSearchInput->currentText() != "Any") {
        query.aperture = DrawingComponentManager<Aperture>::getComponentByID(ui->apertureSearchInput->currentData().value<ElementIndex>());
    }
    if (ui->thickness1SearchInput->currentText() != "Any") {
        query.topThickness = DrawingComponentManager<Material>::getComponentByID(ui->thickness1SearchInput->currentData().value<ElementIndex>());
    }
    if (ui->thickness2SearchInput->currentText() != "Any") {
        query.bottomThickness = DrawingComponentManager<Material>::getComponentByID(ui->thickness2SearchInput->currentData().value<ElementIndex>());
    }
    if (ui->startDateLabel->active()) {
        QDate date = ui->startDateSearchInput->date();
        query.dateRange = { { (unsigned short) date.year(), (unsigned char) date.month(), (unsigned char) date.day() },
                            { 9999, 12, 31 } };
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
        query.machine = DrawingComponentManager<Machine>::getComponentByID(ui->machineSearchInput->currentData().value<ElementIndex>());
    }
    if (ui->quantityOnDeckLabel->active()) {
        query.quantityOnDeck = ui->quantityOnDeckSearchInput->value();
    }
    if (ui->positionLabel->active()) {
        query.position = ui->positionSearchInput->text().toStdString();
    }
    if (ui->deckSearchInput->currentText() != "Any") {
        query.machineDeck = DrawingComponentManager<MachineDeck>::getComponentByID(ui->deckSearchInput->currentData().value<ElementIndex>());
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
            DrawingSummary summary = searchResultsModel->summaryAtRow(item.row());
            menu->addAction(("Open drawing " + summary.drawingNumber + " in new tab").c_str(),
                    [this, summary]() { openDrawingView(summary.matID); }, Qt::Key_Enter);
            menu->popup(ui->searchResultsTable->viewport()->mapToGlobal(pos));
        }
    }
}

void MainMenu::openDrawingView(unsigned matID) {
    unsigned responseEchoCode = getValidRequestCode();
    drawingResponseActions[responseEchoCode] = OPEN_DRAWING_VIEW_TAB;

    DrawingRequest request = DrawingRequest::makeRequest(matID, responseEchoCode);

    unsigned bufferSize;
    void *requestBuffer = request.createBuffer(bufferSize);
    client->addMessageToSendQueue(requestBuffer, bufferSize);
    free(requestBuffer);
}

void MainMenu::closeTab(int index) {
    ui->mainTabs->removeTab(index);
}

void MainMenu::openAddDrawingTab() {
    AddDrawingPageWidget *addDrawingPage = new AddDrawingPageWidget(ui->mainTabs);
    addDrawingPage->setConfirmationCallback([this](const Drawing &drawing, bool force) {
        DrawingInsert insert;
        insert.drawingData = drawing;

        insert.setForce(force);

        insert.responseEchoCode = getValidInsertCode();
        drawingInserts[insert.responseEchoCode] = &drawing;

        unsigned bufferSize = insert.serialisedSize();
        void *buffer = alloca(bufferSize);
        insert.serialise(buffer);

        client->addMessageToSendQueue(buffer, bufferSize);
    });
    ui->mainTabs->addTab(addDrawingPage, tr("Add Drawing"));
    ui->mainTabs->setCurrentWidget(addDrawingPage);
}

void MainMenu::processDrawings() {
    drawingReceivedQueueMutex.lock();
    while (!drawingReceivedQueue.empty()) {
        DrawingRequest *request = drawingReceivedQueue.front();
        drawingReceivedQueue.pop();

        switch (drawingResponseActions[request->responseEchoCode]) {
            case OPEN_DRAWING_VIEW_TAB:
                if (request->drawingData->loadWarning(Drawing::LoadWarning::LOAD_FAILED)) {
                    QMessageBox::about(this, "Drawing Load Failed", "Attempt to load this drawing from "
                                                                    "the database failed.");
                } else {
                    DrawingViewWidget *drawingView = new DrawingViewWidget(request->drawingData.value(), ui->mainTabs);

                    ui->mainTabs->addTab(drawingView, tr((request->drawingData->drawingNumber() + " Details").c_str()));
                    ui->mainTabs->setCurrentWidget(drawingView);
                }
                break;
        }

        drawingResponseActions.erase(request->responseEchoCode);
    }
    drawingReceivedQueueMutex.unlock();
}

void MainMenu::insertDrawingResponse(unsigned responseType, unsigned responseCode) {
    switch ((DrawingInsert::InsertResponseType) responseType) {
        case DrawingInsert::NONE:
            break;
        case DrawingInsert::SUCCESS:
            QMessageBox::about(this, "Insert Drawing", "Drawing successfully added to database.");
            drawingInserts.erase(responseCode);
            break;
        case DrawingInsert::FAILED:
            QMessageBox::about(this, "Insert Drawing", "The attempt to add the drawing to the database"
                                                       "was unsuccessful.");
            drawingInserts.erase(responseCode);
            break;
        case DrawingInsert::DRAWING_EXISTS:
            QMessageBox::StandardButton questionResponse = QMessageBox::question(this, "Insert Drawing", "This drawing already exists in the database. Would you like "
                                                                                                         "to update it?");
            if (questionResponse == QMessageBox::Yes) {
                DrawingInsert insert;
                insert.responseEchoCode = responseCode;
                insert.insertResponseType = DrawingInsert::NONE;
                insert.drawingData = *drawingInserts[responseCode];
                insert.setForce(true);

                unsigned bufferSize = insert.serialisedSize();
                void *buffer = alloca(bufferSize);
                insert.serialise(buffer);

                client->addMessageToSendQueue(buffer, bufferSize);

            }
            break;
    }
}

#pragma clang diagnostic pop