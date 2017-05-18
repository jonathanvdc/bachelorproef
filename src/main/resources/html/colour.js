/**
 * Script for the colours of the visualizer webpage.
 * Author: Sibert Aerts
 */

//------------//
// Class: RGB //
//------------//

var RGB = function(r=0,g=0,b=0){
    this.r = r;
    this.g = g;
    this.b = b;
}

RGB.prototype.mix = function(other, p){
    var out = new RGB();
    out.r = this.r * p + other.r * (1 - p);
    out.g = this.g * p + other.g * (1 - p);
    out.b = this.b * p + other.b * (1 - p);
    return out;
}

RGB.prototype.toString = function(){
    return "rgb(" + Math.round(this.r) + "," + Math.round(this.g) + "," + Math.round(this.b) + ")";
}


//-----------------//
// Class: Gradient //
//-----------------//

function ValCol(val, colour){
    return {val: val, colour: colour};
}

var Gradient = function(colourMap){
    // colourMap is a List[Dict{val, colour}] sorted by increasing val
    this.colourMap = colourMap;
}

Gradient.prototype.get = function(val){
    // Check the upper and lower bounds first
    if(val <= this.colourMap[0].val)
        return this.colourMap[0].colour;
    if(val >= this.colourMap[this.colourMap.length-1].val)
        return this.colourMap[this.colourMap.length-1].colour;

    // It has to be somewhere in the gradient
    for(i in this.colourMap){
        upper = this.colourMap[i];
        if( val == upper.val)
            return upper.colour;
        if( val < upper.val ){
            lower = this.colourMap[i-1];
            p = (upper.val - val) / (upper.val - lower.val);
            return lower.colour.mix(upper.colour, p);
        }
    }
}

// Creates a new scaled gradient.
Gradient.prototype.scale = function(scale){
    var newMap = [];
    for(i in this.colourMap){
        newMap.push(ValCol(this.colourMap[i].val * scale, this.colourMap[i].colour));
    }
    return new Gradient(newMap);
}

// Define a couple gradients that we can use later.
// All of these are from 0 to 1.

// black -> red -> yellow -> white
var heatMap = new Gradient([
    ValCol(0,   new RGB(0,0,0)),
    ValCol(1/3, new RGB(255,0,0)),
    ValCol(2/3, new RGB(255,255,0)),
    ValCol(1,   new RGB(255,255,255)),
]);

// cyan -> blue -> black -> red -> yellow -> white
var superHeatMap = new Gradient([
    ValCol(0,   new RGB(0,255,255)),
    ValCol(1/5, new RGB(0,0,255)),
    ValCol(2/5, new RGB(0,0,0)),
    ValCol(3/5, new RGB(255,0,0)),
    ValCol(4/5, new RGB(255,255,0)),
    ValCol(1,   new RGB(255,255,255)),
]);

// dark blue -> deep purple -> red -> yellow -> white
var ultraHeatMap = new Gradient([
    ValCol(0,   new RGB(0,0,80)),
    ValCol(1/4, new RGB(128,0,128)),
    ValCol(2/4, new RGB(255,0,0)),
    ValCol(3/4, new RGB(255,230,0)),
    ValCol(1,   new RGB(255,255,255)),
]);

// red -> yellow -> green -> cyan -> blue -> purple
var rainbow = new Gradient([
    ValCol(0,   new RGB(255,0,0)),
    ValCol(1/5, new RGB(255,255,0)),
    ValCol(2/5, new RGB(0,255,0)),
    ValCol(3/5, new RGB(0,255,255)),
    ValCol(4/5, new RGB(0,0,255)),
    ValCol(1,   new RGB(255,0,255)),
]);

// black -> white
var monochrome = new Gradient([
    ValCol(0,   new RGB(0)),
    ValCol(1,   new RGB(255,255,255)),
]);

// Put them in a little dictionary for easy iterating.
Gradient.gradients = {
    "Heat map": heatMap,
    "Super heat map": superHeatMap,
    "Ultra heat map": ultraHeatMap,
    "Rainbow": rainbow,
    "Monochrome": monochrome
};