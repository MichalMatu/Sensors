// platformio run --target uploadfs --environment esp32doit-devkit-v1

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
  fetch("/toggle-buzzer")
    .then((response) => response.text())
    .then((status) => {
      updateBuzzerStatus(status);
    })
    .catch((error) => {
      console.error("Error toggling buzzer:", error);
    });
}

function updateBuzzerStatus(status) {
  var statusContainer = document.getElementById("buzzerStatus");
  statusContainer.textContent = status;
}

function toggleRelay() {
  fetch("/toggle-relay")
    .then((response) => response.text())
    .then((status) => {
      updateRelayStatus(status);
    })
    .catch((error) => {
      console.error("Error toggling relay:", error);
    });
}

function updateRelayStatus(status) {
  var statusContainer = document.getElementById("relayStatus");
  statusContainer.textContent = status;
}

document.addEventListener("DOMContentLoaded", function () {
  updateBuzzerStatus("Loading...");
  updateRelayStatus("Loading...");
});

function update() {
  updateValues();
}

setInterval(update, 2000);
