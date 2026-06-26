#include "sales.h"
#include "filemanager.h"     // Needed for saveMovies()/saveSales()/saveConcessions() calls that persist data after each transaction
#include <iostream>
#include <iomanip>           // For setw(), setprecision(), fixed - used to format the printed receipts
#include <algorithm>         // Included for general utility (not heavily used here, but kept for string/transform support)
#include <sstream>           // Not directly used for parsing here, but available for any text-building needs
using namespace std;

// TICKET SALES & MOVIE RECEIPT
// Walks the user through selecting a movie, choosing quantity/discount, then records the sale and prints a receipt.
void recordTicketSale(vector<Movie>& movies, vector<SaleRecord>& sales, int& nextSaleId) {
    cout << "\n----------- RECORD TICKET SALE -----------\n";
    bool anyShowing = false;
    printMovieHeader();
    for (const auto& m : movies) {                       // Only show movies that are currently NowShowing - others can't be sold
        if (m.status == MovieStatus::NowShowing) { printMovieRow(m); anyShowing = true; }
    }
    if (!anyShowing) {                                   // Guard: nothing to sell if no movie is currently showing
        cout << "No movies are currently showing.\n"; 
        cout << "--------------------------------------------------------\n";
        pauseScreen(); 
        return; 
    }

    int id = askInt("\nEnter Movie ID: ");
    Movie* m = findMovieById(movies, id);                // Pointer so we can later modify m->seatsAvailable directly
    if (!m) {                                              // Guard: typed an ID that doesn't exist
        cout << "Movie not found.\n"; 
        cout << "--------------------------------------------------------\n";
        pauseScreen(); 
        return; 
    }
    if (m->status == MovieStatus::Upcoming) {             // Guard: can't sell tickets for a movie that hasn't released yet
        cout << m->title << " hasn't been released yet.\n"; 
        cout << "--------------------------------------------------------\n";
        pauseScreen(); 
        return; 
    }
    if (m->status == MovieStatus::Archived) {              // Guard: can't sell tickets for a movie that already finished its run
        cout << m->title << " has been pulled out / archived.\n"; 
        cout << "--------------------------------------------------------\n";
        pauseScreen(); 
        return; 
    }
    if (m->seatsAvailable <= 0) {                          // Guard: no seats left to sell
        cout << m->title << " is SOLD OUT.\n"; 
        cout << "--------------------------------------------------------\n";
        pauseScreen(); 
        return; 
    }

    int qty = askInt("Number of tickets (Available: " + to_string(m->seatsAvailable) + "): ", 1);   // minVal 1: must buy at least 1 ticket
    if (qty > m->seatsAvailable) {                          // Guard: can't sell more tickets than seats remaining
        cout << "Insufficient seats available. Only " << m->seatsAvailable << " seats left.\n"; 
        cout << "--------------------------------------------------------\n";
        pauseScreen(); 
        return; 
    }

    // --- DISCOUNT SELECTION ---
    cout << "\n--- SELECT DISCOUNT TYPE ---\n";
    cout << "[1] None\n";
    cout << "[2] Senior Citizen (20%)\n";
    cout << "[3] PWD (20%)\n";
    cout << "[4] Student & Teacher (20%)\n";
    cout << "[5] Early Screening Promo (30%)\n";
    int discChoice = askInt("Enter choice (1-5): ", 1);

    double discountRate = 0.0;
    string discountName = "None";

    switch(discChoice) {                                    // Maps the chosen menu number to a discount percentage and label
        case 2: discountRate = 0.20; discountName = "Senior Citizen"; break;
        case 3: discountRate = 0.20; discountName = "PWD"; break;
        case 4: discountRate = 0.20; discountName = "Student & Teacher"; break;
        case 5: discountRate = 0.30; discountName = "Early Screening Promo"; break;
        default: discountRate = 0.0; discountName = "None"; break;   // Covers choice 1 and any unexpected number
    }

    // Calculations
    double originalSubtotal = qty * m->price;                       // Price before any discount
    double discountAmount = originalSubtotal * discountRate;        // How much money is taken off
    double discountedSubtotal = originalSubtotal - discountAmount;  // Final price after discount
    
    // Seat Deduction
    m->seatsAvailable -= qty;     // Reduce the movie's available seat count by however many tickets were just bought

    SaleRecord rec;                            // Build a new sale record to represent this transaction
    rec.saleId = nextSaleId++;                 // Use the current ID, then increment the counter for next time (post-increment)
    rec.movieId = m->id;
    rec.movieTitle = m->title;
    rec.genre = m->genre;
    rec.quantity = qty;
    rec.pricePerTicket = m->price;
    rec.subtotal = discountedSubtotal;         // Saving the final price after discount
    rec.dateTime = getCurrentDateTime();

    sales.push_back(rec);     // Add this transaction to the sales history vector
    saveMovies(movies);       // Persist the updated seat count to disk
    saveSales(sales);         // Persist the new sale record to disk

    double amusementTax = discountedSubtotal * 0.10;   // 10% amusement tax applied on top of the discounted price
    double culturalTax = 0.0;                            // Placeholder tax, currently always zero
    double totalDue = discountedSubtotal + amusementTax + culturalTax;

    // --- Printing the formatted receipt ---
    cout << "\n============================================\n";
    cout << "           MELO MOVIE THEATRE               \n";
    cout << "             TICKET RECEIPT                 \n";
    cout << "============================================\n";
    cout << "Transaction ID: " << rec.saleId << '\n';
    cout << "Date:           " << rec.dateTime << "\n";
    cout << "--------------------------------------------\n";
    cout << "MOVIE DETAILS\n";
    cout << "Title:      " << rec.movieTitle << '\n';
    cout << "Genre:      " << rec.genre << '\n';
    cout << "Show Time:  " << m->showtime << "\n\n";
    cout << "PRICING INFO\n";
    cout << qty << " Ticket(s) x P" << fixed << setprecision(2) << rec.pricePerTicket << '\n';
    // left/right + setw() are used together here to make labels left-aligned and amounts right-aligned, like a real receipt
    cout << left << setw(25) << "Subtotal:" << "P" << right << setw(10) << fixed << setprecision(2) << originalSubtotal << '\n';
    cout << left << setw(25) << ("Discount (" + discountName + "):") << "-P" << right << setw(9) << fixed << setprecision(2) << discountAmount << '\n';
    cout << left << setw(25) << "Amusement Tax (10%):" << "P" << right << setw(10) << fixed << setprecision(2) << amusementTax << '\n';
    cout << left << setw(25) << "Cultural Tax:" << "P" << right << setw(10) << fixed << setprecision(2) << culturalTax << '\n';
    cout << "--------------------------------------------\n";
    cout << left << setw(25) << "TOTAL DUE:" << "P" << right << setw(10) << fixed << setprecision(2) << totalDue << '\n';
    cout << "--------------------------------------------\n";
    cout << "Thank you for choosing Melo Movie Theatre!\n";
    cout << "Enjoy your movie!\n";
    cout << "============================================\n";
    pauseScreen();
}

// Displays a simple table listing every past ticket sale recorded so far.
void viewSalesHistory(const vector<SaleRecord>& sales) {
    cout << "\n----------- TICKET SALES ACTIVITY LOG -----------\n";
    if (sales.empty()) {                       // Guard: nothing to show if there have been no sales yet
        cout << "No sales transactions recorded yet.\n"; 
        cout << "--------------------------------------------------------\n";
        pauseScreen(); 
        return; 
    }
    cout << left << setw(8) << "Sale ID" << setw(25) << "Movie Title" << setw(6) << "Qty" << setw(12) << "Subtotal" << "Date\n";
    cout << "------------------------------------------------------------------------\n";
    for (const auto& s : sales) {              // Loop through every sale and print a formatted row for it
        cout << left << setw(8) << s.saleId 
             << setw(25) << (s.movieTitle.length() > 24 ? s.movieTitle.substr(0, 21) + "..." : s.movieTitle)   // Truncate long titles so the table stays aligned
             << setw(6) << s.quantity 
             << "P" << setw(11) << fixed << setprecision(2) << s.subtotal 
             << s.dateTime << '\n';
    }
    cout << "------------------------------------------------------------------------\n";
    pauseScreen();
}

// Adds up the quantity field across every sale that belongs to the given movie ID.
int totalTicketsSoldForMovie(const vector<SaleRecord>& sales, int movieId) {
    int count = 0;
    for (const auto& s : sales) if (s.movieId == movieId) count += s.quantity;   // Only accumulate sales matching this movie
    return count;
}

// Adds up the subtotal field across every sale that belongs to the given movie ID.
double totalRevenueForMovie(const vector<SaleRecord>& sales, int movieId) {
    double rev = 0;
    for (const auto& s : sales) if (s.movieId == movieId) rev += s.subtotal;
    return rev;
}

// Prompts the user for new snack/drink details and appends it to the concessions inventory.
void addConcessionItem(vector<ConcessionItem>& snacks, int& nextSnackId) {
    cout << "\n----------- ADD CONCESSION ITEM -----------\n";
    ConcessionItem item;
    item.id = nextSnackId++;    // Assign current ID, then increment counter for the next item created
    item.name = askLine("Enter Snack Name: ");
    item.category = askLine("Enter Category (Food/Beverage/etc.): ");
    item.price = askDouble("Enter Unit Price: ", 0.0);
    item.stockQuantity = askInt("Enter Initial Stock Quantity: ", 0);

    snacks.push_back(item);
    saveConcessions(snacks);    // Persist the new inventory list to disk
    cout << "\nItem \"" << item.name << "\" added successfully into inventory!\n";
    pauseScreen();
}

// Prints a table of every concession item, including stock count and an "OUT OF STOCK" flag when applicable.
void viewAllConcessions(const vector<ConcessionItem>& snacks) {
    cout << "\n----------- CONCESSIONS INVENTORY -----------\n";
    if (snacks.empty()) {                       // Guard: nothing to display if inventory is empty
        cout << "No concession items available in inventory.\n"; 
        cout << "--------------------------------------------------------\n";
        pauseScreen(); 
        return; 
    }
    cout << left << setw(5) << "ID" << setw(25) << "Item Name" << setw(15) << "Category" << setw(10) << "Price" << "Stock\n";
    cout << "------------------------------------------------------------------------\n";
    for (const auto& s : snacks) {
        cout << left << setw(5) << s.id 
             << setw(25) << s.name 
             << setw(15) << s.category 
             << "P" << setw(9) << fixed << setprecision(2) << s.price 
             // Ternary: appends a warning label only when stock has hit zero
             << s.stockQuantity << (s.stockQuantity == 0 ? " (OUT OF STOCK)" : "") << '\n';
    }
    cout << "------------------------------------------------------------------------\n";
    pauseScreen();
}

// Permanently removes a concession item after the user types "YES" to confirm.
void deleteConcessionItem(vector<ConcessionItem>& snacks) {
    cout << "\n----------- DELETE CONCESSION ITEM -----------\n";
    int id = askInt("Enter Concession Item ID to delete: ");
    // Iterator loop (instead of range-based for) because erase() requires an iterator position, not just a value
    for (auto it = snacks.begin(); it != snacks.end(); ++it) {
        if (it->id == id) {
            cout << "Are you sure you want to delete \"" << it->name << "\"?\n";
            string confirm = toLowerStr(askLine("Type 'YES' to confirm: "));
            if (confirm == "yes") {
                snacks.erase(it);             // Removes this element from the vector, shifting later elements left
                saveConcessions(snacks);
                cout << "\nItem deleted from database.\n";
            } else {
                cout << "\nDeletion canceled.\n";
            }
            pauseScreen();
            return;     // Stop here once the matching item has been handled
        }
    }
    cout << "Item ID not found.\n";   // Reached only if no item with that ID was found in the loop
    pauseScreen();
}

// Lets the user "shop" for multiple concession items in one transaction (a cart), then prints a combined receipt.
void recordConcessionSale(vector<ConcessionItem>& snacks, int& nextSaleId) {
    cout << "\n----------- PROCESS SNACK SALE -----------\n";
    if (snacks.empty()) {                       // Guard: nothing to sell if inventory is empty
        cout << "No concessions available to sell.\n"; 
        cout << "--------------------------------------------------------\n";
        pauseScreen(); 
        return; 
    }

    vector<pair<int, int>> cart;   // Each element is a pair: <index of the snack in "snacks", quantity bought>
    double subtotal = 0;
    char addMore;

    do {                                       // do-while loop: always runs at least once, then repeats while the user wants to add more
        viewAllConcessions(snacks);
        int id = askInt("\nEnter Concession Item ID to buy: ");
        int foundIdx = -1;
        for (size_t i = 0; i < snacks.size(); ++i) {     // size_t is the unsigned integer type vector::size() returns; used to avoid signed/unsigned mismatch warnings
            if (snacks[i].id == id) { foundIdx = i; break; }   // break exits the loop early once a match is found
        }

        if (foundIdx == -1) {                              // -1 means the loop above never found a match
            cout << "Item not found.\n";
        } else if (snacks[foundIdx].stockQuantity <= 0) {
            cout << "Sorry, \"" << snacks[foundIdx].name << "\" is out of stock.\n";
        } else {
            int qty = askInt("Enter quantity (Available: " + to_string(snacks[foundIdx].stockQuantity) + "): ", 1);
            if (qty > snacks[foundIdx].stockQuantity) {     // Guard: can't buy more than what's in stock
                cout << "Not enough stock. Transaction canceled for this item.\n";
            } else {
                snacks[foundIdx].stockQuantity -= qty;       // Deduct purchased quantity from stock immediately
                cart.push_back({foundIdx, qty});             // Remember this purchase in the cart for the receipt later
                subtotal += (snacks[foundIdx].price * qty);
                cout << qty << "x \"" << snacks[foundIdx].name << "\" added to transaction cart.\n";
            }
        }
        cout << "\nAdd more snacks to this transaction? (Y/N): ";
        string ans = askLine("");
        addMore = (!ans.empty() ? toupper(ans[0]) : 'N');    // Take just the first character, uppercase it; default to 'N' if blank
    } while (addMore == 'Y');

    if (cart.empty()) {                                       // Guard: if nothing was successfully added, abort the whole transaction
        cout << "\nCart is empty. Transaction aborted.\n";
        pauseScreen();
        return;
    }

    saveConcessions(snacks);    // Persist the updated stock quantities to disk
    double vat = subtotal * 0.12;            // 12% value-added tax
    double totalDue = subtotal + vat;

    // --- Printing the combined cart receipt ---
    cout << "\n============================================\n";
    cout << "           MELO MOVIE THEATRE               \n";
    cout << "          CONCESSIONS RECEIPT               \n";
    cout << "============================================\n";
    cout << "Transaction ID: SNACK-" << nextSaleId++ << '\n';   // Note: this reuses/increments the same counter as ticket sale IDs
    cout << "Date:           " << getCurrentDate() << "\n";
    cout << "--------------------------------------------\n";
    cout << "ITEMS PURCHASED:\n";
    for (const auto& item : cart) {           // item is a pair<int,int>: item.first = index into snacks, item.second = quantity bought
        int idx = item.first;
        int qty = item.second;
        double itemCost = snacks[idx].price * qty;
        cout << left << setw(22) << snacks[idx].name 
             << "x" << setw(4) << qty 
             << "P" << right << setw(10) << fixed << setprecision(2) << itemCost << '\n';
    }
    cout << "--------------------------------------------\n";
    cout << left << setw(25) << "Subtotal:" << "P" << right << setw(10) << fixed << setprecision(2) << subtotal << '\n';
    cout << left << setw(25) << "VAT (12%):" << "P" << right << setw(10) << fixed << setprecision(2) << vat << '\n';
    cout << "--------------------------------------------\n";
    cout << left << setw(25) << "TOTAL DUE:" << "P" << right << setw(10) << fixed << setprecision(2) << totalDue << '\n';
    cout << "--------------------------------------------\n";
    cout << "Thank you!\nEnjoy your snacks!\n";
    cout << "============================================\n";
    pauseScreen();
}

// Sub-menu that lets the user choose what they want to do within Concessions Management.
void concessionsMenu(vector<ConcessionItem>& snacks, int& nextSnackId, int& nextSaleId) {
    int choice;
    do {                                        // do-while so the menu always shows at least once before checking for exit
        cout << "\n----------- CONCESSIONS MANAGEMENT -----------\n"
             << "  [1] Add Concession Item (Inventory)\n"
             << "  [2] View All Items\n"
             << "  [3] Process Snack Sale (Cart System)\n"
             << "  [4] Delete Concession Item\n"
             << "  [0] Back to Main Menu\n"
             << "----------------------------------------------\n";
        choice = askInt("Enter choice: ");
        switch (choice) {                        // Routes the chosen menu number to the matching function
            case 1: addConcessionItem(snacks, nextSnackId); break;
            case 2: viewAllConcessions(snacks); break;
            case 3: recordConcessionSale(snacks, nextSaleId); break;
            case 4: deleteConcessionItem(snacks); break;
            case 0: break;                        // 0 just breaks out of the switch; the loop condition below then ends the menu
            default: cout << "Invalid choice.\n"; cout << "--------------------------------------------------------\n"; pauseScreen();
        }
    } while (choice != 0);    // Keep looping until the user selects 0 (Back to Main Menu)
}
