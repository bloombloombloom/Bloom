#include "RegisterGroupWidget.hpp"

#include <QUiLoader>
#include <QStyle>
#include <utility>

#include "RegisterWidget.hpp"

#include "src/Services/PathService.hpp"

namespace Bloom::Widgets
{
    using Bloom::Targets::TargetRegisterDescriptor;

    RegisterGroupWidget::RegisterGroupWidget(
        QString name,
        const std::set<TargetRegisterDescriptor>& registerDescriptors,
        TargetRegistersPaneWidget* parent
    )
        : ItemWidget(parent)
        , name(std::move(name))
    {
        this->setObjectName(this->name);

        this->headerWidget->setObjectName("register-group-header");
        this->headerWidget->setFixedHeight(25);
        auto* headerLayout = new QHBoxLayout(this->headerWidget);
        headerLayout->setContentsMargins(5, 0, 0, 0);

        this->label->setText(this->name);

        this->arrowIcon->setObjectName("arrow-icon");
        auto static arrowIconPath = QString::fromStdString(
            Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/arrow.svg"
        );
        this->arrowIcon->setSvgFilePath(arrowIconPath);
        this->arrowIcon->setContainerHeight(15);
        this->arrowIcon->setContainerWidth(14);

        this->registerGroupIcon->setObjectName("register-group-icon");
        auto static registerIconPath = QString::fromStdString(
            Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/register-group.svg"
        );
        this->registerGroupIcon->setSvgFilePath(registerIconPath);
        this->registerGroupIcon->setContainerHeight(15);
        this->registerGroupIcon->setContainerWidth(15);

        headerLayout->addWidget(this->arrowIcon);
        headerLayout->addWidget(this->registerGroupIcon);
        headerLayout->addWidget(this->label);

        auto* bodyLayout = new QVBoxLayout(this->bodyWidget);
        bodyLayout->setContentsMargins(0, 0,0,0);
        bodyLayout->setSpacing(0);

        for (const auto& descriptor : registerDescriptors) {
            if (!descriptor.name.has_value()) {
                continue;
            }

            if (!descriptor.readable) {
                continue;
            }

            auto* registerWidget = new RegisterWidget(descriptor, this->bodyWidget);
            bodyLayout->addWidget(registerWidget, 0, Qt::AlignmentFlag::AlignTop);

            QObject::connect(
                registerWidget,
                &ItemWidget::selected,
                parent,
                &TargetRegistersPaneWidget::onItemSelectionChange
            );

            this->registerWidgets.insert(registerWidget);
            this->registerWidgetsMappedByDescriptor.insert(std::pair(descriptor, registerWidget));
        }

        bodyLayout->addStretch(1);

        this->layout->setContentsMargins(0,0,0,0);
        this->layout->setSpacing(0);
        this->layout->addWidget(this->headerWidget, 0, Qt::AlignmentFlag::AlignTop);
        this->layout->addWidget(this->bodyWidget, 0, Qt::AlignmentFlag::AlignTop);
        this->layout->addStretch(1);

        this->collapse();

        QObject::connect(this->headerWidget, &ClickableWidget::doubleClicked, [this] {
            if (this->collapsed) {
                this->expand();

            } else {
                this->collapse();
            }
        });

        QObject::connect(
            this->headerWidget,
            &ItemWidget::selected,
            parent,
            &TargetRegistersPaneWidget::onItemSelectionChange
        );
    }

    void RegisterGroupWidget::collapse() {
        this->arrowIcon->setAngle(0);
        this->bodyWidget->setVisible(false);
        this->collapsed = true;
    }

    void RegisterGroupWidget::expand() {
        this->arrowIcon->setAngle(90);
        this->bodyWidget->setVisible(true);
        this->collapsed = false;
    }

    void RegisterGroupWidget::setAllRegistersVisible(bool visible) {
        for (const auto& registerWidget : this->registerWidgets) {
            registerWidget->setVisible(visible);
        }
    }

    void RegisterGroupWidget::filterRegisters(const QString& keyword) {
        int matchingWidgetCount = 0;
        for (const auto& registerWidget : this->registerWidgets) {
            if (keyword.isEmpty() || (registerWidget->searchKeywords.contains(keyword, Qt::CaseInsensitive))) {
                matchingWidgetCount++;
                registerWidget->setVisible(true);

            } else {
                registerWidget->setVisible(false);
            }
        }

        if (matchingWidgetCount == 0) {
            this->collapse();
            this->setVisible(false);

        } else {
            this->setVisible(true);
            if (!keyword.isEmpty()) {
                this->expand();

            } else {
                this->collapse();
            }
        }
    }
}
