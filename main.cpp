#include "student.hpp"
#include "database.hpp"

#include <iostream>
#include <limits>
#include <fstream>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <sstream>

using IIITStudent   = Student<std::string, std::string>;
using IIITDatabase  = StudentDatabase<std::string, std::string>;
using IITStudent    = Student<unsigned int, int>;

const std::string CSV_FILE = "oopd_students.csv";

// ---------------- VALIDATION ----------------
void validateStudentName(const std::string &name) {
    if (name.empty())
        throw std::invalid_argument("Name cannot be empty.");

    for (char c : name) {
        if (!std::isalpha(static_cast<unsigned char>(c)) &&
            !std::isspace(static_cast<unsigned char>(c)))
            throw std::invalid_argument("Name must contain only alphabets and spaces.");
    }
}

bool isAlphabetic(const std::string &str) {
    if (str.empty()) return false;
    for (char c : str)
        if (!std::isalpha(static_cast<unsigned char>(c))) return false;
    return true;
}

bool isNumeric(const std::string &str) {
    if (str.empty()) return false;
    for (char c : str)
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    return true;
}

std::string toUpper(const std::string &s) {
    std::string res = s;
    for (char &ch : res) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return res;
}

// -------- OOPD CHECK (for filtering only) --------
bool studentHasCourseOOPD(const IIITStudent &s) {
    const std::string oopd = "OOPD";

    // Check current courses (case-insensitive)
    for (const auto &c : s.getCurrentCourses()) {
        if (toUpper(c) == oopd) return true;
    }

    // Check completed courses (keys), case-insensitive
    const auto &completed = s.getCompletedCourses();
    for (const auto &p : completed) {
        if (toUpper(p.first) == oopd) return true;
    }

    return false;
}

// -------------- CSV APPEND (with courses + grades) ----------------
void appendStudentsToCSV(const std::string &filename,
                         const std::vector<IIITStudent> &students)
{
    bool exists = false, empty = true;

    {
        std::ifstream chk(filename);
        if (chk.good()) {
            exists = true;
            chk.seekg(0, std::ios::end);
            empty = (chk.tellg() == 0);
        }
    }

    std::ofstream out(filename, std::ios::app);
    if (!out) {
        std::cerr << "Error opening CSV.\n";
        return;
    }

    if (!exists || empty)
        out << "name,roll,branch,startYear,currentCourses,completedCourses\n";

    for (const auto &s : students) {
        std::string currentStr, compStr;

        for (const auto &c : s.getCurrentCourses()) {
            if (!currentStr.empty()) currentStr += ";";
            currentStr += c;
        }

        for (const auto &p : s.getCompletedCourses()) {
            if (!compStr.empty()) compStr += ";";
            compStr += p.first + ":" + std::to_string(p.second);
        }

        out << s.getName()           << ","
            << s.getRoll()           << ","
            << s.getBranch()         << ","
            << s.getStartYear()      << ","
            << currentStr            << ","
            << compStr               << "\n";
    }

    std::cout << "\nSaved " << students.size() << " students to CSV.\n";
}

// -------------- CLEAR CSV ----------------
void clearCSV(const std::string &filename, IIITDatabase &db) {
    char c;
    std::cout << "Are you sure you want to clear CSV? (y/n): ";
    std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (c == 'y' || c == 'Y') {
        std::ofstream out(filename, std::ios::trunc);
        db = IIITDatabase();
        std::cout << "\nCSV cleared and memory reset.\n";
    } else {
        std::cout << "Cancelled.\n";
    }
}

// -------------- MANUAL ENTRY ----------------
void addStudentsManually(IIITDatabase &db) {
    int n;
    std::cout << "\nHow many students? ";
    while (!(std::cin >> n) || n <= 0) {
        std::cin.clear(); std::cin.ignore(10000, '\n');
        std::cout << "Enter valid number: ";
    }
    std::cin.ignore(10000, '\n');

    std::vector<IIITStudent> newStudents;

    for (int i = 0; i < n; i++) {
        std::cout << "\n---- Student " << (i+1) << " ----\n";

        std::string name, roll, branch;
        int startYear;

        while (true) {
            std::cout << "Enter Name: ";
            std::getline(std::cin, name);
            try { validateStudentName(name); break; }
            catch (const std::invalid_argument &e) { std::cout << e.what() << "\n"; }
        }

        std::cout << "Enter Roll: ";
        std::getline(std::cin, roll);

        std::cout << "Enter Branch: ";
        std::getline(std::cin, branch);

        std::cout << "Enter Start Year: ";
        while (!(std::cin >> startYear)) {
            std::cin.clear(); std::cin.ignore(10000, '\n');
            std::cout << "Enter valid integer: ";
        }
        std::cin.ignore(10000, '\n');

        IIITStudent stud(name, roll, branch, startYear);

        // ---------- CURRENT COURSES ----------
        int cur;
        std::cout << "Number of CURRENT courses: ";
        while (!(std::cin >> cur) || cur < 0) {
            std::cin.clear(); std::cin.ignore(10000, '\n');
            std::cout << "Enter valid integer: ";
        }
        std::cin.ignore(10000, '\n');

        for (int j = 0; j < cur; j++) {
            int type;
            std::string course;

            while (true) {
                std::cout << "\nCurrent Course " << (j+1)
                          << " | Type (1=IIITD , 2=IITD): ";

                if (!(std::cin >> type)) {
                    std::cin.clear(); std::cin.ignore(10000, '\n');
                    std::cout << "Enter valid integer.\n"; continue;
                }
                if (type != 1 && type != 2) {
                    std::cin.ignore(10000, '\n');
                    std::cout << "Enter 1 or 2.\n"; continue;
                }
                std::cin.ignore(10000, '\n');

                std::cout << "Course Code: ";
                std::getline(std::cin, course);

                if (type == 1 && !isAlphabetic(course)) {
                    std::cout << "IIITD course must be alphabets only.\n"; continue;
                }
                if (type == 2 && !isNumeric(course)) {
                    std::cout << "IITD course must be integers only.\n"; continue;
                }
                break;
            }
            stud.enrollInCourse(course);
        }

        // ---------- COMPLETED COURSES ----------
        int comp;
        std::cout << "Number of COMPLETED courses: ";
        while (!(std::cin >> comp) || comp < 0) {
            std::cin.clear(); std::cin.ignore(10000, '\n');
            std::cout << "Enter valid integer: ";
        }
        std::cin.ignore(10000, '\n');

        for (int j = 0; j < comp; j++) {
            int type; std::string course; double grade;

            while (true) {
                std::cout << "\nCompleted Course " << (j+1)
                          << " | Type (1=IIITD , 2=IITD): ";

                if (!(std::cin >> type)) {
                    std::cin.clear(); std::cin.ignore(10000, '\n');
                    std::cout << "Enter valid integer.\n"; continue;
                }
                if (type != 1 && type != 2) {
                    std::cout << "Enter 1 or 2.\n"; continue;
                }
                std::cin.ignore(10000, '\n');

                std::cout << "Course Code: ";
                std::getline(std::cin, course);

                if (type == 1 && !isAlphabetic(course)) {
                    std::cout << "IIITD must be alphabets.\n"; continue;
                }
                if (type == 2 && !isNumeric(course)) {
                    std::cout << "IITD must be integers.\n"; continue;
                }
                break;
            }

            std::cout << "Grade: ";
            while (!(std::cin >> grade)) {
                std::cin.clear(); std::cin.ignore(10000, '\n');
                std::cout << "Enter numeric grade: ";
            }
            std::cin.ignore(10000, '\n');

            stud.completeCourse(course, grade);
        }

        // ✅ No automatic OOPD addition here
        db.addStudent(stud);
        newStudents.push_back(stud);
    }

    appendStudentsToCSV(CSV_FILE, newStudents);
}

// -------------- OOPD DISPLAY (FILTER ONLY) ----------------
void showOOPDStudents(const IIITDatabase &db) {
    std::cout << "\n===== OOPD STUDENTS (IIIT-Delhi) =====\n";
    const auto &all = db.getStudents();
    bool found = false;

    for (const auto &s : all) {
        if (studentHasCourseOOPD(s)) {
            found = true;
            std::cout << s << "\n";
        }
    }

    if (!found) std::cout << "No OOPD students found.\n";
}

// ---------------- MENU ----------------
void showMenu() {
    std::cout << "\n========== MENU ==========\n";
    std::cout << "1. Load CSV\n";
    std::cout << "2. Add students manually\n";
    std::cout << "3. Show original records\n";
    std::cout << "4. Sort by roll using threads\n";
    std::cout << "5. Show sorted records\n";
    std::cout << "6. Query grade >= 9\n";
    std::cout << "7. Clear CSV\n";
    std::cout << "8. Show OOPD students (IIIT-Delhi)\n";
    std::cout << "0. Exit\n";
    std::cout << "==========================\n";
    std::cout << "Enter option: ";
}

// ---------------- MAIN ----------------
int main() {
    IIITDatabase db;
    bool running = true;
    bool sorted = false;

    while (running) {
        showMenu();

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear(); std::cin.ignore(10000, '\n');
            continue;
        }
        std::cin.ignore(10000, '\n');

        switch (choice) {

        case 1:
            db.loadFromCSV(CSV_FILE);
            std::cout << "Loaded. Total: " << db.getStudents().size() << "\n";
            break;

        case 2:
            addStudentsManually(db);
            break;

        case 3:
            db.showOriginalOrder();
            break;

        case 4: {
            std::size_t t;
            std::cout << "Threads (>=2 required): ";
            while (!(std::cin >> t)) {
                std::cin.clear(); std::cin.ignore(10000, '\n');
                std::cout << "Enter integer value: ";
            }
            std::cin.ignore(10000, '\n');

            if (t < 2) {
                std::cout << "Using 2 threads (minimum).\n";
                t = 2;
            }

            db.parallelSortByRoll(t);
            sorted = true;
            break;
        }

        case 5:
            if (!sorted) std::cout << "Sort first!\n";
            else db.showSortedOrder();
            break;

        case 6: {
            std::string course;
            std::cout << "Course to search (>=9): ";
            std::getline(std::cin, course);

            db.buildGradeIndex();
            auto result = db.queryByCourseAndMinGrade(course, 9.0);

            if (result.empty()) std::cout << "None found.\n";
            else {
                std::cout << "Students with grade >=9:\n";
                for (auto s : result) std::cout << *s << "\n";
            }
            break;
        }

        case 7:
            clearCSV(CSV_FILE, db);
            sorted = false;
            break;

        case 8:
            showOOPDStudents(db);
            break;

        case 0:
            running = false;
            break;

        default:
            std::cout << "Invalid choice.\n";
        }
    }

    std::cout << "Exiting...\n";
    return 0;
}
