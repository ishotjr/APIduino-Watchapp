/*
 * pebble-js-app.js
 */

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS Ready!');
});


Pebble.addEventListener("appmessage", function(e) {
  console.log("AppMessage received!");
  if (e.payload.verb) {

    // PUT

    // return requested value (NOT actual)
    var dict = {
      'data': e.payload.data
    };

    Pebble.sendAppMessage(dict, function(e) {
      console.log('Send successful.');
    }, function(e) {
      console.log('Send failed!');
    });

  } else {

    // GET

    // return actual value
    var dict = {
      'data': 2
    };

    Pebble.sendAppMessage(dict, function(e) {
      console.log('Send successful.');
    }, function(e) {
      console.log('Send failed!');
    });

  }
});
