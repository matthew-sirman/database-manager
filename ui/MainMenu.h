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

#include "../include/networking/Client.h"
#include "../include/database/DatabaseQuery.h"
#include "../include/database/DatabaseResponseHandler.h"

#include "widgets/AddDrawingPageWidget.h"
#include "widgets/DrawingViewWidget.h"
#include "AddApertureWindow.h"
#include "AddMachineWindow.h"
#include "AddSideIronWindow.h"
#include "AddMaterialWindow.h"

#define DEFAULT_REFRESH_RATE 16

namespace Ui {
    class MainMenu;
}

class MainMenu : public QMainWindow {
    Q_OBJECT
public:
    explicit MainMenu(const std::filesystem::path &clientMetaFilePath, QWidget *parent = nullptr);

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

    void insertDrawingResponse(unsigned responseType, unsigned responseCode);

    void insertComponentResponse(unsigned responseCode);

    void backupResponse(unsigned responseCode);

signals:
    void itemAddedToDrawingQueue();

    void insertDrawingResponseReceived(unsigned responseType, unsigned responseCode);

    void addComponentResponseReceived(unsigned responseCode);

    void backupResponseReceived(unsigned resposneCode);

};


#endif //DATABASE_MANAGER_MAINMENU_H
