var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });
var settings = JSON.parse(localStorage.getItem('clay-settings')) || {};;


var socket;
var reconnectTimer;
var statusRetry;
var statusSending = false;

// Send connection status without overlapping AppMessage requests.
function reportConnection() {
  if (statusSending) return;
  clearTimeout(statusRetry);
  var connected = socket && socket.readyState === WebSocket.OPEN ? 1 : 0;
  var payload = { '2': connected };
  statusSending = true;
  Pebble.sendAppMessage(payload, function() {
    statusSending = false;
    var current = socket && socket.readyState === WebSocket.OPEN ? 1 : 0;
    if (current !== connected) reportConnection();
  }, function() {
    statusSending = false;
    statusRetry = setTimeout(reportConnection, 1000);
  });
}

function connect() {
  clearTimeout(reconnectTimer);
  if (socket) {
    socket.onopen = socket.onclose = socket.onerror = null;
    socket.close();
    socket = null;
  }
  try {
    socket = new WebSocket(settings.WebsocketServer);
    socket.onopen = function() {
      clearTimeout(reconnectTimer);
      reportConnection();
    };
    socket.onclose = function() {
      reportConnection();
      clearTimeout(reconnectTimer);
      reconnectTimer = setTimeout(connect, 5000);
    };
    socket.onerror = function() {
      reportConnection();
      clearTimeout(reconnectTimer);
      reconnectTimer = setTimeout(connect, 5000);
    };
  } catch (error) {
    console.log('Unable to open WebSocket.');
    reconnectTimer = setTimeout(connect, 5000);
  }
  reportConnection();
}

Pebble.addEventListener('ready', connect);

Pebble.addEventListener('showConfiguration', function() {
  clay.setSettings(settings);
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(event) {
  console.log(event)


});

Pebble.addEventListener('appmessage', function(event) {
  reportConnection();
  var heartRate = event.payload['1'];
  if (typeof heartRate === 'number' && socket && socket.readyState === WebSocket.OPEN) {
    try {
      socket.send(JSON.stringify({ event: 'heartRate', data: heartRate }));
    } catch (error) {
      reportConnection();
    }
  }
});
