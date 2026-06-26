#include "reports.h"
#include <iostream>
#include <iomanip>           // For setw(), setprecision(), fixed - used for formatted report tables
#include <algorithm>         // For sort()
#include <map>               // For map<string, ...> - used to group/aggregate data by genre
#include <set>               // For set<string> - used to collect a list of unique genre names
using namespace std;

// Shared aggregation helpers (also used by dashboard.cpp)

// Adds up the "subtotal" of every sale, across all movies, to get the grand total revenue.
double totalRevenueAll(const vector<SaleRecord>& sales) {
    double total = 0;
    for (const auto& s : sales) total += s.subtotal;   // Simple running-sum loop
    return total;
}

// Adds up the "quantity" of every sale, across all movies, to get the grand total tickets sold.
int totalTicketsAll(const vector<SaleRecord>& sales) {
    int total = 0;
    for (const auto& s : sales) total += s.quantity;
    return total;
}

// Labels a movie's performance by comparing its revenue against the average revenue of all movies with sales.
string classifyPerformance(double revenue, double avgRevenue, bool hasSales) {
    if (!hasSales) return "N/A (No Sales Yet)";    // Can't classify a movie that hasn't sold anything
    if (avgRevenue <= 0) return "N/A";              // Can't compute a meaningful ratio if there's no valid average to compare to
    double ratio = revenue / avgRevenue;            // How many times the average this movie's revenue is
    if (ratio >= 2.0) return "Blockbuster";         // Earned 2x+ the average -> Blockbuster
    if (ratio >= 1.0) return "Hit";                  // Earned at least the average -> Hit
    if (ratio >= 0.5) return "Average";              // Earned at least half the average -> Average
    return "Flop";                                    // Earned less than half the average -> Flop
}

// Helper (static = only visible/usable inside this file) that builds several lookup maps grouped by genre:
// how much revenue each genre made, how many tickets each genre sold, how many movies exist per genre,
// and the complete set of genre names seen (from either movies or sales).
static void buildGenreMaps(const vector<Movie>& movies, const vector<SaleRecord>& sales,
                            map<string, double>& genreRevenue, map<string, int>& genreTickets,
                            map<string, int>& genreMovieCount, set<string>& allGenres) {
    for (const auto& s : sales) {                  // Walk every sale and accumulate its genre's revenue/ticket totals
        genreRevenue[s.genre] += s.subtotal;       // map[key] auto-creates the entry with value 0 the first time it's used, then adds to it
        genreTickets[s.genre] += s.quantity;
        allGenres.insert(s.genre);                  // set::insert ignores duplicates automatically, so allGenres stays a unique list
    }
    for (const auto& m : movies) {                  // Walk every movie and count how many belong to each genre
        genreMovieCount[m.genre]++;
        allGenres.insert(m.genre);
    }
}

// Finds the genre with the highest AVERAGE revenue per movie (total genre revenue divided by how many movies are in that genre).
pair<string, double> mostProfitableGenre(const vector<Movie>& movies, const vector<SaleRecord>& sales) {
    map<string, double> genreRevenue; map<string, int> genreTickets, genreMovieCount; set<string> allGenres;
    buildGenreMaps(movies, sales, genreRevenue, genreTickets, genreMovieCount, allGenres);

    string bestGenre = "N/A"; double bestAvg = -1;
    for (const auto& g : allGenres) {               // Loop over every unique genre name found
        int count = genreMovieCount.count(g) ? genreMovieCount[g] : 0;   // map::count() checks if the key exists (returns 0 or 1)
        if (count == 0) continue;                    // Skip genres that have sales but somehow no movie entries (avoids divide-by-zero)
        double avg = genreRevenue[g] / count;
        if (avg > bestAvg) { bestAvg = avg; bestGenre = g; }   // Keep track of the genre with the highest average seen so far
    }
    return { bestGenre, bestAvg < 0 ? 0 : bestAvg };   // pair<string,double> returned as a brace-initialized list; clamp -1 sentinel to 0
}

// Finds the genre that sold the most total tickets (regardless of revenue).
pair<string, int> mostPopularGenre(const vector<SaleRecord>& sales) {
    map<string, int> genreTickets;
    for (const auto& s : sales) genreTickets[s.genre] += s.quantity;
    string best = "N/A"; int bestCount = -1;
    // Structured binding: "auto& [g, c]" unpacks each map entry into its key (g) and value (c) directly
    for (const auto& [g, c] : genreTickets) if (c > bestCount) { bestCount = c; best = g; }
    return { best, bestCount < 0 ? 0 : bestCount };
}

// Revenue & Financial Management (Section 5)
// Shows overall totals plus average/highest/lowest revenue among movies that have at least one sale.
void financialReport(const vector<Movie>& movies, const vector<SaleRecord>& sales) {
    cout << "\n--- FINANCIAL REPORT ---\n";
    double grandTotal = totalRevenueAll(sales);
    int grandTickets = totalTicketsAll(sales);

    vector<pair<const Movie*, double>> revenues;   // Will hold {pointer to movie, that movie's revenue} but only for movies with sales > 0
    for (const auto& m : movies) {
        double rev = totalRevenueForMovie(sales, m.id);
        if (rev > 0) revenues.push_back({ &m, rev });   // Only include movies that have actually sold something
    }

    cout << "Total Revenue (all-time, all movies): P" << fixed << setprecision(2) << grandTotal << '\n';
    cout << "Total Tickets Sold:                   " << grandTickets << '\n';

    if (revenues.empty()) {                            // Guard: nothing more to compute if no movie has sold a ticket yet
        cout << "\nNo per-movie revenue data yet (no tickets sold).\n";
        pauseScreen();
        return;
    }

    double sum = 0;
    auto highest = revenues[0], lowest = revenues[0];   // Start by assuming the first entry is both the highest and lowest
    for (const auto& r : revenues) {
        sum += r.second;                                 // r.second is the revenue value of this pair
        if (r.second > highest.second) highest = r;       // Replace "highest" whenever we find something bigger
        if (r.second < lowest.second) lowest = r;          // Replace "lowest" whenever we find something smaller
    }
    double avg = sum / revenues.size();

    cout << "\nAmong movies with at least one sale:\n";
    cout << "  Average Revenue per Movie: P" << fixed << setprecision(2) << avg << '\n';
    cout << "  Highest Revenue: " << highest.first->title << " (P" << fixed << setprecision(2) << highest.second << ")\n";   // first is the Movie pointer, so ->title accesses its title
    cout << "  Lowest Revenue:  " << lowest.first->title << " (P" << fixed << setprecision(2) << lowest.second << ")\n";
    pauseScreen();
}

// Analytics & Reports (Section 6)

// Scans every movie's revenue and reports whichever one earned the most.
void topPerformingMovie(const vector<Movie>& movies, const vector<SaleRecord>& sales) {
    cout << "\n--- TOP PERFORMING MOVIE ---\n";
    const Movie* best = nullptr; double bestRev = -1;
    for (const auto& m : movies) {
        double rev = totalRevenueForMovie(sales, m.id);
        if (rev > bestRev) { bestRev = rev; best = &m; }    // Keep updating "best" whenever we find a higher revenue
    }
    if (!best || bestRev <= 0) { cout << "No ticket sales recorded yet.\n"; pauseScreen(); return; }   // Guard: nothing sold yet
    cout << "Title:  " << best->title << '\n';
    cout << "Genre:  " << best->genre << '\n';
    cout << "Revenue: P" << fixed << setprecision(2) << bestRev << '\n';
    cout << "Tickets Sold: " << totalTicketsSoldForMovie(sales, best->id) << '\n';
    pauseScreen();
}

// Ranks every genre by total tickets sold, from most popular to least.
void mostPopularGenreReport(const vector<SaleRecord>& sales) {
    cout << "\n--- MOST POPULAR GENRE REPORT (by tickets sold) ---\n";
    if (sales.empty()) { cout << "No ticket sales recorded yet.\n"; pauseScreen(); return; }   // Guard: nothing to rank without sales

    map<string, int> genreTickets;
    for (const auto& s : sales) genreTickets[s.genre] += s.quantity;   // Group/aggregate quantity by genre

    vector<pair<string, int>> ranked(genreTickets.begin(), genreTickets.end());   // Copy the map's entries into a vector so we can sort() them (maps can't be sorted directly by value)
    sort(ranked.begin(), ranked.end(), [](auto& a, auto& b) { return a.second > b.second; });   // Sort descending by ticket count

    // UPDATED: Genre spacing increased to 25
    cout << left << setw(25) << "Genre" << "Tickets Sold\n" << string(40, '-') << '\n';   // string(40, '-') builds a line of 40 dashes
    for (const auto& [g, c] : ranked) cout << left << setw(25) << g << c << '\n';          // Structured binding again to unpack genre name and count

    cout << "\nMost Popular Genre: " << ranked.front().first << " (" << ranked.front().second << " tickets)\n";   // front() = first element, which is the top genre since we sorted descending
    pauseScreen();
}

// Shows, per genre, how many movies exist, how many tickets sold, and total revenue.
void genreAnalysis(const vector<Movie>& movies, const vector<SaleRecord>& sales) {
    cout << "\n--- GENRE ANALYSIS ---\n";
    map<string, double> genreRevenue; map<string, int> genreTickets, genreMovieCount; set<string> allGenres;
    buildGenreMaps(movies, sales, genreRevenue, genreTickets, genreMovieCount, allGenres);

    if (allGenres.empty()) { cout << "No genre data available yet.\n"; pauseScreen(); return; }   // Guard: no movies and no sales at all

    // UPDATED: Genre spacing increased to 25
    cout << left << setw(25) << "Genre" << setw(10) << "Movies" << setw(14) << "Tickets Sold" << "Revenue\n";
    cout << string(65, '-') << '\n';
    for (const auto& g : allGenres) {        // set<string> is automatically sorted alphabetically, so genres print in alphabetical order
        cout << left << setw(25) << g << setw(10) << genreMovieCount[g] << setw(14) << genreTickets[g]
             << "P" << fixed << setprecision(2) << genreRevenue[g] << '\n';
    }
    pauseScreen();
}

// Ranks genres by AVERAGE revenue per movie (a different metric than raw popularity/total revenue).
void genreProfitabilityAnalysis(const vector<Movie>& movies, const vector<SaleRecord>& sales) {
    cout << "\n--- GENRE PROFITABILITY ANALYSIS (Unique Feature) ---\n";
    cout << "Ranks genres by AVERAGE REVENUE PER MOVIE - distinct from raw\n";
    cout << "popularity, this highlights which genres earn the most per title.\n\n";

    map<string, double> genreRevenue; map<string, int> genreTickets, genreMovieCount; set<string> allGenres;
    buildGenreMaps(movies, sales, genreRevenue, genreTickets, genreMovieCount, allGenres);

    vector<tuple<string, double, int>> ranked;   // tuple holds 3 different values together: genre name, average revenue, movie count
    for (const auto& g : allGenres) {
        int count = genreMovieCount.count(g) ? genreMovieCount[g] : 0;
        if (count == 0) continue;                 // Skip genres with no movies (would cause divide-by-zero)
        double avg = genreRevenue[g] / count;
        ranked.push_back({ g, avg, count });
    }

    if (ranked.empty()) { cout << "Not enough data yet (no movies in inventory).\n"; pauseScreen(); return; }

    // get<1>(a) retrieves the 2nd element (index 1, the average) of the tuple, used to sort descending by profitability
    sort(ranked.begin(), ranked.end(), [](auto& a, auto& b) { return get<1>(a) > get<1>(b); });

    // UPDATED: Genre spacing increased to 25
    cout << left << setw(25) << "Genre" << setw(10) << "Movies" << setw(16) << "Total Revenue" << "Avg Rev/Movie\n";
    cout << string(75, '-') << '\n';
    for (const auto& [g, avg, count] : ranked) {    // Structured binding unpacking all 3 tuple elements at once
        cout << left << setw(25) << g << setw(10) << count
             << "P" << right << setw(13) << fixed << setprecision(2) << genreRevenue[g]
             << left << "   P" << fixed << setprecision(2) << avg << '\n';
    }

    cout << "\nMost Profitable Genre: " << get<0>(ranked.front())                                  // get<0> = genre name of the top-ranked entry
         << " (avg P" << fixed << setprecision(2) << get<1>(ranked.front()) << " per movie)\n";    // get<1> = its average revenue
    pauseScreen();
}

// Classifies every movie as Blockbuster/Hit/Average/Flop/N/A, based on its revenue vs the average revenue of movies with sales.
void moviePerformanceClassification(const vector<Movie>& movies, const vector<SaleRecord>& sales) {
    cout << "\n--- MOVIE PERFORMANCE CLASSIFICATION ---\n";
    if (movies.empty()) { cout << "No movies in the inventory.\n"; pauseScreen(); return; }   // Guard: nothing to classify

    vector<pair<const Movie*, double>> withSales;
    for (const auto& m : movies) {
        double rev = totalRevenueForMovie(sales, m.id);
        if (rev > 0) withSales.push_back({ &m, rev });   // Only movies that actually sold something count toward the average
    }
    double avgRevenue = 0;
    for (const auto& r : withSales) avgRevenue += r.second;
    if (!withSales.empty()) avgRevenue /= withSales.size();   // Guard against divide-by-zero if no movie has sales yet

    // UPDATED: Title spacing increased to 35 to accommodate longer titles in the report
    cout << left << setw(35) << "Title" << setw(15) << "Revenue" << setw(10) << "Tickets" << "Classification\n";
    cout << string(80, '-') << '\n';
    for (const auto& m : movies) {                       // Print every movie, even those with zero sales (they'll show "N/A (No Sales Yet)")
        double rev = totalRevenueForMovie(sales, m.id);
        int tix = totalTicketsSoldForMovie(sales, m.id);
        string label = classifyPerformance(rev, avgRevenue, rev > 0);
        cout << left << setw(35) << m.title << "P" << right << setw(13) << fixed << setprecision(2) << rev
             << left << "  " << setw(10) << tix << label << '\n';
    }
    if (withSales.empty()) cout << "\n(Classification needs ticket sales data - none recorded yet.)\n";
    pauseScreen();
}

// Sorting & Organization (Section 7, revenue/ticket-based)

// Rearranges the movies vector so the highest-revenue movie appears first.
void sortMoviesByRevenue(vector<Movie>& movies, const vector<SaleRecord>& sales) {
    // [&] in the lambda capture means "capture all surrounding variables by reference" - here it captures "sales" so the comparator can use it
    sort(movies.begin(), movies.end(), [&](const Movie& a, const Movie& b) {
        return totalRevenueForMovie(sales, a.id) > totalRevenueForMovie(sales, b.id);   // Descending order: bigger revenue comes first
    });
    cout << "\n--- MOVIES SORTED BY REVENUE (highest first) ---\n";
    //Title spacing increased to 35 to accommodate longer titles in the report
    cout << left << setw(35) << "Title" << "Revenue\n" << string(55, '-') << '\n';
    for (const auto& m : movies)
        cout << left << setw(35) << m.title << "P" << fixed << setprecision(2) << totalRevenueForMovie(sales, m.id) << '\n';
    pauseScreen();
}

// Rearranges the movies vector so the movie with the most tickets sold appears first.
void sortMoviesByTicketsSold(vector<Movie>& movies, const vector<SaleRecord>& sales) {
    sort(movies.begin(), movies.end(), [&](const Movie& a, const Movie& b) {
        return totalTicketsSoldForMovie(sales, a.id) > totalTicketsSoldForMovie(sales, b.id);   // Descending order: more tickets sold comes first
    });
    cout << "\n--- MOVIES SORTED BY TICKETS SOLD (highest first) ---\n";
    //Title spacing increased to 35 to accommodate longer titles in the report
    cout << left << setw(35) << "Title" << "Tickets Sold\n" << string(55, '-') << '\n';
    for (const auto& m : movies)
        cout << left << setw(35) << m.title << totalTicketsSoldForMovie(sales, m.id) << '\n';
    pauseScreen();
}
