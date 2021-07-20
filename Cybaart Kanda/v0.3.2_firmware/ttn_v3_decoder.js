function decodeUplink(input) {
  
  var bytes = input.bytes;
  var port = input.fPort;

  var checkBytes = function(name, bytes, num)
  {
    if (bytes.length !== num) {
      throw new Error(name+' must have exactly '+ num+' bytes');
    }
  }

  var bytesToInt = function(bytes) {
    var i = 0;
    for (var x = 0; x < bytes.length; x++) {
      i |= +(bytes[x] << (x * 8));
    }
    return i;
  };


  var unixtime = function(bytes) {
    checkBytes('Unixtime',bytes,4);
    return bytesToInt(bytes);
  };
  unixtime.BYTES = 4;

  var uint8 = function(bytes) {
    checkBytes('Int',bytes,1);
    return bytesToInt(bytes);
  };
  uint8.BYTES = 1;
  
  var uint16 = function(bytes) {
    checkBytes('Int',bytes,2);
    return bytesToInt(bytes);
  };
  uint16.BYTES = 2;
  
  var latLng = function(bytes) {
    checkBytes('Lat/Lon',bytes,8);
  
    var lat = bytesToInt(bytes.slice(0, latLng.BYTES / 2));
    var lng = bytesToInt(bytes.slice(latLng.BYTES / 2, latLng.BYTES));
  
    return [lat / 1e6, lng / 1e6];
  };
  latLng.BYTES = 8;
  
  var temperature = function(bytes) {
    checkBytes('Temperature',bytes,2);
  
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
    checkBytes('RH',bytes,2);
  
    var h = bytesToInt(bytes);
    return h / 1e2;
  };
  humidity.BYTES = 2;
  
  function rawfloat(bytes) {
    checkBytes('Raw float',bytes,4);
  
    var bits = bytes[3]<<24 | bytes[2]<<16 | bytes[1]<<8 | bytes[0];
    var sign = (bits>>>31 === 0) ? 1.0 : -1.0;
    var e = bits>>>23 & 0xff;
    var m = (e === 0) ? (bits & 0x7fffff)<<1 : (bits & 0x7fffff) | 0x800000;
    var f = sign * m * Math.pow(2, e - 150);
    return f;
  }
  rawfloat.BYTES = 4;
  
  var bitmap = function(byte) {
    checkBytes('Bitmap',bytes,1);
  
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
  
  var decoded = {};
  
  switch (port)
  {
    case 1:
      tmp = decode(bytes, [    latLng,        uint16,     rawfloat,  uint16,   uint16, temperature,     humidity,       bitmap], 
                         ['coordinates','gps_elevation_m','hdop','elevation2_m','p','temperature_c','humidity_percent' ,'flags']);                       
  
      decoded = tmp;
      decoded.latitude_deg = tmp.coordinates[0];
      decoded.longitude_deg = tmp.coordinates[1];
      decoded.pressure_hpa = tmp.p / 10;
      decoded.coordinates = {}; // unneeded
      break;
    
    case 2:
      decoded = decode(bytes, [bitmap], ['flags']);
      break;
  
    case 3:
      decoded = decode(bytes, [eosioname], ['miner']);
      break;
  
    default:
      decoded = null;
      break;
  
  }

  return {  
    data: decoded, 
    warnings: [], 
    errors: []  
  };
    
}
