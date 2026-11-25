#pragma once

#include <QList>
#include <QString>
#include <QSqlDatabase>
#include "../models/models.h"

class MotorPEPS {
public:
    static MotorPEPS& getInstance();

    // Ejecuta la transacción completa de venta y descuento PEPS
    bool procesarVenta(const QList<Models::ItemTicket>& items,
                       int idUsuario,
                       QString* errorMsg = nullptr,
                       QString* outFolio = nullptr);

    // Deducción secuencial por insumo según regla FIFO estricta
    bool descontarInsumoPEPS(QSqlDatabase& db,
                             int idInsumo,
                             double cantidadTotalRequerida,
                             int idUsuario,
                             int idVenta,
                             QString* errorMsg = nullptr);

private:
    MotorPEPS() = default;
};
