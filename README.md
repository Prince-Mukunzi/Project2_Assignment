# Dynamic Academic Performance and Risk Analysis Engine

**Student ID:** [your_student_id]  
**Course:** C Programming  
**Project:** Dynamic Academic Performance and Risk Analysis Engine  
**Custom Analysis:** Academic Risk Index

---

## What This Project Does

This is a C program that manages student academic records. It lets you add, remove, search, sort, and analyze student data through a simple text menu. The program stores everything in memory while it runs and cleans up when you exit.

---

## Files

| File | What it contains |
|------|-----------------|
| `main.c` | Entry point, menu loop, and menu items |
| `student.c` | Student struct, dynamic list (add, grow, free) |
| `operations.c` | Add, display, sort, search, remove student functions |
| `analysis.c` | Custom risk analysis and summary report |
| `input.c` | Safe input helpers (read int, read line, read grade) |
| `mock_data.c` | 12 sample students loaded at startup |

---

## How to Compile and Run

You need `gcc` installed (comes with most Linux/Mac setups).

```bash
gcc main.c -o main
./main
```

That's it. No extra libraries needed.

---

## Menu Options

When you run the program you'll see this menu:

```
=== Menu ===
1. Add student
2. Display students
3. Sort students
4. Calculate Average Grade
5. Top Performing Student
6. Search Student by ID or Name
7. Remove Student Record
8. Custom Analysis
9. Summary Report
10. Exit
```

Type the number and press Enter.

---

## What Each Option Does

1. **Add student** — Enter an ID, name, course, and grade. The program rejects duplicate IDs and grades outside 0–100.
2. **Display students** — Prints every student record.
3. **Sort students** — Choose to sort by grade (highest first), name (A–Z), or student ID.
4. **Calculate Average Grade** — Shows the average grade across all students.
5. **Top Performing Student** — Shows the student with the highest grade.
6. **Search** — Find a student by ID or by name (partial name works too).
7. **Remove Student Record** — Delete a student by ID. Memory is shrunk after removal.
8. **Custom Analysis (Academic Risk Index)** — Lists students who are at academic risk:
   - **High risk**: grade below 50
   - **Moderate risk**: grade below 60
9. **Summary Report** — Shows total students, average grade, highest grade, lowest grade, and how many students fall into each risk band.
10. **Exit** — Frees all memory and closes the program.

---

## How the Code Is Structured

### Structs
- `Student` — holds id, name, course, and grade
- `StudentList` — a dynamic array of students with a count and capacity

### Dynamic Memory
- `init_list()` starts with space for 5 students using `malloc`
- `add_to_list()` doubles the capacity with `realloc` when it's full
- `remove_student()` shrinks memory with `realloc` after deletion
- `free_list()` releases everything at the end

### Function Pointers
The menu is built as an array of `MenuItem` structs where each item stores a label and a **function pointer** to the matching operation. When you pick a number, the program calls that pointer directly instead of using a big `if/else` or `switch`.

```c
// example from main.c
typedef void (*MenuAction)(StudentList *list);

static const MenuItem menu[] = {
    {"Add student", add_student},
    {"Display students", display_students},
    // ...
};
```

`apply_to_all()` in `student.c` also uses a function pointer to apply any operation to every student in the list (used for displaying, summing grades, finding the top student, etc.).

### Pointer Traversal
`find_by_id()` walks the array using a regular index but returns a pointer directly into the list so the caller can read or modify that record. `remove_student()` uses pointer subtraction (`target - list->items`) to find the index of the student to delete.

---

## Sample Students (Mock Data)

The program loads 12 students at startup so you can test without typing anything:

| ID | Name | Course | Grade |
|----|------|--------|-------|
| 101 | Alice Uwase | Computer Science | 88.5 |
| 102 | Brian Habimana | Mathematics | 72.0 |
| 103 | Chloe Mukamana | Physics | 45.5 |
| 104 | David Nkurunziza | Computer Science | 91.0 |
| 105 | Eva Ingabire | Biology | 58.0 |
| 106 | Frank Mugisha | Mathematics | 63.5 |
| 107 | Grace Umutoni | Physics | 49.0 |
| 108 | Henry Niyonzima | Biology | 76.5 |
| 109 | Irene Kayitesi | Computer Science | 83.0 |
| 110 | Jean Bosco | Mathematics | 55.0 |
| 111 | Kevin Rugamba | Physics | 67.0 |
| 112 | Linda Mutesi | Biology | 94.5 |

---

## AI Usage Disclosure

I used **Antigravity AI (powered by Google Gemini)** in two parts of this project:

### 1. README file
I asked the AI to write this README based on my assignment brief and my actual source code. I reviewed it and it accurately describes what my code does.

### 2. Mock student data (`mock_data.c`)
I asked the AI to generate a set of 12 realistic-looking student names and grades that fit the assignment requirements (minimum 10, maximum 15 students, grades between 0 and 100, mix of courses). I then reviewed the data and kept it as-is because it covered a good spread of risk levels (some below 50, some between 50–60, and some high performers).

All other code — the structs, dynamic memory logic, function pointers, sorting, analysis, and input handling — was written by me.

