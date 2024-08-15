//
// Created by matthew on 09/08/2020.
//

#ifndef DATABASE_MANAGER_EXPANDINGWIDGET_H
#define DATABASE_MANAGER_EXPANDINGWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QToolButton>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QFrame>
#include <QScrollArea>

/// <summary>
/// ExpandingWidget inherits QWidget
/// A simple widget that can minimised and maximised through functions, \ref expand and \ref collapse.
/// </summary>
class ExpandingWidget : public QWidget {
	Q_OBJECT
public:
	/// <summary>
	/// Constructs a new expanding widget with a visible title.
	/// </summary>
	/// <param name="title">Titlle of widget.</param>
	/// <param name="parent">Parent of widget.</param>
	explicit ExpandingWidget(const QString &title = "", QWidget *parent = nullptr);

	/// <summary>
	/// Destroys widget and all relevant memory.
	/// </summary>
	~ExpandingWidget() override;

	/// <summary>
	/// Fills the widget with a layout.
	/// </summary>
	/// <param name="layout"></param>
	void setContent(QLayout *layout);

	/// <summary>
	/// Expands the widget, making its content visible.
	/// </summary>
	void expand();

	/// <summary>
	/// Collapses the widget, hiding its content.
	/// </summary>
	void collapse();

private:
	QGridLayout *mainLayout = nullptr;
	QToolButton *expandButton = nullptr;
	QFrame *headerFrame = nullptr;
	QScrollArea *contentArea = nullptr;
};

#endif //DATABASE_MANAGER_EXPANDINGWIDGET_H