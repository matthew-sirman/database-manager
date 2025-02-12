//
// Created by matthew on 06/07/2020.
//

#include "../../include/database/DatabaseManager.h"

// Constructor taking the connection information
DatabaseManager::DatabaseManager(const std::string &database,
                                 const std::string &user,
                                 const std::string &password,
                                 const std::string &host)
   : sess(host, 33060, user, password) {
  this->username = user;
  this->password = password;
  this->database = database;

  isConnected = true;
}

// Method to generate details for the DrawingSummaryCompressionSchema used in
// compression with summaries for a search
void DatabaseManager::getCompressionSchemaDetails(
    unsigned &maxMatID, float &maxWidth, float &maxLength, float &maxLapSize,
    unsigned char &maxBarSpacingCount, float &maxBarSpacing,
    unsigned char &maxDrawingLength, unsigned char &maxExtraApertureCount) {
  // Wrapped in a try statement to catch any MySQL errors.
  try {
    // Select the maximum mat_id from the drawings table
    maxMatID = sess.sql("SELECT MAX(mat_id) FROM " + database + ".drawings")
                   .execute()
                   .fetchOne()[0];

    // Select the maximum width from the drawings table
    maxWidth = sess.sql("SELECT MAX(width) FROM " + database + ".drawings")
                   .execute()
                   .fetchOne()[0];

    // Select the maximum length from the drawings table
    maxLength = sess.sql("SELECT MAX(length) FROM " + database + ".drawings")
                    .execute()
                    .fetchOne()[0];

    // Select the maximum overlap/sidelap size from the union of the overlaps
    // and sidelaps tables
    maxLapSize =
        sess.sql("SELECT MAX(width) FROM (SELECT width FROM " + database +
                 ".sidelaps "
                 "UNION SELECT width FROM " +
                 database + ".overlaps) AS laps")
            .execute()
            .fetchOne()[0];

    maxBarSpacingCount =
        sess.sql(
                "SELECT MAX(spacing_count) FROM (SELECT COUNT(MAT_ID) AS "
                "spacing_count FROM " +
                database + ".bar_spacings GROUP BY mat_id) AS mat_spacings")
            .execute()
            .fetchOne()[0]
            .get<unsigned>();

    maxBarSpacing =
        sess.sql("SELECT MAX(bar_spacing) FROM " + database + ".bar_spacings")
            .execute()
            .fetchOne()[0];

    // Select the length of the longest drawing number from the drawings table
    maxDrawingLength = sess.sql("SELECT MAX(LENGTH(drawing_number)) FROM " +
                                database + ".drawings")
                           .execute()
                           .fetchOne()[0]
                           .get<unsigned>();

    mysqlx::Value r =
        sess.sql(
                "SELECT MAX(count.c) FROM (SELECT COUNT(aperture_id) AS c "
                "FROM " +
                database + ".extra_apertures GROUP BY mat_id) AS count")
            .execute()
            .fetchOne()[0];
    if (r.isNull()) {
      maxExtraApertureCount = 0;
    } else {
      maxExtraApertureCount = r.get<unsigned>();
    }

  } catch (mysqlx::Error &e) {
    // If there was an error, print it to the console and exit the program.
    // This error is fatal - if we are unable to create a compression schema,
    // the database cannot be used.
    Logger::logError(e.what(), __LINE__, __FILE__, false);
  }
}

std::vector<DrawingSummary> DatabaseManager::executeSearchQuery(
    const DatabaseSearchQuery &query) {
  // Wrapped in a try statement to catch any MySQL errors.
  try {
    // First, get the SQL query string from the query object.
    // Then, execute this query (returning a RowResult set).
    // Then, call the static DatabaseSearchQuery method to convert each row into
    // a summary object. Finally, return this list of summaries
    std::string s = Format::format(query.toSQLQueryString(), database);
    return DatabaseSearchQuery::getQueryResultSummaries(sess.sql(s).execute());
  } catch (mysqlx::Error &e) {
    // If there was an error, print it to the console.
    // This is not considered a fatal error; if there was an error simply return
    // an empty set. The client will receive no data, but there is no reason to
    // exit the entire system.
    Logger::logError(e.what(), __LINE__, __FILE__);
    return {};
  }
}

Drawing *DatabaseManager::executeDrawingQuery(const DrawingRequest &query) {
  // Wrapped in a try statement to catch any MySQL errors.
  try {
    // Construct an empty drawing object on the heap, as we will be returning
    // this
    Drawing *drawing = new Drawing();
    drawing->setAsDefault();

    // Declare a string stream for queries to the database.
    std::stringstream queryString;

    // MySQL query for selecting all the data from the drawings table and other
    // associated tables which will only contain at most one row (i.e. apertures
    // and machine templates).
    queryString << "SELECT d.drawing_number AS drawing_number, d.product_id AS "
                   "product_id, d.width AS width, "
                << std::endl;
    queryString << "d.length AS length, d.tension_type AS tension_type, "
                   "UNIX_TIMESTAMP(d.drawing_date) AS drawing_date, "
                << std::endl;
    queryString << "d.rebated, d.hyperlink AS hyperlink, d.notes AS notes, "
                << std::endl;
    queryString << "mt.machine_id AS machine_id, mt.quantity_on_deck AS "
                   "quantity_on_deck, mt.position AS position, "
                << std::endl;
    queryString << "mt.deck_id AS deck_id, " << std::endl;
    queryString << "mal.aperture_id AS aperture_id" << std::endl;
    queryString << "FROM " << database << ".drawings AS d " << std::endl;
    queryString << "INNER JOIN " << database
                << ".machine_templates AS mt ON d.template_id=mt.template_id"
                << std::endl;
    queryString << "INNER JOIN " << database
                << ".mat_aperture_link AS mal ON d.mat_id=mal.mat_id"
                << std::endl;
    queryString << "WHERE d.mat_id=" << query.matID << std::endl;

    // Execute this query string and read the first row. This should never
    // return more than one row as the mat_id is a unique key in the database.
    mysqlx::Row drawingResults =
        sess.sql(queryString.str()).execute().fetchOne();

    // If the results were not null, there was a matching drawing.
    if (!drawingResults.isNull()) {
      // Read in all the information from the query into the drawing object
      drawing->setDrawingNumber(drawingResults[0].get<std::string>());
      drawing->setProduct(DrawingComponentManager<Product>::findComponentByID(
          drawingResults[1]));
      drawing->setWidth(drawingResults[2]);
      drawing->setLength(drawingResults[3]);
      drawing->setTensionType((drawingResults[4].get<std::string>() == "Side")
                                  ? Drawing::SIDE
                                  : Drawing::END);
      drawing->setDate(Date::parse(drawingResults[5].get<int>()));
      drawing->setRebated(drawingResults[6].get<bool>());
      drawing->setHyperlink(drawingResults[7].get<std::string>());
      drawing->setNotes(drawingResults[8].get<std::string>());
      drawing->setMachine(DrawingComponentManager<Machine>::findComponentByID(
          drawingResults[9]));
      drawing->setQuantityOnDeck(drawingResults[10].get<int>());
      drawing->setMachinePosition(drawingResults[11].get<std::string>());
      drawing->setMachineDeck(
          DrawingComponentManager<MachineDeck>::findComponentByID(
              drawingResults[12]));

      // It may be the case that the physical aperture is able to be rotated.
      // This is internally represented in the manager as two separate Aperture
      // objects, both with the same componentID. For this reason, we must
      // search for all apertures with the matching component ID to select the
      // correct one.
      Aperture matchingAperture =
          DrawingComponentManager<Aperture>::findComponentByID(
              drawingResults[13]);
      drawing->setAperture(matchingAperture);
    } else {
      // If the drawing we read from the database was null, print a safe error
      // to the console and set a load warning on the drawing, indicating that
      // we failed to load the drawing.
      lockLog {
        *ss << "Failed to load drawing with mat_id: " +
                                      std::to_string(query.matID);
        Logger::logError();
      }
      drawing->setLoadWarning(Drawing::LoadWarning::LOAD_FAILED);
      // Return the drawing. If we could not gather the critical information,
      // there is no point in gathering any more data.
      return drawing;
    }

    // Get all the matching materials for this drawing. There may be more than
    // one material, so this is done in a separate query.
    mysqlx::RowResult materials = sess.getSchema(database)
                                      .getTable("thickness")
                                      .select("material_thickness_id")
                                      .where("mat_id=:matID")
                                      .bind("matID", query.matID)
                                      .execute();

    // Get the first material
    mysqlx::Row topMaterial = materials.fetchOne();
    if (!topMaterial.isNull()) {
      // If the top material existed in the database, get the material ID from
      // the component manager
      drawing->setMaterial(
          Drawing::MaterialLayer::TOP,
          DrawingComponentManager<Material>::findComponentByID(topMaterial[0]));

      // Check if there is a second material for this drawing
      mysqlx::Row bottomMaterial = materials.fetchOne();
      if (!bottomMaterial.isNull()) {
        // Get the material ID and set this material in the drawing object
        drawing->setMaterial(
            Drawing::MaterialLayer::BOTTOM,
            DrawingComponentManager<Material>::findComponentByID(
                bottomMaterial[0]));
      }
    } else {
      // If we didn't find a material, this is an error. All drawings should
      // have a material.
      Logger::logError("Missing material for drawing: " +
                       drawing->drawingNumber());
      // Set a load warning for the drawing that the material was missing
      drawing->setLoadWarning(Drawing::LoadWarning::MISSING_MATERIAL_DETECTED);
    }

    // Read each bar and its corresponding spacing from the database for this
    // drawing
    mysqlx::RowResult bars = sess.getSchema(database)
                                 .getTable("bar_spacings")
                                 .select("bar_spacing", "bar_width")
                                 .where("mat_id=:matID")
                                 .orderBy("bar_index ASC")
                                 .bind("matID", query.matID)
                                 .execute();

    std::vector<float> barSpacings, barWidths;

    for (const mysqlx::Row &row : bars) {
      // For each bar, add to the barSpacings and barWidths vectors
      barSpacings.push_back(row[0]);
      barWidths.push_back(row[1]);
    }

    // get the backing strip
    mysqlx::RowResult backingStrips = sess.getSchema(database)
                                          .getTable("mat_backing_strip_link")
                                          .select("backing_strip_id")
                                          .where("mat_id=:matID")
                                          .bind("matID", query.matID)
                                          .execute();
    mysqlx::Row backingStrip = backingStrips.fetchOne();
    if (!backingStrip.isNull()) {
      drawing->setBackingStrip(
          DrawingComponentManager<BackingStrip>::findComponentByID(
              backingStrip[0]));
    } else {
      drawing->setLoadWarning(
          Drawing::LoadWarning::INVALID_BACKING_STRIP_DETECTED);
    }
    // Get the side irons associated with this drawing
    mysqlx::RowResult sideIrons =
        sess.getSchema(database)
            .getTable("mat_side_iron_link")
            .select("side_iron_id", "bar_width", "inverted", "cut_down",
                    "fixed_end", "feed_end", "hook_orientation", "strap_id")
            .where("mat_id=:matID")
            .orderBy("side_iron_index ASC")
            .bind("matID", query.matID)
            .execute();

    // Get the left (first) side iron
    mysqlx::Row leftSideIron = sideIrons.fetchOne();

    // If the side iron exists
    if (!leftSideIron.isNull()) {
      // Insert the bar spacing from this database read to the barWidths vector
      barWidths.insert(barWidths.begin(), leftSideIron[1]);
      // Set the left side iron in the drawing object to the side iron from the
      // database
      drawing->setSideIron(Drawing::Side::LEFT,
                           DrawingComponentManager<SideIron>::findComponentByID(
                               leftSideIron[0]));
      // Set whether this side iron is inverted
      drawing->setSideIronInverted(Drawing::LEFT, leftSideIron[2].get<bool>());
      // Set whether this side iron is cut down
      drawing->setSideIronCutDown(Drawing::LEFT, leftSideIron[3].get<bool>());
      if (!leftSideIron[4].isNull()) {
        drawing->setSideIronEnding(Drawing::LEFT,
                                   (Drawing::Ending)leftSideIron[4].get<int>());
      }

      if (!leftSideIron[5].isNull() && leftSideIron[5].get<bool>()) {
        drawing->setSideIronFeed(Drawing::LEFT);
      }
      if (!leftSideIron[6].isNull())
        drawing->setSideIronHookOrientation(
            Drawing::LEFT,
            (Drawing::HookOrientation)leftSideIron[6].get<int>());

      if (!leftSideIron[7].isNull())
        drawing->setSideIronStrap(
            Drawing::LEFT, DrawingComponentManager<Strap>::findComponentByID(
                               leftSideIron[7].get<unsigned>()));
      // Get the right (second) side iron
      mysqlx::Row rightSideIron = sideIrons.fetchOne();
      // If the side iron exists
      if (!rightSideIron.isNull()) {
        // Insert the bar spacing for this right hand bar to the barWidths
        // vector
        barWidths.push_back(rightSideIron[1]);
        // Set the right side iron in the drawing object to the side iron from
        // the database
        drawing->setSideIron(
            Drawing::Side::RIGHT,
            DrawingComponentManager<SideIron>::findComponentByID(
                rightSideIron[0]));
        // Set whether this side iron is inverted
        drawing->setSideIronInverted(Drawing::RIGHT,
                                     rightSideIron[2].get<bool>());

        if (!rightSideIron[4].isNull()) {
          drawing->setSideIronEnding(
              Drawing::RIGHT, (Drawing::Ending)rightSideIron[4].get<int>());
        }

        if (!rightSideIron[5].isNull() && rightSideIron[5].get<bool>()) {
          drawing->setSideIronFeed(Drawing::RIGHT);
        }
        if (!rightSideIron[6].isNull())
          drawing->setSideIronHookOrientation(
              Drawing::RIGHT,
              (Drawing::HookOrientation)rightSideIron[6].get<int>());
      if (!rightSideIron[7].isNull())
        drawing->setSideIronStrap(
            Drawing::RIGHT, DrawingComponentManager<Strap>::findComponentByID(
                               rightSideIron[7].get<unsigned>()));
      } else {
        // If this side iron was missing, set the associated bar spacing to 0
        // and send a load warning that there were missing side irons.
        Logger::logError("Missing right side iron for drawing: " +
                         drawing->drawingNumber());
        barWidths.push_back(0);
        drawing->setLoadWarning(
            Drawing::LoadWarning::MISSING_SIDE_IRONS_DETECTED);
      }
    } else {
      // If this side iron was missing, set the associated bar spacing to 0 and
      // send a load warning that there were missing side irons.
      Logger::logError("Missing side irons for drawing: " +
                       drawing->drawingNumber());
      barWidths.insert(barWidths.begin(), 0);
      barWidths.push_back(0);
      drawing->setLoadWarning(
          Drawing::LoadWarning::MISSING_SIDE_IRONS_DETECTED);
    }

    // Add the final bar width to the barSpacings list. This is calculated by
    // the total width minus each bar spacing stored in the database
    barSpacings.push_back(
        drawing->width() -
        std::accumulate(barSpacings.begin(), barSpacings.end(), 0.0f));

    // Set the barSpacings and barWidths constructed into the drawing object
    drawing->setBars(barSpacings, barWidths);

    // Reset the query string
    queryString.str(std::string());

    // SQL query for selecting each sidelap and overlap from the database
    queryString
        << "SELECT 'S' AS type, mat_side, width, attachment_type, material_id "
        << std::endl;
    queryString << "FROM " << database
                << ".sidelaps WHERE mat_id=" << query.matID << std::endl;
    queryString << "UNION SELECT 'O' AS type, mat_side, width, "
                   "attachment_type, material_id "
                << std::endl;
    queryString << "FROM " << database
                << ".overlaps WHERE mat_id=" << query.matID << std::endl;

    // Execute this query
    mysqlx::RowResult laps = sess.sql(queryString.str()).execute();

    // Loop through each lap the database returned
    for (const mysqlx::Row &lap : laps) {
      // Enum variable to store how this lap is attached (Integral or Bonded)
      LapAttachment attachment;
      if (lap[3].isNull()) {
        // If the attachmentString wasn't valid, indicate that there was an
        // error with the lap.
        Logger::logError("Invalid drawing discovered (invalid lap side): " +
                         drawing->drawingNumber());
        // Set a load warning.
        drawing->setLoadWarning(Drawing::LoadWarning::INVALID_LAPS_DETECTED);
        // Continue - this lap is invalid so we just ignore it. The user will be
        // notified that there was an error
        continue;
      }
      // This data is encoded by a MySQL enum which returns a string in the
      // query
      std::string attachmentString = lap[3].get<std::string>();
      // If the attachmentString is the string "Bonded", set that this is a
      // bonded lap. Otherwise set it to be Integral.
      if (attachmentString == "Bonded") {
        attachment = LapAttachment::BONDED;
      } else if (attachmentString == "Integral") {
        attachment = LapAttachment::INTEGRAL;
      } else {
        // If the attachmentString wasn't valid, indicate that there was an
        // error with the attachment.
        Logger::logError(
            "Invalid drawing discovered (invalid lap attachment): " +
            drawing->drawingNumber());
        // Set a load warning.
        drawing->setLoadWarning(Drawing::LoadWarning::INVALID_LAPS_DETECTED);
        // Continue - this lap is invalid so we just ignore it. The user will be
        // notified that there was an error
        continue;
      }

      // Next we find out which lap we are looking at. This depends on the side
      // value in the table, the tension type of the mat and whether it was a
      // sidelap or overlap
      Drawing::Side side;
      if (lap[1].isNull()) {
        // If the side is null, indicate that there was an error with the lap.
        Logger::logError("Invalid drawing discovered (invalid lap side): " +
                         drawing->drawingNumber());
        // Set a load warning.
        drawing->setLoadWarning(Drawing::LoadWarning::INVALID_LAPS_DETECTED);
        // Continue - this lap is invalid so we just ignore it. The user will be
        // notified that there was an error
        continue;
      }
      std::string sideString = lap[1].get<std::string>();
      if (sideString == "Left") {
        side = Drawing::Side::LEFT;
      } else if (sideString == "Right") {
        side = Drawing::Side::RIGHT;
      } else {
        // If the sideString wasn't valid, indicate that there was an error with
        // the lap.
        Logger::logError("Invalid drawing discovered (invalid lap side) " +
                         drawing->drawingNumber());
        // Set a load warning.
        drawing->setLoadWarning(Drawing::LoadWarning::INVALID_LAPS_DETECTED);
        // Continue - this lap is invalid so we just ignore it. The user will be
        // notified that there was an error
        continue;
      }

      // Set either the sidelap or overlap corresponding to this lap, depending
      // on which table the lap came from. We add the width of the lap, the
      // attachment type and the material.
      if (lap[0].get<std::string>() == "S") {
        drawing->setSidelap(
            side,
            Drawing::Lap(
                lap[2], attachment,
                DrawingComponentManager<Material>::findComponentByID(lap[4])));
      } else {
        drawing->setOverlap(
            side,
            Drawing::Lap(
                lap[2], attachment,
                DrawingComponentManager<Material>::findComponentByID(lap[4])));
      }
    }

    // Get all of the press drawing links from the database (if any)
    std::vector<mysqlx::Row> pressHyperlinkRows =
        sess.getSchema(database)
            .getTable("punch_program_pdfs")
            .select("hyperlink")
            .where("mat_id=:matID")
            .bind("matID", query.matID)
            .execute()
            .fetchAll();

    // Declare a vector for the press hyperlinks and reserve the space for it
    std::vector<std::filesystem::path> pressHyperlinks;
    pressHyperlinks.resize(pressHyperlinkRows.size());

    // Map from the rows to the press hyperlinks list with a functional adapter
    std::transform(pressHyperlinkRows.begin(), pressHyperlinkRows.end(),
                   pressHyperlinks.begin(), [](const mysqlx::Row &row) {
                     return row[0].get<std::string>();
                   });

    // Set the press drawing hyperlinks in the drawing object (this may well be
    // empty)
    drawing->setPressDrawingHyperlinks(pressHyperlinks);

    // Impact Pads
    mysqlx::RowResult impactPadResults =
        sess.getSchema(database)
            .getTable("impact_pads")
            .select("material_id", "aperture_id", "width", "length", "x_coord",
                    "y_coord")
            .where("mat_id=:matID")
            .bind("matID", query.matID)
            .execute();

    for (const mysqlx::Row &row : impactPadResults) {
      Drawing::ImpactPad pad;
      pad.setMaterial(
          DrawingComponentManager<Material>::findComponentByID(row[0]));

      Aperture matchingAperture =
          DrawingComponentManager<Aperture>::findComponentByID(row[1]);
      pad.setAperture(matchingAperture);

      pad.width = row[2].get<float>();
      pad.length = row[3].get<float>();
      pad.pos.x = row[4].get<float>();
      pad.pos.y = row[5].get<float>();

      drawing->addImpactPad(pad);
    }

    // Blank Spaces
    mysqlx::RowResult blankSpaceResults =
        sess.getSchema(database)
            .getTable("blank_spaces")
            .select("width", "length", "x_coord", "y_coord")
            .where("mat_id=:matID")
            .bind("matID", query.matID)
            .execute();

    for (const mysqlx::Row &row : blankSpaceResults) {
      Drawing::BlankSpace space;

      space.width = row[0].get<float>();
      space.length = row[1].get<float>();
      space.pos.x = row[2].get<float>();
      space.pos.y = row[3].get<float>();

      drawing->addBlankSpace(space);
    }

    // Extra Apertures
    mysqlx::RowResult extraApertureResult =
        sess.getSchema(database)
            .getTable("extra_apertures")
            .select("width", "length", "x_coord", "y_coord", "aperture_id")
            .where("mat_id=:matID")
            .bind("matID", query.matID)
            .execute();

    for (const mysqlx::Row &row : extraApertureResult) {
      Drawing::ExtraAperture aperture;

      aperture.width = row[0].get<float>();
      aperture.length = row[1].get<float>();
      aperture.pos.x = row[2].get<float>();
      aperture.pos.y = row[3].get<float>();
      aperture.setAperture(DrawingComponentManager<Aperture>::findComponentByID(
          row[4].get<unsigned>()));

      drawing->addExtraAperture(aperture);
    }

    // Dam Bars
    mysqlx::RowResult damBarResults =
        sess.getSchema(database)
            .getTable("dam_bars")
            .select("width", "length", "x_coord", "y_coord", "material_id")
            .where("mat_id=:matID")
            .bind("matID", query.matID)
            .execute();

    for (const mysqlx::Row &row : damBarResults) {
      Drawing::DamBar bar;

      bar.width = row[0].get<float>();
      bar.length = row[1].get<float>();
      bar.pos.x = row[2].get<float>();
      bar.pos.y = row[3].get<float>();
      bar.setMaterial(
          DrawingComponentManager<Material>::findComponentByID(row[4]));

      drawing->addDamBar(bar);
    }

    // Center Holes
    mysqlx::RowResult centreHoleResults =
        sess.getSchema(database)
            .getTable("centre_holes")
            .select("x_coord", "y_coord", "aperture_id")
            .where("mat_id=:matID")
            .bind("matID", query.matID)
            .execute();

    for (const mysqlx::Row &row : centreHoleResults) {
      Drawing::CentreHole hole;
      hole.pos.x = row[0].get<float>();
      hole.pos.y = row[1].get<float>();
      hole.setAperture(DrawingComponentManager<Aperture>::findComponentByID(
          row[2].get<unsigned>()));

      drawing->addCentreHole(hole);
    }

    // Deflectors
    mysqlx::RowResult deflectorResults =
        sess.getSchema(database)
            .getTable("deflectors")
            .select("material_id", "size", "x_coord", "y_coord")
            .where("mat_id=:matID")
            .bind("matID", query.matID)
            .execute();

    for (const mysqlx::Row &row : deflectorResults) {
      Drawing::Deflector deflector;
      deflector.setMaterial(
          DrawingComponentManager<Material>::findComponentByID(row[0]));
      deflector.size = row[1].get<float>();
      deflector.pos.x = row[2].get<float>();
      deflector.pos.y = row[3].get<float>();

      drawing->addDeflector(deflector);
    }

    // Divertors
    mysqlx::RowResult divertorResults =
        sess.getSchema(database)
            .getTable("divertors")
            .select("material_id", "width", "length", "mat_side", "y_coord")
            .where("mat_id=:matID")
            .bind("matID", query.matID)
            .execute();

    for (const mysqlx::Row &row : divertorResults) {
      Drawing::Divertor divertor;
      divertor.setMaterial(
          DrawingComponentManager<Material>::findComponentByID(row[0]));
      divertor.width = row[1].get<float>();
      divertor.length = row[2].get<float>();
      if (row[3].get<std::string>() == "Left") {
        divertor.side = Drawing::LEFT;
      } else {
        divertor.side = Drawing::RIGHT;
      }
      divertor.verticalPosition = row[4].get<float>();

      drawing->addDivertor(divertor);
    }

    // Return the constructed drawing
    return drawing;
  } catch (mysqlx::Error &e) {
    // If there was an error, print it to the console.
    // This is not considered a fatal error; if there was an error we just
    // return a nullptr
    Logger::logError(e.what(), __LINE__, __FILE__);
    return nullptr;
  }
}

mysqlx::SqlResult DatabaseManager::sourceTable(
    const std::string &tableName,
                                               const std::string &orderBy) {
  // Wrapped in a try statement to catch any MySQL errors.
  try {
            return sess
        .sql("SELECT * FROM " + database + "." + tableName +
             (orderBy.empty() ? "" : " ORDER BY " + orderBy))
                .execute();
    } catch (mysqlx::Error &e) {
    // If there was an error, print it to the console.
    // This is not considered a fatal error; if there was an error we just
    // return an empty row set
    Logger::logError(e.what(), __LINE__, __FILE__);
    return {};
  }
}

mysqlx::SqlResult DatabaseManager::sourceMultipleTable(
    const std::string &leftTable, const std::string &rightTable,
    std::tuple<std::string, std::string> commons, const std::string &orderBy) {
  // Wrapped in a try statement to catch any MySQL errors.
  try {
    std::stringstream ss;
    ss << "SELECT * FROM " << database << "." << leftTable << " RIGHT JOIN "
       << database << "." << rightTable << " ON ";
    ss << database << "." << leftTable << "." << std::get<0>(commons) << " = "
       << database << "." << rightTable << "." << std::get<1>(commons)
       << (orderBy.empty() ? "" : " ORDER BY " + orderBy) << std::endl;
    return sess.sql(ss.str()).execute();
  } catch (mysqlx::Error &e) {
    // If there was an error, print it to the console.
    // This is not considered a fatal error; if there was an error we just
    // return an empty row set
    Logger::logError(e.what(), __LINE__, __FILE__);
    return {};
  }
}

mysqlx::SqlResult DatabaseManager::sourceMultipleTable(
    const std::string &leftTable, const std::string &rightTable,
    const std::string &common, const std::string &orderBy) {
  return sourceMultipleTable(leftTable, rightTable, {common, common});
}

bool DatabaseManager::insertDrawing(const DrawingInsert &insert) {
  // Wrapped in a try statement to catch any MySQL errors.
  try {
    // The drawing data in the insert object is an optional, so if it unset,
    // there is no drawing to insert into the database. Therefore, we just
    // return false indicating the insertion failed.
    if (!insert.drawingData.has_value()) {
      return false;
    }
    // To avoid entering corrupted data, we check the drawing for validity (e.g.
    // the widths add up correctly etc.) If the drawing has made it this far,
    // this check should pass, however it is a good idea to check anyway. If it
    // fails this test, return false.
    if (insert.drawingData->checkDrawingValidity() != Drawing::SUCCESS) {
      return false;
    }

    sess.startTransaction();

    // Before adding the drawing, we must check if it exists in the database, so
    // we select any records with the same drawing number.
    mysqlx::Row existingDrawing =
        sess.getSchema(database)
            .getTable("drawings")
            .select("mat_id")
            .where("drawing_number=:drawingNumber")
            .bind("drawingNumber", insert.drawingData->drawingNumber())
            .execute()
            .fetchOne();

    // If the resulting row is not null, i.e. the drawing exist,
    if (!existingDrawing.isNull()) {
      // If we are not in forcing mode (which indicates we should remove and
      // reinsert the drawing) then we return failure as the drawing exists
      if (!insert.forcing()) {
        sess.rollback();
        return false;
      }
      // Otherwise, delete the old drawing from the database.
      sess.getSchema(database)
          .getTable("drawings")
          .remove()
          .where("mat_id=:matID")
          .bind("matID", existingDrawing[0])
          .execute();
    }

    // Each insertion query is constructed as an SQL query by the insert object.

    // Next, we check if the machine template is in the database already
    mysqlx::Row existingTemplate =
        sess.sql(Format::format(insert.testMachineTemplateQuery(), database))
            .execute()
            .fetchOne();

    unsigned templateID;

    if (!existingTemplate.isNull()) {
      // If the template was present, we are done and we just assign the
      // templateID to the one from the database
      templateID = existingTemplate[0];
    } else {
      // Otherwise, we add the template
      sess.sql(Format::format(insert.machineTemplateInsertQuery(), database))
          .execute();
      // "SELECT @@identity" returns the most recently added ID from this
      // session, and so there should be no race condition issue here.
      templateID = sess.sql("SELECT @@identity").execute().fetchOne()[0];
    }

    // Next, we insert the drawing itself into the database and retrieve its ID.
    // The rest of the inserts all depend on the matID, in order to link to this
    // specific drawing.
    sess.sql(Format::format(insert.drawingInsertQuery(templateID), database))
        .execute();
    unsigned matID = sess.sql("SELECT @@identity").execute().fetchOne()[0];

    // Get the query string for the bars. If this is empty, this indicates that
    // there are none to insert, so we just ignore it.
    std::string barInsertQuery = insert.barSpacingInsertQuery(matID);

    if (!barInsertQuery.empty()) {
      sess.sql(Format::format(barInsertQuery, database)).execute();
    }

    // Insert the aperture
    sess.sql(Format::format(insert.apertureInsertQuery(matID), database))
        .execute();
    // Insert the backing strips
    std::string backingStripInsert = insert.backingStripInsertQuery(matID);
    if (backingStripInsert != "")
      sess.sql(Format::format(backingStripInsert, database)).execute();
    // Insert the side irons
    sess.sql(Format::format(insert.sideIronInsertQuery(matID), database))
        .execute();
    // Insert the materials
    sess.sql(Format::format(insert.thicknessInsertQuery(matID), database))
        .execute();

    // Get the query strings for the overlaps and sidelaps. If they are empty,
    // this indicates that there are none to insert, so we just ignore them.
    std::string overlapInsert = insert.overlapsInsertQuery(matID),
                sidelapInsert = insert.sidelapsInsertQuery(matID);

    // If we have overlaps to insert, execute the overlap insert query
    if (!overlapInsert.empty()) {
      sess.sql(Format::format(overlapInsert, database)).execute();
    }
    // If we have sidelaps to insert, execute the sidelap insert query
    if (!sidelapInsert.empty()) {
      sess.sql(Format::format(sidelapInsert, database)).execute();
    }

    // Get the punch program insert query. Most of the time, there are none to
    // insert, so again if this returns an empty string, we ignore it.
    std::string punchProgramInsert = insert.punchProgramsInsertQuery(matID);

    // If we have punch program PDFs to insert, execute the insert query
    if (!punchProgramInsert.empty()) {
      sess.sql(Format::format(punchProgramInsert, database)).execute();
    }

    std::string impactPadsInsert = insert.impactPadsInsertQuery(matID),
                centreHolesInsert = insert.centreHolesInsertQuery(matID),
                deflectorsInsert = insert.deflectorsInsertQuery(matID),
                divertorsInsert = insert.divertorsInsertQuery(matID),
                blankSpaceInsert = insert.blankSpaceInsertQuery(matID),
                extraApertureInsert = insert.extraApertureInsertQuery(matID),
                damBarInsert = insert.damBarInsertQuery(matID);

    if (!impactPadsInsert.empty()) {
      sess.sql(Format::format(impactPadsInsert, database)).execute();
    }
    if (!extraApertureInsert.empty()) {
      sess.sql(Format::format(extraApertureInsert, database)).execute();
    }
    if (!blankSpaceInsert.empty()) {
      sess.sql(Format::format(blankSpaceInsert, database)).execute();
    }
    if (!damBarInsert.empty()) {
      sess.sql(Format::format(damBarInsert, database)).execute();
    }
    if (!centreHolesInsert.empty()) {
      sess.sql(Format::format(centreHolesInsert, database)).execute();
    }
    if (!deflectorsInsert.empty()) {
      sess.sql(Format::format(deflectorsInsert, database)).execute();
    }
    if (!divertorsInsert.empty()) {
      sess.sql(Format::format(divertorsInsert, database)).execute();
    }

    // Finally, if all insertions were successful, we commit and return that the
    // drawing insert was successful.
    sess.commit();
    return true;
  } catch (mysqlx::Error &e) {
    // If there was an error, print it to the console.
    // This is not considered a fatal error; if there was an error we just
    // return that insertion failed
    Logger::logError(e.what(), __LINE__, __FILE__);
    sess.rollback();
    return false;
  }
}

DatabaseManager::DrawingExistsResponse DatabaseManager::drawingExists(
    const std::string &drawingNumber) {
  // Wrapped in a try statement to catch any MySQL errors.
  try {
    // Search the database for a drawing with the drawingNumber passed in. If
    // the result is not null, return that the drawing exists. Otherwise, return
    // that the drawing does not exist.
    if (!sess.getSchema(database)
             .getTable("drawings")
             .select("mat_id")
             .where("drawing_number=:drawingNumber")
             .bind("drawingNumber", drawingNumber)
             .execute()
             .fetchOne()
             .isNull()) {
      return DrawingExistsResponse::EXISTS;
    } else {
      return DrawingExistsResponse::NOT_EXISTS;
    }
  } catch (mysqlx::Error &e) {
    // If there was an error, print it to the console.
    // This is not considered a fatal error; if there was an error we just
    // return that the test failed
    Logger::logError(e.what(), __LINE__, __FILE__);
    return DrawingExistsResponse::R_ERROR;
  }
}

bool DatabaseManager::insertComponent(const ComponentInsert &insert) {
  // Wrapped in a try statement to catch any MySQL errors.
  try {
    // Get the insert query string from the insert object. This string will be
    // empty if there is nothing to insert (which likely indicates an error at
    // some stage)
    std::string insertString = insert.toSQLQueryString();

    // If there is a query string to execute, execute it
    if (!insertString.empty()) {
      sess.sql(Format::format(insertString, database)).execute();
    } else {
      // Return that the insertion failed - there was nothing to insert for some
      // reason
      return false;
    }

    // Return that the transaction was successful
    return true;
  } catch (mysqlx::Error &e) {
    // If there was an error, print it to the console.
    // This is not considered a fatal error; if there was an error we just
    // return that the insertion failed
    Logger::logError(e.what(), __LINE__, __FILE__);
    return false;
  }
}

bool DatabaseManager::createBackup(
    const std::filesystem::path &backupLocation) {
  std::stringstream command;

  command << "mysqldump";
  command << " -u " << username;
  command << " -p\"" << password << "\"";
  command << " --result-file " << backupLocation;
  command << " --no-tablespaces ";
  command << " " << database;

#ifdef _WIN32
  FILE *pipe = _popen(command.str().c_str(), "r");
#else
  FILE *pipe = popen(command.str().c_str(), "r");
#endif

  if (!pipe) {
    return false;
  }

#ifdef _WIN32
  _pclose(pipe);
#else
  pclose(pipe);
#endif

  return true;
}

std::string DatabaseManager::nextAutomaticDrawingNumber() {
  // Wrapped in a try statement to catch any MySQL errors.
  try {
    std::string sqlQuery =
        "SELECT drawing_number FROM " + database +
        ".drawings "
        "ORDER BY IF(drawing_number REGEXP '^[A-Z][0-9]{2,}[A-Z]?$', "
        "CONCAT('0', drawing_number), drawing_number) DESC LIMIT 1;";

    mysqlx::Row row = sess.sql(sqlQuery).execute().fetchOne();

    if (!row.isNull()) {
      std::string latest = row[0].get<std::string>();

      std::string charSection, numberSection;

      unsigned index = 0;

      while (index < latest.size()) {
        if (std::isalpha(latest.at(index))) {
          charSection += latest.at(index++);
        } else {
          break;
        }
      }
      while (index < latest.size()) {
        if (std::isdigit(latest.at(index))) {
          numberSection += latest.at(index++);
        } else {
          break;
        }
      }

      unsigned char number = std::stoi(numberSection);

      std::stringstream next;

      if (number == 99) {
        index = charSection.size() - 1;
        while (index >= 0) {
          char c = charSection[index];
          if (c == 'Z') {
            charSection[index--] = 'A';
          } else {
            charSection[index]++;
            break;
          }
        }
        number = 0;
      }

      next << charSection << number + 1;

      return next.str();
    }

    Logger::logError("Failed to find next automatic drawing number.");

    return std::string();

  } catch (mysqlx::Error &e) {
    // If there was an error, print it to the console.
    // This is not considered a fatal error
    Logger::logError(e.what(), __LINE__, __FILE__);
    return std::string();
  }
}

std::string DatabaseManager::nextManualDrawingNumber() {
  // Wrapped in a try statement to catch any MySQL errors.
  try {
    std::string sqlQuery =
        "SELECT drawing_number FROM " + database +
        ".drawings "
        "WHERE drawing_number LIKE 'M%' "
        "ORDER BY CAST(SUBSTRING(drawing_number, 2) AS UNSIGNED) DESC LIMIT 1;";

    mysqlx::SqlResult test = sess.sql(sqlQuery).execute();
    mysqlx::Row row = test.fetchOne();

    if (!row.isNull()) {
      std::string latest = row[0].get<std::string>();

      std::string charSection, numberSection;

      unsigned index = 0;

      while (index < latest.size()) {
        if (std::isalpha(latest.at(index))) {
          charSection += latest.at(index++);
        } else {
          break;
        }
      }
      while (index < latest.size()) {
        if (std::isdigit(latest.at(index))) {
          numberSection += latest.at(index++);
        } else {
          break;
        }
      }

      unsigned number = std::stoi(numberSection);

      std::stringstream next;

      next << charSection << number + 1;

      return next.str();
    }

    Logger::logError("Failed to find next manual drawing number.");

    return std::string();
  } catch (mysqlx::Error &e) {
    // If there was an error, print it to the console.
    // This is not considered a fatal error
    Logger::logError(e.what(), __LINE__, __FILE__);
    return std::string();
  }
}

void DatabaseManager::closeConnection() {
  // Wrapped in a try statement to catch any MySQL errors.
  try {
    // Close the session
    sess.close();
    isConnected = false;
  } catch (mysqlx::Error &e) {
    // If there was an error, print it to the console.
    // This is not considered a fatal error, though this function will generally
    // only be called during shutdown, so it is likely that no further
    // operations will be performed afterwards anyway.
    Logger::logError(e.what(), __LINE__, __FILE__);
  }
}

bool DatabaseManager::testConnection() {
  // Wrapped in a try statement to catch any MySQL errors.
  try {
    // Get whether this simple SQL statement returns data or not
    bool success = sess.sql("SELECT 1").execute().hasData();

    // If we were unsuccessful, close th connection
    if (!success) {
      closeConnection();
    }
    // Return whether we were successful or not. If this query does not return
    // data, the connection is broken.
    return success;
  } catch (mysqlx::Error &e) {
    // If there was an error, this indicates that the connection was broken, as
    // the simple SQL statement above should never fail. We print the error to
    // the console
    Logger::logError(e.what(), __LINE__, __FILE__);
    closeConnection();
    return false;
  }
  return false;
}

bool DatabaseManager::connected() const { return isConnected; }

// void DatabaseManager::setErrorStream(std::ostream &stream) {
//	errStream = &stream;
// }
//
// void DatabaseManager::logError(const mysqlx::Error &e, unsigned lineNumber,
// bool safe) {
//     *errStream << timestamp() << " ERROR::DatabaseManager.cpp:";
//     if (lineNumber != -1) {
//         *errStream << lineNumber << ":";
//     }
//     *errStream <<  " SQL Error " << e << std::endl;
//
//     if (!safe) {
//         exit(1);
//     }
// }
//
// void DatabaseManager::logError(const std::string& e, unsigned lineNumber,
//     bool safe) {
//
//
//     if (!safe) {
//         exit(1);
//     }
// }
//
// std::string DatabaseManager::timestamp() const {
//     std::stringstream ss;
//     std::time_t now = time(nullptr);
//     std::tm *timePoint = std::localtime(&now);
//     ss << "[" << std::put_time(timePoint, "%d/%m/%Y %H:%M:%S") << "] ";
//     return ss.str();
// }
