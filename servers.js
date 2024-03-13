const express = require('express');

const NUM_SERVERS = 4;
const START_PORT = 8000;
const apps = [];

for (let i = 0; i < NUM_SERVERS; i++) {
    const app = express();
    app.get("/", (req, res) => {
        return res.json({ 
            "server": i, 
            "port": START_PORT + i, 
            "timestamp": Date.now(), 
            "sender": req.ip 
        });
    });

    app.get("/rickroll", (req, res) => {
        return res.redirect("https://www.youtube.com/watch?v=p7YXXieghto");
    });

    apps.push(app);
}

Promise.all(apps.map((app, i) => {
    new Promise(() => {
        const server = app.listen(START_PORT + i, () => {
            console.log(`Server started on ::${START_PORT + i}`)
        })

        server.addListener('request', (req, res) => {
            console.log(`(server ${i}) received a request`)
        })
    })
}));