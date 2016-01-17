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

function apiDuinoCall(isPut, value) {

  // * * * NOT real PUT yet - both are actually GET! * * *

  // construct URL
  // TODO: add config vs. hard-coded!!!
  var url = 'http://192.168.4.1/led'; // Flip-Flip AP mode

  if (isPut) {
    url = url + '/' + value;
  }

  // send request
  xhrRequest(url, 'GET', 
    function(responseText) {

      // APIduino returns led value as JSON
      // *returned value is "requested" for PUT
      // and actual state for GET
      var json = JSON.parse(responseText);

      var led = json.led;
      console.log('led is ' + led);

      // return led value
      var dict = {
        'data': led
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

  apiDuinoCall(e.payload.verb, e.payload.data);
});
