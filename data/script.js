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

function fetchBuzzerStatus() {
  fetch("/get-buzzer-status")
    .then((response) => response.text())
    .then((status) => {
      updateBuzzerStatus(status);
    })
    .catch((error) => {
      console.error("Error getting buzzer status:", error);
    });
}

function fetchRelayStatus() {
  fetch("/get-relay-status")
    .then((response) => response.text())
    .then((status) => {
      updateRelayStatus(status);
    })
    .catch((error) => {
      console.error("Error getting relay status:", error);
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

function toggleRelay() {
  fetch("/toggle-relay")
    .then((response) => response.text())
    .then((status) => {
      console.log("Relay state: " + status);
    })
    .catch((error) => {
      console.error("Error toggling relay:", error);
    });
}

document.addEventListener("DOMContentLoaded", function () {
  fetchBuzzerStatus();
  fetchRelayStatus();
});

function updateBuzzerStatus(status) {
  const buzzerStatusElement = document.getElementById("buzzerStatus");
  if (status === "on") {
    buzzerStatusElement.textContent = "on";
  } else {
    buzzerStatusElement.textContent = "off";
  }
}

function updateRelayStatus(status) {
  const relayStatusElement = document.getElementById("relayStatus");
  if (status === "on") {
    relayStatusElement.textContent = "on";
  } else {
    relayStatusElement.textContent = "off";
  }
}

function update() {
  updateValues();
  fetchBuzzerStatus();
  fetchRelayStatus();
}

setInterval(update, 2000);
