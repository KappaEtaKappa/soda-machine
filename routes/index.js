var express = require('express');
var router = express.Router();
var r = require('rethinkdb');
var config = require(__dirname+"/../config.js")
var moment = require('moment');

/* GET home page. */
router.get('/', function(req, res) {
  res.redirect('/usage');
});

router.get('/test', function(req, res) {
  res.send("1");
});

router.get('/usage', function(req, res) {
	r.table('purchases').eqJoin("memberid", r.table("members")).run(req._rdbConn, function(err, cursor) {
	    if (err) throw err;
	    cursor.toArray(function(err, result) {
	        if (err) throw err;
	        var purchases = [];
	        for (var i = 0; i < result.length; i++) {
	        	var newPurchase = {
	        		name: result[i].right.name,
	        		timestamp: result[i].left.timestamp,
	        		amount: result[i].left.amount,
	        	}
	        	purchases.push(newPurchase);
	        };
	        console.log(purchases);
	        var months = [];
	        for (var i = 0; i < 12; i++) {
	        	var m = moment().subtract(i,"month");
        		var myMonth = moment(m).days(0).hours(0).minutes(0);
	        	var mmmmyyyy = m.format("MMMM YYYY");
	        	var usage = {};
	        	for (var j = 0; j < purchases.length; j++) {
	        		var name = purchases[j].name;
	        		var pm = moment(purchases[j].timestamp);
	        		var a = pm.valueOf() > myMonth.valueOf();
	        		var b = pm.valueOf() < moment(myMonth).add(1, "month").valueOf();
	        		console.log(mmmmyyyy);
	        		console.log(moment(myMonth).format());
	        		console.log(pm.format());
	        		console.log(moment(myMonth).add(1, "month").format());
	        		if (a && b) {
	        			if (name in usage) {
	        				usage[name] += purchases[j].amount;
	        			} else {
	        				usage[name] = purchases[j].amount;
	        			}
	        		}
	        	}
	        	var month = {
	        		name: mmmmyyyy,
	        		usage: usage
	        	}
	        	months.push(month);
	        }
  			res.render('usage', { months: months });
	    });
	});
});

router.get("/scan/:id", function(req, res) {
 	r.table('members').get(req.params.id).run(req._rdbConn, function(error, value) {
        if (error) {
            handleError(res, error);
        }  else {
        	if (value != null) {
        		r.table('purchases').insert({memberid: req.params.id, amount: config.express.sodaCost, timestamp: new Date()}).run(req._rdbConn, function(error, value) {
			        if (error) {
			        	handleError(res, error);
			        }
        			res.send("1");
			    });
        	} else {
        		res.send("0");
        	}
        }
    });
});

module.exports = router;


function handleError(res, error) {
    return res.send(500, {error: error.message});
}
