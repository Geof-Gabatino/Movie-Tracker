#include "movie.h"
#include "filemanager.h"     // Needed because addMovie/updateMovie call saveMovies() to persist changes to disk
#include <iostream>
#include <sstream>           // For stringstream, used to parse input and build formatted strings
#include <iomanip>           // For setw(), setfill(), setprecision() - formatting output columns/numbers
#include <limits>            // For numeric_limits, used inside pauseScreen()
#include <algorithm>         // For sort() and transform()
#include <ctime>             // For time(), localtime(), mktime() - all the date/time functions
using namespace std;

// Converts the MovieStatus enum into a readable string for display to the user.
string statusToDisplay(MovieStatus s) {
    switch (s) {                                   // switch picks the matching case based on the enum value
        case MovieStatus::Upcoming:   return "Upcoming";
        case MovieStatus::NowShowing: return "Now Showing";
        case MovieStatus::Archived:   return "Archived";
    }
    return "Unknown";   // Fallback in case of an unexpected/invalid enum value (defensive coding)
}

// static_cast converts the enum class value into its underlying int (0, 1, or 2) for saving to file.
int statusToFileCode(MovieStatus s) { return static_cast<int>(s); }

// Converts an int loaded from the save file back into the MovieStatus enum.
MovieStatus fileCodeToStatus(int code) {
    if (code == 1) return MovieStatus::NowShowing;
    if (code == 2) return MovieStatus::Archived;
    return MovieStatus::Upcoming;   // Default/fallback: any other code (including 0) becomes Upcoming
}

// Builds today's date as a string in "YYYY-MM-DD" format using the system clock.
string getCurrentDate() {
    time_t now = time(0);          // Gets the current time as a raw time_t value (seconds since epoch)
    tm* lt = localtime(&now);      // Converts that raw time into a tm struct with year/month/day/etc. broken out
    ostringstream ss;              // ostringstream lets us build a string using << like we would with cout
    // tm_year is years since 1900, so we add 1900 back; tm_mon is 0-based (Jan=0), so we add 1
    // setw(2) + setfill('0') makes single-digit months/days print as "01" instead of "1"
    ss << (1900 + lt->tm_year) << '-' << setw(2) << setfill('0') << (1 + lt->tm_mon) << '-' << setw(2) << setfill('0') << lt->tm_mday;
    return ss.str();   // Extracts the built string out of the stringstream
}

// Same idea as getCurrentDate(), but also appends hours:minutes:seconds (used for sales timestamps).
string getCurrentDateTime() {
    time_t now = time(0);
    tm* lt = localtime(&now);
    ostringstream ss;
    ss << (1900 + lt->tm_year) << '-' << setw(2) << setfill('0') << (1 + lt->tm_mon) << '-' << setw(2) << setfill('0') << lt->tm_mday
       << ' ' << setw(2) << setfill('0') << lt->tm_hour << ':' << setw(2) << setfill('0') << lt->tm_min << ':' << setw(2) << setfill('0') << lt->tm_sec;
    return ss.str();
}

// Checks whether a string is a properly formatted, real calendar date "YYYY-MM-DD".
bool isValidDate(const string& date) {
    if (date.length() != 10) return false;              // Must be exactly 10 characters long (YYYY-MM-DD)
    if (date[4] != '-' || date[7] != '-') return false;  // Dashes must be in the correct positions
    string ys = date.substr(0, 4), ms = date.substr(5, 2), ds = date.substr(8, 2);  // Split into year/month/day substrings
    for (char c : ys) if (!isdigit(c)) return false;     // Every character of the year part must be a digit
    for (char c : ms) if (!isdigit(c)) return false;     // Every character of the month part must be a digit
    for (char c : ds) if (!isdigit(c)) return false;     // Every character of the day part must be a digit
    int y = stoi(ys), m = stoi(ms), d = stoi(ds);        // stoi = "string to int", converts the text into numbers
    if (y < 1900 || y > 2100 || m < 1 || m > 12 || d < 1 || d > 31) return false;   // Basic range checks
    if (m == 4 || m == 6 || m == 9 || m == 11) if (d > 30) return false;           // April/June/Sept/Nov only have 30 days
    if (m == 2) {                                         // Special handling for February because of leap years
        bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);   // Standard leap-year rule
        if (leap && d > 29) return false;                 // Leap year Feb can have at most 29 days
        if (!leap && d > 28) return false;                // Non-leap year Feb can have at most 28 days
    }
    return true;   // Passed every check, so it's a valid date
}

// Converts a date string into the number of whole days since the Unix epoch (Jan 1, 1970).
// This lets us subtract two dates to find out how many days apart they are.
long dateToEpochDays(const string& date) {
    if (!isValidDate(date)) return 0;    // Guard: if the date is malformed, just return 0 instead of crashing
    int y = stoi(date.substr(0, 4)), m = stoi(date.substr(5, 2)), d = stoi(date.substr(8, 2));
    tm t = {};                 // {} zero-initializes all fields of the tm struct (avoids garbage values)
    t.tm_year = y - 1900;      // mktime expects years since 1900
    t.tm_mon = m - 1;          // mktime expects 0-based months
    t.tm_mday = d;
    time_t e = mktime(&t);     // Converts the tm struct into seconds-since-epoch
    return (e < 0) ? 0 : e / (24 * 3600);   // Convert seconds to whole days; ternary guards against negative/invalid results
}

// Returns how many days are between two dates (positive if "to" is after "from").
int daysBetween(const string& from, const string& to) {
    return dateToEpochDays(to) - dateToEpochDays(from);
}

// Pauses program execution until the user presses Enter, so they have time to read the screen.
void pauseScreen() {
    cout << "\nPress Enter to continue...";
    // cin.ignore discards leftover characters in the input buffer up to the next newline,
    // so a stray Enter key press elsewhere in the program doesn't skip this pause.
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// Clears the terminal screen. Uses a preprocessor directive (#ifdef) to pick the right OS command at compile time.
void clearScreen() {
#ifdef _WIN32
    system("cls");     // Windows clear-screen command
#else
    system("clear");   // Linux/Mac clear-screen command
#endif
}

// Repeatedly prompts the user until they type a valid integer that is >= minVal.
int askInt(const string& prompt, int minVal) {
    int val; string input;
    while (true) {                          // Infinite loop; we only "return" out of it once input is valid
        cout << prompt;
        if (!getline(cin, input)) continue; // If reading fails (e.g. EOF), just try again
        stringstream ss(input);             // Wrap the typed text in a stringstream so we can attempt to parse a number from it
        // ss >> val tries to extract an int; ss.eof() makes sure there's nothing leftover after the number
        // (this rejects things like "5abc" which would otherwise partially parse as 5)
        if (ss >> val && ss.eof() && val >= minVal) return val;
        cout << "Invalid input. Please enter an integer >= " << minVal << ".\n";
    }
}

// Same pattern as askInt(), but for decimal numbers (doubles), used for prices etc.
double askDouble(const string& prompt, double minVal) {
    double val; string input;
    while (true) {
        cout << prompt;
        if (!getline(cin, input)) continue;
        stringstream ss(input);
        if (ss >> val && ss.eof() && val >= minVal) return val;
        cout << "Invalid input. Please enter a valid price/number.\n";
    }
}

// Asks for a line of free-form text. If allowEmpty is false, it keeps asking until something is typed.
string askLine(const string& prompt, bool allowEmpty) {
    string line;
    while (true) {
        cout << prompt;
        getline(cin, line);                                  // Reads an entire line, including spaces
        if (!allowEmpty && line.empty()) { cout << "Input cannot be empty.\n"; continue; }  // Reject blank input unless allowed
        return line;
    }
}

// Keeps asking for a date string until isValidDate() confirms it's a real, correctly formatted date.
string askDate(const string& prompt) {
    string d;
    while (true) {
        d = askLine(prompt);
        if (isValidDate(d)) return d;
        cout << "Invalid format. Use YYYY-MM-DD.\n";
    }
}

// Displays a small menu of allowed ratings and returns the user's choice as text.
string askRating() {
    int choice;
    while (true) {
        cout << "\n--- SELECT RATING ---\n"
             << "[1] G\n"
             << "[2] PG\n"
             << "[3] R-13\n"
             << "[4] R-16\n"
             << "[5] R-18\n";
        choice = askInt("Enter choice (1-5): ", 1);   // minVal of 1 means 0 or negative numbers are rejected immediately

        switch (choice) {                              // Maps the numeric menu choice to the actual rating text
            case 1: return "G";
            case 2: return "PG";
            case 3: return "R-13";
            case 4: return "R-16";
            case 5: return "R-18";
            default: cout << "Invalid choice. Please enter 1-5.\n";  // Anything above 5 falls here and loops again
        }
    }
}

// Returns a lowercase copy of the input string (the original parameter "s" is a local copy, since it's passed by value).
string toLowerStr(string s) {
    // transform applies ::tolower to every character from s.begin() to s.end(), writing the result back into s.begin()
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// Prompts the user for every field of a new movie, computes its initial status, and adds it to the list.
void addMovie(vector<Movie>& movies, int& nextId) {
    cout << "\n--- ADD MOVIE ---\n";
    Movie m; m.id = nextId;     // New movie gets the next available unique ID
    m.title = askLine("Enter Title: ");
    m.genre = askLine("Enter Genre: ");
    m.duration = askLine("Enter Duration (e.g., 1h 30m): ");
    m.rating = askRating();
    m.price = askDouble("Enter Ticket Price: ", 0.0);
    m.seatCapacity = askInt("Enter Seat Capacity: ", 1);
    m.seatsAvailable = m.seatCapacity;     // Brand-new movie starts with all seats available
    m.releaseDate = askDate("Enter Release Date (YYYY-MM-DD): ");
    m.pullOutDate = askDate("Enter Pull-out Date (YYYY-MM-DD): ");
    m.showtime = askLine("Enter Show Time (e.g., 1:00 PM / 4:30 PM): ");

    // Decide the movie's status by comparing today's date against the release/pull-out dates.
    string today = getCurrentDate();
    if (daysBetween(today, m.releaseDate) > 0) m.status = MovieStatus::Upcoming;       // Release date is still in the future
    else if (daysBetween(today, m.pullOutDate) >= 0) m.status = MovieStatus::NowShowing; // Already released, not yet pulled out
    else m.status = MovieStatus::Archived;                                              // Pull-out date already passed

    movies.push_back(m);   // Adds the new movie to the end of the vector
    nextId++;               // Increment the counter so the next movie gets a different ID
    saveMovies(movies);     // Persist the updated list to movies.txt immediately
    cout << "\nMovie \"" << m.title << "\" added successfully with ID: " << m.id << "!\n";
    pauseScreen();
}

// Lets the user edit an existing movie's fields. Leaving input blank or typing "0" keeps the old value.
void updateMovie(vector<Movie>& movies) {
    cout << "\n--- UPDATE MOVIE ---\n";
    int id = askInt("Enter Movie ID to update: ", -2147483647);
    Movie* m = findMovieById(movies, id);    // Pointer so we can modify the actual movie in the vector (not a copy)
    if (!m) { cout << "Movie not found.\n"; pauseScreen(); return; }   // Early return if no matching movie was found

    cout << "\nUpdating Movie ID " << m->id << " (" << m->title << ")\n";
    cout << "*** TIP: Press ENTER (leave blank) or type '0' to keep the existing value ***\n\n";

    string input;

    // Title - only overwrite if the user actually typed something other than blank/"0"
    input = askLine("Enter New Title [" + m->title + "]: ", true);   // true = allowEmpty, since blank means "keep current"
    if (!input.empty() && input != "0") m->title = input;

    // Genre
    input = askLine("Enter New Genre [" + m->genre + "]: ", true);
    if (!input.empty() && input != "0") m->genre = input;

    // Duration
    input = askLine("Enter New Duration [" + m->duration + "]: ", true);
    if (!input.empty() && input != "0") m->duration = input;

    // Rating - shown as its own little menu, with 0 meaning "keep current rating"
    cout << "\nCurrent Rating: " << m->rating << "\n";
    cout << "[0] Keep Current\n[1] G\n[2] PG\n[3] R-13\n[4] R-16\n[5] R-18\n";
    int rChoice;
    while(true) {                              // Loop until a valid 0-5 choice is entered
        rChoice = askInt("Enter new rating (0-5): ", 0);
        if(rChoice >= 0 && rChoice <= 5) break;
        cout << "Invalid choice.\n";
    }
    switch(rChoice) {
        case 1: m->rating = "G"; break;
        case 2: m->rating = "PG"; break;
        case 3: m->rating = "R-13"; break;
        case 4: m->rating = "R-16"; break;
        case 5: m->rating = "R-18"; break;
        case 0: break;    // 0 means do nothing, keeping the existing rating
    }

    // Price - using 0.0 as the "keep current" sentinel value since a real price should never be exactly 0
    double newPrice = askDouble("Enter New Price (Current: P" + to_string(m->price) + ") [0 to keep]: ", 0.0);
    if (newPrice != 0.0) m->price = newPrice;

    // Capacity - if changed, adjust seatsAvailable by the same difference so existing sold seats stay consistent
    int newCap = askInt("Enter New Capacity (Current: " + to_string(m->seatCapacity) + ") [0 to keep]: ", 0);
    if (newCap != 0) {
        int diff = newCap - m->seatCapacity;     // How much bigger/smaller the new capacity is
        m->seatCapacity = newCap;
        m->seatsAvailable += diff;               // Apply the same change to available seats
        if (m->seatsAvailable < 0) m->seatsAvailable = 0;   // Safety clamp: never let available seats go negative
    }

    // Release Date - loops until either left blank/"0" (keep current) or a valid date is entered
    while (true) {
        input = askLine("Enter New Release Date [" + m->releaseDate + "] (YYYY-MM-DD): ", true);
        if (input.empty() || input == "0") break;          // User chose to keep the current date
        if (isValidDate(input)) { m->releaseDate = input; break; }  // Valid new date accepted
        cout << "Invalid format. Use YYYY-MM-DD or leave blank.\n"; // Otherwise loop again
    }

    // Pull-out Date - same pattern as Release Date above
    while (true) {
        input = askLine("Enter New Pull-out Date [" + m->pullOutDate + "] (YYYY-MM-DD): ", true);
        if (input.empty() || input == "0") break;
        if (isValidDate(input)) { m->pullOutDate = input; break; }
        cout << "Invalid format. Use YYYY-MM-DD or leave blank.\n";
    }

    // Show Time
    input = askLine("Enter New Show Time [" + m->showtime + "]: ", true);
    if (!input.empty() && input != "0") m->showtime = input;

    // Recalculate status using the same release/pull-out date logic as addMovie()
    string today = getCurrentDate();
    if (daysBetween(today, m->releaseDate) > 0) m->status = MovieStatus::Upcoming;
    else if (daysBetween(today, m->pullOutDate) >= 0) m->status = MovieStatus::NowShowing;
    else m->status = MovieStatus::Archived;

    saveMovies(movies);     // Save the updated list back to disk
    cout << "\nMovie records updated successfully!\n";
    pauseScreen();
}

// Searches movies whose title OR genre contains the user's keyword (case-insensitive).
void searchMovie(const vector<Movie>& movies) {
    cout << "\n--- SEARCH MOVIE ---\n";
    string query = toLowerStr(askLine("Enter search keyword (Title/Genre): "));   // Lowercase the query for case-insensitive matching
    bool found = false;
    printMovieHeader();
    for (const auto& m : movies) {       // Range-based for loop: iterates over every Movie in the vector by const reference (no copying, read-only)
        // string::find returns string::npos if the substring is NOT found, so != npos means "it was found"
        if (toLowerStr(m.title).find(query) != string::npos || toLowerStr(m.genre).find(query) != string::npos) {
            printMovieRow(m); found = true;
        }
    }
    if (!found) cout << "No matching movies found.\n";
    pauseScreen();
}

// Manually forces a movie's status to Archived, regardless of its dates.
void archiveMovie(vector<Movie>& movies) {
    cout << "\n--- ARCHIVE MOVIE MANUALLY ---\n";
    int id = askInt("Enter Movie ID to archive: ");
    Movie* m = findMovieById(movies, id);
    if (!m) { cout << "Movie not found.\n"; pauseScreen(); return; }
    m->status = MovieStatus::Archived;
    saveMovies(movies);
    cout << "\nMovie \"" << m->title << "\" manual archive successful!\n";
    pauseScreen();
}

// Permanently removes a movie from the vector after asking the user to confirm with "YES".
void deleteMovie(vector<Movie>& movies) {
    cout << "\n--- DELETE MOVIE PERMANENTLY ---\n";
    int id = askInt("Enter Movie ID to delete: ");
    // We use an iterator-based loop here (instead of range-based for) because erase() needs an iterator, not a value.
    for (auto it = movies.begin(); it != movies.end(); ++it) {
        if (it->id == id) {                   // it->id is shorthand for (*it).id, i.e. accessing the Movie the iterator points to
            cout << "Are you sure you want to permanently delete \"" << it->title << "\"?\n";
            string confirm = toLowerStr(askLine("Type 'YES' to confirm: "));
            if (confirm == "yes") {
                movies.erase(it); saveMovies(movies);   // erase() removes the element at this iterator position from the vector
                cout << "\nMovie deleted completely.\n";
            } else {
                cout << "\nDeletion canceled.\n";
            }
            pauseScreen(); return;   // Return immediately after handling the match so we don't keep looping
        }
    }
    cout << "Movie not found.\n";   // Reached only if the loop finished without finding a matching ID
    pauseScreen();
}

// Prints every single movie currently in the vector.
void viewAllMovies(const vector<Movie>& movies) {
    cout << "\n------------- ALL REGISTERED MOVIES ------------- \n";
    if (movies.empty()) { cout << "No movies registered.\n"; pauseScreen(); return; }   // Guard against an empty list
    printMovieHeader();
    for (const auto& m : movies) printMovieRow(m);   // Loop through and print each movie's row
    pauseScreen();
}

// Prints only the movies whose status matches the one passed in (Upcoming / NowShowing / Archived).
void viewByStatus(const vector<Movie>& movies, MovieStatus status) {
    cout << "\n--- VIEW BY STATUS: " << statusToDisplay(status) << " ---\n";
    bool any = false;
    printMovieHeader();
    for (const auto& m : movies) {
        if (m.status == status) { printMovieRow(m); any = true; }   // Only print rows matching the requested status
    }
    if (!any) cout << "No movies found matching this status.\n";
    pauseScreen();
}

// Rearranges the movies vector in-place, alphabetically by title (case-insensitive).
void sortMoviesByTitle(vector<Movie>& movies) {
    // sort() takes a begin/end range plus a comparator lambda that returns true if "a" should come before "b".
    // [](...) {...} is a lambda: an unnamed inline function used right where it's needed.
    sort(movies.begin(), movies.end(), [](const Movie& a, const Movie& b) {
        return toLowerStr(a.title) < toLowerStr(b.title);   // Compare lowercase titles so sorting ignores letter case
    });
    cout << "\n--- MOVIES SORTED BY TITLE ---\n";
    printMovieHeader();
    for (const auto& m : movies) printMovieRow(m);
    pauseScreen();
}

// Counts how many movies fall into each status and displays totals.
void totalMoviesCounter(const vector<Movie>& movies) {
    int upcoming = 0, nowShowing = 0, archived = 0;
    for (const auto& m : movies) {                       // Single pass through the vector, tallying as we go
        if (m.status == MovieStatus::Upcoming) upcoming++;
        else if (m.status == MovieStatus::NowShowing) nowShowing++;
        else archived++;                                  // Anything that isn't Upcoming/NowShowing must be Archived
    }
    cout << "\n--- TOTAL MOVIES COUNTER ---\n";
    cout << "Total Movies:     " << movies.size() << '\n'
         << "  Now Showing:    " << nowShowing << '\n'
         << "  Upcoming:       " << upcoming << '\n'
         << "  Archived:       " << archived << '\n';
    pauseScreen();
}

// Goes through every movie and recalculates its status based on today's date vs release/pull-out dates.
// This is called repeatedly (e.g. every time a menu is shown) so statuses stay accurate over time.
void updateMovieStatuses(vector<Movie>& movies) {
    string today = getCurrentDate();
    for (auto& m : movies) {                                   // auto& means we get a reference to the actual Movie, so we CAN modify it
        if (daysBetween(today, m.releaseDate) > 0) m.status = MovieStatus::Upcoming;
        else if (daysBetween(today, m.pullOutDate) >= 0) m.status = MovieStatus::NowShowing;
        else m.status = MovieStatus::Archived;
    }
}

// Linear search through the vector for a movie matching the given id; returns a modifiable pointer, or nullptr if not found.
Movie* findMovieById(vector<Movie>& movies, int id) {
    for (auto& m : movies) if (m.id == id) return &m;   // &m takes the address of the matching element so the caller can edit it
    return nullptr;   // nullptr = "no such movie exists"
}

// Same search as above, but returns a read-only (const) pointer - used when the caller should not modify the result.
const Movie* findMovieByIdConst(const vector<Movie>& movies, int id) {
    for (const auto& m : movies) if (m.id == id) return &m;
    return nullptr;
}

// Prints the column titles for the movie table, with dashed separator lines above and below.
void printMovieHeader() {
    cout << "--------------------------------------------------------------------------------------------------------------------------------------------\n";
    // left = left-align text within its column; setw(n) reserves n characters of width for the next thing printed
    cout << left << setw(4)  << "ID" 
         << setw(26) << "Title" 
         << setw(18) << "Genre" 
         << setw(12) << "Duration" 
         << setw(8)  << "Rating" 
         << setw(11) << "Price" 
         << setw(11) << "Seats"
         << setw(12) << "Release"
         << setw(12) << "Pull-out"
         << setw(14) << "Show Time" 
         << "Status\n";
    cout << "--------------------------------------------------------------------------------------------------------------------------------------------\n";
}

// Prints a single movie's data as one row, formatted to align under printMovieHeader()'s columns.
void printMovieRow(const Movie& m) {
    ostringstream priceStream;                                  // Build the price text separately so we can format it with a "P" prefix first
    priceStream << "P" << fixed << setprecision(2) << m.price;  // fixed + setprecision(2) forces exactly 2 decimal places (e.g. P150.00)
    
    // Truncated strings for display: long text is cut short and "..." appended so columns don't overflow/misalign.
    string t = m.title.length() > 25 ? m.title.substr(0, 22) + "..." : m.title;       // Ternary: condition ? valueIfTrue : valueIfFalse
    string g = m.genre.length() > 17 ? m.genre.substr(0, 14) + "..." : m.genre;
    string d = m.duration.length() > 11 ? m.duration.substr(0, 8) + "..." : m.duration;
    string st = m.showtime.length() > 13 ? m.showtime.substr(0, 10) + "..." : m.showtime;
    string seats = to_string(m.seatsAvailable) + "/" + to_string(m.seatCapacity);     // Display as "available/capacity", e.g. "45/50"

    cout << left << setw(4)  << m.id 
         << setw(26) << t
         << setw(18) << g 
         << setw(12) << d 
         << setw(8)  << m.rating 
         << setw(11) << priceStream.str()
         << setw(11) << seats
         << setw(12) << m.releaseDate
         << setw(12) << m.pullOutDate
         << setw(14) << st 
         << statusToDisplay(m.status) << '\n';
}
