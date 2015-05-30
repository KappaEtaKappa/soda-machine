var express = require('express');
var router = express.Router();
var r = require('rethinkdb');

/* GET home page. */
router.get('/', function(req, res) {
  res.render('index', { title: 'Express' });
});

router.get("/scan/:id", function(req, res) {
    res.setHeader('Content-Type', 'application/json');

 	r.table('accounts').orderBy({"name": req.params.id}).run(req._rdbConn, function(error, cursor) {
        if (error) {
            handleError(res, error) 
            next();
        }
        else {
            // Retrieve all the todos in an array
            cursor.toArray(function(error, result) {
                if (error) {
                    handleError(res, error) 
                }
                else {
                    // Send back the data
                    res.send(JSON.stringify(result));
                }
            });
        }
    });
});

module.exports = router;


function handleError(res, error) {
    return res.send(500, {error: error.message});
}
