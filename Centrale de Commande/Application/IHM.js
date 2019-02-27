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

app.use(express.static(__dirname + '/public'));

app.get('/', function(req,res){
	res.render('index.ejs', {});
});

app.get('/banana', function(req,res){
	res.render('banana.ejs', {"nom": "en pyjama"});
});

app.get('/banana/:nom', function(req,res){
	res.render('banana.ejs', {"nom": req.params.nom});
});

server.listen(8080);
