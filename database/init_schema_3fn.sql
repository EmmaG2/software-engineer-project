-- =====================================================================
-- Esquema Físico Relacional en Tercera Forma Normal (3FN)
-- Proyecto: StockCafe POS (Café Ops)
-- Motor: PostgreSQL 16
-- =====================================================================

-- 1. Tabla de Roles (RBAC)
CREATE TABLE IF NOT EXISTS rol (
    id_rol SERIAL PRIMARY KEY,
    nombre_rol VARCHAR(30) NOT NULL UNIQUE,
    descripcion VARCHAR(150)
);

-- 2. Tabla de Usuarios
CREATE TABLE IF NOT EXISTS usuario (
    id_usuario SERIAL PRIMARY KEY,
    id_rol INT NOT NULL REFERENCES rol(id_rol) ON DELETE RESTRICT,
    nombre_usuario VARCHAR(50) NOT NULL UNIQUE,
    nombre_completo VARCHAR(120) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    activo BOOLEAN NOT NULL DEFAULT TRUE,
    creado_en TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- 3. Tabla de Proveedores
CREATE TABLE IF NOT EXISTS proveedor (
    id_proveedor SERIAL PRIMARY KEY,
    nombre_proveedor VARCHAR(100) NOT NULL UNIQUE,
    contacto VARCHAR(100),
    telefono VARCHAR(30),
    email VARCHAR(100),
    direccion TEXT
);

-- 4. Tabla de Insumos (Materia Prima)
CREATE TABLE IF NOT EXISTS insumo (
    id_insumo SERIAL PRIMARY KEY,
    id_proveedor_predeterminado INT REFERENCES proveedor(id_proveedor) ON DELETE SET NULL,
    nombre_insumo VARCHAR(60) NOT NULL UNIQUE,
    unidad_medida VARCHAR(15) NOT NULL CHECK (unidad_medida IN ('Gramos', 'Mililitros', 'Piezas')),
    stock_actual NUMERIC(10,2) NOT NULL DEFAULT 0.00 CHECK (stock_actual >= 0.00),
    stock_minimo NUMERIC(10,2) NOT NULL DEFAULT 0.00 CHECK (stock_minimo >= 0.00),
    stock_maximo NUMERIC(10,2) NOT NULL DEFAULT 0.00 CHECK (stock_maximo >= stock_minimo)
);

-- 5. Tabla de Lotes de Almacén (Base del motor PEPS / FIFO)
CREATE TABLE IF NOT EXISTS lote (
    id_lote SERIAL PRIMARY KEY,
    id_insumo INT NOT NULL REFERENCES insumo(id_insumo) ON DELETE RESTRICT,
    codigo_lote VARCHAR(30) NOT NULL UNIQUE,
    fecha_ingreso DATE NOT NULL DEFAULT CURRENT_DATE,
    fecha_caducidad DATE NOT NULL,
    cantidad_inicial NUMERIC(10,2) NOT NULL CHECK (cantidad_inicial > 0),
    cantidad_restante NUMERIC(10,2) NOT NULL CHECK (cantidad_restante >= 0),
    costo_unitario NUMERIC(10,2) NOT NULL CHECK (costo_unitario > 0),
    estado VARCHAR(20) NOT NULL DEFAULT 'ACTIVO_PEPS' CHECK (estado IN ('ACTIVO_PEPS', 'AGOTADO', 'BAJA_CADUCADO', 'BAJA_MERMA')),
    CONSTRAINT chk_fecha_caducidad CHECK (fecha_caducidad >= fecha_ingreso)
);

-- Índice optimizado para el algoritmo PEPS (ordenado por caducidad ascendente)
CREATE INDEX IF NOT EXISTS idx_lote_peps ON lote (id_insumo, fecha_caducidad ASC)
WHERE estado = 'ACTIVO_PEPS' AND cantidad_restante > 0;

-- 6. Tabla de Productos del Menú
CREATE TABLE IF NOT EXISTS producto (
    id_producto SERIAL PRIMARY KEY,
    nombre_prod VARCHAR(100) NOT NULL UNIQUE,
    categoria VARCHAR(50) NOT NULL,
    precio_venta NUMERIC(10,2) NOT NULL CHECK (precio_venta >= 0),
    activo BOOLEAN NOT NULL DEFAULT TRUE
);

-- 7. Tabla de Recetas / Escandallos (Relación M:N entre Producto e Insumo)
CREATE TABLE IF NOT EXISTS receta_detalle (
    id_receta SERIAL PRIMARY KEY,
    id_producto INT NOT NULL REFERENCES producto(id_producto) ON DELETE CASCADE,
    id_insumo INT NOT NULL REFERENCES insumo(id_insumo) ON DELETE RESTRICT,
    cantidad_descuento NUMERIC(10,2) NOT NULL CHECK (cantidad_descuento > 0),
    CONSTRAINT uq_producto_insumo UNIQUE (id_producto, id_insumo)
);

-- 8. Tabla de Ventas (Cabecera)
CREATE TABLE IF NOT EXISTS venta (
    id_venta SERIAL PRIMARY KEY,
    folio VARCHAR(30) NOT NULL UNIQUE,
    id_usuario INT NOT NULL REFERENCES usuario(id_usuario) ON DELETE RESTRICT,
    fecha_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    total_venta NUMERIC(10,2) NOT NULL CHECK (total_venta >= 0)
);

-- 9. Tabla de Detalle de Venta
CREATE TABLE IF NOT EXISTS venta_detalle (
    id_venta_detalle SERIAL PRIMARY KEY,
    id_venta INT NOT NULL REFERENCES venta(id_venta) ON DELETE CASCADE,
    id_producto INT NOT NULL REFERENCES producto(id_producto) ON DELETE RESTRICT,
    cantidad INT NOT NULL CHECK (cantidad > 0),
    precio_unitario NUMERIC(10,2) NOT NULL CHECK (precio_unitario >= 0),
    subtotal NUMERIC(10,2) NOT NULL CHECK (subtotal >= 0)
);

-- 10. Tabla de Movimientos de Inventario (Bitácora de Auditoría)
CREATE TABLE IF NOT EXISTS movimiento_inventario (
    id_movimiento SERIAL PRIMARY KEY,
    id_lote INT NOT NULL REFERENCES lote(id_lote) ON DELETE RESTRICT,
    id_usuario INT NOT NULL REFERENCES usuario(id_usuario) ON DELETE RESTRICT,
    id_venta INT REFERENCES venta(id_venta) ON DELETE SET NULL,
    tipo_movimiento VARCHAR(30) NOT NULL CHECK (tipo_movimiento IN (
        'ENTRADA', 'VENTA_PEPS_PARCIAL', 'VENTA_PEPS_AGOTADO', 'MERMA_CADUCIDAD', 'MERMA_OPERATIVA', 'AJUSTE'
    )),
    cantidad NUMERIC(10,2) NOT NULL CHECK (cantidad > 0),
    costo_impacto NUMERIC(10,2) NOT NULL DEFAULT 0.00,
    motivo VARCHAR(255),
    fecha_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- 11. Tabla de Órdenes de Compra
CREATE TABLE IF NOT EXISTS orden_compra (
    id_orden SERIAL PRIMARY KEY,
    folio VARCHAR(30) NOT NULL UNIQUE,
    id_proveedor INT NOT NULL REFERENCES proveedor(id_proveedor) ON DELETE RESTRICT,
    id_usuario_emite INT NOT NULL REFERENCES usuario(id_usuario) ON DELETE RESTRICT,
    fecha_emision TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    total_estimado NUMERIC(10,2) NOT NULL DEFAULT 0.00,
    estado VARCHAR(20) NOT NULL DEFAULT 'EMITIDA' CHECK (estado IN ('EMITIDA', 'ENVIADA', 'RECIBIDA', 'CANCELADA'))
);

-- 12. Detalle de Órdenes de Compra
CREATE TABLE IF NOT EXISTS orden_compra_detalle (
    id_orden_detalle SERIAL PRIMARY KEY,
    id_orden INT NOT NULL REFERENCES orden_compra(id_orden) ON DELETE CASCADE,
    id_insumo INT NOT NULL REFERENCES insumo(id_insumo) ON DELETE RESTRICT,
    cantidad_solicitada NUMERIC(10,2) NOT NULL CHECK (cantidad_solicitada > 0),
    costo_unitario_estimado NUMERIC(10,2) NOT NULL DEFAULT 0.00
);

-- =====================================================================
-- POBLACIÓN DE DATOS SEMILLA (Seed Data inicial consistente con prototipos)
-- =====================================================================

-- Roles
INSERT INTO rol (id_rol, nombre_rol, descripcion) VALUES
(1, 'ADMIN', 'Administrador / Gerente con acceso completo al sistema'),
(2, 'CAJERO', 'Barista / Cajero con acceso exclusivo a Punto de Venta y registro de mermas'),
(3, 'ALMACEN', 'Encargado de almacén con acceso a recepción de lotes y órdenes de compra')
ON CONFLICT (id_rol) DO NOTHING;

-- Usuarios (contraseña por defecto para demo: 'admin123', 'cajero123', 'almacen123')
INSERT INTO usuario (id_usuario, id_rol, nombre_usuario, nombre_completo, password_hash, activo) VALUES
(1, 1, 'maria.torres', 'María Torres', '$2a$12$e0M2cONL5IUXwZcE6mD.w.1tHjD98oQe5lq2jK8vEwZ4gP2r7X3yG', true),
(2, 2, 'ana.lopez', 'Ana López', '$2a$12$e0M2cONL5IUXwZcE6mD.w.1tHjD98oQe5lq2jK8vEwZ4gP2r7X3yG', true),
(3, 2, 'carlos.ruiz', 'Carlos Ruiz', '$2a$12$e0M2cONL5IUXwZcE6mD.w.1tHjD98oQe5lq2jK8vEwZ4gP2r7X3yG', true),
(4, 2, 'jorge.diaz', 'Jorge Díaz', '$2a$12$e0M2cONL5IUXwZcE6mD.w.1tHjD98oQe5lq2jK8vEwZ4gP2r7X3yG', false),
(5, 3, 'raul.gomez', 'Raúl Gómez', '$2a$12$e0M2cONL5IUXwZcE6mD.w.1tHjD98oQe5lq2jK8vEwZ4gP2r7X3yG', true)
ON CONFLICT (id_usuario) DO NOTHING;

-- Proveedores
INSERT INTO proveedor (id_proveedor, nombre_proveedor, contacto, telefono, email, direccion) VALUES
(1, 'Lácteos del Norte', 'Roberto Garza', '618-123-4567', 'contacto@lacteosnorte.com', 'Zona Industrial Durango'),
(2, 'Café Durango', 'Patricia Morales', '618-987-6543', 'ventas@cafedurango.com', 'Centro Histórico Durango'),
(3, 'Suministros MX', 'Armando Solís', '555-432-1098', 'pedidos@suministrosmx.com', 'Av. Insurgentes 450')
ON CONFLICT (id_proveedor) DO NOTHING;

-- Insumos
INSERT INTO insumo (id_insumo, id_proveedor_predeterminado, nombre_insumo, unidad_medida, stock_actual, stock_minimo, stock_maximo) VALUES
(1, 2, 'Grano espresso', 'Gramos', 48200.00, 15000.00, 60000.00),
(2, 1, 'Leche entera', 'Mililitros', 8000.00, 20000.00, 50000.00),
(3, 3, 'Jarabe vainilla', 'Mililitros', 14000.00, 20000.00, 40000.00),
(4, 3, 'Vasos 12 oz', 'Piezas', 320.00, 100.00, 1000.00),
(5, 1, 'Leche de avena', 'Mililitros', 6000.00, 10000.00, 25000.00),
(6, 3, 'Croissant repostería', 'Piezas', 25.00, 10.00, 50.00)
ON CONFLICT (id_insumo) DO NOTHING;

-- Lotes activos PEPS
INSERT INTO lote (id_lote, id_insumo, codigo_lote, fecha_ingreso, fecha_caducidad, cantidad_inicial, cantidad_restante, costo_unitario, estado) VALUES
(1, 2, 'LOT-202611-014', CURRENT_DATE - INTERVAL '4 days', CURRENT_DATE + INTERVAL '2 days', 40000.00, 8000.00, 0.028, 'ACTIVO_PEPS'),
(2, 1, 'LOT-202611-021', CURRENT_DATE - INTERVAL '2 days', CURRENT_DATE + INTERVAL '60 days', 50000.00, 48200.00, 0.350, 'ACTIVO_PEPS'),
(3, 3, 'LOT-202610-008', CURRENT_DATE - INTERVAL '15 days', CURRENT_DATE + INTERVAL '120 days', 20000.00, 14000.00, 0.120, 'ACTIVO_PEPS'),
(4, 4, 'LOT-202611-005', CURRENT_DATE - INTERVAL '10 days', CURRENT_DATE + INTERVAL '365 days', 500.00, 320.00, 1.800, 'ACTIVO_PEPS'),
(5, 5, 'LOT-202611-018', CURRENT_DATE - INTERVAL '3 days', CURRENT_DATE + INTERVAL '10 days', 10000.00, 6000.00, 0.045, 'ACTIVO_PEPS'),
(6, 6, 'LOT-202611-030', CURRENT_DATE, CURRENT_DATE + INTERVAL '3 days', 30.00, 25.00, 18.000, 'ACTIVO_PEPS')
ON CONFLICT (id_lote) DO NOTHING;

-- Productos del Menú
INSERT INTO producto (id_producto, nombre_prod, categoria, precio_venta, activo) VALUES
(1, 'Americano', 'Bebidas Calientes', 42.00, true),
(2, 'Latte', 'Bebidas Calientes', 56.00, true),
(3, 'Capuccino', 'Bebidas Calientes', 58.00, true),
(4, 'Cold brew', 'Bebidas Frías', 62.00, true),
(5, 'Chai latte', 'Bebidas Calientes', 60.00, true),
(6, 'Croissant', 'Repostería', 38.00, true)
ON CONFLICT (id_producto) DO NOTHING;

-- Escandallo / Recetas Técnicas
-- Americano: 18g café + 1 vaso 12oz
INSERT INTO receta_detalle (id_producto, id_insumo, cantidad_descuento) VALUES
(1, 1, 18.00),
(1, 4, 1.00),
-- Latte: 18g café + 200ml leche entera + 1 vaso 12oz
(2, 1, 18.00),
(2, 2, 200.00),
(2, 4, 1.00),
-- Capuccino: 18g café + 150ml leche entera + 1 vaso 12oz
(3, 1, 18.00),
(3, 2, 150.00),
(3, 4, 1.00),
-- Cold brew: 30g café + 1 vaso 12oz
(4, 1, 30.00),
(4, 4, 1.00),
-- Chai latte: 200ml leche entera + 1 vaso 12oz
(5, 2, 200.00),
(5, 4, 1.00),
-- Croissant: 1 croissant repostería
(6, 6, 1.00)
ON CONFLICT (id_producto, id_insumo) DO NOTHING;

-- Ajuste de secuencias
SELECT setval('rol_id_rol_seq', (SELECT MAX(id_rol) FROM rol));
SELECT setval('usuario_id_usuario_seq', (SELECT MAX(id_usuario) FROM usuario));
SELECT setval('proveedor_id_proveedor_seq', (SELECT MAX(id_proveedor) FROM proveedor));
SELECT setval('insumo_id_insumo_seq', (SELECT MAX(id_insumo) FROM insumo));
SELECT setval('lote_id_lote_seq', (SELECT MAX(id_lote) FROM lote));
SELECT setval('producto_id_producto_seq', (SELECT MAX(id_producto) FROM producto));
SELECT setval('receta_detalle_id_receta_seq', (SELECT MAX(id_receta) FROM receta_detalle));
