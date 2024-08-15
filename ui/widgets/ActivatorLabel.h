//
// Created by matthew on 13/07/2020.
//

#ifndef DATABASE_MANAGER_ACTIVATORLABEL_H
#define DATABASE_MANAGER_ACTIVATORLABEL_H

#include <QWidget>
#include <QLabel>
#include <QEvent>
#include <QStyle>
#include <QVariant>

#include <vector>

/// <summary>
/// ActivatorLabel inherits QLabel
/// Creates a label widget that is activatible, and links its state with another widget, such as a combobox.
/// </summary>
class ActivatorLabel : public QLabel {
    Q_OBJECT

public:
    /// <summary>
    /// Constructs new label
    /// </summary>
    /// <param name="parent">Parent widget.</param>
    explicit ActivatorLabel(QWidget *parent = nullptr);

    /// <summary>
    /// destroys this object.
    /// </summary>
    ~ActivatorLabel() override;

    /// <summary>
    /// Sets the target for synced activation.
    /// </summary>
    /// <param name="activationTarget">Widget to sync activation with.</param>
    void addTarget(QWidget *activationTarget);

    /// <summary>
    /// Sets this and the synced objects active state.
    /// </summary>
    /// <param name="activeValue">What to set the objects states to.</param>
    void setActive(bool activeValue = true);

    /// <summary>
    /// Getter for current state of this label.
    /// </summary>
    /// <returns>The state of label.</returns>
    [[nodiscard]] bool active() const;

    /// <summary>
    /// Adds a callback to be called when the activation state changes.
    /// </summary>
    /// <param name="callback">A callback function, that takes the new state of the label.</param>
    void addActivationCallback(const std::function<void(bool)> &callback);

protected:
    /// <summary>
    /// Overloads mouse release to toggle the state. Continues event propogation.
    /// </summary>
    /// <param name="ev">The event that caused this function.</param>
    void mouseReleaseEvent(QMouseEvent *ev) override;

    /// <summary>
    /// Overloads enter event to change style when hovered over. Continues event propogation.
    /// </summary>
    /// <param name="ev">The event that caused this function.</param>
    void enterEvent(QEnterEvent *ev) override;

    /// <summary>
    /// Overloads enter event to change style when hovered over. Continues event propogation.
    /// </summary>
    /// <param name="ev">The event that caused this function.</param>
    void leaveEvent(QEvent *ev) override;

private:
    std::vector<QWidget *> targets;

    std::vector<std::function<void(bool)>> activationCallbacks;

    bool isActive = false;
};


#endif //DATABASE_MANAGER_ACTIVATORLABEL_H
