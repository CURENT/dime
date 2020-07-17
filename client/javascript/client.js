class DimeClient {
    constructor(hostname, port) {
        this.workspace = {};
    }

    sendmsg(jsondata, bindata) {
        return new Promise(function(resolve, reject) {
            jsondata = new TextEncoder().encode(JSON.stringify(jsondata));
            bindata = new Uint8Array(bindata);

            let msg = new Uint8Array(12 + jsondata.length + bindata.length);
            const dview = new DataView(msg.buffer);

            dview.setUint32(0, 0x44694D45); // ASCII for "DiME"
            dview.setUint32(4, jsondata.length);
            dview.setUint32(8, bindata.length);

            msg.set(jsondata, 12);
            msg.set(bindata, 12 + jsondata.length);

            msg = msg.buffer;

            function callback(sendinfo) {
                if (sendinfo.resultCode < 0) {
                    reject();
                }

                msg = msg.slice(sendinfo.bytesSent);

                if (msg.length > 0) {
                    chrome.sockets.tcp.send(this.conn, msg, callback);
                } else {
                    resolve();
                }
            }

            chrome.sockets.tcp.send(this.conn, msg, callback);
        });
    }

    recvmsg() {
    }
}
