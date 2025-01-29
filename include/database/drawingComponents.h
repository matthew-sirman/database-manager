//
// Created by matthew on 10/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGCOMPONENTS_H
#define DATABASE_MANAGER_DRAWINGCOMPONENTS_H

#include <cstdio>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>

#include "../../guard.h"
#include "../networking/NetworkMessage.h"
#include "ComboboxDataSource.h"
#include "ExtraPriceManager.h"

//#ifdef CORE_DLL_EXPORT
//#define CORE_API __declspec(dllexport)
//#define DRAWING_COMPONENT_SETUP(component) template class __declspec(dllexport) DrawingComponentManager<component>;
//#define EXTRA_PRICE_SETUP(enm) template class __declspec(dllexport) ExtraPriceManager<ExtraPriceType::enm>;
//#define LOGGER_SETUP class __declspec(dllexport) Logger;
//#else
//#define CORE_API __declspec(dllimport)
//#define DRAWING_COMPONENT_SETUP(component) extern template class __declspec(dllimport) DrawingComponentManager<component>;
//#define EXTRA_PRICE_SETUP(enm) extern template class __declspec(dllimport) ExtraPriceManager<ExtraPriceType::enm>;
//#define LOGGER_SETUP extern class __declspec(dllimport) Logger;
//#endif
//
//#define SETUP_EXTRA_PRICES(MACRO) \
//  MACRO(SIDE_IRON_NUTS)           \
//  MACRO(SIDE_IRON_SCREWS)         \
//  MACRO(TACKYBACK_GLUE)           \
//  MACRO(LABOUR)                   \
//  MACRO(PRIMER)                   \
//  MACRO(SHOT_BLASTING)
//
//#define SETUP_DRAWING_COMPONENT(MACRO) \
//  MACRO(Strap)                         \
//  MACRO(Aperture)                      \
//  MACRO(ApertureShape)                 \
//  MACRO(BackingStrip)                  \
//  MACRO(ExtraPrice)                    \
//  MACRO(LabourTime)                    \
//  MACRO(Machine)                       \
//  MACRO(MachineDeck)                   \
//  MACRO(Material)                      \
//  MACRO(PowderCoatingPrice)            \
//  MACRO(Product)                       \
//  MACRO(SideIron)                      \
//  MACRO(SideIronPrice)
//
//#define SETUP_MANAGERS                             \
//  SETUP_DRAWING_COMPONENT(DRAWING_COMPONENT_SETUP) \
//  SETUP_EXTRA_PRICES(EXTRA_PRICE_SETUP)            \
//  LOGGER_SETUP
//

#define CORE_API

/// <summary>
/// DrawingComponent
/// Base class for each type of drawing component that is associated with an
/// entire drawing. The idea is that these components can be used on many
/// drawings, and so can be synchronised independently and then referenced
/// through internal handles.
/// </summary>
struct CORE_API DrawingComponent {
  /// <summary>
  /// Serialises this component into the provided buffer. The default
  /// implementation simply writes the handle into the buffer, as this should be
  /// synchronised with the server's handle for the same object, and as such is
  /// the minimum amount of information needed to transmit this component
  /// </summary>
  /// <param name="buffer">The buffer to serialise into.</param>
  virtual void serialise(void *buffer) const;

  /// <summary>
  /// Deserialises a component handle from the provided buffer. At this level,
  /// it is not known what sort of component is being deserialised, as there is
  /// no static method polymorphism. So, the method just returns the handle and
  /// the typing is left to a higher level's responsibility
  /// </summary>
  /// <param name="buffer">The buffer to deserialise from.</param>
  /// <returns>The handle for the deserialised component.</returns>
  static unsigned deserialise(void *buffer);

  /// <summary>
  /// Getter for the size this component will take in a buffer. This is the size
  /// of an unsigned representing the handle by default
  /// </summary>
  /// <returns>The size this component will occupy in a data buffer.</returns>
  constexpr size_t serialisedSize() const { return sizeof(unsigned); };

  /// <summary>
  /// Virtual method for converting a component into a ComboboxDataElement used
  /// for displaying these components in data sourced comboboxes. Modes change
  /// how elements are displayed in comboboxes, for example one mode could
  /// display The machine name, another could be machine model and another
  /// machine manufacturer.
  /// </summary>
  /// <param name="mode">An optional mode parameter which can be used by the
  /// overriding methods to particularise the mapping to a data element.</param>
  /// <returns>A ComboboxDataElement object containing the desired information
  /// for representing a component in a combobox.</returns>
  virtual ComboboxDataElement toDataElement(unsigned mode = 0) const = 0;

  /// <summary>
  /// Getter for the internal component ID as stored in the database. Note that
  /// this is not necessarily unique
  /// </summary>
  /// <returns>The component ID associated with this entity in the
  /// database.</returns>
  unsigned componentID() const;

  /// <summary>
  /// Getter for the internal component handle used in the application. Note
  /// that this IS necessarily unique and there is no necessary bijection from
  /// handles to components
  /// </summary>
  /// <returns>The handle assocaited with this entity in the internal
  /// representation.</returns>
  unsigned handle() const;

 protected:
  /// <summary>
  /// Internal constructor based on the component ID for this entity. This is
  /// hidden as it should not be possible to create components which do not
  /// actually exist
  /// </summary>
  /// <param name="id">The component ID as in the database.</param>
  DrawingComponent(unsigned id);

  // Hidden values for the component ID and handle for this component.

  /// <summary>
  /// The component id. This is the same as the ID stored in the database, used
  /// for updating components where possible. This is \a not garunteed to be
  /// unique.
  /// </summary>
  unsigned __componentID;
  /// <summary>
  /// The handle. This is a definitively unique identifier mainly used by the
  /// program.
  /// </summary>
  unsigned __handle;
};

template <typename T>
concept DrawingComponentConcept = std::is_base_of_v<DrawingComponent, T>;

template <typename T>
class DrawingComponentManager;

/// <summary>
/// Product
/// Represents a product type that a drawing may be of.
/// </summary>
struct CORE_API Product : public DrawingComponent {
  // Friend the DrawingComponentManager associated with this type such that it
  // may access the internal values
  friend class DrawingComponentManager<Product>;

 public:
  /// <summary>
  /// A product has a product name, e.g. "Rubber Screen Cloth"
  /// </summary>
  std::string productName;

  /// <summary>
  /// Overridden method to adapt this Product into a ComboboxDataElement
  /// </summary>
  /// <param name="mode">Optional mode parameter to specify which adapter method
  /// to use. For products, this value is unusued.</param> <returns>A
  /// ComboboxDataElement summary for this Product.</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

 protected:
  /// <summary>
  /// Private constructor based on the component ID
  /// </summary>
  /// <param name="id">The component ID for this product as in the
  /// database.</param>
  Product(unsigned id);

  /// <summary>
  /// Static deserialiser to convert a raw data buffer into a product element
  /// </summary>
  /// <param name="buff">The buffer to deserialise from.</param>
  /// <returns></returns>
  static Product *fromSource(unsigned char **buff);
};

/// <summary>
/// ApertureShape
/// An object representation of an aperture shape.
/// </summary>
struct CORE_API ApertureShape : public DrawingComponent {
  friend class DrawingComponentManager<ApertureShape>;

 public:
  /// <summary>
  /// the name of the shape.
  /// </summary>
  std::string shape;

  /// <summary>
  /// An adapter to convert apertureShapes into comboboxDataElement's.
  /// </summary>
  /// <param name="mode">Unused for ApertureShape</param>
  /// <returns>A comboboxDataElement populated with aperture shapes.</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

  /// <summary>
  /// Map that holds the natural ordering of shapes, by aperture shape id.
  /// </summary>
  static const std::unordered_map<unsigned int, unsigned int> shapeOrder;

 protected:
  /// <summary>
  /// Private constructor using component ID.
  /// </summary>
  /// <param name="id">Component ID to find the aperture shape by.</param>
  ApertureShape(unsigned id);

  /// <summary>
  /// Static deserialiser to build aperture shapes from buffers.
  /// </summary>
  /// <param name="buff">Buffer to read aperture shape from.</param>
  /// <returns>New saperture shape.</returns>
  static ApertureShape *fromSource(unsigned char **buff);
};

/// <summary>
/// Aperture
/// Represents an aperture tool
/// </summary>
struct CORE_API Aperture : public DrawingComponent {
  friend class DrawingComponentManager<Aperture>;

 public:
  /// <summary>
  /// The width of the aperture.
  /// </summary>
  float width{};
  /// <summary>
  /// The length of the aperture.
  /// </summary>
  float length{};
  /// <summary>
  /// The width of the base of the aperture.
  /// </summary>
  unsigned short baseWidth{};
  /// <summary>
  /// The length of the base of the aperture.
  /// </summary>
  unsigned short baseLength{};
  /// <summary>
  /// The component id of the shape of this aperture.
  /// </summary>
  unsigned apertureShapeID{};
  /// <summary>
  /// The quantity of these apertures in storage.
  /// </summary>
  unsigned short quantity{};
  /// <summary>
  /// The component id of the aperture this aperture is based off it is is
  /// nibbled.
  /// </summary>
  std::optional<unsigned> nibbleApertureId;

  /// <summary>
  /// Returns the name seen by the user, a composite of width, length and shape.
  /// </summary>
  /// <returns>The name of this aperture.</returns>
  std::string apertureName() const;

  /// <summary>
  /// An adapter to convert apertures into comboboxDataElement's.
  /// </summary>
  /// <param name="mode">Unused for Apertures</param>
  /// <returns>A comboboxDataElement populated with apertures.</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

  /// <summary>
  /// returns the shape of the aperture.
  /// </summary>
  /// <returns>The aperture's shape.</returns>
  virtual ApertureShape &getShape() const;

  /// <summary>
  /// Comparator for ordering apertures.
  /// </summary>
  /// <param name="other">Aperture to compare to.</param>
  /// <returns>True if this should be placed first, false otherwise.</returns>
  bool operator<(const Aperture &other) const;

  /// <summary>
  /// A premade aperture comparator using the < operator to order.
  /// </summary>
  static const std::function<bool(const Aperture &, const Aperture &)>
      apertureComparator;

 protected:
  /// <summary>
  /// Creates an aperture from its component ID from the database.
  /// </summary>
  /// <param name="id">Aperture's ID.</param>
  Aperture(unsigned id);

  /// <summary>
  /// Creates an aperture object from reading from a buffer.
  /// </summary>
  /// <param name="buff">Buffer to read from.</param>
  /// <returns>Newly created aperture object from buffer.</returns>
  static Aperture *fromSource(unsigned char **buff);
};

/// <summary>
/// Material
/// A class representing materials used for mats.
/// </summary>
struct CORE_API Material : public DrawingComponent {
  friend class DrawingComponentManager<Material>;

 public:
  /// <summary>
  /// defines a typedef for material prices. Contains
  /// price_material_id, width, length, price, material_pricing_type.
  /// </summary>
  typedef std::tuple<unsigned, float, float, float, MaterialPricingType>
      MaterialPrice;
  /// <summary>
  /// The name of the material.
  /// </summary>
  std::string materialName;
  /// <summary>
  /// The hardness of this material.
  /// </summary>
  unsigned short hardness{};
  /// <summary>
  /// The thickness of this material.
  /// </summary>
  unsigned short thickness{};

  /// <summary>
  /// All the prices related to this material.
  /// </summary>
  std::vector<MaterialPrice> materialPrices;

  /// <summary>
  /// returns the name of this material.
  /// </summary>
  /// <returns>The name.</returns>
  std::string material() const;

  /// <summary>
  /// An adapter to convert materials into comboboxDataElement's.
  /// </summary>
  /// <param name="mode">Unused for materials</param>
  /// <returns>A comboboxDataElement populated with this material.</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

 protected:
  /// <summary>
  /// Creates a material from its component ID from the database.
  /// </summary>
  /// <param name="id">The material's ID.</param>
  Material(unsigned id);

  /// <summary>
  /// Static deserialiser from buffer, creating new material.
  /// </summary>
  /// <param name="buff">Buffer to deserialise from.</param>
  /// <returns>Newly created material object</returns>
  static Material *fromSource(unsigned char **buff);
};

/// <summary>
/// BackingStrip
/// Represents a backing strip for a drawing.
/// </summary>
struct CORE_API BackingStrip : public DrawingComponent {
  friend class DrawingComponentManager<BackingStrip>;

 public:
  /// <summary>
  /// Getter for backing strip name.
  /// </summary>
  /// <returns>Backing strip name</returns>
  std::string backingStripName() const;

  /// <summary>
  /// Getter for the material of the backing strip.
  /// </summary>
  /// <returns>The backing strip's material.</returns>
  virtual Material &material() const;

  /// <summary>
  /// Sets the backing strips material.
  /// </summary>
  /// <param name="m">The new material for the backing strip.</param>
  void setMaterial(const Material &m);

  /// <summary>
  /// Overriden method to make this class applicable for a ComboboxDataElement.
  /// </summary>
  /// <param name="mode">Set mode, unused for backing strips.</param>
  /// <returns>A comboboxDataElement summary of backing strips</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

 protected:
  /// <summary>
  /// Private constructor for backing strips from component ID.
  /// </summary>
  /// <param name="id">The component ID for this backing strip in the
  /// database.</param>
  BackingStrip(unsigned id);

  /// <summary>
  /// Static deserialiser to convert raw data into a backing strip.
  /// </summary>
  /// <param name="buff">Buffer to deserialise from.</param>
  /// <returns></returns>
  static BackingStrip *fromSource(unsigned char **buff);

  /// <summary>
  /// The component id of the material the backing strip is made from.
  /// </summary>
  unsigned materialID{};
};

/// <summary>
/// ExtraPrice
/// A class representing an extra price, a price not directly attributable to
/// any general component.
/// </summary>
struct CORE_API ExtraPrice : public DrawingComponent {
  friend class DrawingComponentManager<ExtraPrice>;

 public:
  /// <summary>
  /// The type of this price. See \sa ExtraPriceType for more details.
  /// </summary>
  ExtraPriceType type;
  /// <summary>
  /// The price of this extra price.
  /// </summary>
  float price;
  /// <summary>
  /// The amount of square metres per price, is appropriate.
  /// </summary>
  std::optional<float> squareMetres;
  /// <summary>
  /// The amount per price, is appropriate.
  /// </summary>
  std::optional<unsigned> amount;

  /// <summary>
  /// Getter for this extra price's name.
  /// </summary>
  /// <returns>The name of this extra price.</returns>
  std::string extraPrice() const;

  /// <summary>
  /// An adapter to convert extra prices into comboboxDataElement's.
  /// </summary>
  /// <param name="mode">Unused for extra prices</param>
  /// <returns>A comboboxDataElement populated with this extra price.</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

  /// <summary>
  /// Calculates the price when given the relevant quantity measurement.
  /// </summary>
  /// <typeparam name="T">The type of price being requested.</typeparam>
  /// <param name="n">The amount of the type being used.</param>
  /// <returns>The price of n items of T.</returns>
  // template <ExtraPriceType T>
  // float getPrice(float n);

  /// <summary>
  /// maps ExtraPriceType to the name of the price in the database. Reverse of
  /// getType.
  /// </summary>
  inline static const std::unordered_map<ExtraPriceType, std::string> getName =
      {{ExtraPriceType::SIDE_IRON_NUTS, "side_iron_nuts"},
       {ExtraPriceType::TACKYBACK_GLUE, "glue"},
       {ExtraPriceType::SIDE_IRON_SCREWS, "side_iron_screws"},
       {ExtraPriceType::LABOUR, "labour"},
       {ExtraPriceType::PRIMER, "primer"},
       {ExtraPriceType::SHOT_BLASTING, "shot_blasting"}};
  /// <summary>
  /// Maps the name in the database to the ExtraPriceType enum. Reverse of
  /// getName.
  /// </summary>
  inline static const std::unordered_map<std::string, ExtraPriceType> getType =
      {{"side_iron_nuts", ExtraPriceType::SIDE_IRON_NUTS},
       {"glue", ExtraPriceType::TACKYBACK_GLUE},
       {"side_iron_screws", ExtraPriceType::SIDE_IRON_SCREWS},
       {"labour", ExtraPriceType::LABOUR},
       {"primer", ExtraPriceType::PRIMER},
       {"shot_blasting", ExtraPriceType::SHOT_BLASTING}};

 protected:
  /// <summary>
  /// Creates an extra price from its component ID from the database.
  /// </summary>
  /// <param name="id">The extra prices's ID.</param>
  ExtraPrice(unsigned id);

  /// <summary>
  /// Static deserialiser from buffer, creating a new extra price.
  /// </summary>
  /// <param name="buff">Buffer to deserialise from.</param>
  /// <returns>Newly created extra price object</returns>
  static ExtraPrice *fromSource(unsigned char **buff);
};

/// <summary>
/// LabourTime
/// A class representing the time of labour
/// </summary>
struct CORE_API LabourTime : public DrawingComponent {
  friend class DrawingComponentManager<LabourTime>;

 public:
  /// <summary>
  /// The name of the job this relates to.
  /// </summary>
  std::string job;
  /// <summary>
  /// The amount of time this job requires.
  /// </summary>
  unsigned time;

  /// <summary>
  /// Getter for the job's title.
  /// </summary>
  /// <returns>The job's title.</returns>
  std::string labourTime() const;
  /// <summary>
  /// returns the job's title as a member of the LabourType enum.
  /// </summary>
  /// <returns>Job's type.</returns>
  LabourType getType() const;

  /// <summary>
  /// An adapter to convert labour times into comboboxDataElement's.
  /// </summary>
  /// <param name="mode">Unused for labour times</param>
  /// <returns>A comboboxDataElement populated with this labour time.</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

 protected:
  /// <summary>
  /// Creates a labour time from its component ID from the database.
  /// </summary>
  /// <param name="id">The labour time's ID.</param>
  LabourTime(unsigned id);

  /// <summary>
  /// Static deserialiser from buffer, creating a new labour time.
  /// </summary>
  /// <param name="buff">Buffer to deserialise from.</param>
  /// <returns>Newly created labour time object</returns>
  static LabourTime *fromSource(unsigned char **buff);
};

/// <summary>
/// Enum holding all types of side irons.
/// </summary>
enum class CORE_API SideIronType {
  /// <summary>
  /// A side iron missing its type. This usually indicates an error.
  /// </summary>
  None,
  /// <summary>
  /// A side iron with an A type.
  /// </summary>
  A,
  /// <summary>
  /// A side iron with a B type.
  /// </summary>
  B,
  /// <summary>
  /// A side iron with a C type.
  /// </summary>
  C,
  /// <summary>
  /// A side iron with a D type.
  /// </summary>
  D,
  /// <summary>
  /// A side iron with a E type.
  /// </summary>
  E
};

/// <summary>
/// PowderCoatingPrice
/// A class representing the price of powder coating.
/// </summary>
struct CORE_API PowderCoatingPrice : public DrawingComponent {
  friend class DrawingComponentManager<PowderCoatingPrice>;

 public:
  /// <summary>
  /// The price to powder coat a hook.
  /// </summary>
  float hookPrice;
  /// <summary>
  /// The price to powder coat a strap.
  /// </summary>
  float strapPrice;

  /// <summary>
  /// Getter for the name of the powder coating price.
  /// </summary>
  /// <returns>Powder coating price name.</returns>
  std::string powderCoatingPrice() const;

  /// <summary>
  /// An adapter to convert a powder coating price into a comboboxDataElement.
  /// </summary>
  /// <param name="mode">Unused for powder coating prices</param>
  /// <returns>A comboboxDataElement populated with this powder coating
  /// price.</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

 protected:
  /// <summary>
  /// Creates a powder coating price from its component ID from the database.
  /// </summary>
  /// <param name="id">The powder coating price's ID.</param>
  PowderCoatingPrice(unsigned id);

  /// <summary>
  /// Static deserialiser from buffer, creating a new powder coating price.
  /// </summary>
  /// <param name="buff">Buffer to deserialise from.</param>
  /// <returns>Newly created powder coating price object</returns>
  static PowderCoatingPrice *fromSource(unsigned char **buff);
};

/// <summary>
/// SideIron
/// A class representing a single side iron used for mats.
/// </summary>
struct CORE_API SideIron : public DrawingComponent {
  friend class DrawingComponentManager<SideIron>;

 public:
  /// <summary>
  /// The type of the side iron.
  /// </summary>
  SideIronType type = SideIronType::A;
  /// <summary>
  /// The length of the side iron.
  /// </summary>
  unsigned short length{};
  /// <summary>
  /// The side irons drawing number.
  /// </summary>
  std::string drawingNumber;
  /// <summary>
  /// A link to the side irons drawing.
  /// </summary>
  std::string hyperlink;

  /// <summary>
  /// A bool representing whether or not this side iron is for extraflex mats. True if it is for extraflex mats, false otherwise.
  /// </summary>
  bool extraflex;

  /// <summary>
  /// A price for this specific side iron, if known.
  /// </summary>
  std::optional<float> price;
  /// <summary>
  /// The amount of screws required to attach this side iron, if known.
  /// </summary>
  std::optional<unsigned> screws;

  /// <summary>
  /// Getter for the full name of a side iron, including its length and type.
  /// </summary>
  /// <returns>The side iron's name.</returns>
  std::string sideIronStr() const;

  /// <summary>
  /// An adapter to convert a side iron into a comboboxDataElement.
  /// </summary>
  /// <param name="mode">Unused for side irons.</param>
  /// <returns>A comboboxDataElement populated with this side iron.</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

 protected:
  /// <summary>
  /// Creates a side iron from its component ID from the database.
  /// </summary>
  /// <param name="id">The side iron's ID.</param>
  SideIron(unsigned id);

  /// <summary>
  /// Static deserialiser from buffer, creating a new side iron.
  /// </summary>
  /// <param name="buff">Buffer to deserialise from.</param>
  /// <returns>Newly created side iron object</returns>
  static SideIron *fromSource(unsigned char **buff);
};

/// <summary>
/// SideIronPrice
/// A class representing generic side iron prices, prices that are consistent
/// across a range of lengths.
/// </summary>
struct CORE_API SideIronPrice : DrawingComponent {
  friend class DrawingComponentManager<SideIronPrice>;

  /// <summary>
  /// The type of the side iron this price applies to.
  /// </summary>
  SideIronType type;
  /// <summary>
  /// The lowest length of a side iron this price applies to.
  /// </summary>
  unsigned lowerLength;
  /// <summary>
  /// The highest length of a side iron this price appies to.
  /// </summary>
  unsigned upperLength;
  /// <summary>
  /// Whether or not this price is for extraflex mats or not.
  /// </summary>
  bool extraflex;
  /// <summary>
  /// The base price for this stock side iron.
  /// </summary>
  float price;

  /// <summary>
  /// Getter for the name of the set of side irons this price applies to.
  /// </summary>
  /// <returns>The side iron price's name.</returns>
  std::string sideIronPriceStr() const;

  /// <summary>
  /// An adapter to convert a side iron price into a comboboxDataElement.
  /// </summary>
  /// <param name="mode">Unused for side iron prices</param>
  /// <returns>A comboboxDataElement populated with this side iron
  /// price.</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

  /// <summary>
  /// Less than comparator function for side iron prices, comparing only their
  /// prices.
  /// </summary>
  /// <param name="other">side iron price to compare against.</param>
  /// <returns>True if this is smaller than other, false otherwise.</returns>
  bool operator<(const SideIronPrice &other) const {
    return price < other.price;
  }

  /// <summary>
  /// Greater than comparator function for side iron prices, comparing only
  /// their prices.
  /// </summary>
  /// <param name="other">side iron price to compare against.</param>
  /// <returns>True if this is greater than other, false otherwise.</returns>
  bool operator>(const SideIronPrice &other) const {
    return price > other.price;
  }

 protected:
  /// <summary>
  /// Creates a side iron price from its component ID from the database.
  /// </summary>
  /// <param name="id">The side iron price's ID.</param>
  SideIronPrice(unsigned id);

  /// <summary>
  /// Static deserialiser from buffer, creating a new side iron price.
  /// </summary>
  /// <param name="buff">Buffer to deserialise from.</param>
  /// <returns>Newly created side iron price object</returns>
  static SideIronPrice *fromSource(unsigned char **buff);
};

/// <summary>
/// An enum to know how many laps a mat has.
/// </summary>
enum class CORE_API LapSetting {
  /// <summary>
  /// A mat that has no laps.
  /// </summary>
  HAS_NONE,
  /// <summary>
  /// A mat that has one lap.
  /// </summary>
  HAS_ONE,
  /// <summary>
  /// A mat that has both laps.
  /// </summary>
  HAS_BOTH
};

/// <summary>
/// An enum representing how the laps are attached.
/// </summary>
enum class CORE_API LapAttachment {
  /// <summary>
  /// A lap that is integral, so the mat is simply cut longer.
  /// </summary>
  INTEGRAL,
  /// <summary>
  /// A lap that is bonded on, using glue and an additional material.
  /// </summary>
  BONDED
};

/// <summary>
/// Machine
/// A class representing a customer's machine on their.
/// </summary>
struct CORE_API Machine : public DrawingComponent {
  friend class DrawingComponentManager<Machine>;

 public:
  /// <summary>
  /// The name of the manufacturer
  /// </summary>
  std::string manufacturer;
  /// <summary>
  /// The name of the model.
  /// </summary>
  std::string model;

  /// <summary>
  /// Getter for the machines manufacturer and model.
  /// </summary>
  /// <returns>The machines name.</returns>
  std::string machineName() const;

  /// <summary>
  /// An adapter to convert a machine into a comboboxDataElement.
  /// </summary>
  /// <param name="mode">Default mode displays the machines full name, mode
  /// 1 displays the manufacturer and mode 2 displays the model.</param>
  /// <returns>A comboboxDataElement populated with this machine.</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

 protected:
  /// <summary>
  /// Creates a machine from its component ID from the database.
  /// </summary>
  /// <param name="id">The machine's ID.</param>
  Machine(unsigned id);

  /// <summary>
  /// Static deserialiser from buffer, creating a new machine.
  /// </summary>
  /// <param name="buff">Buffer to deserialise from.</param>
  /// <returns>Newly created machine object</returns>
  static Machine *fromSource(unsigned char **buff);
};

/// <summary>
/// MachineDeck
/// A Class representing a deck of a specific machine for a customer.
/// </summary>
struct CORE_API MachineDeck : public DrawingComponent {
  friend class DrawingComponentManager<MachineDeck>;

 public:
  /// <summary>
  /// A description of the deck this is centred around.
  /// </summary>
  std::string deck;

  /// <summary>
  /// An adapter to convert a machine dekc into a comboboxDataElement.
  /// </summary>
  /// <param name="mode">Unused for machine decks.</param>
  /// <returns>A comboboxDataElement populated with this machine deck.</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

 protected:
  /// <summary>
  /// Creates a machine deck from its component ID from the database.
  /// </summary>
  /// <param name="id">The machine deck's ID.</param>
  MachineDeck(unsigned id);

  /// <summary>
  /// Static deserialiser from buffer, creating a new machine deck.
  /// </summary>
  /// <param name="buff">Buffer to deserialise from.</param>
  /// <returns>Newly created machine deck object</returns>
  static MachineDeck *fromSource(unsigned char **buff);
};

/// <summary>
/// Strap inherits DrawingComponent
/// A rubber cover strap or wear tile liner.
/// </summary>
struct CORE_API Strap : public DrawingComponent {
  friend class DrawingComponentManager<Strap>;

 public:
  /// <summary>
  /// Getter for the composite name of the strap.
  /// </summary>
  /// <returns>The name of the strap.</returns>
  const std::string &strap() const;

  /// <summary>
  /// Getter for the material the strap is made from.
  /// </summary>
  /// <returns>The material the strap is made from.</returns>
  Material &material() const;

  /// <summary>
  /// Sets the material of the strap to th provided material.
  /// </summary>
  /// <param name="m">The new strap material.</param>
  void setMaterial(const Material &m);

  /// <summary>
  /// Converts a Strap into a ComboboxDataElement for visualisation in a
  /// DynamicCombobox.
  /// </summary>
  /// <param name="mode">Usused</param>
  /// <returns>An element for use my DynamicCoboboxes</returns>
  ComboboxDataElement toDataElement(unsigned mode = 0) const override;

  /// <summary>
  /// The handle of the material this strap is made from.
  /// </summary>
  unsigned materialHandle;

  /// <summary>
  /// A bool representing whether or not this strap is a wear tile liner. True
  /// if this strap is a wear tile liner, false if this strap is a rubber cover
  /// strap.
  /// </summary>
  bool isWTL;

 protected:
  /// <summary>
  /// Constructs an empty strap with only a componentID.
  /// </summary>
  /// <param name="id"></param>
  Strap(unsigned id);
  
  /// <summary>
  /// Creates a new Strap object from a buffer.
  /// </summary>
  /// <param name="buff">The buffer to read the strap from.</param>
  /// <returns>The newly created strap.</returns>
  static Strap *fromSource(unsigned char **buff);
};

/// <summary>
/// Defines a source for a combobox that is linked directly to a component, and
/// can be easily setup, updated, and read from.
/// </summary>
/// <typeparam name="T">The component that populates the combo box.</typeparam>
template <typename T>
class CORE_API ComboboxComponentDataSource : public ComboboxDataSource {
 public:
  /// <summary>
  /// Constructor that sets a default adapter.
  /// </summary>
  ComboboxComponentDataSource();

  /// <summary>
  /// Causes the combo box to be updated with any changes made to the underlying
  /// data, e.g. additions, removals, and edits.
  /// </summary>
  void updateSource() override;

  /// <summary>
  /// Sets a sorting function that the combo box will follow.
  /// </summary>
  /// <param name="comparator">The sorting function.</param>
  void sort(const std::function<bool(const T &, const T &)> &comparator);

  /// <summary>
  /// Makes all the data unique and removes all duplicates.
  /// </summary>
  void makeDistinct();

  /// <summary>
  /// Changes the toDataElement mode.
  /// </summary>
  /// <param name="mode">The new mode.</param>
  void setMode(unsigned mode);

  /// <summary>
  /// Empties out this object.
  /// </summary>
  /// <returns>The state of success of the operation.</returns>
  bool empty() const;

 private:
  /// <summary>
  /// Sets the adapter for the DataSource.
  /// </summary>
  /// <param name="adapter">The updated adapter</param>
  void setAdapter(
      const std::function<ComboboxDataElement(
          std::vector<unsigned>::const_iterator)> &adapter) override {}

  std::vector<unsigned> handleSet;

  unsigned elementMode = 0;
};

/// <summary>
/// </summary>
/// <typeparam name="T">Component Type</typeparam>
template <typename T>
ComboboxComponentDataSource<T>::ComboboxComponentDataSource() {
  ComboboxDataSource::setAdapter(
      [](std::vector<unsigned>::const_iterator iter) {
        return DrawingComponentManager<T>::getComponentByHandle(*iter)
            .toDataElement();
      });
}
/// <summary>
/// </summary>
/// <typeparam name="T">Component Type</typeparam>
template <typename T>
void ComboboxComponentDataSource<T>::updateSource() {
  handleSet = DrawingComponentManager<T>::dataIndexSet();
  this->__begin = handleSet.begin();
  this->__end = handleSet.end();

  DataSource::updateSource();
}

/// <summary>
/// </summary>
/// <typeparam name="T">Component Type</typeparam>
template <typename T>
void ComboboxComponentDataSource<T>::sort(
    const std::function<bool(const T &, const T &)> &comparator) {
  std::sort(
      handleSet.begin(), handleSet.end(), [comparator](unsigned a, unsigned b) {
        return comparator(DrawingComponentManager<T>::getComponentByHandle(a),
                          DrawingComponentManager<T>::getComponentByHandle(b));
      });
}

/// <summary>
/// </summary>
/// <typeparam name="T">Component Type</typeparam>
template <typename T>
void ComboboxComponentDataSource<T>::makeDistinct() {
  std::vector<unsigned>::const_iterator end = std::unique(
      handleSet.begin(), handleSet.end(), [this](unsigned a, unsigned b) {
        return DrawingComponentManager<T>::getComponentByHandle(a)
                   .toDataElement(elementMode)
                   .text == DrawingComponentManager<T>::getComponentByHandle(b)
                                .toDataElement(elementMode)
                                .text;
      });
  handleSet.erase(end, handleSet.end());
  this->__end = handleSet.end();
}

/// <summary>
/// </summary>
/// <typeparam name="T">Component Type</typeparam>
template <typename T>
void ComboboxComponentDataSource<T>::setMode(unsigned mode) {
  ComboboxDataSource::setAdapter(
      [mode](std::vector<unsigned>::const_iterator iter) {
        return DrawingComponentManager<T>::getComponentByHandle(*iter)
            .toDataElement(mode);
      });
  elementMode = mode;
}

/// <summary>
/// </summary>
/// <typeparam name="T">Component Type</typeparam>
template <typename T>
bool ComboboxComponentDataSource<T>::empty() const {
  return handleSet.empty();
}

/// <summary>
/// DrawingComponentManager
/// The core of access to components. This class can be used to statically
/// access all components.
/// </summary>
/// <typeparam name="T">The component, must inherit from
/// DrawingComponent</typeparam>
template <typename T>
class DrawingComponentManager {
  static_assert(std::is_base_of<DrawingComponent, T>::value,
                "DrawingComponentManager can only be used with types deriving "
                "from DrawingComponent.");

 public:
  /// <summary>
  /// Clears then populates the DrawingComponentManager with all the serialised
  /// objects stored in data.
  /// </summary>
  /// <param name="data">A buffer storing all objects, as a rvalue reference
  /// to indicate this takes ownership of the data.</param>
  /// <param name="dataSize">The size of the buffer.</param>
  static void sourceComponentTable(void *&&data, unsigned dataSize);

  /// <summary>
  /// Whether the data is dirtied.
  /// </summary>
  /// <returns>If the data is dirty.</returns>
  static bool dirty();

  /// <summary>
  /// Sets the data as dirty.
  /// </summary>
  static void setDirty();

  /// <summary>
  /// Find a component from its handle.
  /// </summary>
  /// <param name="handle">Component's handle.</param>
  /// <returns>The Component with handle handle.</returns>
  static T &getComponentByHandle(unsigned handle);

  /// <summary>
  /// Returns the highest handle in existance for this component.
  /// </summary>
  /// <returns>The highest handle.</returns>
  static unsigned maximumHandle();

  /// <summary>
  /// searches for the first component by its component ID.
  /// </summary>
  /// <param name="id">The component ID.</param>
  /// <returns>The component with matching id.</returns>
  static T &findComponentByID(unsigned id);

  /// <summary>
  /// searches for all components by their component ID.
  /// </summary>
  /// <param name="id">The component ID.</param>
  /// <returns>All matching components.</returns>
  static std::vector<T *> allComponentsByID(unsigned id);

  /// <summary>
  /// Checks if an ID has an object attached to it.
  /// </summary>
  /// <param name="id">The component ID to verify.</param>
  /// <returns>True if the component exists, false otherwise.</returns>
  static bool validComponentID(unsigned id);

  /// <summary>
  /// Checks if a handle has an object attached to it.
  /// </summary>
  /// <param name="handle">The handle to verify.</param>
  /// <returns>True if the component exists, false otherwise.</returns>
  static bool validComponentHandle(unsigned handle);

  /// <summary>
  /// Getter for the raw data the components are deserialised from.
  /// </summary>
  /// <returns>The raw serialised data.</returns>
  static void *rawSourceData();

  /// <summary>
  /// The raw data's size.
  /// </summary>
  /// <returns>The size of the raw data.</returns>
  static unsigned rawSourceDataSize();

  /// <summary>
  /// returns a vector of all indexes of the components.
  /// </summary>
  /// <returns>The vector if indexes of components.</returns>
  static std::vector<unsigned> dataIndexSet();

  /// <summary>
  /// Adds a callback to be ran when the data is updated.
  /// </summary>
  /// <param name="callback">The callback function</param>
  static void addCallback(const std::function<void()> &callback);

  // for testing purposes only, do not use
  /// @private
  static void addComponent(T *component);
  /// @private
  static void clear();

 private:
  static std::unordered_map<unsigned, T *> componentLookup;
  static std::unordered_map<unsigned, unsigned> handleToIDMap;
  static std::vector<unsigned> indexSet;

  static bool sourceDirty;

  static void *sourceData;
  static unsigned sourceDataSize;

  // TODO: callbacks never clear?
  static std::vector<std::function<void()>> updateCallbacks;
};
#endif  // DATABASE_MANAGER_DRAWINGCOMPONENTS_H
