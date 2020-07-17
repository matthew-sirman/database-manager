//
// Created by matthew on 13/07/2020.
//

#include "ActivatorLabel.h"

ActivatorLabel::ActivatorLabel(QWidget *parent) : QLabel(parent) {

}

ActivatorLabel::~ActivatorLabel() {

}

void ActivatorLabel::addTarget(QWidget *activationTarget) {
    activationTarget->setEnabled(isActive);
    targets.push_back(activationTarget);
}

void ActivatorLabel::setActive(bool activeValue) {
    isActive = activeValue;
    for (QWidget *target : targets) {
        target->setEnabled(isActive);
    }
    for (const std::function<void(bool)> &callback : activationCallbacks) {
        callback(isActive);
    }
}

bool ActivatorLabel::active() const {
    return isActive;
}

void ActivatorLabel::addActivationCallback(const std::function<void(bool)> &callback) {
    activationCallbacks.push_back(callback);
}

void ActivatorLabel::mouseReleaseEvent(QMouseEvent *ev) {
    setActive(!isActive);
    QLabel::mouseReleaseEvent(ev);
}

void ActivatorLabel::enterEvent(QEvent *ev) {
    if (isEnabled()) {
        setProperty("hovering", true);
        setStyle(style());
    }
    QWidget::enterEvent(ev);
}

void ActivatorLabel::leaveEvent(QEvent *ev) {
    setProperty("hovering", false);
    setStyle(style());
    QWidget::leaveEvent(ev);
}
