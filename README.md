# StockCafe POS (Café Ops)

Sistema integral de punto de venta (POS), gestión de inventario, control de caducidades bajo el principio contable **PEPS (Primeras Entradas, Primeras Salidas / FIFO)** y emisión de órdenes de compra para cafeterías y bistrós gastronómicos.

Desarrollado bajo el estándar **IEEE Std 830-1998** y modelo arquitectónico desacoplado de **3 Capas (MVC)** en **C++20**, **Qt 6 (Widgets/SVG)** y base de datos relacional normalizada en **Tercera Forma Normal (3FN)** con **PostgreSQL 16**.

---

## Stack Tecnológico
* **Lenguaje:** C++20 (GCC 13+ / MinGW 64-bit)
* **GUI / HMI:** Qt 6.7+ (Qt Core, Qt GUI, Qt Widgets, Qt Svg, Qt Sql)
* **Base de Datos:** PostgreSQL 16 (vía Docker Compose) con fallback transparente a SQLite para desarrollo autónomo
* **Gestor de Compilación:** CMake 3.25+ y Ninja
* **Seguridad:** Control de acceso por roles (RBAC) con bloqueo por intentos fallidos

---

## Despliegue de la Base de Datos (Docker Compose)

Para levantar el motor PostgreSQL 16 con el esquema 3FN y pgAdmin 4:

```bash
# Levantar PostgreSQL 16 y pgAdmin 4 en segundo plano
docker compose up -d

# Verificar estado de los contenedores
docker compose ps
```

* **Host:** `localhost` | **Puerto:** `5432`
* **Base de datos:** `stockcafe_db`
* **Usuario:** `postgres` | **Contraseña:** `postgres_password`
* **pgAdmin 4 Web:** `http://localhost:5050` (Usuario: `admin@cafeops.com`, Password: `admin`)

---

## Compilación y Ejecución

Desde PowerShell en Windows (usando Qt y MinGW instalados en `C:\Qt`):

```powershell
# 1. Configurar variables de entorno temporalmente
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.1\mingw_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\CMake_64\bin;" + $env:PATH

# 2. Generar archivos de construcción con CMake y Ninja
cmake -B build -S . -GNinja -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/mingw_64" -DCMAKE_CXX_COMPILER="C:/Qt/Tools/mingw1310_64/bin/g++.exe" -DCMAKE_MAKE_PROGRAM="C:/Qt/Tools/Ninja/ninja.exe"

# 3. Compilar el binario ejecutable
cmake --build build

# 4. Ejecutar la aplicación
.\build\QtDesktopApp.exe
```

---

## Credenciales de Acceso (RBAC)

Al iniciar la aplicación se solicitará autenticación en la pantalla de bienvenida:

| Rol | Usuario | Contraseña | Privilegios / Módulos Disponibles |
| :--- | :--- | :--- | :--- |
| **Administrador / Gerente** | `maria.torres` | `admin123` | Acceso completo (Dashboard, Ventas, Inventario y Lotes, Recepciones, Mermas, Recetas, Alertas, Órdenes de Compra, Reportes, Usuarios) |
| **Barista / Cajero** | `ana.lopez` | `cajero123` | Modo operativo rápido: Punto de Venta (Sales) y Registro justificado de Mermas (Waste) |
| **Barista / Cajero** | `carlos.ruiz` | `cajero123` | Modo operativo rápido |

---

## Módulos Implementados

1. **Dashboard & KPIs:** Métricas ejecutivas en vivo (Ventas de hoy, Insumos en stock crítico, Lotes por vencer < 72h y Mermas del mes) con tabla de alertas prioritarias.
2. **Punto de Venta POS & Motor PEPS:** Selección táctil de productos, cálculo dinámico de ticket y botón `[F5] Cobrar y descontar PEPS`. Descuenta atómicamente de los lotes más antiguos en tiempo real.
3. **Inventario & Lotes:** Tabla de existencias con semaforización de caducidades y diálogo modal para registrar recepción de lotes de insumos.
4. **Recepción de Lotes:** Registro de compras con validación estricta de fecha de caducidad.
5. **Registro de Mermas:** Bajas operativas justificadas por caducidad vencida, daño accidental o error en preparación sin alterar caja.
6. **Catálogo de Recetas:** Parametrización de escandallos con ingredientes en gramos o mililitros.
7. **Semáforo de Alertas:** Detección de lotes que expiran en menos de 72 horas y materias primas por debajo de su umbral mínimo.
8. **Órdenes de Compra (PDF):** Consolidación de insumos faltantes y exportación a PDF formal con formato corporativo y firmas.
9. **Reportes Ejecutivos:** Exportación de mermas a PDF y bitácora histórica de movimientos PEPS a formato CSV.
