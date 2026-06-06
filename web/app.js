const canvas = document.getElementById('mapCanvas');
const ctx = canvas.getContext('2d');
const nodeList = document.getElementById('nodeList');
const resultBox = document.getElementById('resultBox');
const algoSelect = document.getElementById('algoSelect');

// stores all points the user clicks on the canvas
let locations = [];

// ── DRAW EVERYTHING ON CANVAS ──────────────────────────────────
function draw(routePath) {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // draw grid dots for visual reference
    ctx.fillStyle = '#2a2a4a';
    for(let x = 0; x < canvas.width; x += 40)
        for(let y = 0; y < canvas.height; y += 40)
            ctx.fillRect(x, y, 1.5, 1.5);

    // draw route lines if path exists
    if(routePath && routePath.length > 1) {
        ctx.strokeStyle = '#7c6af7';
        ctx.lineWidth = 2.5;
        ctx.setLineDash([]);
        ctx.beginPath();
        ctx.moveTo(locations[routePath[0]].x, locations[routePath[0]].y);
        for(let i = 1; i < routePath.length; i++) {
            ctx.lineTo(locations[routePath[i]].x, locations[routePath[i]].y);
        }
        ctx.stroke();

        // draw arrow direction on each route segment
        for(let i = 0; i < routePath.length - 1; i++) {
            let from = locations[routePath[i]];
            let to = locations[routePath[i+1]];
            let midX = (from.x + to.x) / 2;
            let midY = (from.y + to.y) / 2;
            let angle = Math.atan2(to.y - from.y, to.x - from.x);

            ctx.save();
            ctx.translate(midX, midY);
            ctx.rotate(angle);
            ctx.fillStyle = '#7c6af7';
            ctx.beginPath();
            ctx.moveTo(8, 0);
            ctx.lineTo(-4, -4);
            ctx.lineTo(-4, 4);
            ctx.closePath();
            ctx.fill();
            ctx.restore();
        }
    }

    // draw all location nodes
    locations.forEach((loc, i) => {
        // outer glow circle
        ctx.beginPath();
        ctx.arc(loc.x, loc.y, 18, 0, Math.PI * 2);
        ctx.fillStyle = i === 0 ? 'rgba(247,194,106,0.15)' : 'rgba(124,106,247,0.15)';
        ctx.fill();

        // main circle
        ctx.beginPath();
        ctx.arc(loc.x, loc.y, 10, 0, Math.PI * 2);
        ctx.fillStyle = i === 0 ? '#f7c26a' : '#7c6af7';
        ctx.fill();

        // node number inside circle
        ctx.fillStyle = '#fff';
        ctx.font = 'bold 10px Segoe UI';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(i, loc.x, loc.y);

        // node name below circle
        ctx.fillStyle = i === 0 ? '#f7c26a' : '#fff';
        ctx.font = '11px Segoe UI';
        ctx.fillText(loc.name, loc.x, loc.y + 22);
    });
}

// ── HANDLE CANVAS CLICK ────────────────────────────────────────
canvas.addEventListener('click', function(e) {
    const rect = canvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    // first click is always the warehouse
    const name = locations.length === 0 ? 'Warehouse' : 'Stop ' + locations.length;
    locations.push({ name, x, y });

    // update the sidebar node list
    updateNodeList();
    draw(null);
});

// ── UPDATE SIDEBAR NODE LIST ───────────────────────────────────
function updateNodeList() {
    nodeList.innerHTML = '';
    if(locations.length === 0) {
        nodeList.innerHTML = '<div style="color:#555;">No locations yet...</div>';
        return;
    }
    locations.forEach((loc, i) => {
        let div = document.createElement('div');
        div.style.color = i === 0 ? '#f7c26a' : '#ccc';
        div.textContent = i + '. ' + loc.name +
            ' (' + Math.round(loc.x) + ', ' + Math.round(loc.y) + ')';
        nodeList.appendChild(div);
    });
}

// ── RUN BUTTON ─────────────────────────────────────────────────
document.getElementById('runBtn').addEventListener('click', async function() {
    if(locations.length < 2) {
        resultBox.innerHTML = '<span style="color:#f76a6a;">Please add at least 2 locations.</span>';
        return;
    }

    const algo = algoSelect.value;
    resultBox.innerHTML = 'Running ' + algo + '...';

    // build input.txt content
    // format: ALGORITHM numNodes
    //         name x y  (one per line)
    let inputContent = algo + ' ' + locations.length + '\n';
    locations.forEach((loc, i) => {
        let safeName = loc.name.replace(/ /g, '_');
        inputContent += safeName + ' ' + loc.x.toFixed(1) + ' ' + loc.y.toFixed(1) + '\n';
    });

    try {
        // send to Flask server which runs C++
        const response = await fetch('http://localhost:5000/run', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ input: inputContent })
        });

        const data = await response.json();

        if(data.error) {
            resultBox.innerHTML = '<span style="color:#f76a6a;">Error: ' + data.error + '</span>';
            return;
        }

        // draw the route on canvas
        draw(data.path);

        // show results in sidebar
        resultBox.innerHTML =
            '<span class="highlight">Algorithm:</span> ' + data.algorithm + '<br>' +
            '<span class="highlight">Total Distance:</span> ' + data.totalDistance.toFixed(2) + ' units<br>' +
            '<span class="highlight">Stops:</span> ' + data.path.length + '<br><br>' +
            '<span class="highlight">Route:</span><br>' +
            data.path.map(i => locations[i].name).join(' → ');

    } catch(err) {
        resultBox.innerHTML = '<span style="color:#f76a6a;">Could not connect to server. Is bridge.py running?</span>';
    }
});

// ── CLEAR BUTTON ───────────────────────────────────────────────
document.getElementById('clearBtn').addEventListener('click', function() {
    locations = [];
    updateNodeList();
    draw(null);
    resultBox.innerHTML = 'Run an algorithm to see results here.';
});

// draw empty canvas on load
draw(null);

