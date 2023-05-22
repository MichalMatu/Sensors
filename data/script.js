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

// Add an event listener to the form submission
document
  .getElementById("tvocForm")
  .addEventListener("submit", function (event) {
    // Prevent the default form submission behavior
    event.preventDefault();

    // Get the TVOC value from the input field
    var tvocValue = document.getElementById("tvocInput").value;

    // Create the request body with the TVOC value
    var requestBody = new FormData();
    requestBody.append("tvoc_set", tvocValue);

    // Send a POST request to the server using Fetch API
    fetch("/set-tvoc", {
      method: "POST",
      body: requestBody,
    })
      .then(function (response) {
        // Handle the response if needed
      })
      .catch(function (error) {
        console.error("Error occurred while setting TVOC:", error);
      });
  });
