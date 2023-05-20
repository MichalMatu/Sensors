function updateValues() {
  fetch("/values")
    .then((response) => response.text())
    .then((values) => {
      // Update the content of the TVOC element
      const tvocElement = document.getElementById("values");
      tvocElement.innerHTML = values;
    })
    .catch((error) => {
      console.error("Error:", error);
    });
}

// Update the values initially
updateValues();

// Update the values every 1000ms
setInterval(updateValues, 1000);
