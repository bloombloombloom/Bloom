#pragma once

#include <QUiLoader>
#include <QSize>

namespace Bloom
{
    class UiLoader: public QUiLoader
    {
    Q_OBJECT
    private:
        std::map<QString, std::function<QWidget*(QWidget* parent, const QString& name)>> customWidgetConstructorsByWidgetName = {};

    public:
        explicit UiLoader(QObject* parent);

        QWidget* createWidget(const QString& className, QWidget* parent, const QString& name) override;
    };
}
