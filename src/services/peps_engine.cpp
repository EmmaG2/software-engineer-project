#include "peps_engine.h"
#include "../db/databasemanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>

MotorPEPS& MotorPEPS::getInstance() {
    static MotorPEPS instance;
    return instance;
}

bool MotorPEPS::procesarVenta(const QList<Models::ItemTicket>& items,
                              int idUsuario,
                              QString* errorMsg,
                              QString* outFolio) {
    if (items.isEmpty()) {
        if (errorMsg) *errorMsg = "El ticket de venta está vacío.";
        return false;
    }

    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    if (!db.isOpen()) {
        if (errorMsg) *errorMsg = "La base de datos no está conectada.";
        return false;
    }

    // 1. Iniciar transacción atómica ACID
    if (!db.transaction()) {
        if (errorMsg) *errorMsg = "No se pudo iniciar la transacción en la base de datos.";
        return false;
    }

    // 2. Generar folio único (ej. V-20261122-1042)
    QString folio = QString("V-%1-%2")
                        .arg(QDateTime::currentDateTime().toString("yyyyMMdd"))
                        .arg(QRandomGenerator::global()->bounded(1000, 9999));
    if (outFolio) *outFolio = folio;

    double totalVenta = 0.0;
    for (const auto& item : items) {
        totalVenta += item.subtotal();
    }

    // 3. Insertar cabecera de venta
    QSqlQuery qVenta(db);
    qVenta.prepare("INSERT INTO venta (folio, id_usuario, fecha_hora, total_venta) "
                   "VALUES (?, ?, ?, ?);");
    qVenta.addBindValue(folio);
    qVenta.addBindValue(idUsuario);
    qVenta.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    qVenta.addBindValue(totalVenta);

    if (!qVenta.exec()) {
        db.rollback();
        if (errorMsg) *errorMsg = "Error al registrar la venta: " + qVenta.lastError().text();
        return false;
    }

    int idVenta = qVenta.lastInsertId().toInt();

    // 4. Procesar cada producto del ticket y sus recetas asociadas
    for (const auto& item : items) {
        // Insertar detalle de venta
        QSqlQuery qDet(db);
        qDet.prepare("INSERT INTO venta_detalle (id_venta, id_producto, cantidad, precio_unitario, subtotal) "
                     "VALUES (?, ?, ?, ?, ?);");
        qDet.addBindValue(idVenta);
        qDet.addBindValue(item.idProducto);
        qDet.addBindValue(item.cantidad);
        qDet.addBindValue(item.precioUnitario);
        qDet.addBindValue(item.subtotal());

        if (!qDet.exec()) {
            db.rollback();
            if (errorMsg) *errorMsg = "Error al insertar detalle de venta: " + qDet.lastError().text();
            return false;
        }

        // Consultar escandallo (ingredientes de la receta)
        QSqlQuery qReceta(db);
        qReceta.prepare("SELECT id_insumo, cantidad_descuento FROM receta_detalle WHERE id_producto = ?;");
        qReceta.addBindValue(item.idProducto);

        if (!qReceta.exec()) {
            db.rollback();
            if (errorMsg) *errorMsg = "Error al consultar receta: " + qReceta.lastError().text();
            return false;
        }

        while (qReceta.next()) {
            int idInsumo = qReceta.value(0).toInt();
            double porcionUnitaria = qReceta.value(1).toDouble();
            double cantidadTotalRequerida = porcionUnitaria * item.cantidad;

            // Ejecutar deducción PEPS por insumo
            if (!descontarInsumoPEPS(db, idInsumo, cantidadTotalRequerida, idUsuario, idVenta, errorMsg)) {
                db.rollback();
                return false; // Error o stock insuficiente; aborta toda la venta
            }
        }
    }

    // 5. Confirmar transacción atómica
    if (!db.commit()) {
        db.rollback();
        if (errorMsg) *errorMsg = "Error al consolidar la transacción (COMMIT fallido).";
        return false;
    }

    qInfo() << "[MotorPEPS] Venta procesada exitosamente con folio:" << folio << "Total:" << totalVenta;
    return true;
}

bool MotorPEPS::descontarInsumoPEPS(QSqlDatabase& db,
                                   int idInsumo,
                                   double cantidadTotalRequerida,
                                   int idUsuario,
                                   int idVenta,
                                   QString* errorMsg) {
    // 1. Consultar lotes activos ordenados por caducidad ascendente (Regla PEPS / FIFO)
    QSqlQuery qLotes(db);
    qLotes.prepare("SELECT id_lote, cantidad_restante, fecha_caducidad, costo_unitario FROM lote "
                   "WHERE id_insumo = ? AND cantidad_restante > 0 AND estado = 'ACTIVO_PEPS' "
                   "ORDER BY fecha_caducidad ASC;");
    qLotes.addBindValue(idInsumo);

    if (!qLotes.exec()) {
        if (errorMsg) *errorMsg = "Error al consultar lotes PEPS: " + qLotes.lastError().text();
        return false;
    }

    double cantidadPendiente = cantidadTotalRequerida;

    while (qLotes.next() && cantidadPendiente > 0.0001) {
        int idLote = qLotes.value(0).toInt();
        double stockLote = qLotes.value(1).toDouble();
        double costoUnitario = qLotes.value(3).toDouble();

        if (stockLote >= cantidadPendiente) {
            // El lote actual cubre totalmente lo que falta
            double nuevoStock = stockLote - cantidadPendiente;
            QString nuevoEstado = (nuevoStock <= 0.0001) ? "AGOTADO" : "ACTIVO_PEPS";

            QSqlQuery qUp(db);
            qUp.prepare("UPDATE lote SET cantidad_restante = ?, estado = ? WHERE id_lote = ?;");
            qUp.addBindValue(nuevoStock);
            qUp.addBindValue(nuevoEstado);
            qUp.addBindValue(idLote);
            if (!qUp.exec()) {
                if (errorMsg) *errorMsg = "Error al actualizar lote: " + qUp.lastError().text();
                return false;
            }

            // Registrar movimiento de auditoría
            QSqlQuery qMov(db);
            qMov.prepare("INSERT INTO movimiento_inventario (id_lote, id_usuario, id_venta, tipo_movimiento, cantidad, costo_impacto, motivo, fecha_hora) "
                         "VALUES (?, ?, ?, 'VENTA_PEPS_PARCIAL', ?, ?, 'Venta de comanda en mostrador', ?);");
            qMov.addBindValue(idLote);
            qMov.addBindValue(idUsuario);
            qMov.addBindValue(idVenta);
            qMov.addBindValue(cantidadPendiente);
            qMov.addBindValue(cantidadPendiente * costoUnitario);
            qMov.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
            if (!qMov.exec()) {
                if (errorMsg) *errorMsg = "Error al registrar movimiento: " + qMov.lastError().text();
                return false;
            }

            cantidadPendiente = 0.0;
            break;
        } else {
            // Agota el lote actual y continúa con el siguiente lote
            QSqlQuery qUp(db);
            qUp.prepare("UPDATE lote SET cantidad_restante = 0, estado = 'AGOTADO' WHERE id_lote = ?;");
            qUp.addBindValue(idLote);
            if (!qUp.exec()) {
                if (errorMsg) *errorMsg = "Error al agotar lote: " + qUp.lastError().text();
                return false;
            }

            // Registrar movimiento de auditoría
            QSqlQuery qMov(db);
            qMov.prepare("INSERT INTO movimiento_inventario (id_lote, id_usuario, id_venta, tipo_movimiento, cantidad, costo_impacto, motivo, fecha_hora) "
                         "VALUES (?, ?, ?, 'VENTA_PEPS_AGOTADO', ?, ?, 'Lote agotado por venta en mostrador', ?);");
            qMov.addBindValue(idLote);
            qMov.addBindValue(idUsuario);
            qMov.addBindValue(idVenta);
            qMov.addBindValue(stockLote);
            qMov.addBindValue(stockLote * costoUnitario);
            qMov.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
            if (!qMov.exec()) {
                if (errorMsg) *errorMsg = "Error al registrar movimiento: " + qMov.lastError().text();
                return false;
            }

            cantidadPendiente -= stockLote;
        }
    }

    // Si aún quedó cantidad pendiente, no hay suficiente stock en los lotes
    if (cantidadPendiente > 0.0001) {
        // Obtener nombre del insumo para mensaje amigable
        QSqlQuery qNom(db);
        qNom.prepare("SELECT nombre_insumo, unidad_medida FROM insumo WHERE id_insumo = ?;");
        qNom.addBindValue(idInsumo);
        QString nom = "desconocido";
        QString um = "";
        if (qNom.exec() && qNom.next()) {
            nom = qNom.value(0).toString();
            um = qNom.value(1).toString();
        }

        if (errorMsg) {
            *errorMsg = QString("Stock insuficiente en lotes para: %1 (faltan %2 %3)").arg(nom).arg(cantidadPendiente).arg(um);
        }
        return false;
    }

    // Actualizar stock_actual consolidado en la tabla insumo
    QSqlQuery qIns(db);
    qIns.prepare("UPDATE insumo SET stock_actual = CASE WHEN stock_actual >= ? THEN stock_actual - ? ELSE 0 END WHERE id_insumo = ?;");
    qIns.addBindValue(cantidadTotalRequerida);
    qIns.addBindValue(cantidadTotalRequerida);
    qIns.addBindValue(idInsumo);
    if (!qIns.exec()) {
        if (errorMsg) *errorMsg = "Error al actualizar stock de insumo: " + qIns.lastError().text();
        return false;
    }

    return true;
}
