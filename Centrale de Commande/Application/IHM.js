//------------------------ -----------------------------------------------------------
// Projet_Transversal - IHM.js
//------------------------------------------------------------------------------------
// DATE: 27/02/19
//
// DEVELOPPEUR: BRU Natacha
//
// Groupe: B2
//
//------------------------------------------------------------------------------------
// DÉCLARATION DES VARIABLES
//------------------------------------------------------------------------------------

var express = require ('express');
var app = express();
var http = require('http');
var server = http.Server(app);


//------------------------------------------------------------------------------------
// GESTION DU PORT SERIE   https://serialport.io
//------------------------------------------------------------------------------------

// Port série 
const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
const port = new SerialPort('/dev/tty.usbmodem14101', { baudRate: 9600 });

const parser = port.pipe(new Readline());

// Envoi du message de début
port.write('message a envoyer\n', function(err) {
  if (err) {
    return console.log('Error on write: ', err.message);
  }
  console.log('message envoyé');
});

// Gestion des erreurs
parser.on('error', function(err) {
  console.log('Error: ', err.message);
});


// Reception d'informations
parser.on('data', function (data) {
  console.log('J\'ai reçu:', data);
})

//------------------------------------------------------------------------------------
// GESTION SOCKET.IO   https://openclassrooms.com/fr/courses/1056721-des-applications-ultra-rapides-avec-node-js/1057825-socket-io-passez-au-temps-reel
//------------------------------------------------------------------------------------


// Chargement de socket.io
var io = require('socket.io').listen(server);

// Quand un client se connecte, on le note dans la console
io.sockets.on('connection', function (socket) {
    console.log('Un client est connecté !');
});


//------------------------------------------------------------------------------------
// GESTION DE LA PAGE IHM
//------------------------------------------------------------------------------------

app.use(express.static(__dirname + '/public'));

app.get('/', function(req,res){
	res.render('couverture.ejs', {});
});

app.get('/assistance', function(req,res){
	res.render('assistance.ejs', {"nom": "en pyjama"});
});

app.get('/assistance/:nom', function(req,res){
	res.render('assistance.ejs', {"nom": req.params.nom});
});

//------------------------------------------------------------------------------------

server.listen(8080);  //  http://localhost:8080
