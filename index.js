Object.defineProperty(exports, "__esModule", { value: true });

let nbind = require('nbind');
let path = require('path');
let bsatk;
try {
  bsatk = nbind.init(path.join(__dirname, 'bsatk')).lib;
} catch (err) {
  if (err.message.indexOf('Could not locate the bindings file') !== -1) {
    bsatk = nbind.init().lib;
  } else {
    throw err;
  }
}

module.exports = bsatk;
