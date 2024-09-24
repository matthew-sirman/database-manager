//
// Created by matthew on 09/08/2020.
//

#ifndef DATABASE_MANAGER_INSPECTOR_H
#define DATABASE_MANAGER_INSPECTOR_H

#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QCheckBox>

#include <functional>
#include <limits>

#include "ExpandingWidget.h"
#include "DynamicComboBox.h"
#include "../../include/database/drawingComponents.h"

/// <summary>
/// Insepctor is an object that can display relevant information about a graphical element, such as the aperture of a centre hole.
/// </summary>
class Inspector {
public:

	/// <summary>
	/// Constructs a new inspector with a title.
	/// </summary>
	/// <param name="title">The title of the inspector.</param>
	explicit Inspector(const QString &title);

	/// <summary>
	/// Destroys the insepctor and any relevant memory.
	/// </summary>
	~Inspector();

	/// <summary>
	/// The layout of the information shown by the inspector.
	/// </summary>
	/// <returns></returns>
	QFormLayout *layout() const;

	/// <summary>
	/// The container widget that houses the inspector.
	/// </summary>
	/// <returns></returns>
	ExpandingWidget *container() const;

	/// <summary>
	/// Helps identify who is the owner. This helps to stop the owner clearing and repopulating the inspector itself,
	/// or on deletion of the object in control, clearing the inspector.
	/// </summary>
	/// <returns>Unused</returns>
	unsigned currentOwner() const;

	/// <summary>
	/// Clears the inspector and collapses the ExpandingWidget container.
	/// </summary>
	void clear();

	/// <summary>
	/// Resets the contents of the inspector, then returns an unused owner.
	/// </summary>
	/// <returns>unused</returns>
	unsigned acquire();

	/// <summary>
	/// Adds a new field integer input to the inspector in a form layout.
	/// </summary>
	/// <param name="label">Label of the input.</param>
	/// <param name="field">A memory reference where the fields value will be read from initially and written to.</param>
	/// <param name="min">Minimum value of the input.</param>
	/// <param name="max">Maximum value of the input.</param>
	void addIntegerField(const QString &label, int &field,
						 int min = std::numeric_limits<int>::lowest(), int max = std::numeric_limits<int>::max());

	/// <summary>
	/// Adds a new field integer input to the inspector in a form layout.
	/// </summary>
	/// <param name="label">Label of the input.</param>
	/// <param name="fieldSetter">Function that is ran with the new value of the field on fields update.</param>
	/// <param name="value">Initial value of field.</param>
	/// <param name="min">Minimum value of field.</param>
	/// <param name="max">Maximum value of field.</param>
	void addIntegerField(const QString &label, const std::function<void(int)> &fieldSetter, int value = 0,
						 int min = std::numeric_limits<int>::lowest(), int max = std::numeric_limits<int>::max());

	/// <summary>
	/// Adds a new field float input to the inspector in a form layout.
	/// </summary>
	/// <param name="label">Label of the input.</param>
	/// <param name="field">A memory reference where the fields value will be read from initially and written to.</param>
	/// <param name="precision">Precision of float, ie how many decimal places.</param>
	/// <param name="min">Minimum value of field.</param>
	/// <param name="max">Maximum value of field.</param>
	void addFloatField(const QString &label, float &field, unsigned precision = 1,
					   float min = std::numeric_limits<float>::lowest(), float max = std::numeric_limits<float>::max());

	/// <summary>
	/// Adds a new field float input to the inspector in a form layout.
	/// </summary>
	/// <param name="label">Label of the input.</param>
	/// <param name="fieldSetter">Function to be called with the updated field value.</param>
	/// <param name="value">Initial value of field.</param>
	/// <param name="precision">Precision of float, ie how many decimal places.</param>
	/// <param name="min">Minimum value of field.</param>
	/// <param name="max">Maximum value of field.</param>
	void addFloatField(const QString &label, const std::function<void(float)> &fieldSetter, float value = 0.0f,
					   unsigned precision = 1, float min = std::numeric_limits<float>::lowest(), float max = std::numeric_limits<float>::max());

	/// <summary>
	/// Adds a new field double input to the inspector in a form layout.
	/// </summary>
	/// <param name="label">Label of the input.</param>
	/// <param name="field">Memory reference to read and write value of field to.</param>
	/// <param name="precision">The precision the field allows, ie how many decimal places.</param>
	/// <param name="min">The minimum value of field.</param>
	/// <param name="max">The maximum value of field.</param>
	void addDoubleField(const QString &label, double &field, unsigned precision = 1,
						double min = std::numeric_limits<double>::lowest(), double max = std::numeric_limits<double>::max());

	/// <summary>
	/// Adds a new field double input to the inspector in a form layout.
	/// </summary>
	/// <param name="label">Label of the input.</param>
	/// <param name="fieldSetter">Function to be called with the updated value.</param>
	/// <param name="value">Initial value of field.</param>
	/// <param name="precision">The precision the field allows, ie how many decimal places.</param>
	/// <param name="min">The minimum value of field.</param>
	/// <param name="max">The maximum value of field.</param>
	void addDoubleField(const QString &label, const std::function<void(double)> &fieldSetter, double value = 0.0,
						unsigned precision = 1, double min = std::numeric_limits<double>::lowest(), double max = std::numeric_limits<double>::max());

	/// <summary>
	/// Adds a new field boolean input to the inspector in a form layout.
	/// </summary>
	/// <param name="label">Label of the input.</param>
	/// <param name="field">Memory reference to read and write value of field to.</param>
	void addBooleanField(const QString &label, bool &field);

	/// <summary>
	/// Adds a new field boolean input to the inspector in a form layout.
	/// </summary>
	/// <param name="label">Label of the input.</param>
	/// <param name="fieldSetter">Function to be called with the updated value.</param>
	/// <param name="value">Initial value of field.</param>
	void addBooleanField(const QString &label, const std::function<void(bool)> &fieldSetter, bool value = false);

	/// <summary>
	/// Adds a new combobox field to the inspector in a form layout, that is synced with a
	/// \ref ComboboxComponentDataSource to display all components of the correct type, and be filterable.
	/// </summary>
	/// <typeparam name="T">The type of the component for the field. Must inherit \ref DrawingComponent.</typeparam>
	/// <param name="label">Label of the input.</param>
	/// <param name="fieldSetter">Function to be called with the updated component.</param>
	/// <param name="source">\ref ComboboxComponentDataSource of target type for populating field.</param>
	/// <param name="defaultHandle">Handle to start the combobox at.</param>
	template<typename T>
	void addComponentField(const QString &label, const std::function<void(const T &)> &fieldSetter, 
						   ComboboxComponentDataSource<T> &source, unsigned defaultHandle = -1);

	/// <summary>
	/// Expands ExpandingWidget containing the inspector
	/// </summary>
	void expand();

	/// <summary>
	/// Collapses ExpandingWidget containing the inspector
	/// </summary>
	void collapse();

	/// <summary>
	/// Adds a function to be called whenever any value is updated.
	/// </summary>
	/// <param name="trigger">Function to be triggered on update.</param>
	void addUpdateTrigger(const std::function<void()> &trigger);

private:
	QFormLayout *contents = nullptr;
	ExpandingWidget *expandingContainer = nullptr;

	unsigned currentRow = 0;

	unsigned owner = 0;

	std::vector<std::function<void()>> updateTriggers;

};

template<typename T>
inline void Inspector::addComponentField(const QString& label, const std::function<void(const T&)>& fieldSetter,
	ComboboxComponentDataSource<T>& source, unsigned defaultHandle) {
	static_assert(std::is_base_of<DrawingComponent, T>::value, "Component Field type must inherit DrawingComponent.");

	DynamicComboBox* fieldInput = new DynamicComboBox();
	fieldInput->setDataSource(source);
	fieldInput->setEditable(true);
	fieldInput->setInsertPolicy(QComboBox::NoInsert);

	if (defaultHandle != -1) {
		fieldInput->setCurrentIndex(fieldInput->findData(defaultHandle));
	}

	QWidget::connect(fieldInput, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this, fieldSetter, fieldInput](int index) {
		if (index == -1) {
			return;
		}
		fieldSetter(DrawingComponentManager<T>::getComponentByHandle(fieldInput->itemData(index).toInt()));
		std::for_each(updateTriggers.begin(), updateTriggers.end(), [](const std::function<void()>& trigger) { trigger(); });
		});

	contents->addRow(label, fieldInput);
};
#endif //DATABASE_MANAGER_INSPECTOR_H
