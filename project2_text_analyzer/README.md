# Dynamic Text Mining and Linguistic Pattern Analyzer

**Student ID:** 101  
**Course:** C Programming  
**Project:** Dynamic Text Mining and Linguistic Pattern Analyzer  
**Custom Analysis:** Lexical Diversity Score

---

## What This Project Does

This is a C program that reads a paragraph, splits it into words, counts how often each word appears, and lets you analyze the text through a simple text menu. The text and the word statistics are both stored in dynamic memory that grows when needed. Everything is freed when you exit.

---

## Selected Paragraph and Source

- **Title:** The Gettysburg Address
- **Author:** Abraham Lincoln
- **Publication details:** Speech delivered at Gettysburg, Pennsylvania, 19 November 1863 (public domain)
- **Length:** 271 words (the assignment asks for at least 150)

> Four score and seven years ago our fathers brought forth on this continent, a new nation, conceived in Liberty, and dedicated to the proposition that all men are created equal. Now we are engaged in a great civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are met on a great battle-field of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is altogether fitting and proper that we should do this. But, in a larger sense, we can not dedicate -- we can not consecrate -- we can not hallow -- this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or detract. The world will little note, nor long remember what we say here, but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. It is rather for us to be here dedicated to the great task remaining before us -- that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion -- that we here highly resolve that these dead shall not have died in vain -- that this nation, under God, shall have a new birth of freedom -- and that government of the people, by the people, for the people, shall not perish from the earth.

The speech exists in several versions with small wording differences. This is the commonly printed text, with `--` used for the dashes. You can also paste any other paragraph from menu option 2.

---

## Files

| File | What it contains |
|------|-----------------|
| `main.c` | Entry point, menu loop, and menu items |
| `analyzer.c` | `wordStat` struct, `Analyzer` struct, add/find word, free memory |
| `input.c` | Safe input helpers (read int, read line) |
| `text.c` | Pointer-based word extraction, loading text, reading a pasted paragraph |
| `default_text.c` | The Gettysburg Address and its source details |
| `operations.c` | Count, longest, most frequent, display, sort, and search |
| `analysis.c` | Custom analysis, text summary, and memory inspection |
| `sample_output.txt` | A real run of the program |

---

## How to Compile and Run

```bash
gcc main.c -o main
./main
```

`main.c` includes the other `.c` files, so you only compile `main.c`.

---

## Menu Options

```
=== Menu ===
1. Load default paragraph
2. Enter or paste a paragraph
3. Count words
4. Longest word
5. Most frequent word
6. Display word statistics
7. Sort word statistics
8. Search for a word
9. Text summary
10. Custom analysis (Lexical Diversity Score)
11. Memory inspection
12. Exit
```

When you choose option 2, paste your paragraph and press Enter on an empty line to finish.

---

## How Words Are Handled

These are the rules used when the program extracts words:

| Case | What the program does |
|------|-----------------------|
| Upper and lower case | All letters become lowercase, so `The` and `the` are the same word |
| Punctuation (`, . ; : ! ? " ( )`) | Acts as a separator and is removed |
| Dashes (`--`) | Separator |
| Apostrophes | Kept only between two letters, so `don't` stays one word. A quote around a word is removed |
| Hyphens | Kept only between two letters or digits, so `well-known` stays one word |
| Repeated spaces and line breaks | Treated as one separator |
| Digits | Count as word characters |
| Words longer than 39 characters | Cut to the first 39 characters (the `wordStat` name field holds 40 with the `\0`). The count of cut words is shown in "Count words" |
| Empty input | Rejected, the old text is kept |

---

## How the Code Is Structured

### Structs
- `wordStat` - holds one distinct word (`char name[40]`) and its `count`
- `Analyzer` - holds the text buffer, the `wordStat` array, and the counters

### Dynamic Memory
- `init_analyzer()` allocates the text buffer and the word array with `malloc`
- `read_paragraph()` starts with a 64 byte buffer and doubles it with `realloc` when a line does not fit
- `add_word()` doubles the word array with `realloc` when it is full
- Every `malloc` and `realloc` result is checked before it is used
- `free_analyzer()` releases everything at the end

### Function Pointers
The menu is an array of `MenuItem` structs. Each item stores a label and a function pointer of type `AnalysisAction`. When you pick a number, the program calls that pointer directly.

```c
typedef void (*AnalysisAction)(Analyzer *a);

static const MenuItem menu[] = {
    {"Count words", count_words},
    {"Longest word", longest_word},
    // ...
};
```

Sorting also uses function pointers. `compare_alphabetical` and `compare_frequency` are passed to the same `bubble_sort()`.

### Pointer Traversal
`build_stats()` walks through the text with a `char *` pointer, one character at a time, to find where each word starts and ends. `find_word()`, `find_longest()`, `display_word_statistics()` and the summary walk the `wordStat` array with a `wordStat *` pointer instead of using an index.

### String Functions
- `strcmp()` compares words to find duplicates and to sort alphabetically
- `strlen()` finds word lengths
- `strcpy()` copies a word into the `wordStat` array (safe because words are cut to 39 characters first)

---

## Custom Analysis: Lexical Diversity Score

**Purpose:** shows how varied the vocabulary of the text is. A text that keeps repeating the same words scores low. A text that uses many different words scores high.

**Formula:**

```
Lexical Diversity Score = distinct words / total words
```

The score is between 0 and 1.

**Interpretation:**

| Score | Meaning |
|-------|---------|
| Below 0.40 | Low, many repeated words |
| 0.40 to below 0.60 | Moderate |
| 0.60 and above | High, varied vocabulary |

**Result for the Gettysburg Address:** 138 distinct words / 271 total words = **0.509**, which is Moderate. This makes sense because the speech repeats key words on purpose (`that` appears 13 times and `we` 10 times).

Short texts tend to score higher than long ones, so the score is best used to compare paragraphs of similar length.

---

## Memory Inspection

Menu option 11 prints three addresses with `%p` (cast to `void *`):

| Address | What it is |
|---------|-----------|
| Text buffer address | Start of the heap block that holds the paragraph |
| Word statistics array | Start of the heap block that holds all `wordStat` entries |
| Selected word-stat entry | The entry of the most frequent word, which sits inside the array. Its address is the array address plus `index * sizeof(wordStat)` |

The addresses change between runs and between computers.

---

## Evidence

`sample_output.txt` is a real run that shows:

- every menu operation on the Gettysburg Address
- the memory addresses
- a pasted paragraph with punctuation, repeated words, mixed case, extra spaces, and a very long word. The result shows `don't` and `well-known` kept as single words, `word`, `Word` and `(word)` merged into one entry, and the long word cut to 39 characters
- the text buffer growing from 64 to 128 to 256 bytes while the paragraph was pasted

---

## AI Usage Disclosure

An AI assistant was used only for this README and the mock data (the default Gettysburg Address text in `default_text.c`).
