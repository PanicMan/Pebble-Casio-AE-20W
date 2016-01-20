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
    var options = JSON.parse(localStorage.getItem('cas_ae_20w_opt'));
    console.log("read options: " + JSON.stringify(options));
    console.log("showing configuration");
	var uri = 'http://panicman.github.io/config_c20ae20w.html?title=Casio%20AE-20W%20v2.6';
    if (options) {
        uri += 
			'&inv=' + encodeURIComponent(options.inv) + 
			'&datemode=' + encodeURIComponent(options.datemode) + 
			'&vibr=' + encodeURIComponent(options.vibr) + 
			'&vibr_bt=' + encodeURIComponent(options.vibr_bt) + 
			'&secs=' + encodeURIComponent(options.secs) + 
			'&showsec=' + encodeURIComponent(options.showsec) + 
			'&datefmt=' + encodeURIComponent(options.datefmt);
    }
	console.log("Uri: "+uri);
    Pebble.openURL(uri);
});

Pebble.addEventListener("webviewclosed", function(e) {
    console.log("configuration closed");
    if (e.response !== '') {
        var options = JSON.parse(decodeURIComponent(e.response));
        console.log("storing options: " + JSON.stringify(options));
        localStorage.setItem('cas_ae_20w_opt', JSON.stringify(options));
        Pebble.sendAppMessage(options, appMessageAck, appMessageNack);
    } else {
        console.log("no options received");
    }
});
