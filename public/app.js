// ─────────────────────────────────────────────────────────────────────────────
// Blackjack Simulator — Main Application
// ─────────────────────────────────────────────────────────────────────────────

const RESULT_TYPES = ['doublelose','lose','surrender','insurancelose','insurancewin',
                      'push','win','blackjack','doublewin','roudsplayed'];
const TC_BUCKETS   = 65;  // -8.0 to +8.0 in 0.25 steps
const TC_OFFSET    = 32;  // index 32 = TC 0.0

// ── Payoffs per outcome (in base bet units) ───────────────────────────────────
// insurancewin = net 0 (insurance +1 cancels main bet -1)
// insurancelose = -0.5 (side bet lost; main bet outcome counted separately)
const PAYOFFS = {
    win:          1.0,
    blackjack:    1.5,   // updated dynamically for 6:5
    doublewin:    2.0,
    insurancewin: 0.0,
    push:         0.0,
    lose:        -1.0,
    doublelose:  -2.0,
    surrender:   -0.5,
    insurancelose:-0.5,
};

// ── State ─────────────────────────────────────────────────────────────────────
let workers         = [];
let workerResults   = [];  // latest cumulative result arrays per worker
let workerProgress  = [];  // shoes completed per worker
let workerTotal     = [];  // total shoes assigned per worker
let simStartTime    = 0;
let simRunning      = false;
let animFrame       = null;
let chartEV         = null;
let chartFreq       = null;
let pendingRedraw   = false;

// ── DOM helpers ───────────────────────────────────────────────────────────────
const $  = id => document.getElementById(id);
const tc = i  => (i - TC_OFFSET) / 4.0;  // bucket index → true count value

// ── Collapsible sections ──────────────────────────────────────────────────────
document.querySelectorAll('.section-header').forEach(hdr => {
    hdr.addEventListener('click', () => {
        hdr.parentElement.classList.toggle('open');
    });
});

// ── Deck count → update pen slider max ───────────────────────────────────────
function updatePenSlider() {
    const decks  = parseInt(getSelected('decks'));
    const slider = $('pen-slider');
    const maxDec = decks / 2;                     // half the shoe
    const maxQ   = Math.round(maxDec * 4);        // quarter-deck steps
    const curQ   = parseInt(slider.value);
    slider.max   = maxQ;
    if (curQ > maxQ) slider.value = maxQ;
    updatePenLabel();
}

function updatePenLabel() {
    const val  = parseInt($('pen-slider').value);
    const dec  = val / 4;
    $('pen-val').textContent = dec.toFixed(2) + ' decks';
}

$('pen-slider').addEventListener('input', updatePenLabel);
document.querySelectorAll('[data-group="decks"] button').forEach(b =>
    b.addEventListener('click', updatePenSlider)
);

// ── Button groups ─────────────────────────────────────────────────────────────
document.querySelectorAll('.btn-group').forEach(grp => {
    grp.querySelectorAll('button').forEach(btn => {
        btn.addEventListener('click', () => {
            grp.querySelectorAll('button').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            if(grp.dataset.group === 'decks') updatePenSlider();
        });
    });
});

function getSelected(group) {
    return document.querySelector(`[data-group="${group}"] button.active`)?.dataset.value;
}

function setSelected(group, value) {
    const grp = document.querySelector(`[data-group="${group}"]`);
    grp?.querySelectorAll('button').forEach(b => {
        b.classList.toggle('active', b.dataset.value === String(value));
    });
}

// ── Thread slider ─────────────────────────────────────────────────────────────
const maxThreads = navigator.hardwareConcurrency || 4;
const threadSlider = $('thread-slider');
threadSlider.max = maxThreads;
threadSlider.value = Math.max(1, maxThreads - 1);
$('thread-val').textContent = threadSlider.value;
threadSlider.addEventListener('input', () => {
    $('thread-val').textContent = threadSlider.value;
});

// ── Betting strategy table ────────────────────────────────────────────────────
// Each row: { tc: float | null (catch-all), hands: 1-3, bet: number }
let betRows = [
    { tc: null, hands: 1, bet: 25  },  // catch-all (below all thresholds)
    { tc: 1.0,  hands: 1, bet: 50  },
    { tc: 2.0,  hands: 2, bet: 100 },
    { tc: 3.0,  hands: 3, bet: 200 },
];

function tcOptions(selected) {
    let html = '';
    for(let v = -4.0; v <= 8.0; v = Math.round((v + 0.25) * 100) / 100) {
        const sel = (selected === v) ? 'selected' : '';
        html += `<option value="${v}" ${sel}>${v >= 0 ? '+' : ''}${v.toFixed(2)}</option>`;
    }
    return html;
}

function renderBetTable() {
    const tbody = $('bet-rows');
    tbody.innerHTML = '';

    betRows.forEach((row, i) => {
        const isCatchAll = row.tc === null;
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>${isCatchAll
                ? '<span style="color:var(--text-muted);font-size:12px">Always (below all)</span>'
                : `<select class="tc-sel" data-idx="${i}">${tcOptions(row.tc)}</select>`
            }</td>
            <td>
                <div class="btn-group" style="min-width:80px">
                    ${[1,2,3].map(n => `<button class="hands-btn${row.hands===n?' active':''}" data-idx="${i}" data-val="${n}">${n}</button>`).join('')}
                </div>
            </td>
            <td><input type="number" class="bet-inp" data-idx="${i}" value="${row.bet}" min="1" style="width:80px"></td>
            <td>${isCatchAll ? '' : `<button class="del-row-btn" data-idx="${i}">×</button>`}</td>
        `;
        tbody.appendChild(tr);
    });

    // Events
    tbody.querySelectorAll('.tc-sel').forEach(el =>
        el.addEventListener('change', e => {
            betRows[e.target.dataset.idx].tc = parseFloat(e.target.value);
            sortBetRows();
            renderBetTable();
        })
    );
    tbody.querySelectorAll('.hands-btn').forEach(btn =>
        btn.addEventListener('click', e => {
            const { idx, val } = e.currentTarget.dataset;
            betRows[idx].hands = parseInt(val);
            renderBetTable();
        })
    );
    tbody.querySelectorAll('.bet-inp').forEach(el =>
        el.addEventListener('input', e => {
            betRows[e.target.dataset.idx].bet = parseFloat(e.target.value) || 0;
        })
    );
    tbody.querySelectorAll('.del-row-btn').forEach(btn =>
        btn.addEventListener('click', e => {
            betRows.splice(parseInt(e.currentTarget.dataset.idx), 1);
            renderBetTable();
        })
    );
}

function sortBetRows() {
    // Keep catch-all first, sort rest by tc ascending
    const catchAll  = betRows.filter(r => r.tc === null);
    const thresholds = betRows.filter(r => r.tc !== null).sort((a, b) => a.tc - b.tc);
    betRows = [...catchAll, ...thresholds];
}

$('add-bet-row').addEventListener('click', () => {
    const lastTC = betRows.filter(r => r.tc !== null).pop()?.tc ?? 0;
    const newTC  = Math.min(8.0, Math.round((lastTC + 1) * 4) / 4);
    betRows.push({ tc: newTC, hands: 1, bet: 25 });
    sortBetRows();
    renderBetTable();
});

renderBetTable();

// ── Config extraction ─────────────────────────────────────────────────────────
function getConfig() {
    const penQ  = parseInt($('pen-slider').value);
    const decks = parseInt(getSelected('decks'));
    return {
        H17:             $('h17').checked,
        DAS:             $('das').checked,
        RSA:             $('rsa').checked,
        Surrender:       parseInt(getSelected('surrender')),
        BJ65:            $('bj65').checked,
        maxSplit:        parseInt(getSelected('maxsplit')),
        numDecks:        decks,
        deckPen:         Math.round(penQ * 13),  // quarter-decks → cards
        numOtherPlayers: parseInt($('other-players').value) || 0,
    };
}

// ── Bet lookup ────────────────────────────────────────────────────────────────
// Returns {bet, hands} for a given true count value
function getBetForTC(tcVal) {
    // betRows sorted: catch-all first, then ascending thresholds
    // Walk thresholds in descending order and return first match
    const thresholds = betRows.filter(r => r.tc !== null).sort((a, b) => b.tc - a.tc);
    for(const row of thresholds) {
        if(tcVal >= row.tc) return { bet: row.bet, hands: row.hands };
    }
    // Fall back to catch-all
    const base = betRows.find(r => r.tc === null) ?? { bet: 25, hands: 1 };
    return { bet: base.bet, hands: base.hands };
}

// ── EV / Variance calculation ─────────────────────────────────────────────────
function calcStats(merged, bj65) {
    const bjPayout = bj65 ? 1.2 : 1.5;
    const payoffs = { ...PAYOFFS, blackjack: bjPayout };

    let totalRounds = 0;
    for(let i = 0; i < TC_BUCKETS; i++) totalRounds += merged.roudsplayed[i];
    if(!totalRounds) return null;

    const evPerBucket     = new Float64Array(TC_BUCKETS);
    const varPerBucket    = new Float64Array(TC_BUCKETS);
    const freqPerBucket   = new Float64Array(TC_BUCKETS);

    for(let i = 0; i < TC_BUCKETS; i++) {
        const rounds = merged.roudsplayed[i];
        if(!rounds) continue;
        freqPerBucket[i] = rounds / totalRounds;

        let ev = 0, sq = 0;
        for(const [key, payoff] of Object.entries(payoffs)) {
            const count = merged[key]?.[i] ?? 0;
            ev += count * payoff;
            sq += count * payoff * payoff;
        }
        evPerBucket[i]  = ev / rounds;
        varPerBucket[i] = sq / rounds - evPerBucket[i] ** 2;
    }

    // Weighted EV and variance applying betting strategy
    let weightedEV  = 0;
    let weightedVar = 0;

    for(let i = 0; i < TC_BUCKETS; i++) {
        const tcVal = tc(i);
        const freq  = freqPerBucket[i];
        if(!freq) continue;
        const { bet, hands } = getBetForTC(tcVal);
        weightedEV  += evPerBucket[i]  * bet * hands * freq;
        weightedVar += varPerBucket[i] * bet * bet   * hands * freq;
    }

    return {
        evPerBucket,
        varPerBucket,
        freqPerBucket,
        weightedEV,    // $ per round
        weightedVar,   // $² per round
        totalRounds,
        bjPayout,
    };
}

function calcSessionStats(stats) {
    if(!stats) return null;
    const rph      = parseFloat($('rph').value) || 80;
    const bankroll = parseFloat($('bankroll').value) || 0;
    const evPerHour = stats.weightedEV * rph;

    // House edge = -(EV per unit) at flat minimum bet (catch-all row)
    const baseRow  = betRows.find(r => r.tc === null) ?? { bet: 25, hands: 1 };
    const avgEVUnit = calcFlatEdge(stats);

    // Risk of Ruin (Gambler's Ruin approximation, infinite play)
    let ror = null;
    if(bankroll > 0 && stats.weightedVar > 0) {
        if(stats.weightedEV <= 0) {
            ror = 1.0;
        } else {
            ror = Math.exp(-2 * bankroll * stats.weightedEV / stats.weightedVar);
            ror = Math.min(ror, 1.0);
        }
    }

    return { evPerHour, ror, avgEVUnit, rph };
}

// Unweighted EV per unit (no bet sizing, for house edge display)
function calcFlatEdge(stats) {
    let totalRounds = 0, totalEV = 0;
    for(let i = 0; i < TC_BUCKETS; i++) {
        totalRounds += stats.freqPerBucket[i];
        totalEV     += stats.evPerBucket[i] * stats.freqPerBucket[i];
    }
    return totalRounds ? totalEV : 0;
}

// ── Result merging ────────────────────────────────────────────────────────────
function mergeResults() {
    const merged = {};
    for(const key of RESULT_TYPES) merged[key] = new Array(TC_BUCKETS).fill(0);

    for(const wr of workerResults) {
        if(!wr) continue;
        for(const key of RESULT_TYPES) {
            if(!wr[key]) continue;
            for(let i = 0; i < TC_BUCKETS; i++) {
                merged[key][i] += wr[key][i];
            }
        }
    }
    return merged;
}

// ── Charts ────────────────────────────────────────────────────────────────────
function initCharts() {
    const chartDefaults = {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        plugins: {
            legend: { display: false },
            tooltip: {
                backgroundColor: '#21262d',
                borderColor: '#30363d',
                borderWidth: 1,
                titleColor: '#e6edf3',
                bodyColor: '#7d8590',
            }
        },
        scales: {
            x: {
                grid: { color: 'rgba(255,255,255,.06)' },
                ticks: { color: '#7d8590', font: { size: 11 } },
            },
            y: {
                grid: { color: 'rgba(255,255,255,.06)' },
                ticks: { color: '#7d8590', font: { size: 11 } },
            }
        }
    };

    // EV by True Count
    chartEV = new Chart($('chart-ev'), {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                data: [],
                backgroundColor: [],
                borderWidth: 0,
                barPercentage: 0.9,
            }]
        },
        options: {
            ...chartDefaults,
            plugins: {
                ...chartDefaults.plugins,
                tooltip: {
                    ...chartDefaults.plugins.tooltip,
                    callbacks: {
                        label: ctx => ` $${ctx.raw.toFixed(2)} per round`,
                    }
                }
            },
            scales: {
                x: {
                    ...chartDefaults.scales.x,
                    title: { display: true, text: 'True Count', color: '#7d8590', font: { size: 11 } }
                },
                y: {
                    ...chartDefaults.scales.y,
                    title: { display: true, text: 'EV ($)', color: '#7d8590', font: { size: 11 } }
                }
            }
        }
    });

    // Hand frequency by TC
    chartFreq = new Chart($('chart-freq'), {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                data: [],
                backgroundColor: 'rgba(88,166,255,.4)',
                borderColor: 'rgba(88,166,255,.7)',
                borderWidth: 1,
                barPercentage: 0.9,
            }]
        },
        options: {
            ...chartDefaults,
            plugins: {
                ...chartDefaults.plugins,
                tooltip: {
                    ...chartDefaults.plugins.tooltip,
                    callbacks: {
                        label: ctx => ` ${(ctx.raw * 100).toFixed(2)}% of rounds`,
                    }
                }
            },
            scales: {
                x: {
                    ...chartDefaults.scales.x,
                    title: { display: true, text: 'True Count', color: '#7d8590', font: { size: 11 } }
                },
                y: {
                    ...chartDefaults.scales.y,
                    title: { display: true, text: 'Frequency', color: '#7d8590', font: { size: 11 } },
                    ticks: {
                        ...chartDefaults.scales.y.ticks,
                        callback: v => (v * 100).toFixed(1) + '%'
                    }
                }
            }
        }
    });
}

// Only show every 4th TC label to avoid clutter (-8, -7, -6 ... +8)
function tcLabel(i) {
    const v = tc(i);
    return (i % 4 === 0) ? (v >= 0 ? '+' : '') + v.toFixed(1) : '';
}

function updateCharts(stats) {
    if(!chartEV || !stats) return;

    const labels = Array.from({ length: TC_BUCKETS }, (_, i) => tcLabel(i));
    const bj65   = $('bj65').checked;

    // EV chart: value = ev per round at that count × bet×hands
    const evData  = [];
    const evColor = [];
    for(let i = 0; i < TC_BUCKETS; i++) {
        const tcVal = tc(i);
        const { bet, hands } = getBetForTC(tcVal);
        const evDollar = stats.evPerBucket[i] * bet * hands;
        evData.push(parseFloat(evDollar.toFixed(4)));
        evColor.push(evDollar >= 0 ? 'rgba(63,185,80,.75)' : 'rgba(248,81,73,.75)');
    }

    chartEV.data.labels               = labels;
    chartEV.data.datasets[0].data     = evData;
    chartEV.data.datasets[0].backgroundColor = evColor;
    chartEV.update('none');

    // Frequency chart
    const freqData = Array.from(stats.freqPerBucket);
    chartFreq.data.labels           = labels;
    chartFreq.data.datasets[0].data = freqData;
    chartFreq.update('none');
}

// ── Outcome table ─────────────────────────────────────────────────────────────
function updateOutcomeTable(merged) {
    const thead = $('outcome-thead');
    const tbody = $('outcome-tbody');
    thead.innerHTML = '';
    tbody.innerHTML = '';

    const resultKeys = ['win','lose','push','blackjack','doublewin','doublelose','surrender','insurancewin','insurancelose'];
    const headers = ['TC', ...resultKeys.map(k => k.replace('insurance','ins.').replace('double','dbl.')), 'rounds'];

    const hr = document.createElement('tr');
    headers.forEach(h => { const th = document.createElement('th'); th.textContent = h; hr.appendChild(th); });
    thead.appendChild(hr);

    // Find total rounds for % calculation
    const total = merged.roudsplayed.reduce((a, b) => a + b, 0) || 1;

    for(let i = 0; i < TC_BUCKETS; i++) {
        if(!merged.roudsplayed[i]) continue;
        const rounds = merged.roudsplayed[i];
        const tr = document.createElement('tr');

        const tcTd = document.createElement('td');
        tcTd.textContent = (tc(i) >= 0 ? '+' : '') + tc(i).toFixed(2);
        tr.appendChild(tcTd);

        resultKeys.forEach(key => {
            const td = document.createElement('td');
            const pct = ((merged[key]?.[i] ?? 0) / rounds * 100);
            td.textContent = pct.toFixed(1) + '%';
            tr.appendChild(td);
        });

        const rtd = document.createElement('td');
        rtd.textContent = rounds.toLocaleString();
        tr.appendChild(rtd);

        tbody.appendChild(tr);
    }
}

// ── Summary cards update ──────────────────────────────────────────────────────
function updateSummary(stats, session) {
    if(!stats || !session) return;

    const evHr = session.evPerHour;
    const evEl = $('ev-per-hour');
    evEl.textContent = (evHr >= 0 ? '+' : '') + '$' + Math.abs(evHr).toFixed(2);
    evEl.className = 'value ' + (evHr >= 0 ? 'val-pos' : 'val-neg');

    const edgePct = (calcFlatEdge(stats) * 100);
    const edgeEl  = $('house-edge');
    edgeEl.textContent = (edgePct >= 0 ? '+' : '') + edgePct.toFixed(3) + '%';
    edgeEl.className   = 'value ' + (edgePct >= 0 ? 'val-pos' : 'val-neg');

    const rorEl = $('ror');
    if(session.ror !== null) {
        rorEl.textContent = (session.ror * 100).toFixed(1) + '%';
        rorEl.className   = 'value ' + (session.ror > 0.5 ? 'val-neg' : session.ror > 0.2 ? 'val-warn' : 'val-pos');
    } else {
        rorEl.textContent = '—';
        rorEl.className   = 'value val-neutral';
    }

    const hblEl = $('hours-to-broke');
    if(session.ror !== null && evHr < 0) {
        const bankroll = parseFloat($('bankroll').value) || 0;
        const hrs = bankroll > 0 ? (bankroll / Math.abs(evHr)).toFixed(0) : '—';
        hblEl.textContent = hrs !== '—' ? hrs + ' hrs' : '—';
    } else if(session.ror !== null && evHr > 0) {
        hblEl.textContent = 'Positive EV';
        $('hours-to-broke').className = 'value val-pos';
    } else {
        hblEl.textContent = '—';
    }
    $('hours-to-broke').className = 'value val-neutral';
}

// ── UI redraw (throttled via requestAnimationFrame) ───────────────────────────
function scheduleRedraw() {
    if(pendingRedraw) return;
    pendingRedraw = true;
    requestAnimationFrame(() => {
        pendingRedraw = false;
        const merged  = mergeResults();
        const config  = getConfig();
        const stats   = calcStats(merged, config.BJ65);
        const session = calcSessionStats(stats);
        updateSummary(stats, session);
        updateCharts(stats);
        updateOutcomeTable(merged);
    });
}

// ── Progress UI ───────────────────────────────────────────────────────────────
function updateProgress() {
    const totalShoes   = workerTotal.reduce((a, b) => a + b, 0) || 1;
    const doneShoes    = workerProgress.reduce((a, b) => a + b, 0);
    const pct          = doneShoes / totalShoes;
    const elapsed      = (Date.now() - simStartTime) / 1000;
    const shoesPerSec  = elapsed > 0 ? doneShoes / elapsed : 0;
    const remaining    = shoesPerSec > 0 ? (totalShoes - doneShoes) / shoesPerSec : 0;

    $('progress-bar-fill').style.width = (pct * 100).toFixed(1) + '%';
    $('progress-pct').textContent      = (pct * 100).toFixed(0) + '%';

    const statsEl = $('progress-stats');
    const fmt = n => n >= 1e6 ? (n/1e6).toFixed(1)+'M' : n >= 1e3 ? (n/1e3).toFixed(0)+'K' : n;
    statsEl.textContent = `${fmt(doneShoes)} / ${fmt(totalShoes)} shoes  ·  ${shoesPerSec.toFixed(0)}/s  ·  ETA ${fmtTime(remaining)}`;

    // Per-worker mini bars
    const wbars = document.querySelectorAll('.worker-bar-fill');
    wbars.forEach((bar, i) => {
        bar.style.width = workerTotal[i] ? (workerProgress[i] / workerTotal[i] * 100) + '%' : '0%';
    });
}

function fmtTime(seconds) {
    if(!isFinite(seconds) || seconds > 3600) return '>1h';
    const m = Math.floor(seconds / 60);
    const s = Math.floor(seconds % 60);
    return m > 0 ? `${m}m ${s}s` : `${s}s`;
}

// ── Worker message handler ────────────────────────────────────────────────────
function handleWorkerMessage(e, workerId) {
    const { type, shoesRun, results } = e.data;

    if(type === 'progress') {
        workerProgress[workerId] = shoesRun;
        if(results) workerResults[workerId] = results;
        updateProgress();
        scheduleRedraw();
    } else if(type === 'done') {
        workerProgress[workerId] = workerTotal[workerId];
        updateProgress();
        scheduleRedraw();
        checkAllDone();
    } else if(type === 'error') {
        console.error(`Worker ${workerId} error:`, e.data.message);
        finishSim();
    }
}

function checkAllDone() {
    if(workerProgress.every((p, i) => p >= workerTotal[i])) {
        finishSim();
    }
}

function finishSim() {
    simRunning = false;
    workers.forEach(w => w.terminate());
    workers = [];
    $('run-btn').disabled = false;
    $('run-btn').textContent = 'Run Simulation';
    $('cancel-btn').style.display = 'none';
    $('progress-bar-fill').style.width = '100%';
    $('progress-pct').textContent = '100%';
    $('progress-stats').textContent = 'Complete — ' + (((Date.now()-simStartTime)/1000)).toFixed(1) + 's';
    scheduleRedraw();
}

// ── Export / cache hookup ─────────────────────────────────────────────────────
$('export-btn').addEventListener('click', () => {
    const merged = mergeResults();
    const config = getConfig();
    const blob = {
        meta: {
            config,
            betStrategy: betRows,
            generatedAt: new Date().toISOString(),
            version: 1,
        },
        results: merged,
    };
    const url  = URL.createObjectURL(new Blob([JSON.stringify(blob, null, 2)], { type: 'application/json' }));
    const a    = document.createElement('a');
    a.href     = url;
    a.download = 'bjsim-results.json';
    a.click();
    URL.revokeObjectURL(url);
});

// Future: POST to server
// async function cacheResults(blob) {
//   await fetch('/api/cache', { method: 'POST', body: JSON.stringify(blob),
//     headers: { 'Content-Type': 'application/json' } });
// }

// ── Run / Cancel ─────────────────────────────────────────────────────────────
$('run-btn').addEventListener('click', startSim);
$('cancel-btn').addEventListener('click', () => {
    workers.forEach(w => w.terminate());
    workers = [];
    finishSim();
});

function startSim() {
    if(simRunning) return;
    simRunning = true;

    const config    = getConfig();
    const numShoes  = parseInt(getSelected('shoes'));
    const numWorkers = parseInt($('thread-slider').value);
    const batchSize  = 500;

    // Reset state
    workerResults  = new Array(numWorkers).fill(null);
    workerProgress = new Array(numWorkers).fill(0);
    workerTotal    = [];

    // Distribute shoes across workers
    const base  = Math.floor(numShoes / numWorkers);
    const extra = numShoes % numWorkers;
    for(let i = 0; i < numWorkers; i++) {
        workerTotal.push(base + (i < extra ? 1 : 0));
    }

    // Build per-worker mini-bars
    const wbarsEl = $('worker-bars');
    wbarsEl.innerHTML = '';
    for(let i = 0; i < numWorkers; i++) {
        wbarsEl.innerHTML += `<div class="worker-bar"><div class="worker-bar-fill" id="wbar-${i}"></div></div>`;
    }

    // Reset charts / summary
    $('progress-bar-fill').style.width = '0%';
    $('progress-pct').textContent = '0%';
    $('progress-stats').textContent = 'Starting workers…';
    $('progress-panel').style.display = 'flex';
    $('results-panel').style.display  = 'block';
    $('placeholder').style.display    = 'none';

    $('run-btn').disabled = true;
    $('run-btn').textContent = 'Running…';
    $('cancel-btn').style.display = 'block';

    simStartTime = Date.now();
    workers = [];

    for(let i = 0; i < numWorkers; i++) {
        const w = new Worker('worker.js');
        const id = i;
        w.onmessage = e => handleWorkerMessage(e, id);
        w.onerror   = err => { console.error('Worker error', err); finishSim(); };
        w.postMessage({ type: 'start', config, numShoes: workerTotal[i], batchSize, workerId: id });
        workers.push(w);
    }
}

// ── Init ──────────────────────────────────────────────────────────────────────
updatePenSlider();
initCharts();
