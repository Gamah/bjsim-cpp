// Blackjack simulator Web Worker
// Each worker loads its own WASM instance and runs a slice of the total shoes.
// Reports partial results after each batch so the main thread can update the UI in real time.

importScripts('bjsim.js');

let module = null;

self.onmessage = async (e) => {
    const { type, config, numShoes, batchSize, workerId } = e.data;

    if (type !== 'start') return;

    try {
        // Initialize WASM module (Emscripten factory returns a Promise)
        module = await BJSim();

        // Configure simulation
        module.ccall('bjsim_configure', null, ['string'], [JSON.stringify(config)]);

        let shoesRun = 0;
        while (shoesRun < numShoes) {
            const batch = Math.min(batchSize, numShoes - shoesRun);
            module.ccall('bjsim_run_batch', null, ['number'], [batch]);
            shoesRun += batch;

            // Grab cumulative results from this worker and send to main thread
            const resultJson = module.ccall('bjsim_get_results', 'string', [], []);
            self.postMessage({
                type: 'progress',
                workerId,
                shoesRun,
                totalShoes: numShoes,
                results: JSON.parse(resultJson)
            });
        }

        self.postMessage({ type: 'done', workerId });

    } catch (err) {
        self.postMessage({ type: 'error', workerId, message: err.message });
    }
};
