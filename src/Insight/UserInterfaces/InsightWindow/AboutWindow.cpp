#include <QtUiTools>

#include "AboutWindow.hpp"
#include "TargetWidgets/DIP/DualInlinePackageWidget.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Application.hpp"

using namespace Bloom;
using namespace InsightTargetWidgets;
using namespace Exceptions;

AboutWindow::AboutWindow(QWidget* parent): QObject(parent) {
    auto aboutWindowUiFile = QFile(":/compiled/Insight/UserInterfaces/InsightWindow/UiFiles/AboutWindow.ui");
    auto aboutWindowStylesheet = QFile(":/compiled/Insight/UserInterfaces/InsightWindow/Stylesheets/AboutWindow.qss");

    if (!aboutWindowUiFile.open(QFile::ReadOnly)) {
        throw Exception("Failed to open AboutWindow UI file");
    }

    if (!aboutWindowStylesheet.open(QFile::ReadOnly)) {
        throw Exception("Failed to open AboutWindow QSS file");
    }

    auto uiLoader = QUiLoader();
    this->windowWidget = uiLoader.load(&aboutWindowUiFile, parent);
    this->windowWidget->setStyleSheet(aboutWindowStylesheet.readAll());
    this->windowWidget->setFixedSize(400, 300);

    auto versionLabel = this->windowWidget->findChild<QLabel*>("version-label");

    if (versionLabel != nullptr) {
        versionLabel->setText("Bloom v" + QString::fromStdString(Application::VERSION_STR));
    }
}
