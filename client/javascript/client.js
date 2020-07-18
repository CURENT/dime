class DimeClient {
    constructor(hostname, port) {
        const self = this;

        this.ws = null;
        this.workspace = {};
        this.serialization = "dimeb";
        this.recvbuffer = new ArrayBuffer(0);
        this.recvcallback = null;

        this.connected = new Promise(function(resolve, reject) {
            self.ws = new WebSocket("ws://" + hostname + ":" + port);
            self.ws.binaryType = "arraybuffer";

            self.ws.onmessage = function(event) {
                if (self.recvbuffer.byteLength > 0) {
                    let recvbuffer = new ArrayBuffer(self.recvbuffer.byteLength + event.data.byteLength);

                    recvbuffer.set(self.recvbuffer);
                    recvbuffer.set(event.data, self.recvbuffer.byteLength);

                    self.recvbuffer = recvbuffer;
                } else {
                    self.recvbuffer = event.data;
                }

                if (self.recvcallback) {
                    self.recvcallback();
                }
            };

            self.ws.onopen = function() {
                self.__send({
                    command: "handshake",
                    serialization: self.serialization,
                    tls: false
                });

                self.__recv().then(function([jsondata, bindata]) {
                    if (jsondata.status < 0) {
                        reject(jsondata.error);
                    }

                    // No serialization methods other than dimeb are supported (for now)
                    if (jsondata.serialization !== "dimeb") {
                        reject("Cannot use the selected serialization");
                    }

                    resolve();
                }).catch(reject);
            };
        })
    }

    async send_r(name, kvpairs) {
        if (this.connected) {
            await this.connected;
            this.connected = null;
        }

        for (let [varname, value] of Object.entries(kvpairs)) {
            let jsondata = {
                command: "send",
                name: name,
                varname: varname,
                serialization: this.serialization
            };

            let bindata = dimebdumps(value);

            this.__send(jsondata, bindata);
            [jsondata, bindata] = await this.__recv();

            if (jsondata.status < 0) {
                throw jsondata.error;
            }
        }
    }

    async sync_r(n = -1) {
        this.__send({
            command: "sync",
            n: n
        })

        const ret = {};
        let m = n;

        while (true) {
            const [jsondata, bindata] = await this.__recv();

            if ("status" in jsondata) {
                if (jsondata.status < 0) {
                    throw jsondata.error;
                }

                break;
            }

            if (jsondata.serialization === "dimeb") {
                ret[jsondata.varname] = dimebloads(bindata);
            } else {
                m--;
            }
        }

        if (n > 0 && m < n) {
            Object.assign(ret, await self.sync_r(n - m));
        }

        return ret;
    }

    __send(jsondata, bindata = new ArrayBuffer(0)) {
        jsondata = new TextEncoder().encode(JSON.stringify(jsondata));
        bindata = new Uint8Array(bindata);

        const msg = new Uint8Array(12 + jsondata.length + bindata.length);
        const dview = new DataView(msg.buffer);

        dview.setUint32(0, 0x44694D45); // ASCII for "DiME"
        dview.setUint32(4, jsondata.length);
        dview.setUint32(8, bindata.length);

        msg.set(jsondata, 12);
        msg.set(bindata, 12 + jsondata.length);

        this.ws.send(msg.buffer);
    }

    __recv() {
        const self = this;

        return new Promise(function(resolve, reject) {
            let callback = function() {
                if (self.recvbuffer.byteLength >= 12) {
                    const dview = new DataView(self.recvbuffer);

                    const magic = dview.getUint32(0);
                    const jsondata_len = dview.getUint32(4);
                    const bindata_len = dview.getUint32(8);

                    if (magic != 0x44694D45) { // ASCII for "DiME"
                        self.recvcallback = null;
                        reject("Bad magic value");
                    }

                    if (self.recvbuffer.byteLength >= 12 + jsondata_len + bindata_len) {
                        let jsondata = self.recvbuffer.slice(12, 12 + jsondata_len);
                        let bindata = self.recvbuffer.slice(12 + jsondata_len, 12 + jsondata_len + bindata_len);

                        jsondata = JSON.parse(new TextDecoder().decode(jsondata));

                        self.recvbuffer = self.recvbuffer.slice(12 + jsondata_len + bindata_len);
                        self.recvcallback = null;

                        resolve([jsondata, bindata]);
                    }
                }
            }

            self.recvcallback = callback;
            callback();
        });
    }
}
