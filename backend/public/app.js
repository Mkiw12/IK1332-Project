async function updateDashboard() {
  try {
    const floorRes = await fetch('/elevator/floor');
    const floorData = await floorRes.json();

    const patternRes = await fetch('/elevator/pattern');
    const patternData = await patternRes.json();

    document.getElementById('floor').textContent = floorData.currentFloor;
    document.getElementById('pattern').textContent = patternData.pattern.join(' → ');
  } catch (err) {
    console.error(err);
    document.getElementById('floor').textContent = '?';
    document.getElementById('pattern').textContent = '-';
  }
}

// Poll every second
setInterval(updateDashboard, 1000);
updateDashboard();
