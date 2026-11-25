#include "mainwindow.h"
#include "style/constants.h"
#include "db/databasemanager.h"
#include "services/peps_engine.h"
#include "services/pdf_generator.h"
#include "ui/logindialog.h"

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QScrollArea>
#include <QShortcut>
#include <QSpinBox>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>

using namespace Ui::Tokens;

namespace {

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

QLabel *createSectionHeading(const QString &text, QWidget *parent)
{
    auto *heading = new QLabel(text, parent);
    heading->setStyleSheet(
        QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textTitle).arg(weightSemibold).arg(ink));
    return heading;
}

QPushButton *createPrimaryButton(const QString &text, QWidget *parent)
{
    auto *button = new QPushButton(text, parent);
    button->setMinimumHeight(controlHeight);
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(QStringLiteral(
        "QPushButton { background: %1; color: %2; border: 1px solid %1; border-radius: %3px; "
        "padding: 0 %4px; font-weight: %5; }"
        "QPushButton:hover { background: %6; }"
        "QPushButton:focus { border: 2px solid %7; }")
        .arg(ink)
        .arg(surface)
        .arg(radiusSmall)
        .arg(space3)
        .arg(weightSemibold)
        .arg(sidebarHover)
        .arg(focus));
    return button;
}

QTableWidget *createStyledTable(const QStringList &headers, QWidget *parent)
{
    auto *table = new QTableWidget(0, headers.size(), parent);
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->setMinimumHeight(260);
    table->setFocusPolicy(Qt::NoFocus);
    table->setStyleSheet(QStringLiteral(
        "QTableWidget { background: %1; color: %2; border: 1px solid %3; border-radius: %4px; "
        "alternate-background-color: %5; outline: none; selection-background-color: #e5e5e0; selection-color: %2; }"
        "QHeaderView::section { background: %1; color: %6; border: none; border-bottom: 1px solid %3; "
        "padding: %7px; font-weight: %8; }"
        "QTableWidget::item { padding: %7px; border-bottom: 1px solid %3; outline: none; }"
        "QTableWidget::item:selected { background-color: #e5e5e0; color: %2; outline: none; "
        "border-top: 1.5px solid #000000; border-bottom: 1.5px solid #000000; }"
        "QTableWidget::item:focus { outline: none; border: 1.5px solid #000000; }")
        .arg(surface)
        .arg(ink)
        .arg(border)
        .arg(radiusMedium)
        .arg(surfaceMuted)
        .arg(mutedText)
        .arg(space2)
        .arg(weightSemibold));
    return table;
}

} // namespace

MainWindow::MainWindow(const Models::Usuario& usuarioActual, QWidget *parent)
    : QMainWindow(parent), m_usuario(usuarioActual)
{
    setWindowTitle("Café Operations · InsumoCafé POS");
    setMinimumSize(1024, 680);
    resize(1280, 800);

    configurarVistas();
    recargarDatosGenerales();

    // Atajo de teclado F5 para cobro rápido (Regla de usabilidad Theo Mandel)
    auto* f5Shortcut = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(f5Shortcut, &QShortcut::activated, this, &MainWindow::procesarCobroVenta);

    // Temporizador periódico en segundo plano para Alertas (RF-04: cada 15 min en producción, 1 min para demo)
    m_timerAlertas = new QTimer(this);
    connect(m_timerAlertas, &QTimer::timeout, this, &MainWindow::verificarAlertasPeriodicas);
    m_timerAlertas->start(60000); // 1 minuto para demostración reactiva
}

void MainWindow::configurarVistas()
{
    auto *central = new QWidget(this);
    central->setStyleSheet(QStringLiteral("background: %1; font-family: 'Segoe UI', Sans-Serif;").arg(canvas));
    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Barra lateral de navegación (Sidebar Uber-dark)
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
    toggleButton->setToolTip("Colapsar navegación");
    toggleButton->setCursor(Qt::PointingHandCursor);
    toggleButton->setFixedSize(compactControlHeight, compactControlHeight);
    toggleButton->setStyleSheet(QStringLiteral("QToolButton { color: %1; background: transparent; border: 1px solid %2; border-radius: %3px; } QToolButton:hover { color: %4; background: %2; }").arg(sidebarText).arg(sidebarHover).arg(radiusSmall).arg(surface));
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

    // Header Superior
    auto *header = new QFrame(content);
    header->setFixedHeight(headerHeight);
    header->setStyleSheet(QStringLiteral("QFrame { background: %1; border-bottom: 1px solid %2; }").arg(surface).arg(border));
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(space8, 0, space8, 0);
    auto *headerTitle = new QLabel(header);
    headerTitle->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textTitle).arg(weightSemibold).arg(ink));
    headerLayout->addWidget(headerTitle);
    headerLayout->addStretch();

    // Badge persistente de Alertas (estilo minimalista acorde al diseño)
    m_btnAlertaHeader = new QPushButton("Alertas", header);
    m_btnAlertaHeader->setCursor(Qt::PointingHandCursor);
    m_btnAlertaHeader->setToolTip("Abrir panel de alertas");
    m_btnAlertaHeader->setStyleSheet(
        "QPushButton { background: #f0f0ed; color: #0b0b0b; border: 1px solid #d6d6d2; "
        "border-radius: 4px; padding: 4px 10px; font-size: 12px; font-weight: 500; outline: none; }"
        "QPushButton:hover { background: #e5e5e0; border-color: #0b0b0b; }");
    headerLayout->addWidget(m_btnAlertaHeader);
    headerLayout->addSpacing(space3);

    // Indicador de conexión a Base de Datos
    auto *dbStatus = new QLabel(QString("Base de Datos: %1").arg(DatabaseManager::getInstance().connectionType()), header);
    dbStatus->setStyleSheet(QStringLiteral("color: %1; font-size: %2px; padding: 4px 8px; border-radius: 4px; background: %3;")
                            .arg(success).arg(textSmall).arg(successMuted));
    headerLayout->addWidget(dbStatus);
    headerLayout->addSpacing(space3);

    // Badge de sesión y botón de cambiar usuario en cabecera
    QString nombreMostrar = m_usuario.nombreCompleto.isEmpty() ? "Operador" : m_usuario.nombreCompleto;
    QString rolMostrar = m_usuario.nombreRol.isEmpty() ? "ADMIN" : m_usuario.nombreRol;
    auto *userBadge = new QLabel(QString("<b>%1</b> (%2)").arg(nombreMostrar, rolMostrar), header);
    userBadge->setStyleSheet(QStringLiteral("color: %1; font-size: %2px; font-weight: 500; padding: 4px 8px;").arg(ink).arg(textSmall));
    headerLayout->addWidget(userBadge);

    auto *btnCambiarUserHeader = new QPushButton("Cambiar usuario", header);
    btnCambiarUserHeader->setCursor(Qt::PointingHandCursor);
    btnCambiarUserHeader->setStyleSheet(
        "QPushButton { background: #ffffff; color: #0b0b0b; border: 1px solid #d6d6d2; "
        "border-radius: 4px; padding: 4px 10px; font-size: 12px; font-weight: 500; outline: none; }"
        "QPushButton:hover { background: #f0f0ed; border-color: #000000; }");
    connect(btnCambiarUserHeader, &QPushButton::clicked, this, &MainWindow::abrirDialogoCambiarUsuario);
    headerLayout->addWidget(btnCambiarUserHeader);

    m_pages = new QStackedWidget(content);

    // Módulos con RBAC estricto según perfil
    struct ModuloConfig {
        QString titulo;
        QString descripcion;
        QString icono;
        std::function<QWidget*(QWidget*)> creador;
        QStringList rolesPermitidos;
    };

    QList<ModuloConfig> listaModulos = {
        {"Dashboard", "Panorama de inventario, ventas y alertas en tiempo real.", ":/assets/dashboard.svg", [this](QWidget* p){ return createDashboardPage(p); }, {"ADMIN", "ALMACEN"}},
        {"Sales", "Caja rápida y punto de venta POS con deducción PEPS.", ":/assets/sales.svg", [this](QWidget* p){ return createSalesPage(p); }, {"ADMIN", "BARISTA", "CAJERO"}},
        {"Inventory & Lots", "Existencias consolidadas de insumos en almacén.", ":/assets/inventory.svg", [this](QWidget* p){ return createInventoryPage(p); }, {"ADMIN", "ALMACEN"}},
        {"Lot Receipts", "Recepción de partidas y trazabilidad por código de lote PEPS.", ":/assets/receipts.svg", [this](QWidget* p){ return createReceiptsPage(p); }, {"ADMIN", "ALMACEN"}},
        {"Waste", "Registro justificado de mermas por caducidad o merma operativa.", ":/assets/waste.svg", [this](QWidget* p){ return createWastePage(p); }, {"ADMIN", "BARISTA", "CAJERO", "ALMACEN"}},
        {"Recipes", "Catálogo de recetas y formulaciones técnicas (Escandallos).", ":/assets/recipes.svg", [this](QWidget* p){ return createRecipesPage(p); }, {"ADMIN", "BARISTA", "CAJERO", "ALMACEN"}},
        {"Alerts", "Semáforo preventivo de caducidades críticas y stock mínimo.", ":/assets/alerts.svg", [this](QWidget* p){ return createAlertsPage(p); }, {"ADMIN", "BARISTA", "CAJERO", "ALMACEN"}},
        {"Purchase Orders", "Consolidación de compras y órdenes de reposición a proveedores.", ":/assets/purchase-orders.svg", [this](QWidget* p){ return createPurchaseOrdersPage(p); }, {"ADMIN", "ALMACEN"}},
        {"Reports", "Exportación de análisis de ventas, mermas e historial PEPS.", ":/assets/reports.svg", [this](QWidget* p){ return createReportsPage(p); }, {"ADMIN"}},
        {"Users & Roles", "Control de acceso, operadores y privilegios en el sistema.", ":/assets/users.svg", [this](QWidget* p){ return createUsersPage(p); }, {"ADMIN"}}
    };

    QString rolActual = m_usuario.nombreRol.trimmed().toUpper();
    if (rolActual.isEmpty()) rolActual = "ADMIN";

    QList<QPushButton *> navigationButtons;
    QStringList titulosVisibles;

    int indicePagina = 0;
    for (const auto& mod : listaModulos) {
        if (!mod.rolesPermitidos.contains(rolActual)) {
            continue; // Filtrar módulos según rol autorizado
        }

        m_pages->addWidget(mod.creador(m_pages));
        titulosVisibles.append(mod.titulo);

        auto *button = new QPushButton(QIcon(mod.icono), mod.titulo, sidebar);
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setIconSize(QSize(iconMedium, iconMedium));
        button->setStyleSheet(navigationButtonStyle(false));
        buttonGroup->addButton(button, indicePagina);
        navigationButtons.append(button);
        sidebarLayout->addWidget(button);

        if (mod.titulo == "Alerts") {
            m_btnModuloAlertas = button;
        }

        int currentIdx = indicePagina;
        connect(button, &QPushButton::clicked, m_pages, [this, currentIdx] { m_pages->setCurrentIndex(currentIdx); });
        indicePagina++;
    }

    if (m_btnAlertaHeader) {
        connect(m_btnAlertaHeader, &QPushButton::clicked, this, [this]() {
            if (m_btnModuloAlertas) {
                m_btnModuloAlertas->click();
            }
        });
    }

    sidebarLayout->addStretch();

    // Footer de Sesión de Usuario en barra lateral
    auto *sidebarFooter = new QLabel(QString("%1 · %2").arg(nombreMostrar, rolMostrar), sidebar);
    sidebarFooter->setStyleSheet(QStringLiteral("color: %1; font-size: %2px; padding: %3px;").arg(sidebarText).arg(textSmall).arg(space2));
    sidebarLayout->addWidget(sidebarFooter);

    auto *btnCerrarSesion = new QPushButton("Cerrar sesión", sidebar);
    btnCerrarSesion->setCursor(Qt::PointingHandCursor);
    btnCerrarSesion->setStyleSheet(
        "QPushButton { background: transparent; color: #a1a1aa; border: 1px solid #3f3f46; "
        "border-radius: 4px; padding: 5px 8px; font-size: 12px; outline: none; }"
        "QPushButton:hover { color: #ffffff; border-color: #ffffff; background: #27272a; }");
    connect(btnCerrarSesion, &QPushButton::clicked, this, &MainWindow::abrirDialogoCambiarUsuario);
    sidebarLayout->addWidget(btnCerrarSesion);

    auto setSidebarCollapsed = [sidebar, brandIcon, brand, sidebarFooter, btnCerrarSesion, toggleButton, expandedToggleIcon, collapsedToggleIcon, navigationButtons, titulosVisibles](bool collapsed) {
        sidebar->setFixedWidth(collapsed ? sidebarCollapsedWidth : sidebarWidth);
        brandIcon->setVisible(true);
        brand->setVisible(!collapsed);
        sidebarFooter->setVisible(!collapsed);
        btnCerrarSesion->setVisible(!collapsed);
        toggleButton->setIcon(collapsed ? collapsedToggleIcon : expandedToggleIcon);
        for (int index = 0; index < navigationButtons.size(); ++index) {
            auto *button = navigationButtons.at(index);
            button->setText(collapsed ? QString() : titulosVisibles.at(index));
            button->setStyleSheet(navigationButtonStyle(collapsed));
        }
    };

    connect(toggleButton, &QToolButton::clicked, sidebar, [setSidebarCollapsed, sidebar] { setSidebarCollapsed(sidebar->width() > 100); });
    connect(m_pages, &QStackedWidget::currentChanged, headerTitle, [headerTitle, titulosVisibles](int index) {
        if (index < titulosVisibles.size()) headerTitle->setText(titulosVisibles.at(index));
    });

    if (!navigationButtons.isEmpty()) {
        navigationButtons.first()->setChecked(true);
        m_pages->setCurrentIndex(0);
        headerTitle->setText(titulosVisibles.first());
    }

    contentLayout->addWidget(header);
    contentLayout->addWidget(m_pages, 1);
    rootLayout->addWidget(sidebar);
    rootLayout->addWidget(content, 1);
    setCentralWidget(central);
}

QWidget* MainWindow::createDashboardPage(QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setStyleSheet(QStringLiteral("background: %1;").arg(canvas));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(space8, space6, space8, space8);
    layout->setSpacing(space5);

    auto *metrics = new QGridLayout;
    metrics->setHorizontalSpacing(space3);
    metrics->setVerticalSpacing(space3);

    auto crearKpi = [](const QString& titulo, const QString& detalle, const QString& color, QLabel*& outLabel, QWidget* parent) {
        auto *card = new QFrame(parent);
        card->setStyleSheet(cardStyle());
        auto *ly = new QVBoxLayout(card);
        ly->setContentsMargins(space4, space4, space4, space4);
        ly->setSpacing(space1);
        auto *lblTit = new QLabel(titulo, card);
        lblTit->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textSmall).arg(mutedText));
        outLabel = new QLabel("$0.00", card);
        outLabel->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textHeading).arg(weightBold).arg(ink));
        auto *lblDet = new QLabel(detalle, card);
        lblDet->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textSmall).arg(weightSemibold).arg(color));
        ly->addWidget(lblTit);
        ly->addWidget(outLabel);
        ly->addWidget(lblDet);
        return card;
    };

    metrics->addWidget(crearKpi("Ventas de hoy", "+8% vs. ayer", success, m_kpiVentas, page), 0, 0);
    metrics->addWidget(crearKpi("Stock crítico", "Requiere compra", danger, m_kpiStockCritico, page), 0, 1);
    metrics->addWidget(crearKpi("Lotes por vencer", "< 72 horas", warning, m_kpiLotesVencer, page), 0, 2);
    metrics->addWidget(crearKpi("Mermas del mes", "-12% vs. mes anterior", success, m_kpiMermas, page), 0, 3);
    layout->addLayout(metrics);

    auto *alertsCard = new QFrame(page);
    alertsCard->setStyleSheet(cardStyle());
    auto *alertsLayout = new QVBoxLayout(alertsCard);
    alertsLayout->setContentsMargins(space5, space5, space5, space5);
    alertsLayout->setSpacing(space3);
    alertsLayout->addWidget(createSectionHeading("Alertas prioritarias del almacén", alertsCard));

    m_tablaAlertasDashboard = createStyledTable({"Insumo", "Lote", "Acción"}, alertsCard);
    alertsLayout->addWidget(m_tablaAlertasDashboard);

    layout->addWidget(alertsCard, 1);
    return page;
}

QWidget* MainWindow::createSalesPage(QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setStyleSheet(QStringLiteral("background: %1;").arg(canvas));
    auto *layout = new QHBoxLayout(page);
    layout->setContentsMargins(space8, space6, space8, space8);
    layout->setSpacing(space6);

    // Catálogo dinámico de productos
    auto *catalog = new QWidget(page);
    auto *catalogLayout = new QVBoxLayout(catalog);
    catalogLayout->setContentsMargins(0, 0, 0, 0);
    catalogLayout->setSpacing(space3);
    catalogLayout->addWidget(createSectionHeading("Productos del menú", catalog));

    auto *scrollArea = new QScrollArea(catalog);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *scrollContent = new QWidget(scrollArea);
    auto *productsGrid = new QGridLayout(scrollContent);
    productsGrid->setHorizontalSpacing(space4);
    productsGrid->setVerticalSpacing(space4);

    auto productos = DatabaseManager::getInstance().getProductos();
    for (int i = 0; i < productos.size(); ++i) {
        const auto& prod = productos.at(i);
        auto *card = new QFrame(scrollContent);
        card->setStyleSheet(cardStyle());
        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(space4, space4, space4, space4);
        cardLayout->setSpacing(space2);

        auto *name = new QLabel(prod.nombreProd, card);
        name->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textBody).arg(weightSemibold).arg(ink));
        auto *price = new QLabel(QString("$%1").arg(QString::number(prod.precioVenta, 'f', 2)), card);
        price->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));

        auto *btnAdd = new QPushButton("Agregar", card);
        btnAdd->setMinimumHeight(controlHeight);
        btnAdd->setCursor(Qt::PointingHandCursor);
        btnAdd->setStyleSheet(QStringLiteral("QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: %4px; font-weight: %5; } QPushButton:hover { background: %6; }")
                              .arg(surface).arg(ink).arg(border).arg(radiusSmall).arg(weightSemibold).arg(surfaceMuted));

        int id = prod.idProducto;
        QString nom = prod.nombreProd;
        double pr = prod.precioVenta;
        connect(btnAdd, &QPushButton::clicked, this, [this, id, nom, pr]() {
            agregarProductoAlTicket(id, nom, pr);
        });

        cardLayout->addWidget(name);
        cardLayout->addWidget(price);
        cardLayout->addWidget(btnAdd);
        productsGrid->addWidget(card, i / 3, i % 3);
    }
    scrollArea->setWidget(scrollContent);
    catalogLayout->addWidget(scrollArea);

    // Panel Lateral de Comanda (Ticket)
    auto *ticket = new QFrame(page);
    ticket->setMinimumWidth(430);
    ticket->setStyleSheet(cardStyle());
    auto *ticketLayout = new QVBoxLayout(ticket);
    ticketLayout->setContentsMargins(space5, space5, space5, space5);
    ticketLayout->setSpacing(space4);

    ticketLayout->addWidget(createSectionHeading("Ticket actual", ticket));
    auto *ticketMeta = new QLabel("Comanda en curso · Mostrador", ticket);
    ticketMeta->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textSmall).arg(mutedText));
    ticketLayout->addWidget(ticketMeta);

    m_ticketTable = createStyledTable({"Producto", "Cant.", "Subtotal"}, ticket);
    m_ticketTable->setMinimumHeight(240);
    ticketLayout->addWidget(m_ticketTable);

    m_lblTotalTicket = new QLabel("Total: $0.00", ticket);
    m_lblTotalTicket->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textTitle).arg(weightBold).arg(ink));
    ticketLayout->addWidget(m_lblTotalTicket);

    auto *btnCobrar = createPrimaryButton("F5 · Cobrar y descontar PEPS", ticket);
    ticketLayout->addWidget(btnCobrar);
    connect(btnCobrar, &QPushButton::clicked, this, &MainWindow::procesarCobroVenta);

    auto *ticketButtons = new QHBoxLayout;
    auto *btnQuitar = new QPushButton("Quitar 1 seleccionado", ticket);
    btnQuitar->setMinimumHeight(controlHeight);
    btnQuitar->setStyleSheet(QStringLiteral("QPushButton { background: transparent; color: %1; border: 1px solid %2; border-radius: %3px; } QPushButton:hover { background: %4; }")
                             .arg(mutedText).arg(border).arg(radiusSmall).arg(surfaceMuted));
    connect(btnQuitar, &QPushButton::clicked, this, &MainWindow::quitarItemDelTicket);

    auto *btnLimpiar = new QPushButton("Limpiar ticket", ticket);
    btnLimpiar->setMinimumHeight(controlHeight);
    btnLimpiar->setStyleSheet(QStringLiteral("QPushButton { background: transparent; color: %1; border: 1px solid %2; border-radius: %3px; } QPushButton:hover { background: %4; }")
                             .arg(mutedText).arg(border).arg(radiusSmall).arg(surfaceMuted));
    connect(btnLimpiar, &QPushButton::clicked, this, [this]() {
        m_ticketActual.clear();
        actualizarTablaTicket();
    });

    ticketButtons->addWidget(btnQuitar);
    ticketButtons->addWidget(btnLimpiar);
    ticketLayout->addLayout(ticketButtons);

    connect(m_ticketTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        if (row >= 0 && row < m_ticketActual.size()) {
            if (m_ticketActual[row].cantidad > 1) {
                m_ticketActual[row].cantidad--;
            } else {
                m_ticketActual.removeAt(row);
            }
            actualizarTablaTicket();
        }
    });

    ticketLayout->addStretch();

    layout->addWidget(catalog, 1);
    layout->addWidget(ticket);
    return page;
}

void MainWindow::agregarProductoAlTicket(int idProducto, const QString& nombre, double precio)
{
    for (auto& item : m_ticketActual) {
        if (item.idProducto == idProducto) {
            item.cantidad++;
            actualizarTablaTicket();
            return;
        }
    }
    Models::ItemTicket nuevo;
    nuevo.idProducto = idProducto;
    nuevo.nombreProd = nombre;
    nuevo.cantidad = 1;
    nuevo.precioUnitario = precio;
    m_ticketActual.append(nuevo);
    actualizarTablaTicket();
}

void MainWindow::actualizarTablaTicket()
{
    m_ticketTable->setRowCount(m_ticketActual.size());
    double total = 0.0;
    for (int i = 0; i < m_ticketActual.size(); ++i) {
        const auto& it = m_ticketActual.at(i);
        m_ticketTable->setItem(i, 0, new QTableWidgetItem(it.nombreProd));
        m_ticketTable->setItem(i, 1, new QTableWidgetItem(QString::number(it.cantidad)));
        m_ticketTable->setItem(i, 2, new QTableWidgetItem(QString("$%1").arg(QString::number(it.subtotal(), 'f', 2))));
        total += it.subtotal();
    }
    m_lblTotalTicket->setText(QString("Total: $%1").arg(QString::number(total, 'f', 2)));
}

void MainWindow::quitarItemDelTicket()
{
    int row = m_ticketTable->currentRow();
    if (row >= 0 && row < m_ticketActual.size()) {
        if (m_ticketActual[row].cantidad > 1) {
            m_ticketActual[row].cantidad--;
        } else {
            m_ticketActual.removeAt(row);
        }
        actualizarTablaTicket();
    } else if (!m_ticketActual.isEmpty()) {
        if (m_ticketActual.last().cantidad > 1) {
            m_ticketActual.last().cantidad--;
        } else {
            m_ticketActual.removeLast();
        }
        actualizarTablaTicket();
    }
}

void MainWindow::procesarCobroVenta()
{
    if (m_ticketActual.isEmpty()) {
        QMessageBox::information(this, "Ticket Vacío", "Agregue productos antes de cobrar.");
        return;
    }

    QString errorMsg;
    QString folio;
    int idUser = m_usuario.idUsuario > 0 ? m_usuario.idUsuario : 1;

    bool ok = MotorPEPS::getInstance().procesarVenta(m_ticketActual, idUser, &errorMsg, &folio);
    if (ok) {
        QMessageBox::information(this, "Venta Exitosa",
                                 QString("¡Venta finalizada exitosamente!\n\nFolio: %1\nTotal: %2\nInsumos deducidos por regla PEPS (FIFO).")
                                 .arg(folio, m_lblTotalTicket->text()));
        m_ticketActual.clear();
        actualizarTablaTicket();
        recargarDatosGenerales();
    } else {
        // Alerta acústica reglamentaria (CU-03, Flujo Alterno 5a)
        QApplication::beep();
        QMessageBox::critical(this, "Error en Venta PEPS (Stock Insuficiente)",
                              QString("No se pudo completar la transacción:\n%1").arg(errorMsg));
    }
}

// 4.3 Módulo de Inventario Consolidado
QWidget* MainWindow::createInventoryPage(QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setStyleSheet(QStringLiteral("background: %1;").arg(canvas));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(space8, space6, space8, space8);
    layout->setSpacing(space4);

    auto *toolbar = new QHBoxLayout;
    auto *intro = new QLabel("Existencias físicas consolidadas en almacén y barra de atención.", page);
    intro->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));
    toolbar->addWidget(intro);
    toolbar->addStretch();
    auto *btn = createPrimaryButton("Actualizar existencias", page);
    toolbar->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, &MainWindow::recargarDatosGenerales);
    layout->addLayout(toolbar);

    m_tablaInventarioConsolidado = createStyledTable({"Materia Prima / Insumo", "Existencia Disponible", "Stock Mínimo", "Estado Semafórico"}, page);
    layout->addWidget(m_tablaInventarioConsolidado);
    return page;
}

// 4.4 Módulo de Recepción de Lotes y Trazabilidad PEPS
QWidget* MainWindow::createReceiptsPage(QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setStyleSheet(QStringLiteral("background: %1;").arg(canvas));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(space8, space6, space8, space8);
    layout->setSpacing(space4);

    auto *toolbar = new QHBoxLayout;
    auto *intro = new QLabel("Trazabilidad por código de lote, partidas de compras y fechas de caducidad.", page);
    intro->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));
    toolbar->addWidget(intro);
    toolbar->addStretch();
    auto *btn = createPrimaryButton("Registrar lote recibido", page);
    toolbar->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, &MainWindow::abrirDialogoRegistrarLote);
    layout->addLayout(toolbar);

    m_tablaRecepcionesLotes = createStyledTable({"Insumo", "Código Lote", "Caducidad", "Restante", "Inicial", "Estado PEPS"}, page);
    layout->addWidget(m_tablaRecepcionesLotes);
    return page;
}

QWidget* MainWindow::createWastePage(QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setStyleSheet(QStringLiteral("background: %1;").arg(canvas));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(space8, space6, space8, space8);
    layout->setSpacing(space4);

    auto *toolbar = new QHBoxLayout;
    auto *intro = new QLabel("Registro justificado de mermas (caducidad, rotura o error de preparación).", page);
    intro->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));
    toolbar->addWidget(intro);
    toolbar->addStretch();
    auto *btn = createPrimaryButton("Registrar merma", page);
    toolbar->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, &MainWindow::abrirDialogoRegistrarMerma);
    layout->addLayout(toolbar);

    m_tablaMermas = createStyledTable({"Insumo", "Lote", "Cantidad", "Motivo Formal", "Costo Pérdida", "Usuario"}, page);
    layout->addWidget(m_tablaMermas);
    return page;
}

// 4.6 Módulo de Recetas y Escandallos Técnicos (Master-Detail)
QWidget* MainWindow::createRecipesPage(QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setStyleSheet(QStringLiteral("background: %1;").arg(canvas));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(space8, space6, space8, space8);
    layout->setSpacing(space4);

    auto *toolbar = new QHBoxLayout;
    auto *intro = new QLabel("Formulaciones técnicas unitarias con cálculo de costo de insumos en tiempo real.", page);
    intro->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));
    toolbar->addWidget(intro);
    toolbar->addStretch();
    auto *btn = createPrimaryButton("Nuevo producto", page);
    toolbar->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, &MainWindow::abrirDialogoNuevaReceta);
    layout->addLayout(toolbar);

    // Tabla Maestra: Productos
    m_tablaRecetasProductos = createStyledTable({"Producto", "Categoría", "Precio Venta", "Costo Producción", "Margen Teórico"}, page);
    m_tablaRecetasProductos->setMinimumHeight(180);
    layout->addWidget(m_tablaRecetasProductos);

    connect(m_tablaRecetasProductos, &QTableWidget::cellClicked, this, &MainWindow::onProductoRecetaSeleccionado);

    // Tarjeta Detalle: Escandallo del producto seleccionado
    auto *detailCard = new QFrame(page);
    detailCard->setStyleSheet(cardStyle());
    auto *detLayout = new QVBoxLayout(detailCard);
    detLayout->setContentsMargins(space5, space5, space5, space5);
    detLayout->setSpacing(space3);

    auto *detHeader = new QHBoxLayout;
    m_lblEscandalloTitulo = createSectionHeading("Escandallo Técnico: Seleccione un producto", detailCard);
    m_lblCostoTeorico = new QLabel(detailCard);
    m_lblCostoTeorico->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textBody).arg(weightSemibold).arg(ink));
    detHeader->addWidget(m_lblEscandalloTitulo);
    detHeader->addStretch();
    detHeader->addWidget(m_lblCostoTeorico);
    detLayout->addLayout(detHeader);

    m_tablaEscandalloDetalle = createStyledTable({"Insumo Requerido", "Porción Unitaria", "Unidad", "Costo Insumo", "Costo en Bebida"}, detailCard);
    m_tablaEscandalloDetalle->setMinimumHeight(160);
    detLayout->addWidget(m_tablaEscandalloDetalle);

    layout->addWidget(detailCard);
    return page;
}

void MainWindow::onProductoRecetaSeleccionado(int row, int /*column*/)
{
    auto productos = DatabaseManager::getInstance().getProductos();
    if (row < 0 || row >= productos.size()) return;

    const auto& prod = productos.at(row);
    m_lblEscandalloTitulo->setText(QString("Escandallo Técnico: %1 (%2)").arg(prod.nombreProd, prod.categoria));
    m_lblCostoTeorico->setText(QString("Costo Teórico: $%1 MXN (PVP: $%2)")
                               .arg(QString::number(prod.costoProduccionTeorico, 'f', 2))
                               .arg(QString::number(prod.precioVenta, 'f', 2)));

    auto ingredientes = DatabaseManager::getInstance().getIngredientesReceta(prod.idProducto);
    m_tablaEscandalloDetalle->setRowCount(ingredientes.size());
    for (int i = 0; i < ingredientes.size(); ++i) {
        const auto& ing = ingredientes.at(i);
        m_tablaEscandalloDetalle->setItem(i, 0, new QTableWidgetItem(ing.nombreInsumo));
        m_tablaEscandalloDetalle->setItem(i, 1, new QTableWidgetItem(QString::number(ing.cantidadDescuento, 'f', 2)));
        m_tablaEscandalloDetalle->setItem(i, 2, new QTableWidgetItem(ing.unidadMedida));
        m_tablaEscandalloDetalle->setItem(i, 3, new QTableWidgetItem(QString("$%1 / %2").arg(QString::number(ing.costoUnitarioInsumo, 'f', 3), ing.unidadMedida)));
        m_tablaEscandalloDetalle->setItem(i, 4, new QTableWidgetItem(QString("$%1").arg(QString::number(ing.costoPorcion(), 'f', 2))));
    }
}

QWidget* MainWindow::createAlertsPage(QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setStyleSheet(QStringLiteral("background: %1;").arg(canvas));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(space8, space6, space8, space8);
    layout->setSpacing(space4);

    auto *toolbar = new QHBoxLayout;
    auto *intro = new QLabel("Semáforo preventivo de riesgos de caducidad (<72h) y stock mínimo.", page);
    intro->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));
    toolbar->addWidget(intro);
    toolbar->addStretch();
    auto *btn = createPrimaryButton("Actualizar alertas", page);
    toolbar->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, &MainWindow::recargarDatosGenerales);
    layout->addLayout(toolbar);

    m_tablaAlertas = createStyledTable({"Insumo", "Lote / Tipo", "Nivel", "Acción Sugerida"}, page);
    layout->addWidget(m_tablaAlertas);
    return page;
}

// 4.8 Consolidación Automática de Órdenes de Compra (RF-05)
QWidget* MainWindow::createPurchaseOrdersPage(QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setStyleSheet(QStringLiteral("background: %1;").arg(canvas));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(space8, space6, space8, space8);
    layout->setSpacing(space4);

    auto *toolbar = new QHBoxLayout;
    auto *intro = new QLabel("Consolidación inteligente de reabastecimiento agrupado por proveedor.", page);
    intro->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));
    toolbar->addWidget(intro);
    toolbar->addStretch();
    auto *btn = createPrimaryButton("Generar orden de compra (PDF)", page);
    toolbar->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, &MainWindow::abrirDialogoGenerarOrdenPdf);
    layout->addLayout(toolbar);

    m_tablaOrdenes = createStyledTable({"Folio", "Proveedor", "Total Estimado", "Estado", "Fecha Emisión"}, page);
    layout->addWidget(m_tablaOrdenes);
    return page;
}

QWidget* MainWindow::createReportsPage(QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setStyleSheet(QStringLiteral("background: %1;").arg(canvas));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(space8, space6, space8, space8);
    layout->setSpacing(space4);

    auto *toolbar = new QHBoxLayout;
    auto *intro = new QLabel("Reportes ejecutivos de inventario, mermas y auditoría PEPS.", page);
    intro->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));
    toolbar->addWidget(intro);
    toolbar->addStretch();

    auto *btnVentasPdf = createPrimaryButton("Exportar Ventas en PDF", page);
    toolbar->addWidget(btnVentasPdf);
    connect(btnVentasPdf, &QPushButton::clicked, this, &MainWindow::exportarReporteVentas);

    auto *btnMermasPdf = new QPushButton("Exportar Mermas en PDF", page);
    btnMermasPdf->setMinimumHeight(controlHeight);
    btnMermasPdf->setStyleSheet(QStringLiteral("QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: %4px; padding: 0 16px; font-weight: %5; }")
                                .arg(surface).arg(ink).arg(border).arg(radiusSmall).arg(weightSemibold));
    toolbar->addWidget(btnMermasPdf);
    connect(btnMermasPdf, &QPushButton::clicked, this, &MainWindow::exportarReporteMermas);

    auto *btnCsv = new QPushButton("Exportar Historial CSV", page);
    btnCsv->setMinimumHeight(controlHeight);
    btnCsv->setStyleSheet(QStringLiteral("QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: %4px; padding: 0 16px; font-weight: %5; }")
                         .arg(surface).arg(ink).arg(border).arg(radiusSmall).arg(weightSemibold));
    toolbar->addWidget(btnCsv);
    connect(btnCsv, &QPushButton::clicked, this, &MainWindow::exportarReporteCsv);

    layout->addLayout(toolbar);

    auto *table = createStyledTable({"Reporte", "Periodo", "Formato"}, page);
    table->setRowCount(3);
    table->setItem(0, 0, new QTableWidgetItem("Resumen de Ventas"));
    table->setItem(0, 1, new QTableWidgetItem("Noviembre 2026"));
    table->setItem(0, 2, new QTableWidgetItem("PDF Oficial"));

    table->setItem(1, 0, new QTableWidgetItem("Mermas Financieras"));
    table->setItem(1, 1, new QTableWidgetItem("Noviembre 2026"));
    table->setItem(1, 2, new QTableWidgetItem("PDF Oficial"));

    table->setItem(2, 0, new QTableWidgetItem("Movimientos PEPS / Auditoría"));
    table->setItem(2, 1, new QTableWidgetItem("Histórico Completo"));
    table->setItem(2, 2, new QTableWidgetItem("CSV"));
    layout->addWidget(table);

    return page;
}

QWidget* MainWindow::createUsersPage(QWidget *parent)
{
    auto *page = new QWidget(parent);
    page->setStyleSheet(QStringLiteral("background: %1;").arg(canvas));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(space8, space6, space8, space8);
    layout->setSpacing(space4);

    auto *toolbar = new QHBoxLayout;
    auto *intro = new QLabel("Usuarios y roles registrados con privilegios en el sistema.", page);
    intro->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));
    toolbar->addWidget(intro);
    toolbar->addStretch();
    layout->addLayout(toolbar);

    m_tablaUsuarios = createStyledTable({"Nombre Completo", "Usuario", "Rol", "Estado"}, page);
    layout->addWidget(m_tablaUsuarios);
    return page;
}

void MainWindow::verificarAlertasPeriodicas()
{
    // Monitoreo periódico reactivo en segundo plano: actualiza paneles y badges sin popups invasivos
    recargarDatosGenerales();
}

void MainWindow::recargarDatosGenerales()
{
    int lotesCriticos = DatabaseManager::getInstance().getLotesPorVencerCount();
    int stockCritico = DatabaseManager::getInstance().getStockCriticoCount();
    int totalAlertas = lotesCriticos + stockCritico;

    // Actualizar badge superior persistente
    if (m_btnAlertaHeader) {
        if (totalAlertas > 0) {
            m_btnAlertaHeader->setText(QString("Alertas: %1 por vencer · %2 stock bajo").arg(lotesCriticos).arg(stockCritico));
            m_btnAlertaHeader->setStyleSheet(
                "QPushButton { background: #fef0ee; color: #b42318; border: 1px solid #fecdc7; "
                "border-radius: 4px; padding: 4px 10px; font-size: 12px; font-weight: 600; outline: none; }"
                "QPushButton:hover { background: #fee2e2; border-color: #b42318; }");
            m_btnAlertaHeader->setVisible(true);
        } else {
            m_btnAlertaHeader->setText("Inventario en orden");
            m_btnAlertaHeader->setStyleSheet(
                "QPushButton { background: #e7f5ec; color: #147a43; border: 1px solid #bbf7d0; "
                "border-radius: 4px; padding: 4px 10px; font-size: 12px; font-weight: 500; outline: none; }"
                "QPushButton:hover { background: #dcfce7; border-color: #147a43; }");
            m_btnAlertaHeader->setVisible(true);
        }
    }

    // 1. Dashboard
    if (m_kpiVentas) m_kpiVentas->setText(QString("$%1").arg(QString::number(DatabaseManager::getInstance().getVentasHoy(), 'f', 2)));
    if (m_kpiStockCritico) m_kpiStockCritico->setText(QString("%1 insumos").arg(stockCritico));
    if (m_kpiLotesVencer) m_kpiLotesVencer->setText(QString("%1 lotes").arg(lotesCriticos));
    if (m_kpiMermas) m_kpiMermas->setText(QString("$%1").arg(QString::number(DatabaseManager::getInstance().getMermasMes(), 'f', 2)));

    auto alertas = DatabaseManager::getInstance().getAlertas();
    if (m_tablaAlertasDashboard) {
        m_tablaAlertasDashboard->setRowCount(alertas.size());
        for (int i = 0; i < alertas.size(); ++i) {
            const auto& a = alertas.at(i);
            m_tablaAlertasDashboard->setItem(i, 0, new QTableWidgetItem(a.insumo));
            m_tablaAlertasDashboard->setItem(i, 1, new QTableWidgetItem(a.lote));
            auto *itAcc = new QTableWidgetItem(a.accion);
            if (a.esPeligro) itAcc->setForeground(QColor(danger));
            m_tablaAlertasDashboard->setItem(i, 2, itAcc);
        }
    }

    // 2. Inventario Consolidado (4.3)
    if (m_tablaInventarioConsolidado) {
        auto consolidado = DatabaseManager::getInstance().getInventarioConsolidado();
        m_tablaInventarioConsolidado->setRowCount(consolidado.size());
        for (int i = 0; i < consolidado.size(); ++i) {
            const auto& c = consolidado.at(i);
            m_tablaInventarioConsolidado->setItem(i, 0, new QTableWidgetItem(c.nombreInsumo));
            m_tablaInventarioConsolidado->setItem(i, 1, new QTableWidgetItem(c.detallePresentacion));
            m_tablaInventarioConsolidado->setItem(i, 2, new QTableWidgetItem(QString("%1 %2").arg(c.stockMinimo).arg(c.unidadMedida)));
            auto *itSem = new QTableWidgetItem(c.estadoSemaforo);
            if (c.estadoSemaforo == "Crítico") itSem->setForeground(QColor(danger));
            else if (c.estadoSemaforo == "Bajo") itSem->setForeground(QColor(warning));
            else itSem->setForeground(QColor(success));
            m_tablaInventarioConsolidado->setItem(i, 3, itSem);
        }
    }

    // 3. Recepciones de Lotes (4.4)
    if (m_tablaRecepcionesLotes) {
        auto lotes = DatabaseManager::getInstance().getLotes();
        m_tablaRecepcionesLotes->setRowCount(lotes.size());
        for (int i = 0; i < lotes.size(); ++i) {
            const auto& l = lotes.at(i);
            m_tablaRecepcionesLotes->setItem(i, 0, new QTableWidgetItem(l.nombreInsumo));
            m_tablaRecepcionesLotes->setItem(i, 1, new QTableWidgetItem(l.codigoLote));
            m_tablaRecepcionesLotes->setItem(i, 2, new QTableWidgetItem(l.fechaCaducidad.toString("yyyy-MM-dd")));
            m_tablaRecepcionesLotes->setItem(i, 3, new QTableWidgetItem(QString::number(l.cantidadRestante, 'f', 2)));
            m_tablaRecepcionesLotes->setItem(i, 4, new QTableWidgetItem(QString::number(l.cantidadInicial, 'f', 2)));
            auto *itEst = new QTableWidgetItem(l.estado);
            if (l.estado == "AGOTADO") itEst->setForeground(QColor(mutedText));
            else if (l.proximoAVencer()) itEst->setForeground(QColor(danger));
            m_tablaRecepcionesLotes->setItem(i, 5, itEst);
        }
    }

    // 4. Mermas
    if (m_tablaMermas) {
        auto mermas = DatabaseManager::getInstance().getMermas();
        m_tablaMermas->setRowCount(mermas.size());
        for (int i = 0; i < mermas.size(); ++i) {
            const auto& m = mermas.at(i);
            m_tablaMermas->setItem(i, 0, new QTableWidgetItem(m.nombreInsumo));
            m_tablaMermas->setItem(i, 1, new QTableWidgetItem(m.codigoLote));
            m_tablaMermas->setItem(i, 2, new QTableWidgetItem(QString("%1 %2").arg(m.cantidad).arg(m.unidadMedida)));
            m_tablaMermas->setItem(i, 3, new QTableWidgetItem(m.motivo));
            m_tablaMermas->setItem(i, 4, new QTableWidgetItem(QString("$%1").arg(QString::number(m.costoImpacto, 'f', 2))));
            m_tablaMermas->setItem(i, 5, new QTableWidgetItem(m.usuario));
        }
    }

    // 5. Recetas / Productos (4.6)
    if (m_tablaRecetasProductos) {
        auto prods = DatabaseManager::getInstance().getProductos();
        m_tablaRecetasProductos->setRowCount(prods.size());
        for (int i = 0; i < prods.size(); ++i) {
            const auto& p = prods.at(i);
            m_tablaRecetasProductos->setItem(i, 0, new QTableWidgetItem(p.nombreProd));
            m_tablaRecetasProductos->setItem(i, 1, new QTableWidgetItem(p.categoria));
            m_tablaRecetasProductos->setItem(i, 2, new QTableWidgetItem(QString("$%1").arg(QString::number(p.precioVenta, 'f', 2))));
            m_tablaRecetasProductos->setItem(i, 3, new QTableWidgetItem(QString("$%1").arg(QString::number(p.costoProduccionTeorico, 'f', 2))));
            double margen = p.precioVenta - p.costoProduccionTeorico;
            double pct = (p.precioVenta > 0) ? (margen / p.precioVenta) * 100.0 : 0.0;
            m_tablaRecetasProductos->setItem(i, 4, new QTableWidgetItem(QString("$%1 (%2%)").arg(QString::number(margen, 'f', 2)).arg(QString::number(pct, 'f', 0))));
        }
        if (!prods.isEmpty() && m_tablaEscandalloDetalle && m_tablaEscandalloDetalle->rowCount() == 0) {
            onProductoRecetaSeleccionado(0, 0);
        }
    }

    // 6. Alertas
    if (m_tablaAlertas) {
        m_tablaAlertas->setRowCount(alertas.size());
        for (int i = 0; i < alertas.size(); ++i) {
            const auto& a = alertas.at(i);
            m_tablaAlertas->setItem(i, 0, new QTableWidgetItem(a.insumo));
            m_tablaAlertas->setItem(i, 1, new QTableWidgetItem(a.lote));
            auto *itNivel = new QTableWidgetItem(a.estado);
            if (a.esPeligro) itNivel->setForeground(QColor(danger));
            m_tablaAlertas->setItem(i, 2, itNivel);
            m_tablaAlertas->setItem(i, 3, new QTableWidgetItem(a.accion));
        }
    }

    // 7. Órdenes de compra
    if (m_tablaOrdenes) {
        auto ords = DatabaseManager::getInstance().getOrdenesCompra();
        m_tablaOrdenes->setRowCount(ords.size());
        for (int i = 0; i < ords.size(); ++i) {
            const auto& o = ords.at(i);
            m_tablaOrdenes->setItem(i, 0, new QTableWidgetItem(o.folio));
            m_tablaOrdenes->setItem(i, 1, new QTableWidgetItem(o.proveedor));
            m_tablaOrdenes->setItem(i, 2, new QTableWidgetItem(QString("$%1").arg(QString::number(o.totalEstimado, 'f', 2))));
            m_tablaOrdenes->setItem(i, 3, new QTableWidgetItem(o.estado));
            m_tablaOrdenes->setItem(i, 4, new QTableWidgetItem(o.fechaEmision.toString("yyyy-MM-dd")));
        }
    }

    // 8. Usuarios
    if (m_tablaUsuarios) {
        auto users = DatabaseManager::getInstance().getUsuarios();
        m_tablaUsuarios->setRowCount(users.size());
        for (int i = 0; i < users.size(); ++i) {
            const auto& u = users.at(i);
            m_tablaUsuarios->setItem(i, 0, new QTableWidgetItem(u.nombreCompleto));
            m_tablaUsuarios->setItem(i, 1, new QTableWidgetItem(u.nombreUsuario));
            m_tablaUsuarios->setItem(i, 2, new QTableWidgetItem(u.nombreRol));
            auto *itAct = new QTableWidgetItem(u.activo ? "Activo" : "Suspendido");
            if (!u.activo) itAct->setForeground(QColor(danger));
            m_tablaUsuarios->setItem(i, 3, itAct);
        }
    }
}

void MainWindow::abrirDialogoRegistrarLote()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Registrar Recepción de Lote (Trazabilidad PEPS)");
    dlg.setFixedSize(460, 440);
    dlg.setStyleSheet(QStringLiteral(
        "QDialog { background: %1; }"
        "QLabel { color: %2; font-size: 13px; font-weight: 500; }"
        "QComboBox { background-color: #ffffff; color: #0b0b0b; border: 1px solid %3; border-radius: 6px; padding: 6px 10px; font-size: 13px; }"
        "QComboBox:focus { border: 1.5px solid #000000; }"
        "QComboBox QAbstractItemView { background-color: #ffffff; color: #0b0b0b; border: 1px solid %3; selection-background-color: #e5e5e0; selection-color: #0b0b0b; }"
        "QLineEdit, QDoubleSpinBox, QDateEdit { background-color: #ffffff; color: #0b0b0b; border: 1px solid %3; border-radius: 6px; padding: 6px 10px; font-size: 13px; }"
        "QLineEdit:focus, QDoubleSpinBox:focus, QDateEdit:focus { border: 1.5px solid #000000; }")
        .arg(surface).arg(ink).arg(border));

    auto *ly = new QVBoxLayout(&dlg);
    ly->setContentsMargins(space5, space5, space5, space5);
    ly->setSpacing(space3);

    auto insumos = DatabaseManager::getInstance().getInsumos();
    auto *cbInsumo = new QComboBox(&dlg);
    for (const auto& ins : insumos) {
        cbInsumo->addItem(QString("%1 (%2)").arg(ins.nombreInsumo, ins.unidadMedida), ins.idInsumo);
    }

    auto *txtCodigo = new QLineEdit(&dlg);
    txtCodigo->setText(QString("LOT-%1-%2").arg(QDate::currentDate().toString("yyyyMM")).arg(QRandomGenerator::global()->bounded(100, 999)));

    auto *dtCaducidad = new QDateEdit(QDate::currentDate().addDays(30), &dlg);
    dtCaducidad->setCalendarPopup(true);

    auto *spCantidad = new QDoubleSpinBox(&dlg);
    spCantidad->setRange(1.0, 100000.0);
    spCantidad->setValue(1000.0);
    spCantidad->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto actualizarUnidadInsumo = [&]() {
        int idx = cbInsumo->currentIndex();
        if (idx >= 0 && idx < insumos.size()) {
            spCantidad->setSuffix(QString(" %1").arg(insumos.at(idx).unidadMedida));
        }
    };
    connect(cbInsumo, &QComboBox::currentIndexChanged, actualizarUnidadInsumo);
    actualizarUnidadInsumo();

    auto *spCosto = new QDoubleSpinBox(&dlg);
    spCosto->setRange(0.001, 5000.0);
    spCosto->setDecimals(3);
    spCosto->setValue(0.150);
    spCosto->setPrefix("$ ");
    spCosto->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    ly->addWidget(new QLabel("Insumo / Materia prima:", &dlg));
    ly->addWidget(cbInsumo);
    ly->addWidget(new QLabel("Código de Lote:", &dlg));
    ly->addWidget(txtCodigo);
    ly->addWidget(new QLabel("Fecha de Caducidad:", &dlg));
    ly->addWidget(dtCaducidad);
    ly->addWidget(new QLabel("Cantidad Recibida:", &dlg));
    ly->addWidget(spCantidad);
    ly->addWidget(new QLabel("Costo Unitario ($):", &dlg));
    ly->addWidget(spCosto);

    auto *btnGuardar = createPrimaryButton("Guardar Lote en Cola PEPS", &dlg);
    ly->addWidget(btnGuardar);

    connect(btnGuardar, &QPushButton::clicked, [&]() {
        if (dtCaducidad->date() < QDate::currentDate()) {
            QMessageBox::warning(&dlg, "Fecha Inválida", "No se puede recibir un lote con fecha de caducidad vencida (Regla de calidad alimentaria).");
            return;
        }
        int idIns = cbInsumo->currentData().toInt();
        bool ok = DatabaseManager::getInstance().registrarLote(idIns, txtCodigo->text(), dtCaducidad->date(), spCantidad->value(), spCosto->value());
        if (ok) {
            dlg.accept();
            QMessageBox::information(this, "Éxito", "Lote registrado y agregado exitosamente a la cola PEPS.");
            recargarDatosGenerales();
        } else {
            QMessageBox::critical(&dlg, "Error", "No se pudo registrar el lote.");
        }
    });

    dlg.exec();
}

void MainWindow::abrirDialogoRegistrarMerma()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Registrar Merma Operativa / Vencimiento");
    dlg.setFixedSize(480, 420);
    dlg.setStyleSheet(QStringLiteral(
        "QDialog { background: %1; }"
        "QLabel { color: %2; font-size: 13px; font-weight: 500; }"
        "QComboBox { background-color: #ffffff; color: #0b0b0b; border: 1px solid %3; border-radius: 6px; padding: 6px 10px; font-size: 13px; }"
        "QComboBox:focus { border: 1.5px solid #000000; }"
        "QComboBox QAbstractItemView { background-color: #ffffff; color: #0b0b0b; border: 1px solid %3; selection-background-color: #e5e5e0; selection-color: #0b0b0b; outline: none; }"
        "QDoubleSpinBox { background-color: #ffffff; color: #0b0b0b; border: 1px solid %3; border-radius: 6px; padding: 6px 10px; font-size: 13px; }"
        "QDoubleSpinBox:focus { border: 1.5px solid #000000; }")
        .arg(surface).arg(ink).arg(border));

    auto *ly = new QVBoxLayout(&dlg);
    ly->setContentsMargins(space5, space5, space5, space5);
    ly->setSpacing(space3);

    auto lotes = DatabaseManager::getInstance().getLotes();
    QList<Models::LoteAlmacen> lotesActivos;
    auto *cbLotes = new QComboBox(&dlg);

    for (const auto& l : lotes) {
        if (l.cantidadRestante > 0) {
            lotesActivos.append(l);
            QString infoLote = QString("%1 · %2 (Disponible: %3 %4)")
                .arg(l.nombreInsumo, l.codigoLote)
                .arg(QString::number(l.cantidadRestante, 'f', 0))
                .arg(l.unidadMedida);
            cbLotes->addItem(infoLote, l.idLote);
        }
    }

    auto *spCantidad = new QDoubleSpinBox(&dlg);
    spCantidad->setRange(0.01, 100000.0);
    spCantidad->setValue(100.0);
    spCantidad->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto *lblImpacto = new QLabel(&dlg);
    lblImpacto->setStyleSheet("color: #b42318; font-size: 13px; font-weight: 600; padding: 2px 0;");

    auto actualizarDetalleLote = [&]() {
        int idx = cbLotes->currentIndex();
        if (idx >= 0 && idx < lotesActivos.size()) {
            const auto& l = lotesActivos.at(idx);
            spCantidad->setSuffix(QString(" %1").arg(l.unidadMedida));
            spCantidad->setMaximum(l.cantidadRestante);
            if (l.unidadMedida.toLower().contains("pieza")) {
                spCantidad->setDecimals(0);
                spCantidad->setSingleStep(1);
                if (spCantidad->value() > l.cantidadRestante || spCantidad->value() < 1) {
                    spCantidad->setValue(1);
                }
            } else {
                spCantidad->setDecimals(2);
                spCantidad->setSingleStep(50);
            }
            double perdidaEstimada = spCantidad->value() * l.costoUnitario;
            lblImpacto->setText(QString("Pérdida monetaria calculada: <b>$%1 MXN</b> ($%2 / %3)")
                .arg(QString::number(perdidaEstimada, 'f', 2))
                .arg(QString::number(l.costoUnitario, 'f', 3), l.unidadMedida));
        }
    };

    connect(cbLotes, &QComboBox::currentIndexChanged, actualizarDetalleLote);
    connect(spCantidad, &QDoubleSpinBox::valueChanged, actualizarDetalleLote);
    actualizarDetalleLote();

    auto *cbMotivo = new QComboBox(&dlg);
    cbMotivo->addItem("Derrame o caída accidental en barra", "DAÑO_ACCIDENTAL");
    cbMotivo->addItem("Error de preparación o mala extracción", "ERROR_PREPARACION");
    cbMotivo->addItem("Caducidad vencida / Producto descompuesto", "CADUCIDAD_VENCIDA");
    cbMotivo->addItem("Calibración técnica de molino / pruebas", "CALIBRACION");
    cbMotivo->addItem("Defecto de empaque / Proveedor", "DEFECTO_PROVEEDOR");

    ly->addWidget(new QLabel("Lote Físico Afectado:", &dlg));
    ly->addWidget(cbLotes);
    ly->addWidget(new QLabel("Cantidad a Mermar:", &dlg));
    ly->addWidget(spCantidad);
    ly->addWidget(lblImpacto);
    ly->addWidget(new QLabel("Motivo Formal del Desperdicio:", &dlg));
    ly->addWidget(cbMotivo);
    ly->addSpacing(space2);

    auto *btnConfirmar = createPrimaryButton("Confirmar Baja de Inventario", &dlg);
    ly->addWidget(btnConfirmar);

    connect(btnConfirmar, &QPushButton::clicked, [&]() {
        if (cbLotes->count() == 0) {
            QMessageBox::warning(&dlg, "Sin Lotes", "No hay lotes con inventario disponible para mermar.");
            return;
        }
        int idL = cbLotes->currentData().toInt();
        int idU = m_usuario.idUsuario > 0 ? m_usuario.idUsuario : 1;
        QString motivo = cbMotivo->currentData().toString();
        bool ok = DatabaseManager::getInstance().registrarMerma(idL, idU, spCantidad->value(), motivo);
        if (ok) {
            dlg.accept();
            QMessageBox::information(this, "Merma Registrada", "Inventario actualizado y registrado en auditoría.");
            recargarDatosGenerales();
        } else {
            QMessageBox::warning(&dlg, "Error", "No se pudo registrar la merma (verifique disponibilidad del lote).");
        }
    });

    dlg.exec();
}

void MainWindow::abrirDialogoNuevaReceta()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Nuevo Producto del Menú y Escandallo");
    dlg.setFixedSize(460, 420);
    dlg.setStyleSheet(QStringLiteral(
        "QDialog { background: %1; }"
        "QLabel { color: %2; font-size: 13px; font-weight: 500; }"
        "QComboBox { background-color: #ffffff; color: #0b0b0b; border: 1px solid %3; border-radius: 6px; padding: 6px 10px; font-size: 13px; }"
        "QComboBox:focus { border: 1.5px solid #000000; }"
        "QComboBox QAbstractItemView { background-color: #ffffff; color: #0b0b0b; border: 1px solid %3; selection-background-color: #e5e5e0; selection-color: #0b0b0b; }"
        "QLineEdit, QDoubleSpinBox { background-color: #ffffff; color: #0b0b0b; border: 1px solid %3; border-radius: 6px; padding: 6px 10px; font-size: 13px; }"
        "QLineEdit:focus, QDoubleSpinBox:focus { border: 1.5px solid #000000; }")
        .arg(surface).arg(ink).arg(border));

    auto *ly = new QVBoxLayout(&dlg);
    ly->setContentsMargins(space5, space5, space5, space5);
    ly->setSpacing(space3);

    auto *txtNombre = new QLineEdit(&dlg);
    txtNombre->setPlaceholderText("Ej. Caramel Macchiato");

    auto *cbCategoria = new QComboBox(&dlg);
    cbCategoria->addItems({"Bebidas Calientes", "Bebidas Frías", "Repostería", "Alimentos"});

    auto *spPrecio = new QDoubleSpinBox(&dlg);
    spPrecio->setRange(1.0, 1000.0);
    spPrecio->setValue(65.0);
    spPrecio->setPrefix("$ ");
    spPrecio->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto insumos = DatabaseManager::getInstance().getInsumos();
    auto *cbIngrediente = new QComboBox(&dlg);
    for (const auto& ins : insumos) {
        cbIngrediente->addItem(QString("%1 (%2)").arg(ins.nombreInsumo, ins.unidadMedida), ins.idInsumo);
    }

    auto *spPorcion = new QDoubleSpinBox(&dlg);
    spPorcion->setRange(0.1, 5000.0);
    spPorcion->setValue(18.0);
    spPorcion->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto actualizarUnidadReceta = [&]() {
        int idx = cbIngrediente->currentIndex();
        if (idx >= 0 && idx < insumos.size()) {
            spPorcion->setSuffix(QString(" %1").arg(insumos.at(idx).unidadMedida));
        }
    };
    connect(cbIngrediente, &QComboBox::currentIndexChanged, actualizarUnidadReceta);
    actualizarUnidadReceta();

    ly->addWidget(new QLabel("Nombre del Producto:", &dlg));
    ly->addWidget(txtNombre);
    ly->addWidget(new QLabel("Categoría:", &dlg));
    ly->addWidget(cbCategoria);
    ly->addWidget(new QLabel("Precio de Venta ($):", &dlg));
    ly->addWidget(spPrecio);
    ly->addWidget(new QLabel("Ingrediente Principal:", &dlg));
    ly->addWidget(cbIngrediente);
    ly->addWidget(new QLabel("Porción en Gramos/Mililitros/Piezas:", &dlg));
    ly->addWidget(spPorcion);

    auto *btnGuardar = createPrimaryButton("Guardar Producto y Receta", &dlg);
    ly->addWidget(btnGuardar);

    connect(btnGuardar, &QPushButton::clicked, [&]() {
        if (txtNombre->text().trimmed().isEmpty()) return;
        bool ok = DatabaseManager::getInstance().registrarProducto(txtNombre->text().trimmed(), cbCategoria->currentText(), spPrecio->value());
        if (ok) {
            auto prods = DatabaseManager::getInstance().getProductos();
            int idNuevoProd = prods.last().idProducto;
            int idIns = cbIngrediente->currentData().toInt();
            DatabaseManager::getInstance().registrarReceta(idNuevoProd, idIns, spPorcion->value());

            dlg.accept();
            QMessageBox::information(this, "Producto Creado", "Producto y escandallo base añadidos exitosamente.");
            recargarDatosGenerales();
        }
    });

    dlg.exec();
}

// Implementación formal de Consolidación de Órdenes de Compra (RF-05)
void MainWindow::abrirDialogoGenerarOrdenPdf()
{
    auto insumosCriticos = DatabaseManager::getInstance().getInsumosCriticosParaCompra();
    if (insumosCriticos.isEmpty()) {
        QMessageBox::information(this, "Niveles Óptimos (RF-05)",
                                 "No hay insumos por debajo del stock mínimo actualmente.\nEl almacén se encuentra completamente abastecido.");
        return;
    }

    // Agrupar por proveedor
    QMap<int, QList<Models::InsumoCriticoCompra>> porProveedor;
    QMap<int, QString> nombresProveedor;
    for (const auto& item : insumosCriticos) {
        porProveedor[item.idProveedor].append(item);
        nombresProveedor[item.idProveedor] = item.nombreProveedor;
    }

    QDialog dlg(this);
    dlg.setWindowTitle("Consolidación Automática de Compras (RF-05)");
    dlg.setFixedSize(580, 460);
    dlg.setStyleSheet(QStringLiteral(
        "QDialog { background: %1; }"
        "QLabel { color: %2; font-size: 13px; font-weight: 500; }"
        "QComboBox { background-color: #ffffff; color: #0b0b0b; border: 1px solid %3; border-radius: 6px; padding: 6px 10px; font-size: 13px; }"
        "QComboBox:focus { border: 1.5px solid #000000; }"
        "QComboBox QAbstractItemView { background-color: #ffffff; color: #0b0b0b; border: 1px solid %3; selection-background-color: #e5e5e0; selection-color: #0b0b0b; }")
        .arg(surface).arg(ink).arg(border));

    auto *ly = new QVBoxLayout(&dlg);
    ly->setSpacing(space3);

    auto *lblIntro = new QLabel("Insumos en nivel crítico agrupados por proveedor predeterminado:", &dlg);
    lblIntro->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: %2;").arg(textBody).arg(weightSemibold));
    ly->addWidget(lblIntro);

    auto *cbProv = new QComboBox(&dlg);
    for (auto it = porProveedor.begin(); it != porProveedor.end(); ++it) {
        int idProv = it.key();
        cbProv->addItem(QString("%1 (%2 insumos críticos)").arg(nombresProveedor[idProv]).arg(it.value().size()), idProv);
    }
    ly->addWidget(cbProv);

    auto *tablaPreview = createStyledTable({"Insumo", "Stock Actual", "Mínimo", "Cantidad Sugerida", "Subtotal Est."}, &dlg);
    tablaPreview->setMinimumHeight(200);
    ly->addWidget(tablaPreview);

    auto *lblTotalOrden = new QLabel(&dlg);
    lblTotalOrden->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textTitle).arg(weightBold).arg(ink));
    ly->addWidget(lblTotalOrden);

    auto actualizarTabla = [&]() {
        int idProvSel = cbProv->currentData().toInt();
        const auto& items = porProveedor[idProvSel];
        tablaPreview->setRowCount(items.size());
        double total = 0.0;
        for (int i = 0; i < items.size(); ++i) {
            const auto& it = items.at(i);
            tablaPreview->setItem(i, 0, new QTableWidgetItem(it.nombreInsumo));
            tablaPreview->setItem(i, 1, new QTableWidgetItem(QString("%1 %2").arg(it.stockActual).arg(it.unidadMedida)));
            tablaPreview->setItem(i, 2, new QTableWidgetItem(QString("%1 %2").arg(it.stockMinimo).arg(it.unidadMedida)));
            tablaPreview->setItem(i, 3, new QTableWidgetItem(QString("%1 %2").arg(it.cantidadSugerida).arg(it.unidadMedida)));
            tablaPreview->setItem(i, 4, new QTableWidgetItem(QString("$%1").arg(QString::number(it.subtotalEstimado(), 'f', 2))));
            total += it.subtotalEstimado();
        }
        lblTotalOrden->setText(QString("Total Estimado de Compra: $%1 MXN").arg(QString::number(total, 'f', 2)));
    };

    connect(cbProv, &QComboBox::currentIndexChanged, actualizarTabla);
    actualizarTabla();

    auto *btnGenerar = createPrimaryButton("Generar y Exportar Orden a PDF", &dlg);
    ly->addWidget(btnGenerar);

    connect(btnGenerar, &QPushButton::clicked, [&]() {
        int idProvSel = cbProv->currentData().toInt();
        const auto& items = porProveedor[idProvSel];
        QString nombreProv = nombresProveedor[idProvSel];

        QString destino = QFileDialog::getSaveFileName(&dlg, "Guardar Orden de Compra PDF", QString("Orden_Compra_%1.pdf").arg(nombreProv.replace(" ", "_")), "Archivos PDF (*.pdf)");
        if (destino.isEmpty()) return;

        QString folio = QString("OC-%1-%2").arg(QDate::currentDate().toString("yyyyMM")).arg(QRandomGenerator::global()->bounded(100, 999));
        QList<QPair<QString, double>> itemsPdf;
        QList<QPair<int, double>> itemsBd;
        double total = 0.0;

        for (const auto& it : items) {
            itemsPdf.append({QString("%1 (%2)").arg(it.nombreInsumo, it.unidadMedida), it.cantidadSugerida});
            itemsBd.append({it.idInsumo, it.cantidadSugerida});
            total += it.subtotalEstimado();
        }

        bool ok = PdfGenerator::generarOrdenCompraPdf(destino, folio, nombreProv, m_usuario.nombreCompleto, itemsPdf, total);
        if (ok) {
            int idU = m_usuario.idUsuario > 0 ? m_usuario.idUsuario : 1;
            DatabaseManager::getInstance().crearOrdenCompra(idProvSel, idU, folio, total, itemsBd);
            dlg.accept();
            QMessageBox::information(this, "Orden Emitida (RF-05)",
                                     QString("Orden de compra generada exitosamente en formato formal con firmas:\n%1").arg(destino));
            recargarDatosGenerales();
        } else {
            QMessageBox::critical(&dlg, "Error", "No se pudo generar el documento PDF.");
        }
    });

    dlg.exec();
}

void MainWindow::exportarReporteMermas()
{
    QString destino = QFileDialog::getSaveFileName(this, "Exportar Reporte de Mermas PDF", "Reporte_Mermas.pdf", "Archivos PDF (*.pdf)");
    if (destino.isEmpty()) return;

    auto mermas = DatabaseManager::getInstance().getMermas();
    bool ok = PdfGenerator::generarReporteMermasPdf(destino, mermas);
    if (ok) {
        QMessageBox::information(this, "Éxito", QString("Reporte exportado exitosamente a:\n%1").arg(destino));
    } else {
        QMessageBox::critical(this, "Error", "No se pudo generar el reporte en PDF.");
    }
}

void MainWindow::exportarReporteVentas()
{
    QString destino = QFileDialog::getSaveFileName(this, "Exportar Resumen de Ventas PDF", "Resumen_Ventas_Noviembre.pdf", "Archivos PDF (*.pdf)");
    if (destino.isEmpty()) return;

    bool ok = PdfGenerator::generarReporteVentasPdf(destino);
    if (ok) {
        QMessageBox::information(this, "Éxito", QString("Reporte de ventas exportado exitosamente a:\n%1").arg(destino));
    } else {
        QMessageBox::critical(this, "Error", "No se pudo generar el reporte de ventas en PDF.");
    }
}

void MainWindow::exportarReporteCsv()
{
    QString destino = QFileDialog::getSaveFileName(this, "Exportar Histórico PEPS CSV", "Movimientos_PEPS.csv", "Archivos CSV (*.csv)");
    if (destino.isEmpty()) return;

    bool ok = PdfGenerator::exportarMovimientosCsv(destino);
    if (ok) {
        QMessageBox::information(this, "Éxito", QString("Histórico CSV exportado exitosamente a:\n%1").arg(destino));
    } else {
        QMessageBox::critical(this, "Error", "No se pudo exportar el archivo CSV.");
    }
}

void MainWindow::abrirDialogoCambiarUsuario()
{
    LoginDialog login(this);
    if (login.exec() == QDialog::Accepted) {
        m_usuario = login.getUsuarioAutenticado();
        configurarVistas();
        recargarDatosGenerales();
    }
}
