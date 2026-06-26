#include "filemanager.h"
#include <iostream>
#include <fstream>           // Needed for ifstream (file input) and ofstream (file output)
#include <sstream>           // Needed for stringstream, used to split each line on the '|' delimiter
#include <vector>

using namespace std;

// --- MOVIE FUNCTIONS ---

// Writes every movie's data to movies.txt, one movie per line, with fields separated by '|'.
// Using '|' as a delimiter (instead of a comma) avoids conflicts with text that might contain commas.
void saveMovies(const vector<Movie>& movies) {
    ofstream fout("movies.txt");    // Opens (or creates/overwrites) movies.txt for writing
    for (const auto& m : movies) {
        fout << m.id << '|' << m.title << '|' << m.genre << '|' << m.duration << '|' 
             << m.rating << '|' << m.price << '|' << m.releaseDate << '|' 
             << m.pullOutDate << '|' << m.seatsAvailable << '|' << m.seatCapacity << '|' 
             << static_cast<int>(m.status) << '|' << m.showtime << '\n';   // static_cast converts the enum into a plain int for storage
    }
    // fout automatically closes/flushes when it goes out of scope (its destructor runs at the end of the function)
}

// Reads movies.txt back into the movies vector, parsing each '|'-separated line into a Movie struct.
void loadMovies(vector<Movie>& movies, int& nextId) {
    ifstream fin("movies.txt");     // Opens movies.txt for reading
    if (!fin) return;               // Guard: if the file doesn't exist yet (e.g. first run), just leave the vector empty and return
    movies.clear();                 // Wipe any existing data before loading fresh from file
    string line;
    int maxId = 0;                  // Tracks the highest ID seen so far, so we know what ID to use next
    while (getline(fin, line)) {    // Reads the file one line at a time until end-of-file
        if (line.empty()) continue;  // Skip any blank lines
        stringstream ss(line);       // Wrap this line in a stringstream so we can split it piece by piece
        string segment;
        vector<string> data;         // Will hold each '|'-separated field as a string
        while (getline(ss, segment, '|')) data.push_back(segment);   // getline with '|' as the delimiter splits on that character instead of newline
        
        if (data.size() < 12) continue;    // Guard: skip malformed/incomplete lines that don't have all 12 expected fields
        
        try {                                // try/catch guards against bad data (e.g. text where a number is expected) crashing the program
            Movie m;
            m.id = stoi(data[0]);            // stoi = "string to int"
            m.title = data[1];
            m.genre = data[2];
            m.duration = data[3];
            m.rating = data[4];
            m.price = stod(data[5]);         // stod = "string to double"
            m.releaseDate = data[6];
            m.pullOutDate = data[7];
            m.seatsAvailable = stoi(data[8]);
            m.seatCapacity = stoi(data[9]);
            m.status = static_cast<MovieStatus>(stoi(data[10]));   // Convert the stored int back into the MovieStatus enum
            m.showtime = data[11];
            
            if (m.id > maxId) maxId = m.id;   // Keep track of the largest ID encountered
            movies.push_back(m);
        } catch (...) { continue; }    // "catch(...)" catches ANY type of exception; if parsing this line fails, just skip it and move on
    }
    nextId = maxId + 1;    // The next new movie should get an ID one higher than the largest one currently on file
}

// --- SALES FUNCTIONS ---

// Writes every sale record to sales.txt, one sale per line, '|'-delimited (same pattern as saveMovies).
void saveSales(const vector<SaleRecord>& sales) {
    ofstream fout("sales.txt");
    for (const auto& s : sales) {
        fout << s.saleId << '|' << s.movieId << '|' << s.movieTitle << '|' << s.genre << '|'
             << s.quantity << '|' << s.pricePerTicket << '|' << s.subtotal << '|' << s.dateTime << '|' 
             << s.seatNumbers << '|' << s.paymentMethod << '\n';
    }
}

// Reads sales.txt back into the sales vector.
void loadSales(vector<SaleRecord>& sales, int& nextSaleId) {
    ifstream fin("sales.txt");
    if (!fin) return;               // Guard: file may not exist yet on first run
    sales.clear();
    string line;
    int maxId = 0;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string segment;
        vector<string> data;
        while (getline(ss, segment, '|')) data.push_back(segment);
        if (data.size() < 8) continue;    // Only the first 8 fields are strictly required; seatNumbers/paymentMethod are optional (see below)
        try {
            SaleRecord s;
            s.saleId = stoi(data[0]);
            s.movieId = stoi(data[1]);
            s.movieTitle = data[2];
            s.genre = data[3];
            s.quantity = stoi(data[4]);
            s.pricePerTicket = stod(data[5]);
            s.subtotal = stod(data[6]);
            s.dateTime = data[7];
            // Backward compatibility: older save files might not have these two extra fields, so default them if missing
            s.seatNumbers = (data.size() > 8) ? data[8] : "N/A";
            s.paymentMethod = (data.size() > 9) ? data[9] : "Cash";
            if (s.saleId > maxId) maxId = s.saleId;
            sales.push_back(s);
        } catch (...) { continue; }    // Skip any line that fails to parse correctly
    }
    nextSaleId = maxId + 1;
}

// --- CONCESSIONS FUNCTIONS ---

// Writes every concession item to concessions.txt, one item per line, '|'-delimited.
void saveConcessions(const vector<ConcessionItem>& snacks) {
    ofstream fout("concessions.txt");
    for (const auto& s : snacks) {
        fout << s.id << '|' << s.name << '|' << s.category << '|' << s.price << '|' << s.stockQuantity << '\n';
    }
}

// Reads concessions.txt back into the snacks vector.
void loadConcessions(vector<ConcessionItem>& snacks, int& nextSnackId) {
    ifstream fin("concessions.txt");
    if (!fin) return;               // Guard: file may not exist yet on first run
    snacks.clear();
    string line;
    int maxId = 0;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string segment;
        vector<string> data;
        while (getline(ss, segment, '|')) data.push_back(segment);
        
        if (data.size() < 5) continue;    // Guard: skip lines that don't have all 5 expected fields
        
        try {
            ConcessionItem s;
            s.id = stoi(data[0]);
            s.name = data[1];
            s.category = data[2];
            s.price = stod(data[3]);
            s.stockQuantity = stoi(data[4]);
            
            if (s.id > maxId) maxId = s.id;
            snacks.push_back(s);
        } catch (...) { continue; }
    }
    nextSnackId = maxId + 1;
}
