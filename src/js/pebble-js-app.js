/*
 * pebble-js-app.js
 * Sends the sample message once it is initialized.
 */

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS Ready!');

  // Construct a dictionary
  var dict = {
    'KEY_DATA': 1
  };

  // Send a string to Pebble
  Pebble.sendAppMessage(dict, function(e) {
    console.log('Send successful.');
  }, function(e) {
    console.log('Send failed!');
  });
});