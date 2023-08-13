#include "AboutWindow.hpp"


#include "UiLoader.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

#include "src/Exceptions/Exception.hpp"
#include "src/Services/PathService.hpp"
#include "src/Application.hpp"

using namespace Exceptions;

AboutWindow::AboutWindow(QWidget* parent): QObject(parent) {
    auto aboutWindowUiFile = QFile(QString::fromStdString(
            Services::PathService::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/UiFiles/AboutWindow.ui"
        )
    );
    auto aboutWindowStylesheet = QFile(QString::fromStdString(
            Services::PathService::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Stylesheets/AboutWindow.qss"
        )
    );

    if (!aboutWindowUiFile.open(QFile::ReadOnly)) {
        throw Exception("Failed to open AboutWindow UI file");
    }

    if (!aboutWindowStylesheet.open(QFile::ReadOnly)) {
        throw Exception("Failed to open AboutWindow QSS file");
    }

    auto uiLoader = UiLoader(this);
    this->windowWidget = uiLoader.load(&aboutWindowUiFile, parent);
    this->windowWidget->setStyleSheet(aboutWindowStylesheet.readAll());
    this->windowWidget->setFixedSize(400, 300);

    auto versionLabel = this->windowWidget->findChild<Widgets::Label*>("version-label");

    if (versionLabel != nullptr) {
        versionLabel->setText("Bloom v" + QString::fromStdString(Application::VERSION.toString()));
    }
}
