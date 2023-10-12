var express = require('express');
var router = express.Router();
var axios = require('axios');
var MongoDB_Data;

/* GET home page. */
router.get('/', function(req, res, next) {
  // res.render('index', { title: 'Express' });
  let date_ob = new Date();
	let date = ("0" + date_ob.getDate()).slice(-2);
	let month = ("0" + (date_ob.getMonth() + 1)).slice(-2);
	let year = date_ob.getFullYear();
	let hours = (date_ob.getHours() + 7)%24;
	let minutes = date_ob.getMinutes();
	let seconds = date_ob.getSeconds();
	let sum = date + '/' + month + '/' + year + ' ' + hours + ':' + minutes + ':' + seconds;
	ReadMongoDB();
	
	try {
		res.render('index', {datatime : MongoDB_Data.document.Time,time : sum, temperature : MongoDB_Data.document.Evironment.Temp, humidity : MongoDB_Data.document.Evironment.Humi});
	}catch(E){
		// ReadMongoDB();
		res.render('error', {message : "Reload page, please!"});
	}
	
  // res.render('index', {datatime : '1',time : 'sum', temperature : 'MongoDB_Data.document.Evironment.Temp', humidity : 'MongoDB_Data.document.Evironment.Humi'});
});

function ReadMongoDB (){
	var axios = require('axios');
	var data = JSON.stringify({
		"collection": "Weather",
		"database": "Earth",
		"dataSource": "Cluster0",
		"projection": {
			"_id": "650bc4a2e0ddb653931e3400", 
			"Time" : 1, 
			"Evironment" : 1
		}
	});

	var config = {
		method: 'post',
		url: 'https://ap-southeast-1.aws.data.mongodb-api.com/app/data-ikhcm/endpoint/data/v1/action/findOne',
		headers: {
		  'Content-Type': 'application/json',
		  'Access-Control-Request-Headers': '*',
		  'api-key': 'j7XHAlHtKgbTWSCD9zb4reClAgYmwPhsD0AqSnoEzLE7ZHRAk1ukUDzHZp0By5jH',
		},	
		data: data
	};
	axios(config)
	.then(function (response) {		
		// console.log(JSON.stringify(response.data));
		MongoDB_Data = JSON.parse(JSON.stringify(response.data));
	})
	.catch(function (error) {
		console.log(error);
	}
	)
}


module.exports = router;
