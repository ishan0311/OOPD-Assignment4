#ifndef STUDENT_HPP
#define STUDENT_HPP

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>

// ========================
// Template Student class (Q1)
// ========================

template <typename RollT, typename CourseCodeT>
class Student {
private:
    std::string name;
    RollT roll;
    std::string branch;
    int startYear;

    std::vector<CourseCodeT> currentCourses;
    std::map<CourseCodeT, double> completedCourses;  // course -> grade

public:
    Student() = default;

    Student(const std::string &name,
            const RollT &roll,
            const std::string &branch,
            int startYear)
        : name(name), roll(roll), branch(branch), startYear(startYear) {}

    // Getters – abstraction & data hiding
    const std::string &getName() const { return name; }
    const RollT &getRoll() const { return roll; }
    const std::string &getBranch() const { return branch; }
    int getStartYear() const { return startYear; }

    const std::vector<CourseCodeT> &getCurrentCourses() const {
        return currentCourses;
    }

    const std::map<CourseCodeT, double> &getCompletedCourses() const {
        return completedCourses;
    }

    // Behavior
    void enrollInCourse(const CourseCodeT &course) {
        currentCourses.push_back(course);
    }

    void completeCourse(const CourseCodeT &course, double grade) {
        // remove from current if present
        auto it = std::find(currentCourses.begin(), currentCourses.end(), course);
        if (it != currentCourses.end()) {
            currentCourses.erase(it);
        }
        completedCourses[course] = grade;
    }
};

// Pretty-print a Student (for demo)
template <typename RollT, typename CourseCodeT>
std::ostream &operator<<(std::ostream &os,
                         const Student<RollT, CourseCodeT> &s) {
    os << "Name: " << s.getName()
       << ", Roll: " << s.getRoll()
       << ", Branch: " << s.getBranch()
       << ", StartYear: " << s.getStartYear();
    return os;
}

#endif // STUDENT_HPP
