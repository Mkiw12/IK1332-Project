async function updateDashboard() {
  const res = await fetch('/elevator/floor');
  const data = await res.json();

  document.getElementById('floor').textContent = data.currentFloor;
  //document.getElementById('direction').textContent = data.direction;
  //document.getElementById('state').textContent = data.motionState;
}

// Poll every second
setInterval(updateDashboard, 1000);
updateDashboard();

