const cp = require('child_process');

async function main() {
  try {
    await fs.stat(bsatk);
    await cp.spawn('git', ['update'], { cwd: 'bsatk' });
  } catch (err) {
    await cp.spawn('git', ['clone', '--branch', 'noboost', '--depth=1', 'https://github.com/TanninOne/modorganizer-bsatk', 'bsatk']);
  }
}

main();

