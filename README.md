# C Programming Project 2 Assignment

**Student ID:** 101  
**Course:** C Programming

This repository holds the three projects of the Project 2 assignment. Each project is a separate C program in its own folder, with its own README that explains the code, how to run it, and its custom analysis.

---

## Projects

| Folder | Project | Custom part |
|--------|---------|-------------|
| [`project1_academic_risk`](project1_academic_risk) | Dynamic Academic Performance and Risk Analysis Engine | Academic Risk Index |
| [`project2_text_analyzer`](project2_text_analyzer) | Dynamic Text Mining and Linguistic Pattern Analyzer | Lexical Diversity Score |
| [`project3_iot_engine`](project3_iot_engine) | Dynamic IoT Event Monitoring and Callback Engine | Sensor Drift Detector |

---

## How to Compile and Run

You need `gcc` installed. In each project folder, `main.c` includes the other `.c` files, so only `main.c` is compiled:

```bash
cd project1_academic_risk
gcc main.c -o main
./main
```

Use the same two commands inside `project2_text_analyzer` and `project3_iot_engine`.

---

## C Concepts Used

| Concept | Project 1 | Project 2 | Project 3 |
|---------|-----------|-----------|-----------|
| Structs | `Student`, `StudentList` | `wordStat`, `Analyzer` | `Device`, `Monitor`, `Alert` |
| Union | - | - | Reading of a device |
| Dynamic memory (`malloc`, `realloc`, `free`) | Student array | Text buffer and word array | Device array and alert array |
| Function pointers | Menu, sorting, search, `apply_to_all` | Menu and sorting | Menu and device callbacks |
| Pointer traversal | `find_by_id`, pointer subtraction | Word extraction from the text | Device and alert arrays |
| String functions | `strcasecmp`, `strcasestr` | `strcmp`, `strlen`, `strcpy` | `strcpy`, `snprintf` |

---

## Folder Layout

```
project1_academic_risk/   main.c, student.c, input.c, operations.c, analysis.c, mock_data.c
project2_text_analyzer/   main.c, analyzer.c, input.c, text.c, default_text.c, operations.c, analysis.c
project3_iot_engine/      main.c, device.c, input.c, callbacks.c, operations.c, alerts.c, simulation.c, mock_data.c
```

Projects 2 and 3 were first written as one `main.c` file and then split into separate files. The commit history shows this: one commit with everything in `main.c`, then one commit for each new file, and a last commit that trims `main.c` down to the menu and the includes.

---

## AI Usage Disclosure

Project 1 has its own disclosure in its README.

An AI assistant was used only for the READMEs and the mock data in Projects 2 and 3. The sample outputs are real runs of the programs.
