/* SETUP OPTIONS WHICH ARE NOT TO BE TOUCHED ONCE THEY WORK */
console.log("Setting up libraries");
var serialjs=require('serialport-js');
var request = require('request');
var gpsd = require('node-gpsd');
var socket = require('socket.io-client')('http://ec2-52-10-133-86.us-west-2.compute.amazonaws.com:3000');

console.log("Setting up static variables");
var doingRoute = false;
var sp;
var forwardInt = 1;
var backInt = 2;
var breakInt = 3;
var straightInt = 4;
var rightInt = 5;
var leftInt = 6;
var fullSpeed = 9;
var delay = 1000;
var sph = 0;
var tpv = 0;

// CREATE A GPSD DAEMON
console.log("Creating gps Daemon");
var daemon = new gpsd.Daemon({
        program: '/usr/sbin/gpsd',
        device: '/dev/ttyAMA0',
        verbose: true
});
// GPS DAEMON
console.log("Creating gps listener");
daemon.start(function() {
    var listener = new gpsd.Listener();
    
    listener.on('TPV', function (timePosVel) {
	console.log("RECEIVED A TPV FROM GPS");
	tpv = timePosVel;
        console.log(tpv);
    });
    
    listener.connect(function() {
	console.log("CONNECTED TO GPS");
        listener.watch();
    });
});

// SET UP STANDARD WEBSOCKET STUFF
socket.on('connect', function(){
	console.log("Connected to Websocket");
	socket.emit('join', {access_token:"bacon"});
});

socket.on('disconnect', function(){
	console.log("Disconnected from server");
});

// FIND A SERIAL PORT THAT IS AN ARDUINO
serialjs.find(serialDevicesPopulated);
function serialDevicesPopulated(ports){
    console.log("Connecting to serial Port");
    if(!ports[0]){ return; }
    console.log(ports[0]);
    serialjs.open(ports[0].port,start,'\n');
    //serialjs.open("/dev/ttyACM0",start,'\n');
}

// SEND DATA TO SP
function start(port){
    console.log("Finializing SP Connection");
    port.on('data', gotData);
    sp = port;
    console.log("Setting Full Speed");
    sp.send(fullSpeed);
}
// RECIEVE DATA FROM SP
function gotData(data){
    console.log("Recieved from the serial port: " + data);
    sph = data;
}


// GET THE ROUTE INFORMATION FOR A ROUTE
function getRoute(num, callback){
	// Query the server for route information
	console.log("Requesting route information");
	request.post({url:'http://ec2-52-10-133-86.us-west-2.compute.amazonaws.com/Lux/Assets/query.php?access_token=bacon'
		,form: {"query":{"type":"route", "route":num}}}
		, function(err,httpResponse,body){
			if (!err && httpResponse.statusCode == 200) {
				console.log("body: "); console.log(body);
				callback(JSON.parse(body));
			}
		});
}
// SET THE STATUS INFORMATION
setInterval(function(){
	// Update the status information on the server
	console.log("Updating Status information");
	request.post({url:'http://ec2-52-10-133-86.us-west-2.compute.amazonaws.com/Lux/Assets/upsert.php?access_token=bacon'
		,form: {"query":{"type":"status"}, update:{"battery":"20", "fps":"120", "sph":sph, "gps":"10", "wifi":"50", "cpu":"90"}}}
		, function(err,httpResponse,body){ 
			if (!err && httpResponse.statusCode == 200) {
//				console.log("body: "); console.log(body);
			}
		});
},delay*5);












/*
*
* YOU CAN TOUCH FROM HERE DOWN 
*
*/
// RECIEVE DATA FROM WS
/*
 * Create a websocket port so that when it gets a message 
 * that says it is an update, run the doRoute function. 
 * Just to prevent a billion messages from coming, it also 
 * sets the doingRoute to true.
 */
socket.on('updated', function(data){
	console.log("Recieved Data from WebSocket");
	if(data.inprogress){
		socket.emit('upsert', {query:{"route":data.route}, update:{'$set':{'inprogress':false}}});
		if(!doingRoute){
			doingRoute = true;
			var current = tpv;// TODO get GPS location
			sp.send(forwardInt);
			setTimeout(function(){
				doRoute(data.route_cord, current);
			},delay);
		}
	}
});

function calculateVector(old, current, target){
	// leftInt
	// rightInt
	// straightInt
	console.log("Checking Vector:");
	console.log(old);
	console.log(current);
	console.log(target);
	return leftInt || straightInt;
}
function withinThreshold(current, target){
	console.log("Checking Threshold:");
	console.log(current);
	console.log(target);
	return true;
}
/*
 * Recursive function to calculate how to get to the next point,
 * Constantly recalculating to see if it is along the correct vector
 * Every itteration it goes a certain distance forward
 */
function doRoute(cords, old){
	sp.send(breakInt);
	if(cords.length != undefined && cords.length > 0){
		console.log("DOING ROUTE");
		console.log("Target: " + JSON.stringify(cords[0]));
		// Here is the logic of the function
		var current = tpv;// TODO get GPS location
		console.log("Current: " + JSON.stringify(current));
		if(withinThreshold(current, cords[0])){
		// if you are close enough
			var cords2 = cords;
			cords2.shift();
			doRoute(cords2, old);
		}else{
			sp.send(calculateVector(old, current, target));
			sp.send(forwardInt);
			setTimeout(function(){
				doRoute(cords, current);
			},delay);
		}
	}else{
		doingRoute = false;
		console.log("Route Completed");
	}
}

