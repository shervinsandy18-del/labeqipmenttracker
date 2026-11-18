// main.cpp
// EquipTrack C++ Backend Server
//
// A lightweight HTTP server built with POSIX sockets.
// Serves the frontend files and provides a REST API for equipment management.
//
// Endpoints:
//   GET  /api/equipment          - list all equipment
//   GET  /api/equipment?id=N     - get single equipment by id
//   GET  /api/stats              - get dashboard statistics
//   POST /api/equipment          - add new equipment (JSON body)
//   PUT  /api/status             - update equipment status (JSON body)
//   GET  /<static file>          - serve frontend files from ../frontend/
//
// Compilation (Linux/Mac):
//   g++ -std=c++17 main.cpp equipment.cpp -o server
//
// Compilation (Windows with MinGW):
//   g++ -std=c++17 main.cpp equipment.cpp -lws2_32 -o server.exe

#include "equipment.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>

// Platform-specific socket headers
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #endif

// Server configuration
#define PORT 8080
#define BUFFER_SIZE 4096
#define BACKEND_DIR "backend"
#define DATA_FILE "../data/equipment.csv"
#define FRONTEND_DIR "../frontend"

// Global equipment list — loaded once at startup.
std::vector<Equipment> equipmentList;

// Helper: read entire file into a string.
// Returns empty string if file cannot be opened.
static std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Helper: get file extension to determine MIME type.
static std::string getMimeType(const std::string& path) {
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") return "text/html";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css")  return "text/css";
    if (path.size() >= 3 && path.substr(path.size() - 3) == ".js")   return "application/javascript";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".json") return "application/json";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".png")  return "image/png";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".svg")  return "image/svg+xml";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".ico")  return "image/x-icon";
    return "application/octet-stream";
}

// Helper: extract a query parameter value from a URL path.
// Example: "/api/equipment?id=5" -> returns "5" for key "id".
static std::string getQueryParam(const std::string& path, const std::string& key) {
    std::string search = key + "=";
    size_t pos = path.find(search);
    if (pos == std::string::npos) return "";
    pos += search.size();
    std::string value;
    while (pos < path.size() && path[pos] != '&' && path[pos] != ' ') {
        value += path[pos];
        pos++;
    }
    return value;
}

// Helper: extract the JSON body from an HTTP request.
// The body starts after the blank line (\r\n\r\n).
static std::string extractBody(const std::string& request) {
    size_t pos = request.find("\r\n\r\n");
    if (pos == std::string::npos) return "";
    return request.substr(pos + 4);
}

// Helper: extract a string field from a JSON string by key.
// Simple parser — works for flat JSON objects with string values.
// Example: {"name":"Laptop","category":"Computers"} -> "Laptop" for key "name"
static std::string getJsonField(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos += search.size();
    std::string value;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            value += json[pos + 1];
            pos += 2;
        } else {
            value += json[pos];
            pos++;
        }
    }
    return value;
}

// Helper: extract an integer field from a JSON string by key.
// Example: {"quantity":25} -> 25
static int getJsonInt(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return 0;
    pos += search.size();
    std::string numStr;
    while (pos < json.size() && (json[pos] >= '0' && json[pos] <= '9')) {
        numStr += json[pos];
        pos++;
    }
    if (numStr.empty()) return 0;
    return std::stoi(numStr);
}

// Helper: build a complete HTTP response string.
static std::string buildResponse(int statusCode, const std::string& contentType,
                                  const std::string& body) {
    std::string statusText;
    switch (statusCode) {
        case 200: statusText = "OK"; break;
        case 201: statusText = "Created"; break;
        case 400: statusText = "Bad Request"; break;
        case 404: statusText = "Not Found"; break;
        case 500: statusText = "Internal Server Error"; break;
        default:  statusText = "OK"; break;
    }

    std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + statusText + "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: GET, POST, PUT, OPTIONS, DELETE\r\n";
    response += "Access-Control-Allow-Headers: Content-Type\r\n";
    response += "Connection: close\r\n";
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    response += "\r\n";
    response += body;
    return response;
}

// Process an API request and return the response string.
static std::string handleApiRequest(const std::string& method,
                                     const std::string& path,
                                     const std::string& body) {
    // GET /api/equipment - return all equipment or single by id
    if (method == "GET" && path.find("/api/equipment") == 0) {
        std::string idParam = getQueryParam(path, "id");
        if (!idParam.empty()) {
            int id = std::stoi(idParam);
            for (size_t i = 0; i < equipmentList.size(); i++) {
                if (equipmentList[i].id == id) {
                    return buildResponse(200, "application/json",
                                         singleEquipmentToJson(equipmentList[i]));
                }
            }
            return buildResponse(404, "application/json", "{\"error\":\"Not found\"}");
        }
        return buildResponse(200, "application/json", equipmentToJson(equipmentList));
    }

    // GET /api/stats - return dashboard statistics
    if (method == "GET" && path == "/api/stats") {
        return buildResponse(200, "application/json", getStatsJson(equipmentList));
    }

    // POST /api/equipment - add new equipment
    if (method == "POST" && path == "/api/equipment") {
        Equipment item;
        item.id = 0;
        item.name = getJsonField(body, "name");
        item.category = getJsonField(body, "category");
        item.quantity = getJsonInt(body, "quantity");
        item.location = getJsonField(body, "location");
        item.status = getJsonField(body, "status");

        if (item.name.empty()) {
            return buildResponse(400, "application/json",
                                 "{\"error\":\"Name is required\"}");
        }

        int newId = addEquipment(equipmentList, item);
        saveEquipment(DATA_FILE, equipmentList);

        // Find and return the newly added item
        for (size_t i = 0; i < equipmentList.size(); i++) {
            if (equipmentList[i].id == newId) {
                return buildResponse(201, "application/json",
                                     singleEquipmentToJson(equipmentList[i]));
            }
        }
        return buildResponse(500, "application/json", "{\"error\":\"Failed to add\"}");
    }

    // PUT /api/status - update equipment status
    if (method == "PUT" && path == "/api/status") {
        int id = getJsonInt(body, "id");
        std::string newStatus = getJsonField(body, "status");

        if (newStatus.empty()) {
            return buildResponse(400, "application/json",
                                 "{\"error\":\"Status is required\"}");
        }

        if (updateStatus(equipmentList, id, newStatus)) {
            saveEquipment(DATA_FILE, equipmentList);
            // Return the updated item
            for (size_t i = 0; i < equipmentList.size(); i++) {
                if (equipmentList[i].id == id) {
                    return buildResponse(200, "application/json",
                                         singleEquipmentToJson(equipmentList[i]));
                }
            }
        }
        return buildResponse(404, "application/json", "{\"error\":\"Not found\"}");
    }

    // Handle CORS preflight
    if (method == "OPTIONS") {
        return buildResponse(200, "text/plain", "");
    }

    return buildResponse(404, "application/json", "{\"error\":\"Unknown endpoint\"}");
}

// Serve a static file from the frontend directory.
static std::string serveStaticFile(const std::string& path) {
    std::string filePath = FRONTEND_DIR + path;

    // If root, serve index.html
    if (path == "/" || path.empty()) {
        filePath = std::string(FRONTEND_DIR) + "/index.html";
    }

    // Prevent directory traversal attacks (check only the user-supplied path)
    if (path.find("..") != std::string::npos) {
        return buildResponse(400, "text/plain", "Bad Request");
    }

    std::string content = readFile(filePath);
    if (content.empty()) {
        return buildResponse(404, "text/plain", "404 Not Found");
    }

    return buildResponse(200, getMimeType(filePath), content);
}

int main() {
    // Load equipment data from CSV
    equipmentList = loadEquipment(DATA_FILE);
    std::cout << "============================================\n";
    std::cout << "  EquipTrack Backend Server\n";
    std::cout << "============================================\n";
    std::cout << "Loaded " << equipmentList.size() << " equipment records.\n";

#ifdef _WIN32
    // Initialize Winsock on Windows
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: Winsock initialization failed.\n";
        return 1;
    }
#endif

    // Create the server socket
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        std::cerr << "Error: Cannot create socket.\n";
        return 1;
    }

    // Allow port reuse (prevents "address already in use" error)
    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    // Bind socket to port
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Error: Cannot bind to port " << PORT << ".\n";
#ifdef _WIN32
        closesocket(serverFd);
        WSACleanup();
#else
        close(serverFd);
#endif
        return 1;
    }

    // Start listening for connections
    if (listen(serverFd, 10) < 0) {
        std::cerr << "Error: Cannot listen on port.\n";
        return 1;
    }

    std::cout << "Server running on http://localhost:" << PORT << "\n";
    std::cout << "Open your browser and visit the URL above.\n";
    std::cout << "Press Ctrl+C to stop the server.\n";
    std::cout << "============================================\n\n";

    // Main server loop — accept and handle connections
    while (true) {
        socklen_t addrLen = sizeof(address);
        int clientFd = accept(serverFd, (struct sockaddr*)&address, &addrLen);

        if (clientFd < 0) {
            std::cerr << "Warning: Failed to accept connection.\n";
            continue;
        }

        // Read the HTTP request
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        int bytesRead = recv(clientFd, buffer, BUFFER_SIZE - 1, 0);

        if (bytesRead <= 0) {
#ifdef _WIN32
            closesocket(clientFd);
#else
            close(clientFd);
#endif
            continue;
        }

        std::string request(buffer, bytesRead);

        // Parse the request line: METHOD PATH HTTP/1.1
        std::istringstream reqStream(request);
        std::string method, path, version;
        reqStream >> method >> path >> version;

        std::string response;

        // Route the request
        if (path.find("/api/") == 0) {
            // API request
            std::string body = extractBody(request);
            response = handleApiRequest(method, path, body);
        } else {
            // Static file request
            response = serveStaticFile(path);
        }

        // Send the response
        send(clientFd, response.c_str(), (int)response.size(), 0);

        // Close the client connection
#ifdef _WIN32
        closesocket(clientFd);
#else
        close(clientFd);
#endif
    }

    // Cleanup (unreachable in normal operation, but good practice)
#ifdef _WIN32
    closesocket(serverFd);
    WSACleanup();
#else
    close(serverFd);
#endif
    return 0;
}
