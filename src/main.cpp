#include <QApplication>
#include <QIcon>
#include <QStyleFactory>
#include "mainwindow.h"
#include "db/databasemanager.h"
#include "ui/logindialog.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setWindowIcon(QIcon(":/assets/cafe-ops.svg"));

    // Establecer estilo Fusion para eliminar artefactos y acentos morados nativos del SO
    application.setStyle(QStyleFactory::create("Fusion"));

    // Estilos globales de controles y cajas de texto: contorno negro en focus, sin fondos blancos molestos ni acentos morados
    application.setStyleSheet(QStringLiteral(
        "QWidget { outline: none; }"
        "QLineEdit, QSpinBox, QDoubleSpinBox, QDateEdit, QTextEdit, QPlainTextEdit {"
        "    background-color: #ffffff;"
        "    color: #0b0b0b;"
        "    border: 1px solid #d6d6d2;"
        "    border-radius: 6px;"
        "    padding: 6px 10px;"
        "    selection-background-color: #000000;"
        "    selection-color: #ffffff;"
        "    outline: none;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {"
        "    border: 1.5px solid #000000;"
        "    outline: none;"
        "}"
        "QComboBox {"
        "    background-color: #ffffff;"
        "    color: #0b0b0b;"
        "    border: 1px solid #d6d6d2;"
        "    border-radius: 6px;"
        "    padding: 6px 10px;"
        "    min-height: 24px;"
        "    selection-background-color: #e5e5e0;"
        "    selection-color: #0b0b0b;"
        "}"
        "QComboBox:focus {"
        "    border: 1.5px solid #000000;"
        "}"
        "QComboBox::drop-down {"
        "    subcontrol-origin: padding;"
        "    subcontrol-position: top right;"
        "    width: 24px;"
        "    border-left: 1px solid #d6d6d2;"
        "    background-color: #f0f0ed;"
        "    border-top-right-radius: 6px;"
        "    border-bottom-right-radius: 6px;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background-color: #ffffff;"
        "    color: #0b0b0b;"
        "    border: 1px solid #d6d6d2;"
        "    border-radius: 4px;"
        "    selection-background-color: #e5e5e0;"
        "    selection-color: #0b0b0b;"
        "    outline: none;"
        "    padding: 2px;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "    min-height: 26px;"
        "    padding: 4px 8px;"
        "    color: #0b0b0b;"
        "    background-color: #ffffff;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "    background-color: #e5e5e0;"
        "    color: #0b0b0b;"
        "}"
        "QTableWidget {"
        "    outline: none;"
        "    selection-background-color: #e5e5e0;"
        "    selection-color: #0b0b0b;"
        "}"
        "QTableWidget::item {"
        "    outline: none;"
        "}"
        "QTableWidget::item:focus {"
        "    outline: none;"
        "    border: 1.5px solid #000000;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #e5e5e0;"
        "    color: #0b0b0b;"
        "    outline: none;"
        "}"
        "QMessageBox {"
        "    background-color: #ffffff;"
        "}"
        "QMessageBox QLabel {"
        "    color: #0b0b0b;"
        "    font-size: 13px;"
        "    font-weight: 500;"
        "    min-height: 40px;"
        "}"
        "QMessageBox QPushButton {"
        "    background-color: #0b0b0b;"
        "    color: #ffffff;"
        "    border: 1px solid #0b0b0b;"
        "    border-radius: 6px;"
        "    padding: 6px 20px;"
        "    font-weight: 600;"
        "    font-size: 13px;"
        "    min-width: 80px;"
        "    outline: none;"
        "}"
        "QMessageBox QPushButton:hover {"
        "    background-color: #2a2a2a;"
        "}"
        "QMessageBox QPushButton:focus {"
        "    border: 1.5px solid #000000;"
        "}"
    ));

    // 1. Inicializar conexión a la base de datos (PostgreSQL con fallback SQLite)
    DatabaseManager::getInstance().connectToDatabase();

    // 2. Desplegar diálogo de autenticación con RBAC
    LoginDialog login;
    Models::Usuario usuario;

    if (login.exec() == QDialog::Accepted) {
        usuario = login.getUsuarioAutenticado();
    } else {
        // Fallback resiliente: Si el usuario cancela o cierra la ventana de login,
        // la aplicación no se apaga; inicia con la sesión de Gerente Administrador
        auto adminOpt = DatabaseManager::getInstance().autenticarUsuario("maria.torres", "admin123");
        if (adminOpt.has_value()) {
            usuario = adminOpt.value();
        } else {
            usuario.idUsuario = 1;
            usuario.nombreUsuario = "maria.torres";
            usuario.nombreCompleto = "María Torres";
            usuario.nombreRol = "ADMIN";
        }
    }

    MainWindow window(usuario);
    window.show();
    return application.exec();
}
