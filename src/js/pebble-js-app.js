var initialised = false;

function appMessageAck(e) {
    console.log("options sent to Pebble successfully");
}

function appMessageNack(e) {
    console.log("options not sent to Pebble: " + e.error.message);
}

Pebble.addEventListener("ready", function() {
    initialised = true;
});

Pebble.addEventListener("showConfiguration", function() {
    var options = JSON.parse(window.localStorage.getItem('cas_ae_20w_opt'));
    console.log("read options: " + JSON.stringify(options));
    console.log("showing configuration");
    if (options == null) {
        var uri = 'http://panicman.byto.de/config_common.html?title=Casio%20AE-20W';
    } else {
        var uri = 'http://panicman.byto.de/config_common.html?title=Casio%20AE-20W' + 
			'&inv=' + encodeURIComponent(options['inv']) + 
			'&vibr=' + encodeURIComponent(options['vibr']) + 
			'&datefmt=' + encodeURIComponent(options['datefmt']);
    }
	console.log("Uri: "+uri);
    Pebble.openURL(uri);
});

Pebble.addEventListener("webviewclosed", function(e) {
    console.log("configuration closed");
    if (e.response != '') {
        var options = JSON.parse(decodeURIComponent(e.response));
        console.log("storing options: " + JSON.stringify(options));
        window.localStorage.setItem('cas_ae_20w_opt', JSON.stringify(options));
        Pebble.sendAppMessage(options, appMessageAck, appMessageNack);
    } else {
        console.log("no options received");
    }
});
