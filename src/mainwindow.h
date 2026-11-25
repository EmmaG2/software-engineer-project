#pragma once

#include <QMainWindow>
#include <QList>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QStackedWidget>
#include <QTimer>
#include "models/models.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const Models::Usuario& usuarioActual = {}, QWidget *parent = nullptr);

public slots:
    void recargarDatosGenerales();
    void verificarAlertasPeriodicas();

private slots:
    void agregarProductoAlTicket(int idProducto, const QString& nombre, double precio);
    void procesarCobroVenta();
    void abrirDialogoRegistrarLote();
    void abrirDialogoRegistrarMerma();
    void abrirDialogoNuevaReceta();
    void abrirDialogoGenerarOrdenPdf();
    void exportarReporteVentas();
    void exportarReporteMermas();
    void exportarReporteCsv();
    void quitarItemDelTicket();
    void onProductoRecetaSeleccionado(int row, int column);
    void abrirDialogoCambiarUsuario();

private:
    void configurarVistas();
    QWidget* createDashboardPage(QWidget *parent);
    QWidget* createSalesPage(QWidget *parent);
    QWidget* createInventoryPage(QWidget *parent);
    QWidget* createReceiptsPage(QWidget *parent);
    QWidget* createWastePage(QWidget *parent);
    QWidget* createRecipesPage(QWidget *parent);
    QWidget* createAlertsPage(QWidget *parent);
    QWidget* createPurchaseOrdersPage(QWidget *parent);
    QWidget* createReportsPage(QWidget *parent);
    QWidget* createUsersPage(QWidget *parent);

    void actualizarTablaTicket();

    Models::Usuario m_usuario;
    QList<Models::ItemTicket> m_ticketActual;

    QStackedWidget* m_pages{nullptr};
    QTableWidget* m_ticketTable{nullptr};
    QLabel* m_lblTotalTicket{nullptr};

    // Referencias Dashboard
    QLabel* m_kpiVentas{nullptr};
    QLabel* m_kpiStockCritico{nullptr};
    QLabel* m_kpiLotesVencer{nullptr};
    QLabel* m_kpiMermas{nullptr};
    QTableWidget* m_tablaAlertasDashboard{nullptr};

    // Tablas de módulos
    QTableWidget* m_tablaInventarioConsolidado{nullptr};
    QTableWidget* m_tablaRecepcionesLotes{nullptr};
    QTableWidget* m_tablaMermas{nullptr};
    QTableWidget* m_tablaRecetasProductos{nullptr};
    QTableWidget* m_tablaEscandalloDetalle{nullptr};
    QLabel* m_lblEscandalloTitulo{nullptr};
    QLabel* m_lblCostoTeorico{nullptr};

    QTableWidget* m_tablaAlertas{nullptr};
    QTableWidget* m_tablaOrdenes{nullptr};
    QTableWidget* m_tablaUsuarios{nullptr};

    // Panel y badge superior de Alertas (persistente, sin pop-ups invasivos)
    QPushButton* m_btnAlertaHeader{nullptr};
    QPushButton* m_btnModuloAlertas{nullptr};

    // Timer de monitoreo periódico RF-04
    QTimer* m_timerAlertas{nullptr};
    int m_alertasCriticasPrevias{0};
};
