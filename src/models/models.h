#pragma once

#include <QString>
#include <QDateTime>
#include <QList>

namespace Models {

struct Rol {
    int idRol{0};
    QString nombreRol; // "ADMIN", "CAJERO"
    QString descripcion;
};

struct Usuario {
    int idUsuario{0};
    int idRol{0};
    QString nombreRol;
    QString nombreUsuario;
    QString nombreCompleto;
    QString passwordHash;
    bool activo{true};
};

struct Proveedor {
    int idProveedor{0};
    QString nombreProveedor;
    QString contacto;
    QString telefono;
    QString email;
    QString direccion;
};

struct Insumo {
    int idInsumo{0};
    int idProveedorPredeterminado{0};
    QString nombreProveedor;
    QString nombreInsumo;
    QString unidadMedida; // "Gramos", "Mililitros", "Piezas"
    double stockActual{0.0};
    double stockMinimo{0.0};
    double stockMaximo{0.0};

    bool esCritico() const {
        return stockActual <= stockMinimo;
    }
};

struct InsumoConsolidado {
    int idInsumo{0};
    QString nombreInsumo;
    QString unidadMedida;
    double stockActual{0.0};
    double stockMinimo{0.0};
    QString estadoSemaforo; // "Óptimo", "Bajo", "Crítico"
    QString detallePresentacion; // Ej. "48.2 kg disponibles"
};

struct InsumoCriticoCompra {
    int idInsumo{0};
    QString nombreInsumo;
    QString unidadMedida;
    double stockActual{0.0};
    double stockMinimo{0.0};
    double stockMaximo{0.0};
    double cantidadSugerida{0.0}; // stockMaximo - stockActual
    double costoEstimadoUnitario{0.0};
    int idProveedor{0};
    QString nombreProveedor;

    double subtotalEstimado() const {
        return cantidadSugerida * costoEstimadoUnitario;
    }
};

struct LoteAlmacen {
    int idLote{0};
    int idInsumo{0};
    QString nombreInsumo;
    QString unidadMedida;
    QString codigoLote;
    QDate fechaIngreso;
    QDate fechaCaducidad;
    double cantidadInicial{0.0};
    double cantidadRestante{0.0};
    double costoUnitario{0.0};
    QString estado; // "ACTIVO_PEPS", "AGOTADO", "BAJA_CADUCADO", "BAJA_MERMA"

    int diasParaCaducar() const {
        return QDate::currentDate().daysTo(fechaCaducidad);
    }

    bool proximoAVencer() const {
        return diasParaCaducar() <= 3 && diasParaCaducar() >= 0;
    }

    bool caducado() const {
        return diasParaCaducar() < 0;
    }
};

struct RecetaIngrediente {
    int idReceta{0};
    int idInsumo{0};
    QString nombreInsumo;
    QString unidadMedida;
    double cantidadDescuento{0.0};
    double costoUnitarioInsumo{0.0};

    double costoPorcion() const {
        return cantidadDescuento * costoUnitarioInsumo;
    }
};

struct ProductoMenu {
    int idProducto{0};
    QString nombreProd;
    QString categoria;
    double precioVenta{0.0};
    bool activo{true};
    double costoProduccionTeorico{0.0};
    QList<RecetaIngrediente> receta;
};

struct ItemTicket {
    int idProducto{0};
    QString nombreProd;
    int cantidad{1};
    double precioUnitario{0.0};

    double subtotal() const {
        return cantidad * precioUnitario;
    }
};

struct AlertaItem {
    QString insumo;
    QString lote;
    QString estado; // "Crítico", "Bajo", "Óptimo"
    QString accion; // "Crítico · usar hoy", "Bajo · reordenar", etc.
    QString detalle;
    bool esPeligro{false};
};

struct MermaItem {
    int idMovimiento{0};
    int idLote{0};
    QString codigoLote;
    QString nombreInsumo;
    double cantidad{0.0};
    QString unidadMedida;
    QString motivo; // "CADUCIDAD_VENCIDA", "DAÑO_ACCIDENTAL", "ERROR_PREPARACION"
    double costoImpacto{0.0};
    QDateTime fechaHora;
    QString usuario;
};

struct OrdenCompraItem {
    int idOrden{0};
    QString folio;
    QString proveedor;
    int totalInsumos{0};
    double totalEstimado{0.0};
    QString estado; // "EMITIDA", "ENVIADA", "RECIBIDA"
    QDateTime fechaEmision;
};

} // namespace Models
