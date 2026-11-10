#include "mainwindow.h"

#include "style/constants.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSize>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStyle>
#include <QTabWidget>
#include <QTableWidget>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {

using namespace Ui::Tokens;

QString navigationButtonStyle(bool collapsed)
{
    const QString padding = collapsed ? QStringLiteral("%1px %2px").arg(space3).arg(space1)
                                      : QStringLiteral("%1px %2px").arg(space3).arg(space3);
    const QString alignment = collapsed ? QStringLiteral("center") : QStringLiteral("left");
    return QStringLiteral(
               "QPushButton { color: %1; background: transparent; border: 2px solid transparent; "
               "border-radius: %2px; padding: %3; text-align: %4; font-size: %5px; }"
               "QPushButton:hover { background: %6; color: %7; }"
               "QPushButton:pressed { background: %8; color: %9; }"
               "QPushButton:checked { background: %10; color: %11; font-weight: %12; }"
               "QPushButton:focus { border-color: %13; }")
        .arg(sidebarText)
        .arg(radiusSmall)
        .arg(padding)
        .arg(alignment)
        .arg(textBody)
        .arg(sidebarHover)
        .arg(surface)
        .arg(surface)
        .arg(ink)
        .arg(surface)
        .arg(ink)
        .arg(weightSemibold)
        .arg(focus);
}

QString cardStyle()
{
    return QStringLiteral("QFrame { background: %1; border: 1px solid %2; border-radius: %3px; }"
                          "QLabel { border: none; }")
        .arg(surface)
        .arg(border)
        .arg(radiusMedium);
}

QFrame *createCard(const QString &title, const QString &description, QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setFrameShape(QFrame::StyledPanel);
    card->setStyleSheet(cardStyle());

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(space5, space5, space5, space5);
    layout->setSpacing(space2);

    auto *cardTitle = new QLabel(title, card);
    cardTitle->setStyleSheet(
        QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textTitle).arg(weightSemibold).arg(ink));
    auto *cardDescription = new QLabel(description, card);
    cardDescription->setWordWrap(true);
    cardDescription->setStyleSheet(
        QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));

    layout->addWidget(cardTitle);
    layout->addWidget(cardDescription);
    return card;
}

QLabel *createSectionHeading(const QString &text, QWidget *parent)
{
    auto *heading = new QLabel(text, parent);
    heading->setStyleSheet(
        QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textTitle).arg(weightSemibold).arg(ink));
    return heading;
}

QFrame *createMetricCard(const QString &label, const QString &value, const QString &detail, QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setStyleSheet(cardStyle());
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(space4, space4, space4, space4);
    layout->setSpacing(space1);

    auto *metricLabel = new QLabel(label, card);
    metricLabel->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textSmall).arg(mutedText));
    auto *metricValue = new QLabel(value, card);
    metricValue->setStyleSheet(
        QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textHeading).arg(weightBold).arg(ink));
    auto *metricDetail = new QLabel(detail, card);
    metricDetail->setStyleSheet(
        QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textSmall).arg(weightMedium).arg(success));
    layout->addWidget(metricLabel);
    layout->addWidget(metricValue);
    layout->addWidget(metricDetail);
    return card;
}

QString showcaseControlStyle()
{
    return QStringLiteral(
               "QLineEdit, QComboBox, QSpinBox, QDateEdit { background: %1; color: %2; border: 1px solid %3; "
               "border-radius: %4px; min-height: %5px; padding: 0 %6px; font-size: %7px; }"
               "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDateEdit:focus { border: 2px solid %8; }"
               "QCheckBox, QRadioButton { color: %2; font-size: %7px; spacing: %6px; }"
               "QCheckBox:focus, QRadioButton:focus { outline: 2px solid %8; }"
               "QSlider::groove:horizontal { height: %9px; background: %10; border-radius: %9px; }"
               "QSlider::handle:horizontal { width: %11px; margin: -%12px 0; background: %2; border-radius: %13px; }"
               "QProgressBar { color: %2; background: %10; border: 1px solid %3; border-radius: %4px; text-align: center; }"
               "QProgressBar::chunk { background: %14; border-radius: %4px; }")
        .arg(surface)
        .arg(ink)
        .arg(border)
        .arg(radiusSmall)
        .arg(controlHeight)
        .arg(space2)
        .arg(textBody)
        .arg(focus)
        .arg(space1)
        .arg(surfaceMuted)
        .arg(space4)
        .arg(space1)
        .arg(radiusMedium)
        .arg(success);
}

QWidget *createDashboardPage(QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setStyleSheet(QStringLiteral("background: %1;").arg(canvas));
    auto *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(QStringLiteral("QScrollArea { background: %1; }").arg(canvas));
    auto *content = new QWidget(scrollArea);
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(space8, space6, space8, space8);
    layout->setSpacing(space5);

    auto *intro = new QLabel("A static reference surface for Café Ops controls, data, and feedback.", content);
    intro->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));
    layout->addWidget(intro);

    auto *metrics = new QGridLayout;
    metrics->setHorizontalSpacing(space3);
    metrics->setVerticalSpacing(space3);
    metrics->addWidget(createMetricCard("Beans on hand", "48.2 kg", "Healthy stock", content), 0, 0);
    metrics->addWidget(createMetricCard("Milk cartons", "36", "Delivery today", content), 0, 1);
    metrics->addWidget(createMetricCard("Open waste", "2 items", "Review needed", content), 0, 2);
    metrics->addWidget(createMetricCard("Today’s orders", "126", "+8% vs. Tuesday", content), 0, 3);
    layout->addLayout(metrics);

    layout->addWidget(createSectionHeading("Actions & inputs", content));
    auto *controlsCard = new QFrame(content);
    controlsCard->setStyleSheet(cardStyle());
    auto *controlsLayout = new QGridLayout(controlsCard);
    controlsLayout->setContentsMargins(space5, space5, space5, space5);
    controlsLayout->setHorizontalSpacing(space4);
    controlsLayout->setVerticalSpacing(space4);

    auto *search = new QLineEdit(controlsCard);
    search->setPlaceholderText("Search ingredients or lots");
    search->setClearButtonEnabled(true);
    auto *location = new QComboBox(controlsCard);
    location->addItems({"Main bar", "Cold storage", "Dry goods"});
    auto *reorderPoint = new QSpinBox(controlsCard);
    reorderPoint->setRange(0, 100);
    reorderPoint->setValue(12);
    reorderPoint->setSuffix(" units");
    auto *deliveryDate = new QDateEdit(QDate(2025, 2, 14), controlsCard);
    deliveryDate->setCalendarPopup(true);
    auto *stockFilter = new QCheckBox("Show low-stock items", controlsCard);
    stockFilter->setChecked(true);
    auto *allStock = new QRadioButton("All inventory", controlsCard);
    auto *availableStock = new QRadioButton("Available only", controlsCard);
    allStock->setChecked(true);
    auto *buffer = new QSlider(Qt::Horizontal, controlsCard);
    buffer->setRange(0, 100);
    buffer->setValue(68);
    auto *coverage = new QProgressBar(controlsCard);
    coverage->setRange(0, 100);
    coverage->setValue(74);
    coverage->setFormat("Stock coverage 74%");

    auto *primary = new QPushButton("Create purchase order", controlsCard);
    auto *secondary = new QPushButton("Export snapshot", controlsCard);
    auto *dangerButton = new QPushButton("Flag waste", controlsCard);
    auto *moreActions = new QToolButton(controlsCard);
    moreActions->setIcon(controlsCard->style()->standardIcon(QStyle::SP_TitleBarMenuButton));
    moreActions->setToolTip("More static actions");
    moreActions->setFixedSize(controlHeight, controlHeight);
    for (auto *button : {primary, secondary, dangerButton}) {
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumHeight(controlHeight);
    }
    primary->setStyleSheet(QStringLiteral("QPushButton { background: %1; color: %2; border: 1px solid %1; border-radius: %3px; padding: 0 %4px; font-weight: %5; } QPushButton:hover { background: %6; } QPushButton:focus { border: 2px solid %7; }").arg(ink).arg(surface).arg(radiusSmall).arg(space3).arg(weightSemibold).arg(sidebarHover).arg(focus));
    secondary->setStyleSheet(QStringLiteral("QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: %4px; padding: 0 %5px; font-weight: %6; } QPushButton:hover { background: %7; } QPushButton:focus { border: 2px solid %8; }").arg(surface).arg(ink).arg(border).arg(radiusSmall).arg(space3).arg(weightSemibold).arg(surfaceMuted).arg(focus));
    dangerButton->setStyleSheet(QStringLiteral("QPushButton { background: %1; color: %2; border: 1px solid %1; border-radius: %3px; padding: 0 %4px; font-weight: %5; } QPushButton:hover { background: %6; } QPushButton:focus { border: 2px solid %7; }").arg(danger).arg(surface).arg(radiusSmall).arg(space3).arg(weightSemibold).arg(QStringLiteral("#8f1d14")).arg(focus));
    moreActions->setStyleSheet(QStringLiteral("QToolButton { background: %1; border: 1px solid %2; border-radius: %3px; } QToolButton:hover { background: %4; } QToolButton:focus { border: 2px solid %5; }").arg(surface).arg(border).arg(radiusSmall).arg(surfaceMuted).arg(focus));

    controlsCard->setStyleSheet(cardStyle() + showcaseControlStyle());
    controlsLayout->addWidget(search, 0, 0, 1, 2);
    controlsLayout->addWidget(location, 0, 2);
    controlsLayout->addWidget(reorderPoint, 0, 3);
    controlsLayout->addWidget(deliveryDate, 1, 0);
    controlsLayout->addWidget(stockFilter, 1, 1);
    controlsLayout->addWidget(allStock, 1, 2);
    controlsLayout->addWidget(availableStock, 1, 3);
    controlsLayout->addWidget(buffer, 2, 0, 1, 2);
    controlsLayout->addWidget(coverage, 2, 2, 1, 2);
    controlsLayout->addWidget(primary, 3, 0);
    controlsLayout->addWidget(secondary, 3, 1);
    controlsLayout->addWidget(dangerButton, 3, 2);
    controlsLayout->addWidget(moreActions, 3, 3, Qt::AlignRight);
    layout->addWidget(controlsCard);

    layout->addWidget(createSectionHeading("Inventory views", content));
    auto *tabs = new QTabWidget(content);
    tabs->setStyleSheet(QStringLiteral("QTabWidget::pane { border: 1px solid %1; background: %2; } QTabBar::tab { background: %3; color: %4; padding: %5px %6px; border: 1px solid %1; } QTabBar::tab:selected { background: %2; font-weight: %7; } QTabBar::tab:focus { border: 2px solid %8; }").arg(border).arg(surface).arg(surfaceMuted).arg(ink).arg(space2).arg(space3).arg(weightSemibold).arg(focus));
    auto *table = new QTableWidget(3, 4, tabs);
    table->setHorizontalHeaderLabels({"Ingredient", "On hand", "Par level", "Status"});
    const QList<QStringList> rows = {{"Espresso beans", "48.2 kg", "35 kg", "Ready"}, {"Oat milk", "8 cartons", "12 cartons", "Low"}, {"Vanilla syrup", "14 bottles", "10 bottles", "Ready"}};
    for (int row = 0; row < rows.size(); ++row) {
        for (int column = 0; column < rows.at(row).size(); ++column) {
            table->setItem(row, column, new QTableWidgetItem(rows.at(row).at(column)));
        }
    }
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    auto *list = new QListWidget(tabs);
    list->addItems({"Morning roast delivery — 08:30", "Oat milk count — 10:00", "Cold brew batch — 13:00"});
    auto *tree = new QTreeWidget(tabs);
    tree->setHeaderLabels({"Storage area", "Items"});
    auto *bar = new QTreeWidgetItem(tree, {"Main bar", "18"});
    new QTreeWidgetItem(bar, {"Espresso beans", "48.2 kg"});
    new QTreeWidgetItem(bar, {"Paper cups", "320"});
    auto *cold = new QTreeWidgetItem(tree, {"Cold storage", "11"});
    new QTreeWidgetItem(cold, {"Oat milk", "8 cartons"});
    tree->expandAll();
    tabs->addTab(table, "Stock table");
    tabs->addTab(list, "Prep list");
    tabs->addTab(tree, "Storage tree");
    layout->addWidget(tabs);

    auto *feedbackLayout = new QHBoxLayout;
    auto *statusGroup = new QGroupBox("Status & feedback", content);
    statusGroup->setStyleSheet(QStringLiteral("QGroupBox { color: %1; font-weight: %2; border: 1px solid %3; border-radius: %4px; margin-top: %5px; padding: %6px; } QGroupBox::title { subcontrol-origin: margin; left: %6px; padding: 0 %7px; }").arg(ink).arg(weightSemibold).arg(border).arg(radiusMedium).arg(space2).arg(space4).arg(space1));
    auto *statusLayout = new QVBoxLayout(statusGroup);
    auto *successLabel = new QLabel("Inventory sync is represented as current", statusGroup);
    successLabel->setStyleSheet(QStringLiteral("color: %1; background: %2; border-radius: %3px; padding: %4px; font-size: %5px;").arg(success).arg(successMuted).arg(radiusSmall).arg(space2).arg(textBody));
    auto *dangerLabel = new QLabel("Oat milk is below its static par level", statusGroup);
    dangerLabel->setStyleSheet(QStringLiteral("color: %1; background: %2; border-radius: %3px; padding: %4px; font-size: %5px;").arg(danger).arg(dangerMuted).arg(radiusSmall).arg(space2).arg(textBody));
    statusLayout->addWidget(successLabel);
    statusLayout->addWidget(dangerLabel);
    feedbackLayout->addWidget(statusGroup, 1);
    feedbackLayout->addWidget(createCard("Static showcase", "These widgets demonstrate presentation only; no café operations are changed.", content), 1);
    layout->addLayout(feedbackLayout);
    layout->addStretch();

    scrollArea->setWidget(content);
    pageLayout->addWidget(scrollArea);
    return page;
}

QWidget *createPage(const QString &title, const QString &description, QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setStyleSheet(QStringLiteral("background: %1;").arg(canvas));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(space8, space6, space8, space8);
    layout->setSpacing(space4);

    auto *heading = new QLabel(title, page);
    heading->setStyleSheet(
        QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textHeading).arg(weightBold).arg(ink));
    auto *intro = new QLabel(description, page);
    intro->setWordWrap(true);
    intro->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));

    layout->addWidget(heading);
    layout->addWidget(intro);
    layout->addWidget(createCard("Workspace placeholder", "Static content for this module will appear here in a future iteration.", page));
    layout->addStretch();
    return page;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Café Operations");
    setMinimumSize(1024, 680);
    resize(1280, 800);

    auto *central = new QWidget(this);
    central->setStyleSheet(QStringLiteral("background: %1; font-family: Sans Serif;").arg(canvas));
    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *sidebar = new QFrame(central);
    sidebar->setFixedWidth(sidebarWidth);
    sidebar->setStyleSheet(QStringLiteral("QFrame { background: %1; }").arg(Ui::Tokens::sidebar));
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(space3, space5, space3, space4);
    sidebarLayout->setSpacing(space2);

    auto *sidebarHeader = new QHBoxLayout;
    sidebarHeader->setContentsMargins(space1, 0, space1, space3);
    auto *brandIcon = new QLabel(sidebar);
    brandIcon->setPixmap(QIcon(QStringLiteral(":/assets/cafe-ops.svg")).pixmap(QSize(iconLarge, iconLarge)));
    brandIcon->setFixedSize(iconLarge, iconLarge);
    auto *brand = new QLabel("CAFÉ OPS", sidebar);
    brand->setStyleSheet(QStringLiteral("color: %1; font-size: %2px; font-weight: %3; letter-spacing: 0.5px;").arg(surface).arg(textTitle).arg(weightBold));

    const QIcon expandedToggleIcon(QStringLiteral(":/assets/panel-left.svg"));
    const QIcon collapsedToggleIcon(QStringLiteral(":/assets/panel-right.svg"));
    auto *toggleButton = new QToolButton(sidebar);
    toggleButton->setIcon(expandedToggleIcon);
    toggleButton->setIconSize(QSize(iconMedium, iconMedium));
    toggleButton->setToolTip("Collapse navigation");
    toggleButton->setCursor(Qt::PointingHandCursor);
    toggleButton->setFixedSize(compactControlHeight, compactControlHeight);
    toggleButton->setStyleSheet(QStringLiteral("QToolButton { color: %1; background: transparent; border: 1px solid %2; border-radius: %3px; } QToolButton:hover { color: %4; background: %2; border-color: %1; } QToolButton:pressed { background: %4; } QToolButton:focus { border: 2px solid %5; }").arg(sidebarText).arg(sidebarHover).arg(radiusSmall).arg(surface).arg(focus));
    sidebarHeader->addWidget(brandIcon);
    sidebarHeader->addWidget(brand);
    sidebarHeader->addStretch();
    sidebarHeader->addWidget(toggleButton);
    sidebarLayout->addLayout(sidebarHeader);

    auto *buttonGroup = new QButtonGroup(sidebar);
    buttonGroup->setExclusive(true);

    auto *content = new QWidget(central);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    auto *header = new QFrame(content);
    header->setFixedHeight(headerHeight);
    header->setStyleSheet(QStringLiteral("QFrame { background: %1; border-bottom: 1px solid %2; }").arg(surface).arg(border));
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(space8, 0, space8, 0);
    auto *headerTitle = new QLabel(header);
    headerTitle->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textTitle).arg(weightSemibold).arg(ink));
    auto *headerStatus = new QLabel("Static desktop interface", header);
    headerStatus->setStyleSheet(QStringLiteral("color: %1; background: %2; border: 1px solid %1; border-radius: %3px; padding: %4px %5px; font-size: %6px; font-weight: %7;").arg(success).arg(surface).arg(radiusSmall).arg(space1).arg(space2).arg(textSmall).arg(weightSemibold));
    headerLayout->addWidget(headerTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(headerStatus);

    auto *pages = new QStackedWidget(content);
    const QList<QPair<QString, QString>> modules = {
        {"Dashboard", "A static overview of café operations and upcoming activity."},
        {"Sales", "Review sales activity and daily performance at a glance."},
        {"Inventory & Lots", "Browse inventory and lot information from one workspace."},
        {"Lot Receipts", "Track incoming lots and receiving activity."},
        {"Waste", "Review waste records and operational placeholders."},
        {"Recipes", "Organize recipe definitions and ingredient references."},
        {"Alerts", "View operational alerts requiring attention."},
        {"Purchase Orders", "Review purchasing activity and order placeholders."},
        {"Reports", "Access reporting summaries and exports."},
        {"Users & Roles", "Manage user access and role placeholders."},
    };
    const QStringList iconResources = {
        ":/assets/dashboard.svg", ":/assets/sales.svg", ":/assets/inventory.svg", ":/assets/receipts.svg", ":/assets/waste.svg",
        ":/assets/recipes.svg", ":/assets/alerts.svg", ":/assets/purchase-orders.svg", ":/assets/reports.svg", ":/assets/users.svg",
    };
    QList<QPushButton *> navigationButtons;

    for (int index = 0; index < modules.size(); ++index) {
        const auto &module = modules.at(index);
        pages->addWidget(index == 0 ? createDashboardPage(pages) : createPage(module.first, module.second, pages));

        auto *button = new QPushButton(QIcon(iconResources.at(index)), module.first, sidebar);
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setIconSize(QSize(iconMedium, iconMedium));
        button->setStyleSheet(navigationButtonStyle(false));
        buttonGroup->addButton(button, index);
        navigationButtons.append(button);
        sidebarLayout->addWidget(button);
        connect(button, &QPushButton::clicked, pages, [pages, index] { pages->setCurrentIndex(index); });
    }

    sidebarLayout->addStretch();
    auto *sidebarFooter = new QLabel("Desktop workspace", sidebar);
    sidebarFooter->setStyleSheet(QStringLiteral("color: %1; font-size: %2px; padding: %3px;").arg(sidebarText).arg(textSmall).arg(space2));
    sidebarLayout->addWidget(sidebarFooter);

    auto setSidebarCollapsed = [sidebar, brandIcon, brand, sidebarFooter, toggleButton, expandedToggleIcon, collapsedToggleIcon, navigationButtons, modules](bool collapsed) {
        sidebar->setFixedWidth(collapsed ? sidebarCollapsedWidth : sidebarWidth);
        brandIcon->setVisible(true);
        brand->setVisible(!collapsed);
        brand->setText("CAFÉ OPS");
        sidebarFooter->setVisible(!collapsed);
        toggleButton->setIcon(collapsed ? collapsedToggleIcon : expandedToggleIcon);
        toggleButton->setToolTip(collapsed ? "Expand navigation" : "Collapse navigation");
        for (int index = 0; index < navigationButtons.size(); ++index) {
            auto *button = navigationButtons.at(index);
            button->setText(collapsed ? QString() : modules.at(index).first);
            button->setToolTip(collapsed ? modules.at(index).first : QString());
            button->setStyleSheet(navigationButtonStyle(collapsed));
        }
    };

    connect(toggleButton, &QToolButton::clicked, sidebar, [setSidebarCollapsed, sidebar] { setSidebarCollapsed(sidebar->width() > 100); });
    connect(pages, &QStackedWidget::currentChanged, headerTitle, [headerTitle, modules](int index) { headerTitle->setText(modules.at(index).first); });

    buttonGroup->button(0)->setChecked(true);
    pages->setCurrentIndex(0);
    headerTitle->setText(modules.first().first);

    contentLayout->addWidget(header);
    contentLayout->addWidget(pages, 1);
    rootLayout->addWidget(sidebar);
    rootLayout->addWidget(content, 1);
    setCentralWidget(central);
}
