#include "EventTracker.h"

EventTracker::EventTracker(QLineEdit* watched, QWidget* parent) : QObject(parent) {
	this->watched = watched;
};

bool EventTracker::eventFilter(QObject* obj, QEvent* event) {
	if (event->type() == QEvent::FocusOut && watched) {
		std::basic_regex rx("^[a-zA-Z]{2}[0-9]$");
		if (std::regex_match(watched->text().toStdString(), rx)) {
			watched->setText(watched->text().toStdString().insert(2, "0").c_str());
		}
		return QObject::eventFilter(obj, event);
	}
	else {
		return QObject::eventFilter(obj, event);
	}
}