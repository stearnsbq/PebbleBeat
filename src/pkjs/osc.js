// src/pkjs/osc.js

function pad4(length) {
  return (4 - (length % 4)) % 4;
}

function oscString(value) {
  var length = value.length + 1;
  var output = new Uint8Array(length + pad4(length));
  var i;

  for (i = 0; i < value.length; i++) {
    output[i] = value.charCodeAt(i) & 0xff;
  }

  output[value.length] = 0;
  return output;
}

function oscInt32(value) {
  var output = new Uint8Array(4);
  var view = new DataView(output.buffer);

  view.setInt32(0, value, false);
  return output;
}

function oscFloat32(value) {
  var output = new Uint8Array(4);
  var view = new DataView(output.buffer);

  view.setFloat32(0, value, false);
  return output;
}

function concatArrays(arrays) {
  var total = 0;
  var offset = 0;
  var output;
  var i;

  for (i = 0; i < arrays.length; i++) {
    total += arrays[i].length;
  }

  output = new Uint8Array(total);

  for (i = 0; i < arrays.length; i++) {
    output.set(arrays[i], offset);
    offset += arrays[i].length;
  }

  return output;
}

function encodeOsc(address, args) {
  var typeTags = ",";
  var values = [];
  var i;
  var arg;

  args = args || [];

  for (i = 0; i < args.length; i++) {
    arg = args[i];

    if (arg.type === "i") {
      typeTags += "i";
      values.push(oscInt32(arg.value));
    } else if (arg.type === "f") {
      typeTags += "f";
      values.push(oscFloat32(arg.value));
    } else if (arg.type === "s") {
      typeTags += "s";
      values.push(oscString(String(arg.value)));
    } else if (arg.type === "b") {
      // OSC booleans are represented entirely by the type tag.
      typeTags += arg.value ? "T" : "F";
    } else {
      throw new Error("Unsupported OSC type: " + arg.type);
    }
  }

  return concatArrays([
    oscString(address),
    oscString(typeTags)
  ].concat(values));
}
module.exports = {
  encode: encodeOsc
};