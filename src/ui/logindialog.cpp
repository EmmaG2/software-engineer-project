#include "logindialog.h"
#include "../db/databasemanager.h"
#include "../style/constants.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QIcon>
#include <QMessageBox>

using namespace Ui::Tokens;

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Café Ops · Iniciar Sesión");
    setFixedSize(440, 520);
    setStyleSheet(QStringLiteral("QDialog { background: %1; }").arg(canvas));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(space8, space8, space8, space8);
    layout->setSpacing(space4);

    // Tarjeta central blanca/clara o grafito
    auto* card = new QFrame(this);
    card->setStyleSheet(QStringLiteral(
        "QFrame { background: %1; border: 1px solid %2; border-radius: %3px; }"
    ).arg(surface).arg(border).arg(radiusMedium));

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(space6, space6, space6, space6);
    cardLayout->setSpacing(space4);

    // Marca / Logo
    auto* brandHeader = new QHBoxLayout;
    auto* iconLabel = new QLabel(card);
    iconLabel->setPixmap(QIcon(":/assets/cafe-ops.svg").pixmap(iconLarge, iconLarge));
    auto* brand = new QLabel("CAFÉ OPS", card);
    brand->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: %2; color: %3; letter-spacing: 1px;")
                         .arg(textHeading).arg(weightBold).arg(ink));
    brandHeader->addWidget(iconLabel);
    brandHeader->addWidget(brand);
    brandHeader->addStretch();
    cardLayout->addLayout(brandHeader);

    auto* subTitle = new QLabel("Inicia sesión para ingresar al sistema", card);
    subTitle->setStyleSheet(QStringLiteral("font-size: %1px; color: %2;").arg(textBody).arg(mutedText));
    cardLayout->addWidget(subTitle);

    cardLayout->addSpacing(space2);

    // Input Usuario
    auto* lblUser = new QLabel("Usuario", card);
    lblUser->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textSmall).arg(weightSemibold).arg(ink));
    m_txtUsuario = new QLineEdit(card);
    m_txtUsuario->setPlaceholderText("Ej. maria.torres o ana.lopez");
    m_txtUsuario->setText("maria.torres"); // Relleno rápido para agilidad del usuario
    m_txtUsuario->setMinimumHeight(controlHeight);
    m_txtUsuario->setStyleSheet(QStringLiteral(
        "QLineEdit { background: %1; color: %2; border: 1px solid %3; border-radius: %4px; padding: 0 %5px; font-size: %6px; outline: none; }"
        "QLineEdit:focus { border: 1.5px solid #000000; outline: none; }")
        .arg(surface).arg(ink).arg(border).arg(radiusSmall).arg(space3).arg(textBody));
    cardLayout->addWidget(lblUser);
    cardLayout->addWidget(m_txtUsuario);

    // Input Contraseña
    auto* lblPass = new QLabel("Contraseña", card);
    lblPass->setStyleSheet(QStringLiteral("font-size: %1px; font-weight: %2; color: %3;").arg(textSmall).arg(weightSemibold).arg(ink));
    m_txtPassword = new QLineEdit(card);
    m_txtPassword->setEchoMode(QLineEdit::Password);
    m_txtPassword->setPlaceholderText("••••••••");
    m_txtPassword->setText("admin123");
    m_txtPassword->setMinimumHeight(controlHeight);
    m_txtPassword->setStyleSheet(QStringLiteral(
        "QLineEdit { background: %1; color: %2; border: 1px solid %3; border-radius: %4px; padding: 0 %5px; font-size: %6px; outline: none; }"
        "QLineEdit:focus { border: 1.5px solid #000000; outline: none; }")
        .arg(surface).arg(ink).arg(border).arg(radiusSmall).arg(space3).arg(textBody));
    cardLayout->addWidget(lblPass);
    cardLayout->addWidget(m_txtPassword);

    // Mensaje de Error
    m_lblError = new QLabel(card);
    m_lblError->setStyleSheet(QStringLiteral("color: %1; font-size: %2px; font-weight: %3;").arg(danger).arg(textSmall).arg(weightMedium));
    m_lblError->setVisible(false);
    cardLayout->addWidget(m_lblError);

    // Botón Iniciar Sesión
    m_btnIngresar = new QPushButton("Iniciar sesión", card);
    m_btnIngresar->setMinimumHeight(controlHeight);
    m_btnIngresar->setCursor(Qt::PointingHandCursor);
    m_btnIngresar->setStyleSheet(QStringLiteral(
        "QPushButton { background: %1; color: %2; border: 1px solid %1; border-radius: %3px; font-weight: %4; font-size: %5px; }"
        "QPushButton:hover { background: %6; }"
        "QPushButton:pressed { background: %7; }"
        "QPushButton:disabled { background: %8; color: %9; border-color: %8; }")
        .arg(ink).arg(surface).arg(radiusSmall).arg(weightSemibold).arg(textBody).arg(sidebarHover).arg(focus).arg(surfaceMuted).arg(mutedText));
    cardLayout->addWidget(m_btnIngresar);

    // Ayuda de Credenciales Demo
    m_lblInfoDemo = new QLabel("Demo: maria.torres (admin123) | ana.lopez (cajero123)", card);
    m_lblInfoDemo->setStyleSheet(QStringLiteral("font-size: %1px; color: %2; text-align: center;").arg(textSmall).arg(mutedText));
    m_lblInfoDemo->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(m_lblInfoDemo);

    layout->addWidget(card);

    connect(m_btnIngresar, &QPushButton::clicked, this, &LoginDialog::intentarLogin);
    connect(m_txtPassword, &QLineEdit::returnPressed, this, &LoginDialog::intentarLogin);
    connect(m_txtUsuario, &QLineEdit::returnPressed, this, &LoginDialog::intentarLogin);

    m_timerBloqueo = new QTimer(this);
    connect(m_timerBloqueo, &QTimer::timeout, this, &LoginDialog::desbloquearIntento);
}

void LoginDialog::intentarLogin() {
    QString usuario = m_txtUsuario->text().trimmed();
    QString pass = m_txtPassword->text();

    if (usuario.isEmpty() || pass.isEmpty()) {
        m_lblError->setText("Por favor ingrese usuario y contraseña.");
        m_lblError->setVisible(true);
        return;
    }

    auto auth = DatabaseManager::getInstance().autenticarUsuario(usuario, pass);
    if (auth.has_value()) {
        m_usuarioActual = auth.value();
        accept(); // Cierra con éxito y retorna QDialog::Accepted
    } else {
        m_intentosFallidos++;
        m_txtPassword->clear();
        m_lblError->setVisible(true);

        if (m_intentosFallidos >= 3) {
            // Regla RF-01: Bloqueo de 60 segundos por 3 intentos fallidos
            m_segundosRestantes = 60;
            m_btnIngresar->setEnabled(false);
            m_txtUsuario->setEnabled(false);
            m_txtPassword->setEnabled(false);
            m_lblError->setText(QString("Acceso bloqueado por seguridad. Espere %1 segundos.").arg(m_segundosRestantes));
            m_timerBloqueo->start(1000);
        } else {
            m_lblError->setText(QString("Credenciales incorrectas (Intento %1 de 3).").arg(m_intentosFallidos));
        }
    }
}

void LoginDialog::desbloquearIntento() {
    m_segundosRestantes--;
    if (m_segundosRestantes <= 0) {
        m_timerBloqueo->stop();
        m_intentosFallidos = 0;
        m_btnIngresar->setEnabled(true);
        m_txtUsuario->setEnabled(true);
        m_txtPassword->setEnabled(true);
        m_lblError->setVisible(false);
        m_btnIngresar->setText("Iniciar sesión");
    } else {
        m_lblError->setText(QString("Acceso bloqueado por seguridad. Espere %1 segundos.").arg(m_segundosRestantes));
    }
}
