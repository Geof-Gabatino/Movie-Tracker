#pragma once
#include <vector>
#include "movie.h"
#include "sales.h"
using namespace std;

// Displays a one-screen snapshot of the cinema's current state (counts, revenue, top movie, alerts, etc.)
void cinemaDashboard(const vector<Movie>& movies, const vector<SaleRecord>& sales);

// Generates simple rule-based business suggestions (e.g. book more of a profitable genre, archive a flop)
void cinemaRecommendationSystem(const vector<Movie>& movies, const vector<SaleRecord>& sales);
