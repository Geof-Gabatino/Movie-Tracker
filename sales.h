#pragma once               // Header guard: prevents this file from being included twice in the same compilation
#include <vector>
#include <string>
#include "movie.h"           // Needed because SaleRecord/functions here reference Movie and ConcessionItem
using namespace std;

// Holds the details of one completed ticket purchase. Each sale is recorded for history/reporting purposes.
struct SaleRecord {
    int saleId;            // Unique ID for this transaction
    int movieId;            // Which movie this sale was for (links back to Movie::id)
    string movieTitle;      // Movie title is duplicated here so sales history still shows it even if the movie is later deleted
    string genre;           // Same idea - genre is copied here for reporting even if the original movie record changes/disappears
    int quantity;            // How many tickets were purchased in this transaction
    double pricePerTicket;   // The price per ticket at the time of sale (in case the movie's price changes later)
    double subtotal;         // Total amount for this sale AFTER any discount is applied
    string dateTime;         // When the sale happened, "YYYY-MM-DD HH:MM:SS"
    string seatNumbers;      // BAGO (new) - reserved for storing assigned seat numbers
    string paymentMethod;    // BAGO (new) - reserved for storing how the customer paid
};

// Ticket Sales Functions
void recordTicketSale(vector<Movie>& movies, vector<SaleRecord>& sales, int& nextSaleId);   // Handles the full process of selling tickets for a movie
void viewSalesHistory(const vector<SaleRecord>& sales);                                       // Displays a log of all past ticket sales

// Helper functions used in reports/dashboard to compute totals for one specific movie.
int totalTicketsSoldForMovie(const vector<SaleRecord>& sales, int movieId);     // Sums up quantity across all sales for this movie
double totalRevenueForMovie(const vector<SaleRecord>& sales, int movieId);      // Sums up subtotal across all sales for this movie

// Concessions (snacks/drinks) management functions.
void addConcessionItem(vector<ConcessionItem>& snacks, int& nextSnackId);                          // Adds a new snack/drink to inventory
void viewAllConcessions(const vector<ConcessionItem>& snacks);                                     // Lists all concession items and their stock
void recordConcessionSale(vector<ConcessionItem>& snacks, int& nextSaleId);                        // Handles a cart-style snack purchase
void concessionsMenu(vector<ConcessionItem>& snacks, int& nextSnackId, int& nextSaleId);            // Sub-menu that routes to the concessions functions above
void deleteConcessionItem(vector<ConcessionItem>& snacks);                                          // Removes a concession item permanently
