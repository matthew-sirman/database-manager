//
// Created by matthew on 10/07/2020.
//

#include "../../include/database/drawingComponents.h"
#include "../../include/database/ExtraPriceManager.h"

void DrawingComponent::serialise(void *buffer) const {
    *((unsigned *) buffer) = __handle;
}

unsigned DrawingComponent::deserialise(void *buffer) {
    return *((unsigned *) buffer);
}

unsigned DrawingComponent::componentID() const {
    return __componentID;
}

unsigned DrawingComponent::handle() const {
    return __handle;
}

DrawingComponent::DrawingComponent(unsigned id) {
    this->__componentID = id;
}

Product::Product(unsigned id) : DrawingComponent(id) {

}

Product *Product::fromSource(unsigned char** buff) {

    Product *product = new Product(*((unsigned *) *buff));
    *buff += sizeof(unsigned);

    unsigned char nameSize = *(*buff)++;

    product->productName = std::string((const char *) *buff, nameSize);
    *buff += nameSize;

    return product;
}

ComboboxDataElement Product::toDataElement(unsigned mode) const {
    return { productName, __handle };
}

BackingStrip::BackingStrip(unsigned id) : DrawingComponent(id) {

}


BackingStrip* BackingStrip::fromSource(unsigned char** buff) {

    BackingStrip* strip = new BackingStrip(*((unsigned*)*buff));
    *buff += sizeof(unsigned);

    strip->materialID = *((unsigned*)(*buff));
    *buff += sizeof(unsigned);

    return strip;
}

std::string BackingStrip::backingStripName() const {
    return DrawingComponentManager<Material>::getComponentByHandle(materialID).material();
}

ComboboxDataElement BackingStrip::toDataElement(unsigned mode) const {
    return { backingStripName(), __handle };
}

Aperture::Aperture(unsigned id) : DrawingComponent(id) {

}

ApertureShape& Aperture::getShape() const {
    return DrawingComponentManager<ApertureShape>::getComponentByHandle(this->apertureShapeID);
}

Aperture *Aperture::fromSource(unsigned char** buff) {

    Aperture *aperture = new Aperture(*((unsigned *) *buff));
    *buff += sizeof(unsigned);

    aperture->width = *((float *) (*buff));
    *buff += sizeof(float);
    aperture->length = *((float *) (*buff));
    *buff += sizeof(float);
    aperture->baseWidth = *((unsigned short *) (*buff));
    *buff += sizeof(unsigned short);
    aperture->baseLength = *((unsigned short *) (*buff));
    *buff += sizeof(unsigned short);
    aperture->apertureShapeID = *((unsigned *) (*buff));
    *buff += sizeof(unsigned);
    aperture->quantity = *((unsigned short *) (*buff));
    *buff += sizeof(unsigned short);
    aperture->isNibble = *(*buff)++;
    if (aperture->isNibble) {
        aperture->nibbleApertureId = *((unsigned*)(*buff));
        *buff += sizeof(unsigned);
    }

    return aperture;
}

std::string Aperture::apertureName() const {
    ApertureShape shape = DrawingComponentManager<ApertureShape>::getComponentByHandle(apertureShapeID);

    std::stringstream shapeName;

    if (shape.shape == "Blank") {
        shapeName << "Blank";
    } else if (shape.shape == "SQ" || shape.shape == "DIA") {
        shapeName << width << shape.shape;
    } else if (shape.shape == "BOTH") {
        shapeName << "ERROR!";
    } else {
        shapeName << width << shape.shape << length;
    }

    if (isNibble) {
        shapeName << " (Nibble using "+ DrawingComponentManager<Aperture>::findComponentByID(nibbleApertureId).apertureName() + ")";
    }

    return shapeName.str();
}

ComboboxDataElement Aperture::toDataElement(unsigned mode) const {
    return { apertureName(), __handle };
}

bool Aperture::operator<(const Aperture& other) const {
    if (this->apertureShapeID != other.apertureShapeID) {
        const unsigned int ap1 = ApertureShape::shapeOrder.at(this->apertureShapeID);
        const unsigned int ap2 = ApertureShape::shapeOrder.at(other.apertureShapeID);
        return ap1 < ap2;
    }
    return this->width < other.width;
}

const std::function<bool(const Aperture&, const Aperture&)>  Aperture::apertureComparator =
    [](const Aperture& a, const Aperture& b) {return a < b; };

ApertureShape::ApertureShape(unsigned id) : DrawingComponent(id) {

}

ApertureShape *ApertureShape::fromSource(unsigned char** buff) {

    ApertureShape *apertureShape = new ApertureShape(*((unsigned *) *buff));
    *buff += sizeof(unsigned);

    unsigned char shapeSize = *(*buff)++;
    apertureShape->shape = std::string((const char *) *buff, shapeSize);
    *buff += shapeSize;

    return { apertureShape };
}

ComboboxDataElement ApertureShape::toDataElement(unsigned mode) const {
    return { shape, __handle };
}

const std::unordered_map<unsigned int, unsigned int> ApertureShape::shapeOrder = {
            {1, 1},
            {2, 4},
            {3, 2},
            {4, 3},
            {5, 5},
            {6, 0},
			{7, 6},
            {8, 7}
    };

Material::Material(unsigned id) : DrawingComponent(id) {

}

Material* Material::fromSource(unsigned char** buff) {

    Material* material = new Material(*((unsigned*)*buff));
    *buff += sizeof(unsigned);

    material->hardness = *((unsigned short*)(*buff));
    *buff += sizeof(unsigned short);

    material->thickness = *((unsigned short*)(*buff));
    *buff += sizeof(unsigned short);


    unsigned char nameSize = *(*buff)++;

    material->materialName = std::string((const char*)*buff, nameSize);
    *buff += nameSize;

    unsigned char priceElements = *(*buff)++;
    for (unsigned char i = 0; i < priceElements; i++) {
        unsigned material_price_id = *((unsigned*)(*buff));
        *buff += sizeof(unsigned);
        float width = *((float*)(*buff));
        *buff += sizeof(float);
        float length = *((float*)(*buff));
        *buff += sizeof(float);
        float price = *((float*)(*buff));
        *buff += sizeof(float);
        MaterialPricingType pricingType = *((MaterialPricingType*)(*buff));
        *buff += sizeof(MaterialPricingType);
        material->materialPrices.push_back({ material_price_id, width, length, price, pricingType });
    }
    std::function<bool(const MaterialPrice&,
        const MaterialPrice&)> materialPriceComparator =
        [](const MaterialPrice& t1,
            const MaterialPrice& t2) {
                if (std::get<1>(t1) != std::get<1>(t2)) {
                    return std::get<1>(t1) < std::get<1>(t2);
                }
                return std::get<3>(t1) < std::get<3>(t2);
        };
    sort(material->materialPrices.begin(), material->materialPrices.end(), materialPriceComparator);

    return material;
}

ExtraPrice::ExtraPrice(unsigned id) :DrawingComponent(id) {

}

std::string ExtraPrice::extraPrice() const {
    switch (type) {
        case (ExtraPriceType::SIDE_IRON_NUTS) :
            return "Side Iron Nuts";
        case (ExtraPriceType::SIDE_IRON_SCREWS) :
            return "Side Iron Screws";
        case (ExtraPriceType::TACKYBACK_GLUE):
            return "Tackyback Glue";
        case (ExtraPriceType::LABOUR):
            return "Labour";
        case (ExtraPriceType::PRIMER):
            return "Primer";
        default:
            return std::string();
    }
}

ComboboxDataElement ExtraPrice::toDataElement(unsigned mode) const
{
    return { extraPrice(), __handle };;
}

template<>
float ExtraPrice::getPrice<ExtraPriceType::SIDE_IRON_NUTS>(typename ExtraPriceTrait<ExtraPriceType::SIDE_IRON_NUTS>::numType n) {
    return n * (price / amount.value());
}

template<>
float ExtraPrice::getPrice<ExtraPriceType::SIDE_IRON_SCREWS>(typename ExtraPriceTrait<ExtraPriceType::SIDE_IRON_SCREWS>::numType n) {
    return n * (price / amount.value());
}

template<>
float ExtraPrice::getPrice<ExtraPriceType::TACKYBACK_GLUE>(typename ExtraPriceTrait<ExtraPriceType::TACKYBACK_GLUE>::numType surfaceArea) {
    return surfaceArea * (price / squareMetres.value());
}

template<>
float ExtraPrice::getPrice<ExtraPriceType::LABOUR>(typename ExtraPriceTrait<ExtraPriceType::LABOUR>::numType n) {
    return n * (price / 60);
}

template<>
float ExtraPrice::getPrice<ExtraPriceType::PRIMER>(typename ExtraPriceTrait<ExtraPriceType::PRIMER>::numType n) {
    return n * (price / squareMetres.value());
}

ExtraPrice* ExtraPrice::fromSource(unsigned char** buff) {

    ExtraPrice* extraPrice = new ExtraPrice(*((unsigned*)*buff));
    *buff += sizeof(unsigned);

    ExtraPriceType type = *((ExtraPriceType*)(*buff));
    *buff += sizeof(ExtraPriceType);

    extraPrice->type = type;

    extraPrice->price = *((float*)(*buff));
    *buff += sizeof(float);

    switch (extraPrice->type) {
        case (ExtraPriceType::SIDE_IRON_NUTS): case (ExtraPriceType::SIDE_IRON_SCREWS):
            extraPrice->amount = *((unsigned*)(*buff));
            extraPrice->squareMetres = std::nullopt;
            *buff += sizeof(unsigned);
            break;
        case (ExtraPriceType::TACKYBACK_GLUE): case (ExtraPriceType::PRIMER):
            extraPrice->amount = std::nullopt;
            extraPrice->squareMetres = *((float*)(*buff));
            *buff += sizeof(float);
            break;
        case (ExtraPriceType::LABOUR):
            break;
    }
    switch (type) {
        case ExtraPriceType::SIDE_IRON_NUTS :
            ExtraPriceManager<ExtraPriceType::SIDE_IRON_NUTS>::setExtraPrice(extraPrice);
            break;
        case ExtraPriceType::SIDE_IRON_SCREWS :
            ExtraPriceManager<ExtraPriceType::SIDE_IRON_SCREWS>::setExtraPrice(extraPrice);
            break;
        case ExtraPriceType::TACKYBACK_GLUE :
            ExtraPriceManager<ExtraPriceType::TACKYBACK_GLUE>::setExtraPrice(extraPrice);
            break;
        case ExtraPriceType::LABOUR :
            ExtraPriceManager<ExtraPriceType::LABOUR>::setExtraPrice(extraPrice);
            break;
        case ExtraPriceType::PRIMER:
            ExtraPriceManager<ExtraPriceType::PRIMER>::setExtraPrice(extraPrice);
            break;
    }
    return extraPrice;

}

std::string LabourTime::labourTime() const {
    return job;
}

LabourType LabourTime::getType() const {
    if (job == "Cutting Amount")
        return LabourType::CUTTING_AMOUNT;
    else if (job == "Time to Punch")
        return LabourType::TIME_TO_PUNCH;
    else if (job == "Time to Shod")
        return LabourType::TIME_TO_SHOD;
    else if (job == "Time to Rebate")
        return LabourType::TIME_TO_REBATE;
    else if (job == "Backing Strips")
        return LabourType::BACKING_STRIPS;
    else if (job == "Cover Straps")
        return LabourType::COVER_STRAPS;
    else if (job == "Bonded Overlap")
        return LabourType::BONDED_OVERLAP;
    else if (job == "Cutting to Size")
        return LabourType::CUTTING_TO_SIZE;
    else if (job == "Impact Pads")
        return LabourType::IMPACT_PADS;
    else if (job == "Centre Holes")
        return LabourType::CENTRE_HOLES;
    else if (job == "Divertors")
        return LabourType::DIVERTORS;
    else if (job == "Deflectors")
        return LabourType::DEFLECTORS;
    else if (job == "Dam Bars")
        return LabourType::DAM_BARS;
    else if (job == "Cut Down")
        return LabourType::CUT_DOWN;
    return LabourType::ERR;
}

ComboboxDataElement LabourTime::toDataElement(unsigned mode) const {
    return { labourTime(), __handle };
}

LabourTime* LabourTime::fromSource(unsigned char** buff) {

    LabourTime* data = new LabourTime(*((unsigned*)*buff));
    *buff += sizeof(unsigned);

    size_t strLength = *((size_t*)*buff);
    *buff += sizeof(size_t);

    data->job.resize(strLength);
    std::memcpy(&data->job[0], *buff, strLength);
    *buff += strLength;

    data->time = *((unsigned*)(*buff));

    *buff += sizeof(unsigned);
    return data;
}

LabourTime::LabourTime(unsigned id) : DrawingComponent(id) {}

std::string Material::material() const {
    std::stringstream name;

    name << thickness << "mm " << materialName << " " << hardness << " Shore Hardness";

    return name.str();
}

ComboboxDataElement Material::toDataElement(unsigned mode) const {
    return { material(), __handle };
}

SideIron::SideIron(unsigned id) : DrawingComponent(id) {

}

std::string PowderCoatingPrice::powderCoatingPrice() const {
    std::stringstream out;
    out << "Type " << (char)('A' + this->componentID() - 1);
    out << ": hook price: " << this->hookPrice << " , strap price:" << this->strapPrice;
    return out.str();
}

ComboboxDataElement PowderCoatingPrice::toDataElement(unsigned mode) const {
    return { powderCoatingPrice(), __handle };
}

PowderCoatingPrice::PowderCoatingPrice(unsigned id) : DrawingComponent(id) {

}

PowderCoatingPrice* PowderCoatingPrice::fromSource(unsigned char** buff) {
    PowderCoatingPrice* price = new PowderCoatingPrice(*((unsigned*)*buff));
    *buff += sizeof(unsigned);

    price->hookPrice = *((float*)(*buff));
    *buff += sizeof(float);

    price->strapPrice = *((float*)(*buff));
    *buff += sizeof(float);

    return price;
}

SideIron *SideIron::fromSource(unsigned char** buff) {

    SideIron *sideIron = new SideIron(*((unsigned *) *buff));
    *buff += sizeof(unsigned);

    sideIron->type = (SideIronType) *(*buff);
    *buff += sizeof(unsigned char);
    sideIron->length = *((unsigned short *) (*buff));
    *buff += sizeof(unsigned short);

    unsigned char drawingNumberSize = *(*buff)++;
    sideIron->drawingNumber = std::string((const char *) *buff, drawingNumberSize);
    *buff += drawingNumberSize;

    unsigned char hyperlinkSize = *(*buff)++;
    sideIron->hyperlink = std::string((const char *) *buff, hyperlinkSize);
    *buff += hyperlinkSize;

    bool hasPrice = *(*buff)++;
    if (hasPrice) {
        sideIron->price = *((float*)(*buff));
        *buff += sizeof(float);
    }
    else
        sideIron->price = std::nullopt;

    bool hasScrews = *(*buff)++;
    if (hasScrews) {
        sideIron->screws = *((unsigned*)(*buff));
        *buff += sizeof(unsigned);
    }
    else
        sideIron->screws = std::nullopt;

    return sideIron;
}

std::string SideIron::sideIronStr() const {
    std::stringstream ss;
    ss << length << "mm ";
    ss << "Type ";
    switch (type) {
        case SideIronType::None:
            return "None";
        case SideIronType::A:
            ss << "A";
            break;
        case SideIronType::B:
            ss << "B";
            break;
        case SideIronType::C:
            ss << "C";
            break;
        case SideIronType::D:
            ss << "D";
            break;
        case SideIronType::E:
            ss << "E";
            break;
    }
    ss << " " << drawingNumber << " Side Iron";
    return ss.str();
}

ComboboxDataElement SideIron::toDataElement(unsigned mode) const {
    return { sideIronStr(), __handle };
}

SideIronPrice::SideIronPrice(unsigned id) : DrawingComponent(id) {

}

std::string SideIronPrice::sideIronPriceStr() const {
    std::stringstream ss;
    switch (type) {
        case SideIronType::None:
            return "None";
        case SideIronType::A:
            ss << "A";
            break;
        case SideIronType::B:
            ss << "B";
            break;
        case SideIronType::C:
            ss << "C";
            break;
        case SideIronType::D:
            ss << "D";
            break;
        case SideIronType::E:
            ss << "E";
            break;
    }
    ss << " Side Iron";
    return ss.str();
}

SideIronPrice* SideIronPrice::fromSource(unsigned char** buff) {

    SideIronPrice* sideIronPrice = new SideIronPrice(*((unsigned int*)*buff));
    *buff += sizeof(unsigned); 

    sideIronPrice->type = *((SideIronType*)(*buff));
    *buff += sizeof(SideIronType);
    sideIronPrice->lowerLength = *((unsigned*)(*buff));
    *buff += sizeof(unsigned);
    sideIronPrice->upperLength = *((unsigned*)(*buff));
    *buff += sizeof(unsigned);
    sideIronPrice->extraflex = *(*buff)++;
    sideIronPrice->price = *((float*)(*buff));
    *buff += sizeof(float);


    return sideIronPrice;

}

ComboboxDataElement SideIronPrice::toDataElement(unsigned mode) const {
    return { sideIronPriceStr(), __handle };
}

Machine::Machine(unsigned int id) : DrawingComponent(id) {

}

Machine *Machine::fromSource(unsigned char** buff) {

    Machine *machine = new Machine(*((unsigned *) *buff));
    *buff += sizeof(unsigned);

    unsigned char manufacturerSize = *(*buff)++;
    machine->manufacturer = std::string((const char *) *buff, manufacturerSize);
    *buff += manufacturerSize;

    unsigned char modelSize = *(*buff)++;
    machine->model = std::string((const char *) *buff, modelSize);
    *buff += modelSize;

    return machine;
}

std::string Machine::machineName() const {
    return manufacturer + " " + model;
}

ComboboxDataElement Machine::toDataElement(unsigned mode) const {
    switch (mode) {
        case 1:
            return { manufacturer, __handle };
        case 2:
            return { model, __handle };
        default:
            return { machineName(), __handle };
    }
}

MachineDeck::MachineDeck(unsigned int id) : DrawingComponent(id) {

}

MachineDeck *MachineDeck::fromSource(unsigned char** buff) {

    MachineDeck *machineDeck = new MachineDeck(*((unsigned *) *buff));
    *buff += sizeof(unsigned);

    unsigned char deckSize = *(*buff)++;
    machineDeck->deck = std::string((const char *) *buff, deckSize);
    *buff += deckSize;

    return machineDeck;
}

ComboboxDataElement MachineDeck::toDataElement(unsigned mode) const {
    return { deck, __handle };
}