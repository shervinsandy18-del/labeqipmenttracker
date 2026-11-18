// script.js
// EquipTrack Frontend Logic
//
// Handles:
//   - Fetching equipment data from the C++ backend
//   - Rendering dashboard statistics
//   - Drawing Chart.js graphs (doughnut + bar)
//   - Populating the inventory table
//   - Search/filter functionality
//   - Add equipment, view details, update status
//
// All data comes from the backend API at /api/...

// Global variables to hold chart instances and data
let statusChart = null;
let categoryChart = null;
let allEquipment = [];
let currentStatusId = 0;

// ===== Navigation =====

// Show a specific section (dashboard or inventory)
function showSection(section) {
    // Hide all sections
    document.querySelectorAll('.content-section').forEach(s => {
        s.style.display = 'none';
    });

    // Show the selected section
    document.getElementById(section + '-section').style.display = 'block';

    // Update nav items
    document.querySelectorAll('.nav-item').forEach(n => n.classList.remove('active'));

    // Update title
    const titles = {
        dashboard: ['Dashboard', 'Overview of equipment status and analytics'],
        inventory: ['Inventory', 'Complete equipment listing with search and management']
    };

    if (titles[section]) {
        document.getElementById('pageTitle').textContent = titles[section][0];
        document.getElementById('pageSubtitle').textContent = titles[section][1];
    }

    // Mark the correct nav item as active
    if (section === 'dashboard') {
        document.querySelectorAll('.nav-item')[0].classList.add('active');
    } else if (section === 'inventory') {
        document.querySelectorAll('.nav-item')[1].classList.add('active');
    }

    // Close sidebar on mobile
    closeSidebarOnMobile();
}

// Toggle sidebar (for mobile)
function toggleSidebar() {
    document.getElementById('sidebar').classList.toggle('open');
}

function closeSidebarOnMobile() {
    if (window.innerWidth <= 768) {
        document.getElementById('sidebar').classList.remove('open');
    }
}

// ===== Data Fetching =====

// Fetch all equipment from the backend
async function fetchEquipment() {
    try {
        const response = await fetch('/api/equipment');
        if (!response.ok) throw new Error('Failed to fetch');
        const data = await response.json();
        allEquipment = data;
        return data;
    } catch (error) {
        console.error('Error fetching equipment:', error);
        showToast('Cannot connect to server. Is the C++ backend running?', 'error');
        return [];
    }
}

// Fetch dashboard statistics from the backend
async function fetchStats() {
    try {
        const response = await fetch('/api/stats');
        if (!response.ok) throw new Error('Failed to fetch stats');
        return await response.json();
    } catch (error) {
        console.error('Error fetching stats:', error);
        return null;
    }
}

// ===== Dashboard Rendering =====

// Update the four stat cards
function updateStatCards(stats) {
    document.getElementById('statTotal').textContent = stats.total;
    document.getElementById('statAvailable').textContent = stats.available;
    document.getElementById('statIssued').textContent = stats.issued;
    document.getElementById('statMaintenance').textContent = stats.maintenance;
}

// Create or update the doughnut chart for equipment status
function renderStatusChart(stats) {
    const ctx = document.getElementById('statusChart').getContext('2d');

    const data = {
        labels: ['Available', 'Issued', 'Maintenance'],
        datasets: [{
            data: [stats.available, stats.issued, stats.maintenance],
            backgroundColor: ['#10b981', '#3b82f6', '#f59e0b'],
            borderWidth: 0,
            hoverOffset: 8
        }]
    };

    const options = {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
            legend: {
                position: 'bottom',
                labels: {
                    padding: 20,
                    font: { size: 13, family: 'Inter' },
                    usePointStyle: true,
                    pointStyle: 'circle'
                }
            }
        },
        cutout: '65%'
    };

    if (statusChart) {
        statusChart.data = data;
        statusChart.update();
    } else {
        statusChart = new Chart(ctx, { type: 'doughnut', data, options });
    }
}

// Create or update the bar chart for equipment categories
function renderCategoryChart(stats) {
    const ctx = document.getElementById('categoryChart').getContext('2d');

    const data = {
        labels: ['Computers', 'Accessories', 'Office Equip.', 'Lab Equip.', 'Audio/Visual'],
        datasets: [{
            label: 'Quantity',
            data: [
                stats.categories.computers,
                stats.categories.accessories,
                stats.categories.officeEquipment,
                stats.categories.laboratoryEquipment,
                stats.categories.audioVisual
            ],
            backgroundColor: ['#6366f1', '#3b82f6', '#8b5cf6', '#10b981', '#f59e0b'],
            borderRadius: 8,
            borderSkipped: false
        }]
    };

    const options = {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
            legend: { display: false }
        },
        scales: {
            y: {
                beginAtZero: true,
                grid: { color: '#f1f5f9' },
                ticks: { font: { size: 12, family: 'Inter' }, color: '#94a3b8' }
            },
            x: {
                grid: { display: false },
                ticks: { font: { size: 12, family: 'Inter' }, color: '#64748b' }
            }
        }
    };

    if (categoryChart) {
        categoryChart.data = data;
        categoryChart.update();
    } else {
        categoryChart = new Chart(ctx, { type: 'bar', data, options });
    }
}

// ===== Table Rendering =====

// Get CSS class for status badge
function statusBadgeClass(status) {
    if (status === 'Available') return 'available';
    if (status === 'Issued') return 'issued';
    if (status === 'Maintenance') return 'maintenance';
    return 'available';
}

// Render the full inventory table
function renderInventoryTable(equipment) {
    const tbody = document.getElementById('inventoryTableBody');

    if (equipment.length === 0) {
        tbody.innerHTML = '<tr><td colspan="7" class="loading-row">No equipment found.</td></tr>';
        return;
    }

    let html = '';
    for (let i = 0; i < equipment.length; i++) {
        const e = equipment[i];
        html += '<tr>';
        html += '<td>#' + e.id + '</td>';
        html += '<td style="font-weight:600;">' + escapeHtml(e.name) + '</td>';
        html += '<td>' + escapeHtml(e.category) + '</td>';
        html += '<td>' + e.quantity + '</td>';
        html += '<td>' + escapeHtml(e.location) + '</td>';
        html += '<td><span class="badge ' + statusBadgeClass(e.status) + '">' + e.status + '</span></td>';
        html += '<td style="white-space:nowrap;">';
        html += '<button class="action-btn" onclick="viewEquipment(' + e.id + ')">View</button>';
        html += '<button class="action-btn" onclick="openStatusModal(' + e.id + ',\'' + escapeHtml(e.name) + '\',\'' + e.status + '\')">Status</button>';
        html += '</td>';
        html += '</tr>';
    }
    tbody.innerHTML = html;
}

// Render the recent equipment table (first 5 items)
function renderRecentTable(equipment) {
    const tbody = document.getElementById('recentTableBody');
    const recent = equipment.slice(0, 5);

    if (recent.length === 0) {
        tbody.innerHTML = '<tr><td colspan="6" class="loading-row">No equipment found.</td></tr>';
        return;
    }

    let html = '';
    for (let i = 0; i < recent.length; i++) {
        const e = recent[i];
        html += '<tr>';
        html += '<td>#' + e.id + '</td>';
        html += '<td style="font-weight:600;">' + escapeHtml(e.name) + '</td>';
        html += '<td>' + escapeHtml(e.category) + '</td>';
        html += '<td>' + e.quantity + '</td>';
        html += '<td>' + escapeHtml(e.location) + '</td>';
        html += '<td><span class="badge ' + statusBadgeClass(e.status) + '">' + e.status + '</span></td>';
        html += '</tr>';
    }
    tbody.innerHTML = html;
}

// Escape HTML to prevent XSS
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// ===== Search / Filter =====

// Filter the inventory table based on the search input
function filterTable() {
    const query = document.getElementById('searchInput').value.toLowerCase().trim();

    if (query === '') {
        renderInventoryTable(allEquipment);
        return;
    }

    const filtered = allEquipment.filter(function(e) {
        return e.name.toLowerCase().includes(query) ||
               e.category.toLowerCase().includes(query) ||
               e.location.toLowerCase().includes(query) ||
               e.status.toLowerCase().includes(query);
    });

    renderInventoryTable(filtered);
}

// ===== Modals =====

// Show the add equipment modal
function addEquipmentModal() {
    document.getElementById('formName').value = '';
    document.getElementById('formCategory').value = 'Computers';
    document.getElementById('formQty').value = '';
    document.getElementById('formLocation').value = '';
    document.getElementById('formStatus').value = 'Available';
    document.getElementById('addModal').style.display = 'flex';
    closeSidebarOnMobile();
}

// Close any modal
function closeModal(id) {
    document.getElementById(id).style.display = 'none';
}

// Submit new equipment to the backend
async function submitEquipment() {
    const name = document.getElementById('formName').value.trim();
    const category = document.getElementById('formCategory').value;
    const quantity = parseInt(document.getElementById('formQty').value);
    const location = document.getElementById('formLocation').value.trim();
    const status = document.getElementById('formStatus').value;

    // Basic validation
    if (!name) {
        showToast('Please enter an equipment name.', 'error');
        return;
    }
    if (!quantity || quantity < 1) {
        showToast('Please enter a valid quantity.', 'error');
        return;
    }
    if (!location) {
        showToast('Please enter a location.', 'error');
        return;
    }

    try {
        const response = await fetch('/api/equipment', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                name: name,
                category: category,
                quantity: quantity,
                location: location,
                status: status
            })
        });

        if (!response.ok) throw new Error('Failed to add');

        showToast('Equipment added successfully!', 'success');
        closeModal('addModal');

        // Refresh all data
        await refreshAll();
    } catch (error) {
        console.error('Error adding equipment:', error);
        showToast('Failed to add equipment. Check server connection.', 'error');
    }
}

// View a single equipment's details
async function viewEquipment(id) {
    try {
        const response = await fetch('/api/equipment?id=' + id);
        if (!response.ok) throw new Error('Failed to fetch');
        const e = await response.json();

        const body = document.getElementById('viewModalBody');
        body.innerHTML = `
            <div class="detail-row">
                <span class="detail-label">ID</span>
                <span class="detail-value">#${e.id}</span>
            </div>
            <div class="detail-row">
                <span class="detail-label">Name</span>
                <span class="detail-value">${escapeHtml(e.name)}</span>
            </div>
            <div class="detail-row">
                <span class="detail-label">Category</span>
                <span class="detail-value">${escapeHtml(e.category)}</span>
            </div>
            <div class="detail-row">
                <span class="detail-label">Quantity</span>
                <span class="detail-value">${e.quantity}</span>
            </div>
            <div class="detail-row">
                <span class="detail-label">Location</span>
                <span class="detail-value">${escapeHtml(e.location)}</span>
            </div>
            <div class="detail-row">
                <span class="detail-label">Status</span>
                <span class="detail-value"><span class="badge ${statusBadgeClass(e.status)}">${e.status}</span></span>
            </div>
        `;

        document.getElementById('viewModal').style.display = 'flex';
    } catch (error) {
        console.error('Error viewing equipment:', error);
        showToast('Failed to load equipment details.', 'error');
    }
}

// Open the status update modal
function openStatusModal(id, name, currentStatus) {
    currentStatusId = id;
    document.getElementById('statusEquipName').textContent = name + ' (ID #' + id + ')';
    document.getElementById('statusSelect').value = currentStatus;
    document.getElementById('statusModal').style.display = 'flex';
}

// Submit status update to the backend
async function submitStatusUpdate() {
    const newStatus = document.getElementById('statusSelect').value;

    try {
        const response = await fetch('/api/status', {
            method: 'PUT',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                id: currentStatusId,
                status: newStatus
            })
        });

        if (!response.ok) throw new Error('Failed to update');

        showToast('Status updated successfully!', 'success');
        closeModal('statusModal');

        // Refresh all data
        await refreshAll();
    } catch (error) {
        console.error('Error updating status:', error);
        showToast('Failed to update status. Check server connection.', 'error');
    }
}

// ===== Toast Notifications =====

// Show a toast message
function showToast(message, type) {
    const toast = document.getElementById('toast');
    toast.textContent = message;
    toast.className = 'toast show ' + (type || '');

    setTimeout(function() {
        toast.className = 'toast ' + (type || '');
    }, 3000);
}

// ===== Main Refresh Function =====

// Refresh all data: stats, charts, and tables
async function refreshAll() {
    // Fetch stats and equipment in parallel
    const [stats, equipment] = await Promise.all([fetchStats(), fetchEquipment()]);

    if (stats) {
        updateStatCards(stats);
        renderStatusChart(stats);
        renderCategoryChart(stats);
    }

    if (equipment.length > 0) {
        renderInventoryTable(equipment);
        renderRecentTable(equipment);
        // Re-apply any active search filter
        filterTable();
    }
}

// ===== Initialize on Page Load =====

// Load everything when the page is ready
window.addEventListener('DOMContentLoaded', async function() {
    await refreshAll();

    // Auto-refresh data every 30 seconds
    setInterval(refreshAll, 30000);
});

// Close modal when clicking outside
document.addEventListener('click', function(e) {
    if (e.target.classList && e.target.classList.contains('modal-overlay')) {
        e.target.style.display = 'none';
    }
});
