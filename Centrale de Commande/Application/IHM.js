
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

// var fs = require('fs');
// http.createServer(function (req, res) {
//   fs.readFile('bonjour.html', function(err, data) {
//     res.writeHead(200, {'Content-Type': 'text/html'});
//     res.write(data);
//     res.end();
//   });
// }).listen(8080);