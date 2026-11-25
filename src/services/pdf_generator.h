#pragma once

#include <QString>
#include <QList>
#include "../models/models.h"

class PdfGenerator {
public:
    static bool generarOrdenCompraPdf(const QString& rutaArchivo,
                                      const QString& folio,
                                      const QString& nombreProveedor,
                                      const QString& usuarioEmite,
                                      const QList<QPair<QString, double>>& insumos,
                                      double totalEstimado);

    static bool generarReporteMermasPdf(const QString& rutaArchivo,
                                        const QList<Models::MermaItem>& mermas);

    static bool generarReporteVentasPdf(const QString& rutaArchivo);

    static bool exportarMovimientosCsv(const QString& rutaArchivo);
};
