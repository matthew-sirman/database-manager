#pragma once

#include "drawingComponents.h"

/// <summary>\ingroup database
/// A manager for extra prices, allowing static access to all extra prices.
/// </summary>
/// <typeparam name="T">The type of extra price from the ExtraPriceType enum. </typeparam>
template<ExtraPriceType T>
class ExtraPriceManager {
public:
	/// <summary>
	/// Statically get the extra price of the given type.
	/// </summary>
	/// <returns>The extra price.</returns>
	static ExtraPrice* getExtraPrice();

	/// <summary>
	/// sets the extra price of a specific extra price.
	/// </summary>
	/// <param name="price">The extra price to set.</param>
	static void setExtraPrice(ExtraPrice *price);

private:
	static ExtraPrice* extraPrice;
};

template<ExtraPriceType T>
ExtraPrice* ExtraPriceManager<T>::extraPrice;

template<ExtraPriceType T>
void ExtraPriceManager<T>::setExtraPrice(ExtraPrice* price) {
	ExtraPriceManager<T>::extraPrice = price;
}

template<ExtraPriceType T>
ExtraPrice* ExtraPriceManager<T>::getExtraPrice() {
	return ExtraPriceManager<T>::extraPrice;
}