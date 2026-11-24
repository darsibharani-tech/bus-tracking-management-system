# Bus Tracking and Management Application

An Object-Oriented C++ software and web dashboard built for regional bus depot managers to organize daily fleet routes, track vehicle coordinates, and prevent **bus bunching** through automated spatial proximity alerts.

---

## 📌 Project Overview
- **Institution:** SRM University – AP, Andhra Pradesh
- **Department:** Computer Science and Engineering
- **Course:** CSE 202 – Object Oriented Programming with C++
- **Academic Year:** 2026 – 2027
- **Faculty Mentor:** Ms. Pithani Ashwini
- **Vice Chancellor:** Prof. Ch. Satish Kumar

### Team Members
1. **Selvasiva S** — `AP25122230011`
2. **Sarathi R** — `AP25122230040`
3. **D.S.Bharani** — `AP25110100027`

---

## 🚀 Key Features
1. **Fleet Registration:** Add and manage depot buses (registration number, chassis model, capacity).
2. **Daily Route Assignment:** Assign vehicles to routes with driver ID, conductor ID, departure timings, and date.
3. **Conflict Validation:** Custom C++ exception handling prevents double-booking a bus for the same day.
4. **Live Telemetry & Tracking:** Inspect current stop progress and coordinates.
5. **Bus Bunching & Proximity Alerts:** Automatically applies the **Haversine formula** to calculate pairwise distances between buses on the same route. If separation drops below **0.75 km**, it triggers an instant hazard warning advising trailing drivers to slow down.
6. **Data Persistence:** Exports daily assignments to standard `route_assignments_report.csv`.
7. **Interactive Web Portal:** Complete responsive dashboard with OpenStreetMap tracking.

---

## 🛠️ Tech Stack & Concepts
- **Core Engine:** C++17 (Standard Template Library)
- **OOP Concepts:**
  - Abstract base class (`Vehicle`) with pure virtual functions
  - Single/hierarchical inheritance (`Bus : public Vehicle`)
  - Runtime polymorphism & Virtual Destructors
  - Operator overloading (`operator<<` for Bus streaming)
  - Custom Exception Hierarchy (`TransitException`, `DuplicateAllocationException`, etc.)
  - Encapsulation & Data Hiding
  - Generic Templates
- **Spatial Mathematics:** Spherical Haversine Great-Circle Distance Metric
- **Web Frontend:** HTML5, CSS3, Vanilla JavaScript, Leaflet.js

---

## 📂 Project Structure
```text
├── main.cpp                     # Complete C++17 Console Application
├── index.html                   # Interactive Satellite Web Dashboard & Dispatch Command
├── daily_route_schedule.csv     # Active Daily Operational Schedule Table
├── route_schedule_template.csv  # Blank Template for Office Staff Entry
├── Figure_2_Flowchart.png       # System Operational Flowchart
├── Bus_Tracking_Report.doc      # Academic Report (Word Format)
├── PROJECT_REPORT.md            # Full Academic Markdown Report
└── README.md                    # Project Documentation
```

---

## 💻 How to Compile and Run the C++ Application

### Using GCC / MinGW (g++):
```bash
# Compile with C++17 standard
g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o bus_tracking_app.exe

# Run the program
.\bus_tracking_app.exe
```

---

## 🌐 How to Open the Web Dashboard
Simply double-click `index.html` in your file explorer, or open it with Google Chrome, Microsoft Edge, or Firefox. No web server or node installation is needed!
