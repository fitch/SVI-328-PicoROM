const dgram = require('dgram');
const net = require('net');
const fs = require('fs');
const path = require('path');

const UDP_PORT = 4243;
const TCP_PORT = 4242;
const HANDSHAKE_MESSAGE = "SVI-328 PicoROM hello!";

const romFilePath = process.argv[2];
if (!romFilePath || !fs.existsSync(romFilePath)) {
    console.error("Usage: node send_rom.js <path/to/romfile.rom>");
    process.exit(1);
}

const romData = fs.readFileSync(romFilePath);
if (romData.length !== 32768) {
    console.error("ROM file must be exactly 32768 bytes (32kB)");
    process.exit(1);
}

const udpServer = dgram.createSocket('udp4');

udpServer.on('listening', () => {
    const address = udpServer.address();
    console.log(`Waiting for SVI-328 at address ${address.address}...`);
});

udpServer.on('message', (message, remote) => {
    const msgString = message.toString().trim();

    if (msgString === HANDSHAKE_MESSAGE) {
        console.log(`Connecting to SVI-328 at ${remote.address}...`);

        udpServer.close();

        const tcpClient = new net.Socket();
        tcpClient.connect(TCP_PORT, remote.address, () => {
            console.log("Connected. Sending ROM upload command...");
            tcpClient.write("UPLOAD_ROM\n");
            tcpClient.write(romData);
            console.log("ROM sent. Listening for logs:");
            tcpClient.write("DUMP_LOG\n");
        });

        tcpClient.on('data', (data) => {
            process.stdout.write(`${data.toString()}`);
        });

        tcpClient.on('close', () => {
            console.log('TCP connection closed');
        });

        tcpClient.on('error', (err) => {
            console.error(`TCP Error: ${err.message}`);
        });
    }
});

udpServer.bind(UDP_PORT, () => {
    udpServer.setBroadcast(true);
});
