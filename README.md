# Movie Theatre Management System
**Inventory Management Group 9**

A menu-driven C++ system that treats movies as inventory items: each movie
has a ticket price and a seat/ticket stock count, moves automatically through
a release lifecycle (Upcoming -> Now Showing -> Archived), and generates
real, data-driven revenue and genre analytics from actual recorded ticket
sales (no mocked numbers).

## Project Structure
```
MovieTheatreManagement/
|-- main.cpp            // Menu controller, wires all modules together
|-- movie.h / movie.cpp  // Movie struct, date utilities, input helpers,
|                        // Movie Management (Section 1), Movie Viewing (2),
|                        // Lifecycle Management (3)
|-- sales.h / sales.cpp  // SaleRecord struct, Ticket Sales Management (4)
|-- reports.h / reports.cpp // Revenue & Financial Management (5),
|                        // Analytics & Reports (6), revenue/ticket sorting (7)
|-- filemanager.h / filemanager.cpp // File Handling (9), status-grouped saves
|-- dashboard.h / dashboard.cpp // Smart Features (8): dashboards & recommendations
|-- movies.txt           // Persisted movie inventory (auto-created/updated)
|-- sales.txt             // Persisted ticket sales history (auto-created/updated)
|-- README.md
```

## Build
```
g++ *.cpp resource.res -o "Melo_Movie_Theatre" -static-libgcc -static-libstdc++

## Core Features (mapped to the project's feature list)

### 1. Movie Management
- Add / Update / Search / Delete Movie
- **Archive Movie** is a separate, manual action from Delete: it immediately
  marks a movie `Archived` without removing its record, and is *sticky* —
  it will not be reverted by the automatic date-based status logic.
- **Auto-Generated Movie ID** (Unique Feature) — IDs are assigned
  automatically and never reused, even after deletion.
- Deleting a movie permanently removes it from `movies.txt`. Its **past
  ticket sales remain in `sales.txt`** (the sale record stores its own copy
  of the movie's title/genre), so historical revenue and genre reports stay
  accurate even after a movie is removed from inventory.

### 2. Movie Viewing
- View All Movies / Now Showing / Upcoming / Archived (filtered by current
  lifecycle status)

### 3. Movie Lifecycle Management
- Each movie has a `releaseDate` and `pullOutDate`.
- **Automatic Status Determination**: on every program loop and file load,
  status is recalculated from today's date vs. those two dates:
  - today < releaseDate -> `Upcoming`
  - releaseDate <= today < pullOutDate -> `Now Showing`
  - today >= pullOutDate -> `Archived`
  - Exception: a manually archived movie stays `Archived` regardless of dates.
- Upcoming movies show **days remaining until release**; Now Showing movies
  show **days remaining until pull-out**.

### 4. Ticket Sales Management
- Record Ticket Sale: validates the movie is currently showing and has
  enough seats, then deducts seats automatically.
- View Sales History doubles as the Activity Log / Transaction History
  (every sale is a permanent record in `sales.txt`).

### 5. Revenue & Financial Management
All computed live from `sales.txt`, never hard-coded:
- Total Revenue / Total Tickets Sold (all-time)
- Average / Highest / Lowest Revenue (among movies with at least one sale)

### 6. Analytics & Reports
- **Top Performing Movie** — highest all-time revenue
- **Most Popular Genre Report** — genre with the most tickets sold
- **Genre Analysis** — per-genre movie count, tickets sold, total revenue
- **Genre Profitability Analysis** (Unique Feature) — ranks genres by
  *average revenue per movie* rather than raw totals. This is a deliberately
  different lens from "popularity": a genre with only 1 movie that sells
  extremely well can outrank a genre with many movies that each sell modestly.
- **Movie Performance Classification** — each movie with sales is compared
  against the average revenue (of movies that have sold at least one
  ticket) and labeled:
  - Revenue >= 2x average -> **Blockbuster**
  - Revenue >= 1x average -> **Hit**
  - Revenue >= 0.5x average -> **Average**
  - Below that -> **Flop**
  - No sales yet -> `N/A (No Sales Yet)`

### 7. Sorting & Organization
- Sort by Title, by Revenue, by Tickets Sold
- Total Movies Counter (with breakdown by status)
- **Status-Based File Organization** (Unique Feature) — `movies.txt` is not
  just a flat dump; every save groups records under `#UPCOMING`,
  `#NOWSHOWING`, and `#ARCHIVED` section headers, keeping the data file
  itself organized by lifecycle stage.

### 8. Smart Features (Unique)
- **Cinema Dashboard** — quick snapshot: movie counts by status, total
  tickets sold, total revenue.
- **Cinema Intelligence Dashboard** — proactive view: top performing movie,
  most profitable genre, movies releasing within 3 days, movies pulling out
  within 3 days, and movies with 5 or fewer seats left.
- **Cinema Recommendation System** — rule-based suggestions generated from
  real classification + lifecycle data, e.g. extending a nearly-sold-out
  Hit/Blockbuster, archiving a Flop early, or booking more movies in the
  most profitable genre.

### 9. File Handling
- `movies.txt` and `sales.txt` are loaded on startup and saved immediately
  after every mutating action (add/update/archive/delete/sale), not just on
  exit, to avoid data loss.

## Data File Formats
**movies.txt** (pipe-delimited, grouped by status section):
```
#UPCOMING
id|title|genre|duration|price|releaseDate|pullOutDate|seatsAvailable|seatCapacity|statusCode
#NOWSHOWING
...
#ARCHIVED
...
```
statusCode: `0` = Upcoming, `1` = Now Showing, `2` = Archived

**sales.txt** (pipe-delimited):
```
saleId|movieId|movieTitle|genre|quantity|pricePerTicket|subtotal|dateTime
```

## Known Limitations
- Field values containing a literal `|` character would break the simple
  pipe-delimited parser (not expected in normal movie titles/genres for
  this project's scope).
- Date math assumes a Gregorian calendar via `mktime`/epoch-day conversion,
  accurate for any realistic release/pull-out date range used in this system.
