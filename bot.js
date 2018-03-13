var http = require('http');

var options = {
  host: "proxy.mei.co.jp",
  port: 8080,
  path: "http://panasonic.com",
  headers: {
    Host: "panasonic.com"
  }
};

http.get(options, (res) => {
  var body = '';
  res.on('data', (chunk) => {
      body += chunk;
  });
  res.on('end', (res) => {
      console.log(body);
  });
}).on('error', (e) => {
  console.log(e.message);
});
