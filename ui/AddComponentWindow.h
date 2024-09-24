//
// Created by alistair on 27/08/2024.
//
#pragma once
#ifndef DATABASE_MANAGER_ADDCOMPONENTWINDOW_H
#define DATABASE_MANAGER_ADDCOMPONENTWINDOW_H

#include <QCheckBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <array>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"
#include "Helper.h"
#include "widgets/ActivatorLabel.h"
#include "widgets/DynamicComboBox.h"

/// <summary>
/// Concept that verifies the templated class is either a child of
/// DrawingComponent, of a std::optional of a child of Drawing Component.
/// </summary>
template <typename T>
concept IsDrawingComponent =
    DrawingComponentConcept<T> || IsOptionalDrawingComponent<T>;

/// <summary>
/// A concept that is true if and only if the type houses a drawing component as
/// its value type.
/// </summary>
template <typename T>
concept IsComboboxSource = requires { typename T::value_type; } &&
                           DrawingComponentConcept<typename T::value_type>;

namespace Ui {
class AddComponentWindow;
}

/// <summary>
/// AddCompWindow inherits QDialog
/// The window displayed to the user, controlled by AddComponentWindow.
/// </summary>
class AddCompWindow : public QDialog {
  Q_OBJECT

 public:
  /// <summary>
  /// Constructs a new AddCompWindow.
  /// </summary>
  /// <param name="client">The network client to send results to.</param>
  /// <param name="name">The name of the component being added (e.g.
  /// material).</param>
  /// <param name="parent">The parent window of this dialog</param>
  explicit AddCompWindow(Client* client, std::string name,
                         QWidget* parent = nullptr);  // Add component

  /// <summary>
  /// Getter for the widget holding the main layout of this window.
  /// </summary>
  /// <returns>The main layout widget.</returns>
  QWidget* getLayoutContainer();

  /// <summary>
  /// Sets a callback to be ran when a conformation of insertion is recieved
  /// from the server.
  /// </summary>
  /// <param name="func">The function to be ran.</param>
  void setAcceptCallback(std::function<void(int)>&& func);

 private:
  Ui::AddComponentWindow* ui = nullptr;

  std::function<void(int)> acceptCallback;
};

/// <summary>
/// Recursive constructor for a tuple.
/// </summary>
/// @internal
/// <typeparam name=""></typeparam>
/// <typeparam name=""></typeparam>
/// @endinternal
template <typename, typename>
struct Cons;

/// <summary>
/// Adds Head to the tuple Tail
/// </summary>
/// <typeparam name="T">The type to be put on the head of the tuple.</typeparam>
/// <typeparam name="...Tail">The tuple to have the head added to.</typeparam>
template <typename T, typename... Tail>
struct Cons<T, std::tuple<Tail...>> {
  /// <summary>
  /// The newly formed type of the head attached to the tail.
  /// </summary>
  using type = std::tuple<T, Tail...>;
};

/// <summary>
/// A type filter that converts DrawingComponent 's into
/// ComboboxComponentDataSource 's, and removes everything else.
/// </summary>
/// @internal
/// <typeparam name="..."></typeparam>
/// @endinternal
template <typename...>
struct filter;

/// @cond
template <>
struct filter<> {
  using type = std::tuple<>;
};

template <typename, typename = std::void_t<>>
struct has_value_type : std::false_type {};

template <typename T>
struct has_value_type<T, std::void_t<typename T::value_type>> : std::true_type {
};

template <typename T>
struct select_value_type {
  using type =
      std::conditional_t<has_value_type<T>::value, typename T::value_type, T>;
};

template <typename Head, typename... Tail>
  requires DrawingComponentConcept<Head>
struct filter<Head, Tail...> {
  using type = typename Cons<ComboboxComponentDataSource<Head>,
                             typename filter<Tail...>::type>::type;
};

template <typename Head, typename... Tail>
  requires IsOptionalDrawingComponent<Head>
struct filter<Head, Tail...> {
  using type =
      typename Cons<ComboboxComponentDataSource<typename Head::value_type>,
                    typename filter<Tail...>::type>::type;
};

template <typename Head, typename... Tail>
struct filter<Head, Tail...> {
  using type = typename filter<Tail...>::type;
};

template <typename... Us>
using filter_t = typename filter<Us...>::type;

/// @endcond

/// <summary>
/// Maps DrawingComponents and optional DrawingComponents to unsigneds and
/// optional unsigneds respectively.
/// <typeparam name="T">The type to be mapped to a different type (if required).</typeparam>
/// </summary>
template <typename T>
struct Map {
  /// <summary>
  /// Map subtype that contains the mapped type.
  /// </summary>
  using type = typename std::conditional<
      DrawingComponentConcept<T>, unsigned,
      typename std::conditional<IsOptionalDrawingComponent<T>,
                                std::optional<unsigned>, T>::type>::type;
};

/// <summary>
/// A class for creating a window that allows a user to add a new component to
/// the database. T refers to the component to be created, and the Us parameter
/// pack is all of the types that make up the component, in the order they
/// appear in the relevant ComponentInsert data.
/// </summary>
/// <typeparam name="T"> The type of the component to generate the window
/// around.</typeparam> <typeparam name="Us"> The composite types that create
/// the main type T.</typeparam>
template <typename T, typename... Us>
class AddComponentWindow {
 public:
  /// <summary>
  /// Creates an AddComponentWindow for updating an existing component.
  /// This is not implemented.
  /// </summary>
  /// <param name="client"> the networking client for updating the
  /// database.</param> <param name="comp"> The name of the component.</param>
  /// <param name="attrs"> a list of pointers and names of all the attributes
  /// that make up the component.</param> <param name="parent"> The parent
  /// window.</param>
  explicit AddComponentWindow(
      Client* client, std::string comp,
      std::pair<
          std::variant<Us T::*, unsigned T::*, std::optional<unsigned> T::*>,
          std::string>&&... attrs,
      QWidget* parent = nullptr);  // update or remove

  /// <summary>
  /// Creates an AddComponentWindow for creating a new component.
  /// </summary>
  /// <param name="client"> The networking client for updating the
  /// database.</param> <param name="comp"> The name of the component being
  /// displayed.</param> <param name="names"> A list of all the names of the
  /// attributes that make up the component.</param> <param name="parent"> The
  /// parent window.</param>
  explicit AddComponentWindow(Client* client, std::string comp,
                              std::array<std::string, sizeof...(Us)> names,
                              QWidget* parent = nullptr);

  /// <summary>
  /// Displays the underlying AddCompWindow to the user.
  /// </summary>
  void show() const;

 private:
  std::tuple<
      std::variant<Us T::*, unsigned T::*, std::optional<unsigned> T::*>...>
      attrs;
  filter_t<Us...> components;

  Client* client;

  QFormLayout* layout;

  AddCompWindow* window;
};

template <typename T, typename... Us>
AddComponentWindow<T, Us...>::AddComponentWindow(
    Client* client, std::string comp,
    std::pair<
        std::variant<Us T::*, unsigned T::*, std::optional<unsigned> T::*>,
        std::string>&&... attrs,
    QWidget* parent)
    : client(client){
          // window = new AddCompWindow(client, comp, parent);
          // layout = new QFormLayout(window);
          // this->attrs = std::make_tuple(
          //    std::forward<
          //        std::variant<Us T::*, unsigned T::*, std::optional<unsigned>
          //        T::*>>(
          //        attrs.first)...);
          //(
          //    [this, attrs] {
          //      if constexpr (IsDrawingComponent<Us>) {
          //        if constexpr (DrawingComponentConcept<Us>)
          //          Helper<Us>::addDrawingComponent<T>(
          //              this->layout, std::get<1>(attrs.first), attrs.second);
          //        else
          //          Helper<Us>::addOptionalDrawingComponent<T>(
          //              this->layout, std::get<2>(attrs.first), attrs.second);
          //      } else {
          //        Helper<Us>::addRefComponent<T>(this->layout,
          //        std::get<0>(attrs.first),
          //                                       attrs.second);
          //      }
          //    }(),
          //    ...);
          // window->setLayout(layout);
      };

/// <summary>
/// Maps DrawingComponents to their ComponentInsert Data types for sending to
/// the database.
/// </summary>
template <typename T>
struct TypeMap {};

/// @cond
template <>
struct TypeMap<Strap> {
  using insertType = ComponentInsert::StrapData;
};

template <>
struct TypeMap<Aperture> {
  using insertType = ComponentInsert::ApertureData;
};

template <>
struct TypeMap<Machine> {
  using insertType = ComponentInsert::MachineData;
};

template <>
struct TypeMap<SideIron> {
  using insertType = ComponentInsert::SideIronData;
};
/// @endcond

template <typename T, typename... Us>
AddComponentWindow<T, Us...>::AddComponentWindow(
    Client* client, std::string comp,
    std::array<std::string, sizeof...(Us)> names, QWidget* parent)
    : client(client) {
  window = new AddCompWindow(client, comp, parent);
  layout = new QFormLayout(window->getLayoutContainer());
  std::vector<QWidget*> widgets;
  std::size_t index = 0;
  (
      [&]() {
        QWidget* widget = nullptr;
        if constexpr (DrawingComponentConcept<Us>) {
          ComboboxComponentDataSource<Us> source;
          std::get<ComboboxComponentDataSource<Us>>(components) =
              std::move(source);
          DrawingComponentManager<Us>::addCallback([this]() {
            std::get<ComboboxComponentDataSource<Us>>(components)
                .updateSource();
          });
          std::get<ComboboxComponentDataSource<Us>>(components).updateSource();
          widget = Helper<Us>::addDrawingComponent(
              this->layout, names[index++],
              std::get<ComboboxComponentDataSource<Us>>(components));
        } else if constexpr (IsOptionalDrawingComponent<Us>) {
          ComboboxComponentDataSource<typename Us::value_type> source;
          std::get<ComboboxComponentDataSource<typename Us::value_type>>(
              components) = std::move(source);
          DrawingComponentManager<typename Us::value_type>::addCallback(
              [this]() {
                std::get<ComboboxComponentDataSource<typename Us::value_type>>(
                    components)
                    .updateSource();
              });
          std::get<ComboboxComponentDataSource<typename Us::value_type>>(
              components)
              .updateSource();
          widget = Helper<Us>::addOptionalDrawingComponent(
              this->layout, names[index++],
              std::get<ComboboxComponentDataSource<typename Us::value_type>>(
                  components));
        } else if constexpr (std::is_enum_v<Us>) {
          widget = Helper<Us>::addEnumToLayout(this->layout, names[index++]);
        } else {
          widget =
              Helper<Us>::addComponentToLayout(this->layout, names[index++]);
        }
        widgets.push_back(widget);
      }(),
      ...);

  std::function<void(int)>&& accept = [widgets, client](int result) {
    switch ((QDialog::DialogCode)result) {
      case QDialog::DialogCode::Accepted: {
        std::size_t index = 0;
        ComponentInsert insert;
        insert.setComponentData<typename TypeMap<T>::insertType>(
            {([widgets, &index]() {
              if constexpr (std::is_enum_v<Us>)
                return std::forward<typename Map<Us>::type>(
                    Helper<typename Map<Us>::type>::getEnum(widgets[index++]));
              else if constexpr (IsDrawingComponent<Us>)
                return std::forward<typename Map<Us>::type>(
                    Helper<typename Map<Us>::type>::getComponent(
                        widgets[index++], true));
              else
                return std::forward<typename Map<Us>::type>(
                    Helper<typename Map<Us>::type>::getComponent(
                        widgets[index++], false));
            }())...});
        unsigned bufferSize = insert.serialisedSize();
        void* buffer = alloca(bufferSize);
        insert.serialise(buffer);
        client->addMessageToSendQueue(buffer, bufferSize);
        break;
      }
      case QDialog::DialogCode::Rejected:
        break;
    };
  };
  window->getLayoutContainer()->setLayout(layout);
  window->setAcceptCallback(std::move(accept));
};

template <typename T, typename... Us>
void AddComponentWindow<T, Us...>::show() const {
  window->show();
}
#endif  // DATABASE_MANAGER_ADDCOMPONENTWINDOW_H
