#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "student.hpp"

#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <numeric>
#include <algorithm>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <functional>
#include <iostream>
#include <cctype>

// ========================
// StudentDatabase (Q3–Q5)
// ========================

template <typename RollT, typename CourseCodeT>
class StudentDatabase {
public:
    using StudentT = Student<RollT, CourseCodeT>;

private:
    std::vector<StudentT>  students;         // original order
    std::vector<size_t>    sortedIndices;    // index view for sorted order
    std::vector<long long> threadTimesMs;    // time taken by each sorting thread

    using GradeIndex =
        std::unordered_map<CourseCodeT,
                           std::multimap<double, size_t, std::greater<double>>>;

    GradeIndex gradeIndex;

public:
    // Add a student directly
    void addStudent(const StudentT &s) {
        students.push_back(s);
    }

    const std::vector<StudentT> &getStudents() const {
        return students;
    }

    // ========================
    // CSV loading (with courses & grades)
    // CSV format:
    // name,roll,branch,startYear,currentCourses,completedCourses
    // currentCourses:  "oopd;ml"
    // completedCourses:"12345:9.8;ga:7.0"
    // ========================
    bool loadFromCSV(const std::string &filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Could not open CSV file: " << filename << "\n";
            return false;
        }

        std::string line;
        bool firstLine = true;

        // Optional: start fresh each time you load
        students.clear();

        auto trim = [](std::string &s) {
            while (!s.empty() &&
                   std::isspace(static_cast<unsigned char>(s.front())))
                s.erase(s.begin());
            while (!s.empty() &&
                   std::isspace(static_cast<unsigned char>(s.back())))
                s.pop_back();
        };

        while (std::getline(file, line)) {
            if (line.empty()) continue;

            if (firstLine) {   // header
                firstLine = false;
                continue;
            }

            std::stringstream ss(line);
            std::string name, rollStr, branch, yearStr, currentStr, completedStr;

            if (!std::getline(ss, name, ','))        continue;
            if (!std::getline(ss, rollStr, ','))     continue;
            if (!std::getline(ss, branch, ','))      continue;
            if (!std::getline(ss, yearStr, ','))     continue;
            if (!std::getline(ss, currentStr, ','))  currentStr.clear();
            if (!std::getline(ss, completedStr, ',')) completedStr.clear();

            trim(name);
            trim(rollStr);
            trim(branch);
            trim(yearStr);
            trim(currentStr);
            trim(completedStr);

            try {
                int startYear = std::stoi(yearStr);

                RollT rollValue{};
                if constexpr (std::is_same<RollT, std::string>::value) {
                    rollValue = rollStr;
                } else if constexpr (std::is_integral<RollT>::value) {
                    rollValue = static_cast<RollT>(std::stoull(rollStr));
                }

                StudentT s(name, rollValue, branch, startYear);

                // Parse current courses: "oopd;ml"
                if (!currentStr.empty()) {
                    std::stringstream cc(currentStr);
                    std::string token;
                    while (std::getline(cc, token, ';')) {
                        trim(token);
                        if (!token.empty()) {
                            s.enrollInCourse(token);
                        }
                    }
                }

                // Parse completed: "12345:9.8;ga:7.0"
                if (!completedStr.empty()) {
                    std::stringstream cm(completedStr);
                    std::string token;
                    while (std::getline(cm, token, ';')) {
                        trim(token);
                        if (token.empty()) continue;

                        auto pos = token.find(':');
                        if (pos == std::string::npos) continue;

                        std::string course   = token.substr(0, pos);
                        std::string gradeStr = token.substr(pos + 1);
                        trim(course);
                        trim(gradeStr);

                        double grade = std::stod(gradeStr);
                        s.completeCourse(course, grade);
                    }
                }

                students.push_back(s);
            }
            catch (const std::exception &e) {
                std::cerr << "Skipping invalid CSV row: '" << line
                          << "' (" << e.what() << ")\n";
            }
        }

        return true;
    }

    // ========================
    // Parallel sort by roll
    // ========================
    void parallelSortByRoll(std::size_t numThreads = 2) {
        const std::size_t n = students.size();
        if (n == 0) {
            std::cerr << "No students to sort.\n";
            return;
        }

        sortedIndices.resize(n);
        std::iota(sortedIndices.begin(), sortedIndices.end(), 0);

        // Enforce at least 2 threads (assignment requirement)
        if (numThreads < 2) numThreads = 2;
        if (numThreads > n) numThreads = n;
        if (numThreads == 0) numThreads = 1;

        threadTimesMs.assign(numThreads, 0);

        std::vector<std::thread> threads;
        threads.reserve(numThreads);

        std::vector<std::pair<std::size_t, std::size_t>> segments;
        segments.reserve(numThreads);

        std::size_t baseSize = n / numThreads;
        std::size_t remainder = n % numThreads;
        std::size_t start = 0;

        for (std::size_t i = 0; i < numThreads; ++i) {
            std::size_t blockSize = baseSize + (i < remainder ? 1 : 0);
            std::size_t end = start + blockSize;
            segments.emplace_back(start, end);
            start = end;
        }

        auto comp = [this](std::size_t a, std::size_t b) {
            return students[a].getRoll() < students[b].getRoll();
        };

        for (std::size_t i = 0; i < numThreads; ++i) {
            auto [segStart, segEnd] = segments[i];

            threads.emplace_back([this, i, segStart, segEnd, comp]() {
                auto tStart = std::chrono::high_resolution_clock::now();

                std::sort(sortedIndices.begin() + segStart,
                          sortedIndices.begin() + segEnd,
                          comp);

                auto tEnd = std::chrono::high_resolution_clock::now();
                threadTimesMs[i] =
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        tEnd - tStart).count();
            });
        }

        for (auto &t : threads) t.join();

        std::size_t currentStart = segments[0].first;
        std::size_t currentEnd   = segments[0].second;

        for (std::size_t i = 1; i < segments.size(); ++i) {
            std::size_t nextStart = segments[i].first;
            std::size_t nextEnd   = segments[i].second;

            std::inplace_merge(sortedIndices.begin() + currentStart,
                               sortedIndices.begin() + nextStart,
                               sortedIndices.begin() + nextEnd,
                               comp);

            currentEnd = nextEnd;
        }

        std::cout << "\nThread timing (parallel sort, " << numThreads
                  << " threads used):\n";
        for (std::size_t i = 0; i < numThreads; ++i) {
            auto [segStart, segEnd] = segments[i];
            std::cout << "  Thread " << i << " sorted block ["
                      << segStart << ", " << segEnd << ") in "
                      << threadTimesMs[i] << " microseconds\n";
        }
    }

    // ========================
    // Display functions
    // ========================
    void printStudentDetailed(const StudentT &s) const {
        std::cout << s << "\n";

        const auto &current = s.getCurrentCourses();
        if (!current.empty()) {
            std::cout << "    Current: ";
            for (const auto &c : current) std::cout << c << " ";
            std::cout << "\n";
        }

        const auto &completed = s.getCompletedCourses();
        if (!completed.empty()) {
            std::cout << "    Completed: ";
            for (const auto &p : completed)
                std::cout << "(" << p.first << ", grade=" << p.second << ") ";
            std::cout << "\n";
        }
    }

    void showOriginalOrder() const {
        std::cout << "\n=== Original order of records ===\n";
        for (const auto &s : students) {
            printStudentDetailed(s);
        }
    }

    void showSortedOrder() const {
        if (sortedIndices.empty()) {
            std::cout << "\nSorted indices empty. Call parallelSortByRoll() first.\n";
            return;
        }

        std::cout << "\n=== Sorted order by roll ===\n";
        for (auto idx : sortedIndices) {
            printStudentDetailed(students[idx]);
        }
    }

    // ========================
    // Grade-based query
    // ========================
    void buildGradeIndex() {
        gradeIndex.clear();
        for (std::size_t i = 0; i < students.size(); ++i) {
            const auto &completed = students[i].getCompletedCourses();
            for (const auto &p : completed) {
                gradeIndex[p.first].emplace(p.second, i);
            }
        }
    }

    std::vector<const StudentT *>
    queryByCourseAndMinGrade(const CourseCodeT &course,
                             double minGrade = 9.0) const {
        std::vector<const StudentT *> result;
        auto it = gradeIndex.find(course);
        if (it == gradeIndex.end()) return result;

        for (const auto &entry : it->second) {
            if (entry.first < minGrade) break;
            result.push_back(&students[entry.second]);
        }
        return result;
    }
};

#endif // DATABASE_HPP
