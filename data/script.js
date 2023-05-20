function updateValues() {
  fetch("/tvoc")
    .then((response) => response.text())
    .then((tvocValue) => {
      // Log the received TVOC value
      console.log("Received TVOC Value:", tvocValue);

      // Update the content of the TVOC element
      const tvocElement = document.getElementById("tvocValue");
      tvocElement.textContent = "TVOC Value: " + tvocValue;
    })
    .catch((error) => {
      console.error("Error:", error);
    });

  fetch("/eco2")
    .then((response) => response.text())
    .then((eco2Value) => {
      // Log the received TVOC value
      console.log("Received eCO2 Value:", eco2Value);

      // Update the content of the TVOC element
      const eco2Element = document.getElementById("eco2Value");
      eco2Element.textContent = "eCO2 Value: " + eco2Value;
    })
    .catch((error) => {
      console.error("Error:", error);
    });
}

// Update the TVOC value initially
updateValues();

// Update the TVOC value every 500ms
setInterval(updateValues, 1000);
