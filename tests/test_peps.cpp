#include <cassert>
#include <iostream>
#include <QCoreApplication>
#include <QDate>
#include "../src/db/databasemanager.h"
#include "../src/services/peps_engine.h"
#include "../src/models/models.h"

void testDatabaseConnection() {
    std::cout << "[TEST 1] Probando conexion a la Base de Datos 3FN... ";
    bool connected = DatabaseManager::getInstance().connectToDatabase();
    assert(connected && "La base de datos debe inicializarse correctamente");
    assert(DatabaseManager::getInstance().isConnected() && "DatabaseManager debe reportar estado conectado");
    std::cout << "PASO (" << DatabaseManager::getInstance().connectionType().toStdString() << ")\n";
}

void testPepsCasoPracticoMatematico() {
    std::cout << "[TEST 2] Probando Algoritmo PEPS (Caso Matematico Seccion 6.2 del Documento)... ";
    auto& dbMgr = DatabaseManager::getInstance();
    auto db = dbMgr.getDatabase();

    // 1. Crear insumo de prueba y dos lotes escalonados
    QSqlQuery q(db);
    q.exec("INSERT INTO insumo (id_insumo, nombre_insumo, unidad_medida, stock_actual, stock_minimo, stock_maximo) "
           "VALUES (999, 'Leche Prueba PEPS', 'Mililitros', 1150.00, 200.00, 2000.00);");

    // Lote A: 150 ml, vence en 2 días (mas antiguo)
    QString fechaA = QDate::currentDate().addDays(2).toString("yyyy-MM-dd");
    q.exec(QString("INSERT INTO lote (id_lote, id_insumo, codigo_lote, fecha_ingreso, fecha_caducidad, cantidad_inicial, cantidad_restante, costo_unitario, estado) "
                   "VALUES (998, 999, 'LOT-TEST-A', '2026-11-20', '%1', 150.0, 150.0, 0.03, 'ACTIVO_PEPS');").arg(fechaA));

    // Lote B: 1000 ml, vence en 15 días (mas reciente)
    QString fechaB = QDate::currentDate().addDays(15).toString("yyyy-MM-dd");
    q.exec(QString("INSERT INTO lote (id_lote, id_insumo, codigo_lote, fecha_ingreso, fecha_caducidad, cantidad_inicial, cantidad_restante, costo_unitario, estado) "
                   "VALUES (999, 999, 'LOT-TEST-B', '2026-11-20', '%1', 1000.0, 1000.0, 0.03, 'ACTIVO_PEPS');").arg(fechaB));

    // 2. Ejecutar deducción de 400 ml (correspondiente a 2 lattes de 200 ml)
    db.transaction();
    QString error;
    bool exito = MotorPEPS::getInstance().descontarInsumoPEPS(db, 999, 400.0, 1, 1, &error);
    db.commit();

    assert(exito && "La deduccion PEPS de 400 ml debe ser exitosa");

    // 3. Verificar estado del Lote A (debe quedar en 0 y AGOTADO)
    QSqlQuery qLoteA("SELECT cantidad_restante, estado FROM lote WHERE id_lote = 998;", db);
    assert(qLoteA.next());
    double stockA = qLoteA.value(0).toDouble();
    QString estadoA = qLoteA.value(1).toString();
    assert(stockA == 0.0 && "Lote A debe quedar con stock 0");
    assert(estadoA == "AGOTADO" && "Lote A debe cambiar a estado AGOTADO");

    // 4. Verificar estado del Lote B (debe quedar en 750 ml y ACTIVO_PEPS)
    QSqlQuery qLoteB("SELECT cantidad_restante, estado FROM lote WHERE id_lote = 999;", db);
    assert(qLoteB.next());
    double stockB = qLoteB.value(0).toDouble();
    QString estadoB = qLoteB.value(1).toString();
    assert(stockB == 750.0 && "Lote B debe quedar con exactamente 750 ml");
    assert(estadoB == "ACTIVO_PEPS" && "Lote B debe permanecer como ACTIVO_PEPS");

    // Limpieza
    q.exec("DELETE FROM movimiento_inventario WHERE id_lote IN (998, 999);");
    q.exec("DELETE FROM lote WHERE id_lote IN (998, 999);");
    q.exec("DELETE FROM insumo WHERE id_insumo = 999;");

    std::cout << "PASO (Lote A: 150ml->0ml [AGOTADO], Lote B: 1000ml->750ml [ACTIVO_PEPS])\n";
}

void testStockInsuficienteRollback() {
    std::cout << "[TEST 3] Probando Aborto Atómico (ROLLBACK) ante Stock Insuficiente... ";
    auto& dbMgr = DatabaseManager::getInstance();
    auto db = dbMgr.getDatabase();

    QSqlQuery q(db);
    q.exec("INSERT INTO insumo (id_insumo, nombre_insumo, unidad_medida, stock_actual, stock_minimo, stock_maximo) "
           "VALUES (888, 'Insumo Escaso', 'Gramos', 50.0, 10.0, 100.0);");
    q.exec("INSERT INTO lote (id_lote, id_insumo, codigo_lote, fecha_ingreso, fecha_caducidad, cantidad_inicial, cantidad_restante, costo_unitario, estado) "
           "VALUES (888, 888, 'LOT-ESCASO', '2026-11-20', '2027-01-01', 50.0, 50.0, 0.5, 'ACTIVO_PEPS');");

    // Intentar deducir 200 g cuando solo hay 50 g
    db.transaction();
    QString error;
    bool exito = MotorPEPS::getInstance().descontarInsumoPEPS(db, 888, 200.0, 1, 1, &error);
    db.rollback();

    assert(!exito && "Deduccion debe fallar por stock insuficiente");
    assert(!error.isEmpty() && "Debe devolver mensaje descriptivo del faltante");

    // Verificar que el stock no fue alterado tras el rollback
    QSqlQuery qVerif("SELECT cantidad_restante FROM lote WHERE id_lote = 888;", db);
    assert(qVerif.next());
    assert(qVerif.value(0).toDouble() == 50.0 && "El stock debe permanecer intacto tras rollback");

    // Limpieza
    q.exec("DELETE FROM lote WHERE id_lote = 888;");
    q.exec("DELETE FROM insumo WHERE id_insumo = 888;");

    std::cout << "PASO (Transaccion abortada y stock protegido)\n";
}

void testCalculoCaducidadSemaforo() {
    std::cout << "[TEST 4] Probando Regla Temporal de Caducidad (< 72h)... ";
    Models::LoteAlmacen l1;
    l1.fechaCaducidad = QDate::currentDate().addDays(2);
    assert(l1.diasParaCaducar() == 2);
    assert(l1.proximoAVencer() == true && "Debe clasificarse como proximo a vencer (< 72h)");
    assert(l1.caducado() == false);

    Models::LoteAlmacen l2;
    l2.fechaCaducidad = QDate::currentDate().addDays(-1);
    assert(l2.caducado() == true && "Debe clasificarse como caducado");

    std::cout << "PASO (Semaforizacion de caducidad correcta)\n";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    std::cout << "========================================================\n";
    std::cout << "   SUITE DE PRUEBAS UNITARIAS: INSUMOCAFE POS (PEPS)    \n";
    std::cout << "   Ingenieria de Software - Unidad 3                    \n";
    std::cout << "========================================================\n";

    testDatabaseConnection();
    testPepsCasoPracticoMatematico();
    testStockInsuficienteRollback();
    testCalculoCaducidadSemaforo();

    std::cout << "========================================================\n";
    std::cout << "   TODAS LAS PRUEBAS (4/4) PASARON EXITOSAMENTE         \n";
    std::cout << "========================================================\n";
    return 0;
}
