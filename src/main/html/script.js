
// Credit to Fearphage at http://stackoverflow.com/a/4673436
String.prototype.format = function() {
    var args = arguments;
    return this.replace(/{(\d+)}/g, (m, d) =>
        typeof args[d] != 'undefined' ? args[d] : m
    );
};

function sleep(time) { 
    return new Promise((resolve) => setTimeout(resolve, time));
}

address = '127.0.0.1';
port = 5731;
baseUrl = 'http://' + address + ':' + port + '/';

function makeRequest(path, argDict){
    argStr = '?';
    for(var arg in argDict)
        argStr += arg + '=' + argDict[arg] + '&';

    var request = new XMLHttpRequest();
    request.open('GET', baseUrl + path + argStr);
    request.responseType = 'json';
    return request;
}

var online = true;

async function updateLoop(){
    while(online){
        updateInfected();
        await sleep(200);
    }
}

function updateInfected(){
    var request = makeRequest('API/sim/infectedCount');

    request.onload = function(){
        console.log("Infected count request answer: " + request.response.value);
        $('.infected').text(request.response.value);
    }

    request.onerror = async function () {
        console.log("Unable to connect, process must have stopped.");
        online = false;
    }

    request.send();
}



// Sends a math request to the server and writes the outcome in the .data section of the page
function mathRequest(x, y, op = "add"){
    var request = makeRequest('API/math', {x : x, y : y, op : op});

    request.onload = function(){
        console.log("Math request answer: " + request.response.value);
        $('.data').text(request.response.value);
    }

    request.send();
}

function onConnect(request){
    console.log(request.response);
    $('.status').text("Online!");
    mathRequest(10, 20);
    updateLoop();
}

// Function which will continuously try connecting until it succeeds
// Calls onConnect with the request response upon connecting.
async function tryConnecting() {
    var request = new XMLHttpRequest();
    request.open('GET', baseUrl + 'API/online');
    request.responseType = 'json';

    request.onload = () => onConnect(request);

    request.onerror = async function () {
        console.log("Unable to connect, retrying...");
        await sleep(500);
        tryConnecting();
    }

    request.send();
}

$(document).ready(function(){

    $('.port').text(port)
    // Try connecting as soon as the document has finished loading.
    tryConnecting();
});