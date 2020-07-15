//
// Created by matthew on 13/07/2020.
//

#ifndef DATABASE_MANAGER_ACTIVATORLABEL_H
#define DATABASE_MANAGER_ACTIVATORLABEL_H

#include <QLabel>
#include <QEvent>
#include <QStyle>
#include <QVariant>

#include <vector>

class ActivatorLabel : public QLabel {
    Q_OBJECT

public:
    explicit ActivatorLabel(QWidget *parent = nullptr);

    ~ActivatorLabel() override;

    void addTarget(QWidget *activationTarget);

    void setActive(bool activeValue = true);

    [[nodiscard]] bool active() const;

protected:
    void mouseReleaseEvent(QMouseEvent *ev) override;

    void enterEvent(QEvent *ev) override;

    void leaveEvent(QEvent *ev) override;

private:
    std::vector<QWidget *> targets;

    bool isActive = false;
};


#endif //DATABASE_MANAGER_ACTIVATORLABEL_H
