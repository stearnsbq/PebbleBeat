var socket;

Pebble.addEventListener("ready", function () {
  socket = new WebSocket("ws://192.168.8.181:8080");
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('Received message from watch!', e);
  
  const heartRate = e.payload["1"]
  
  socket.send(JSON.stringify({event: "heart_rate", data: heartRate}))

});
