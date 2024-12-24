#include "ChangeListPane.hpp"

#include <QFile>
#include <QVBoxLayout>
#include <algorithm>
#include <QScrollBar>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Widgets
{
    using Exceptions::Exception;

    ChangeListPane::ChangeListPane(
        DifferentialHexViewerWidget* hexViewerWidgetA,
        DifferentialHexViewerWidget* hexViewerWidgetB,
        PaneState& state,
        PanelWidget* parent
    )
        : PaneWidget(state, parent)
        , hexViewerWidgetA(hexViewerWidgetA)
        , hexViewerWidgetB(hexViewerWidgetB)
    {
        this->setObjectName("change-list-pane");
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        auto widgetUiFile = QFile{
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager"
                    + "/SnapshotDiff/ChangeListPane/UiFiles/ChangeListPane.ui"
            )
        };

        if (!widgetUiFile.open(QFile::ReadOnly)) {
            throw Exception{"Failed to open ChangeListPane UI file"};
        }

        auto uiLoader = UiLoader{this};
        this->container = uiLoader.load(&widgetUiFile, this);

        this->container->setFixedSize(this->size());
        this->container->setContentsMargins(0, 0, 0, 0);

        this->placeHolderLabel = this->container->findChild<Label*>("placeholder-label");

        auto* containerLayout = this->container->findChild<QVBoxLayout*>();

        this->changeListView = new ListView{{}, this};
        this->changeListView->viewport()->installEventFilter(parent);
        this->changeListView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);

        this->changeListScene = this->changeListView->listScene();
        this->changeListScene->setSelectionLimit(1);

        containerLayout->addWidget(this->changeListView);

        QObject::connect(
            this->changeListView->verticalScrollBar(),
            &QScrollBar::rangeChanged,
            this,
            &ChangeListPane::refreshChangeListViewSize
        );

        QObject::connect(
            this->changeListScene,
            &ListScene::selectionChanged,
            this,
            &ChangeListPane::onItemSelectionChanged
        );

        QObject::connect(
            this->changeListScene,
            &ListScene::itemContextMenu,
            this,
            &ChangeListPane::onItemContextMenu
        );

        QObject::connect(
            this->selectBytesAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->selectedChangeListItem == nullptr) {
                    return;
                }

                const auto addresses = this->selectedChangeListItem->addressRange.addresses();
                this->hexViewerWidgetA->selectByteItems(addresses);
                this->hexViewerWidgetB->selectByteItems(addresses);
            }
        );

        QObject::connect(
            this->restoreBytesAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->selectedChangeListItem == nullptr) {
                    return;
                }

                emit this->restoreBytesRequested(this->selectedChangeListItem->addressRange.addresses());
            }
        );

        this->show();
    }

    void ChangeListPane::setDiffRanges(const std::vector<Targets::TargetMemoryAddressRange>& diffRanges) {
        this->changeListScene->clearListItems();

        for (const auto& diffRange : diffRanges) {
            this->changeListScene->addListItem(new ChangeListItem{diffRange});
        }

        this->changeListScene->refreshGeometry();

        this->changeListView->setVisible(!diffRanges.empty());
        this->placeHolderLabel->setVisible(diffRanges.empty());

        // Trigger a resize event
        this->resize(this->size());
    }

    void ChangeListPane::setRestoreEnabled(bool restoreEnabled) {
        this->restoreEnabled = restoreEnabled;
    }

    void ChangeListPane::resizeEvent(QResizeEvent* event) {
        const auto size = this->size();
        this->container->setFixedSize(size.width() - 1, size.height());

        this->refreshChangeListViewSize();

        PaneWidget::resizeEvent(event);
    }

    void ChangeListPane::showEvent(QShowEvent* event) {
        this->refreshChangeListViewSize();
        PaneWidget::showEvent(event);
    }

    void ChangeListPane::onItemSelectionChanged(const std::list<ListItem*>& selectedItems) {
        if (selectedItems.size() < 1) {
            return;
        }

        const auto* item = dynamic_cast<ChangeListItem*>(selectedItems.front());
        assert(item != nullptr);

        this->selectedChangeListItem = item;

        this->hexViewerWidgetA->highlightPrimaryByteItemRanges({item->addressRange});
        this->hexViewerWidgetA->centerOnByte(item->addressRange.startAddress);

        this->hexViewerWidgetB->highlightPrimaryByteItemRanges({item->addressRange});
        this->hexViewerWidgetB->centerOnByte(item->addressRange.startAddress);
    }

    void ChangeListPane::onItemContextMenu(ListItem *item, QPoint sourcePosition) {
        auto* changeListItem = dynamic_cast<ChangeListItem*>(item);

        if (changeListItem == nullptr) {
            return;
        }

        auto* menu = new QMenu{this};

        menu->addAction(this->selectBytesAction);
        menu->addSeparator();
        menu->addAction(this->restoreBytesAction);
        this->restoreBytesAction->setEnabled(this->restoreEnabled);

        menu->exec(sourcePosition);
    }

    void ChangeListPane::refreshChangeListViewSize() {
        this->changeListView->setFixedWidth(
            this->container->width() - (
                this->changeListView->verticalScrollBar()->maximum() > 0 ? this->parentPanel->getHandleSize() : 0
            )
        );
    }
}
