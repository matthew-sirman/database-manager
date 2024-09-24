#ifndef UI_HELPER_H
#define UI_HELPER_H

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <iostream>
#include <string>

#include "../include/database/drawingComponents.h"
#include "widgets/ActivatorLabel.h"
#include "widgets/DynamicComboBox.h"

/// <summary>
/// A concept that is true if and only if the type given is an optional, and the
/// value type its holding is a DrawingComponent.
/// </summary>
template <typename T>
concept IsOptionalDrawingComponent =
    requires { typename T::value_type; } &&
    DrawingComponentConcept<typename T::value_type> &&
    std::is_same_v<T, std::optional<typename T::value_type>>;

/// <summary>
/// A type that maps enums to the text equivilant for display.
/// </summary>
/// <typeparam name="T">The enum to get text for.</typeparam>
template <typename T>
  requires std::is_enum_v<T>
struct EnumMap;

/// @cond
template <>
struct EnumMap<SideIronType> {
  inline static const std::vector<std::string> names = {"A", "B", "C", "D",
                                                        "E"};
};
/// @endcond
/// <summary>
/// A concept that is true if and only if the provided type is an optional.
/// </summary>
template <typename U>
concept IsOptional = requires { typename U::value_type; } &&
                     std::is_same_v<U, std::optional<typename U::value_type>>;

/// <summary>
/// A helper class for AddComponentWindow. Provided many static functions for
/// adding components to a window, and getting their underlying values.
/// </summary>
/// <typeparam name="U">The type of data to be displayed/got.</param>
template <typename U>
class Helper {
 public:
  /// <summary>
  /// Adds a drawing component to the layout and binds the value to the provided
  /// getter.
  /// </summary>
  /// <typeparam name="X">The DrawingComponent to bind the value to.</typeparam>
  /// <param name="layout">The layout to add the widgets to.</param>
  /// <param name="attr">The data to bind the value to.</param>
  /// <param name="name">The name of the attribute.</param>
  template <typename X>
  static void addDrawingComponent(QFormLayout* layout, unsigned X::*attr,
                                  std::string name);

  /// <summary>
  /// Adds an optional drawing component, similar to addDrawingComponent, but
  /// includes an ActivatorLabel.
  /// </summary>
  /// <typeparam name="X">The DrawingComponent to bind the value to.</typeparam>
  /// <param name="layout">The layout to add the widgets to.</param>
  /// <param name="attr">The data to bind the value to.</param>
  /// <param name="name">The name of the attribute.</param>
  template <typename X>
  static void addOptionalDrawingComponent(QFormLayout* layout,
                                          std::optional<unsigned> X::*attr,
                                          std::string name);

  /// <summary>
  /// Adds a easily interpretable piece of data to the layout (such as a float
  /// or string) and binds its value to the attribute.
  /// </summary>
  /// <typeparam name="X">The DrawingComponent to bind the value to.</typeparam>
  /// <param name="layout">The layout to add the widgets to.</param>
  /// <param name="attr">The data to bind the value to.</param>
  /// <param name="name">The name of the attribute.</param>
  template <typename X>
  static void addRefComponent(QFormLayout* layout, U X::*attr,
                              std::string name);

  /// <summary>
  /// Adds a easily interpretable piece of data to the layout (such as a float
  /// or string).
  /// </summary>
  /// <param name="layout">The layout to add the widgets to.</param>
  /// <param name="name">The name of the attribute.</param>
  /// <returns>The widget that holds the component information.</returns>
  static QWidget* addComponentToLayout(QFormLayout* layout, std::string name);
  /// <summary>
  /// Adds an enums' values as a dropdown to the layout.
  /// or string).
  /// </summary>
  /// <typeparam name"T">A template for enuring the Helper class is template upon an enum.</param>
  /// <param name="layout">The layout to add the widgets to.</param>
  /// <param name="name">The name of the attribute.</param>
  /// <returns>The widget that holds the component information.</returns>
  template <typename T = U>
    requires std::is_enum_v<T>
  static QWidget* addEnumToLayout(QFormLayout* layout, std::string name);
  /// <summary>
  /// Adds a DrawingComponent to the layout.
  /// or string).
  /// </summary>
  /// <typeparam name"T">A template for enuring the Helper class is template upon a DrawingComponent.</param>
  /// <param name="layout">The layout to add the widgets to.</param>
  /// <param name="name">The name of the attribute.</param>
  /// <param name="source">A reference to a data source to populate the DynamicCombobox.</param>
  /// <returns>The widget that holds the component information.</returns>
  template <typename T = U>
    requires DrawingComponentConcept<T>
  static QWidget* addDrawingComponent(QFormLayout* layout, std::string name,
                                      ComboboxDataSource& source);
  /// <summary>
  /// Adds an optional DrawingComponent to the layout, with an activator label.
  /// or string).
  /// </summary>
  /// <typeparam name"T">A template for enuring the Helper class is template upon a DrawingComponent.
  /// <param name="layout">The layout to add the widgets to.</param>
  /// <param name="name">The name of the attribute.</param>
  /// <param name="source">A reference to a data source to populate the DynamicCombobox.</param>
  /// <returns>The widget that holds the component information.</returns>
  template <typename T = U>
    requires IsOptionalDrawingComponent<T>
  static QWidget* addOptionalDrawingComponent(QFormLayout* layout,
                                              std::string name,
                                              ComboboxDataSource& source);

  /// <summary>
  /// Gets the value of a widget.
  /// </summary>
  /// <param name="widget"> the widget to get the value from </param>
  /// <param name="isComponent"> True if the component is a DrawingCompnent, and thus needs special extraction, False otherwise.</param>
  /// <returns>A newly creates value interpreted from the provided widget</returns>
  static U getComponent(QWidget* widget, bool isComponent);
  /// <summary>
  /// Similar to getComponent, but for enums.
  /// </summary>
  /// <param name="widget"> the widget to get the value from </param>
  /// <returns>A newly creates value interpreted from the provided widget</returns>
  static U getEnum(QWidget* widget);
};

template <typename U>
template <typename X>
void Helper<U>::addDrawingComponent(QFormLayout* layout, unsigned X::*attr,
                                    std::string name){};

template <typename U>
template <typename X>
void Helper<U>::addOptionalDrawingComponent(QFormLayout* layout,
                                            std::optional<unsigned> X::*attr,
                                            std::string name){};

/// @cond
template <>
template <typename X>
void Helper<float>::addRefComponent(QFormLayout* layout, float X::*attr,
                                    std::string name) {
  QDoubleSpinBox* box = new QDoubleSpinBox();
  layout->addRow(name.c_str(), box);
};

template <>
template <typename X>
void Helper<unsigned short>::addRefComponent(QFormLayout* layout,
                                             unsigned short X::*attr,
                                             std::string name) {
  QSpinBox* box = new QSpinBox();
  layout->addRow(name.c_str(), box);
}

template <>
template <typename X>
void Helper<bool>::addRefComponent(QFormLayout* layout, bool X::*attr,
                                   std::string name) {}

template <>
template <typename X>
void Helper<unsigned>::addRefComponent(QFormLayout* layout, unsigned X::*attr,
                                       std::string name) {
  QSpinBox* box = new QSpinBox();
  layout->addRow(name.c_str(), box);
};

template <>
template <typename X>
void Helper<std::string>::addRefComponent(QFormLayout* layout,
                                          std::string X::*attr,
                                          std::string name) {
  QLineEdit* edit = new QLineEdit();
  layout->addRow(name.c_str(), edit);
};

template <>
template <typename X>
void Helper<std::optional<std::string>>::addRefComponent(
    QFormLayout* layout, std::optional<std::string> X::*attr,
    std::string name) {
  QLineEdit* edit = new QLineEdit();
  ActivatorLabel* label = new ActivatorLabel();
  label->addTarget(edit);
  label->setText(name.c_str());
  layout->addRow(label, edit);
};

/// @endcond

template <typename U>
template <typename T>
  requires DrawingComponentConcept<T>
QWidget* Helper<U>::addDrawingComponent(QFormLayout* layout, std::string name,
                                        ComboboxDataSource& source) {
  DynamicComboBox* combobox = new DynamicComboBox();
  combobox->setDataSource(source);
  layout->addRow(name.c_str(), combobox);
  return combobox;
};

template <typename U>
template <typename T>
  requires IsOptionalDrawingComponent<T>
QWidget* Helper<U>::addOptionalDrawingComponent(QFormLayout* layout,
                                                std::string name,
                                                ComboboxDataSource& source) {
  DynamicComboBox* combobox = new DynamicComboBox();
  ActivatorLabel* label = new ActivatorLabel();
  label->addTarget(combobox);
  layout->addRow(label, combobox);
  return combobox;
};

template <typename U>
template <typename T>
  requires std::is_enum_v<T>
QWidget* Helper<U>::addEnumToLayout(QFormLayout* layout, std::string name) {
  QComboBox* box = new QComboBox();
  for (std::string s : EnumMap<U>::names) {
    box->addItem(s.c_str());
  }
  layout->addRow(name.c_str(), box);
  return box;
}
template <typename U>
U Helper<U>::getEnum(QWidget* widget) {
  QComboBox* box = (QComboBox*)widget;
  return static_cast<U>(box->currentIndex());
}
#endif UI_HELPER_H
