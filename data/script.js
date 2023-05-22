// platformio run --target uploadfs --environment esp32doit-devkit-v1
function update() {
  updateValues();
}
setInterval(update, 1000);

function updateValues() {
  fetch("/values")
    .then((response) => response.text())
    .then((values) => {
      // Update the content of the values element
      const valuesElement = document.getElementById("values");
      valuesElement.innerHTML = values;
    })
    .catch((error) => {
      console.error("Error:", error);
    });
}

function toggleBuzzer() {
  fetch("/toggle-buzzer");
}

function toggleRelay() {
  fetch("/toggle-relay");
}
