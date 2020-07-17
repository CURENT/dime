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
