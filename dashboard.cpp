#include "dashboard.h"
#include "reports.h"         // Needed for totalRevenueAll(), totalTicketsAll(), mostProfitableGenre(), mostPopularGenre(), classifyPerformance()
#include <iostream>
#include <iomanip>           // For setprecision()/fixed - used to format peso amounts
using namespace std;

// ===================== CINEMA DASHBOARD =====================
// Quick snapshot + proactive intelligence in one view:
// - Movie counts by status, total tickets sold, total revenue
// - Top performing movie & most profitable genre
// - Movies releasing / pulling out within 3 days
// - Movies running low on seats (<=5 left)
void cinemaDashboard(const vector<Movie>& movies, const vector<SaleRecord>& sales) {
    cout << "\n========== CINEMA DASHBOARD ==========\n";

    // --- Counts by status ---
    int upcoming = 0, nowShowing = 0, archived = 0;
    for (const auto& m : movies) {                       // Single pass through every movie, tallying which status bucket it falls into
        if (m.status == MovieStatus::Upcoming) upcoming++;
        else if (m.status == MovieStatus::NowShowing) nowShowing++;
        else archived++;
    }

    cout << "Total Movies:        " << movies.size() << '\n';
    cout << "  Now Showing:       " << nowShowing << '\n';
    cout << "  Upcoming:          " << upcoming << '\n';
    cout << "  Archived:          " << archived << '\n';
    cout << "Total Tickets Sold:  " << totalTicketsAll(sales) << '\n';     // Reused helper from reports.cpp/.h
    cout << "Total Revenue:       P" << fixed << setprecision(2) << totalRevenueAll(sales) << '\n';
    cout << "---------------------------------------\n";

    // --- Top performing movie ---
    const Movie* topMovie = nullptr; double topRev = -1;
    for (const auto& m : movies) {                       // Linear scan to find whichever movie has earned the most revenue
        double rev = totalRevenueForMovie(sales, m.id);
        if (rev > topRev) { topRev = rev; topMovie = &m; }
    }
    cout << "\nTOP PERFORMING MOVIE\n";
    if (topMovie && topRev > 0) {                          // Guard: only show a result if there is actually a movie with sales
        cout << "  " << topMovie->title << " - P" << fixed << setprecision(2) << topRev
             << " (" << totalTicketsSoldForMovie(sales, topMovie->id) << " tickets)\n";
    } else {
        cout << "  No ticket sales recorded yet.\n";
    }

    // --- Most profitable genre ---
    auto profGenrePair = mostProfitableGenre(movies, sales);   // Returns a pair<string,double>: {genre name, avg revenue}
    cout << "\nMOST PROFITABLE GENRE\n";
    if (profGenrePair.first != "N/A" && profGenrePair.second > 0) {   // .first = genre name, .second = its average revenue
        cout << "  " << profGenrePair.first << " (avg P" << fixed << setprecision(2)
             << profGenrePair.second << " per movie)\n";
    } else {
        cout << "  Not enough data yet.\n";
    }

    string today = getCurrentDate();

    // --- Releasing within 3 days ---
    cout << "\nRELEASING WITHIN 3 DAYS\n";
    bool anyReleasing = false;
    for (const auto& m : movies) {
        if (m.status == MovieStatus::Upcoming) {            // Only Upcoming movies are relevant to a "releasing soon" alert
            int d = daysBetween(today, m.releaseDate);        // How many days from today until this movie's release date
            if (d >= 0 && d <= 3) {                            // Within the next 3 days (and not already in the past)
                cout << "  " << m.title << " - releases in " << d << " day(s) (" << m.releaseDate << ")\n";
                anyReleasing = true;
            }
        }
    }
    if (!anyReleasing) cout << "  None.\n";   // Fallback message if the loop never found a match

    // --- Pulling out within 3 days ---
    cout << "\nPULLING OUT WITHIN 3 DAYS\n";
    bool anyPullout = false;
    for (const auto& m : movies) {
        if (m.status == MovieStatus::NowShowing) {           // Only currently-showing movies can be "pulling out soon"
            int d = daysBetween(today, m.pullOutDate);
            if (d >= 0 && d <= 3) {
                cout << "  " << m.title << " - pulls out in " << d << " day(s) (" << m.pullOutDate << ")\n";
                anyPullout = true;
            }
        }
    }
    if (!anyPullout) cout << "  None.\n";

    // --- Low seats remaining ---
    cout << "\nLOW SEATS REMAINING (5 or fewer)\n";
    bool anyLowSeats = false;
    for (const auto& m : movies) {
        if (m.status == MovieStatus::NowShowing && m.seatsAvailable <= 5) {   // Alert threshold: 5 or fewer seats left
            cout << "  " << m.title << " - " << m.seatsAvailable << " seat(s) left\n";
            anyLowSeats = true;
        }
    }
    if (!anyLowSeats) cout << "  None.\n";

    cout << "=======================================\n";
    pauseScreen();
}

// ================= CINEMA RECOMMENDATION SYSTEM =================
// Rule-based suggestions generated from real classification + lifecycle data:
// - Booking more movies in the most profitable / most popular genre
// - Extending a nearly-sold-out Hit/Blockbuster
// - Archiving an underperforming Flop early
void cinemaRecommendationSystem(const vector<Movie>& movies, const vector<SaleRecord>& sales) {
    cout << "\n===== CINEMA RECOMMENDATION SYSTEM =====\n";

    if (movies.empty()) {                                    // Guard: no movies means no recommendations are possible
        cout << "No movies in inventory yet. Add movies to get recommendations.\n";
        cout << "=========================================\n";
        pauseScreen();
        return;
    }

    // Average revenue among movies that have at least one sale (same basis as classifyPerformance)
    vector<pair<const Movie*, double>> withSales;
    for (const auto& m : movies) {
        double rev = totalRevenueForMovie(sales, m.id);
        if (rev > 0) withSales.push_back({ &m, rev });        // Only count movies that actually have sales toward the average
    }
    double avgRevenue = 0;
    for (const auto& r : withSales) avgRevenue += r.second;
    if (!withSales.empty()) avgRevenue /= withSales.size();    // Guard against divide-by-zero if nothing has sold yet

    bool anyTip = false;     // Tracks whether we printed at least one recommendation, so we know whether to show a fallback message

    // 1. Most profitable genre -> suggest booking more
    auto profGenrePair = mostProfitableGenre(movies, sales);
    string profGenre = profGenrePair.first;
    double profAvg = profGenrePair.second;
    if (profGenre != "N/A" && profAvg > 0) {
        cout << "- Your most profitable genre is " << profGenre << " (avg P" << fixed << setprecision(2)
             << profAvg << " per movie). Consider booking more " << profGenre << " movies.\n";
        anyTip = true;
    }

    // 2. Most popular genre by tickets sold (if different from the profitable one)
    auto popGenrePair = mostPopularGenre(sales);
    string popGenre = popGenrePair.first;
    int popCount = popGenrePair.second;
    if (popGenre != "N/A" && popCount > 0 && popGenre != profGenre) {   // Only show this tip if it's a genuinely different insight from tip #1
        cout << "- " << popGenre << " is your most popular genre by tickets sold (" << popCount
             << " tickets). Audiences are clearly drawn to it - keep it well-stocked.\n";
        anyTip = true;
    }

    // 3. Per-movie suggestions based on performance classification + lifecycle
    for (const auto& m : movies) {
        double rev = totalRevenueForMovie(sales, m.id);
        bool hasSales = rev > 0;
        string label = classifyPerformance(rev, avgRevenue, hasSales);   // Reuses the same classification logic as the reports module

        // If a movie is doing great AND is almost sold out, suggest extending its run
        if ((label == "Hit" || label == "Blockbuster") && m.status == MovieStatus::NowShowing && m.seatsAvailable <= 5) {
            cout << "- \"" << m.title << "\" is a " << label << " and nearly sold out ("
                 << m.seatsAvailable << " seat(s) left). Consider extending its pull-out date.\n";
            anyTip = true;
        }
        // If a movie is underperforming while still showing, suggest archiving it early
        if (label == "Flop" && m.status == MovieStatus::NowShowing) {
            cout << "- \"" << m.title << "\" is underperforming (Flop). Consider archiving it early to free up the showtime slot.\n";
            anyTip = true;
        }
    }

    if (!anyTip) {                 // If none of the rules above triggered any tip, let the user know data is still too thin
        cout << "No specific recommendations yet - record more ticket sales to unlock insights.\n";
    }

    cout << "=========================================\n";
    pauseScreen();
}
