# Environment Monitoring System

A multi-process Linux application that simulates a real-time monitoring pipeline for environmental conditions in an industrial setup. The system generates simulated sensor values, processes the data, detects abnormal conditions, and logs all system activity to a structured log file.

---

## Overview

This project demonstrates core Operating System concepts through a practical, domain-relevant implementation:

- Process creation and management using `fork()` and `execl()`
- Multithreading with POSIX threads
- Inter-Process Communication (IPC) using Message Queue, Shared Memory, and Pipe
- Signal handling for graceful shutdown
- Structured file logging with `lseek`
- Modular software design with separate executables per process

---

## Architecture

<img width="1536" height="1024" alt="image" src="https://github.com/user-attachments/assets/b9600957-1964-4548-8ffc-6e082bcede80" />


```
Sensor → Processor → Display → Logger → system_log.txt
```

Each stage runs as an independent process. Data flows through IPC mechanisms between stages.

| Stage     | Role                                      | IPC Used                    |
|-----------|-------------------------------------------|-----------------------------|
| Sensor    | Generates Temperature, Humidity, AQI data | Message Queue (sender)      |
| Processor | Receives data and stores to shared memory | Message Queue + Shared Mem  |
| Display   | Reads data, prints alerts, sends to log   | Shared Mem + Pipe           |
| Logger    | Receives log lines and writes to file     | Pipe (reader)               |

---

## System Workflow

### 1. Sensor Process (`sensor_final.c`)

Simulates environmental sensor inputs using random values seeded with `srand(42)`.

Generated parameters:
- Temperature (°C)
- Humidity (%)
- Air Quality Index (AQI)

Each cycle, the sensor sends data to the Processor via a POSIX Message Queue.

```
[Sensor] Temp=25 Hum=60 AQI=120
```

### 2. Processor Process (`process_final.c`)

Receives sensor data from the Message Queue and writes it into Shared Memory. Synchronization between Processor and Display is handled using POSIX Semaphores.

```
[Processor] Temp=25 Hum=60 AQI=120
```

### 3. Display Process (`display_final.c`)

Reads data from Shared Memory using two POSIX threads:

- **Thread 1** — Reads and displays sensor values every second, checks alert thresholds, and sends log lines to the Logger via a pipe.
- **Thread 2** — Prints a periodic system health summary every 5 seconds.

Alert rules:
- Temperature > 40°C → High Temperature alert
- AQI > 200 → Poor Air Quality alert

```
[Display] Temp=47 Hum=78 AQI=89
[Display ALERT] High Temperature!
```

### 4. Logger Process (`logger_final.c`)

Reads log lines from the pipe (via `STDIN`) and writes them to `environment_log.txt`. Uses `lseek` to append a timestamped session header and footer. Each write is immediately flushed to disk using `fsync`.

```
=== Session started 2026-03-24 10:12:01 ===
[Display] Temp=47 Hum=78 AQI=89
[ALERT] High Temperature
[Alert Monitor] System alive - cycle 1
=== Session ended ===
```

---

## Technologies Used

| Component       | Technology            |
|-----------------|-----------------------|
| Language        | C                     |
| Build System    | Makefile              |
| Threading       | POSIX pthread         |
| IPC             | Message Queue, Shared Memory, Pipe |
| Synchronization | POSIX Semaphores      |
| Signal Handling | `signal()`            |
| Logging         | File I/O (`open`, `write`, `fsync`, `lseek`) |

---

## System Calls and APIs Used

| System Call      | Purpose                          |
|------------------|----------------------------------|
| `fork()`         | Create child processes           |
| `execl()`        | Execute each process binary      |
| `wait()`         | Wait for child process exit      |
| `kill()`         | Send SIGTERM to children on shutdown |
| `pthread_create()` | Create display and monitor threads |
| `pthread_join()` | Wait for thread completion       |
| `mq_open()`      | Create POSIX message queue       |
| `mq_send()`      | Send sensor data                 |
| `mq_receive()`   | Receive sensor data              |
| `shm_open()`     | Create POSIX shared memory       |
| `mmap()`         | Map shared memory into process   |
| `sem_open()`     | Create named semaphore           |
| `sem_wait()`     | Lock shared resource             |
| `sem_post()`     | Unlock shared resource           |
| `pipe()`         | Create pipe between display and logger |
| `dup2()`         | Redirect pipe file descriptors   |
| `signal()`       | Register signal handler          |
| `open()`         | Open log file                    |
| `write()`        | Write log entries                |
| `fsync()`        | Flush data to disk immediately   |
| `lseek()`        | Position file offset for header/footer |

---

## Signal Handling

The Supervisor process handles the following signals:

| Signal    | Purpose                          |
|-----------|----------------------------------|
| `SIGINT`  | Graceful shutdown (Ctrl + C) — sends SIGTERM to all child processes |
| `SIGTERM` | Same graceful shutdown behavior  |

Example:
```
^C
[Supervisor] Shutdown signal received. Stopping all processes...
[Supervisor] All processes stopped.
```

---

## Project Structure

```
project/
│
├── env_final.h          # Shared constants and data structures
├── main_final.c         # Supervisor: forks all child processes
├── sensor_final.c       # Sensor process
├── process_final.c      # Processor process
├── display_final.c      # Display process (2 threads)
├── logger_final.c       # Logger process
├── Makefile             # Build, run, and clean targets
│
└── environment_log.txt  # Generated at runtime
```

---

## Build Instructions

### Requirements

- Linux (Ubuntu / Debian recommended)
- GCC with POSIX support
- `librt` and `pthread` libraries (included by default on most Linux distros)

### Compile

```bash
make
```

### Run

```bash
make run
```

or directly:

```bash
./main_final
```

### Clean

```bash
make clean
```

This removes all compiled binaries, the log file, and all IPC resources (`/dev/mqueue`, `/dev/shm`).

---

## Testing Scenarios

### Normal Operation

- Temperature ≤ 40°C and AQI ≤ 200
- System runs without any alerts

### High Temperature

- Temperature > 40°C
```
[Display ALERT] High Temperature!
[ALERT] High Temperature
```

### Poor Air Quality

- AQI > 200
```
[Display ALERT] Poor Air Quality!
[ALERT] Poor Air Quality
```

### Critical Condition

- Temperature > 40°C AND AQI > 200
```
[Display ALERT] High Temperature!
[Display ALERT] Poor Air Quality!
[ALERT] High Temperature
[ALERT] Poor Air Quality
```

---

## Demo Steps

1. Build the project
```bash
make
```

2. Run the system
```bash
./main_final
```

3. Observe real-time output on terminal

4. Wait 10 seconds then stop
```bash
Ctrl + C
```

5. Check the log file
```bash
cat environment_log.txt
```

---

## Challenges Faced

- Synchronizing shared memory access between Processor and Display processes
- Handling pipe file descriptor inheritance across `fork()` and `execl()`
- Ensuring log file data reaches disk before process termination
- Managing multiple IPC cleanup on shutdown

---

## What We Learned

- Linux process management with `fork`, `execl`, `wait`
- Inter-process communication using three different IPC mechanisms
- Thread synchronization with mutexes and semaphores
- Signal handling and graceful multi-process shutdown
- System-level file I/O with `lseek` and `fsync`

---

## Author

**Sanket Chavan**  
Branch: Electronics and Telecommunication Engineering  
Institute: AISSMS IOIT, Pune  
Project Type: Embedded Linux / Operating Systems Mini Project
