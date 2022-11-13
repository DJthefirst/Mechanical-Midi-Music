

if ("serial" in navigator) console.log("The Web Serial API is supported.");

document.getElementById("btnAddDevice").onclick = () => { openPort() };


async function openPort() {
    console.log("click");
    // Prompt user to select any serial port.
    const port = await navigator.serial.requestPort();

    console.log(port);

}