//https://leafletjs.com/examples/crs-simple/crs-simple.html


var map = L.map('mapid', {crs: L.CRS.Simple});

var bounds = [[0,0], [720,1000]];
var image = L.imageOverlay('iu.jpeg', bounds).addTo(map);

map.fitBounds(bounds);

var map = L.map('mapid', {
    crs: L.CRS.Simple,
    minZoom: -3
});

var sol = L.latLng([ 110, 170 ]);
L.marker(sol).addTo(map).bindPopup('Sol');