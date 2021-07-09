#pragma once
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QEvent>
#include <regex>

class EventTracker : public QObject {
	Q_OBJECT
public:
	explicit EventTracker(QLineEdit* watched, QWidget* parent = nullptr);
protected:
	bool eventFilter(QObject* obj, QEvent* event);
private:
	QLineEdit* watched = nullptr;
};