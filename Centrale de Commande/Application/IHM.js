//------------------------ -----------------------------------------------------------
// Projet_Transversal - IHM.js
//------------------------------------------------------------------------------------
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
var util = require ('util');
var fs = require('fs');
var server = http.Server(app);
var log_file = fs.createWriteStream('Desktop' + 'debug.log', {flags : 'w'});
var log_stdout = process.stdout;
var ftpClient = require('ftp-client');

// Chargement de socket.io
var io = require('socket.io').listen(server);


//------------------------------------------------------------------------------------
// GESTION DU PORT SERIE   https://serialport.io
//------------------------------------------------------------------------------------

// Ports série 
const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
//const port = new SerialPort('/dev/tty.usbserial', { baudRate: 19200 });
const port = new SerialPort('/dev/tty.Bluetooth-Incoming-Port', { baudRate: 19200 });

const parser = port.pipe(new Readline());


// Envoi du message de début
port.write('Robot Connecté\n', function(err) {
  if (err) {
    return console.log('Error on write: ', err.message);
  }
  console.log('Robot OK');
});

// Gestion des erreurs
parser.on('error', function(err) {
  console.log('Error: ', err.message);
});


// Reception du message d'init
parser.on('data', function (data) {
  console.log('\r\n Message du robot: ', data);
  io.to('all').emit('message', data);
});


//------------------------------------------------------------------------------------
// GESTION DU PORT SERIE BLUETOOTH    https://stackoverflow.com/questions/8393636/node-log-in-a-file-instead-of-the-console
//------------------------------------------------------------------------------------

//const portblue = new SerialPort('/dev/tty.usbserial-A9054ZJ1', { baudRate: 19200 });
/*const parserblue = portblue.pipe(new Readline());

// Envoi du message de début
portblue.write('Cortex connecté\n', function(err) {
  if (err) {
    return console.log('Error on write: ', err.message);
  }
  console.log('Cortex OK');
});

// Gestion des erreurs
parserblue.on('error', function(err) {
  console.log('Error: ', err.message);
});

// Reception du message d'init
parserblue.on('data', function (data) {
  log_file.write(util.format(d) + '\n');
  log_stdout.write(util.format(d) + '\n');
});*/


//------------------------------------------------------------------------------------
// GESTION FTP   https://www.npmjs.com/package/ftp-client
//------------------------------------------------------------------------------------

//var ftpClient = require('./lib/client.js'),
config = {
    host: 'localhost',
    port: 21,
    user: 'centrale',
    password: 'centrale'
}

options = {
    logging: 'basic'
}

client = new ftpClient(config, options);

client.connect(function () {
  client.upload(['test/**'], '/public/testFTP', {
      baseDir: 'test',
      overwrite: 'older'
  }, function (result) {
      console.log(result);
  });

  client.download('/public/testFTP2', 'testFTP2/', {
      overwrite: 'all'
  }, function (result) {
      console.log(result);
  });

});

//------------------------------------------------------------------------------------
// GESTION SOCKET.IO   https://openclassrooms.com/fr/courses/1056721-des-applications-ultra-rapides-avec-node-js/1057825-socket-io-passez-au-temps-reel
//------------------------------------------------------------------------------------


io.sockets.on('connection', function (socket) {
    console.log('message :', "Mise à jour de l'IHM effectuée");
    socket.join('all');

    // Quand le serveur reçoit un signal de type "message" du client    
    socket.on('message', function (message) {
        //console.log("Message de l'IHM :  "+ message);
        port.write( message, function(err) {
          if (err) {
            return console.log('Error on write: ', err.message);
	  	    };
	  	  console.log('message envoyé au robot : ', message);
        // Ajoute un message dans la page
        function insereMessage(message) {
          $('#zone_chat').prepend('<p>'+ message + '</p>');
        }
	  	});
    });
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
