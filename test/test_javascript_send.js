const WebSocket = require("ws");
const { TextEncoder, TextDecoder } = require("util");

function randmatrix(n, m) {
    let array = new Float64Array(n * m);

    for (let i = 0; i < n * m; i++) {
        array[i] = Math.random();
    }

    return new dime.NDArray('F', [n, m], false, array);
}

const assert = require("assert").strict;

(async function() {
    let hostname = process.argv[2];
    let port = Number(process.argv[3]);

    let d1 = new dime.DimeClient(hostname, port);
    let d2 = new dime.DimeClient(hostname, port);
    let d3 = new dime.DimeClient(hostname, port);

    await d1.join("d1");
    await d2.join("d2");
    await d3.join("d3");

    d1.workspace.a = randmatrix(500, 500);
    d1.workspace.b = randmatrix(500, 500);
    d1.workspace.c = randmatrix(500, 500);

    await d1.send("d2", "a", "b", "c");
    await d2.sync();

    assert.deepStrictEqual(d1.workspace.a, d2.workspace.a);
    assert.deepStrictEqual(d1.workspace.b, d2.workspace.b);
    assert.deepStrictEqual(d1.workspace.c, d2.workspace.c);

    d3.workspace.a = null;
    d3.workspace.b = null;
    d3.workspace.c = null;

    await d3.sync();

    assert.notDeepStrictEqual(d1.workspace.a, d3.workspace.a);
    assert.notDeepStrictEqual(d1.workspace.b, d3.workspace.b);
    assert.notDeepStrictEqual(d1.workspace.c, d3.workspace.c);
})().then(process.exit).catch(function(err) { console.log(err); process.exit(1); })
