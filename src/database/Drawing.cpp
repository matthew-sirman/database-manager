//
// Created by matthew on 06/07/2020.
//

#include "../../include/database/Drawing.h"

void writeAtBitOffset(void *value, size_t valueByteLength, void *target,
                      size_t bitOffset) {
  size_t startIndex = bitOffset / 8;
  byte offset = bitOffset % 8;
  byte writeByte;

  byte *valueBytes = (byte *)value, *targetBytes = (byte *)target + startIndex;

  if (offset == 0) {
    memcpy(targetBytes, value, valueByteLength);
    return;
  }

  writeByte = *targetBytes;

  for (unsigned i = 0; i < valueByteLength; i++) {
    writeByte |= (byte)(valueBytes[i] << offset);
    *targetBytes++ = writeByte;
    writeByte = valueBytes[i] >> (8u - offset);
  }

  *targetBytes = writeByte;
}

void readFromBitOffset(void *data, size_t bitOffset, void *target,
                       size_t bitReadSize) {
  size_t startIndex = bitOffset / 8, fullReadLength = bitReadSize / 8;
  byte offset = bitOffset % 8, finalByteSize = bitReadSize % 8;
  byte readByte;

  byte *dataBytes = (byte *)data, *targetBytes = (byte *)target;

  if (offset == 0) {
    memcpy(target, dataBytes + startIndex, fullReadLength);

    if (finalByteSize != 0) {
      targetBytes[fullReadLength] = dataBytes[startIndex + fullReadLength] &
                                    (0xFFu >> (8u - finalByteSize));
    }

    return;
  }

  for (unsigned i = 0; i < fullReadLength; i++) {
    readByte = dataBytes[startIndex + i] >> offset;
    readByte |= (byte)(dataBytes[startIndex + i + 1] << (8u - offset));
    *targetBytes++ = readByte;
  }

  if (finalByteSize != 0) {
    readByte = dataBytes[startIndex + fullReadLength] >> offset;
    readByte |=
        (byte)(dataBytes[startIndex + fullReadLength + 1] << (8u - offset));
    *targetBytes = readByte & (0xFFu >> (8u - finalByteSize));
  }
}

Date::Date(unsigned year, unsigned month, unsigned day) {
  this->year = year;
  this->month = month;
  this->day = day;
}

std::string Date::toMySQLDateString() const {
  std::stringstream ss;
  ss << year << "-" << (unsigned)month << "-" << (unsigned)day << " 00:00:00";
  return ss.str();
}

Date Date::parse(std::time_t rawDate) {
  std::tm *timePoint = std::localtime(&rawDate);
  return {(unsigned)(timePoint->tm_year + 1900),
          (unsigned)(timePoint->tm_mon + 1), (unsigned)timePoint->tm_mday};
}

Date Date::today() {
  time_t now = time(0);
  std::tm *timePoint = std::localtime(&now);
  return {(unsigned)(timePoint->tm_year + 1900),
          (unsigned)(timePoint->tm_mon + 1), (unsigned)timePoint->tm_mday};
}

Drawing::Drawing() = default;

Drawing::Drawing(const Drawing &drawing) {
  this->__drawingNumber = drawing.__drawingNumber;
  this->__date = drawing.__date;
  this->__width = drawing.__width;
  this->__length = drawing.__length;
  this->__hyperlink = drawing.__hyperlink;
  this->__notes = drawing.__notes;
  this->__machineTemplate = drawing.__machineTemplate;
  this->productHandle = drawing.productHandle;
  this->apertureHandle = drawing.apertureHandle;
  this->backingStripHandle = drawing.backingStripHandle;
  this->__tensionType = drawing.__tensionType;
  this->__rebated = drawing.__rebated;
  this->__pressDrawingHyperlinks = drawing.__pressDrawingHyperlinks;
  this->barSpacings = drawing.barSpacings;
  this->barWidths = drawing.barWidths;
  this->sideIronHandles[0] = drawing.sideIronHandles[0];
  this->sideIronHandles[1] = drawing.sideIronHandles[1];
  this->sideIronsInverted[0] = drawing.sideIronsInverted[0];
  this->sideIronsInverted[1] = drawing.sideIronsInverted[1];
  this->sideIronsCutDown[0] = drawing.sideIronsCutDown[0];
  this->sideIronsCutDown[1] = drawing.sideIronsCutDown[1];
  this->feedEnd = drawing.feedEnd;
  this->ending[0] = drawing.ending[0];
  this->ending[1] = drawing.ending[1];
  this->hookOrientation[0] = drawing.hookOrientation[0];
  this->hookOrientation[1] = drawing.hookOrientation[1];
  this->strapHandle[0] = drawing.strapHandle[0];
  this->strapHandle[1] = drawing.strapHandle[1];
  this->sidelaps[0] = drawing.sidelaps[0];
  this->sidelaps[1] = drawing.sidelaps[1];
  this->overlaps[0] = drawing.overlaps[0];
  this->overlaps[1] = drawing.overlaps[1];
  this->topLayerThicknessHandle = drawing.topLayerThicknessHandle;
  this->bottomLayerThicknessHandle = drawing.bottomLayerThicknessHandle;
  this->__impactPads = drawing.__impactPads;
  this->__damBars = drawing.__damBars;
  this->__blankSpaces = drawing.__blankSpaces;
  this->__extraApertures = drawing.__extraApertures;
  this->__centreHoles = drawing.__centreHoles;
  this->__deflectors = drawing.__deflectors;
  this->__divertors = drawing.__divertors;
  this->loadWarnings = drawing.loadWarnings;
}

Drawing::~Drawing() = default;

void Drawing::setAsDefault() {
  this->__drawingNumber = "";
  this->__date = {2000, 1, 1};
  this->__width = 0;
  this->__length = 0;
  this->__hyperlink = "";
  this->__notes = "";
  this->__machineTemplate = MachineTemplate();
  this->productHandle = 0;
  this->apertureHandle = 0;
  this->backingStripHandle = std::nullopt;
  this->__tensionType = TensionType::SIDE;
  this->__rebated = false;
  // this->__hasBackingStrips = false;
  this->__pressDrawingHyperlinks = std::vector<std::filesystem::path>();
  this->barSpacings = {0};
  this->barWidths = {0, 0};
  this->sideIronHandles[0] =
      DrawingComponentManager<SideIron>::findComponentByID(1).handle();
  this->sideIronHandles[1] =
      DrawingComponentManager<SideIron>::findComponentByID(1).handle();
  this->sideIronsInverted[0] = false;
  this->sideIronsInverted[1] = false;
  this->sideIronsCutDown[0] = false;
  this->sideIronsCutDown[1] = false;
  this->feedEnd = std::nullopt;
  this->ending[0] = std::nullopt;
  this->ending[1] = std::nullopt;
  this->hookOrientation[0] = std::nullopt;
  this->hookOrientation[1] = std::nullopt;
  this->sidelaps[0] = std::nullopt;
  this->sidelaps[1] = std::nullopt;
  this->overlaps[0] = std::nullopt;
  this->overlaps[1] = std::nullopt;
  this->topLayerThicknessHandle = 0;
  this->bottomLayerThicknessHandle = std::nullopt;
  this->__impactPads = {};
  this->__damBars = {};
  this->__blankSpaces = {};
  this->__extraApertures = {};
  this->__centreHoles = {};
  this->__deflectors = {};
  this->__divertors = {};
  this->loadWarnings = 0;
}

std::string Drawing::drawingNumber() const { return __drawingNumber; }

void Drawing::setDrawingNumber(const std::string &newDrawingNumber) {
  __drawingNumber = newDrawingNumber;
  invokeUpdateCallbacks();
}

Date Drawing::date() const { return __date; }

void Drawing::setDate(Date newDate) {
  __date = newDate;
  invokeUpdateCallbacks();
}

float Drawing::width() const { return __width; }

void Drawing::setWidth(float newWidth) {
  __width = newWidth;
  invokeUpdateCallbacks();
}

float Drawing::length() const { return __length; }

void Drawing::setLength(float newLength) {
  __length = newLength;
  invokeUpdateCallbacks();
}

std::filesystem::path Drawing::hyperlink() const { return __hyperlink; }

void Drawing::setHyperlink(const std::filesystem::path &newHyperlink) {
  __hyperlink = newHyperlink;
  invokeUpdateCallbacks();
}

std::string Drawing::notes() const { return __notes; }

void Drawing::setNotes(const std::string &newNotes) {
  __notes = newNotes;
  invokeUpdateCallbacks();
}

Drawing::MachineTemplate Drawing::machineTemplate() const {
  return __machineTemplate;
}

void Drawing::setMachineTemplate(const Machine &machine,
                                 unsigned int quantityOnDeck,
                                 const std::string &position,
                                 const MachineDeck &deck) {
  __machineTemplate.machineHandle = machine.handle();
  __machineTemplate.quantityOnDeck = quantityOnDeck;
  __machineTemplate.position = position;
  __machineTemplate.deckHandle = deck.handle();
  invokeUpdateCallbacks();
}

void Drawing::setMachine(const Machine &machine) {
  __machineTemplate.machineHandle = machine.handle();
  invokeUpdateCallbacks();
}

void Drawing::setQuantityOnDeck(unsigned quantityOnDeck) {
  __machineTemplate.quantityOnDeck = quantityOnDeck;
  invokeUpdateCallbacks();
}

void Drawing::setMachinePosition(const std::string &position) {
  __machineTemplate.position = position;
  invokeUpdateCallbacks();
}

void Drawing::setMachineDeck(const MachineDeck &deck) {
  __machineTemplate.deckHandle = deck.handle();
  invokeUpdateCallbacks();
}

Product Drawing::product() const {
  return DrawingComponentManager<Product>::getComponentByHandle(productHandle);
}

void Drawing::setProduct(const Product &prod) {
  productHandle = prod.handle();
  invokeUpdateCallbacks();
}

Aperture &Drawing::aperture() const {
  return DrawingComponentManager<Aperture>::getComponentByHandle(
      apertureHandle);
}

void Drawing::setAperture(const Aperture &ap) {
  apertureHandle = ap.handle();
  invokeUpdateCallbacks();
}

Drawing::TensionType Drawing::tensionType() const { return __tensionType; }

void Drawing::setTensionType(Drawing::TensionType newTensionType) {
  __tensionType = newTensionType;
  invokeUpdateCallbacks();
}

bool Drawing::rebated() const { return __rebated; }

void Drawing::setRebated(bool isRebated) { __rebated = isRebated; }

std::optional<BackingStrip> Drawing::backingStrip() const {
  if (backingStripHandle == std::nullopt) return std::nullopt;
  return DrawingComponentManager<BackingStrip>::getComponentByHandle(
      backingStripHandle.value());
}

void Drawing::setBackingStrip(const BackingStrip &strip) {
  backingStripHandle = strip.handle();
}

void Drawing::removeBackingStrip() { backingStripHandle = std::nullopt; }

bool Drawing::hasBackingStrips() const {
  if (backingStripHandle == std::nullopt) return false;
  return DrawingComponentManager<BackingStrip>::validComponentHandle(
      backingStripHandle.value());
}

std::optional<Material> Drawing::material(Drawing::MaterialLayer layer) const {
  switch (layer) {
    case TOP:
      return DrawingComponentManager<Material>::getComponentByHandle(
          topLayerThicknessHandle);
    case BOTTOM:
      if (bottomLayerThicknessHandle.has_value()) {
        return DrawingComponentManager<Material>::getComponentByHandle(
            bottomLayerThicknessHandle.value());
      } else {
        return std::nullopt;
      }
  }
  return std::nullopt;
}

void Drawing::setMaterial(Drawing::MaterialLayer layer, const Material &mat) {
  switch (layer) {
    case TOP:
      topLayerThicknessHandle = mat.handle();
      break;
    case BOTTOM:
      bottomLayerThicknessHandle = mat.handle();
      break;
  }
  invokeUpdateCallbacks();
}

void Drawing::removeBottomLayer() {
  bottomLayerThicknessHandle = std::nullopt;
  invokeUpdateCallbacks();
}

unsigned Drawing::numberOfBars() const { return MAX(0, barWidths.size() - 2); }

void Drawing::setBars(const std::vector<float> &spacings,
                      const std::vector<float> &widths) {
  barSpacings = spacings;
  barWidths = widths;
  invokeUpdateCallbacks();
}

float Drawing::barSpacing(unsigned int index) const {
  return barSpacings[index];
}

float Drawing::barWidth(unsigned int index) const { return barWidths[index]; }

float Drawing::leftMargin() const { return barWidths.front(); }

float Drawing::rightMargin() const { return barWidths.back(); }

std::vector<float> Drawing::allBarSpacings() const { return barSpacings; }

const std::vector<float> &Drawing::allBarWidths() const { return barWidths; }

SideIron Drawing::sideIron(Drawing::Side side) const {
  switch (side) {
    case LEFT:
      return DrawingComponentManager<SideIron>::getComponentByHandle(
          sideIronHandles[0]);
    case RIGHT:
      return DrawingComponentManager<SideIron>::getComponentByHandle(
          sideIronHandles[1]);
  }
  ERROR_RAW(
      "Invalid Side Iron side requested. The side must be either Left or Right",
      std::cerr);
}

bool Drawing::sideIronInverted(Drawing::Side side) const {
  switch (side) {
    case LEFT:
      return sideIronsInverted[0];
    case RIGHT:
      return sideIronsInverted[1];
  }
  ERROR_RAW(
      "Invalid Side Iron side requested. The side must be either Left or Right",
      std::cerr);
}

bool Drawing::sideIronCutDown(Drawing::Side side) const {
  switch (side) {
    case LEFT:
      return sideIronsCutDown[0];
    case RIGHT:
      return sideIronsCutDown[1];
  }
  ERROR_RAW(
      "Invalid Side Iron side requested. The side must be either Left or Right",
      std::cerr);
}
bool Drawing::sideIronFixedEnd() const {
  return (ending[0].has_value() || ending[1].has_value());
}

std::optional<Drawing::Ending> Drawing::sideIronFixedEnd(Side side) const {
  switch (side) {
    case LEFT:
      return ending[0];
    case RIGHT:
      return ending[1];
  }
  ERROR_RAW(
      "Invalid Side Iron side requested. The side must be either Left or Right",
      std::cerr);
}
std::optional<Drawing::Side> Drawing::sideIronFeedEnd() const {
  return feedEnd;
}

std::optional<Drawing::HookOrientation> Drawing::sideIronHookOrientation(
    Side side) const {
  switch (side) {
    case LEFT:
      return hookOrientation[0];
    case RIGHT:
      return hookOrientation[1];
  }
  ERROR_RAW(
      "Invalid Side Iron side requested. The side must be either Left or Right",
      std::cerr);
}

void Drawing::setSideIron(Drawing::Side side, const SideIron &sideIron) {
  switch (side) {
    case LEFT:
      sideIronHandles[0] = sideIron.handle();
      break;
    case RIGHT:
      sideIronHandles[1] = sideIron.handle();
      break;
  }
  __sideIronType = sideIron.type;
  invokeUpdateCallbacks();
}

void Drawing::setSideIronInverted(Drawing::Side side, bool inverted) {
  switch (side) {
    case LEFT:
      sideIronsInverted[0] = inverted;
      break;
    case RIGHT:
      sideIronsInverted[1] = inverted;
      break;
  }
  invokeUpdateCallbacks();
}

void Drawing::setSideIronCutDown(Drawing::Side side, bool cutDown) {
  switch (side) {
    case LEFT:
      sideIronsCutDown[0] = cutDown;
      break;
    case RIGHT:
      sideIronsCutDown[1] = cutDown;
      break;
  }
  invokeUpdateCallbacks();
}

void Drawing::setSideIronEnding(Drawing::Side side, Drawing::Ending ending) {
  switch (side) {
    case LEFT:
      this->ending[0] = std::optional<Drawing::Ending>(ending);
      break;
    case RIGHT:
      this->ending[1] = std::optional<Drawing::Ending>(ending);
      break;
  }
  invokeUpdateCallbacks();
}

void Drawing::setSideIronFeed(Drawing::Side side) {
  feedEnd = std::optional<Drawing::Side>(side);
}

void Drawing::setSideIronHookOrientation(Drawing::Side side,
                                         Drawing::HookOrientation orientation) {
  switch (side) {
    case LEFT:
      hookOrientation[0] = std::optional<Drawing::HookOrientation>(orientation);
      return;
    case RIGHT:
      hookOrientation[1] = std::optional<Drawing::HookOrientation>(orientation);
      return;
  }
}

bool Drawing::hasStraps() const {
  return strapHandle[0].has_value() || strapHandle[1].has_value();
}

unsigned char Drawing::strapsCount() const {
  return strapHandle[0].has_value() + strapHandle[1].has_value();
}

Strap &Drawing::sideIronStrap(Drawing::Side side) const {
  switch (side) {
    case LEFT:
      if (strapHandle[0].has_value())
        return DrawingComponentManager<Strap>::getComponentByHandle(
            strapHandle[0].value());
    case RIGHT:
      if (strapHandle[1].has_value())
        return DrawingComponentManager<Strap>::getComponentByHandle(
            strapHandle[1].value());
  }
  Logger::logError("Tried to get non-existent strap.", __LINE__, __FILE__,
                   false);
}

bool Drawing::hasStrap(Drawing::Side side) const {
  switch (side) {
    case LEFT:
      return strapHandle[0].has_value();
    case RIGHT:
      return strapHandle[1].has_value();
  }
  Logger::logError("Tried to get non-existent strap.", __LINE__, __FILE__,
                   false);
}

void Drawing::setSideIronStrap(Drawing::Side side, const Strap &s) {
  switch (side) {
    case LEFT:
      strapHandle[0] = std::optional<unsigned>(s.handle());
      return;
    case RIGHT:
      strapHandle[1] = std::optional<unsigned>(s.handle());
      return;
  }
}

void Drawing::removeSideIronStrap(Drawing::Side side) {
  switch (side) {
    case LEFT:
      strapHandle[0] = std::nullopt;
      return;
    case RIGHT:
      strapHandle[1] = std::nullopt;
      return;
  }
}

void Drawing::removeSideIronFeed(Drawing::Side side) {
  if (feedEnd.has_value() && side == feedEnd.value()) {
    feedEnd = std::nullopt;
  }
}

void Drawing::removeSideIronEnding(Drawing::Side side) {
  switch (side) {
    case LEFT:
      ending[0] = std::nullopt;
      return;
    case RIGHT:
      ending[1] = std::nullopt;
      return;
  }
}

void Drawing::removeSideIronHookOrientation(Drawing::Side side) {
  switch (side) {
    case LEFT:
      hookOrientation[0] = std::nullopt;
      break;
    case RIGHT:
      hookOrientation[1] = std::nullopt;
      break;
  }
}

void Drawing::removeSideIron(Drawing::Side side) {
  switch (side) {
    case LEFT:
      sideIronHandles[0] =
          DrawingComponentManager<SideIron>::findComponentByID(1).handle();
      sideIronsInverted[0] = false;
      sideIronsCutDown[0] = false;

      break;
    case RIGHT:
      sideIronHandles[1] =
          DrawingComponentManager<SideIron>::findComponentByID(1).handle();
      sideIronsInverted[1] = false;
      sideIronsCutDown[1] = false;
      break;
  }
  invokeUpdateCallbacks();
}

std::optional<Drawing::Lap> Drawing::sidelap(Drawing::Side side) const {
  switch (side) {
    case LEFT:
      return sidelaps[0];
    case RIGHT:
      return sidelaps[1];
  }
  return std::nullopt;
}

void Drawing::setSidelap(Drawing::Side side, const Drawing::Lap &lap) {
  switch (side) {
    case LEFT:
      sidelaps[0] = lap;
      break;
    case RIGHT:
      sidelaps[1] = lap;
      break;
  }
  invokeUpdateCallbacks();
}

void Drawing::removeSidelap(Drawing::Side side) {
  switch (side) {
    case LEFT:
      sidelaps[0] = std::nullopt;
      break;
    case RIGHT:
      sidelaps[1] = std::nullopt;
      break;
  }
  invokeUpdateCallbacks();
}

std::optional<Drawing::Lap> Drawing::overlap(Drawing::Side side) const {
  switch (side) {
    case LEFT:
      return overlaps[0];
    case RIGHT:
      return overlaps[1];
  }
  return std::nullopt;
}

void Drawing::setOverlap(Drawing::Side side, const Drawing::Lap &lap) {
  switch (side) {
    case LEFT:
      overlaps[0] = lap;
      break;
    case RIGHT:
      overlaps[1] = lap;
      break;
  }
  invokeUpdateCallbacks();
}

void Drawing::removeOverlap(Drawing::Side side) {
  switch (side) {
    case LEFT:
      overlaps[0] = std::nullopt;
      break;
    case RIGHT:
      overlaps[1] = std::nullopt;
      break;
  }
  invokeUpdateCallbacks();
}

std::vector<std::filesystem::path> Drawing::pressDrawingHyperlinks() const {
  return __pressDrawingHyperlinks;
}

void Drawing::setPressDrawingHyperlinks(
    const std::vector<std::filesystem::path> &hyperlinks) {
  __pressDrawingHyperlinks = hyperlinks;
  invokeUpdateCallbacks();
}

bool Drawing::hasSidelaps() const {
  return sidelaps[0].has_value() || sidelaps[1].has_value();
}

bool Drawing::hasOverlaps() const {
  return overlaps[0].has_value() || overlaps[1].has_value();
}

void Drawing::addImpactPad(const ImpactPad &impactPad) {
  __impactPads.push_back(impactPad);
  invokeUpdateCallbacks();
}

std::vector<Drawing::ImpactPad> Drawing::impactPads() const {
  return __impactPads;
}

Drawing::ImpactPad &Drawing::impactPad(unsigned index) {
  return __impactPads[index];
}

const Drawing::ImpactPad &Drawing::safeImpactPad(unsigned index) const {
  return __impactPads[index];
}

void Drawing::removeImpactPad(const Drawing::ImpactPad &pad) {
  __impactPads.erase(std::find(__impactPads.begin(), __impactPads.end(), pad));
}

unsigned Drawing::numberOfImpactPads() const { return __impactPads.size(); }

void Drawing::addDamBar(const DamBar &bar) {
  __damBars.push_back(bar);
  invokeUpdateCallbacks();
}

std::vector<Drawing::DamBar> Drawing::damBars() const { return __damBars; }

Drawing::DamBar &Drawing::damBar(unsigned index) { return __damBars[index]; }

void Drawing::removeDamBar(const Drawing::DamBar &bar) {
  __damBars.erase(std::find(__damBars.begin(), __damBars.end(), bar));
}

unsigned Drawing::numberOfDamBars() const { return __damBars.size(); };

void Drawing::addBlankSpace(const BlankSpace &blankSpace) {
  __blankSpaces.push_back(blankSpace);
  invokeUpdateCallbacks();
}

std::vector<Drawing::BlankSpace> Drawing::blankSpaces() const {
  return __blankSpaces;
}

Drawing::BlankSpace &Drawing::blankSpace(unsigned index) {
  return __blankSpaces[index];
}

void Drawing::removeBlankSpace(const Drawing::BlankSpace &space) {
  __blankSpaces.erase(
      std::find(__blankSpaces.begin(), __blankSpaces.end(), space));
}

unsigned Drawing::numberOfBlankSpaces() const { return __blankSpaces.size(); }

void Drawing::addExtraAperture(const ExtraAperture &extraAperture) {
  __extraApertures.push_back(extraAperture);
  invokeUpdateCallbacks();
}

std::vector<Drawing::ExtraAperture> Drawing::extraApertures() const {
  return __extraApertures;
}

Drawing::ExtraAperture &Drawing::extraAperture(unsigned index) {
  return __extraApertures[index];
}

void Drawing::removeExtraAperture(const Drawing::ExtraAperture &aperture) {
  __extraApertures.erase(
      std::find(__extraApertures.begin(), __extraApertures.end(), aperture));
}

unsigned Drawing::numberOfExtraApertures() const {
  return __extraApertures.size();
}

void Drawing::addCentreHole(const CentreHole &centreHole) {
  __centreHoles.push_back(centreHole);
  invokeUpdateCallbacks();
}

const std::vector<Drawing::CentreHole> &Drawing::centreHoles() const {
  return __centreHoles;
}

std::vector<Drawing::CentreHole> &Drawing::centreHoles() {
  return __centreHoles;
}

Drawing::CentreHole &Drawing::centreHole(unsigned index) {
  return __centreHoles[index];
}

void Drawing::removeCentreHole(const Drawing::CentreHole &hole) {
  __centreHoles.erase(
      std::find(__centreHoles.begin(), __centreHoles.end(), hole));
}

unsigned Drawing::numberOfCentreHoles() const { return __centreHoles.size(); }

void Drawing::addDeflector(const Deflector &deflector) {
  __deflectors.push_back(deflector);
  invokeUpdateCallbacks();
}

const std::vector<Drawing::Deflector> &Drawing::deflectors() const {
  return __deflectors;
}

std::vector<Drawing::Deflector> &Drawing::deflectors() { return __deflectors; }

Drawing::Deflector &Drawing::deflector(unsigned index) {
  return __deflectors[index];
}

void Drawing::removeDeflector(const Drawing::Deflector &deflector) {
  __deflectors.erase(
      std::find(__deflectors.begin(), __deflectors.end(), deflector));
}

unsigned Drawing::numberOfDeflectors() const { return __deflectors.size(); }

void Drawing::addDivertor(const Divertor &divertor) {
  __divertors.push_back(divertor);
  invokeUpdateCallbacks();
}

std::vector<Drawing::Divertor> &Drawing::divertors() { return __divertors; }

const std::vector<Drawing::Divertor> &Drawing::divertors() const {
  return __divertors;
}

Drawing::Divertor &Drawing::divertor(unsigned index) {
  return __divertors[index];
}

void Drawing::removeDivertor(const Drawing::Divertor &divertor) {
  __divertors.erase(
      std::find(__divertors.begin(), __divertors.end(), divertor));
}

unsigned Drawing::numberOfDivertors() const { return __divertors.size(); }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCSimplifyInspection"

Drawing::BuildWarning Drawing::checkDrawingValidity(unsigned exclusions) const {
  if (!std::regex_match(__drawingNumber,
                        std::regex(drawingNumberRegexPattern)) &&
      !(exclusions & INVALID_DRAWING_NUMBER)) {
    return INVALID_DRAWING_NUMBER;
  }
  if (!DrawingComponentManager<Product>::validComponentHandle(productHandle) &&
      !(exclusions & INVALID_PRODUCT)) {
    return INVALID_PRODUCT;
  }
  if (__width <= 0 && !(exclusions & INVALID_WIDTH)) {
    return INVALID_WIDTH;
  }
  if (__length <= 0 && !(exclusions & INVALID_LENGTH)) {
    return INVALID_LENGTH;
  }
  if (!DrawingComponentManager<Material>::validComponentHandle(
          topLayerThicknessHandle) &&
      !(exclusions & INVALID_TOP_MATERIAL)) {
    return INVALID_TOP_MATERIAL;
  }
  if (bottomLayerThicknessHandle.has_value()) {
    if (!DrawingComponentManager<Material>::validComponentHandle(
            bottomLayerThicknessHandle.value()) &&
        !(exclusions & INVALID_BOTTOM_MATERIAL)) {
      return INVALID_BOTTOM_MATERIAL;
    }
  }
  if (strapHandle[0].has_value()) {
    if (!DrawingComponentManager<Strap>::validComponentHandle(
            strapHandle[0].value()) &&
        !(exclusions & INVALID_STRAPS)) {
      return INVALID_STRAPS;
    }
  }
  if (!DrawingComponentManager<Aperture>::validComponentHandle(
          apertureHandle) &&
      !(exclusions & INVALID_APERTURE)) {
    return INVALID_APERTURE;
  }
  if (std::accumulate(barSpacings.begin(), barSpacings.end(), 0.0f) !=
          __width &&
      !(exclusions & INVALID_BAR_SPACINGS)) {
    return INVALID_BAR_SPACINGS;
  }
  if (std::find(barWidths.begin(), barWidths.end(), 0.0f) != barWidths.end() &&
      !(exclusions & INVALID_BAR_WIDTHS)) {
    return INVALID_BAR_WIDTHS;
  }
  if (!DrawingComponentManager<SideIron>::validComponentHandle(
          sideIronHandles[0]) &&
      !(exclusions & INVALID_SIDE_IRONS)) {
    return INVALID_SIDE_IRONS;
  }
  if (!DrawingComponentManager<SideIron>::validComponentHandle(
          sideIronHandles[1]) &&
      !(exclusions & INVALID_SIDE_IRONS)) {
    return INVALID_SIDE_IRONS;
  }
  if (!DrawingComponentManager<Machine>::validComponentHandle(
          __machineTemplate.machineHandle) &&
      !(exclusions & INVALID_MACHINE)) {
    return INVALID_MACHINE;
  }
  if (!std::regex_match(__machineTemplate.position,
                        std::regex(positionRegexPattern)) &&
      !(exclusions & INVALID_MACHINE_POSITION)) {
    return INVALID_MACHINE_POSITION;
  }
  if (!DrawingComponentManager<MachineDeck>::validComponentHandle(
          __machineTemplate.deckHandle) &&
      !(exclusions & INVALID_MACHINE_DECK)) {
    return INVALID_MACHINE_DECK;
  }
  if (__hyperlink.empty() && !(exclusions & INVALID_HYPERLINK)) {
    return INVALID_HYPERLINK;
  }

  return SUCCESS;
}

#pragma clang diagnostic pop

void Drawing::setLoadWarning(Drawing::LoadWarning warning) {
  loadWarnings |= warning;
}

bool Drawing::loadWarning(Drawing::LoadWarning warning) const {
  return loadWarnings & warning;
}

void Drawing::addUpdateCallback(const std::function<void()> &callback) {
  updateCallbacks.push_back(callback);
}

void Drawing::invokeUpdateCallbacks() const {
  for (const std::function<void()> &callback : updateCallbacks) {
    if (callback) {
      callback();
    }
  }
}

void DrawingSerialiser::serialise(const Drawing &drawing, void *target) {
  unsigned char *buffer = (unsigned char *)target;

  // Drawing Number
  unsigned char drawingNumberSize = drawing.__drawingNumber.size();
  *buffer++ = drawingNumberSize;
  memcpy(buffer, drawing.__drawingNumber.c_str(), drawingNumberSize);
  buffer += drawingNumberSize;

  // Date
  memcpy(buffer, &drawing.__date, sizeof(Date));
  buffer += sizeof(Date);

  // Width
  *((float *)buffer) = drawing.__width;
  buffer += sizeof(float);

  // Length
  *((float *)buffer) = drawing.__length;
  buffer += sizeof(float);

  // Hyperlink
  unsigned char hyperlinkSize = drawing.__hyperlink.generic_string().size();
  *buffer++ = hyperlinkSize;
  memcpy(buffer, drawing.__hyperlink.generic_string().c_str(), hyperlinkSize);
  buffer += hyperlinkSize;

  // Notes
  unsigned char notesSize = drawing.__notes.size();
  *buffer++ = notesSize;
  memcpy(buffer, drawing.__notes.c_str(), notesSize);
  buffer += notesSize;

  // Machine Template: Machine ID, Quantity on Deck, Position string (<256
  // chars), Deck ID
  *((unsigned *)buffer) = drawing.__machineTemplate.machine().handle();
  buffer += sizeof(unsigned);
  *((unsigned *)buffer) = drawing.__machineTemplate.quantityOnDeck;
  buffer += sizeof(unsigned);
  unsigned char machinePositionSize = drawing.__machineTemplate.position.size();
  *buffer++ = machinePositionSize;
  memcpy(buffer, drawing.__machineTemplate.position.c_str(),
         machinePositionSize);
  buffer += machinePositionSize;
  *((unsigned *)buffer) = drawing.__machineTemplate.deck().handle();
  buffer += sizeof(unsigned);

  // Product ID
  *((unsigned *)buffer) = drawing.productHandle;
  buffer += sizeof(unsigned);

  // Aperture ID
  *((unsigned *)buffer) = drawing.apertureHandle;
  buffer += sizeof(unsigned);

  // Backing Strip ID, which is optional, so must have a bool to identify its
  // presence
  *buffer++ = drawing.hasBackingStrips();
  if (drawing.hasBackingStrips()) {
    *((unsigned *)buffer) = drawing.backingStripHandle.value();
  }
  buffer += sizeof(unsigned);

  // Tension Type
  *buffer++ = (unsigned char)drawing.__tensionType;

  // Rebated
  *buffer++ = drawing.__rebated;

  // Press Drawing Links
  *buffer++ = drawing.__pressDrawingHyperlinks.size();
  for (const std::filesystem::path &pdl : drawing.__pressDrawingHyperlinks) {
    unsigned char pdlSize = pdl.generic_string().size();
    *buffer++ = pdlSize;
    memcpy(buffer, pdl.generic_string().c_str(), pdlSize);
    buffer += pdlSize;
  }

  // Bar spacings
  *buffer++ = drawing.barSpacings.size();
  for (float b : drawing.barSpacings) {
    *((float *)buffer) = b;
    buffer += sizeof(float);
  }

  // Bar widths
  *buffer++ = drawing.barWidths.size();
  for (float b : drawing.barWidths) {
    *((float *)buffer) = b;
    buffer += sizeof(float);
  }

  // Side Irons
  *((unsigned *)buffer) = drawing.sideIronHandles[0];
  buffer += sizeof(unsigned);
  *buffer++ = drawing.sideIronsInverted[0];
  *buffer++ = drawing.sideIronsCutDown[0];
  *((unsigned *)buffer) = drawing.sideIronHandles[1];
  buffer += sizeof(unsigned);
  *buffer++ = drawing.sideIronsInverted[1];
  *buffer++ = drawing.sideIronsCutDown[1];

  *buffer++ = drawing.feedEnd.has_value();
  if (drawing.feedEnd.has_value()) {
    *((Drawing::Side *)buffer) = drawing.feedEnd.value();
    buffer += sizeof(Drawing::Side);
  }
  *buffer++ = drawing.ending[0].has_value();
  if (drawing.ending[0].has_value()) {
    *((Drawing::Ending *)buffer) = drawing.ending[0].value();
    buffer += sizeof(Drawing::Side);
  }
  *buffer++ = drawing.ending[1].has_value();
  if (drawing.ending[1].has_value()) {
    *((Drawing::Ending *)buffer) = drawing.ending[1].value();
    buffer += sizeof(Drawing::Side);
  }
  *buffer++ = drawing.hookOrientation[0].has_value();
  if (drawing.hookOrientation[0].has_value()) {
    *((Drawing::HookOrientation *)buffer) = drawing.hookOrientation[0].value();
    buffer += sizeof(Drawing::HookOrientation);
  }
  *buffer++ = drawing.hookOrientation[1].has_value();
  if (drawing.hookOrientation[1].has_value()) {
    *((Drawing::HookOrientation *)buffer) = drawing.hookOrientation[1].value();
    buffer += sizeof(Drawing::HookOrientation);
  }
  *buffer++ = drawing.strapHandle[0].has_value();
  if (drawing.strapHandle[0].has_value()) {
    *((unsigned *)buffer) = drawing.strapHandle[0].value();
    buffer += sizeof(unsigned);
  }
  *buffer++ = drawing.strapHandle[1].has_value();
  if (drawing.strapHandle[1].has_value()) {
    *((unsigned *)buffer) = drawing.strapHandle[1].value();
    buffer += sizeof(unsigned);
  }

  // A byte for flags: UNUSED, UNUSED, UNUSED, HAS_BOTTOM_LAYER, OL_R, OL_L,
  // SL_R, SL_L
  enum Flags {
    SIDELAP_L = 0x01,
    SIDELAP_R = 0x02,
    OVERLAP_L = 0x04,
    OVERLAP_R = 0x08,
    HAS_BOTTOM_LAYER = 0x10
  };

  *buffer = 0x00;

  if (drawing.sidelaps[0].has_value()) {
    *buffer |= SIDELAP_L;
  }
  if (drawing.sidelaps[1].has_value()) {
    *buffer |= SIDELAP_R;
  }
  if (drawing.overlaps[0].has_value()) {
    *buffer |= OVERLAP_L;
  }
  if (drawing.overlaps[1].has_value()) {
    *buffer |= OVERLAP_R;
  }
  if (drawing.bottomLayerThicknessHandle.has_value()) {
    *buffer |= HAS_BOTTOM_LAYER;
  }

  buffer++;

  // Lap: Attachment Type, Width, Material ID
  // Left sidelap
  if (drawing.sidelaps[0].has_value()) {
    *buffer++ = (unsigned char)drawing.sidelaps[0]->attachmentType;
    *((float *)buffer) = drawing.sidelaps[0]->width;
    buffer += sizeof(float);
    *((unsigned *)buffer) = drawing.sidelaps[0]->material().handle();
    buffer += sizeof(unsigned);
  }
  // Right sidelap
  if (drawing.sidelaps[1].has_value()) {
    *buffer++ = (unsigned char)drawing.sidelaps[1]->attachmentType;
    *((float *)buffer) = drawing.sidelaps[1]->width;
    buffer += sizeof(float);
    *((unsigned *)buffer) = drawing.sidelaps[1]->material().handle();
    buffer += sizeof(unsigned);
  }
  // Left overlap
  if (drawing.overlaps[0].has_value()) {
    *buffer++ = (unsigned char)drawing.overlaps[0]->attachmentType;
    *((float *)buffer) = drawing.overlaps[0]->width;
    buffer += sizeof(float);
    *((unsigned *)buffer) = drawing.overlaps[0]->material().handle();
    buffer += sizeof(unsigned);
  }
  // Right overlap
  if (drawing.overlaps[1].has_value()) {
    *buffer++ = (unsigned char)drawing.overlaps[1]->attachmentType;
    *((float *)buffer) = drawing.overlaps[1]->width;
    buffer += sizeof(float);
    *((unsigned *)buffer) = drawing.overlaps[1]->material().handle();
    buffer += sizeof(unsigned);
  }

  // Top layer material ID
  *((unsigned *)buffer) = drawing.topLayerThicknessHandle;
  buffer += sizeof(unsigned);

  // Bottom layer material ID
  if (drawing.bottomLayerThicknessHandle.has_value()) {
    *((unsigned *)buffer) = drawing.bottomLayerThicknessHandle.value();
    buffer += sizeof(unsigned);
  }

  // Impact Pads
  *buffer++ = drawing.__impactPads.size();
  for (const Drawing::ImpactPad &pad : drawing.__impactPads) {
    pad.serialise(buffer);
    buffer += pad.serialisedSize();
  }

  // Dam Bars
  *buffer++ = drawing.__damBars.size();
  for (const Drawing::DamBar &bar : drawing.__damBars) {
    bar.serialise(buffer);
    buffer += bar.serialisedSize();
  }

  // Blank Spaces
  *buffer++ = drawing.__blankSpaces.size();
  for (const Drawing::BlankSpace &pad : drawing.__blankSpaces) {
    pad.serialise(buffer);
    buffer += pad.serialisedSize();
  }

  // Extra Apertures
  *buffer++ = drawing.__extraApertures.size();
  for (const Drawing::ExtraAperture &aperture : drawing.__extraApertures) {
    aperture.serialise(buffer);
    buffer += aperture.serialisedSize();
  }

  // Centre Holes
  *buffer++ = drawing.__centreHoles.size();
  for (const Drawing::CentreHole &hole : drawing.__centreHoles) {
    hole.serialise(buffer);
    buffer += hole.serialisedSize();
  }

  // Deflectors
  *buffer++ = drawing.__deflectors.size();
  for (const Drawing::Deflector &deflector : drawing.__deflectors) {
    deflector.serialise(buffer);
    buffer += deflector.serialisedSize();
  }

  // Divertors
  *buffer++ = drawing.__divertors.size();
  for (const Drawing::Divertor &divertor : drawing.__divertors) {
    divertor.serialise(buffer);
    buffer += divertor.serialisedSize();
  }

  // Load warnings
  *((unsigned *)buffer) = drawing.loadWarnings;
  buffer += sizeof(unsigned);
}

unsigned DrawingSerialiser::serialisedSize(const Drawing &drawing) {
  unsigned size = 0;
  // Drawing Number
  size += sizeof(unsigned char) + drawing.__drawingNumber.size();
  // Date
  size += sizeof(Date);
  // Width
  size += sizeof(float);
  // Length
  size += sizeof(float);
  // Hyperlink
  size += sizeof(unsigned char) + drawing.__hyperlink.generic_string().size();
  // Notes
  size += sizeof(unsigned char) + drawing.__notes.size();
  // Machine Template: Machine ID, Quantity on Deck, Position string (<256
  // chars), Deck ID
  size += sizeof(unsigned) + sizeof(unsigned) + sizeof(unsigned char) +
          drawing.__machineTemplate.position.size() + sizeof(unsigned);
  // Product ID
  size += sizeof(unsigned);
  // Aperture ID
  size += sizeof(unsigned);
  // Backing Strip
  size += sizeof(bool) + sizeof(unsigned);
  // Tension Type
  size += sizeof(unsigned char);
  // Rebated
  size += sizeof(bool);
  // Backing Strips
  size += sizeof(bool);
  // Press Drawing Links
  size += sizeof(unsigned char) +
          std::accumulate(drawing.__pressDrawingHyperlinks.begin(),
                          drawing.__pressDrawingHyperlinks.end(), 0,
                          [](unsigned t, const std::filesystem::path &s) {
                            return sizeof(unsigned char) +
                                   s.generic_string().size() + t;
                          });
  // Bar spacings
  size += sizeof(unsigned char) + drawing.barSpacings.size() * sizeof(float);
  // Bar widths
  size += sizeof(unsigned char) + drawing.barWidths.size() * sizeof(float);
  // Side Irons
  size += 2 * (sizeof(unsigned) + sizeof(unsigned char));

  size += 5;
  if (drawing.ending[0].has_value()) {
    size += sizeof(Drawing::Ending);
  }
  if (drawing.ending[1].has_value()) {
    size += sizeof(Drawing::Ending);
  }
  if (drawing.feedEnd.has_value()) {
    size += sizeof(Drawing::Side);
  }
  if (drawing.hookOrientation[0].has_value()) {
    size += sizeof(Drawing::HookOrientation);
  }
  if (drawing.hookOrientation[1].has_value()) {
    size += sizeof(Drawing::HookOrientation);
  }
  size += sizeof(bool) * 2;
  if (drawing.strapHandle[0].has_value()) {
    size += sizeof(unsigned);
  }
  if (drawing.strapHandle[1].has_value()) {
    size += sizeof(unsigned);
  }
  // A byte for flags: UNUSED, UNUSED, UNUSED, HAS_BOTTOM_LAYER, OL_R, OL_L,
  // SL_R, SL_L
  size += sizeof(unsigned char);
  // Lap: Attachment Type, Width, Material ID
  // Left sidelap
  if (drawing.sidelaps[0].has_value()) {
    size += sizeof(unsigned char) + sizeof(float) + sizeof(unsigned);
  }
  // Right sidelap
  if (drawing.sidelaps[1].has_value()) {
    size += sizeof(unsigned char) + sizeof(float) + sizeof(unsigned);
  }
  // Left overlap
  if (drawing.overlaps[0].has_value()) {
    size += sizeof(unsigned char) + sizeof(float) + sizeof(unsigned);
  }
  // Right overlap
  if (drawing.overlaps[1].has_value()) {
    size += sizeof(unsigned char) + sizeof(float) + sizeof(unsigned);
  }
  // Top layer material ID
  size += sizeof(unsigned);
  // Bottom layer material ID
  if (drawing.bottomLayerThicknessHandle.has_value()) {
    size += sizeof(unsigned);
  }
  // Impact Pads
  size +=
      sizeof(unsigned char) +
      std::accumulate(drawing.__impactPads.begin(), drawing.__impactPads.end(),
                      0, [](unsigned size, const Drawing::ImpactPad &pad) {
                        return size + pad.serialisedSize();
                      });
  // Dam Bars
  size += sizeof(unsigned char) +
          std::accumulate(drawing.__damBars.begin(), drawing.__damBars.end(), 0,
                          [](unsigned size, const Drawing::DamBar &bar) {
                            return size + bar.serialisedSize();
                          });
  // Blank Spaces
  size += sizeof(unsigned char) +
          std::accumulate(drawing.__blankSpaces.begin(),
                          drawing.__blankSpaces.end(), 0,
                          [](unsigned size, const Drawing::BlankSpace &space) {
                            return size + space.serialisedSize();
                          });
  // Extra Apertures
  size += sizeof(unsigned char) +
          std::accumulate(
              drawing.__extraApertures.begin(), drawing.__extraApertures.end(),
              0, [](unsigned size, const Drawing::ExtraAperture &aperture) {
                return size + aperture.serialisedSize();
              });
  // Centre Holes
  size += sizeof(unsigned char) +
          std::accumulate(drawing.__centreHoles.begin(),
                          drawing.__centreHoles.end(), 0,
                          [](unsigned size, const Drawing::CentreHole &hole) {
                            return size + hole.serialisedSize();
                          });
  // Deflectors
  size += sizeof(unsigned char) +
          std::accumulate(
              drawing.__deflectors.begin(), drawing.__deflectors.end(), 0,
              [](unsigned size, const Drawing::Deflector &deflector) {
                return size + deflector.serialisedSize();
              });
  // Divertors
  size +=
      sizeof(unsigned char) +
      std::accumulate(drawing.__divertors.begin(), drawing.__divertors.end(), 0,
                      [](unsigned size, const Drawing::Divertor &divertor) {
                        return size + divertor.serialisedSize();
                      });
  // Load Warnings
  size += sizeof(unsigned);

  return size;
}

Drawing &DrawingSerialiser::deserialise(void *data) {
  Drawing *drawing = new Drawing();

  unsigned char *buffer = (unsigned char *)data;

  // Drawing Number
  unsigned char drawingNumberSize = *buffer++;
  drawing->__drawingNumber =
      std::string((const char *)buffer, drawingNumberSize);
  buffer += drawingNumberSize;

  // Date
  memcpy(&drawing->__date, buffer, sizeof(Date));
  buffer += sizeof(Date);

  // Width
  drawing->__width = *((float *)buffer);
  buffer += sizeof(float);

  // Length
  drawing->__length = *((float *)buffer);
  buffer += sizeof(float);

  // Hyperlink
  unsigned char hyperlinkSize = *buffer++;
  drawing->__hyperlink = std::string((const char *)buffer, hyperlinkSize);
  buffer += hyperlinkSize;

  // Notes
  unsigned char notesSize = *buffer++;
  drawing->__notes = std::string((const char *)buffer, notesSize);
  buffer += notesSize;

  // Machine Template: Machine ID, Quantity on Deck, Position string (<256
  // chars), Deck ID
  unsigned machineHandle = *((unsigned *)buffer);
  buffer += sizeof(unsigned);
  unsigned quantityOnDeck = *((unsigned *)buffer);
  buffer += sizeof(unsigned);
  unsigned char machinePositionSize = *buffer++;
  std::string position = std::string((const char *)buffer, machinePositionSize);
  buffer += machinePositionSize;
  unsigned deckHandle = *((unsigned *)buffer);
  buffer += sizeof(unsigned);

  drawing->setMachineTemplate(
      DrawingComponentManager<Machine>::getComponentByHandle(machineHandle),
      quantityOnDeck, position,
      DrawingComponentManager<MachineDeck>::getComponentByHandle(deckHandle));

  // Product ID
  drawing->productHandle = *((unsigned *)buffer);
  buffer += sizeof(unsigned);

  // Aperture ID
  drawing->apertureHandle = *((unsigned *)buffer);
  buffer += sizeof(unsigned);

  // Backing Strip ID
  bool backingStripExists = *buffer++;
  if (backingStripExists) {
    drawing->backingStripHandle = *((unsigned *)buffer);
  } else {
    drawing->backingStripHandle = std::nullopt;
  }
  buffer += sizeof(unsigned);

  // Tension Type
  drawing->__tensionType = (Drawing::TensionType)*buffer++;

  // Rebated
  drawing->__rebated = *buffer++;

  // Press Drawing Links
  unsigned char noPressDrawings = *buffer++;
  for (unsigned char pdl = 0; pdl < noPressDrawings; pdl++) {
    unsigned char pdlSize = *buffer++;
    drawing->__pressDrawingHyperlinks.emplace_back(
        std::string((const char *)buffer, pdlSize));
    buffer += pdlSize;
  }

  // Bar spacings
  unsigned char noBarSpacings = *buffer++;
  for (unsigned char b = 0; b < noBarSpacings; b++) {
    drawing->barSpacings.push_back(*((float *)buffer));
    buffer += sizeof(float);
  }

  // Bar widths
  unsigned char noBarWidths = *buffer++;
  for (unsigned char b = 0; b < noBarWidths; b++) {
    drawing->barWidths.push_back(*((float *)buffer));
    buffer += sizeof(float);
  }

  // Side Irons
  drawing->sideIronHandles[0] = *((unsigned *)buffer);
  buffer += sizeof(unsigned);
  drawing->sideIronsInverted[0] = *buffer++;
  drawing->sideIronsCutDown[0] = *buffer++;

  drawing->sideIronHandles[1] = *((unsigned *)buffer);
  buffer += sizeof(unsigned);
  drawing->sideIronsInverted[1] = *buffer++;
  drawing->sideIronsCutDown[1] = *buffer++;
  /// feed, ending, hook

  bool hasFeedEnd = *buffer++;
  if (hasFeedEnd) {
    drawing->feedEnd = *((Drawing::Side *)buffer);
    buffer += sizeof(Drawing::Side);
  }
  bool hasLeftEnding = *buffer++;
  if (hasLeftEnding) {
    drawing->ending[0] = *((Drawing::Ending *)buffer);
    buffer += sizeof(Drawing::Ending);
  }
  bool hasRightEnding = *buffer++;
  if (hasRightEnding) {
    drawing->ending[1] = *((Drawing::Ending *)buffer);
    buffer += sizeof(Drawing::Ending);
    ;
  }
  bool hasLeftHookOrientation = *buffer++;
  if (hasLeftHookOrientation) {
    drawing->hookOrientation[0] = *((Drawing::HookOrientation *)buffer);
    buffer += sizeof(Drawing::HookOrientation);
  }
  bool hasRightHookOrientation = *buffer++;
  if (hasRightHookOrientation) {
    drawing->hookOrientation[1] = *((Drawing::HookOrientation *)buffer);
    buffer += sizeof(Drawing::HookOrientation);
  }
  bool hasLeftStrap = *buffer++;
  if (hasLeftStrap) {
    drawing->strapHandle[0] = *((unsigned *)buffer);
    buffer += sizeof(unsigned);
  }
  bool hasRightStrap = *buffer++;
  if (hasRightStrap) {
    drawing->strapHandle[1] = *((unsigned *)buffer);
    buffer += sizeof(unsigned);
  }

  // A byte for flags: UNUSED, UNUSED, UNUSED, HAS_BOTTOM_LAYER, OL_R, OL_L,
  // SL_R, SL_L
  enum Flags {
    SIDELAP_L = 0x01,
    SIDELAP_R = 0x02,
    OVERLAP_L = 0x04,
    OVERLAP_R = 0x08,
    HAS_BOTTOM_LAYER = 0x10
  };

  unsigned char flags = *buffer++;

  // Lap: Attachment Type, Width, Material ID
  // Left sidelap
  if (flags & SIDELAP_L) {
    LapAttachment attachment = (LapAttachment)*buffer++;
    float width = *((float *)buffer);
    buffer += sizeof(float);
    unsigned materialHandle = *((unsigned *)buffer);
    buffer += sizeof(unsigned);

    Drawing::Lap lap(width, attachment,
                     DrawingComponentManager<Material>::getComponentByHandle(
                         materialHandle));
    drawing->sidelaps[0] = lap;
  } else {
    drawing->sidelaps[0] = std::nullopt;
  }
  // Right sidelap
  if (flags & SIDELAP_R) {
    LapAttachment attachment = (LapAttachment)*buffer++;
    float width = *((float *)buffer);
    buffer += sizeof(float);
    unsigned materialHandle = *((unsigned *)buffer);
    buffer += sizeof(unsigned);

    Drawing::Lap lap(width, attachment,
                     DrawingComponentManager<Material>::getComponentByHandle(
                         materialHandle));
    drawing->sidelaps[1] = lap;
  } else {
    drawing->sidelaps[1] = std::nullopt;
  }
  // Left overlap
  if (flags & OVERLAP_L) {
    LapAttachment attachment = (LapAttachment)*buffer++;
    float width = *((float *)buffer);
    buffer += sizeof(float);
    unsigned materialHandle = *((unsigned *)buffer);
    buffer += sizeof(unsigned);

    Drawing::Lap lap(width, attachment,
                     DrawingComponentManager<Material>::getComponentByHandle(
                         materialHandle));
    drawing->overlaps[0] = lap;
  } else {
    drawing->overlaps[0] = std::nullopt;
  }
  // Right overlap
  if (flags & OVERLAP_R) {
    LapAttachment attachment = (LapAttachment)*buffer++;
    float width = *((float *)buffer);
    buffer += sizeof(float);
    unsigned materialHandle = *((unsigned *)buffer);
    buffer += sizeof(unsigned);

    Drawing::Lap lap(width, attachment,
                     DrawingComponentManager<Material>::getComponentByHandle(
                         materialHandle));
    drawing->overlaps[1] = lap;
  } else {
    drawing->overlaps[1] = std::nullopt;
  }

  // Top layer material ID
  drawing->topLayerThicknessHandle = *((unsigned *)buffer);
  buffer += sizeof(unsigned);

  // Bottom layer material ID
  if (flags & HAS_BOTTOM_LAYER) {
    drawing->bottomLayerThicknessHandle = *((unsigned *)buffer);
    buffer += sizeof(unsigned);
  } else {
    drawing->bottomLayerThicknessHandle = std::nullopt;
  }

  // Impact Pads
  unsigned char impactPadCount = *buffer++;
  for (unsigned i = 0; i < impactPadCount; i++) {
    Drawing::ImpactPad &pad = Drawing::ImpactPad::deserialise(buffer);
    buffer += pad.serialisedSize();
    drawing->__impactPads.push_back(pad);
  }

  // Dam Bars
  unsigned char damBarCount = *buffer++;
  for (unsigned i = 0; i < damBarCount; i++) {
    Drawing::DamBar &bar = Drawing::DamBar::deserialise(buffer);
    buffer += bar.serialisedSize();
    drawing->__damBars.push_back(bar);
  }

  // Blank Spaces
  unsigned char blankSpaceCount = *buffer++;
  for (unsigned i = 0; i < blankSpaceCount; i++) {
    Drawing::BlankSpace &space = Drawing::BlankSpace::deserialise(buffer);
    buffer += space.serialisedSize();
    drawing->__blankSpaces.push_back(space);
  }

  // Extra Apertures
  unsigned char extraApertureCount = *buffer++;
  for (unsigned i = 0; i < extraApertureCount; i++) {
    Drawing::ExtraAperture &aperture =
        Drawing::ExtraAperture::deserialise(buffer);
    buffer += aperture.serialisedSize();
    drawing->__extraApertures.push_back(aperture);
  }

  // Centre Holes
  unsigned char centreHoleCount = *buffer++;
  for (unsigned i = 0; i < centreHoleCount; i++) {
    Drawing::CentreHole &hole = Drawing::CentreHole::deserialise(buffer);
    buffer += hole.serialisedSize();
    drawing->__centreHoles.push_back(hole);
  }

  // Deflectors
  unsigned char deflectorCount = *buffer++;
  for (unsigned i = 0; i < deflectorCount; i++) {
    Drawing::Deflector &deflector = Drawing::Deflector::deserialise(buffer);
    buffer += deflector.serialisedSize();
    drawing->__deflectors.push_back(deflector);
  }

  // Divertors
  unsigned char divertorCount = *buffer++;
  for (unsigned i = 0; i < divertorCount; i++) {
    Drawing::Divertor &divertor = Drawing::Divertor::deserialise(buffer);
    buffer += divertor.serialisedSize();
    drawing->__divertors.push_back(divertor);
  }

  // Load Warnings
  drawing->loadWarnings = *((unsigned *)buffer);
  buffer += sizeof(unsigned);

  return *drawing;
}

bool DrawingSummary::hasTwoLayers() const { return thicknessHandles[1] != 0; }

unsigned DrawingSummary::numberOfLaps() const {
  if (__lapSizes[3] != 0) {
    return 4;
  } else if (__lapSizes[2] != 0) {
    return 3;
  } else if (__lapSizes[1] != 0) {
    return 2;
  } else if (__lapSizes[0] != 0) {
    return 1;
  }
  return 0;
  // return (__lapSizes[0] != 0) + (__lapSizes[1] != 0) + (__lapSizes[2] != 0)
  // + (__lapSizes[3] != 0);
}

std::string DrawingSummary::summaryString() const {
  std::stringstream s;

  if (__lapSizes[0] != 0) {
    s << lapSize(0) << "+";
  }
  s << width();
  if (__lapSizes[1] != 0) {
    s << "+" << lapSize(1);
  }
  s << " x ";
  if (__lapSizes[2] != 0) {
    s << lapSize(2) << "+";
  }
  s << length();
  if (__lapSizes[3] != 0) {
    s << "+" << lapSize(3);
  }
  s << " x "
    << DrawingComponentManager<Material>::getComponentByHandle(
           thicknessHandles[0])
           .thickness;
  if (thicknessHandles[1] != 0) {
    s << "+"
      << DrawingComponentManager<Material>::getComponentByHandle(
             thicknessHandles[1])
             .thickness;
  }
  s << " x "
    << DrawingComponentManager<Aperture>::getComponentByHandle(apertureHandle)
           .apertureName();
  for (unsigned i : __extraApertures) {
    s << " / "
      << DrawingComponentManager<Aperture>::getComponentByHandle(i)
             .apertureName();
  }

  return s.str();
}

float DrawingSummary::width() const { return ((float)__width) / 2; }

float DrawingSummary::length() const { return ((float)__length) / 2; }

void DrawingSummary::setWidth(float width) { __width = (unsigned)(width * 2); }

void DrawingSummary::setLength(float length) {
  __length = (unsigned)(length * 2);
}

float DrawingSummary::lapSize(unsigned index) const {
  if (index < 0 || index > 3) {
    ERROR_RAW("Invalid lap index.", std::cerr)
  }
  return ((float)__lapSizes[index]) / 2;
}

void DrawingSummary::setLapSize(unsigned index, float size) {
  if (index < 0 || index > 3) {
    ERROR_RAW("Invalid lap index.", std::cerr)
  }
  __lapSizes[index] = (unsigned)(size * 2);
}

std::vector<float> DrawingSummary::barSpacings() const {
  std::vector<float> spacings;

  for (unsigned spacing : __barSpacings) {
    spacings.push_back((float)spacing / 2.0f);
  }

  spacings.push_back(width() -
                     std::accumulate(spacings.begin(), spacings.end(), 0.0f));

  return spacings;
}

void DrawingSummary::addSpacing(float spacing) {
  __barSpacings.push_back((unsigned)(spacing * 2));
}

void DrawingSummary::clearSpacings() { __barSpacings.clear(); }

unsigned DrawingSummary::barSpacingCount() const {
  return __barSpacings.size() + 1;
}

void DrawingSummary::addExtraAperture(unsigned apertureHandle) {
  __extraApertures.push_back(apertureHandle);
}

std::vector<unsigned> DrawingSummary::extraApertures() const {
  return __extraApertures;
}

void DrawingSummary::clearExtraApertures() { __extraApertures.clear(); }

unsigned DrawingSummary::extraApertureCount() const {
  return __extraApertures.size();
}

DrawingSummaryCompressionSchema::DrawingSummaryCompressionSchema(
    unsigned int maxMatID, float maxWidth, float maxLength,
    unsigned int maxThicknessHandle, float maxLapSize,
    unsigned int maxApertureHandle, unsigned char maxBarSpacingCount,
    float maxBarSpacing, unsigned char maxDrawingLength,
    unsigned char maxExtraApertureCount) {
  this->matIDSize = MIN_COVERING_BITS(maxMatID);
  this->widthSize = MIN_COVERING_BITS((unsigned)(maxWidth * 2));
  this->lengthSize = MIN_COVERING_BITS((unsigned)(maxLength * 2));
  this->thicknessHandleSize = MIN_COVERING_BITS(maxThicknessHandle);
  this->lapSize = MIN_COVERING_BITS((unsigned)(maxLapSize * 2));
  this->apertureHandleSize = MIN_COVERING_BITS(maxApertureHandle);
  this->barSpacingCountSize = MIN_COVERING_BITS(maxBarSpacingCount);
  this->barSpacingSize = MIN_COVERING_BITS((unsigned)(maxBarSpacing * 2));
  this->extraApertureCountSize = MIN_COVERING_BITS(maxExtraApertureCount);

  this->maxDrawingLength = maxDrawingLength;
  this->maxBarSpacingCount = maxBarSpacingCount;
  this->maxExtraApertureCount = maxExtraApertureCount;

  matIDBytes = MIN_COVERING_BYTES(matIDSize);
  widthBytes = MIN_COVERING_BYTES(widthSize);
  lengthBytes = MIN_COVERING_BYTES(lengthSize);
  thicknessHandleBytes = MIN_COVERING_BYTES(thicknessHandleSize);
  lapBytes = MIN_COVERING_BYTES(lapSize);
  apertureHandleBytes = MIN_COVERING_BYTES(apertureHandleSize);
  barSpacingCountBytes = MIN_COVERING_BYTES(barSpacingCountSize);
  barSpacingBytes = MIN_COVERING_BYTES(barSpacingSize);
  extraApertureCountBytes = MIN_COVERING_BYTES(extraApertureCountSize);
}

unsigned DrawingSummaryCompressionSchema::compressedSize(
    const DrawingSummary &summary) const {
  return sizeof(unsigned char) + summary.drawingNumber.size() +
         MIN_COVERING_BYTES(
             matIDSize + widthSize + lengthSize + apertureHandleSize +
             thicknessHandleSize + 1 +
             (summary.hasTwoLayers() ? thicknessHandleSize : 0) + 3 +
             summary.numberOfLaps() * lapSize + barSpacingCountSize +
             (summary.barSpacingCount() - 1) * barSpacingSize +
             extraApertureCountSize +
             (summary.extraApertureCount() * apertureHandleSize));
}

void DrawingSummaryCompressionSchema::compressSummary(
    const DrawingSummary &summary, void *target) const {
  unsigned char *buff = (unsigned char *)target;
  *buff++ = summary.drawingNumber.size();
  memcpy(buff, summary.drawingNumber.c_str(), summary.drawingNumber.size());
  buff += summary.drawingNumber.size();

  unsigned offset = 0;
  writeAtBitOffset((void *)&summary.matID, matIDBytes, buff, offset);
  offset += matIDSize;
  writeAtBitOffset((void *)&summary.__width, widthBytes, buff, offset);
  offset += widthSize;
  writeAtBitOffset((void *)&summary.__length, lengthBytes, buff, offset);
  offset += lengthSize;
  writeAtBitOffset((void *)&summary.apertureHandle, apertureHandleBytes, buff,
                   offset);
  offset += apertureHandleSize;
  writeAtBitOffset((void *)&summary.thicknessHandles[0], thicknessHandleBytes,
                   buff, offset);
  offset += thicknessHandleSize;
  bool hasTwoLayers = summary.hasTwoLayers();
  writeAtBitOffset(&hasTwoLayers, 1, buff, offset);
  offset += 1;
  if (hasTwoLayers) {
    writeAtBitOffset((void *)&summary.thicknessHandles[1], thicknessHandleBytes,
                     buff, offset);
    offset += thicknessHandleSize;
  }
  unsigned noOfLaps = summary.numberOfLaps();
  writeAtBitOffset(&noOfLaps, 1, buff, offset);
  offset += 3;
  for (unsigned i = 0; i < noOfLaps; i++) {
    writeAtBitOffset((void *)&summary.__lapSizes[i], lapBytes, buff, offset);
    offset += lapSize;
  }
  unsigned noOfBarSpacings = summary.barSpacingCount() - 1;
  writeAtBitOffset(&noOfBarSpacings, barSpacingCountBytes, buff, offset);
  offset += barSpacingCountSize;
  for (unsigned i = 0; i < noOfBarSpacings; i++) {
    writeAtBitOffset((void *)&summary.__barSpacings[i], barSpacingBytes, buff,
                     offset);
    offset += barSpacingSize;
  }
  unsigned noOfExtraApertures = summary.extraApertureCount();
  writeAtBitOffset(&noOfExtraApertures, extraApertureCountBytes, buff, offset);
  offset += extraApertureCountSize;
  for (unsigned i = 0; i < noOfExtraApertures; i++) {
    writeAtBitOffset((void *)&summary.__extraApertures[i], apertureHandleBytes,
                     buff, offset);
    offset += apertureHandleSize;
  }
}

DrawingSummary DrawingSummaryCompressionSchema::uncompressSummary(
    void *data, unsigned &size) const {
  DrawingSummary summary{};

  unsigned char *buff = (unsigned char *)data;
  unsigned char drawingNumberSize = *buff++;

  summary.drawingNumber = std::string((const char *)buff, drawingNumberSize);
  buff += drawingNumberSize;

  unsigned offset = 0;
  readFromBitOffset(buff, offset, &summary.matID, matIDSize);
  offset += matIDSize;
  readFromBitOffset(buff, offset, &summary.__width, widthSize);
  offset += widthSize;
  readFromBitOffset(buff, offset, &summary.__length, lengthSize);
  offset += lengthSize;
  readFromBitOffset(buff, offset, &summary.apertureHandle, apertureHandleSize);
  offset += apertureHandleSize;
  readFromBitOffset(buff, offset, &summary.thicknessHandles[0],
                    thicknessHandleSize);
  offset += thicknessHandleSize;
  bool hasTwoLayers;
  readFromBitOffset(buff, offset, &hasTwoLayers, 1);
  offset += 1;
  if (hasTwoLayers) {
    readFromBitOffset(buff, offset, &summary.thicknessHandles[1],
                      thicknessHandleSize);
    offset += thicknessHandleSize;
  }
  unsigned noOfLaps = 0;
  readFromBitOffset(buff, offset, &noOfLaps, 3);
  offset += 3;
  for (unsigned i = 0; i < noOfLaps; i++) {
    readFromBitOffset(buff, offset, &summary.__lapSizes[i], lapSize);
    offset += lapSize;
  }
  unsigned noOfBarSpacings = 0;
  readFromBitOffset(buff, offset, &noOfBarSpacings, barSpacingCountSize);
  offset += barSpacingCountSize;
  summary.__barSpacings.resize(noOfBarSpacings);
  for (unsigned i = 0; i < noOfBarSpacings; i++) {
    readFromBitOffset(buff, offset, &summary.__barSpacings[i], barSpacingSize);
    offset += barSpacingSize;
  }

  unsigned noOfExtraApertures = 0;
  readFromBitOffset(buff, offset, &noOfExtraApertures, extraApertureCountSize);
  offset += extraApertureCountSize;
  summary.__extraApertures.resize(noOfExtraApertures);
  for (unsigned i = 0; i < noOfExtraApertures; i++) {
    readFromBitOffset(buff, offset, &summary.__extraApertures[i],
                      apertureHandleSize);
    offset += apertureHandleSize;
  }

  size = MIN_COVERING_BYTES(offset) + sizeof(unsigned char) +
         summary.drawingNumber.size();

  return summary;
}

unsigned DrawingSummaryCompressionSchema::maxCompressedSize() const {
  return sizeof(unsigned char) + maxDrawingLength +
         MIN_COVERING_BYTES(
             matIDSize + widthSize + lengthSize + thicknessHandleSize * 2 + 4 +
             lapSize * 4 + apertureHandleSize + barSpacingCountSize +
             maxBarSpacingCount * barSpacingSize + extraApertureCountSize +
             maxExtraApertureCount * apertureHandleSize);
}

Drawing::MachineTemplate::MachineTemplate() {
  // Set the machine handle to the handle of the default machine
  machineHandle =
      DrawingComponentManager<Machine>::findComponentByID(1).handle();
  // Default the quantity on deck to 0
  quantityOnDeck = 0;
  // Default the position string to an empty string
  position = std::string();
  // Set the machine deck handle to the handle of the default deck
  deckHandle =
      DrawingComponentManager<MachineDeck>::findComponentByID(1).handle();
}

Machine &Drawing::MachineTemplate::machine() const {
  // Gets the machine from the drawing component manager
  return DrawingComponentManager<Machine>::getComponentByHandle(machineHandle);
}

MachineDeck &Drawing::MachineTemplate::deck() const {
  // Gets the machine deck from the drawing component manager
  return DrawingComponentManager<MachineDeck>::getComponentByHandle(deckHandle);
}

Material &Drawing::Lap::material() const {
  return DrawingComponentManager<Material>::getComponentByHandle(
      materialHandle);
}

Material &Drawing::ImpactPad::material() const {
  // Get the material from the drawing component manager and return it
  // directly
  return DrawingComponentManager<Material>::getComponentByHandle(
      materialHandle);
}

Aperture &Drawing::ImpactPad::aperture() const {
  // Get the material handle from the drawing component manager and return
  // it directly
  return DrawingComponentManager<Aperture>::getComponentByHandle(
      apertureHandle);
}

Material &Drawing::DamBar::material() const {
  return DrawingComponentManager<Material>::findComponentByID(materialID);
}

const Aperture &Drawing::ExtraAperture::aperture() const {
  return DrawingComponentManager<Aperture>::findComponentByID(apertureID);
}

Aperture &Drawing::CentreHole::aperture() const {
  return DrawingComponentManager<Aperture>::findComponentByID(apertureID);
}

Material &Drawing::Deflector::material() const {
  return DrawingComponentManager<Material>::getComponentByHandle(
      materialHandle);
}

Material &Drawing::Divertor::material() const {
  return DrawingComponentManager<Material>::getComponentByHandle(
      materialHandle);
}
