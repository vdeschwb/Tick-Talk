Pebble.addEventListener('ready', function() {
    console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function(e) {
    var url = 'http://vdeschwb.github.io/Tick-Talk/';
    console.log('Showing configuration page: ' + url);

    Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
    var configData = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration page returned: ' + JSON.stringify(configData));

    var dict = {};
    dict['DISPLAY_STYLE_KEY'] = configData['display_style'];
    dict['FPS_KEY'] = configData['fps'];

    // Send to watchapp
    Pebble.sendAppMessage(dict, function() {
        console.log('Send successful: ' + JSON.stringify(dict));
        }, function() {
        console.log('Send failed!');
    });
});