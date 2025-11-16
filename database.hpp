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

// ========================
// StudentDatabase (Q3–Q5)
// ========================

template <typename RollT, typename CourseCodeT>
class StudentDatabase {
public:
    using StudentT = Student<RollT, CourseCodeT>;

private:
    std::vector<StudentT> students;          // original order
    std::vector<size_t> sortedIndices;       // index view for sorted order
    std::vector<long long> threadTimesMs;    // time taken by each sorting thread

    // course → multimap<grade, student-index> (sorted by grade desc)
    using GradeIndex =
        std::unordered_map<CourseCodeT,
                           std::multimap<double, size_t, std::greater<double>>>;

    GradeIndex gradeIndex;

public:
    // Add a student directly
    void addStudent(const StudentT &s) {
        students.push_back(s);
    }

    const std::vector<StudentT> &getStudents() const { return students; }

    // ========================
    // CSV loading (Q3)
    // Format assumed:
    // name,roll,branch,startYear
    // First non-empty line is treated as header and skipped.
    // ========================
    bool loadFromCSV(const std::string &filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Could not open CSV file: " << filename << "\n";
            return false;
        }

        std::string line;
        bool firstLine = true;

        while (std::getline(file, line)) {
            if (line.empty())
                continue;

            // Skip header line (e.g. "name,roll,branch,startYear")
            if (firstLine) {
                firstLine = false;
                continue;
            }

            std::stringstream ss(line);
            std::string name, rollStr, branch, startYearStr;

            if (!std::getline(ss, name, ','))       continue;
            if (!std::getline(ss, rollStr, ','))    continue;
            if (!std::getline(ss, branch, ','))     continue;
            if (!std::getline(ss, startYearStr, ',')) continue;

            try {
                int startYear = std::stoi(startYearStr);

                RollT rollValue{};
                if constexpr (std::is_same<RollT, std::string>::value) {
                    rollValue = rollStr;
                } else if constexpr (std::is_integral<RollT>::value) {
                    rollValue = static_cast<RollT>(std::stoull(rollStr));
                } else {
                    rollValue = RollT(rollStr);
                }

                StudentT s(name, rollValue, branch, startYear);
                students.push_back(s);
            } catch (const std::exception &e) {
                std::cerr << "Skipping invalid CSV line: '"
                          << line << "' (reason: " << e.what() << ")\n";
            }
        }

        return true;
    }

    // ========================
    // Parallel sort by roll (Q3)
    // ========================
    void parallelSortByRoll(std::size_t numThreads = 2) {
        const std::size_t n = students.size();
        if (n == 0) {
            std::cerr << "No students to sort.\n";
            return;
        }

        sortedIndices.resize(n);
        std::iota(sortedIndices.begin(), sortedIndices.end(), 0);

        // 🔹 Enforce at least 2 threads (assignment requirement),
        // but never more than number of students.
        if (numThreads < 2) {
            numThreads = 2;
        }
        if (numThreads > n) {
            numThreads = n;
        }
        // If n == 1, numThreads will become 1 (edge safety)
        if (numThreads == 0) {
            numThreads = 1;
        }

        threadTimesMs.assign(numThreads, 0);

        std::vector<std::thread> threads;
        threads.reserve(numThreads);

        std::vector<std::pair<std::size_t, std::size_t>> segments;
        segments.reserve(numThreads);

        // create segments
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

        // launch threads to sort their chunks
        for (std::size_t i = 0; i < numThreads; ++i) {
            auto [segStart, segEnd] = segments[i];

            threads.emplace_back([this, i, segStart, segEnd, comp]() {
                auto tStart = std::chrono::high_resolution_clock::now();

                std::sort(sortedIndices.begin() + segStart,
                          sortedIndices.begin() + segEnd,
                          comp);

                auto tEnd = std::chrono::high_resolution_clock::now();
                auto duration =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        tEnd - tStart)
                        .count();
                threadTimesMs[i] = duration;
            });
        }

        for (auto &t : threads) {
            t.join();
        }

        // sequentially merge sorted segments
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

        // Log thread times
        std::cout << "\nThread timing (Q3 - parallel sort, "
                  << numThreads << " threads used):\n";
        for (std::size_t i = 0; i < numThreads; ++i) {
            auto [segStart, segEnd] = segments[i];
            std::cout << "  Thread " << i
                      << " sorted indices [" << segStart << ", " << segEnd
                      << ") in " << threadTimesMs[i] << " ms\n";
        }
    }

    // ========================
    // Q4: Show records in original & sorted order
    // ========================

    void printStudentDetailed(const StudentT &s) const {
        std::cout << s << "\n";

        const auto &current = s.getCurrentCourses();
        if (!current.empty()) {
            std::cout << "    Current courses: ";
            for (const auto &c : current) {
                std::cout << c << " ";
            }
            std::cout << "\n";
        }

        const auto &completed = s.getCompletedCourses();
        if (!completed.empty()) {
            std::cout << "    Completed: ";
            for (const auto &p : completed) {
                std::cout << "(" << p.first << ", grade=" << p.second << ") ";
            }
            std::cout << "\n";
        }
    }

    // Original order using normal forward iterator
    void showOriginalOrder() const {
        std::cout << "\n=== Original order of records (Q4) ===\n";
        for (auto it = students.cbegin(); it != students.cend(); ++it) {
            printStudentDetailed(*it);
        }
    }

    // Sorted order using index-view iterators (no copying of students)
    void showSortedOrder() const {
        if (sortedIndices.empty()) {
            std::cout << "\nSorted indices empty. Did you call parallelSortByRoll()?\n";
            return;
        }

        std::cout << "\n=== Sorted order by roll (Q4) ===\n";
        for (auto it = sortedIndices.cbegin(); it != sortedIndices.cend(); ++it) {
            const StudentT &s = students[*it];
            printStudentDetailed(s);
        }

        // also show reverse iteration over sorted order
        std::cout << "\n=== Reverse iteration over sorted order ===\n";
        for (auto rit = sortedIndices.crbegin();
             rit != sortedIndices.crend();
             ++rit) {
            const StudentT &s = students[*rit];
            std::cout << s << "\n";
        }
    }

    // ========================
    // Q5: Fast grade-based queries
    // ========================

    // Build index: course → sorted by grade desc
    void buildGradeIndex() {
        gradeIndex.clear();
        for (std::size_t i = 0; i < students.size(); ++i) {
            const auto &completed = students[i].getCompletedCourses();
            for (const auto &p : completed) {
                const CourseCodeT &course = p.first;
                double grade = p.second;
                gradeIndex[course].emplace(grade, i);
            }
        }
    }

    // Get students with grade >= minGrade in given course
    std::vector<const StudentT *>
    queryByCourseAndMinGrade(const CourseCodeT &course,
                             double minGrade = 9.0) const {
        std::vector<const StudentT *> result;

        auto it = gradeIndex.find(course);
        if (it == gradeIndex.end()) {
            return result;
        }

        const auto &mm = it->second;
        for (auto mit = mm.begin(); mit != mm.end(); ++mit) {
            double grade = mit->first;
            if (grade < minGrade) break;
            std::size_t idx = mit->second;
            result.push_back(&students[idx]);
        }

        return result;
    }
};

#endif // DATABASE_HPP
