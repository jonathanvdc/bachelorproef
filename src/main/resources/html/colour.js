/**
 * Script for the colours of the visualizer webpage.
 * Author: Sibert Aerts
 */

//------------//
// Class: RGB //
//------------//

/// Class representing an RGB colour.
var RGB = function(r=0,g=0,b=0){
    this.r = clamp(r, 0, 255);
    this.g = clamp(g, 0, 255);
    this.b = clamp(b, 0, 255);
}

/// Produce a new colour that's a measure p of this colour, and a measure (1-p) of the other.
RGB.prototype.mix = function(other, p){
    p = clamp(p, 0, 1);
    var out = new RGB();
    out.r = this.r * p + other.r * (1 - p);
    out.g = this.g * p + other.g * (1 - p);
    out.b = this.b * p + other.b * (1 - p);
    return out;
}

/// Convert the colour to a css-formatted string.
RGB.prototype.toString = function(){
    return "rgb(" + Math.round(this.r) + "," + Math.round(this.g) + "," + Math.round(this.b) + ")";
}


//-----------------//
// Class: Gradient //
//-----------------//

/// Quickly make a {value, colour} tuple.
var valCol = (val, colour) => ({val: val, colour: colour});

var Gradient = function(colourMap){
    // colourMap is a List[Dict{val, colour}] sorted by increasing val
    this.colourMap = colourMap;
}

/// Get the colour at a certain spot in the gradient.
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

/// Creates a new gradient scaled from this one.
Gradient.prototype.scale = function(scale){
    var newMap = [];
    for(i in this.colourMap){
        newMap.push(valCol(this.colourMap[i].val * scale, this.colourMap[i].colour));
    }
    return new Gradient(newMap);
}

/// Turn a list of RGB colours into an evenly-spaced gradient.
Gradient.makeSimpleGradient = function(colours, size=1){
    colourMap = [];
    for(var i in colours)
        colourMap.push(valCol(size*i/(colours.length-1), colours[i]));
    return new Gradient(colourMap);
}

// Define a couple of gradients that we can use later.
// All of these are from 0 to 1.

// black -> red -> yellow -> white
var heatMap = Gradient.makeSimpleGradient([
    new RGB(0,0,0),
    new RGB(255,0,0),
    new RGB(255,255,0),
    new RGB(255,255,255),
]);

// cyan -> blue -> black -> red -> yellow -> white
var superHeatMap = Gradient.makeSimpleGradient([
    new RGB(0,255,255),
    new RGB(0,0,255),
    new RGB(0,0,0),
    new RGB(255,0,0),
    new RGB(255,255,0),
    new RGB(255,255,255),
]);

// dark blue -> deep purple -> red -> yellow -> white
var ultraHeatMap = Gradient.makeSimpleGradient([
    new RGB(0,0,80),
    new RGB(128,0,128),
    new RGB(255,0,0),
    new RGB(255,230,0),
    new RGB(255,255,255),
]);

// red -> yellow -> green -> cyan -> blue -> purple
var rainbow = Gradient.makeSimpleGradient([
    new RGB(255,0,0),
    new RGB(255,255,0),
    new RGB(0,255,0),
    new RGB(0,255,255),
    new RGB(0,0,255),
    new RGB(255,0,255),
]);

// red -> yellow -> green -> cyan -> blue -> purple
var fluPlusPlus = Gradient.makeSimpleGradient([
    new RGB(60,0,40),
    new RGB(255,0,128),
    new RGB(210,210,60),
    new RGB(0,255,128),
    new RGB(255,255,255),
]);

// black -> white
var monochrome = Gradient.makeSimpleGradient([new RGB(0), new RGB(255,255,255)]);

// Put them in a little dictionary for easy iterating.
Gradient.gradients = {
    "Monochrome": monochrome,
    "Heat map": heatMap,
    "Super heat map": superHeatMap,
    "Ultra heat map": ultraHeatMap,
    "Rainbow": rainbow,
    "Flu++": fluPlusPlus,
};