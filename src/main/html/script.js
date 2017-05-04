
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

baseUrl = 'http://127.0.0.1:5731/';


// Sends a math request to the server and writes the outcome in the .data section of the page
function mathRequest(x, y, op = "add"){
    var request = new XMLHttpRequest();
    request.open('GET', baseUrl + 'API/math?x={0}&y={1}&op={2}'.format(x, y, op));
    request.responseType = 'json';

    request.onload = function(){
        console.log("Math request answer: " + request.response);
        $('.data').text(request.response.value);
    }

    request.send();
}

function onConnect(request){
    console.log(request.response);
    $('.status').text("Online!");
    mathRequest(10, 20);
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

// Try connecting as soon as the document has finished loading.
$(document).ready(() => tryConnecting());