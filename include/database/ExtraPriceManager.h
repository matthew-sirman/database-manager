#pragma once

#include "drawingComponents.h"

template<ExtraPriceType T>
class ExtraPriceManager {
public:
	static ExtraPrice* getExtraPrice();

	static void setExtraPrice(ExtraPrice*);

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