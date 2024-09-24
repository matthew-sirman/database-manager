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
#include <regex>

#include "Inspector.h"
#include "../../include/database/Drawing.h"
#include "../../include/database/DrawingPDFWriter.h"
#include "../../include/database/componentFilters.h"

#define REBATED_NOTE "Rebate 50mm in down to 15mm"
#define BACKING_STRIPS_NOTE "Backing Strips 6mm Tackyback"
#define RUBBER_COVER_STRAPS_NOTES "Rubber Cover Straps 6mm Tackyback"
#define DRAWING_WTL_STRAPS_NOTE "Wear Tile Liner Strap 6mm Tackyback"
#define SIDE_IRON_WTL_STRAPS_NOTE "Wear Tile Liner Strap 6mm Tackyback - Make to suit Side Iron"
#define AUTOPRESS_LOCATION "T:/Drawings/2. Rubber Screen Cloths/Completed Screen Cloth Drawings/AutoPress Drawings"
#define MANUAL_LOCATION "T:/Drawings/2. Rubber Screen Cloths/Completed Screen Cloth Drawings/Manual Drawings"

namespace Ui {
    class AddDrawingPageWidget;
}

/// <summary>
/// A widget dedicated to adding a new drawing.
/// </summary>
class AddDrawingPageWidget : public QWidget {
    Q_OBJECT

public:
    /// <summary>
    /// An enum holding the different modes this object can function in.
    /// </summary>
    enum AddDrawingMode {
        /// <summary>
        /// A new drawing is being added. only loads a the next drawing number and create a blank drawing to build upon.
        /// </summary>
        ADD_NEW_DRAWING,
        /// <summary>
        /// A drawing is being edited. Loads the drawing being edited and makes changed to the object, which is then reinserted to the db.
        /// </summary>
        EDIT_DRAWING,
        /// <summary>
        /// A drawing is being cloned. Copies a provided drawing, provides a new drawing number and ,when finished, is inserted to the db.
        /// </summary>
        CLONE_DRAWING
    };

    /// <summary>
    /// Creates a new widget for a new drawing with the drawing number set.
    /// </summary>
    /// <param name="drawingNumber">Drawing number to assign to new drawing.</param>
    /// <param name="automatic">True if this is an automatic drawing, false otherwise.</param>
    /// <param name="parent">The parent of this widget.</param>
    explicit AddDrawingPageWidget(const std::string &drawingNumber, bool automatic, QWidget *parent = nullptr);

    /// <summary>
    /// Creates a new widget for editing or cloning an already existing drawing.
    /// </summary>
    /// <param name="drawing">The drawing to clone or edit.</param>
    /// <param name="mode">The drawing mode, clone or edit.</param>
    /// <param name="automatic">True if this is an automatic drawing, false otherwise.</param>
    /// <param name="parent">The parent widget of this widget.</param>
    AddDrawingPageWidget(const Drawing &drawing, AddDrawingMode mode, bool automatic, QWidget *parent = nullptr);

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
    const bool autopress;
    
    Ui::AddDrawingPageWidget *ui = nullptr;

    Inspector *inspector = nullptr;

    ComboboxComponentDataSource<Product> productSource;
    ComboboxComponentDataSource<Aperture> apertureSource;
    ComboboxComponentDataSource<Material> topMaterialSource, bottomMaterialSource;
    ComboboxComponentDataSource<SideIron> leftSideIronSource, rightSideIronSource;
    ComboboxComponentDataSource<Machine> machineManufacturerSource, machineModelSource;
    ComboboxComponentDataSource<MachineDeck> machineDeckSource;
    ComboboxComponentDataSource<BackingStrip> backingStripSource;
    ComboboxComponentDataSource<Strap> strapSource;

    SideIronFilter *leftSideIronFilter = nullptr, *rightSideIronFilter = nullptr;
    MachineModelFilter *machineModelFilter = nullptr;

    QGraphicsScene *visualsScene = nullptr;

    Drawing drawing;

    DrawingPDFWriter pdfWriter;

    std::string userEmail;

    unsigned leftSICache = 0, rightSICache = 0;
    bool leftSIInverted = false, rightSIInverted = false;
    //unsigned leftStrapCache = 1, rightStrapCache = 1;

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

    /// <summary>
    /// A set of flags to indicate certain attributes of a drawing.
    /// </summary>
    enum DrawingNoteType {
        /// <summary>
        /// A flag that indicates the mat is rebated when present.
        /// </summary>
        REBATED,
        /// <summary>
        /// A flag that indicates the mat has backing strips when present.
        /// </summary>
        HAS_BACKING_STRIPS,
        /// <summary>
        /// A flag that indicates the mat has rubber cover straps when present.
        /// </summary>
        RUBBER_COVER_STRAPS,
        /// <summary>
        /// A flag that indicates the mat has wear tile liner straps when present.
        /// </summary>
        WEAR_TILE_LINER_STRAPS
    };

    std::unordered_set<DrawingNoteType> addedNotes;

    unsigned int* boxesValues = new unsigned int[4]{ 0, 0, 0, 0 };

signals:
    /// <summary>
    /// Emits this signal when the drawing tries to be added to the database.
    /// </summary>
    void drawingAddedToDb();
private slots:
    void capitaliseLineEdit(const QString &text);
};

#endif //DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H
