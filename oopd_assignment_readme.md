# OOPD Assignment 4 — Student Record Management System

Overview
This project implements a **Student Management System** for OOPD course of IIIT-Delhi using Object-Oriented Programming concepts in C++. It demonstrates:

- **Classes, inheritance, and templates**
- **Multithreading** (sorting using 2 or more threads)
- **File handling with CSV**
- **Iterators and STL containers**
- **Exception handling & input validation**
- **Querying students based on grades**
- **Filtering students enrolled in OOPD**

The program keeps track of students, their academic details, registered courses, completed courses, and grades.

---

## 🧱 File Structure
```
|-- main.cpp
|-- student.hpp
|-- database.hpp
|-- generate_3000.cpp
|-- Makefile
|-- oopd_students.csv
```

---

## Features Implemented
| Feature | Description |
|--------|------------|
| Add Students | Enter details manually with validation |
| Load CSV | Reads student records from CSV |
| Save to CSV | Stores all updated entries |
| Multithreaded Sorting | Parallel sorting using 2+ threads |
| Show Sorted Records | Displays students sorted by roll number |
| Filter OOPD Students | Shows students who have OOPD as a course |
| Query Top Students | Shows students with grade >= 9 in a selected course |
| Generate Large Dataset | Auto-generate 3000 random entries using code |

---

## Multithreaded Sorting Explanation
Sorting uses **index-based sorting** and divides the index array into thread blocks.
Each thread sorts its segment. Then `inplace_merge()` merges them.

Example Output:
```
Thread 0 sorted block [0, 1500) in 235 microseconds
Thread 1 sorted block [1500, 3000) in 212 microseconds
```
---

## CSV Format
```
name,roll,branch,startYear,currentCourses,completedCourses
student1,21185,csai,2021,oopd;dbms,34567:6.30;45678:8.04
```

### Example Meaning
- `currentCourses`: courses currently enrolled (semicolon separated)
- `completedCourses`: "courseCode:grade"

---

Compilation & Usage
### Compile
```bash
make
```
### Run
```bash
./oopdassign4
```

### Menu Options
```
1. Load CSV
2. Add students manually
3. Show original records
4. Sort by roll using threads
5. Show sorted records
6. Query grade >= 9
0. Exit
```
---

## Generating 3000 Random Students
Compile and run generator:
```bash
g++ -std=c++17 generate_3000.cpp -o gen3000
./gen3000
```
This creates `oopd_students.csv` with 3000 random entries.

---

## Concepts Demonstrated
- OOP concepts: **Encapsulation, Abstraction, Constructors, Custom Methods**
- STL Containers: `vector`, `map`, `unordered_map`, `multimap`
- Threads: `std::thread`, `inplace_merge`, `chrono timing`
- File handling: CSV reading/writing using stringstream
- Exception handling: try-catch blocks, input validation

---

## Future Improvements
- Search student by roll number
- Delete/Edit student record
- UI table formatting
- Graph plotting for grade comparison
- Web/GUI interface

---

## Outcome
This project demonstrates an efficient **OOP-based Student Record System** integrating multithreading, file handling, and structured dataset processing — aligned perfectly with OOPD course objectives.

--

