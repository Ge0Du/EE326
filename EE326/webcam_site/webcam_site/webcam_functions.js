// webcam_functions.js

var gateway = "ws://" + window.location.hostname + "/ws";
var websocket = null;
var isStreaming = false;
var frameCount = 0;
var fpsCount = 0;
var fpsLastTime = Date.now();

// start live clock on page load
window.addEventListener("load", function() {
  setInterval(updateClock, 1000);
  updateClock();
});

function toggleStream() {
  if (!isStreaming) {
    startStream();
  } else {
    stopStream();
  }
}

function startStream() {
  //*** open websocket connection ***//
  websocket = new WebSocket(gateway);
  websocket.binaryType = "arraybuffer";
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onerror = onError;
  websocket.onmessage = onMessage;
  //*** ***//

  isStreaming = true;
  document.getElementById("streamToggle").textContent = "Stop Stream";
  document.getElementById("statusText").textContent = "Connecting...";
}

function stopStream() {
  //*** close websocket connection ***//
  if (websocket) {
    websocket.close();
  }
  //*** ***//

  isStreaming = false;
  document.getElementById("streamToggle").textContent = "Start Stream";
  document.getElementById("statusText").textContent = "Stopped";
  document.getElementById("fpsDisplay").textContent = "0";
}

function onOpen(event) {
  document.getElementById("statusText").textContent = "Connected";
}

function onClose(event) {
  document.getElementById("statusText").textContent = "Disconnected";
  if (isStreaming) {
    isStreaming = false;
    document.getElementById("streamToggle").textContent = "Start Stream";
  }
}

function onError(event) {
  document.getElementById("statusText").textContent = "Error";
}

function onMessage(event) {
  //*** receive jpeg frame from esp32 and display it ***//
  if (event.data instanceof ArrayBuffer) {
    var blob = new Blob([event.data], { type: "image/jpeg" });
    var url = URL.createObjectURL(blob);
    var img = document.getElementById("webcamImage");
    if (img.src.startsWith("blob:")) URL.revokeObjectURL(img.src);
    img.src = url;

    // human-readable frame timestamp
    updateTimestamp();

    // fps
    frameCount++;
    fpsCount++;
    var now = Date.now();
    var elapsed = (now - fpsLastTime) / 1000;
    if (elapsed >= 1.0) {
      document.getElementById("fpsDisplay").textContent = (fpsCount / elapsed).toFixed(1);
      fpsCount = 0;
      fpsLastTime = now;
    }
  }
  //*** ***//
}

function updateTimestamp() {
  //*** build human-readable timestamp (not just unix time) ***//
  var d = new Date();
  var days = ["Sun","Mon","Tue","Wed","Thu","Fri","Sat"];
  var months = ["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"];
  var h = d.getHours(), m = d.getMinutes(), s = d.getSeconds();
  var ampm = h >= 12 ? "PM" : "AM";
  h = h % 12 || 12;
  var str = days[d.getDay()] + " " + months[d.getMonth()] + " " + d.getDate() + " " + d.getFullYear()
          + " " + h + ":" + pad(m) + ":" + pad(s) + " " + ampm;
  //*** ***//
  document.getElementById("timestamp").textContent = str;
}

function updateClock() {
  var d = new Date();
  var h = d.getHours(), m = d.getMinutes(), s = d.getSeconds();
  var ampm = h >= 12 ? "PM" : "AM";
  h = h % 12 || 12;
  document.getElementById("clockDisplay").textContent = h + ":" + pad(m) + ":" + pad(s) + " " + ampm;
}

function pad(n) { return String(n).padStart(2, "0"); }
