# CS111: Operating Systems - Summer 2025

## 1. Course Information and Structure

### 1.1 Course Delivery
- This course is offered asynchronously.
- No in-person or live sessions.
- All lectures are recorded and made available online.
- Exams, labs, and office hours are conducted online.

### 1.2 Instructor
- Name: Peter Reier
- Affiliation: UCLA Computer Science Department (since late 1970s)
- Expertise: Operating systems research and publication
- Contact: Provided via email (check class portal)
- Office Hours: By appointment via Zoom (no preset slots)

### 1.3 Teaching Assistant (TA)
- Name: Sara Khosravi
- Office Hours: Via Zoom (announced separately)
- Role: Handles all lab/project-related questions and grading

### 1.4 Instructor & TA Roles

| Responsibility              | Instructor            | TA                             |
|----------------------------|-----------------------|--------------------------------|
| Lectures                   | Yes                   | No                             |
| Readings                   | Assigns               | No                             |
| Exams                      | Creates & Grades      | No                             |
| Projects/Labs              | No                    | Manages & Grades               |
| Project Help               | Redirect to TA        | Provides help and instruction  |
| Office Hours               | By appointment        | Set schedule via Zoom          |

### 1.5 Course Website
- Contains readings schedule, lecture slides (PowerPoint & PDF), exam access, project submission, and announcements.
- Sample midterm and final exams posted before respective dates.
- Zoom IDs generally not on site (except for lecture links).

## 2. Prerequisites

### 2.1 Required Courses
- **CS 32: Intermediate Programming**
  - Requires CS 31 as pre-req
  - Covers object-oriented programming, data structures (trees, etc.)

- **CS 33: Systems Programming**
  - Introduction to assembly, CPU architecture, memory/registers
  - Teaches linking, stack frames, register saving

- **CS 35L: Tools and Software Construction**
  - Practical tools: GitHub, debuggers, and lab-related tools

### 2.2 Assumed Knowledge
- Must understand core programming concepts
- Familiarity with system-level programming tools
- Ability to write and debug C/C++ programs with command-line tools

## 3. Course Components

### 3.1 Lectures
- Pre-recorded; roughly 2 hours each
- Expected time: 4–6 hours/week (including re-watching)

### 3.2 Readings
- Primary Textbook:  
  **Operating Systems: Three Easy Pieces** by Remzi & Andrea Arpacci-Dusseau (University of Wisconsin)  
  - Free, online
  - Weekly chapter links posted
- Supplementary readings provided via external web links
- Expected time: 3–6 hours/week

### 3.3 Labs/Projects
- **Warm-Up Lab** (9%) – Tests readiness for the course
- Four individual, programming-intensive projects (11% each)

| Type            | Description                                  |
|----------------|----------------------------------------------|
| Warm-Up        | Diagnostic - ensure course readiness         |
| Project 1–4    | OS features: file systems, threading, etc.   |

> **Note** Projects are individual. No collaboration is allowed.

- Estimated time: 3–20 hours/week
- Labs include real coding, debugging, and measurement

#### Lab Policies
- Deadlines on syllabus unless changed by TA
- TA can extend deadlines or revise late policy for entire class
- Contact only TA for:
  - Extensions
  - Clarifications
  - Grade disputes

> Instructor will defer all lab issues to the TA.

### 3.4 Exams

#### Midterm
- **Date**: End of Week 5 (Friday)
- **Format**: Multiple-choice, online
- **Access**: Available 24 hours on Pacific Time
- **Coverage**: All lecture and reading material up to date
- **Exclusions**: Project-only material not covered  
- **Resources allowed**: Open book, notes, web reading

#### Final Exam
- **Date**: Friday, August 29th
- **Format**: Multiple-choice, online
- **Access**: 24-hour window (PST)
- **Coverage**: Entire course
- **Emphasis**: Post-midterm material

> Both exams test understanding and application of OS principles.

### 3.5 Grading Breakdown

| Component             | Weight (%)     |
|----------------------|----------------|
| Class Evaluation     | 1              |
| Midterm              | 20             |
| Final Exam           | 26             |
| Warm-Up Project      | 9              |
| Project 1            | 11             |
| Project 2            | 11             |
| Project 3            | 11             |
| Project 4            | 11             |

- No formal curve; grades based on performance groupings
- No extra credit
- Grades on **MyUCLA** are authoritative
- Ignore “Bruin Learn” estimated grades

## 4. Academic Integrity

### 4.1 Allowed
- Studying with peers
- External research (must cite sources)
- Reading related documentation/txts

### 4.2 Prohibited
- Sharing or copying any code or answers
- Using AI tools (e.g., ChatGPT) for assignments or exams
- Copying solutions online, even partial
- Letting others see or use your code

> Violations are submitted to the UCLA Dean of Students and result in serious penalties.

## 5. Operating System Overview

### 5.1 Course Relevance
- Builds on lower-level CS classes
- Serves as foundation for upper-level topics like:
  - Networking
  - Distributed Systems
  - Databases, Security, Fault-tolerance

### 5.2 What You'll Learn

| Concept               | Description                          |
|-----------------------|--------------------------------------|
| Process & Threads     | Program execution contexts           |
| Virtual Address Space | Abstracted memory model              |
| File                  | Persistent, abstracted data storage  |
| Synchronization       | Prevent inconsistency & races        |
| Locking & Deadlock    | Safe mutual exclusion mechanisms     |

### 5.3 Why Study Operating Systems?

#### 5.3.1 Bridging Hardware and Software
The OS provides an abstraction over hardware:
- RAM → abstract memory
- CPU → multiple abstract processes
- Disk → files
- Devices → usable interfaces

#### 5.3.2 Enables High-Level Applications
An OS makes possible:
- Web browsing
- Games
- Document editing
- Audio/video communications

#### 5.3.3 Hidden Complexity
The OS abstracts:
- Coordination of concurrent activities
- Multi-application fairness
- Device sharing and safety
- Efficient hardware utilization

#### 5.3.4 Real-World Exposure
- You will interface with OSes as developers and users
- Skills gained: debugging, configuration, programming using OS APIs

#### 5.3.5 Computer Science Heritage
Many foundational CS ideas (like process scheduling, concurrency, memory allocation, security) were first explored within OS design.

#### 5.3.6 Required Understanding for CS Degrees
Having knowledge of OS is essential to professing: “I understand how computers work.”

## 6. Key Concepts Covered

### 6.1 What is an Operating System?
- **Type**: System software (not application software)
- **Purpose**: 
  - Abstract hardware complexity
  - Provide safe, fair, and efficient resource sharing
- **Role**:
  - Manages hardware
  - Sits between hardware and applications
  - Offers stable abstractions (processes, files, etc.)

### 6.2 Hardware vs. Software Capabilities

| Hardware Component   | Operations                                |
|----------------------|-------------------------------------------|
| RAM                  | Read/write 64-bit words                   |
| Flash Drive          | Read/write 4K–16K blocks                  |
| Mouse                | Read X & Y movement; detect clicks        |
| CPU                  | Execute low-level instructions            |
| Screen               | Write pixels, erase screen                |

> Applications need more than these capabilities. OS bridges the gap.

### 6.3 What Does an OS Provide?

| Feature                    | Purpose                                |
|----------------------------|----------------------------------------|
| Abstractions               | Files, processes, threads              |
| Safety                     | Users can't harm system or each other  |
| Resource Management        | Scheduling & fair usage                |
| Efficient Execution        | Performance optimization               |
| Familiar Interfaces        | API/library to hardware mapping        |

### 6.4 OS Internals and Structure

#### 6.4.1 OS as a Set of Services

- Mediates access to hardware
- Runs privileged instructions
- Must be trusted to be stable and correct

#### 6.4.2 OS Interfaces

| Interface Type          | Purpose                                   |
|--------------------------|-------------------------------------------|
| Application Binary Interface (ABI) | Binary-level API access         |
| System Call Interface    | Application requests OS services         |

#### 6.4.3 Diagram Summary

```
Applications
    |
Libraries (ABIs)
    |
System Calls
    |
Operating System (Privileged Mode)
    |
Hardware (RAM, CPU, Devices)
```

### 6.5 Platform Basics

- **ISA (Instruction Set Architecture):** Defines operations CPU supports
- **Privileged vs Standard Mode:**  
  - Privileged (OS-only): allows hardware control  
  - Standard (User): cannot execute privileged instructions

- **Platform = ISA + Devices**
  OS must be compatible with the whole platform, not just CPU.

### 6.6 Device Drivers and Configuration

#### 6.6.1 Device Drivers
- Code that enables OS to interact with device (e.g., screen, mouse)

#### 6.6.2 Hardware Discovery
- Modern OS perform automatic detection of hardware using self-identifying buses

#### 6.6.3 Binary Configuration
- OS installation includes:
  - Detecting devices
  - Loading appropriate drivers (plug-in model)
  - Allocating RAM appropriately

### 6.7 OS Code Complexity

| Item                               | Implementation Location     |
|------------------------------------|-----------------------------|
| Needs privileged instructions      | OS                          |
| Uses OS-protected data structures  | OS                          |
| Simple computations (e.g., string ops) | Libraries               |

- OS code must be minimal yet complete for performance and reliability.

### 6.8 Instruction Set Architectures (ISA)

| Feature               | Description                                                |
|-----------------------|------------------------------------------------------------|
| Word Width            | 8, 16, 32, or 64 bit CPU architectures                     |
| Power Efficiency      | Some ISAs target low-energy applications                   |
| Family Lineage        | Compatibility and continuity across hardware generations   |
| ISA Examples          | x86/Intel, ARM, RISC-V                                     |

### 6.9 Binary Distribution
- Users receive OS in binary, ISA-specific form
- Open-source (Linux) and proprietary (Windows) follow this model
- Avoid disruptions by maintaining **stable interfaces**

## 7. Popular Modern Operating Systems

| OS         | Primary Use                             |
|------------|------------------------------------------|
| Windows    | Dominant in user desktops, laptops      |
| MacOS      | All Apple hardware (desktop to watches) |
| Linux      | Preferred for servers and cloud systems |

## Summary

This lecture introduced the structure, administration, and purpose of CS111, emphasizing the importance of operating systems in modern computing. It covered the architecture of operating systems, the hardware-software interface, system-level abstractions (e.g., processes, files), and the rationale for studying OS as foundational to computer science. It explained the course logistics, grading, and the heavy workload expected. Further, it explored real-world applications and the OS’s role in mediating between raw hardware capabilities and user-friendly computing experiences.