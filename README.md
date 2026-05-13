# Copérnico — Reserva de Oficina

Aplicación de escritorio para Kubuntu 24.04 (KDE Plasma 5.27 / Qt 5.15) que
permite reservar los 8 lugares de la mesa de oficina por día, con vista
calendarizada, modo claro/oscuro y una API REST para integraciones externas.

- **UI**: Qt Quick 2 (QML) con `QtQuick.Controls 2` estilo Fusion.
- **Persistencia**: SQLite (Qt Sql).
- **API REST**: `QTcpServer` embebido (`/seats`, `/reservations`).
- **Tema**: claro/oscuro conmutables desde la barra superior.
- **Lugares disponibles**: en **verde**. **Reservados**: en **rojo**.
- **Calendario** en el cajón lateral, con botón **Hoy** en la barra superior.

## Dependencias (Kubuntu 24.04)

```bash
sudo apt install \
    build-essential cmake \
    qtbase5-dev \
    qtdeclarative5-dev \
    qtquickcontrols2-5-dev \
    qttools5-dev-tools \
    qml-module-qtquick-controls2 \
    qml-module-qtquick-layouts \
    qml-module-qtquick-window2 \
    qml-module-qt-labs-calendar \
    libqt5sql5-sqlite
```

> `qtquickcontrols2-5-dev` aporta `Qt5QuickControls2Config.cmake` (necesario en
> tiempo de compilación). `qml-module-qtquick-controls2` es el plugin QML que
> se carga en runtime — ambos son necesarios.

## Compilación

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
```

## Ejecución

```bash
./build/kde-reservas              # API REST en :8080
./build/kde-reservas -p 9000      # puerto REST custom
./build/kde-reservas -d /tmp/r.db # ruta DB custom
```

La base de datos por defecto se crea en
`~/.local/share/kde-reservas/reservas.sqlite`.

## API REST

Todas las respuestas son JSON. Por defecto el servicio escucha en
`0.0.0.0:8080`.

### `GET /seats?date=YYYY-MM-DD`

Devuelve los 8 lugares de la mesa con el estado para esa fecha. Si se omite
`date`, se asume la fecha de hoy.

```bash
curl http://localhost:8080/seats?date=2026-05-13
```

```json
{
  "date": "2026-05-13",
  "seats": [
    { "seat": 1, "reserved": true,  "first_name": "Lucas",   "last_name": "Paninka",  "id": 1 },
    { "seat": 2, "reserved": true,  "first_name": "Martín",  "last_name": "Detlefsen","id": 2 },
    { "seat": 3, "reserved": false, "first_name": "",         "last_name": "",         "id": 0 },
    ...
  ]
}
```

### `POST /reservations`

Reserva un lugar para una fecha.

```bash
curl -X POST http://localhost:8080/reservations \
  -H 'Content-Type: application/json' \
  -d '{"seat":3,"date":"2026-05-13","first_name":"Ana","last_name":"García"}'
```

- `201 Created` si la reserva fue exitosa.
- `409 Conflict` si el lugar ya estaba reservado para esa fecha.
- `400 Bad Request` si faltan datos o la fecha es inválida.

### `DELETE /reservations?seat=N&date=YYYY-MM-DD`

Libera la reserva del lugar `N` en la fecha indicada.

### `GET /health`

Devuelve `{"status":"ok","seats_per_table":8}`.

## Modelo de datos

Una sola tabla en SQLite con índice único `(seat_number, reservation_date)`
para impedir reservas dobles:

```sql
CREATE TABLE reservations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    seat_number INTEGER NOT NULL CHECK (seat_number BETWEEN 1 AND 8),
    reservation_date TEXT NOT NULL,
    first_name TEXT NOT NULL,
    last_name TEXT NOT NULL,
    created_at TEXT NOT NULL DEFAULT (datetime('now')),
    UNIQUE(seat_number, reservation_date)
);
```

## Estructura del proyecto

```
.
├── CMakeLists.txt
├── README.md
├── qml/
│   ├── qml.qrc
│   ├── Theme.qml              # singleton de tema (claro/oscuro)
│   ├── SeatCell.qml           # celda de un lugar
│   ├── ReservationDialog.qml  # diálogo nombre + apellido
│   └── main.qml               # ventana principal
└── src/
    ├── main.cpp
    ├── database.{h,cpp}
    ├── reservationmodel.{h,cpp}
    └── restserver.{h,cpp}
```
