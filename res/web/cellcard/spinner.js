function setNumSweeps(numSweeps)
{
    const spinner = document.getElementById("spinner");
    //const num = parseInt(document.getElementById("numOptions").value, 10);
  
    // Clear existing options
    spinner.innerHTML = "";
  
    // Add new options
    for (let i = 1; i <= numSweeps; i++) {
        let option = document.createElement("option");
        option.value = i;
        option.text = "Option " + i;
        spinner.appendChild(option);
    }
}
