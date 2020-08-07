//
// Created by matthew on 29/07/2020.
//

#ifndef DATABASE_MANAGER_COMPONENTFILTERS_H
#define DATABASE_MANAGER_COMPONENTFILTERS_H

#include "DataSource.h"
#include "drawingComponents.h"

typedef SourceFilter<std::vector<unsigned>::const_iterator> ComboboxSourceFilter;

class RubberScreenClothMaterialFilter : public ComboboxSourceFilter {
private:
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class TackyBackMaterialFilter : public ComboboxSourceFilter {
private:
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class PolyurethaneMaterialFilter : public ComboboxSourceFilter {
private:
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class FlexBottomMaterialFilter : public ComboboxSourceFilter {
private:
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class BivitecMaterialFilter : public ComboboxSourceFilter {
private:
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class RubberModuleMaterialFilter : public ComboboxSourceFilter {
private:
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class SideIronFilter : public ComboboxSourceFilter {
public:
	void setSideIronType(SideIronType type);

	void removeSideIronType();

	void setSideIronFilterMinimumLength(unsigned length);

	void removeMinimumLength();

private:
	std::optional<SideIronType> filterType = std::nullopt;
	std::optional<unsigned> minimumLength = std::nullopt;

	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class MachineModelFilter : public ComboboxSourceFilter {
public:
	void setManufacturer(const std::string &manufacturerName);

	void removeManufacturerFilter();
private:
	std::optional<std::string> manufacturer = std::nullopt;

	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

#endif //DATABASE_MANAGER_COMPONENTFILTERS_H