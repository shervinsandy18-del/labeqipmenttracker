// equipment.h
// Header file for equipment management functions.
// Declares the Equipment structure and functions used by the backend.

#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <string>
#include <vector>

// Structure to hold one equipment record.
// Keeping it simple so it is easy to explain during viva.
struct Equipment {
    int id;
    std::string name;
    std::string category;
    int quantity;
    std::string location;
    std::string status; // "Available", "Issued", or "Maintenance"
};

// Load all equipment records from the CSV file.
std::vector<Equipment> loadEquipment(const std::string& filename);

// Save all equipment records back to the CSV file.
void saveEquipment(const std::string& filename, const std::vector<Equipment>& list);

// Add a new equipment record and return its id.
int addEquipment(std::vector<Equipment>& list, const Equipment& item);

// Update the status of an equipment record by id.
// Returns true if found and updated, false otherwise.
bool updateStatus(std::vector<Equipment>& list, int id, const std::string& newStatus);

// Convert the full equipment list to a JSON string.
std::string equipmentToJson(const std::vector<Equipment>& list);

// Convert a single equipment record to a JSON string.
std::string singleEquipmentToJson(const Equipment& item);

// Calculate and return dashboard statistics as JSON.
// Includes total, available, issued, and maintenance counts,
// plus category-wise counts for the bar chart.
std::string getStatsJson(const std::vector<Equipment>& list);

#endif
