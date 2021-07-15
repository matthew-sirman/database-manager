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

class Inspector {
public:
	explicit Inspector(const QString &title);

	~Inspector();

	QFormLayout *layout() const;

	ExpandingWidget *container() const;

	unsigned currentOwner() const;

	unsigned acquire();

	void addIntegerField(const QString &label, int &field,
						 int min = std::numeric_limits<int>::lowest(), int max = std::numeric_limits<int>::max());

	void addIntegerField(const QString &label, const std::function<void(int)> &fieldSetter, int value = 0,
						 int min = std::numeric_limits<int>::lowest(), int max = std::numeric_limits<int>::max());

	void addFloatField(const QString &label, float &field, unsigned precision = 1,
					   float min = std::numeric_limits<float>::lowest(), float max = std::numeric_limits<float>::max());

	void addFloatField(const QString &label, const std::function<void(float)> &fieldSetter, float value = 0.0f,
					   unsigned precision = 1, float min = std::numeric_limits<float>::lowest(), float max = std::numeric_limits<float>::max());

	void addDoubleField(const QString &label, double &field, unsigned precision = 1,
						double min = std::numeric_limits<double>::lowest(), double max = std::numeric_limits<double>::max());

	void addDoubleField(const QString &label, const std::function<void(double)> &fieldSetter, double value = 0.0,
						unsigned precision = 1, double min = std::numeric_limits<double>::lowest(), double max = std::numeric_limits<double>::max());

	void addBooleanField(const QString &label, bool &field);

	void addBooleanField(const QString &label, const std::function<void(bool)> &fieldSetter, bool value = false);

	template<typename T>
	void addComponentField(const QString &label, const std::function<void(const T &)> &fieldSetter, 
						   ComboboxDataSource &source, unsigned defaultHandle = -1);

	void expand();

	void collapse();

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
	ComboboxDataSource& source, unsigned defaultHandle) {
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
