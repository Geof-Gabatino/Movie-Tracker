# Movie Theatre Management System
**Inventory Management Group 9**

A menu-driven C++ system that treats movies as inventory items: each movie
has a ticket price and a seat/ticket stock count, moves automatically through
a release lifecycle (Upcoming -> Now Showing -> Archived), and generates
real, data-driven revenue and genre analytics from actual recorded ticket
sales (no mocked numbers). The system also runs a small point-of-sale module
for cinema concessions (snacks/drinks) alongside ticket sales.

## Project Structure
```
MovieTheatreManagement/
|-- main.cpp                // Menu controller, wires all modules together
|-- movie.h / movie.cpp     // Movie struct, ConcessionItem struct, date utilities,
|                           // input helpers, Movie Management (Section 1),
|                           // Movie Viewing (2), Lifecycle Management (3)
|-- sales.h / sales.cpp     // SaleRecord struct, Ticket Sales Management (4),
|                           // Concessions Management (4)
|-- reports.h / reports.cpp // Revenue & Financial Management (5),
|                           // Analytics & Reports (6), revenue/ticket sorting (7)
|-- filemanager.h / filemanager.cpp // File Handling (9)
|-- dashboard.h / dashboard.cpp     // Smart Features (8): dashboard & recommendations
|-- movies.txt              // Persisted movie inventory (auto-created/updated)
|-- sales.txt                // Persisted ticket sales history (auto-created/updated)
|-- concessions.txt          // Persisted concessions inventory (auto-created/updated)
|-- README.md
```

## Build
```
g++ *.cpp resource.res -o "Melo_Movie_Theatre" -static-libgcc -static-libstdc++
```
`resource.res` is an optional Windows icon resource — if you don't have one, the
project builds fine without it, e.g.:
```
g++ *.cpp -o Melo_Movie_Theatre
```

## Core Features (mapped to the project's feature list)

### 1. Movie Management
- Add / Update / Search / Delete Movie
- Each movie tracks title, genre, duration, **parental rating** (chosen from a
  fixed menu: G / PG / R-13 / R-16 / R-18), ticket price, seat capacity &
  availability, release date, pull-out date, and show time.
- **Archive Movie** is a separate, manual action from Delete: it immediately
  marks a movie `Archived` without removing its record. *(See Known
  Limitations — this manual override is currently re-evaluated, and can be
  reverted, the next time the main menu refreshes.)*
- **Auto-Generated Movie ID** (Unique Feature) — IDs are assigned
  automatically and increase monotonically while the program is running, so
  no two movies in the same session ever collide. *(See Known Limitations —
  an ID can be reassigned across separate program runs if the
  highest-numbered movie is deleted.)*
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
- Upcoming movies show **days remaining until release**; Now Showing movies
  show **days remaining until pull-out** (surfaced on the Cinema Dashboard).

### 4. Ticket Sales & Concessions Management

**Ticket Sales**
- Validates the movie is currently showing and has enough seats, then
  deducts seats automatically.
- Point-of-sale **discount menu**: None, Senior Citizen (20%), PWD (20%),
  Student & Teacher (20%), Early Screening Promo (30%).
- Pricing breakdown: `(tickets x price)` minus the discount, plus a 10%
  Amusement Tax (a Cultural Tax line is reserved for future use, currently
  always P0.00), printed on a formatted ticket receipt.
- View Sales History doubles as the Activity Log / Transaction History
  (every sale is a permanent record in `sales.txt`).

**Concessions Management** (snacks & drinks)
- **Add Concession Item** — register a snack/drink with name, category,
  unit price, and starting stock quantity.
- **View All Items** — inventory table; items with zero stock are flagged
  `(OUT OF STOCK)`.
- **Process Snack Sale (Cart System)** — add multiple different items and
  quantities to one transaction before checking out; stock is deducted live
  as each item is added to the cart.
- A 12% VAT is added to the cart subtotal for the total due, and a
  formatted receipt is printed (Transaction ID prefixed `SNACK-`).
- **Delete Concession Item** — permanently removes an item after a typed
  `YES` confirmation.
- All changes are saved to `concessions.txt` immediately.

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

### 8. Smart Features (Unique)
- **Cinema Dashboard** — a combined snapshot + proactive intelligence view
  in one screen: movie counts by status, total tickets sold, total revenue,
  top performing movie, most profitable genre, movies releasing within 3
  days, movies pulling out within 3 days, and movies with 5 or fewer seats
  left.
- **Cinema Recommendation System** — rule-based suggestions generated from
  real classification + lifecycle data, e.g. extending a nearly-sold-out
  Hit/Blockbuster, archiving a Flop early, or booking more movies in the
  most profitable/popular genre.

### 9. File Handling
- `movies.txt`, `sales.txt`, and `concessions.txt` are loaded on startup
  and saved immediately after every mutating action (add/update/archive/
  delete/sale), not just on exit, to avoid data loss.

## Data File Formats
**movies.txt** (pipe-delimited, one movie per line):
```
id|title|genre|duration|rating|price|releaseDate|pullOutDate|seatsAvailable|seatCapacity|statusCode|showtime
```
statusCode: `0` = Upcoming, `1` = Now Showing, `2` = Archived

**sales.txt** (pipe-delimited):
```
saleId|movieId|movieTitle|genre|quantity|pricePerTicket|subtotal|dateTime|seatNumbers|paymentMethod
```

**concessions.txt** (pipe-delimited):
```
id|name|category|price|stockQuantity
```

## Known Limitations
- Field values containing a literal `|` character would break the simple
  pipe-delimited parser (not expected in normal movie/concession titles for
  this project's scope).
- Date math assumes a Gregorian calendar via `mktime`/epoch-day conversion,
  accurate for any realistic release/pull-out date range used in this system.
- **Manual archiving is not currently "sticky":** `archiveMovie()` sets a
  movie's status to `Archived` right away, but `updateMovieStatuses()` runs
  again on every main-menu refresh and recalculates status purely from
  `releaseDate`/`pullOutDate`. If the pull-out date hasn't actually passed
  yet, the manual archive will be overwritten back to `Now Showing` the next
  time the menu redraws.
- `seatNumbers` and `paymentMethod` exist in the `SaleRecord` struct and in
  the `sales.txt` format, but `recordTicketSale()` does not yet prompt the
  user for them — they are currently always saved blank. This is scaffolding
  for a future update, not a functioning feature yet.
- **Movie/sale/concession IDs can be reused across program restarts:** on
  load, the "next ID" counter for each entity is recalculated as
  `(highest ID currently in the file) + 1`. If the highest-numbered record
  is deleted and the program is then restarted (not just returned to the
  main menu), that ID becomes available again for a new record. IDs are
  guaranteed unique only within a single continuous run.
