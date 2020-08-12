//
// Created by matthew on 29/07/2020.
//

#include "../../include/database/componentFilters.h"

bool RubberScreenClothMaterialFilter::__filter(std::vector<unsigned>::const_iterator element) const {
	return DrawingComponentManager<Material>::getComponentByHandle(*element).materialName == "Rubber Screen Cloth";
}

bool TackyBackMaterialFilter::__filter(std::vector<unsigned>::const_iterator element) const {
	return DrawingComponentManager<Material>::getComponentByHandle(*element).materialName == "Tacky Back";
}

bool PolyurethaneMaterialFilter::__filter(std::vector<unsigned>::const_iterator element) const {
	return DrawingComponentManager<Material>::getComponentByHandle(*element).materialName == "Polyurethane";
}

bool FlexBottomMaterialFilter::__filter(std::vector<unsigned>::const_iterator element) const {
	Material &mat = DrawingComponentManager<Material>::getComponentByHandle(*element);
	return mat.materialName == "Rubber Screen Cloth" && 5 <= mat.thickness && mat.thickness <= 10;
}

bool BivitecMaterialFilter::__filter(std::vector<unsigned>::const_iterator element) const {
	return DrawingComponentManager<Material>::getComponentByHandle(*element).materialName == "Moulded Polyurethane";
}

bool RubberModuleMaterialFilter::__filter(std::vector<unsigned>::const_iterator element) const {
	return DrawingComponentManager<Material>::getComponentByHandle(*element).materialName == "Rubber x60";
}

void SideIronFilter::setSideIronType(SideIronType type) {
	filterType = type;
	if (filterUpdateCallback) {
		filterUpdateCallback();
	}
}

void SideIronFilter::removeSideIronType() {
	filterType = std::nullopt;
	if (filterUpdateCallback) {
		filterUpdateCallback();
	}
}

void SideIronFilter::setSideIronFilterMinimumLength(unsigned length) {
	minimumLength = length;
	if (filterUpdateCallback) {
		filterUpdateCallback();
	}
}

void SideIronFilter::removeMinimumLength() {
	minimumLength = std::nullopt;
	if (filterUpdateCallback) {
		filterUpdateCallback();
	}
}

bool SideIronFilter::__filter(std::vector<unsigned>::const_iterator element) const {
	SideIron &sideIron = DrawingComponentManager<SideIron>::getComponentByHandle(*element);
	bool match = true;
	if (filterType.has_value()) {
		if (filterType.value() != sideIron.type) {
			match = false;
		}
	}
	if (minimumLength.has_value()) {
		if (minimumLength.value() > sideIron.length) {
			match = false;
		}
	}
	return match;
}

void MachineModelFilter::setManufacturer(const std::string &manufacturerName) {
	manufacturer = manufacturerName;
	if (filterUpdateCallback) {
		filterUpdateCallback();
	}
}

void MachineModelFilter::removeManufacturerFilter() {
	manufacturer = std::nullopt;
	if (filterUpdateCallback) {
		filterUpdateCallback();
	}
}

bool MachineModelFilter::__filter(std::vector<unsigned>::const_iterator element) const {
	if (!manufacturer.has_value()) {
		return false;
	}
	Machine &machine = DrawingComponentManager<Machine>::getComponentByHandle(*element);
	return machine.manufacturer == manufacturer.value();
}
