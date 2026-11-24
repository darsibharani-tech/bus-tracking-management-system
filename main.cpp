/**
 * ============================================================================
 * Project Title : Bus Tracking and Management Application
 * Course        : CSE 202 – Object Oriented Programming with C++
 * Academic Year : 2026 - 2027
 * Institution   : Department of Computer Science and Engineering,
 *                 SRM University - AP, Andhra Pradesh
 * 
 * Authors:
 *   1. Selvasiva S  (AP25122230011)
 *   2. Sarathi R    (AP25122230040)
 *   3. D.S.Bharani  (AP25110100027)
 * 
 * Faculty Mentor: Ms. Pithani Ashwini
 * Vice Chancellor: Prof. Ch. Satish Kumar
 * ============================================================================
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <algorithm>
#include <stdexcept>

namespace TransitCore {

    // ==========================================
    // Custom Exception Hierarchy
    // ==========================================
    class TransitException : public std::runtime_error {
    public:
        explicit TransitException(const std::string& message)
            : std::runtime_error(message) {}
    };

    class DuplicateBusException : public TransitException {
    public:
        explicit DuplicateBusException(const std::string& reg)
            : TransitException("Error: Bus with registration " + reg + " is already registered in the depot!") {}
    };

    class BusNotFoundException : public TransitException {
    public:
        explicit BusNotFoundException(const std::string& reg)
            : TransitException("Error: Bus registration " + reg + " was not found in the depot database!") {}
    };

    class RouteNotFoundException : public TransitException {
    public:
        explicit RouteNotFoundException(int routeId)
            : TransitException("Error: Route ID " + std::to_string(routeId) + " does not exist in master routes!") {}
    };

    class DuplicateAllocationException : public TransitException {
    public:
        explicit DuplicateAllocationException(const std::string& reg)
            : TransitException("Error: Bus " + reg + " is already assigned to an active route schedule today!") {}
    };

    // ==========================================
    // Geographical Location and Haversine Distance
    // ==========================================
    struct GeoLocation {
        double latitude;
        double longitude;

        GeoLocation(double lat = 0.0, double lon = 0.0) : latitude(lat), longitude(lon) {}

        // Haversine formula to calculate great-circle distance in kilometers
        double distanceTo(const GeoLocation& other) const {
            const double EARTH_RADIUS_KM = 6371.0;
            const double PI = 3.14159265358979323846;

            double lat1Rad = latitude * (PI / 180.0);
            double lon1Rad = longitude * (PI / 180.0);
            double lat2Rad = other.latitude * (PI / 180.0);
            double lon2Rad = other.longitude * (PI / 180.0);

            double dLat = lat2Rad - lat1Rad;
            double dLon = lon2Rad - lon1Rad;

            double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
                       std::cos(lat1Rad) * std::cos(lat2Rad) *
                       std::sin(dLon / 2.0) * std::sin(dLon / 2.0);

            double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
            return EARTH_RADIUS_KM * c;
        }
    };

    // ==========================================
    // Abstract Base Class: Vehicle (Demonstrating Abstraction & Polymorphism)
    // ==========================================
    class Vehicle {
    protected:
        std::string registrationNumber;
        std::string model;
        int capacity;
        bool active;

    public:
        Vehicle(const std::string& reg, const std::string& mdl, int cap)
            : registrationNumber(reg), model(mdl), capacity(cap), active(true) {}

        virtual ~Vehicle() = default; // Virtual destructor

        virtual void displayDetails() const = 0; // Pure virtual function

        std::string getRegistrationNumber() const { return registrationNumber; }
        std::string getModel() const { return model; }
        int getCapacity() const { return capacity; }
        bool isActive() const { return active; }
        void setActive(bool status) { active = status; }
    };

    // ==========================================
    // Derived Class: Bus
    // ==========================================
    class Bus : public Vehicle {
    private:
        GeoLocation currentLocation;
        std::string lastUpdatedStop;

        static int totalBusesRegistered;

    public:
        Bus() : Vehicle("UNKNOWN", "Standard", 50), currentLocation(0.0, 0.0), lastUpdatedStop("Depot Yard") {
            totalBusesRegistered++;
        }

        Bus(const std::string& reg, const std::string& mdl, int cap = 50)
            : Vehicle(reg, mdl, cap), currentLocation(0.0, 0.0), lastUpdatedStop("Depot Yard") {
            totalBusesRegistered++;
        }

        GeoLocation getCurrentLocation() const { return currentLocation; }
        std::string getLastUpdatedStop() const { return lastUpdatedStop; }

        void setLocation(const GeoLocation& loc, const std::string& stopName) {
            currentLocation = loc;
            lastUpdatedStop = stopName;
        }

        void displayDetails() const override {
            std::cout << std::left 
                      << std::setw(16) << registrationNumber
                      << std::setw(20) << model
                      << std::setw(10) << capacity
                      << std::setw(30) << lastUpdatedStop
                      << std::fixed << std::setprecision(4)
                      << "(" << currentLocation.latitude << ", " << currentLocation.longitude << ")"
                      << "\n";
        }

        static int getTotalBuses() { return totalBusesRegistered; }

        // Operator Overloading for stream output
        friend std::ostream& operator<<(std::ostream& os, const Bus& bus) {
            os << "Bus [" << bus.registrationNumber << "] " << bus.model 
               << " (Cap: " << bus.capacity << ") @ " << bus.lastUpdatedStop;
            return os;
        }
    };

    int Bus::totalBusesRegistered = 0;

    // ==========================================
    // Class: Stop
    // ==========================================
    class Stop {
    private:
        int stopId;
        std::string stopName;
        GeoLocation location;
        double distanceFromStartKm;

    public:
        Stop(int id, const std::string& name, double lat, double lon, double dist)
            : stopId(id), stopName(name), location(lat, lon), distanceFromStartKm(dist) {}

        int getStopId() const { return stopId; }
        std::string getStopName() const { return stopName; }
        GeoLocation getLocation() const { return location; }
        double getDistance() const { return distanceFromStartKm; }
    };

    // ==========================================
    // Class: Route
    // ==========================================
    class Route {
    private:
        int routeId;
        std::string source;
        std::string destination;
        std::vector<Stop> stops;
        double totalDistanceKm;

    public:
        Route() : routeId(0), source(""), destination(""), totalDistanceKm(0.0) {}
        Route(int id, const std::string& src, const std::string& dst)
            : routeId(id), source(src), destination(dst), totalDistanceKm(0.0) {}

        void addStop(const Stop& stop) {
            stops.push_back(stop);
            if (stops.size() > 1) {
                totalDistanceKm = stops.back().getDistance();
            }
        }

        int getRouteId() const { return routeId; }
        std::string getSource() const { return source; }
        std::string getDestination() const { return destination; }
        const std::vector<Stop>& getStops() const { return stops; }
        double getTotalDistance() const { return totalDistanceKm; }
    };

    // ==========================================
    // Class: RouteAssignment (Schedule Allocation)
    // ==========================================
    class RouteAssignment {
    private:
        std::string assignmentId;
        std::string busRegNumber;
        int routeId;
        std::string driverId;
        std::string conductorId;
        std::string departureTime;
        std::string allocationDate;
        bool active;

        static int totalAssignments;

    public:
        RouteAssignment() : assignmentId(""), busRegNumber(""), routeId(0), active(false) {}

        RouteAssignment(const std::string& busReg, int rId, const std::string& drv,
                        const std::string& cnd, const std::string& time, const std::string& date)
            : busRegNumber(busReg), routeId(rId), driverId(drv), conductorId(cnd),
              departureTime(time), allocationDate(date), active(true) {
            totalAssignments++;
            assignmentId = "ASGN-" + std::to_string(totalAssignments);
        }

        std::string getAssignmentId() const { return assignmentId; }
        std::string getBusRegNumber() const { return busRegNumber; }
        int getRouteId() const { return routeId; }
        std::string getDriverId() const { return driverId; }
        std::string getConductorId() const { return conductorId; }
        std::string getDepartureTime() const { return departureTime; }
        std::string getAllocationDate() const { return allocationDate; }
        bool isActive() const { return active; }

        static int getTotalAssignments() { return totalAssignments; }

        // Friend function for clean tabular printing
        friend void printAssignmentRow(const RouteAssignment& asgn, const Route& route, const Bus& bus);
    };

    int RouteAssignment::totalAssignments = 0;

    void printAssignmentRow(const RouteAssignment& asgn, const Route& route, const Bus& bus) {
        std::cout << std::left 
                  << std::setw(10) << asgn.assignmentId
                  << std::setw(15) << asgn.busRegNumber
                  << std::setw(8)  << asgn.routeId
                  << std::setw(28) << (route.getSource() + " -> " + route.getDestination())
                  << std::setw(12) << asgn.driverId
                  << std::setw(12) << asgn.conductorId
                  << std::setw(12) << asgn.departureTime
                  << std::setw(25) << bus.getLastUpdatedStop()
                  << "\n";
    }

    // ==========================================
    // Generic Template Utility Function
    // ==========================================
    template<typename T>
    void printHeader(const std::string& title) {
        std::cout << "\n========================================================================================================\n";
        std::cout << "                                  " << title << "\n";
        std::cout << "========================================================================================================\n";
    }

    // ==========================================
    // Depot Application Controller Class
    // ==========================================
    class BusDepotApplication {
    private:
        std::map<std::string, Bus> registeredBuses;
        std::map<int, Route> routes;
        std::vector<RouteAssignment> assignments;
        double proximityThresholdKm;

        static int proximityAlertEvents;

    public:
        BusDepotApplication(double thresholdKm = 0.75) 
            : proximityThresholdKm(thresholdKm) {
            initializeMasterData();
        }

        void initializeMasterData() {
            // Seed Tri-Campus Master Routes (SRM AP, VIT-AP, Amrita Vishwa Vidyapeetham)
            Route r101(101, "SRM AP Campus", "VIT-AP University");
            r101.addStop(Stop(1, "SRM AP Main Campus Gate",       16.5362, 80.5058, 0.0));
            r101.addStop(Stop(2, "Neerukonda Village Junction",   16.5280, 80.5080, 1.8));
            r101.addStop(Stop(3, "Sakhamuru Central Circle",       16.5180, 80.5020, 3.5));
            r101.addStop(Stop(4, "Inavolu Village Cross",         16.5060, 80.4960, 5.2));
            r101.addStop(Stop(5, "VIT-AP University Main Entrance", 16.4975, 80.4990, 7.2));
            routes[101] = r101;

            Route r202(202, "SRM AP Campus", "Amrita University");
            r202.addStop(Stop(1, "SRM AP Main Campus Gate",          16.5362, 80.5058, 0.0));
            r202.addStop(Stop(2, "Neerukonda South Cross",           16.5250, 80.5180, 2.2));
            r202.addStop(Stop(3, "Kuragallu Junction",               16.5050, 80.5360, 4.8));
            r202.addStop(Stop(4, "Navuluru Circle",                  16.4880, 80.5480, 7.1));
            r202.addStop(Stop(5, "Amrita Vishwa Vidyapeetham Campus", 16.4780, 80.5520, 8.8));
            routes[202] = r202;

            Route r303(303, "VIT-AP University", "Amrita University");
            r303.addStop(Stop(1, "VIT-AP University Main Entrance", 16.4975, 80.4990, 0.0));
            r303.addStop(Stop(2, "Inavolu East Link Road",          16.4980, 80.5150, 2.1));
            r303.addStop(Stop(3, "Kuragallu Bypass",                16.4920, 80.5350, 4.4));
            r303.addStop(Stop(4, "Amrita Vishwa Vidyapeetham Campus", 16.4780, 80.5520, 6.5));
            routes[303] = r303;

            // Seed Initial Campus Fleet Shuttles
            registeredBuses["AP16-TJ-1001"] = Bus("AP16-TJ-1001", "Ashok Leyland Viking AC", 55);
            registeredBuses["AP16-TJ-1002"] = Bus("AP16-TJ-1002", "Tata Starbus Ultra", 48);
            registeredBuses["AP16-TJ-1003"] = Bus("AP16-TJ-1003", "Eicher Skyline Pro", 45);

            // Pre-seed allocations for immediate campus tracking and proximity demonstration
            RouteAssignment a1("AP16-TJ-1001", 101, "DRV-801", "CND-401", "08:15 AM", "2026-11-26");
            RouteAssignment a2("AP16-TJ-1002", 101, "DRV-802", "CND-402", "08:25 AM", "2026-11-26");
            RouteAssignment a3("AP16-TJ-1003", 202, "DRV-803", "CND-403", "08:30 AM", "2026-11-26");

            assignments.push_back(a1);
            assignments.push_back(a2);
            assignments.push_back(a3);

            // Set initial locations: Bus 1 & Bus 2 on Route 101 are close near Sakhamuru Circle (simulating bunching)
            registeredBuses["AP16-TJ-1001"].setLocation(GeoLocation(16.5180, 80.5020), "Sakhamuru Central Circle");
            registeredBuses["AP16-TJ-1002"].setLocation(GeoLocation(16.5205, 80.5035), "Sakhamuru North Link");
            registeredBuses["AP16-TJ-1003"].setLocation(GeoLocation(16.5050, 80.5360), "Kuragallu Junction");
        }

        // 1. Register a bus
        void registerBus(const std::string& reg, const std::string& model, int capacity = 50) {
            if (registeredBuses.find(reg) != registeredBuses.end()) {
                throw DuplicateBusException(reg);
            }
            registeredBuses[reg] = Bus(reg, model, capacity);
            std::cout << "\n[SUCCESS] Bus " << reg << " (" << model << ", Capacity: " << capacity 
                      << ") registered successfully!\n";
        }

        // 2. Assign bus to a route
        void assignBusToRoute(const std::string& busReg, int routeId, const std::string& driverId,
                              const std::string& conductorId, const std::string& depTime, const std::string& date) {
            if (registeredBuses.find(busReg) == registeredBuses.end()) {
                throw BusNotFoundException(busReg);
            }
            if (routes.find(routeId) == routes.end()) {
                throw RouteNotFoundException(routeId);
            }
            for (const auto& asgn : assignments) {
                if (asgn.getBusRegNumber() == busReg && asgn.isActive() && asgn.getAllocationDate() == date) {
                    throw DuplicateAllocationException(busReg);
                }
            }

            RouteAssignment newAsgn(busReg, routeId, driverId, conductorId, depTime, date);
            assignments.push_back(newAsgn);

            // Set bus to starting stop of route
            const Route& r = routes[routeId];
            if (!r.getStops().empty()) {
                registeredBuses[busReg].setLocation(r.getStops()[0].getLocation(), r.getStops()[0].getStopName());
            }

            std::cout << "\n[SUCCESS] Bus " << busReg << " successfully assigned to Route " << routeId 
                      << " (" << r.getSource() << " -> " << r.getDestination() << ") at " << depTime << "\n";
        }

        // 3. Display registered buses
        void displayRegisteredBuses() const {
            printHeader<Bus>("DEPOT REGISTERED BUS FLEET");
            std::cout << std::left 
                      << std::setw(16) << "Registration"
                      << std::setw(20) << "Model"
                      << std::setw(10) << "Capacity"
                      << std::setw(30) << "Current Location"
                      << "Coordinates" << "\n";
            std::cout << "--------------------------------------------------------------------------------------------------------\n";
            for (const auto& pair : registeredBuses) {
                pair.second.displayDetails();
            }
            std::cout << "========================================================================================================\n";
            std::cout << "Total Registered Buses: " << registeredBuses.size() << "\n";
        }

        // 4. Display route assignments
        void displayRouteAssignments() const {
            printHeader<RouteAssignment>("ACTIVE DAILY ROUTE ASSIGNMENTS");
            std::cout << std::left 
                      << std::setw(10) << "Asgn ID"
                      << std::setw(15) << "Bus Reg"
                      << std::setw(8)  << "Route"
                      << std::setw(28) << "Corridor"
                      << std::setw(12) << "Driver ID"
                      << std::setw(12) << "Conductor"
                      << std::setw(12) << "Dep. Time"
                      << std::setw(25) << "Current Location"
                      << "\n";
            std::cout << "--------------------------------------------------------------------------------------------------------\n";
            for (const auto& asgn : assignments) {
                auto rIt = routes.find(asgn.getRouteId());
                auto bIt = registeredBuses.find(asgn.getBusRegNumber());
                if (rIt != routes.end() && bIt != registeredBuses.end()) {
                    printAssignmentRow(asgn, rIt->second, bIt->second);
                }
            }
            std::cout << "========================================================================================================\n";
            std::cout << "Total Active Daily Assignments: " << assignments.size() << "\n";
        }

        // 5. Enter/update sample bus location
        void updateBusLocation(const std::string& busReg, const std::string& stopName, double lat, double lon) {
            if (registeredBuses.find(busReg) == registeredBuses.end()) {
                throw BusNotFoundException(busReg);
            }
            registeredBuses[busReg].setLocation(GeoLocation(lat, lon), stopName);
            std::cout << "\n[SUCCESS] Location updated for bus " << busReg << " -> " 
                      << stopName << " (" << lat << ", " << lon << ")\n";
        }

        // 6. Check same-route bus proximity (Core Alert Engine)
        void checkSameRouteProximity() {
            printHeader<Route>("REAL-TIME SAME-ROUTE BUS PROXIMITY AUDIT");
            
            std::map<int, std::vector<std::string>> busesOnRoute;
            for (const auto& asgn : assignments) {
                if (asgn.isActive()) {
                    busesOnRoute[asgn.getRouteId()].push_back(asgn.getBusRegNumber());
                }
            }

            bool alertFound = false;

            for (const auto& pair : busesOnRoute) {
                int routeId = pair.first;
                const auto& busList = pair.second;
                if (busList.size() < 2) continue;

                for (size_t i = 0; i < busList.size(); ++i) {
                    for (size_t j = i + 1; j < busList.size(); ++j) {
                        const Bus& b1 = registeredBuses[busList[i]];
                        const Bus& b2 = registeredBuses[busList[j]];

                        double distance = b1.getCurrentLocation().distanceTo(b2.getCurrentLocation());

                        if (distance <= proximityThresholdKm) {
                            alertFound = true;
                            proximityAlertEvents++;

                            std::cout << "\n***********************************************************************************\n";
                            std::cout << " [!] ALERT: SAME-ROUTE BUS PROXIMITY CONFLICT DETECTED (ROUTE " << routeId << ")!\n";
                            std::cout << "     Alert Event  : #" << proximityAlertEvents << " - BUS BUNCHING HAZARD\n";
                            std::cout << "     Vehicle 1    : " << b1.getRegistrationNumber() << " (At: " << b1.getLastUpdatedStop() << ")\n";
                            std::cout << "     Vehicle 2    : " << b2.getRegistrationNumber() << " (At: " << b2.getLastUpdatedStop() << ")\n";
                            std::cout << "     Separation   : " << std::fixed << std::setprecision(3) << distance << " km "
                                      << "(Safety Threshold: " << proximityThresholdKm << " km)\n";
                            std::cout << "     Advisory     : Two buses on Route " << routeId 
                                      << " are passing very closely! Depot Manager advisory:\n"
                                      << "                    Regulate trailing vehicle speed to maintain scheduled interval.\n";
                            std::cout << "***********************************************************************************\n";
                        }
                    }
                }
            }

            if (!alertFound) {
                std::cout << "[STATUS] All active buses on the same routes are operating with safe headway distances (> " 
                          << proximityThresholdKm << " km).\n";
            }
            std::cout << "========================================================================================================\n";
        }

        // Export data to CSV file on exit
        void exportToCSV(const std::string& filename = "route_assignments_report.csv") const {
            std::ofstream file(filename);
            if (!file.is_open()) return;

            file << "AssignmentID,BusRegistration,RouteID,DriverID,ConductorID,DepartureTime,Date,CurrentLocation\n";
            for (const auto& asgn : assignments) {
                auto bIt = registeredBuses.find(asgn.getBusRegNumber());
                std::string loc = (bIt != registeredBuses.end()) ? bIt->second.getLastUpdatedStop() : "Unknown";
                file << asgn.getAssignmentId() << ","
                     << asgn.getBusRegNumber() << ","
                     << asgn.getRouteId() << ","
                     << asgn.getDriverId() << ","
                     << asgn.getConductorId() << ","
                     << "\"" << asgn.getDepartureTime() << "\","
                     << asgn.getAllocationDate() << ","
                     << "\"" << loc << "\"\n";
            }
            file.close();
            std::cout << "\n[DATA SAVED] Route assignments successfully exported to " << filename << "\n";
        }
    };

    int BusDepotApplication::proximityAlertEvents = 0;

} // namespace TransitCore

// ==========================================
// Main Function with the Exact Menu Structure
// ==========================================
int main() {
    using namespace TransitCore;

    BusDepotApplication app(0.75); // 750 meters bunching threshold

    int choice = -1;

    while (choice != 0) {
        std::cout << "\n===== BUS TRACKING AND MANAGEMENT =====\n";
        std::cout << "1. Register a bus\n";
        std::cout << "2. Assign bus to a route\n";
        std::cout << "3. Display registered buses\n";
        std::cout << "4. Display route assignments\n";
        std::cout << "5. Enter/update sample bus location\n";
        std::cout << "6. Check same-route bus proximity\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter your choice: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "\n[ERROR] Invalid input! Please enter a number between 0 and 6.\n";
            continue;
        }

        try {
            switch (choice) {
                case 1: {
                    std::string reg, model;
                    int cap = 50;
                    std::cout << "Enter bus registration number: ";
                    std::cin >> reg;
                    std::cout << "Enter bus model: ";
                    std::cin.ignore();
                    std::getline(std::cin, model);
                    std::cout << "Enter seating capacity (default 50): ";
                    if (!(std::cin >> cap)) {
                        cap = 50;
                        std::cin.clear();
                        std::cin.ignore(10000, '\n');
                    }
                    app.registerBus(reg, model, cap);
                    break;
                }
                case 2: {
                    std::string reg, drv, cnd, time, date;
                    int rId;
                    std::cout << "Enter bus registration number: ";
                    std::cin >> reg;
                    std::cout << "Enter route ID (e.g., 101 for Vijayawada, 202 for Guntur): ";
                    std::cin >> rId;
                    std::cout << "Enter driver ID: ";
                    std::cin >> drv;
                    std::cout << "Enter conductor ID: ";
                    std::cin >> cnd;
                    std::cin.ignore();
                    std::cout << "Enter departure time (e.g. 08:30 AM): ";
                    std::getline(std::cin, time);
                    std::cout << "Enter date (e.g. 2026-11-26): ";
                    std::cin >> date;

                    app.assignBusToRoute(reg, rId, drv, cnd, time, date);
                    break;
                }
                case 3:
                    app.displayRegisteredBuses();
                    break;
                case 4:
                    app.displayRouteAssignments();
                    break;
                case 5: {
                    std::string reg, stopName;
                    double lat, lon;
                    std::cout << "Enter bus registration number: ";
                    std::cin >> reg;
                    std::cin.ignore();
                    std::cout << "Enter sample stop / landmark name: ";
                    std::getline(std::cin, stopName);
                    std::cout << "Enter latitude (e.g. 16.4440): ";
                    std::cin >> lat;
                    std::cout << "Enter longitude (e.g. 80.5615): ";
                    std::cin >> lon;

                    app.updateBusLocation(reg, stopName, lat, lon);
                    break;
                }
                case 6:
                    app.checkSameRouteProximity();
                    break;
                case 0:
                    app.exportToCSV("route_assignments_report.csv");
                    std::cout << "\nExiting Bus Tracking and Management System. Have a nice day!\n";
                    break;
                default:
                    std::cout << "\n[ERROR] Unknown option! Please select from 0 to 6.\n";
                    break;
            }
        } catch (const TransitException& ex) {
            std::cout << "\n[EXCEPTION CAUGHT]: " << ex.what() << "\n";
        } catch (const std::exception& ex) {
            std::cout << "\n[SYSTEM ERROR]: " << ex.what() << "\n";
        }
    }

    return 0;
}
