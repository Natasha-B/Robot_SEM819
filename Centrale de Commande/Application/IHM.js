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
// GESTION DU PORT SERIE
//------------------------------------------------------------------------------------

// Port série https://serialport.io
const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
const port = new SerialPort('/dev/tty.usbmodem14101', { baudRate: 9600 });

const parser = new Readline();
port.pipe(parser);

// Envoi du message de début
port.write('message a envoyer\n', function(err) {
  if (err) {
    return console.log('Error on write: ', err.message);
  }
  console.log('message envoyé');
});

// Gestion des erreurs
port.on('error', function(err) {
  console.log('Error: ', err.message);
});


// Switches the port into "flowing mode"
port.on('data', function (data) {
  console.log('J\'ai reçu:', data);
})

//parser.on('data', line => console.log(`> ${line}`))


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

server.listen(8080);  //http://localhost:8080
