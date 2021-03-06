//
// Created by matthew on 10/07/2020.
//

#include "../../include/database/drawingComponents.h"

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

Product *Product::fromSource(void *buffer, unsigned &elementSize) {
    unsigned char *buff = (unsigned char *) buffer;

    elementSize = 0;

    Product *product = new Product(*((unsigned *) buff));
    elementSize += sizeof(unsigned);

    unsigned char nameSize = buff[elementSize++];
    product->productName = std::string((const char *) &buff[elementSize], nameSize);
    elementSize += nameSize;

    return product;
}

ComboboxDataElement Product::toDataElement(unsigned mode) const {
    return { productName, __handle };
}

Aperture::Aperture(unsigned id) : DrawingComponent(id) {

}

Aperture *Aperture::fromSource(void *buffer, unsigned &elementSize) {
    unsigned char *buff = (unsigned char *) buffer;

    elementSize = 0;

    Aperture *aperture = new Aperture(*((unsigned *) buff));
    elementSize += sizeof(unsigned);

    aperture->width = *((float *) (buff + elementSize));
    elementSize += sizeof(float);
    aperture->length = *((float *) (buff + elementSize));
    elementSize += sizeof(float);
    aperture->baseWidth = *((unsigned short *) (buff + elementSize));
    elementSize += sizeof(unsigned short);
    aperture->baseLength = *((unsigned short *) (buff + elementSize));
    elementSize += sizeof(unsigned short);
    aperture->apertureShapeID = *((unsigned *) (buff + elementSize));
    elementSize += sizeof(unsigned);
    aperture->quantity = *((unsigned short *) (buff + elementSize));
    elementSize += sizeof(unsigned short);

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

    return shapeName.str();
}

ComboboxDataElement Aperture::toDataElement(unsigned mode) const {
    return { apertureName(), __handle };
}

ApertureShape::ApertureShape(unsigned id) : DrawingComponent(id) {

}

ApertureShape *ApertureShape::fromSource(void *buffer, unsigned &elementSize) {
    unsigned char *buff = (unsigned char *) buffer;

    elementSize = 0;

    ApertureShape *apertureShape = new ApertureShape(*((unsigned *) buff));
    elementSize += sizeof(unsigned);

    unsigned char shapeSize = buff[elementSize++];
    apertureShape->shape = std::string((const char *) &buff[elementSize], shapeSize);
    elementSize += shapeSize;

    return { apertureShape };
}

ComboboxDataElement ApertureShape::toDataElement(unsigned mode) const {
    return { shape, __handle };
}

Material::Material(unsigned id) : DrawingComponent(id) {

}

Material *Material::fromSource(void *buffer, unsigned &elementSize) {
    unsigned char *buff = (unsigned char *) buffer;

    elementSize = 0;

    Material *material = new Material(*((unsigned *) buff));
    elementSize += sizeof(unsigned);

    material->hardness = *((unsigned short *) (buff + elementSize));
    elementSize += sizeof(unsigned short);
    material->thickness = *((unsigned short *) (buff + elementSize));
    elementSize += sizeof(unsigned short);

    unsigned char nameSize = buff[elementSize++];
    material->materialName = std::string((const char *) &buff[elementSize], nameSize);
    elementSize += nameSize;

    return material;
}

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

SideIron *SideIron::fromSource(void *buffer, unsigned &elementSize) {
    unsigned char *buff = (unsigned char *) buffer;

    elementSize = 0;

    SideIron *sideIron = new SideIron(*((unsigned *) buff));
    elementSize += sizeof(unsigned);

    sideIron->type = (SideIronType) *(buff + elementSize);
    elementSize += sizeof(unsigned char);
    sideIron->length = *((unsigned short *) (buff + elementSize));
    elementSize += sizeof(unsigned short);

    unsigned char drawingNumberSize = buff[elementSize++];
    sideIron->drawingNumber = std::string((const char *) &buff[elementSize], drawingNumberSize);
    elementSize += drawingNumberSize;

    unsigned char hyperlinkSize = buff[elementSize++];
    sideIron->hyperlink = std::string((const char *) &buff[elementSize], hyperlinkSize);
    elementSize += hyperlinkSize;

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

Machine::Machine(unsigned int id) : DrawingComponent(id) {

}

Machine *Machine::fromSource(void *buffer, unsigned int &elementSize) {
    unsigned char *buff = (unsigned char *) buffer;

    elementSize = 0;

    Machine *machine = new Machine(*((unsigned *) buff));
    elementSize += sizeof(unsigned);

    unsigned char manufacturerSize = buff[elementSize++];
    machine->manufacturer = std::string((const char *) &buff[elementSize], manufacturerSize);
    elementSize += manufacturerSize;

    unsigned char modelSize = buff[elementSize++];
    machine->model = std::string((const char *) &buff[elementSize], modelSize);
    elementSize += modelSize;

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

MachineDeck *MachineDeck::fromSource(void *buffer, unsigned int &elementSize) {
    unsigned char *buff = (unsigned char *) buffer;

    elementSize = 0;

    MachineDeck *machineDeck = new MachineDeck(*((unsigned *) buff));
    elementSize += sizeof(unsigned);

    unsigned char deckSize = buff[elementSize++];
    machineDeck->deck = std::string((const char *) &buff[elementSize], deckSize);
    elementSize += deckSize;

    return machineDeck;
}

ComboboxDataElement MachineDeck::toDataElement(unsigned mode) const {
    return { deck, __handle };
}