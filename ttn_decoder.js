
function Decoder(bytes) {
    // calculate dewpoint (degrees C) given temperature (C) and relative humidity (0..100)
// from http://andrew.rsmas.miami.edu/bmcnoldy/Humidity.html
// rearranged for efficiency and to deal sanely with very low (< 1%) RH
function dewpoint(t, rh) {
    var c1 = 243.04;
    var c2 = 17.625;
    var h = rh / 100;
    if (h <= 0.01)
        h = 0.01;
    else if (h > 1.0)
        h = 1.0;

    var lnh = Math.log(h);
    var tpc1 = t + c1;
    var txc2 = t * c2;
    var txc2_tpc1 = txc2 / tpc1;

    var tdew = c1 * (lnh + txc2_tpc1) / (c2 - lnh - txc2_tpc1);
    return tdew;
}

var bytesToInt = function(bytes) {
  var i = 0;
  for (var x = 0; x < bytes.length; x++) {
    i |= +(bytes[x] << (x * 8));
  }
  return i;
};

var unixtime = function(bytes) {
  if (bytes.length !== unixtime.BYTES) {
    throw new Error('Unix time must have exactly 4 bytes');
  }
  return bytesToInt(bytes);
};
unixtime.BYTES = 4;

var uint8 = function(bytes) {
  if (bytes.length !== uint8.BYTES) {
    throw new Error('int must have exactly 1 byte');
  }
  return bytesToInt(bytes);
};
uint8.BYTES = 1;

var uint16 = function(bytes) {
  if (bytes.length !== uint16.BYTES) {
    throw new Error('int must have exactly 2 bytes');
  }
  return bytesToInt(bytes);
};
uint16.BYTES = 2;

var latLng = function(bytes) {
  if (bytes.length !== latLng.BYTES) {
    throw new Error('Lat/Long must have exactly 8 bytes');
  }

  var lat = bytesToInt(bytes.slice(0, latLng.BYTES / 2));
  var lng = bytesToInt(bytes.slice(latLng.BYTES / 2, latLng.BYTES));

  return [lat / 1e6, lng / 1e6];
};
latLng.BYTES = 8;

var temperature = function(bytes) {
  if (bytes.length !== temperature.BYTES) {
    throw new Error('Temperature must have exactly 2 bytes');
  }
  var isNegative = bytes[0] & 0x80;
  var b = ('00000000' + Number(bytes[0]).toString(2)).slice(-8)
        + ('00000000' + Number(bytes[1]).toString(2)).slice(-8);
  if (isNegative) {
    var arr = b.split('').map(function(x) { return !Number(x); });
    for (var i = arr.length - 1; i > 0; i--) {
      arr[i] = !arr[i];
      if (arr[i]) {
        break;
      }
    }
    b = arr.map(Number).join('');
  }
  var t = parseInt(b, 2);
  if (isNegative) {
    t = -t;
  }
  return t / 1e2;
};
temperature.BYTES = 2;

var humidity = function(bytes) {
  if (bytes.length !== humidity.BYTES) {
    throw new Error('Humidity must have exactly 2 bytes');
  }

  var h = bytesToInt(bytes);
  return h / 1e2;
};
humidity.BYTES = 2;

// Based on https://stackoverflow.com/a/37471538 by Ilya Bursov
// quoted by Arjan here https://www.thethingsnetwork.org/forum/t/decode-float-sent-by-lopy-as-node/8757
function rawfloat(bytes) {
  if (bytes.length !== rawfloat.BYTES) {
    throw new Error('Float must have exactly 4 bytes');
  }
  // JavaScript bitwise operators yield a 32 bits integer, not a float.
  // Assume LSB (least significant byte first).
  var bits = bytes[3]<<24 | bytes[2]<<16 | bytes[1]<<8 | bytes[0];
  var sign = (bits>>>31 === 0) ? 1.0 : -1.0;
  var e = bits>>>23 & 0xff;
  var m = (e === 0) ? (bits & 0x7fffff)<<1 : (bits & 0x7fffff) | 0x800000;
  var f = sign * m * Math.pow(2, e - 150);
  return f;
}
rawfloat.BYTES = 4;

var bitmap = function(byte) {
  if (byte.length !== bitmap.BYTES) {
    throw new Error('Bitmap must have exactly 1 byte');
  }
  var i = bytesToInt(byte);
  var bm = ('00000000' + Number(i).toString(2)).substr(-8).split('').map(Number).map(Boolean);
  return ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
    .reduce(function(obj, pos, index) {
      obj[pos] = bm[index];
      return obj;
    }, {});
};
bitmap.BYTES = 1;

var decode = function(bytes, mask, names) {

  var maskLength = mask.reduce(function(prev, cur) {
    return prev + cur.BYTES;
  }, 0);
  if (bytes.length < maskLength) {
    throw new Error('Mask length is ' + maskLength + ' whereas input is ' + bytes.length);
  }

  names = names || [];
  var offset = 0;
  return mask
    .map(function(decodeFn) {
      var current = bytes.slice(offset, offset += decodeFn.BYTES);
      return decodeFn(current);
    })
    .reduce(function(prev, cur, idx) {
      prev[names[idx] || idx] = cur;
      return prev;
    }, {});
};

if (typeof module === 'object' && typeof module.exports !== 'undefined') {
  module.exports = {
    unixtime: unixtime,
    uint8: uint8,
    uint16: uint16,
    temperature: temperature,
    humidity: humidity,
    latLng: latLng,
    bitmap: bitmap,
    rawfloat: rawfloat,
    decode: decode
  };
}

    tmp = decode(bytes, [uint16, uint16, temperature, humidity], ['z', 'p', 'tempC', 'rh']);
    
    // Nick added functions
    tmp.p = tmp.p / 10;
    tmp.tDewC = dewpoint( tmp.tempC, tmp.rh );
    
    return tmp;
}
