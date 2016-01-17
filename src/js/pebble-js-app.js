/*
 * pebble-js-app.js
 */

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function apiDuinoCall(isPut, value, endpoint) {

  // * * * NOT real PUT yet - both are actually GET! * * *

  // construct URL
  // TODO: add config vs. hard-coded!!!
  var url = 'http://192.168.4.1'; // Flip-Flip AP mode

  switch (endpoint) {
    // endpoints currently hard-coded numeric mapped values for ease of implementation
    case 0:
      url = url + '/' + 'led';

      // should be PUT, but GET will do for now; other endpoints are read-only
      if (isPut) {
        url = url + '/' + value;
      }      

      break;
    case 1:
      url = url + '/' + 'tmp';
      break;
    case 2:
      url = url + '/' + 'adc';
      break;
    case 3:
      url = url + '/' + 'relay';
      break;
    default:
      // do nothing for now    
  }



  // send request
  xhrRequest(url, 'GET', 
    function(responseText) {

      // API returns value as JSON
      // *returned value is "requested" for PUT
      // and actual state for GET
      var json = JSON.parse(responseText);
      var data = 0;

      switch (endpoint) {
        // endpoints currently hard-coded numeric mapped values for ease of implementation
        case 0:
          data = json.led;
          console.log('led is ' + data);
          break;
        case 1:
          data = json.tmp;
          console.log('tmp is ' + data);
          break;
        case 2:
          data = json.adc;
          console.log('adc is ' + data);
          break;
        case 3:
          data = json.relay;
          console.log('relay is ' + data);
          break;
        default:
          // do nothing for now    
      }


      // return JSON data value
      var dict = {
        'data': data
      };

      Pebble.sendAppMessage(dict, function(e) {
        console.log('Send successful.');
      }, function(e) {
        console.log('Send failed!');
      });

    }      
  );
}


Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS Ready!');
});


Pebble.addEventListener("appmessage", function(e) {
  console.log("AppMessage received!");

  apiDuinoCall(e.payload.verb, e.payload.data, e.payload.endpoint);
});
