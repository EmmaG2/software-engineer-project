#include "mainwindow.h"

#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSize>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

namespace {

QFrame *createCard(const QString &title, const QString &description, QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setFrameShape(QFrame::StyledPanel);
    card->setStyleSheet(
        "QFrame { background: #ffffff; border: 1px solid #d9e0e8; border-radius: 8px; }"
        "QLabel { border: none; }");

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(6);

    auto *cardTitle = new QLabel(title, card);
    cardTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #1f2937;");
    auto *cardDescription = new QLabel(description, card);
    cardDescription->setWordWrap(true);
    cardDescription->setStyleSheet("color: #64748b;");

    layout->addWidget(cardTitle);
    layout->addWidget(cardDescription);
    return card;
}

QWidget *createPage(const QString &title, const QString &description, QWidget *parent)
{
    auto *page = new QWidget(parent);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 28, 32, 32);
    layout->setSpacing(18);

    auto *heading = new QLabel(title, page);
    heading->setStyleSheet("font-size: 28px; font-weight: 600; color: #172033;");
    auto *intro = new QLabel(description, page);
    intro->setWordWrap(true);
    intro->setStyleSheet("font-size: 14px; color: #64748b;");

    layout->addWidget(heading);
    layout->addWidget(intro);
    layout->addWidget(createCard("Workspace placeholder",
                                 "Static content for this module will appear here in a future iteration.",
                                 page));
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
    central->setStyleSheet("background: #f5f7fa;");
    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *sidebar = new QFrame(central);
    sidebar->setFixedWidth(230);
    sidebar->setStyleSheet("QFrame { background: #172033; }");
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(12, 18, 12, 16);
    sidebarLayout->setSpacing(8);

    auto *sidebarHeader = new QHBoxLayout;
    sidebarHeader->setContentsMargins(4, 0, 4, 12);
    auto *brandIcon = new QLabel(sidebar);
    brandIcon->setPixmap(QIcon(QStringLiteral(":/assets/cafe-ops.svg")).pixmap(QSize(30, 30)));
    brandIcon->setFixedSize(30, 30);
    auto *brand = new QLabel("CAFÉ OPS", sidebar);
    brand->setStyleSheet("color: #ffffff; font-size: 19px; font-weight: 700;");

    const QIcon expandedToggleIcon(QStringLiteral(":/assets/panel-left.svg"));
    const QIcon collapsedToggleIcon(QStringLiteral(":/assets/panel-right.svg"));
    auto *toggleButton = new QToolButton(sidebar);
    toggleButton->setIcon(expandedToggleIcon);
    toggleButton->setIconSize(QSize(20, 20));
    toggleButton->setToolTip("Collapse navigation");
    toggleButton->setCursor(Qt::PointingHandCursor);
    toggleButton->setFixedSize(32, 32);
    toggleButton->setStyleSheet(
        "QToolButton { color: #cbd5e1; background: transparent; border: 1px solid #334155; "
        "border-radius: 6px; font-size: 22px; }"
        "QToolButton:hover { color: #ffffff; background: #26344d; border-color: #64748b; }");
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
    header->setFixedHeight(78);
    header->setStyleSheet("QFrame { background: #ffffff; border-bottom: 1px solid #d9e0e8; }");
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(32, 0, 32, 0);
    auto *headerTitle = new QLabel(header);
    headerTitle->setStyleSheet("font-size: 18px; font-weight: 600; color: #172033;");
    auto *headerStatus = new QLabel("Static desktop interface", header);
    headerStatus->setStyleSheet("color: #64748b;");
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
        ":/assets/dashboard.svg",
        ":/assets/sales.svg",
        ":/assets/inventory.svg",
        ":/assets/receipts.svg",
        ":/assets/waste.svg",
        ":/assets/recipes.svg",
        ":/assets/alerts.svg",
        ":/assets/purchase-orders.svg",
        ":/assets/reports.svg",
        ":/assets/users.svg",
    };
    QList<QPushButton *> navigationButtons;

    for (int index = 0; index < modules.size(); ++index) {
        const auto &module = modules.at(index);
        pages->addWidget(createPage(module.first, module.second, pages));

        auto *button = new QPushButton(QIcon(iconResources.at(index)), module.first, sidebar);
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setIconSize(QSize(20, 20));
        button->setStyleSheet(
            "QPushButton { color: #cbd5e1; background: transparent; border: none; border-radius: 6px; "
            "padding: 10px 12px; text-align: left; font-size: 14px; }"
            "QPushButton:hover { background: #26344d; color: #ffffff; }"
            "QPushButton:checked { background: #3867d6; color: #ffffff; font-weight: 600; }");
        buttonGroup->addButton(button, index);
        navigationButtons.append(button);
        sidebarLayout->addWidget(button);

        connect(button, &QPushButton::clicked, pages, [pages, index] {
            pages->setCurrentIndex(index);
        });
    }

    sidebarLayout->addStretch();
    auto *sidebarFooter = new QLabel("Desktop workspace", sidebar);
    sidebarFooter->setStyleSheet("color: #718096; font-size: 12px; padding: 8px;");
    sidebarLayout->addWidget(sidebarFooter);

    auto setSidebarCollapsed = [sidebar, brandIcon, brand, sidebarFooter, toggleButton, expandedToggleIcon,
                                 collapsedToggleIcon, navigationButtons, modules](bool collapsed) {
        sidebar->setFixedWidth(collapsed ? 76 : 230);
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
            button->setStyleSheet(collapsed
                                      ? "QPushButton { color: #cbd5e1; background: transparent; border: none; "
                                        "border-radius: 6px; padding: 10px 4px; text-align: center; }"
                                        "QPushButton:hover { background: #26344d; color: #ffffff; }"
                                        "QPushButton:checked { background: #3867d6; color: #ffffff; }"
                                      : "QPushButton { color: #cbd5e1; background: transparent; border: none; "
                                        "border-radius: 6px; padding: 10px 12px; text-align: left; font-size: 14px; }"
                                        "QPushButton:hover { background: #26344d; color: #ffffff; }"
                                        "QPushButton:checked { background: #3867d6; color: #ffffff; font-weight: 600; }");
        }
    };

    connect(toggleButton, &QToolButton::clicked, sidebar, [setSidebarCollapsed, sidebar] {
        setSidebarCollapsed(sidebar->width() > 100);
    });

    connect(pages, &QStackedWidget::currentChanged, headerTitle, [headerTitle, modules](int index) {
        headerTitle->setText(modules.at(index).first);
    });

    buttonGroup->button(0)->setChecked(true);
    pages->setCurrentIndex(0);
    headerTitle->setText(modules.first().first);

    contentLayout->addWidget(header);
    contentLayout->addWidget(pages, 1);
    rootLayout->addWidget(sidebar);
    rootLayout->addWidget(content, 1);
    setCentralWidget(central);
}
