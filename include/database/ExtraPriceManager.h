#pragma once


/// <summary>
/// ExtraPriceTrait
/// Defines a struct templated onto the enum ExtraPriceType. Each template
/// defines the type of data, that reflects quantity, it should store, e.g.
/// unsigned.
/// </summary>
/// <typeparam name="T">ExtraPriceType enum</typeparam>
template <ExtraPriceType T>
struct ExtraPriceTrait {
    /// <summary>
    /// Default to using unsigned as the measure of quantity.
    /// </summary>
    using numType = unsigned;
};

/// <summary>
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
    /// Calculates the price using the underlying extraPrice object.
    /// </summary>
    /// <param name="n">Some measure of quantity for calculating total price.</param>
    /// <returns>The price if the data exists, otherwise std::nullopt</returns>
    static std::optional<float> getPrice(typename ExtraPriceTrait<T>::numType n);

	/// <summary>
	/// sets the extra price of a specific extra price.
	/// </summary>
	/// <param name="price">The extra price to set.</param>
	static void setExtraPrice(ExtraPrice *price);

private:
	static ExtraPrice* extraPrice;
};

template <ExtraPriceType T>
ExtraPrice *ExtraPriceManager<T>::extraPrice = nullptr;

