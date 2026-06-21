/*
    Menu-Driven Cinema Movie Inventory and Customer Ticket Ordering System
    -----------------------------------------------------------------------
    Movies are treated as inventory items (each with a ticket price and a
    seat/ticket stock count). Features: add/update/delete/search/view
    movies, monitor seat stock, process customer ticket orders with stock
    validation and auto-deduction, persistent file storage
    (movies.txt, transactions.txt), and order receipts.
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <climits>
#include <algorithm>
#include <ctime>
using namespace std;

// ---------- Data ----------
struct Movie {
    int id;
    string title;
    string genre;
    string duration;     // e.g. "2h 30m"
    double price;        // ticket price
    int seatsAvailable;  // acts as "stock" for this movie
};

struct OrderLine { int movieId; string title; int qty; double subtotal; };

vector<Movie> movies;
const string MOVIE_FILE = "movies.txt";
const string TXN_FILE    = "transactions.txt";
int nextId = 1;

// ---------- Small helpers ----------
void pause() { cout << "\nPress Enter to continue..."; cin.ignore(numeric_limits<streamsize>::max(), '\n'); }

int askInt(const string& prompt, int minVal = INT_MIN) {
    int val;
    while (true) {
        cout << prompt;
        if (cin >> val && val >= minVal) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); return val; }
        cout << "Invalid input. Please enter a number" << (minVal != INT_MIN ? " >= " + to_string(minVal) : "") << ".\n";
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

double askDouble(const string& prompt, double minVal = 0) {
    double val;
    while (true) {
        cout << prompt;
        if (cin >> val && val >= minVal) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); return val; }
        cout << "Invalid input. Please enter a valid number >= " << minVal << ".\n";
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string askLine(const string& prompt, bool allowEmpty = false) {
    string val;
    while (true) {
        cout << prompt;
        getline(cin, val);
        if (!val.empty() || allowEmpty) return val;
        cout << "This field cannot be empty.\n";
    }
}

string currentDateTime() {
    time_t now = time(0);
    tm* lt = localtime(&now);
    ostringstream ss;
    ss << (1900 + lt->tm_year) << '-' << setw(2) << setfill('0') << (1 + lt->tm_mon) << '-'
       << setw(2) << setfill('0') << lt->tm_mday << ' '
       << setw(2) << setfill('0') << lt->tm_hour << ':' << setw(2) << setfill('0') << lt->tm_min;
    return ss.str();
}

// ---------- File handling ----------
void saveMovies() {
    ofstream out(MOVIE_FILE);
    for (const auto& m : movies)
        out << m.id << '|' << m.title << '|' << m.genre << '|' << m.duration << '|'
            << m.price << '|' << m.seatsAvailable << '\n';
}

void loadMovies() {
    ifstream in(MOVIE_FILE);
    if (!in) return; // first run, no file yet
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string idStr, title, genre, duration, priceStr, seatsStr;
        getline(ss, idStr, '|'); getline(ss, title, '|'); getline(ss, genre, '|');
        getline(ss, duration, '|'); getline(ss, priceStr, '|'); getline(ss, seatsStr, '|');
        try {
            Movie m{stoi(idStr), title, genre, duration, stod(priceStr), stoi(seatsStr)};
            movies.push_back(m);
            nextId = max(nextId, m.id + 1);
        } catch (...) { /* skip malformed line */ }
    }
}

void logTransaction(const string& text) {
    ofstream out(TXN_FILE, ios::app);
    out << text;
}

// ---------- Lookup ----------
Movie* findById(int id) {
    for (auto& m : movies) if (m.id == id) return &m;
    return nullptr;
}

void printMovieRow(const Movie& m) {
    cout << left << setw(5) << m.id << setw(22) << m.title << setw(12) << m.genre
         << setw(9) << m.duration << "P" << right << setw(7) << fixed << setprecision(2) << m.price
         << left << "  " << setw(6) << m.seatsAvailable << '\n';
}

void printHeader() {
    cout << left << setw(5) << "ID" << setw(22) << "Title" << setw(12) << "Genre"
         << setw(9) << "Duration" << setw(8) << "Price" << "  " << setw(6) << "Seats" << '\n';
    cout << string(64, '-') << '\n';
}

// ---------- Core features ----------
void addMovie() {
    cout << "\n--- ADD NEW MOVIE ---\n";
    Movie m;
    m.id = nextId++;
    m.title = askLine("Movie Title: ");
    m.genre = askLine("Genre: ");
    m.duration = askLine("Duration (e.g., 2h 30m): ");
    m.price = askDouble("Ticket Price: P", 0.01);
    m.seatsAvailable = askInt("Available Seats/Tickets: ", 0);
    movies.push_back(m);
    saveMovies();
    cout << "Movie added! Assigned ID: " << m.id << '\n';
    pause();
}

void viewAllMovies() {
    cout << "\n--- ALL MOVIES IN INVENTORY ---\n";
    if (movies.empty()) { cout << "No movies in the inventory.\n"; pause(); return; }
    printHeader();
    for (const auto& m : movies) printMovieRow(m);
    cout << "\nTotal movies: " << movies.size() << '\n';
    pause();
}

void searchMovie() {
    cout << "\n--- SEARCH MOVIE ---\n";
    if (movies.empty()) { cout << "No movies in the inventory.\n"; pause(); return; }
    string term = askLine("Enter title or genre to search: ");
    string lowerTerm = term; transform(lowerTerm.begin(), lowerTerm.end(), lowerTerm.begin(), ::tolower);

    printHeader();
    bool found = false;
    for (const auto& m : movies) {
        string lt = m.title, lg = m.genre;
        transform(lt.begin(), lt.end(), lt.begin(), ::tolower);
        transform(lg.begin(), lg.end(), lg.begin(), ::tolower);
        if (lt.find(lowerTerm) != string::npos || lg.find(lowerTerm) != string::npos) {
            printMovieRow(m); found = true;
        }
    }
    if (!found) cout << "No matching movies found.\n";
    pause();
}

void updateMovie() {
    cout << "\n--- UPDATE MOVIE ---\n";
    if (movies.empty()) { cout << "No movies in the inventory.\n"; pause(); return; }
    int id = askInt("Enter Movie ID to update: ");
    Movie* m = findById(id);
    if (!m) { cout << "Movie not found.\n"; pause(); return; }

    printMovieRow(*m);
    cout << "[1] Title  [2] Genre  [3] Duration  [4] Price  [5] Seats/Tickets\n";
    int choice = askInt("Field to update: ", 1);
    switch (choice) {
        case 1: m->title = askLine("New Title: "); break;
        case 2: m->genre = askLine("New Genre: "); break;
        case 3: m->duration = askLine("New Duration: "); break;
        case 4: m->price = askDouble("New Ticket Price: P", 0.01); break;
        case 5: m->seatsAvailable = askInt("New Available Seats/Tickets: ", 0); break;
        default: cout << "Invalid choice.\n"; pause(); return;
    }
    saveMovies();
    cout << "Movie updated.\n";
    pause();
}

void deleteMovie() {
    cout << "\n--- DELETE MOVIE ---\n";
    if (movies.empty()) { cout << "No movies in the inventory.\n"; pause(); return; }
    int id = askInt("Enter Movie ID to delete: ");
    auto it = find_if(movies.begin(), movies.end(), [&](const Movie& m) { return m.id == id; });
    if (it == movies.end()) { cout << "Movie not found.\n"; pause(); return; }

    printMovieRow(*it);
    string confirm = askLine("Delete this movie? (y/n): ");
    if (!confirm.empty() && (confirm[0] == 'y' || confirm[0] == 'Y')) {
        movies.erase(it);
        saveMovies();
        cout << "Movie deleted.\n";
    } else {
        cout << "Cancelled.\n";
    }
    pause();
}

void viewLowStock() {
    cout << "\n--- LOW SEAT ALERT (<=5 seats left) ---\n";
    bool any = false;
    printHeader();
    for (const auto& m : movies) {
        if (m.seatsAvailable <= 5) { printMovieRow(m); any = true; }
    }
    if (!any) cout << "No movies are running low on seats.\n";
    pause();
}

// Processes one customer order that may include tickets for multiple movies.
void processOrder() {
    cout << "\n--- CUSTOMER TICKET ORDER ---\n";
    if (movies.empty()) { cout << "No movies in the inventory.\n"; pause(); return; }

    printHeader();
    for (const auto& m : movies) printMovieRow(m);

    vector<OrderLine> lines;
    double total = 0;

    while (true) {
        int id = askInt("\nEnter Movie ID to order tickets for (0 to finish): ", 0);
        if (id == 0) break;
        Movie* m = findById(id);
        if (!m) { cout << "Invalid Movie ID.\n"; continue; }
        if (m->seatsAvailable == 0) { cout << m->title << " is SOLD OUT.\n"; continue; }

        int qty = askInt("Tickets for " + m->title + " (Available: " + to_string(m->seatsAvailable) + "): ", 1);
        if (qty > m->seatsAvailable) { cout << "Not enough seats! Only " << m->seatsAvailable << " left.\n"; continue; }

        double subtotal = qty * m->price;
        m->seatsAvailable -= qty;     // auto-deduct after validated order
        total += subtotal;
        lines.push_back({m->id, m->title, qty, subtotal});
        cout << "Added to order.\n";
    }

    if (lines.empty()) { cout << "Order cancelled - no tickets selected.\n"; pause(); return; }

    saveMovies(); // persist updated seat counts

    // Receipt
    ostringstream receipt;
    receipt << "===== TICKET ORDER RECEIPT =====\n";
    receipt << "Date: " << currentDateTime() << '\n';
    for (const auto& l : lines)
        receipt << l.title << " x" << l.qty << " = P" << fixed << setprecision(2) << l.subtotal << '\n';
    receipt << "TOTAL: P" << fixed << setprecision(2) << total << '\n';
    receipt << "=================================\n\n";

    cout << '\n' << receipt.str();
    logTransaction(receipt.str());
    cout << "Order completed and saved.\n";
    pause();
}

void viewTransactionLog() {
    cout << "\n--- TRANSACTION HISTORY ---\n";
    ifstream in(TXN_FILE);
    if (!in) { cout << "No transactions recorded yet.\n"; pause(); return; }
    cout << in.rdbuf();
    pause();
}

// ---------- Menu ----------
void displayMenu() {
    cout << "\n===== CINEMA MOVIE INVENTORY SYSTEM =====\n"
         << "[1] Add Movie\n[2] View All Movies\n[3] Search Movie\n"
         << "[4] Update Movie\n[5] Delete Movie\n[6] Process Ticket Order\n"
         << "[7] Low Seat Alert\n[8] Transaction History\n[0] Exit\n"
         << "Enter choice: ";
}

int main() {
    loadMovies();
    int choice;
    do {
        displayMenu();
        if (!(cin >> choice)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); choice = -1; }
        else cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1: addMovie(); break;
            case 2: viewAllMovies(); break;
            case 3: searchMovie(); break;
            case 4: updateMovie(); break;
            case 5: deleteMovie(); break;
            case 6: processOrder(); break;
            case 7: viewLowStock(); break;
            case 8: viewTransactionLog(); break;
            case 0: cout << "Inventory saved. Goodbye!\n"; break;
            default: cout << "Invalid choice. Try again.\n"; pause();
        }
    } while (choice != 0);
    return 0;
}