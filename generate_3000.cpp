// generate_3000.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <random>
#include <unordered_set>

int main() {
    const std::string filename = "oopd_students.csv";

    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Could not open " << filename << " for writing\n";
        return 1;
    }

    // CSV header (MUST match what your main program expects)
    out << "name,roll,branch,startYear,currentCourses,completedCourses\n";

    std::vector<std::string> branches = {"cse", "ece", "csam", "csai", "csd", "csss"};
    std::vector<std::string> currentCoursePool   = {"oopd", "dbms", "ml", "ga", "os", "math"};
    std::vector<std::string> completedCoursePool = {"12345", "23456", "34567", "45678", "56789"};

    // Create 3000 unique roll numbers and shuffle them for random order
    std::vector<int> rolls(3000);
    std::iota(rolls.begin(), rolls.end(), 20000);   // 20000–22999

    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(rolls.begin(), rolls.end(), rng);

    std::uniform_int_distribution<int> yearDist(2020, 2024);
    std::uniform_int_distribution<int> branchDist(0, (int)branches.size() - 1);
    std::uniform_int_distribution<int> currCountDist(1, 3);     // 1–3 current courses
    std::uniform_int_distribution<int> compCountDist(1, 3);     // 1–3 completed courses
    std::uniform_int_distribution<int> currPoolDist(0, (int)currentCoursePool.size() - 1);
    std::uniform_int_distribution<int> compPoolDist(0, (int)completedCoursePool.size() - 1);
    std::uniform_real_distribution<double> gradeDist(5.0, 10.0); // grades between 5 and 10

    for (int i = 0; i < 3000; ++i) {
        std::string name   = "student" + std::to_string(i + 1);
        int rollNum        = rolls[i];
        std::string roll   = std::to_string(rollNum);
        std::string branch = branches[branchDist(rng)];
        int startYear      = yearDist(rng);

        // ---- Unique current courses ----
        int curN = currCountDist(rng);
        std::unordered_set<std::string> usedCurrent;
        while ((int)usedCurrent.size() < curN) {
            usedCurrent.insert(currentCoursePool[currPoolDist(rng)]);
        }

        std::string currentCourses;
        for (const auto &c : usedCurrent) {
            if (!currentCourses.empty()) currentCourses += ";";
            currentCourses += c;
        }

        // ---- Unique completed courses ----
        int compN = compCountDist(rng);
        std::unordered_set<std::string> usedCompleted;
        while ((int)usedCompleted.size() < compN) {
            usedCompleted.insert(completedCoursePool[compPoolDist(rng)]);
        }

        std::string completedCourses;
        for (const auto &code : usedCompleted) {
            if (!completedCourses.empty()) completedCourses += ";";
            double grade = gradeDist(rng);
            completedCourses += code + ":" + std::to_string(grade);
        }

        // Write CSV row
        out << name << ","
            << roll << ","
            << branch << ","
            << startYear << ","
            << currentCourses << ","
            << completedCourses << "\n";
    }

    std::cout << "Generated 3000 entries in " << filename << "\n";
    return 0;
}
