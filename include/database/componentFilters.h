//
// Created by matthew on 29/07/2020.
//

#ifndef DATABASE_MANAGER_COMPONENTFILTERS_H
#define DATABASE_MANAGER_COMPONENTFILTERS_H

#include "DataSource.h"
#include "drawingComponents.h"

// Simple typedef for the correct type of filter object for the ComboboxDataSources
typedef SourceFilter<std::vector<unsigned>::const_iterator> ComboboxSourceFilter;

/// <summary>
/// RubberScreenClothMaterialFilter
/// A filter to install on a material data source to filter to only materials made of Rubber Screen Cloth.
/// </summary>
class RubberScreenClothMaterialFilter : public ComboboxSourceFilter {
private:
	// Internal filter function called through interface
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

/// <summary>
/// RubberScreenClothMaterialFilter
/// A filter to install on a material data source to filter to only materials made of Tackyback rubber.
/// </summary>
class TackyBackMaterialFilter : public ComboboxSourceFilter {
private:
	// Internal filter function called through interface
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

/// <summary>
/// RubberScreenClothMaterialFilter
/// A filter to install on a material data source to filter to only materials mode of Polyurethane.
/// </summary>
class PolyurethaneMaterialFilter : public ComboboxSourceFilter {
private:
	// Internal filter function called through interface
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

/// <summary>
/// RubberScreenClothMaterialFilter
/// A filter to install on a material data source to filter to only materials applicable to Extraflex and Polyflex 
/// materials.
/// </summary>
class FlexBottomMaterialFilter : public ComboboxSourceFilter {
private:
	// Internal filter function called through interface
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

/// <summary>
/// BivitecMaterialFilter
/// A filter to install on a material data source to filter to only materials used in Bivitec mats.
/// </summary>
class BivitecMaterialFilter : public ComboboxSourceFilter {
private:
	// Internal filter function called through interface
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

/// <summary>
/// BivitecMaterialFilter
/// A filter to install on a material data source to filter to only materials used in Rubber Module mats.
/// </summary>
class RubberModuleMaterialFilter : public ComboboxSourceFilter {
private:
	// Internal filter function called through interface
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

/// <summary>
/// SideIronFilter
/// Filters a set of side irons to those of an (optional) given type and ab (optional) minimum length.
/// </summary>
class SideIronFilter : public ComboboxSourceFilter {
public:
	/// <summary>
	/// Setter for the side iron type to filter on
	/// </summary>
	/// <param name="type">The type to filter by</param>
	void setSideIronType(SideIronType type);

	/// <summary>
	/// Remover for the side iron type filter. After calling this, no type filter will apply
	/// </summary>
	void removeSideIronType();

	/// <summary>
	/// Setter for the minimum side iron length to filter by
	/// </summary>
	/// <param name="length">The length to filter by</param>
	void setSideIronFilterMinimumLength(unsigned length);

	/// <summary>
	/// Remover for the side iron length filter. After calling this, no length filter will apply
	/// </summary>
	void removeMinimumLength();

	/// <summary>
	///  Setter for the type of material the mat uses to filter by
	/// </summary>
	/// <param name="rubberScreenCloth">Whether the mat is rubber screen cloth or extraflex</param>
	void setSideIronFilterMatType(bool isScreenCloth);

	/// <summary>
	/// Remover for the mat material filter. After calling this, no mat material filter will apply
	/// </summary>
	void removeMatType();
private:
	// The optional for the side iron type. Defaults to nullopt representing no filter.
	std::optional<SideIronType> filterType = std::nullopt;
	// The optional for the minimum length. Defaults to nullopt representing no filter.
	std::optional<unsigned> minimumLength = std::nullopt;
	// The optional for the material type. Defaults to nullopt representing no filter.
	std::optional<bool> rubberScreenCloth = std::nullopt;

	// Internal filter function called through interface
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

/// <summary>
/// MachineModelFilter
/// Filters a machine data source by a given manufacturer. This has the result of returing all the models for a given
/// manufacturer.
/// </summary>
class MachineModelFilter : public ComboboxSourceFilter {
public:
	/// <summary>
	/// Setter for the manufacturer name to filter on
	/// </summary>
	/// <param name="manufacturerName">The manufacturer name to filter on</param>
	void setManufacturer(const std::string &manufacturerName);

	/// <summary>
	/// Remover for the manufacturer fitler. After calling this, no filter will apply.
	/// </summary>
	void removeManufacturerFilter();
private:
	// The optional for the manufacturer name to filter on. Defaults to nullopt representing no filter.
	std::optional<std::string> manufacturer = std::nullopt;

	// Internal filter function called through interface
	bool __filter(std::vector<unsigned>::const_iterator element) const override;
};

#endif //DATABASE_MANAGER_COMPONENTFILTERS_H