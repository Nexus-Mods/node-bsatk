Object.defineProperty(exports, "__esModule", { value: true });

let lib;
try {
  lib = require('./bsatk');
} catch (e) {
  lib = require('./build/Release/bsatk');
}


module.exports = lib;
