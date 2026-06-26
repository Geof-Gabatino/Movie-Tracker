// filemanager.h
#ifndef FILEMANAGER_H        // Classic "include guard" pattern (alternative to #pragma once): prevents double-inclusion
#define FILEMANAGER_H

#include <vector>
#include "movie.h"            // Needed for the Movie and ConcessionItem struct definitions
#include "sales.h"            // Needed for the SaleRecord struct definition
using namespace std;

// Declarations only. NO "inline".
// These functions handle reading/writing each data type to/from its own plain-text save file.
void saveMovies(const vector<Movie>& movies);            // Writes the full movie list to movies.txt
void loadMovies(vector<Movie>& movies, int& nextId);      // Reads movies.txt back into the vector and figures out the next free ID
void saveSales(const vector<SaleRecord>& sales);          // Writes the full sales history to sales.txt
void loadSales(vector<SaleRecord>& sales, int& nextSaleId); // Reads sales.txt back into the vector and figures out the next free sale ID
void saveConcessions(const vector<ConcessionItem>& snacks);            // Writes the concessions inventory to concessions.txt
void loadConcessions(vector<ConcessionItem>& snacks, int& nextSnackId); // Reads concessions.txt back into the vector and figures out the next free item ID

#endif    // End of the include guard started above
