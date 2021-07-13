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

enum trackerFlags {
	FOCUSOUT = 0x1 >> 0,
	FOCUSIN = 0X1 >> 1,
};
class CTracker;
template <typename T>
class Tracker {
	friend CTracker;
public:
	Tracker(T* watch, int flags);

private:
	T* watched = nullptr;
	CTracker* child = nullptr;
};

class CTracker : public QObject {
	Q_OBJECT
public:
	template<typename T>
	explicit CTracker(Tracker<T>* tracker, int flags, QWidget* parent = nullptr);
};

template<typename T>
Tracker<T>::Tracker(T* watch, int flags) {
	watched = watch;
	child = new CTracker(this, flags);
};