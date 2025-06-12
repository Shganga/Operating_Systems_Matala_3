# Operation Systems 3 - Convex Hull 

This project implements several server architectures for geometric computation over TCP, supporting multiple clients and real-time convex hull area monitoring.


---

## Directory Layout

- **step1** to **step10** — Each directory represents a progressive stage, containing its own server, client, and makefile.
- **makefile** — The root makefile automates building all steps at once.

---



## Building the Project

To compile everything from the root directory, simply run:
```sh
make
```
This command will recursively build all stages (step1–step10).

To remove all compiled files:
```sh
make clean
```

For building or cleaning a specific stage, navigate into that step’s folder and use `make` or `make clean`.

---

## Usage Example

**Sample interaction:**
```
Newgraph 4
0,0
0,10
10,10
10,0
CH
```

---

## Supported Commands

| Command             | Functionality                                    |
|---------------------|--------------------------------------------------|
| `Newgraph N`        | Begin a new graph, expecting N points next       |
| `Newpoint X,Y`      | Add a point to the current graph                 |
| `Removepoint X,Y`   | Remove a specific point if it exists             |
| `CH`                | Calculate and return the convex hull area        |


---
