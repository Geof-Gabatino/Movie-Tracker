#pragma once               // Prevents this header from being included more than once in the same file (avoids duplicate definition errors)
#include <vector>           // Needed because we use vector<Movie> and vector<ConcessionItem>
#include <string>            // Needed because Movie/ConcessionItem use the string type
using namespace std;         // Lets us write "string" and "vector" instead of "std::string" and "std::vector"

// enum class = a strongly-typed set of named integer constants.
// "class" makes it so we must write MovieStatus::Upcoming instead of just "Upcoming" (avoids name clashes).
// We explicitly assign 0, 1, 2 so the numbers match what gets saved/loaded from the text file.
enum class MovieStatus { Upcoming = 0, NowShowing = 1, Archived = 2 };

// struct = a simple data container (like a class, but members are public by default).
// This holds all the information needed to describe one movie.
struct Movie {
    int id;                  // Unique identifier for the movie (auto-generated, used to find/match a movie)
    string title;
    string genre;
    string duration;         // Stored as text (e.g. "1h 30m") instead of a number for simplicity
    string rating;           // Parental guidance rating (e.g., G, PG, R-13, R-16, R-18)
    double price;            // Ticket price; double is used because money can have decimal centavos
    string releaseDate;      // Date the movie starts showing, stored as text in YYYY-MM-DD format
    string pullOutDate;      // Date the movie stops showing, also YYYY-MM-DD format
    int seatsAvailable;      // How many seats are left to sell
    int seatCapacity;        // The total/maximum number of seats for this movie
    MovieStatus status;      // Current lifecycle state (Upcoming / NowShowing / Archived)
    string showtime;
};

// struct para sa concession items (separate from Movie, used for snacks/drinks sold at the cinema)
struct ConcessionItem {
    int id;
    string name;       // e.g., "Popcorn Large"
    string category;   // e.g., "Food", "Beverage"
    double price;
    int stockQuantity;
};

// ----- Status helpers -----
// These convert between the enum (used in code) and either text (for display) or an int (for saving to file).
string statusToDisplay(MovieStatus s);     // Converts enum value to a human-readable string like "Now Showing"
int statusToFileCode(MovieStatus s);       // Converts enum value to an int (0/1/2) for writing to the save file
MovieStatus fileCodeToStatus(int code);    // Converts an int (0/1/2) read from the save file back into the enum

// ----- Date helpers -----
// All dates in this program are plain strings in "YYYY-MM-DD" format; these functions do the date math/validation.
string getCurrentDate();                  // Returns today's date as a string "YYYY-MM-DD"
string getCurrentDateTime();              // Returns today's date AND time as a string "YYYY-MM-DD HH:MM:SS"
bool isValidDate(const string& date);     // Checks that a date string is properly formatted and a real calendar date
long dateToEpochDays(const string& date); // Converts a date string into a single number (days since epoch) so dates can be subtracted/compared
int daysBetween(const string& from, const string& to); // Returns how many days are between two date strings (to - from)

// ----- Generic console input helpers -----
// These wrap cin/cout so every part of the program asks for input in a consistent, validated way.
void pauseScreen();          // Waits for the user to press Enter before continuing (so they can read output)
void clearScreen();          // Clears the console screen (uses a different system command depending on OS)
int askInt(const string& prompt, int minVal = -2147483647);     // Repeatedly asks until the user enters a valid integer >= minVal
double askDouble(const string& prompt, double minVal = 0.0);    // Repeatedly asks until the user enters a valid double >= minVal
string askLine(const string& prompt, bool allowEmpty = false);  // Asks for a line of text; can require it to be non-empty
string askDate(const string& prompt);     // Repeatedly asks until the user enters a valid YYYY-MM-DD date
string askRating();                       // Shows a menu of ratings (G, PG, R-13, R-16, R-18) and returns the chosen one  // DITO YUNG DAGDAG
string toLowerStr(string s);               // Returns a lowercase copy of the given string (useful for case-insensitive search)

// ----- Movie Management -----
// Functions that let the user create/modify the list of movies. They take the vector BY REFERENCE (&)
// so that changes made inside the function actually affect the original list in main().
void addMovie(vector<Movie>& movies, int& nextId);     // Creates a new Movie and appends it to the vector; nextId is incremented
void updateMovie(vector<Movie>& movies);                // Finds a movie by ID and lets the user edit its fields
void searchMovie(const vector<Movie>& movies);          // const& because this function only reads the list, never modifies it
void archiveMovie(vector<Movie>& movies);               // Manually sets a movie's status to Archived
void deleteMovie(vector<Movie>& movies);                // Permanently removes a movie from the vector

// ----- Movie Viewing -----
void viewAllMovies(const vector<Movie>& movies);                         // Prints every movie in the list
void viewByStatus(const vector<Movie>& movies, MovieStatus status);      // Prints only movies matching the given status

// ----- Movie Lifecycle Management -----
void updateMovieStatuses(vector<Movie>& movies);   // Recalculates every movie's status based on today's date vs release/pull-out dates

// ----- Lookup -----
// Two versions are provided: one returns a non-const pointer (so the caller can modify the found movie),
// and one returns a const pointer (so the caller can only read it). This is a common C++ pattern.
Movie* findMovieById(vector<Movie>& movies, int id);
const Movie* findMovieByIdConst(const vector<Movie>& movies, int id);

// ----- Sorting -----
void sortMoviesByTitle(vector<Movie>& movies);             // Rearranges the vector alphabetically by title
void totalMoviesCounter(const vector<Movie>& movies);      // Counts and displays how many movies are in each status

// ----- Display helpers -----
void printMovieHeader();              // Prints the column headers for the movie table (ID, Title, Genre, etc.)
void printMovieRow(const Movie& m);   // Prints one movie's data formatted to line up under the header
