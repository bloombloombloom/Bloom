#include "CreateSnapshotWindow.hpp"

#include <QFile>
#include <QSize>
#include <QDesktopServices>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

#include "src/Insight/InsightSignals.hpp"

#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::Widgets
{
    using Bloom::Exceptions::Exception;

    CreateSnapshotWindow::CreateSnapshotWindow(
        Targets::TargetMemoryType memoryType,
        const std::optional<Targets::TargetMemoryBuffer>& data,
        const bool& staleData,
        QWidget* parent
    )
        : QWidget(parent)
        , data(data)
        , staleData(staleData)
    {
        this->setWindowFlag(Qt::Window);
        this->setObjectName("create-snapshot-window");
        this->setWindowTitle(
            "New Snapshot - " + QString(memoryType == Targets::TargetMemoryType::EEPROM ? "EEPROM" : "RAM")
        );

        auto windowUiFile = QFile(
            QString::fromStdString(Paths::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane"
                + "/SnapshotManager/CreateSnapshotWindow/UiFiles/CreateSnapshotWindow.ui"
            )
        );

        if (!windowUiFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open CreateSnapshotWindow UI file");
        }

        this->setFixedSize(QSize(500, 300));

        auto uiLoader = UiLoader(this);
        this->container = uiLoader.load(&windowUiFile, this);

        this->container->setFixedSize(this->size());
        this->container->setContentsMargins(15, 15, 15, 15);

        auto* formContainer = this->container->findChild<QWidget*>("form-container");

        this->nameInput = formContainer->findChild<TextInput*>("name-input");
        this->descriptionInput = formContainer->findChild<QPlainTextEdit*>("description-input");
        this->includeFocusedRegionsInput = formContainer->findChild<QCheckBox*>("include-focus-regions-input");
        this->captureDirectlyFromTargetInput = formContainer->findChild<QCheckBox*>("capture-directly-from-target-input");

        this->staleDataWarning = formContainer->findChild<QWidget*>("stale-data-warning");

        this->captureButton = this->container->findChild<PushButton*>("capture-btn");
        this->closeButton = this->container->findChild<PushButton*>("close-btn");

        QObject::connect(this->nameInput, &QLineEdit::textEdited, this, &CreateSnapshotWindow::refreshForm);
        QObject::connect(
            this->captureDirectlyFromTargetInput,
            &QCheckBox::stateChanged,
            this,
            &CreateSnapshotWindow::refreshForm
        );

        QObject::connect(this->captureButton, &QPushButton::clicked, this, &CreateSnapshotWindow::issueCaptureRequest);
        QObject::connect(this->closeButton, &QPushButton::clicked, this, &QWidget::close);

        auto* insightSignals = InsightSignals::instance();

        QObject::connect(
            insightSignals,
            &InsightSignals::targetStateUpdated,
            this,
            [this] (Targets::TargetState newState) {
                this->targetState = newState;
                this->refreshForm();
            }
        );
    }

    void CreateSnapshotWindow::refreshForm() {
        this->captureButton->setEnabled(this->captureEnabled());
        this->staleDataWarning->setVisible(this->staleData && !this->captureDirectlyFromTargetInput->isChecked());
    }

    void CreateSnapshotWindow::showEvent(QShowEvent* event) {
        this->move(this->parentWidget()->window()->geometry().center() - this->rect().center());
        this->resetForm();
        this->refreshForm();
        QWidget::showEvent(event);
    }

    bool CreateSnapshotWindow::captureEnabled() {
        if (this->targetState != Targets::TargetState::STOPPED) {
            return false;
        }

        if (!this->data.has_value()) {
            return false;
        }

        if (this->nameInput->text().isEmpty()) {
            return false;
        }

        return true;
    }

    void CreateSnapshotWindow::resetForm() {
        this->nameInput->setText("Untitled Snapshot");
        this->descriptionInput->setPlainText("");
        this->includeFocusedRegionsInput->setChecked(true);
        this->captureDirectlyFromTargetInput->setChecked(false);
    }

    void CreateSnapshotWindow::issueCaptureRequest() {
        if (!this->captureEnabled()) {
            // Sanity check
            this->refreshForm();
            return;
        }

        emit this->snapshotCaptureRequested(
            this->nameInput->text(),
            this->descriptionInput->toPlainText(),
            this->includeFocusedRegionsInput->isChecked(),
            this->captureDirectlyFromTargetInput->isChecked()
        );

        this->close();
    }
}
