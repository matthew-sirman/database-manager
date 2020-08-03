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

    std::ifstream clientMetaFile;
    clientMetaFile.open(clientMetaFilePath / "clientMeta.json");

    nlohmann::json clientMeta;

    try {
        clientMetaFile >> clientMeta;

        clientMetaFile.close();
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

    std::filesystem::path keyPath = clientMeta["keyPath"].get<std::string>();
    std::filesystem::path serverSignaturePath = clientMeta["serverSignaturePath"].get<std::string>();

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
        clientKey.privateKey = readPlaintextPrivateKey<32>(keyPath / "client_key.pri");
        clientKey.publicKey = readPublicKey<32>(keyPath / "client_key.pub");
    }

    DigitalSignatureKeyPair::Public serverSignature = readPublicKey<24>(serverSignaturePath / "signature.pub");

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
        std::filesystem::path repeatTokenPath = (*repeatTokenPathIter).get<std::string>();

        uint256 repeatToken;

        std::ifstream repeatTokenFile(repeatTokenPath);
        repeatTokenFile.read((char *) &repeatToken, sizeof(uint256));
        repeatTokenFile.close();

        switch (client->connectWithToken(serverIP, serverPort, repeatToken)) {
        case Client::ConnectionStatus::NO_CONNECTION:
            QMessageBox::about(this, "Connection to Server Failed", "Failed to connect to the server. "
                "The application cannot be used without a connection to the server. Is the server running?");

            exit(0);
        case Client::ConnectionStatus::CREDS_EXCHANGE_FAILED:
            QMessageBox::about(this, "Connection to Server Failed", "Credentials exchange failed. "
                "Failed to connect to the server due to bad credentials.");

            exit(0);
        case Client::ConnectionStatus::INVALID_REPEAT_TOKEN:
            QMessageBox::about(this, "Invalid Token", "Your repeat login token has expired. Please log back in "
                "through Outlook.");

            client->disconnect();
            client->initialiseClient();

            connectToServerWithJWT(serverIP, serverPort);

            if (QMessageBox::question(this, "Save Login Details", "Would you like to save login details for future use?") == QMessageBox::Yes) {
                handler->setRepeatTokenResponseCallback([this, repeatTokenPath](const uint256 &token) {
                    std::ofstream repeatTokenFile(repeatTokenPath);
                    repeatTokenFile.write((const char *)&token, sizeof(uint256));
                    repeatTokenFile.close();
                });

                client->requestRepeatToken((unsigned) RequestType::REPEAT_TOKEN_REQUEST);
            }
            break;
        case Client::ConnectionStatus::SUCCESS:
            break;
        }
    } else {
        connectToServerWithJWT(serverIP, serverPort);

        if (QMessageBox::question(this, "Save Login Details", "Would you like to save login details for future use?") == QMessageBox::Yes) {
            handler->setRepeatTokenResponseCallback([this, clientMetaFilePath, clientMeta](const uint256 &token) mutable {
                std::string repeatTokenFilePath = (clientMetaFilePath / "repeat.tok").string();

                std::ofstream repeatTokenFile(repeatTokenFilePath);
                repeatTokenFile.write((const char *)&token, sizeof(uint256));
                repeatTokenFile.close();

                clientMeta["repeatTokenPath"] = repeatTokenFilePath;

                std::ofstream clientMetaFile(clientMetaFilePath / "clientMeta.json");
                clientMetaFile << std::setw(4) << clientMeta << std::endl;
                clientMetaFile.close();
            });

            client->requestRepeatToken((unsigned)RequestType::REPEAT_TOKEN_REQUEST);
        }
    }

    client->startClientLoop();

    sendSourceTableRequests();

    connect(ui->searchButton, SIGNAL(clicked()), this, SLOT(searchButtonPressed()));
    connect(ui->searchResultsTable, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(handleSearchElementContextMenu(const QPoint &)));
    ui->searchResultsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->mainTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(ui->drawingMenu_addDrawingAction, SIGNAL(triggered()), this, SLOT(openAddDrawingTab()));
    connect(this, SIGNAL(itemAddedToDrawingQueue()), this, SLOT(processDrawings()));

    connect(ui->componentsMenu_addApertureAction, &QAction::triggered, [this]() {
        (new AddApertureWindow(client))->show();
    });
    connect(ui->componentsMenu_addMachineAction, &QAction::triggered, [this]() {
        (new AddMachineWindow(client))->show();
    });
    connect(ui->componentsMenu_addSideIronAction, &QAction::triggered, [this]() {
        (new AddSideIronWindow(client))->show();
    });
    connect(ui->componentsMenu_addMaterialAction, &QAction::triggered, [this]() {
        (new AddMaterialWindow(client))->show();
    });

    handler->setAddComponentResponseCallback([this](ComponentInsert::ComponentInsertResponse responseCode) {
        emit addComponentResponseReceived((unsigned)responseCode);
    });

    connect(this, SIGNAL(addComponentResponseReceived(unsigned)), this, SLOT(insertComponentResponse(unsigned)));

    connect(ui->fileMenu_createBackupAction, &QAction::triggered, [this]() {
        std::time_t currentTimePoint = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm *currentTime = std::localtime(&currentTimePoint);

        std::stringstream dateString;
        dateString << std::put_time(currentTime, "%Y-%m-%d_%H-%M-%S");

        QString backupName = QInputDialog::getText(this, "Database Backup", "Enter name for backup", 
            QLineEdit::Normal, dateString.str().c_str());

        if (backupName.isEmpty()) {
            QMessageBox::about(this, "Invalid Backup Name", "Backup name must not be empty.");
            return;
        }
        if (backupName.contains(QRegExp("\\|/"))) {
            QMessageBox::about(this, "Invalid Backup Name", "Backup name must not contain slashes.");
            return;
        }

        DatabaseBackup backup;
        backup.backupName = backupName.toStdString();
        backup.responseCode = DatabaseBackup::BackupResponse::NONE;

        unsigned bufferSize = backup.serialisedSize();
        void *requestBuffer = alloca(bufferSize);
        backup.serialise(requestBuffer);

        client->addMessageToSendQueue(requestBuffer, bufferSize);
    });

    handler->setBackupResponseCallback([this](DatabaseBackup::BackupResponse responseCode) {
        emit backupResponseReceived((unsigned)responseCode);
    });

    connect(this, SIGNAL(backupResponseReceived(unsigned)), this, SLOT(backupResponse(unsigned)));

    ui->mainTabs->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    ui->mainTabs->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);

    handler->setDrawingReceivedHandler([this](DrawingRequest &drawingRequest) { onReceiveDrawing(drawingRequest); });

    handler->setDrawingInsertResponseHandler([this](DrawingInsert::InsertResponseCode responseType, unsigned responseCode) {
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
    client->disconnect();
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
    ui->apertureSearchInput->setDataSource(apertureSource);
    ui->thickness1SearchInput->setDataSource(materialSource);
    ui->thickness2SearchInput->setDataSource(materialSource);
    ui->machineSearchInput->setDataSource(machineSource);
    ui->deckSearchInput->setDataSource(machineDeckSource);
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

    ui->productTypeLabel->addTarget(ui->productTypeSearchInput);

    ui->numberOfBarsLabel->addTarget(ui->numberOfBarsSearchInput);

    ui->apertureLabel->addTarget(ui->apertureSearchInput);

    ui->thickness1SearchLabel->addTarget(ui->thickness1SearchInput);
    ui->thickness2SearchLabel->addTarget(ui->thickness2SearchInput);

    ui->startDateLabel->addTarget(ui->startDateSearchInput);

    ui->endDateLabel->addTarget(ui->endDateSearchInput);

    ui->sideIronTypeLabel->addTarget(ui->sideIronTypeSearchInput);
    ui->sideIronLengthLabel->addTarget(ui->sideIronLengthSearchInput);

    ui->sidelapsLabel->addTarget(ui->sidelapsSearchInput);
    ui->sidelapWidthLabel->addTarget(ui->sidelapWidthSearchInput);
    ui->sidelapWidthLabel->addTarget(ui->sidelapWidthToleranceLabel);
    ui->sidelapWidthLabel->addTarget(ui->sidelapWidthToleranceSearchInput);
    ui->sidelapAttachmentLabel->addTarget(ui->sidelapAttachmentSearchInput);

    ui->overlapsLabel->addTarget(ui->overlapsSearchInput);
    ui->overlapWidthLabel->addTarget(ui->overlapWidthSearchInput);
    ui->overlapWidthLabel->addTarget(ui->overlapWidthToleranceLabel);
    ui->overlapWidthLabel->addTarget(ui->overlapWidthToleranceSearchInput);
    ui->overlapAttachmentLabel->addTarget(ui->overlapAttachmentSearchInput);

    ui->machineLabel->addTarget(ui->machineSearchInput);

    ui->quantityOnDeckLabel->addTarget(ui->quantityOnDeckSearchInput);

    ui->positionLabel->addTarget(ui->positionSearchInput);

    ui->deckLabel->addTarget(ui->deckSearchInput);
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

void MainMenu::connectToServerWithJWT(const std::string &serverIP, unsigned serverPort) {
    switch (client->connectToServer(serverIP, serverPort,
        [](const std::string &url) { QDesktopServices::openUrl(QUrl(url.c_str())); })) {
    case Client::ConnectionStatus::NO_CONNECTION:
        QMessageBox::about(this, "Connection to Server Failed", "Failed to connect to the server. "
            "The application cannot be used without a connection to the server. Is the server running?");

        exit(0);
    case Client::ConnectionStatus::CREDS_EXCHANGE_FAILED:
        QMessageBox::about(this, "Connection to Server Failed", "Credentials exchange failed. "
            "Failed to connect to the server due to bad credentials.");

        exit(0);
    case Client::ConnectionStatus::INVALID_JWT:
        QMessageBox::about(this, "Invalid Login", "Your login was denied by the server.");

        exit(0);
    case Client::ConnectionStatus::SUCCESS:
        break;
    }
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
    if (ui->productTypeLabel->active()) {
        query.productType = DrawingComponentManager<Product>::getComponentByHandle(ui->productTypeSearchInput->currentData().toInt());
    }
    if (ui->numberOfBarsLabel->active()) {
        query.numberOfBars = ui->numberOfBarsSearchInput->value();
    }
    if (ui->apertureLabel->active()) {
        query.aperture = DrawingComponentManager<Aperture>::getComponentByHandle(ui->apertureSearchInput->currentData().toInt());
    }
    if (ui->thickness1SearchLabel->active()) {
        query.topThickness = DrawingComponentManager<Material>::getComponentByHandle(ui->thickness1SearchInput->currentData().toInt());
    }
    if (ui->thickness2SearchLabel->active()) {
        query.bottomThickness = DrawingComponentManager<Material>::getComponentByHandle(ui->thickness2SearchInput->currentData().toInt());
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
    if (ui->sideIronTypeLabel->active()) {
        query.sideIronType = (SideIronType) (ui->sideIronTypeSearchInput->currentIndex());
    }
    if (ui->sideIronLengthLabel->active()) {
        query.sideIronLength = ui->sideIronLengthSearchInput->value();
    }
    if (ui->sidelapsLabel->active()) {
        query.sidelapMode = (LapSetting) (ui->sidelapsSearchInput->currentIndex() - 1);
    }
    if (ui->overlapsLabel->active()) {
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
    if (ui->sidelapAttachmentLabel->active()) {
        query.sidelapAttachment = (LapAttachment) (ui->sidelapAttachmentSearchInput->currentIndex() - 1);
    }
    if (ui->overlapAttachmentLabel->active()) {
        query.overlapAttachment = (LapAttachment) (ui->overlapAttachmentSearchInput->currentIndex() - 1);
    }
    if (ui->machineLabel->active()) {
        query.machine = DrawingComponentManager<Machine>::getComponentByHandle(ui->machineSearchInput->currentData().toInt());
    }
    if (ui->quantityOnDeckLabel->active()) {
        query.quantityOnDeck = ui->quantityOnDeckSearchInput->value();
    }
    if (ui->positionLabel->active()) {
        query.position = ui->positionSearchInput->text().toStdString();
    }
    if (ui->deckLabel->active()) {
        query.machineDeck = DrawingComponentManager<MachineDeck>::getComponentByHandle(ui->deckSearchInput->currentData().toInt());
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
                    Drawing &drawing = request->drawingData.value();

                    drawingView->setUpdateDrawingCallback([this, drawing]() {
                        AddDrawingPageWidget *addDrawingPage = new AddDrawingPageWidget(drawing, ui->mainTabs);
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
                    });

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
    switch ((DrawingInsert::InsertResponseCode) responseType) {
        case DrawingInsert::NONE:
            break;
        case DrawingInsert::SUCCESS:
            QMessageBox::about(this, "Insert Drawing", "Drawing successfully added to database.");
            drawingInserts.erase(responseCode);
            break;
        case DrawingInsert::FAILED:
            QMessageBox::about(this, "Insert Drawing", "The attempt to add the drawing to the database "
                                                       "was unsuccessful.");
            drawingInserts.erase(responseCode);
            break;
        case DrawingInsert::DRAWING_EXISTS:
            QMessageBox::StandardButton questionResponse = QMessageBox::question(this, "Insert Drawing", "This drawing already exists in the database. Would you like "
                                                                                                         "to update it?");
            if (questionResponse == QMessageBox::Yes) {
                DrawingInsert insert;
                insert.responseEchoCode = responseCode;
                insert.insertResponseCode = DrawingInsert::NONE;
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

void MainMenu::insertComponentResponse(unsigned responseCode) {
    switch ((ComponentInsert::ComponentInsertResponse)responseCode) {
        case ComponentInsert::ComponentInsertResponse::SUCCESS:
            QMessageBox::about(this, "Add Component", "Component successfully added to database.");
            break;
        case ComponentInsert::ComponentInsertResponse::FAILED:
            QMessageBox::about(this, "Add Component", "There was an error while trying to add component to the database.");
            break;
        default:
            break;
    }
}

void MainMenu::backupResponse(unsigned responseCode) {
    switch ((DatabaseBackup::BackupResponse)responseCode) {
        case DatabaseBackup::BackupResponse::SUCCESS:
            QMessageBox::about(this, "Database Backup", "Backup created successfully.");
            break;
        case DatabaseBackup::BackupResponse::FAILED:
            QMessageBox::about(this, "Database Backup", "There was an error while trying to create the backup.");
            break;
    }
}

#pragma clang diagnostic pop