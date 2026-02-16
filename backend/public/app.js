async function updateDashboard() {
 try {
    const res = await fetch('/elevator/dashboard');
    const data = await res.json();

    document.getElementById('floor').textContent = data.currentFloor;
    document.getElementById('pattern').textContent = data.pattern.join(' → ');

    const container = document.getElementById('alarms');
    container.innerHTML = '';

    if (!data.alarm) {
      container.textContent = 'None';
    } else {
      container.innerHTML = `
        <strong>${data.alarm.type}</strong> - ${data.alarm.message}
        <button onclick="ackAlarm('${data.alarm.id}')">Acknowledge</button>
      `;
    }
      } catch (err) {
        console.error(err);
      }
}      

// Poll every 5 seconds
setInterval(updateDashboard, 5000);
updateDashboard();
