#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include "../models/models.h"

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget* parent = nullptr);

    Models::Usuario getUsuarioAutenticado() const {
        return m_usuarioActual;
    }

private slots:
    void intentarLogin();
    void desbloquearIntento();

private:
    QLineEdit* m_txtUsuario{nullptr};
    QLineEdit* m_txtPassword{nullptr};
    QPushButton* m_btnIngresar{nullptr};
    QLabel* m_lblError{nullptr};
    QLabel* m_lblInfoDemo{nullptr};

    int m_intentosFallidos{0};
    QTimer* m_timerBloqueo{nullptr};
    int m_segundosRestantes{0};

    Models::Usuario m_usuarioActual;
};
