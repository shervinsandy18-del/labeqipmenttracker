=================================================
   EquipTrack - Equipment Management Dashboard
   College Project | C++ Backend + Web Frontend
=================================================

-------------------------------------------------
1. PROJECT OVERVIEW
-------------------------------------------------

EquipTrack is an equipment management dashboard web application.
It helps track equipment availability, issued items, and maintenance status.

The backend is written entirely in C++ using POSIX sockets.
The frontend uses HTML, CSS, JavaScript, and Chart.js.

Architecture:

   [Browser / Frontend]  <--HTTP-->  [C++ Backend Server]  <--File I/O-->  [CSV Data]

   Frontend (HTML/CSS/JS)  --> sends HTTP requests
   C++ Backend (main.cpp)  --> handles requests, serves files and API
   CSV File (equipment.csv)--> stores all equipment data


-------------------------------------------------
2. FOLDER STRUCTURE
-------------------------------------------------

EquipTrack/
├── backend/
│   ├── main.cpp           (C++ HTTP server - entry point)
│   ├── equipment.h        (Header file - structure & function declarations)
│   └── equipment.cpp      (Equipment functions implementation)
├── frontend/
│   ├── index.html         (Dashboard HTML page)
│   ├── style.css          (All styling and design)
│   └── script.js          (Frontend logic - API calls, charts, tables)
├── data/
│   └── equipment.csv      (Equipment data storage)
├── README.txt             (This file)
└── start.bat              (Windows batch file to compile and run)


-------------------------------------------------
3. REQUIREMENTS
-------------------------------------------------

Software needed:

   1. C++ Compiler (GCC / MinGW)
      - Windows: Install MinGW (https://www.mingw-w64.org/)
      - Or install via MSYS2: https://www.msys2.org/

   2. VS Code (any version)
      - Download from: https://code.visualstudio.com/

   3. A modern web browser
      - Chrome, Firefox, Edge, etc.

No external C++ libraries are needed.
The server uses only standard C++ and system sockets.
Chart.js is loaded from a CDN (internet required for charts only).


-------------------------------------------------
4. SETUP AND EXECUTION (Windows)
-------------------------------------------------

Step 1: Open the EquipTrack folder in VS Code.

Step 2: Open a terminal in VS Code (Terminal > New Terminal).

Step 3: Run the start.bat file:

   start.bat

   OR manually compile and run:

   cd backend
   g++ -std=c++17 main.cpp equipment.cpp -lws2_32 -o server.exe
   cd ..
   backend\server.exe

Step 4: Open your browser and go to:

   http://localhost:8080

Step 5: You should see the EquipTrack dashboard.

Step 6: Press Ctrl+C in the terminal to stop the server.


-------------------------------------------------
5. SETUP AND EXECUTION (Linux / Mac)
-------------------------------------------------

Step 1: Open a terminal in the EquipTrack folder.

Step 2: Compile the server:

   cd backend
   g++ -std=c++17 main.cpp equipment.cpp -o server
   cd ..

Step 3: Run the server:

   ./backend/server

Step 4: Open your browser and go to:

   http://localhost:8080


-------------------------------------------------
6. API ENDPOINTS
-------------------------------------------------

The C++ backend provides the following REST API:

   GET  /api/equipment          Returns all equipment (JSON array)
   GET  /api/equipment?id=N     Returns single equipment by ID
   GET  /api/stats              Returns dashboard statistics
   POST /api/equipment          Adds new equipment (JSON body)
   PUT  /api/status             Updates equipment status (JSON body)

   GET  /                       Serves index.html
   GET  /style.css              Serves the CSS file
   GET  /script.js              Serves the JavaScript file


-------------------------------------------------
7. CSV DATA FORMAT
-------------------------------------------------

File: data/equipment.csv

Format:
   id,name,category,quantity,location,status

Example:
   1,Laptops,Computers,25,Lab Room A,Available
   2,Desktop Computers,Computers,15,Lab Room B,Issued

Status values: Available, Issued, Maintenance
Categories: Computers, Accessories, Office Equipment, Laboratory Equipment, Audio/Visual


-------------------------------------------------
8. FEATURES
-------------------------------------------------

   1. Dashboard with 4 statistic cards (Total, Available, Issued, Maintenance)
   2. Doughnut chart showing equipment status distribution
   3. Bar chart showing equipment counts by category
   4. Full inventory table with search functionality
   5. Add new equipment (modal form)
   6. View equipment details (modal)
   7. Update equipment status (modal)
   8. Auto-refresh data every 30 seconds
   9. Responsive design (works on mobile and desktop)
  10. Data persists in CSV file


-------------------------------------------------
9. PROJECT EXPLANATION (FOR VIVA)
-------------------------------------------------

Q: How does the project work?

A: The C++ backend creates an HTTP server using POSIX sockets.
   It listens on port 8080 for incoming connections.
   When a browser sends a request, the server:
     - Serves HTML/CSS/JS files for the frontend
     - Handles API requests (/api/equipment, /api/stats, etc.)
     - Reads and writes equipment data from a CSV file
   The frontend uses JavaScript fetch() to call the API.
   Chart.js renders graphs from the data received.


Q: How is data stored?

A: All equipment data is stored in a CSV file (data/equipment.csv).
   The C++ backend reads this file into a vector of Equipment structs
   at startup. When data changes (add/update), it saves back to the file.


Q: How do the charts work?

A: The frontend fetches statistics from /api/stats.
   Chart.js takes the data and draws:
     - A doughnut chart for status (Available/Issued/Maintenance)
     - A bar chart for categories (Computers/Accessories/etc.)


Q: How does search work?

A: The frontend filters the equipment array in JavaScript.
   It checks if the search text matches name, category, location, or status.


-------------------------------------------------
10. MAIN C++ CODE EXPLANATION
-------------------------------------------------

main.cpp:
   - Creates a TCP socket using socket()
   - Binds it to port 8080 using bind()
   - Listens for connections using listen()
   - In a loop, accepts connections with accept()
   - Reads the HTTP request from the client
   - Parses the request (method, path, body)
   - Routes API requests to handler functions
   - Serves static files (HTML/CSS/JS) for other paths
   - Sends the HTTP response back to the browser
   - Closes the client connection

equipment.cpp:
   - loadEquipment(): Reads CSV file, splits by commas, creates Equipment structs
   - saveEquipment(): Writes all equipment back to CSV
   - addEquipment(): Adds a new item with auto-incremented ID
   - updateStatus(): Changes the status field of an item by ID
   - equipmentToJson(): Converts equipment list to JSON string
   - getStatsJson(): Calculates totals and category counts, returns JSON

equipment.h:
   - Defines the Equipment struct (id, name, category, quantity, location, status)
   - Declares all functions used by the backend


-------------------------------------------------
11. VIVA QUESTIONS AND ANSWERS
-------------------------------------------------

Q1: What is EquipTrack?
A1: It is a web-based equipment management system that tracks
    equipment availability, issued items, and maintenance status.
    It has a C++ backend and an HTML/CSS/JS frontend.

Q2: Why did you use C++ for the backend?
A2: C++ provides low-level socket programming capabilities and
    is efficient. It demonstrates understanding of network
    programming and HTTP protocol handling.

Q3: How does the C++ server work?
A3: It uses POSIX sockets. The server creates a socket, binds to
    port 8080, listens for connections, and processes HTTP requests
    in an infinite loop. Each request is parsed and routed.

Q4: What HTTP methods does your server support?
A4: GET (fetch data), POST (add equipment), PUT (update status),
    and OPTIONS (for CORS preflight requests).

Q5: How is data persisted?
A5: Data is stored in a CSV file. The server reads it at startup
    and writes back whenever data changes.

Q6: What is the Equipment structure?
A6: It contains: id (int), name (string), category (string),
    quantity (int), location (string), and status (string).

Q7: How do the frontend and backend communicate?
A7: The frontend uses JavaScript fetch() API to send HTTP requests
    to the C++ backend. The backend responds with JSON data.
    The frontend then renders the data in tables and charts.

Q8: What is CORS and why is it needed?
A8: CORS (Cross-Origin Resource Sharing) allows the browser to
    make requests to the server. The backend sends
    "Access-Control-Allow-Origin: *" headers to enable this.

Q9: How do the charts update?
A10: When equipment is added or status is changed, the frontend
     re-fetches data from /api/stats and updates the Chart.js
     instances with the new data.

Q10: What is a socket?
A10: A socket is an endpoint for communication between two
     programs over a network. We use TCP sockets for reliable
     HTTP communication.

Q11: How does the server parse HTTP requests?
A11: The server reads the raw request string, extracts the method
     (GET/POST/PUT) and path from the first line, and extracts
     the JSON body (for POST/PUT) from after the blank line.

Q12: What is the difference between GET and POST?
A12: GET is used to retrieve data from the server. POST is used
     to send new data to the server (like adding equipment).

Q13: What libraries does your C++ code use?
A13: Only standard C++ libraries: iostream, fstream, sstream,
     string, vector, and system socket headers. No external
     frameworks are used.

Q14: How does the search functionality work?
A14: The frontend filters the equipment array in JavaScript.
     It checks if the search text matches any field (name,
     category, location, or status) using string includes().

Q15: What is the role of the CSV file?
A15: The CSV file acts as the database. Each line is a record
     with comma-separated fields. It is simple and easy to
     understand for a college project.


-------------------------------------------------
12. ARCHITECTURE DIAGRAM
-------------------------------------------------

     +-------------------+        HTTP         +-------------------+
     |                   |  <-------------->  |                   |
     |    FRONTEND       |   GET /api/stats   |    C++ BACKEND    |
     |                   |   GET /api/equip   |                   |
     |  - index.html     |   POST /api/equip  |  - main.cpp       |
     |  - style.css      |   PUT /api/status  |  - equipment.cpp  |
     |  - script.js      |                     |  - equipment.h    |
     |  - Chart.js       |   Static Files     |                   |
     |                   |  GET /, *.css, *.js|  Socket Server    |
     +-------------------+                     +-------------------+
                                                           |
                                                      File I/O
                                                           |
                                                           v
                                                   +-------------------+
                                                   |    DATA LAYER     |
                                                   |                   |
                                                   |  equipment.csv    |
                                                   |                   |
                                                   +-------------------+

     Browser (port 8080) <--> C++ Server (port 8080) <--> CSV File

     Data Flow:
     1. User opens browser -> http://localhost:8080
     2. C++ server sends index.html, style.css, script.js
     3. JavaScript calls /api/equipment and /api/stats
     4. C++ server reads CSV, returns JSON
     5. JavaScript renders tables and charts
     6. User adds equipment -> POST /api/equipment
     7. C++ server updates CSV, returns new record
     8. JavaScript refreshes dashboard


-------------------------------------------------
13. TROUBLESHOOTING
-------------------------------------------------

Problem: "Port 8080 already in use"
Solution: Change PORT in main.cpp to 8081 or 3000, and
          update the URL in your browser accordingly.

Problem: "g++ not found"
Solution: Install MinGW and add it to your system PATH.
          Verify with: g++ --version

Problem: "Charts not showing"
Solution: Ensure you have an internet connection (Chart.js loads
          from a CDN). Or download Chart.js and link locally.

Problem: "Cannot connect to server"
Solution: Make sure the C++ server is running. Check the terminal
          for errors. Verify the port number.

Problem: "Data not saving"
Solution: Check that data/equipment.csv exists and is writable.
          The server must be started from the backend/ directory.


-------------------------------------------------
14. SAMPLE DATA
-------------------------------------------------

The project includes 15 sample equipment records:

   Laptops (25), Desktop Computers (15), Projectors (8),
   Keyboards (30), Mouse (35), Printers (5), Monitors (20),
   Cameras (6), Laboratory Equipment (12), Scanners (3),
   Headphones (18), Webcams (10), Speakers (7),
   Microscopes (4), Whiteboards (12)

Total: 210 items across 5 categories and 3 statuses.


=================================================
   End of README
=================================================
