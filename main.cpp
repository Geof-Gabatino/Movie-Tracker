#include "movie.h"
#include "sales.h"
#include "filemanager.h"
#include "reports.h"
#include "dashboard.h"
#include <iostream>
using namespace std;

// Sub-menu for everything related to creating/editing/removing movies (not viewing them).
// movies is passed by reference (&) so any add/update/delete actually changes the original list back in main().
// nextId is also passed by reference because addMovie() needs to increment the shared ID counter.
void movieManagementMenu(vector<Movie>& movies, int& nextId) {
    int choice;
    do {                                   // do-while: show the menu at least once, then keep repeating until the user picks 0 (Back)
        cout << "\n--- MOVIE MANAGEMENT ---\n"
             << "[1] Add Movie\n[2] Update Movie\n[3] Search Movie\n"
             << "[4] Archive Movie\n[5] Delete Movie\n[0] Back\n";
        choice = askInt("Enter choice: ");
        switch (choice) {                    // Routes the numeric choice to the matching function
            case 1: addMovie(movies, nextId); break;
            case 2: updateMovie(movies); break;
            case 3: searchMovie(movies); break;
            case 4: archiveMovie(movies); break;
            case 5: deleteMovie(movies); break;
            case 0: break;                     // 0 just exits the switch; the do-while condition below then ends the loop
            default: cout << "Invalid choice.\n"; pauseScreen();   // Anything else (e.g. typing 9) falls here
        }
    } while (choice != 0);
}

// Sub-menu for browsing/viewing movies by various filters (read-only, doesn't modify the list except refreshing statuses).
void movieViewingMenu(vector<Movie>& movies) {
    int choice;
    do {
        updateMovieStatuses(movies);    // Refresh every movie's status each time this menu is shown, so dates are always up to date
        cout << "\n--- MOVIE VIEWING ---\n"
             << "[1] View All Movies\n[2] View Now Showing\n[3] View Upcoming\n"
             << "[4] View Archived\n[0] Back\n";
        choice = askInt("Enter choice: ");
        switch (choice) {
            case 1: viewAllMovies(movies); break;
            case 2: viewByStatus(movies, MovieStatus::NowShowing); break;
            case 3: viewByStatus(movies, MovieStatus::Upcoming); break;
            case 4: viewByStatus(movies, MovieStatus::Archived); break;
            case 0: break;
            default: cout << "Invalid choice.\n"; pauseScreen();
        }
    } while (choice != 0);
}

// Sub-menu for recording ticket sales and viewing the sales/activity history.
void ticketSalesMenu(vector<Movie>& movies, vector<SaleRecord>& sales, int& nextSaleId) {
    int choice;
    do {
        cout << "\n--- TICKET SALES ---\n"
             << "[1] Record Ticket Sale\n[2] View Sales History / Activity Log\n[0] Back\n";
        choice = askInt("Enter choice: ");
        switch (choice) {
            case 1: recordTicketSale(movies, sales, nextSaleId); break;
            case 2: viewSalesHistory(sales); break;
            case 0: break;
            default: cout << "Invalid choice.\n"; pauseScreen();
        }
    } while (choice != 0);
}

// Sub-menu collecting all the revenue/financial/analytics report options.
void analyticsMenu(vector<Movie>& movies, vector<SaleRecord>& sales) {
    int choice;
    do {
        cout << "\n--- REVENUE, FINANCIAL & ANALYTICS REPORTS ---\n"
             << "[1] Financial Report (Total/Avg/Highest/Lowest Revenue)\n"
             << "[2] Top Performing Movie\n[3] Most Popular Genre Report\n"
             << "[4] Genre Analysis\n[5] Genre Profitability Analysis\n"
             << "[6] Movie Performance Classification\n[0] Back\n";
        choice = askInt("Enter choice: ");
        switch (choice) {
            case 1: financialReport(movies, sales); break;
            case 2: topPerformingMovie(movies, sales); break;
            case 3: mostPopularGenreReport(sales); break;
            case 4: genreAnalysis(movies, sales); break;
            case 5: genreProfitabilityAnalysis(movies, sales); break;
            case 6: moviePerformanceClassification(movies, sales); break;
            case 0: break;
            default: cout << "Invalid choice.\n"; pauseScreen();
        }
    } while (choice != 0);
}

// Sub-menu for sorting the movie list and viewing the total movies counter.
void sortingMenu(vector<Movie>& movies, vector<SaleRecord>& sales) {
    int choice;
    do {
        cout << "\n--- SORTING & ORGANIZATION ---\n"
             << "[1] Sort by Title\n[2] Sort by Revenue\n[3] Sort by Tickets Sold\n"
             << "[4] Total Movies Counter\n[0] Back\n";
        choice = askInt("Enter choice: ");
        switch (choice) {
            case 1: sortMoviesByTitle(movies); break;
            case 2: sortMoviesByRevenue(movies, sales); break;
            case 3: sortMoviesByTicketsSold(movies, sales); break;
            case 4: totalMoviesCounter(movies); break;
            case 0: break;
            default: cout << "Invalid choice.\n"; pauseScreen();
        }
    } while (choice != 0);
}

// Sub-menu for the smart dashboard features (overview snapshot + rule-based recommendations).
void dashboardMenu(vector<Movie>& movies, vector<SaleRecord>& sales) {
    int choice;
    do {
        cout << "\n--- SMART DASHBOARDS ---\n"
             << "[1] Cinema Dashboard\n"
             << "[2] Cinema Recommendation System\n[0] Back\n";
        choice = askInt("Enter choice: ");
        switch (choice) {
            case 1: cinemaDashboard(movies, sales); break;
            case 2: cinemaRecommendationSystem(movies, sales); break;
            case 0: break;
            default: cout << "Invalid choice.\n"; pauseScreen();
        }
    } while (choice != 0);
}

// Program entry point - this is where execution starts and ends.
int main() {
    // These vectors hold all of the program's data in memory while it runs.
    vector<Movie> movies;
    vector<SaleRecord> sales;
    vector<ConcessionItem> snacks; 
    // These counters track the next unique ID to assign for each type of record; passed by reference wherever new records are created.
    int nextId = 1, nextSaleId = 1, nextSnackId = 1;

    // Load any previously saved data from disk into memory before the program starts interacting with the user.
    loadMovies(movies, nextId);
    loadSales(sales, nextSaleId);
    loadConcessions(snacks, nextSnackId); 
    updateMovieStatuses(movies);    // Make sure statuses reflect today's date right away, in case time has passed since the last save

    int choice;
    do {                              // Main program loop: keeps showing the main menu until the user chooses to exit (0)
        updateMovieStatuses(movies);   // Re-check statuses every time the main menu is redisplayed (e.g. after returning from a sub-menu)
        cout << "\n===== MOVIE THEATRE MANAGEMENT SYSTEM =====\n"
             << "Date: " << getCurrentDate() << "\n"
             << "[1] Movie Management\n[2] Movie Viewing\n"
             << "[3] Ticket Sales\n[4] Concessions Management\n"
             << "[5] Revenue, Financial & Analytics Reports\n"
             << "[6] Sorting & Organization\n[7] Smart Dashboards\n[0] Exit\n";
        choice = askInt("Enter choice: ");
        switch (choice) {                              // Routes the main menu choice to the correct sub-menu function
            case 1: movieManagementMenu(movies, nextId); break; 
            case 2: movieViewingMenu(movies); break; 
            case 3: ticketSalesMenu(movies, sales, nextSaleId); break; 
            case 4: concessionsMenu(snacks, nextSnackId, nextSaleId); break;  
            case 5: analyticsMenu(movies, sales); break; 
            case 6: sortingMenu(movies, sales); break;
            case 7: dashboardMenu(movies, sales); break; 
            case 0:                                       // Exit option: save everything one final time before closing
                saveMovies(movies);
                saveSales(sales);
                saveConcessions(snacks); 
                cout << "Data saved. Goodbye!\n";
                break;
            default: cout << "Invalid choice.\n"; pauseScreen();
        }
    } while (choice != 0);    // Keep looping until the user selects 0 (Exit)

    return 0;   // Returning 0 from main() signals to the operating system that the program finished successfully
}
