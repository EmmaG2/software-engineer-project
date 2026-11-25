# Credenciales de Acceso y Matriz de Roles (RBAC)

Este documento contiene los usuarios, contraseñas y permisos del sistema **InsumoCafé (Café Operations)**, configurados tanto para la base de datos local (**SQLite**) como para la base de datos de producción (**PostgreSQL** en Docker).

---

## 1. Tabla de Usuarios y Contraseñas de Prueba

| Rol en el Sistema | Nombre Completo | Usuario (`username`) | Contraseña (`password`) | Estado | Permisos Principales |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **ADMIN** (Gerente) | María Torres | `maria.torres` | `admin123` | Activo | **Acceso Total:** POS, Inventario, Lotes, Mermas, Recetas, Órdenes, Reportes PDF/CSV y Usuarios. |
| **BARISTA / CAJERO** | Ana López | `ana.lopez` | `cajero123` | Activo | **Operativo:** Punto de Venta (POS), cobro de pedidos, consulta de recetas y registro de mermas por derrame/preparación. |
| **BARISTA / CAJERO** | Carlos Ruiz | `carlos.ruiz` | `cajero123` | Activo | **Operativo:** Mismo nivel que Ana López (cajero secundario). |
| **ALMACÉN** | Raúl Gómez | `raul.gomez` | `almacen123` | Activo | **Logística:** Recepción de lotes en cola PEPS, control de caducidades, registro de mermas por vencimiento y órdenes de compra. |
| *Inactivo / Suspendido* | Jorge Díaz | `jorge.diaz` | `cajero123` | *Suspendido* | Bloqueado para verificar el control de acceso de usuarios inactivos. |

> [!TIP]
> **Modo Rápido de Prueba:** Si al iniciar la aplicación cierras o cancelas la ventana de inicio de sesión, el sistema iniciará automáticamente como **Administrador (`maria.torres`)** para facilitar pruebas inmediatas.

---

## 2. Matriz de Permisos por Rol (Matriz RBAC)

| Módulo del Sistema | ADMIN (Gerente) | BARISTA (Caja) | ALMACÉN (Stock) |
| :--- | :---: | :---: | :---: |
| **Dashboard y Métricas** |  Total |  Lectura |  Lectura |
| **Punto de Venta (POS)** |  Total |  Total | Sin acceso |
| **Cobro y Ticket (F5)** |  Total |  Total | Sin acceso |
| **Inventario Consolidado** |  Total |  Lectura |  Total |
| **Recepción de Lotes (PEPS)** |  Total | Sin acceso |  Total |
| **Registro de Mermas** |  Total |  (Derrame/Error) |  (Caducidad) |
| **Recetas y Escandallos** |  Total |  Lectura |  Lectura |
| **Generación de Órdenes** |  Total | Sin acceso |  Total |
| **Exportación de Reportes** |  Total | Sin acceso | Sin acceso |
| **Gestión de Usuarios** |  Total | Sin acceso | Sin acceso |

---

## 3. Credenciales de la Base de Datos

### Modo Local (SQLite - Activo por defecto)
* **Archivo:** `stockcafe_local.db` (ubicado en la carpeta de ejecución).
* **Conexión:** Automática, no requiere servidor ni credenciales adicionales.

### Modo Producción (PostgreSQL 16 en Docker)
* **Host:** `localhost` o `127.0.0.1`
* **Puerto:** `5432`
* **Base de Datos:** `stockcafe_db`
* **Usuario:** `postgres`
* **Contraseña:** `postgres123`
* **Levantamiento con Docker:**
  ```bash
  docker compose up -d
  ```
* **Interfaz Gráfica PgAdmin 4:**
  * **URL:** `http://localhost:5050`
  * **Email:** `admin@stockcafe.com`
  * **Contraseña:** `admin123`
