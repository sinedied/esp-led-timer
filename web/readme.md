# Web UI

The web UI is developed using vanilla JS/HTML/CSS. All the code is located in the `web/index.html` folder.

To build the web UI, you need to have [Node.js](https://nodejs.org/) installed.

## Development

To start the development server, run:

```bash
npm install # needed only once
npm run dev
```

This will start a development server on port 5173. You can access the web UI at http://localhost:5173.

## Building header file

To build the header file, run:

```bash
npm install # needed only once
npm run build
```

This will create/update the `src/html.h` file used by the web server.
