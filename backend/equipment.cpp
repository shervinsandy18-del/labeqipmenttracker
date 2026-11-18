// equipment.cpp
// Implements all equipment management functions.
// Uses simple file I/O, vectors, loops, and string operations.

#include "equipment.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Helper: split a CSV line by commas into fields.
// Simple loop-based approach — easy to explain in viva.
static std::vector<std::string> splitLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == ',') {
            fields.push_back(current);
            current = "";
        } else {
            current += line[i];
        }
    }
    fields.push_back(current); // last field
    return fields;
}

// Helper: trim whitespace and carriage returns from a string.
static std::string trim(const std::string& s) {
    std::string result = s;
    // Remove leading spaces
    while (!result.empty() && (result[0] == ' ' || result[0] == '\r' || result[0] == '\n')) {
        result.erase(0, 1);
    }
    // Remove trailing spaces
    while (!result.empty() && (result.back() == ' ' || result.back() == '\r' || result.back() == '\n')) {
        result.pop_back();
    }
    return result;
}

// Load equipment from CSV file.
// CSV format: id,name,category,quantity,location,status
std::vector<Equipment> loadEquipment(const std::string& filename) {
    std::vector<Equipment> list;
    std::ifstream file(filename);

    if (!file.is_open()) {
        return list; // empty list if file not found
    }

    std::string line;
    bool firstLine = true;

    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        // Skip the header row
        if (firstLine) {
            firstLine = false;
            // Check if this looks like a header
            if (line.find("name") != std::string::npos && line.find("category") != std::string::npos) {
                continue;
            }
        }

        std::vector<std::string> fields = splitLine(line);
        if (fields.size() >= 6) {
            Equipment e;
            e.id = std::stoi(trim(fields[0]));
            e.name = trim(fields[1]);
            e.category = trim(fields[2]);
            e.quantity = std::stoi(trim(fields[3]));
            e.location = trim(fields[4]);
            e.status = trim(fields[5]);
            list.push_back(e);
        }
    }

    file.close();
    return list;
}

// Save equipment list back to CSV.
void saveEquipment(const std::string& filename, const std::vector<Equipment>& list) {
    std::ofstream file(filename);

    // Write header
    file << "id,name,category,quantity,location,status\n";

    // Write each equipment record
    for (size_t i = 0; i < list.size(); i++) {
        file << list[i].id << ","
             << list[i].name << ","
             << list[i].category << ","
             << list[i].quantity << ","
             << list[i].location << ","
             << list[i].status << "\n";
    }

    file.close();
}

// Add new equipment. Auto-assigns the next available id.
int addEquipment(std::vector<Equipment>& list, const Equipment& item) {
    int maxId = 0;
    for (size_t i = 0; i < list.size(); i++) {
        if (list[i].id > maxId) {
            maxId = list[i].id;
        }
    }
    Equipment newItem = item;
    newItem.id = maxId + 1;
    list.push_back(newItem);
    return newItem.id;
}

// Update the status of an equipment item by id.
bool updateStatus(std::vector<Equipment>& list, int id, const std::string& newStatus) {
    for (size_t i = 0; i < list.size(); i++) {
        if (list[i].id == id) {
            list[i].status = newStatus;
            return true;
        }
    }
    return false;
}

// Helper: escape a string for JSON (handles quotes and backslashes).
static std::string escapeJson(const std::string& s) {
    std::string result;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '"') {
            result += "\\\"";
        } else if (s[i] == '\\') {
            result += "\\\\";
        } else if (s[i] == '\n' || s[i] == '\r') {
            result += " ";
        } else {
            result += s[i];
        }
    }
    return result;
}

// Convert a single equipment record to JSON.
std::string singleEquipmentToJson(const Equipment& item) {
    std::string json = "{";
    json += "\"id\":" + std::to_string(item.id) + ",";
    json += "\"name\":\"" + escapeJson(item.name) + "\",";
    json += "\"category\":\"" + escapeJson(item.category) + "\",";
    json += "\"quantity\":" + std::to_string(item.quantity) + ",";
    json += "\"location\":\"" + escapeJson(item.location) + "\",";
    json += "\"status\":\"" + escapeJson(item.status) + "\"";
    json += "}";
    return json;
}

// Convert the entire equipment list to a JSON array string.
std::string equipmentToJson(const std::vector<Equipment>& list) {
    std::string json = "[";
    for (size_t i = 0; i < list.size(); i++) {
        json += singleEquipmentToJson(list[i]);
        if (i < list.size() - 1) {
            json += ",";
        }
    }
    json += "]";
    return json;
}

// Calculate dashboard statistics and return as JSON.
std::string getStatsJson(const std::vector<Equipment>& list) {
    int total = 0;
    int available = 0;
    int issued = 0;
    int maintenance = 0;

    // Category counters for the bar chart
    int computers = 0;
    int accessories = 0;
    int officeEquip = 0;
    int labEquip = 0;
    int audioVisual = 0;

    for (size_t i = 0; i < list.size(); i++) {
        total += list[i].quantity;

        if (list[i].status == "Available") {
            available += list[i].quantity;
        } else if (list[i].status == "Issued") {
            issued += list[i].quantity;
        } else if (list[i].status == "Maintenance") {
            maintenance += list[i].quantity;
        }

        // Count by category (case-insensitive check on first word)
        std::string cat = list[i].category;
        if (cat == "Computers") {
            computers += list[i].quantity;
        } else if (cat == "Accessories") {
            accessories += list[i].quantity;
        } else if (cat == "Office Equipment") {
            officeEquip += list[i].quantity;
        } else if (cat == "Laboratory Equipment") {
            labEquip += list[i].quantity;
        } else if (cat == "Audio/Visual") {
            audioVisual += list[i].quantity;
        }
    }

    // Build JSON string
    std::string json = "{";
    json += "\"total\":" + std::to_string(total) + ",";
    json += "\"available\":" + std::to_string(available) + ",";
    json += "\"issued\":" + std::to_string(issued) + ",";
    json += "\"maintenance\":" + std::to_string(maintenance) + ",";
    json += "\"categories\":{";
    json += "\"computers\":" + std::to_string(computers) + ",";
    json += "\"accessories\":" + std::to_string(accessories) + ",";
    json += "\"officeEquipment\":" + std::to_string(officeEquip) + ",";
    json += "\"laboratoryEquipment\":" + std::to_string(labEquip) + ",";
    json += "\"audioVisual\":" + std::to_string(audioVisual);
    json += "}";
    json += "}";
    return json;
}
