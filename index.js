const express = require('express');
const mqtt = require('mqtt');
const app = express();
const port = 3000;
const http = require('http');
const server = http.createServer(app);
const path = require('path');
const { Server } = require("socket.io");
const io = new Server(server);
const routes = require('./src/routes');
const sensorModel = require('./src/models/sensors.model');

