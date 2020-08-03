//
// Created by matthew on 29/07/2020.
//

#ifndef DATABASE_MANAGER_COMPONENTFILTERS_H
#define DATABASE_MANAGER_COMPONENTFILTERS_H

#include "DataSource.h"
#include "drawingComponents.h"

class RubberScreenClothMaterialFilter : public SourceFilter<std::vector<unsigned>::const_iterator> {
private:
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class TackyBackMaterialFilter : public SourceFilter<std::vector<unsigned>::const_iterator> {
private:
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class PolyurethaneMaterialFilter : public SourceFilter<std::vector<unsigned>::const_iterator> {
private:
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class FlexBottomMaterialFilter : public SourceFilter<std::vector<unsigned>::const_iterator> {
private:
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class BivitecMaterialFilter : public SourceFilter<std::vector<unsigned>::const_iterator> {
private:
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class RubberModuleMaterialFilter : public SourceFilter<std::vector<unsigned>::const_iterator> {
private:
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

class SideIronFilter : public SourceFilter<std::vector<unsigned>::const_iterator> {
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



#endif //DATABASE_MANAGER_COMPONENTFILTERS_H