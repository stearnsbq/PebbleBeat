var socket;
var statusSending = false;
var statusRetry;

function reportConnection() {
  if (statusSending) {
    return;
  }
  clearTimeout(statusRetry);
  var connected = socket && socket.readyState === WebSocket.OPEN ? 1 : 0;
  statusSending = true;
  Pebble.sendAppMessage({ "2": connected }, function () {
    statusSending = false;
    var current = socket && socket.readyState === WebSocket.OPEN ? 1 : 0;
    if (current !== connected) {
      reportConnection();
    }
  }, function () {
    statusSending = false;
    statusRetry = setTimeout(reportConnection, 1000);
  });
}

Pebble.addEventListener("ready", function () {
  socket = new WebSocket("ws://192.168.8.181:8080");
  socket.onopen = reportConnection;
  socket.onclose = reportConnection;
  socket.onerror = reportConnection;
  reportConnection();
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('Received message from watch!', e);
  reportConnection();
  
  const heartRate = e.payload["1"]
  
  socket.send(JSON.stringify({event: "heart_rate", data: heartRate}))

});
