const WebSocket = require("ws");
const { TextEncoder, TextDecoder } = require("util");

const assert = require("assert").strict;

(async function() {
    let hostname = process.argv[2];
    let port = Number(process.argv[3]);

    let d1 = new dime.DimeClient(hostname, port);
    let d2 = new dime.DimeClient(hostname, port);
    let d3 = new dime.DimeClient(hostname, port);
    let d4 = new dime.DimeClient(hostname, port);

    await d1.join("a", "b", "c", "d", "e");
    await d2.join("a", "b", "c");
    await d3.join("e", "f");
    await d4.join("g");

    let devices = await d1.devices();
    devices.sort();

    assert.deepStrictEqual(devices, ["a", "b", "c", "d", "e", "f", "g"]);

    await d1.leave("a", "b", "c", "d");
    await d2.leave("a", "b");
    await d3.leave("e", "f");

    await d1.join("b");
    await d3.join("h");
    await d4.join("f");

    devices = await d1.devices();
    devices.sort();

    assert.deepStrictEqual(devices, ["b", "c", "e", "f", "g", "h"]);

    await d1.leave("b", "e");
    await d2.leave("c");
    await d3.leave("h");
    await d4.leave("f", "g");

    devices = await d1.devices();

    assert.deepStrictEqual(devices, []);
})().then(process.exit).catch(function(err) { console.log(err); process.exit(1); })
