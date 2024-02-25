async function getTemperature() {
  const result = await fetch("api/temperature");  
  const { temperature } = await result.json();
  const temp = Number(temperature.toFixed(2));
  console.log(temp);
  const el = document.getElementById("temperature-val");
  el.innerText = temp;
}

setInterval(getTemperature, 1000);

async function getDistance() {
  const result = await fetch("api/distance");  
  const { distance } = await result.json();
  const temp = Number(distance.toFixed(2));
  console.log(temp);
  const el = document.getElementById("distance");
  el.innerText = temp;
}

setInterval(getDistance, 2000);


let isLedOn = false;

async function toggleLed() {
  const el = document.getElementById("led-button");
  isLedOn = !isLedOn;
  fetch("api/led", { method: "POST", body: JSON.stringify({ isLedOn }) });
  if (isLedOn) {
    el.classList.add("led-on");
    el.classList.remove("led-off");
  } else {
    el.classList.add("led-off");
    el.classList.remove("led-on");
  }
}