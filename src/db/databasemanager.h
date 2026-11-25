#pragma once

#include <QDate>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <memory>

#include "../models/models.h"

class DatabaseManager {
public:
    static DatabaseManager& getInstance();

    // Ciclo de vida y conexión
    bool connectToDatabase(const QString& host = "127.0.0.1",
                           int port = 5432,
                           const QString& dbName = "stockcafe_db",
                           const QString& user = "postgres",
                           const QString& password = "postgres_password");
    void disconnect();
    bool isConnected() const;
    QString connectionType() const; // "PostgreSQL" o "SQLite (Fallback local)"

    // Consultas del Sistema
    QList<Models::ProductoMenu> getProductos();
    QList<Models::Insumo> getInsumos();
    QList<Models::InsumoConsolidado> getInventarioConsolidado();
    QList<Models::InsumoCriticoCompra> getInsumosCriticosParaCompra();
    QList<Models::RecetaIngrediente> getIngredientesReceta(int idProducto);
    double calcularCostoProduccion(int idProducto);

    QList<Models::LoteAlmacen> getLotes();
    QList<Models::AlertaItem> getAlertas();
    QList<Models::MermaItem> getMermas();
    QList<Models::OrdenCompraItem> getOrdenesCompra();
    QList<Models::Usuario> getUsuarios();
    QList<Models::Proveedor> getProveedores();

    // Altas y Transacciones
    bool registrarLote(int idInsumo, const QString& codigo, const QDate& caducidad, double cantidad, double costo);
    bool registrarMerma(int idLote, int idUsuario, double cantidad, const QString& motivo);
    bool registrarProducto(const QString& nombre, const QString& categoria, double precio);
    bool registrarReceta(int idProducto, int idInsumo, double cantidad);
    bool crearOrdenCompra(int idProveedor, int idUsuario, const QString& folio, double total, const QList<QPair<int, double>>& insumos);

    // Métricas del Dashboard
    double getVentasHoy();
    int getStockCriticoCount();
    int getLotesPorVencerCount();
    double getMermasMes();

    // Autenticación RBAC
    std::optional<Models::Usuario> autenticarUsuario(const QString& username, const QString& password);

    // Acceso directo a base de datos para transacciones
    QSqlDatabase getDatabase() const;

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    void initializeFallbackSchema();

    mutable QMutex m_mutex;
    QSqlDatabase m_db;
    bool m_connected{false};
    QString m_connectionType{"Desconectado"};
};
