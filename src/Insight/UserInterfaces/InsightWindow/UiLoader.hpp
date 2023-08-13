#pragma once

#include <QUiLoader>
#include <QSize>

class UiLoader: public QUiLoader
{
    Q_OBJECT

public:
    explicit UiLoader(QObject* parent);

    QWidget* createWidget(const QString& className, QWidget* parent, const QString& name) override;

private:
    std::map<
        QString,
        std::function<QWidget*(QWidget* parent, const QString& name)>
    > customWidgetConstructorsByWidgetName = {};
};
