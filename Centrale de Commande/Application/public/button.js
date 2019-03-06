//------------------------ -----------------------------------------------------------
// Projet_Transversal - button.js
//------------------------------------------------------------------------------------
// DATE: 27/02/19
//
// DEVELOPPEUR: BRU Natacha
//
// Groupe: B2
//
//------------------------------------------------------------------------------------


var sockets = io.connect('http://localhost:8080');
sockets.on('message', function(message) {
    alert(message);
});

$('#arret').click(function () {
    sockets.emit('message', 'stop');
});
$('#debut').click(function () {
    sockets.emit('message', 'D1');
});
$('#avance').click(function () {
    sockets.emit('message', 'mogo 1:20 2:20');
});
$('#recule').click(function () {
    sockets.emit('message', 'TV vitesse 20 B 20');
});
$('#tourne_d').click(function () {
    sockets.emit('message', 'TV vitesse 20 RD');
});
$('#tourne_g').click(function () {
    sockets.emit('message', 'TV vitesse 20 RG');
});
$('#demiTour').click(function () {
    sockets.emit('message', 'TV vitesse 20 RC');
});
$('#stop').click(function () {
    sockets.emit('message', 'E');
});
$('#mesure_d').click(function () {
    sockets.emit('message', 'Mesure la distance');
});
$('#mesure_c').click(function () {
    sockets.emit('message', 'Mesure la consommation');
});
$('#emission').click(function () {
    sockets.emit('message', 'Emet un son');
});
$('#reception').click(function () {
    sockets.emit('message', 'Ecoute le son');
});
$('#image').click(function () {
    sockets.emit('message', 'Prends une photo');
});
$('#active_p').click(function () {
    sockets.emit('message', 'Active le pointeur lumineux');
});
$('#intensité_plus').click(function () {
    sockets.emit('message', "Augmente l'intensité");
});
$('#intensité_moins').click(function () {
    sockets.emit('message', "Diminue l'intensité");
});