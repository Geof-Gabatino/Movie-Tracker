#pragma once
#include <vector>
#include <string>
#include "movie.h"
#include "sales.h"
using namespace std;

// ----- Revenue & Financial Management (Section 5) -----
void financialReport(const vector<Movie>& movies, const vector<SaleRecord>& sales);   // Shows total/average/highest/lowest revenue across movies

// ----- Analytics & Reports (Section 6) -----
void topPerformingMovie(const vector<Movie>& movies, const vector<SaleRecord>& sales);          // Finds and displays the single highest-earning movie
void mostPopularGenreReport(const vector<SaleRecord>& sales);                                    // Ranks genres by total tickets sold
void genreAnalysis(const vector<Movie>& movies, const vector<SaleRecord>& sales);                // Breaks down movie count/tickets/revenue per genre
void genreProfitabilityAnalysis(const vector<Movie>& movies, const vector<SaleRecord>& sales);   // Ranks genres by average revenue per movie
void moviePerformanceClassification(const vector<Movie>& movies, const vector<SaleRecord>& sales); // Labels each movie as Blockbuster/Hit/Average/Flop

// ----- Sorting (Section 7, revenue/ticket-based; needs sales data) -----
void sortMoviesByRevenue(vector<Movie>& movies, const vector<SaleRecord>& sales);       // Reorders movies list from highest to lowest revenue
void sortMoviesByTicketsSold(vector<Movie>& movies, const vector<SaleRecord>& sales);   // Reorders movies list from most to least tickets sold

// ----- Shared computation helpers reused by dashboard.cpp -----
// These are declared here (not static) specifically so dashboard.cpp can call them too, avoiding duplicate logic.
double totalRevenueAll(const vector<SaleRecord>& sales);     // Sums subtotal across ALL sales (every movie combined)
int totalTicketsAll(const vector<SaleRecord>& sales);         // Sums quantity across ALL sales (every movie combined)
string classifyPerformance(double revenue, double avgRevenue, bool hasSales);  // Returns a label like "Hit"/"Flop" based on revenue vs average
pair<string, double> mostProfitableGenre(const vector<Movie>& movies, const vector<SaleRecord>& sales);  // Returns {genre, avgRevenue} for the top genre by avg revenue
pair<string, int> mostPopularGenre(const vector<SaleRecord>& sales);   // Returns {genre, ticketCount} for the genre with the most tickets sold
