//
// Created by matthew on 09/08/2020.
//

#ifndef DATABASE_MANAGER_INSPECTOR_H
#define DATABASE_MANAGER_INSPECTOR_H

#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>

#include <functional>
#include <limits>

#include "ExpandingWidget.h"

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

	void addFloatField(const QString &label, float &field, unsigned precision = 1,
		float min = std::numeric_limits<float>::lowest(), float max = std::numeric_limits<float>::max());

	void addDoubleField(const QString &label, double &field, unsigned precision = 1,
		double min = std::numeric_limits<double>::lowest(), double max = std::numeric_limits<double>::max());

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

#endif //DATABASE_MANAGER_INSPECTOR_H