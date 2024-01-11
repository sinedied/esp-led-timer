// Read index.html file, gzip it and write it to ../src/html.h as a C string
// to be included in the web server code.

import fs from 'fs/promises';
import zlib from 'zlib';
import util from 'util'
;
const gzip = util.promisify(zlib.gzip);

async function createHeader() {
  try {
    const data = await fs.readFile('index.html');
    const compressed = await gzip(data);

    // Convert to C array of bytes, with lines of 16 integer values
    const index_data = 
      Array.from(compressed)
        .map((val, i) => (i % 16 === 0) ? `\n  ${val}` : val)
        .join(', ');

    const file = 
`#ifndef __HTML_H
#define __HTML_H

#include <arduino.h>

const uint32_t INDEX_HTML_SIZE = ${compressed.length};
const uint8_t INDEX_HTML[] PROGMEM = {${index_data}
};

#endif // __HTML_H
`;

    // Write to ../src/html.h
    await fs.writeFile('../src/html.h', file);
    console.log(`File written successfully (gizpped size: ${compressed.length})`);
  } catch (err) {
    console.error('Error:', err);
  }
}

createHeader();
