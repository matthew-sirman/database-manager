//
// Created by matthew on 14/07/2020.
//

#ifndef DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H
#define DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H

#include <QWidget>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QMessageBox>

#include <unordered_set>

#include "Inspector.h"
#include "../../include/database/Drawing.h"
#include "../../include/database/DrawingPDFWriter.h"
#include "../../include/database/componentFilters.h"

#define REBATED_NOTE "Rebate 50mm in down to 15mm"
#define BACKING_STRIPS_NOTE "Backing Strips 6mm Tackyback"

namespace Ui {
    class AddDrawingPageWidget;
}

class AddDrawingPageWidget : public QWidget {
    Q_OBJECT

public:
    enum AddDrawingMode {
        ADD_NEW_DRAWING,
        EDIT_DRAWING,
        CLONE_DRAWING
    };

    explicit AddDrawingPageWidget(const std::string &drawingNumber, QWidget *parent = nullptr);

    AddDrawingPageWidget(const Drawing &drawing, AddDrawingMode mode, QWidget *parent = nullptr);

    ~AddDrawingPageWidget() override;

    void confirmDrawing();

    void setConfirmationCallback(const std::function<void(const Drawing &, bool)> &callback);

    void setMode(AddDrawingMode mode);

private:
    Ui::AddDrawingPageWidget *ui = nullptr;

    Inspector *inspector = nullptr;

    ComboboxComponentDataSource<Product> productSource;
    ComboboxComponentDataSource<Aperture> apertureSource;
    ComboboxComponentDataSource<Material> topMaterialSource, bottomMaterialSource;
    ComboboxComponentDataSource<SideIron> leftSideIronSource, rightSideIronSource;
    ComboboxComponentDataSource<Machine> machineManufacturerSource, machineModelSource;
    ComboboxComponentDataSource<MachineDeck> machineDeckSource;

    SideIronFilter *leftSideIronFilter = nullptr, *rightSideIronFilter = nullptr;
    MachineModelFilter *machineModelFilter = nullptr;

    QGraphicsScene *visualsScene = nullptr;

    Drawing drawing;

    DrawingPDFWriter pdfWriter;

    unsigned leftSICache = 0, rightSICache = 0;
    bool leftSIInverted = false, rightSIInverted = false;

    const QRegExp drawingNumberRegex = QRegExp("^([a-zA-Z]{1,2}[0-9]{2}[a-zA-Z]?|M[0-9]{3}[a-zA-Z]?)$");

    std::function<void(const Drawing &, bool)> confirmationCallback;

    AddDrawingMode addMode = ADD_NEW_DRAWING;
    
    void setupComboboxSources();

    void setupActivators();

    void setupDrawingUpdateConnections();

    void loadDrawing();

    bool checkDrawing(unsigned exclusions = 0);

    enum DrawingNoteType {
        REBATED,
        HAS_BACKING_STRIPS
    };

    std::unordered_set<DrawingNoteType> addedNotes;

private slots:
    void capitaliseLineEdit(const QString &text);
};

#endif //DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H
