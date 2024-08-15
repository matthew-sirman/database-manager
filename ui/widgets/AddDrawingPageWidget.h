//
// Created by matthew on 14/07/2020.
//

#ifndef DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H
#define DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H

#include <QWidget>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QtGlobal>
#include <QDesktopServices>
#include <QMessageBox>
#include <QShortcut>
#include <qcombobox.h>

#include <unordered_set>


#include "Inspector.h"
#include "../../include/database/Drawing.h"
#include "../../include/database/DrawingPDFWriter.h"
#include "../../include/database/componentFilters.h"

#define REBATED_NOTE "Rebate 50mm in down to 15mm"
#define BACKING_STRIPS_NOTE "Backing Strips 6mm Tackyback"
#define RUBBER_COVER_STRAPS_NOTES "Rubber Cover Straps 6mm Tackyback"
#define DRAWING_WTL_STRAPS_NOTE "Wear Tile Liner Strap 6mm Tackyback"
#define SIDE_IRON_WTL_STRAPS_NOTE "Wear Tile Liner Strap 6mm Tackyback - Make to suit Side Iron"

namespace Ui {
    class AddDrawingPageWidget;
}

/// <summary>
/// A widget dedicated to adding a new drawing.
/// </summary>
class AddDrawingPageWidget : public QWidget {
    Q_OBJECT

public:
    enum AddDrawingMode {
        ADD_NEW_DRAWING,
        EDIT_DRAWING,
        CLONE_DRAWING
    };

    /// <summary>
    /// Creates a new widget for a new drawing with the drawing number set.
    /// </summary>
    /// <param name="drawingNumber">Drawing number to assign to new drawing.</param>
    /// <param name="parent">The parent of this widget.</param>
    explicit AddDrawingPageWidget(const std::string &drawingNumber, QWidget *parent = nullptr);

    /// <summary>
    /// Creates a new widget for editing or cloning an already existing drawing.
    /// </summary>
    /// <param name="drawing">The drawing to clone or edit.</param>
    /// <param name="mode">The drawing mode, clone or edit.</param>
    /// <param name="parent">The parent widget of this widget.</param>
    AddDrawingPageWidget(const Drawing &drawing, AddDrawingMode mode, QWidget *parent = nullptr);

    /// <summary>
    /// Destroys this and any relevant objects.
    /// </summary>
    ~AddDrawingPageWidget() override;

    /// <summary>
    /// Confirms that the user wants to commit the changes, then emits a signal to do so.
    /// </summary>
    void confirmDrawing();

    /// <summary>
    /// Sets a callback of what to do when a drawing is added or edited.
    /// </summary>
    /// <param name="callback">The callback to ben ran on a change.</param>
    void setConfirmationCallback(const std::function<void(const Drawing &, bool)> &callback);

    /// <summary>
    /// Set the drawing mode, to clone edit or add.
    /// </summary>
    /// <param name="mode"></param>
    void setMode(AddDrawingMode mode);

    /// <summary>
    /// Sets the email of the creator for the initials.
    /// </summary>
    /// <param name="email">The creators email.</param>
    void setUserEmail(const std::string &email);

protected:
    /// <summary>
    /// Called when widget is closed, to handle quitting verification.
    /// </summary>
    /// <param name="event"></param>
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::AddDrawingPageWidget *ui = nullptr;

    Inspector *inspector = nullptr;

    ComboboxComponentDataSource<Product> productSource;
    ComboboxComponentDataSource<Aperture> apertureSource;
    ComboboxComponentDataSource<Material> topMaterialSource, bottomMaterialSource;
    ComboboxComponentDataSource<SideIron> leftSideIronSource, rightSideIronSource;
    ComboboxComponentDataSource<Machine> machineManufacturerSource, machineModelSource;
    ComboboxComponentDataSource<MachineDeck> machineDeckSource;
    ComboboxComponentDataSource<BackingStrip> backingStripSource;

    SideIronFilter *leftSideIronFilter = nullptr, *rightSideIronFilter = nullptr;
    MachineModelFilter *machineModelFilter = nullptr;

    QGraphicsScene *visualsScene = nullptr;

    Drawing drawing;

    DrawingPDFWriter pdfWriter;

    std::string userEmail;

    unsigned leftSICache = 0, rightSICache = 0;
    bool leftSIInverted = false, rightSIInverted = false;

    // const QRegularExpression drawingNumberRegex = QRegularExpression("^([a-zA-Z]{1,2}[0-9]{2}[a-zA-Z]?|M[0-9]{3}[a-zA-Z]?)$");

    const QRegularExpression drawingNumberRegex = QRegularExpression(QRegularExpression::anchoredPattern("^([a-zA-Z]{1,2}[0-9]{2}[a-zA-Z]?|M[0-9]{3}[a-zA-Z]?)$"));

    std::function<void(const Drawing &, bool)> confirmationCallback;

    AddDrawingMode addMode = ADD_NEW_DRAWING;

    bool drawingAdded = false;
    
    void setupComboboxSources();

    void setupActivators();

    void setupDrawingUpdateConnections();

    void loadDrawing();

    bool checkDrawing(unsigned exclusions = 0);

    enum DrawingNoteType {
        REBATED,
        HAS_BACKING_STRIPS,
        RUBBER_COVER_STRAPS,
        WEAR_TILE_LINER_STRAPS
    };

    std::unordered_set<DrawingNoteType> addedNotes;

    unsigned int* boxesValues = new unsigned int[4]{ 0, 0, 0, 0 };

signals:
    void drawingAddedToDb();
private slots:
    void capitaliseLineEdit(const QString &text);
};

#endif //DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H
