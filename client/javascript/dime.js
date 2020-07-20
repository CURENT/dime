const dime = (function() {

class Complex {
    constructor(real, imag) {
        this.real = real;
        this.imag = imag;

        Object.freeze(this);
    }

    add(rhs) {
        return new Complex(this.real + rhs.real, this.imag + rhs.imag);
    }

    sub(rhs) {
        return new Complex(this.real - rhs.real, this.imag - rhs.imag);
    }

    mul(rhs) {
        const real = this.real * rhs.real - this.imag * rhs.imag;
        const imag = this.imag * rhs.real + this.real * rhs.imag;

        return new Complex(real, imag);
    }

    div(rhs) {
        const divisor = rhs.real * rhs.real + rhs.imag * rhs.imag;

        const real = (this.real * rhs.real + this.imag * rhs.imag) / divisor;
        const imag = (this.imag * rhs.real - this.real * rhs.imag) / divisor;

        return new Complex(real, imag);
    }
}

class NDArray {
	constructor(order, shape, complex, array) {
		this.order = order;
		this.shape = shape;
        this.complex = complex;

		if (!array) {
			let total = 1;

			for (let x of shape) {
				total *= x;
			}

            if (complex) {
                total *= 2;
            }

			array = new Float64Array(total);
		}

		this.array = array;
	}

	_makeIndex(index, strict) {
		const shape = this.shape;

		if (index.length !== shape.length) {
			throw 'bad index: ' + index.length + ' !== ' + shape.length;
		}

		let realIndex = 0;

		for (let i = 0; i < index.length; i++) {
			const shapeIndex = (this.order === 'F' ? index.length - i - 1 : i);
			const s = shape[shapeIndex];
			const j = index[shapeIndex];

			if (strict && j >= s) {
				throw 'bad index: index = ' + index + ', shape = ' + shape;
			}

			realIndex = realIndex * s + j;
		}

        if (this.complex) {
            realIndex *= 2;
        }

		return realIndex;
	}

	get(...index) {
		const i = this._makeIndex(index, true);

        if (this.complex) {
            return new Complex(this.array[i], this.array[i + 1]);
        } else {
            return this.array[i];
        }
	}

	set(value, ...index) {
		const i = this._makeIndex(index, true);

        if (this.complex) {
            this.array[i] = value.real;
            this.array[i + 1] = value.imag;
        } else {
            this.array[i] = value;
        }
	}

	/*column(n) {
		if (this.order !== 'F') {
			throw 'bad order, expected "' + this.order + '"';
		}
		const start = this._makeIndex([0, n], false);
		const end = this._makeIndex([0, n+1], false);
		console.log({ start, end });
		return this.typedArray.slice(start, end);
	}

	row(n) {
		if (this.order !== 'C') {
			throw 'bad order, expected "' + this.order + '"';
		}
		const start = this._makeIndex([n, 0], false);
		const end = this._makeIndex([n+1, 0], false);
		console.log({ start, end });
		return this.typedArray.slice(start, end);
	}

	extents() {
		if (this.shape[0] !== 1) {
			throw 'extents: expected column vector';
		}
		const typedArray = this.typedArray;
		let prev = typedArray[0];
		for (let i=1; i<typedArray.length; ++i) {
			const cur = typedArray[i];
			if (cur !== prev + 1) {
				throw 'extents: non contiguous';
			}
			prev = cur;
		}
		return { begin: typedArray[0], end: typedArray[typedArray.length - 1] + 1 };
	}

	subarray({ begin, end }) {
		return new NDArray(this.order, [1, end - begin - 1], this.typedArray.subarray(begin, end));
	}

	subindex(idx) {
		if (this.shape[0] !== 1) {
			throw 'extents: expected column vector';
		}

		if (idx.shape[0] !== 1) {
			throw 'extents: expected column vector';
		}

		const out_values = new Float64Array(idx.shape[1]);

		for (let i=0; i<idx.typedArray.length; i++){
			const pos = idx.typedArray[i];
			out_values[i] = this.get(0, pos);
		}

		return new NDArray(this.order, [1, idx.shape[1]], out_values)
	}*/
}

// Boolean sentinels
const TYPE_NULL  = 0x00;
const TYPE_TRUE  = 0x01;
const TYPE_FALSE = 0x02;

// Scalar types
const TYPE_I8  = 0x03;
const TYPE_I16 = 0x04;
const TYPE_I32 = 0x05;
const TYPE_I64 = 0x06;
const TYPE_U8  = 0x07;
const TYPE_U16 = 0x08;
const TYPE_U32 = 0x09;
const TYPE_U64 = 0x0A;

const TYPE_SINGLE         = 0x0B;
const TYPE_DOUBLE         = 0x0C;
const TYPE_COMPLEX_SINGLE = 0x0D;
const TYPE_COMPLEX_DOUBLE = 0x0E;

// Matrix types
const TYPE_MAT_I8  = 0x10 | TYPE_I8;
const TYPE_MAT_I16 = 0x10 | TYPE_I16;
const TYPE_MAT_I32 = 0x10 | TYPE_I32;
const TYPE_MAT_I64 = 0x10 | TYPE_I64;
const TYPE_MAT_U8  = 0x10 | TYPE_U8;
const TYPE_MAT_U16 = 0x10 | TYPE_U16;
const TYPE_MAT_U32 = 0x10 | TYPE_U32;
const TYPE_MAT_U64 = 0x10 | TYPE_U64;

const TYPE_MAT_SINGLE         = 0x10 | TYPE_SINGLE;
const TYPE_MAT_DOUBLE         = 0x10 | TYPE_DOUBLE;
const TYPE_MAT_COMPLEX_SINGLE = 0x10 | TYPE_COMPLEX_SINGLE;
const TYPE_MAT_COMPLEX_DOUBLE = 0x10 | TYPE_COMPLEX_DOUBLE;

// Other types
const TYPE_STRING     = 0x20;
const TYPE_ARRAY      = 0x21;
const TYPE_ASSOCARRAY = 0x22;

function __loadsmat(bytes, dtype, complex) {
    const [constructor, method, itemsize] = dtype;

    let dview = new DataView(bytes, 1);
    const rank = dview.getUint8(0);

    dview = new DataView(bytes, 2);
    const shape = [];

    for (let i = 0; i < rank; i++) {
        shape.push(dview.getUint32(i * 4));
    }

    const nelems = shape.reduce((a, b) => a * b) * (complex ? 2 : 1);

    dview = new DataView(bytes, 2 + 4 * rank);
    let array = new constructor(nelems);

    for (let i = 0; i < nelems; i++) {
        array[i] = dview[method](i * itemsize);
    }

    let obj = new NDArray("F", shape, complex, array);

    const nread = 2 + 4 * rank + nelems * itemsize;

    return [obj, nread];
}

function __loads(bytes) {
    let obj, nread;

    const dview = new DataView(bytes, 1);

    switch (new DataView(bytes).getUint8(0)) {
    case TYPE_NULL:
        obj = null;
        nread = 1;

        break;

    case TYPE_TRUE:
        obj = true;
        nread = 1;

        break;

    case TYPE_FALSE:
        obj = false;
        nread = 1;

        break;

    case TYPE_I8:
        obj = dview.getInt8(0);
        nread = 2;

        break;

    case TYPE_I16:
        obj = dview.getInt16(0);
        nread = 3;

        break;

    case TYPE_I32:
        obj = dview.getInt32(0);
        nread = 5;

        break;

    case TYPE_I64:
        obj = dview.getBigInt64(0);
        nread = 9;

        break;

    case TYPE_U8:
        obj = dview.getUint8(0);
        nread = 2;

        break;

    case TYPE_U16:
        obj = dview.getUint16(0);
        nread = 3;

        break;

    case TYPE_U32:
        obj = dview.getUint32(0);
        nread = 5;

        break;

    case TYPE_U64:
        obj = dview.getBigUint64(0);
        nread = 9;

        break;

    case TYPE_SINGLE:
        obj = dview.getFloat32(0);
        nread = 5;

        break;

    case TYPE_DOUBLE:
        obj = dview.getFloat64(0);
        nread = 5;

        break;

    case TYPE_COMPLEX_SINGLE:
        {
            const realpart = dview.getFloat32(0);
            const imagpart = dview.getFloat32(4);

            obj = new Complex(realpart, imagpart);
        }
        nread = 9;

        break;

    case TYPE_COMPLEX_DOUBLE:
        {
            const realpart = dview.getFloat64(0);
            const imagpart = dview.getFloat64(8);

            obj = new Complex(realpart, imagpart);
        }
        nread = 17;

        break;

    case TYPE_MAT_I8:
        [obj, nread] = __loadsmat(bytes, [Int8Array, "getInt8", 1], false);
        break;

    case TYPE_MAT_I16:
        [obj, nread] = __loadsmat(bytes, [Int16Array, "getInt16", 2], false);
        break;

    case TYPE_MAT_I32:
        [obj, nread] = __loadsmat(bytes, [Int32Array, "getInt32", 4], false);
        break;

    case TYPE_MAT_I64:
        [obj, nread] = __loadsmat(bytes, [BigInt64Array, "getBigInt64", 8], false);
        break;

    case TYPE_MAT_U8:
        [obj, nread] = __loadsmat(bytes, [Uint8Array, "getUint8", 1], false);
        break;

    case TYPE_MAT_U16:
        [obj, nread] = __loadsmat(bytes, [Uint16Array, "getUint16", 2], false);
        break;

    case TYPE_MAT_U32:
        [obj, nread] = __loadsmat(bytes, [Uint32Array, "getUint32", 4], false);
        break;

    case TYPE_MAT_U64:
        [obj, nread] = __loadsmat(bytes, [BigUint64Array, "getBigUint64", 8], false);
        break;

    case TYPE_MAT_SINGLE:
        [obj, nread] = __loadsmat(bytes, [Float32Array, "getFloat32", 4], false);
        break;

    case TYPE_MAT_DOUBLE:
        [obj, nread] = __loadsmat(bytes, [Float64Array, "getFloat64", 8], false);
        break;

    case TYPE_MAT_COMPLEX_SINGLE:
        [obj, nread] = __loadsmat(bytes, [Float32Array, "getFloat32", 4], true);
        break;

    case TYPE_MAT_COMPLEX_DOUBLE:
        [obj, nread] = __loadsmat(bytes, [Float64Array, "getFloat64", 8], true);
        break;

    case TYPE_STRING:
        {
            const len = dview.getUint32(0);

            obj = new TextDecoder().decode(bytes.slice(5, len + 5));
            nread = len + 5;
        }

        break;

    case TYPE_ARRAY:
        {
            const len = dview.getUint32(0);

            obj = [];
            nread = 5;

            for (let i = 0; i < len; i++) {
                const [elem, elem_siz] = __loads(bytes.slice(nread));

                obj.push(elem);
                nread += elem_siz;
            }
        }

        break;

    case TYPE_ASSOCARRAY:
        {
            const len = dview.getUint32(0);

            obj = {};
            nread = 5;

            for (let i = 0; i < len; i++) {
                const [key, key_siz] = __loads(bytes.slice(nread));
                nread += key_siz;

                const [val, val_siz] = __loads(bytes.slice(nread));
                nread += val_siz;

                obj[key] = val;
            }
        }

        break;
    }

    return [obj, nread];
}

function dimebloads(bytes) {
    const [obj, nread] = __loads(bytes);

    return obj;
}

function dimebdumps(obj) {
    let bytes;

    if (obj === null) {
        bytes = new Uint8Array(1);
        bytes[0] = TYPE_NULL;
    } else if (typeof obj === "boolean") {
        bytes = new Uint8Array(1);
        bytes[0] = (obj ? TYPE_TRUE : TYPE_FALSE);
    } else if (typeof obj === "number") {
        bytes = new Uint8Array(9);
        let dview = new DataView(bytes.buffer, 1);

        if (Number.isInteger(obj)) {
            bytes[0] = TYPE_I64;
            dview.setBigInt64(0, BigInt(obj));
        } else {
            bytes[0] = TYPE_DOUBLE;
            dview.setFloat64(0, obj);
        }
    } else if (obj instanceof Complex) {
        bytes = new Uint8Array(17);
        let dview = new DataView(bytes.buffer, 1);

        bytes[0] = TYPE_COMPLEX_DOUBLE;
        dview.setFloat64(0, obj.real);
        dview.setFloat64(8, obj.imag);
    } else if (obj instanceof NDArray) {
		if (obj.order !== 'F') {
			throw "C-order NDArrays are currently not supported for dimebdumps";
		}

        let dtype, method, itemsize;

        if (obj.array instanceof Int8Array) {
            dtype = TYPE_MAT_I8;
            method = "setInt8";
            itemsize = 1;
        } else if (obj.array instanceof Int16Array) {
            dtype = TYPE_MAT_I16;
            method = "setInt16";
            itemsize = 2;
        } else if (obj.array instanceof Int32Array) {
            dtype = TYPE_MAT_I32;
            method = "setInt32";
            itemsize = 4;
        } else if (obj.array instanceof BigInt64Array) {
            dtype = TYPE_MAT_I64;
            method = "setBigInt64";
            itemsize = 8;
        } else if (obj.array instanceof Uint8Array) {
            dtype = TYPE_MAT_U8;
            method = "setUint8";
            itemsize = 1;
        } else if (obj.array instanceof Uint16Array) {
            dtype = TYPE_MAT_U16;
            method = "setUint16";
            itemsize = 2;
        } else if (obj.array instanceof Uint32Array) {
            dtype = TYPE_MAT_U32;
            method = "setUint32";
            itemsize = 4;
        } else if (obj.array instanceof BigUint64Array) {
            dtype = TYPE_MAT_U64;
            method = "setBigUint64";
            itemsize = 8;
        } else if (obj.array instanceof Float32Array) {
            dtype = (obj.complex ? TYPE_MAT_COMPLEX_SINGLE : TYPE_MAT_SINGLE);
            method = "setFloat32";
            itemsize = 4;
        } else if (obj.array instanceof Float64Array) {
            dtype = (obj.complex ? TYPE_MAT_COMPLEX_DOUBLE : TYPE_MAT_DOUBLE);
            method = "setFloat64";
            itemsize = 8;
        }

        if (obj.complex && dtype != TYPE_MAT_COMPLEX_SINGLE && TYPE_MAT_COMPLEX_DOUBLE) {
            dtype = TYPE_MAT_COMPLEX_DOUBLE;
            method = "setFloat64";
            itemsize = 8;
        }

        const rank = obj.shape.length;
        const nelems = obj.array.length;

        bytes = new Uint8Array(2 + 4 * rank + nelems * itemsize);
        let dview = new DataView(bytes.buffer, 2);

        bytes[0] = dtype;
        bytes[1] = rank;

        for (let i = 0; i < rank; i++) {
            dview.setUint32(i * 4, obj.shape[i]);
        }

        dview = new DataView(bytes.buffer, 2 + 4 * rank);

        for (let i = 0; i < nelems; i++) {
            dview[method](i * itemsize, obj.array[i]);
        }
    } else if (typeof obj === "string") {
        const stringbytes = new TextEncoder().encode(obj);

        bytes = new Uint8Array(5 + stringbytes.length);
        dview = new DataView(bytes.buffer, 1);

        bytes[0] = TYPE_STRING;
        dview.setUint32(0, stringbytes.length);
        bytes.set(stringbytes, 5);
    } else if (Array.isArray(obj)) {
        const elemsbytes = obj.map(dimebdumps);

        bytes = new Uint8Array(5 + elemsbytes.reduce((acc, elembytes) => acc + elembytes.length, 0));
        let dview = new DataView(bytes.buffer, 1);

        bytes[0] = TYPE_ARRAY;
        dview.setUint32(0, elemsbytes.length);

        let pos = 5;

        for (const elembytes of elemsbytes) {
            bytes.set(elembytes, pos);
            pos += elembytes.length;
        }
    } else {
        const elemsbytes = Array.prototype.concat.apply([], Object.entries(obj)).map(dimebdumps);

        bytes = new Uint8Array(5 + elemsbytes.reduce((acc, elembytes) => acc + elembytes.length, 0));
        let dview = new DataView(bytes.buffer, 1);

        bytes[0] = TYPE_ASSOCARRAY;
        dview.setUint32(0, elemsbytes.length / 2);

        let pos = 5;

        for (const elembytes of elemsbytes) {
            bytes.set(elembytes, pos);
            pos += elembytes.length;
        }
    }

    return bytes;
}

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

    async join(...names) {
        this.__send({
            command: "join",
            name: names
        })

        const [jsondata, bindata] = await this.__recv();

        if (jsondata.status < 0) {
            throw status.error;
        }
    }

    async leave(...names) {
        this.__send({
            command: "leave",
            name: names
        })

        const [jsondata, bindata] = await this.__recv();

        if (jsondata.status < 0) {
            throw status.error;
        }
    }

    async send(name, ...varnames) {
        const kvpairs = {};

        for (let varname of varnames) {
            kvpairs[varname] = this.workspace[varname];
        }

        await this.send_r(name, kvpairs);
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

    async broadcast(name, ...varnames) {
        const kvpairs = {};

        for (let varname of varnames) {
            kvpairs[varname] = this.workspace[varname];
        }

        await this.broadcast_r(kvpairs);
    }

    async broadcast_r(kvpairs) {
        if (this.connected) {
            await this.connected;
            this.connected = null;
        }

        for (let [varname, value] of Object.entries(kvpairs)) {
            let jsondata = {
                command: "broadcast",
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

    async sync(n = -1) {
        Object.assign(this.workspace, await this.sync_r(n));
    }

    async sync_r(n = -1) {
        if (this.connected) {
            await this.connected;
            this.connected = null;
        }

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

    async wait() {
        this.__send({command: "wait"})

        const [jsondata, bindata] = await this.__recv();

        if (jsondata.status < 0) {
            throw status.error;
        }
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

return {
    dimebloads,
    dimebdumps,
    Complex,
    NDArray,
    DimeClient
};

})();