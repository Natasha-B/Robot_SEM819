//------------------------ -----------------------------------------------------------
// Projet_Transversal - IHM.js
//------------------------------------------------------------------------------------
// DATE: 27/02/19
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
//  NOMS: Blasco, Bru, Renotton
//
// Groupe: B2
//
//------------------------------------------------------------------------------------
var express = require ('express');
var app = express();
var http = require('http');
var server = http.Server(app);

//------------------------------------------------------------------------------------

// Port série https://serialport.io
const SerialPort = require('serialport')
const Readline = require('@serialport/parser-readline')
const port = new SerialPort('/dev/tty-usbserial1', { baudRate: 256000 })

const parser = new Readline()
port.pipe(parser)

parser.on('data', line => console.log(`> ${line}`))

// Envoi du message de début
port.write('ROBOT POWER ON\n', function(err) {
  if (err) {
    return console.log('Error on write: ', err.message)
  }
  console.log('message written')
})

// Gestion des erreurs
port.on('error', function(err) {
  console.log('Error: ', err.message)
})


// Switches the port into "flowing mode"
port.on('data', function (data) {
  console.log('Data:', data)
})

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

server.listen(8080);
