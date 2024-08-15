//
// Created by matthew on 02/07/2020.
//

#ifndef DATABASE_MANAGER_MAINMENU_H
#define DATABASE_MANAGER_MAINMENU_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QTableWidgetItem>
#include <QInputDialog>
#include <QKeyEvent>
#include <regex>
#include <QShortcut>
#include <memory>

#include "../include/networking/Client.h"
#include "../include/database/DatabaseQuery.h"
#include "../include/database/DatabaseResponseHandler.h"
#include "widgets/AddDrawingPageWidget.h"
#include "widgets/DrawingViewWidget.h"
#include "AddApertureWindow.h"
#include "AddMachineWindow.h"
#include "AddSideIronWindow.h"
#include "AddMaterialWindow.h"
#include "MaterialPricingWindow.h"
#include "SideIronPricingWindow.h"
#include "ExtraPricingWindow.h"
#include "LabourTimesWindow.h"
#include "SpecificSideIronPricingWindow.h"
#include "PowderCoatingPricingWindow.h"


#define DEFAULT_REFRESH_RATE 16

namespace Ui {
    class MainMenu;
}

/// <summary>
/// MainMenu Inherits QMainWindow
/// The main menu of the manager, including search functionality, and the basic window design. It also controls the tabs.
/// </summary>
class MainMenu : public QMainWindow {
    Q_OBJECT
public:
    /// <summary>
    /// Constructs a new main window, initialising a connection to the server.
    /// </summary>
    /// <param name="clientMetaFilePath">The client meta file to read information about how to connect
    /// to the server from.</param>
    /// <param name="parent">The parent of this widget.</param>
    explicit MainMenu(const std::filesystem::path &clientMetaFilePath, QWidget *parent = nullptr);

    /// <summary>
    /// Default Destructor.
    /// </summary>
    ~MainMenu() override;

private:
    enum DrawingResponseMode {
        OPEN_DRAWING_VIEW_TAB
    };

    Ui::MainMenu *ui = nullptr;

    Client *client = nullptr;
    DatabaseResponseHandler *handler = nullptr;

    std::string clientEmailAddress;

    void closeEvent(QCloseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void sendSourceTableRequests() const;

    void sourceTable(RequestType requestType) const;

    void setupComboboxSources();

    void setupValidators();

    void setupActivators();

    void setupSearchResultsTable();

    void requestNextDrawingNumbers() const;

    void onReceiveDrawing(DrawingRequest &drawingRequest);

    unsigned getValidRequestCode() const;

    unsigned getValidInsertCode() const;

    void openAddDrawingTab(NextDrawing::DrawingType type);

    void connectToServerWithJWT(const std::string &serverIP, unsigned serverPort);

    ComboboxComponentDataSource<Product> productSource;
    ComboboxComponentDataSource<Aperture> apertureSource;
    ComboboxComponentDataSource<ApertureShape> apertureShapeSource;
    ComboboxComponentDataSource<Material> topMaterialSource, bottomMaterialSource;
    ComboboxComponentDataSource<SideIron> sideIronSource;
    ComboboxComponentDataSource<Machine> machineManufacturerSource, machineModelSource;
    ComboboxComponentDataSource<MachineDeck> machineDeckSource;

    MachineModelFilter *machineModelFilter = nullptr;

    DrawingSearchResultsModel *searchResultsModel = nullptr;

    std::unordered_map<unsigned, DrawingResponseMode> drawingResponseActions;
    std::queue<DrawingRequest *> drawingReceivedQueue;
    std::mutex drawingReceivedQueueMutex;

    std::unordered_map<unsigned, const Drawing *> drawingInserts;

    std::string nextAutomaticDrawingNumber, nextManualDrawingNumber;

private slots:
    void searchButtonPressed();

    void capitaliseLineEdit(const QString &text);

    void handleSearchElementContextMenu(const QPoint &pos);

    void openDrawingView(unsigned matID);

    void closeTab(int index);

    void processDrawings();

    void insertDrawingResponse(DrawingInsert::InsertResponseCode responseType, unsigned responseCode);

    void insertComponentResponse(ComponentInsert::ComponentInsertResponse responseCode);

    void backupResponse(DatabaseBackup::BackupResponse responseCode);

signals:
    /// <summary>
    /// This signal is emitted when a \ref DrawingRequest has been recieved.
    /// </summary>
    void itemAddedToDrawingQueue();

    /// <summary>
    /// This singal is emitted when the \ref DatabaseResponseHandler recieves a \ref DrawingInsert.
    /// </summary>
    /// <param name="insertResponseType">The response code of the insert.</param>
    /// <param name="echoResponseCode">Currently unsed, however this code links to DrawingResponseCode,
    /// and is kept the same across the lifespan of a drawing insert, so when it is returned, the
    /// relevant DrawingResponseCode can be selected.</param>
    void insertDrawingResponseReceived(DrawingInsert::InsertResponseCode insertResponseType, unsigned echoResponseCode);

    /// <summary>
    /// This signal is emitted when the \ref DatabaseResponseHandler recieves a \ref ComponentInsert.
    /// </summary>
    /// <param name="responseCode">The response code of the insert.</param>
    void addComponentResponseReceived(ComponentInsert::ComponentInsertResponse responseCode);

    /// <summary>
    /// This signal is emitted when the \ref DatabaseResponseHandler recieves a \ref DatabaseBackup.
    /// </summary>
    /// <param name="resposneCode">The response code of the backup.</param>
    void backupResponseReceived(DatabaseBackup::BackupResponse resposneCode);

};


#endif //DATABASE_MANAGER_MAINMENU_H
