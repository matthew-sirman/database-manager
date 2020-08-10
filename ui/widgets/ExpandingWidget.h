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

class ExpandingWidget : public QWidget {
	Q_OBJECT
public:
	explicit ExpandingWidget(const QString &title = "", unsigned expandTime = 200, QWidget *parent = nullptr);

	~ExpandingWidget() override;

	void setContent(QLayout *layout);

	void expand();

	void collapse();

private:
	QGridLayout *mainLayout = nullptr;
	QToolButton *expandButton = nullptr;
	QFrame *headerFrame = nullptr;
	QScrollArea *contentArea = nullptr;
	unsigned expandTime = 200;
};

#endif //DATABASE_MANAGER_EXPANDINGWIDGET_H