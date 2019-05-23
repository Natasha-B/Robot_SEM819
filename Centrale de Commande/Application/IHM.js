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
var cortex_file = fs.createWriteStream('Desktop' + 'cortex.txt', {flags : 'w'});
var log_stdout = process.stdout;
var Client = require('ftp');

// Chargement de socket.io
var io = require('socket.io').listen(server);


//------------------------------------------------------------------------------------
// GESTION DU PORT SERIE   https://serialport.io
//------------------------------------------------------------------------------------

// Ports série 
const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
const port = new SerialPort('/dev/tty.usbserial', { baudRate: 19200 }, function(err) {if (err) {return console.log('Error on write: ', err.message);}});
//const port = new SerialPort('/dev/tty.Bluetooth-Incoming-Port', { baudRate: 19200 }, function(err) {if (err) {return console.log('Error on write: ', err.message);}});


const parser = port.pipe(new Readline());


// Envoi du message de début
port.write('Robot Connecté\n', function(err) {
  if (err) {
    return console.log('Error on write: ', err.message);
  }
  console.log('Serial Robot OK');
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

const portblue = new SerialPort('/dev/tty.usbserial-A9054ZJ1', { baudRate: 115200, rtscts: true }, function(err) {if (err) {return console.log('Error on write: ', err.message);}});
const parserb = new Readline();
portblue.pipe(parserb);

// Envoi du message de début
portblue.write('Cortex connecté\n', function(err) {
  if (err) {
    return console.log('Error on write: ', err.message);
  }
  console.log('Serial Cortex OK');
});

// Gestion des erreurs
parserb.on('error', function(err) {
  console.log('Error: ', err.message);
});


var string = ''
portblue.on('data',function(data){
  var part = data.toString();
  string += part;
  console.log('Message du cortex:  ' + part);
  //io.to('all').emit('message', part);
  fs.appendFile('../../../../../Dropbox/Projet\ Transversal/FichiersPCcalcul/cortex.txt', part, (err) => {
  if (err) throw err;
  console.log('Message Cortex saved!');
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
	  	});
    });
});



//------------------------------------------------------------------------------------
// Lecture des fichiers traités https://nodejs.org/api/fs.html
// https://www.ipgirl.com/949/verifiez-si-le-fichier-repertoire-existe-dans-node-js.html
//
//------------------------------------------------------------------------------------

const readline = require('readline');

if (fs.existsSync('../../../../../Dropbox/Projet\ Transversal/FichiersPCcalcul/test.txt')){
      const rl = readline.createInterface({
          input: fs.createReadStream('../../../../../Dropbox/Projet\ Transversal/FichiersPCcalcul/test.txt')
      });

      // Each new line emits an event - every time the stream receives \r, \n, or \r\n
      rl.on('line', (line) => {
          port.write( line, function(err) {
            if (err) {
              return console.log('Error on write: ', err.message);
            };
            });
          console.log(line);
      });

      rl.on('close', () => {
          console.log('Done reading file');
          fs.unlink('../../../../../Dropbox/Projet\ Transversal/FichiersPCcalcul/test.txt', (err) => {
              if (err) throw err;
          console.log('file successfully deleted');
      });
      });
};

//------------------------------------------------------------------------------------
// GESTION DE LA PAGE IHM
//------------------------------------------------------------------------------------

app.use(express.static(__dirname + '/public'));

app.get('/', function(req,res){
	res.render('couverture.ejs', {});
});


app.get('/assistance', function(req,res){
	res.render('assistance.ejs');
});

//------------------------------------------------------------------------------------

server.listen(8080);  //  http://localhost:8080
