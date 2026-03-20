// ─────────────────────────────────────────────────────────────────────────────
// Blackjack Simulator — Main Application
// ─────────────────────────────────────────────────────────────────────────────

const RESULT_TYPES = ['doublelose','lose','surrender','insurancelose','insurancewin',
                      'push','win','blackjack','doublewin','roudsplayed'];
const TC_BUCKETS   = 65;   // -8.0 to +8.0 in 0.25 steps
const TC_OFFSET    = 32;   // index 32 = TC 0.0
const LS_KEY       = 'bjsim_profiles';

const PAYOFFS = {
    win:           1.0,
    blackjack:     1.5,   // overridden dynamically for 6:5
    doublewin:     2.0,
    insurancewin:  0.0,
    push:          0.0,
    lose:         -1.0,
    doublelose:   -2.0,
    surrender:    -0.5,
    insurancelose:-0.5,
};

// ── Simulation state ──────────────────────────────────────────────────────────
let workers        = [];
let workerResults  = [];
let workerProgress = [];
let workerTotal    = [];
let simStartTime   = 0;
let simRunning     = false;
let hasRanSim      = false;  // true once first sim completes or results are loaded
let resimTimer     = null;   // debounce handle for auto-resim

// ── Chart handles ─────────────────────────────────────────────────────────────
let chartEV        = null;
let chartFreq      = null;
let pendingRedraw  = false;

// ── DOM helpers ───────────────────────────────────────────────────────────────
const $  = id => document.getElementById(id);
const tc = i  => (i - TC_OFFSET) / 4.0;

function fmtShoes(n) {
    if (!n) return 'no results';
    if (n >= 1e9) return (n / 1e9).toFixed(0) + 'B shoes';
    if (n >= 1e6) return (n / 1e6).toFixed(0) + 'M shoes';
    if (n >= 1e3) return (n / 1e3).toFixed(0) + 'K shoes';
    return n + ' shoes';
}

// ── Collapsible sections ──────────────────────────────────────────────────────
document.querySelectorAll('.section-header').forEach(hdr => {
    hdr.addEventListener('click', () => hdr.parentElement.classList.toggle('open'));
});

// ── Deck count → update pen slider max ───────────────────────────────────────
function updatePenSlider() {
    const decks  = parseInt(getSelected('decks'));
    const slider = $('pen-slider');
    const maxQ   = Math.round((decks / 2) * 4);
    const curQ   = parseInt(slider.value);
    slider.max   = maxQ;
    if (curQ > maxQ) slider.value = maxQ;
    updatePenLabel();
}

function updatePenLabel() {
    const val = parseInt($('pen-slider').value);
    $('pen-val').textContent = (val / 4).toFixed(2) + ' decks';
}

// ── Button groups ─────────────────────────────────────────────────────────────
document.querySelectorAll('.btn-group').forEach(grp => {
    grp.querySelectorAll('button').forEach(btn => {
        btn.addEventListener('click', () => {
            grp.querySelectorAll('button').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
        });
    });
});

function getSelected(group) {
    return document.querySelector(`[data-group="${group}"] button.active`)?.dataset.value;
}

function setSelected(group, value) {
    document.querySelector(`[data-group="${group}"]`)
        ?.querySelectorAll('button')
        .forEach(b => b.classList.toggle('active', b.dataset.value === String(value)));
}

// ── Thread slider ─────────────────────────────────────────────────────────────
const maxThreads   = navigator.hardwareConcurrency || 4;
const threadSlider = $('thread-slider');
threadSlider.max   = maxThreads;
threadSlider.value = Math.max(1, maxThreads - 1);
$('thread-val').textContent = threadSlider.value;
threadSlider.addEventListener('input', () => { $('thread-val').textContent = threadSlider.value; });

// ── RPH slider ────────────────────────────────────────────────────────────────
$('rph').addEventListener('input', () => {
    $('rph-val').textContent = $('rph').value;
    scheduleRedraw();
});

// ── Bankroll ──────────────────────────────────────────────────────────────────
$('bankroll').addEventListener('input', scheduleRedraw);

// ── BJ65 toggle ───────────────────────────────────────────────────────────────
// BJ65 only affects the JS-side EV calculation, not the simulation — instant recalc only
$('bj65').addEventListener('change', scheduleRedraw);

// ── Rule inputs that require re-simulation ────────────────────────────────────
function triggerAutoResim() {
    if (!hasRanSim) return;
    clearTimeout(resimTimer);
    resimTimer = setTimeout(() => startSim(), 800);
}

$('h17').addEventListener('change', triggerAutoResim);
$('das').addEventListener('change', triggerAutoResim);
$('rsa').addEventListener('change', triggerAutoResim);
$('other-players').addEventListener('input', triggerAutoResim);
$('pen-slider').addEventListener('input', () => { updatePenLabel(); triggerAutoResim(); });

// Deck count and button-group rules: wire after DOMContentLoaded (groups already init above)
document.querySelectorAll('[data-group="decks"] button').forEach(b =>
    b.addEventListener('click', () => { updatePenSlider(); triggerAutoResim(); })
);
document.querySelectorAll('[data-group="surrender"] button').forEach(b =>
    b.addEventListener('click', triggerAutoResim)
);
document.querySelectorAll('[data-group="maxsplit"] button').forEach(b =>
    b.addEventListener('click', triggerAutoResim)
);

// ── Betting strategy table ────────────────────────────────────────────────────
let betRows = [
    { tc: null, hands: 1, bet: 25  },
    { tc: 1.0,  hands: 1, bet: 50  },
    { tc: 2.0,  hands: 2, bet: 100 },
    { tc: 3.0,  hands: 3, bet: 200 },
];

function tcOptions(selected) {
    let html = '';
    for (let v = -4.0; v <= 8.0; v = Math.round((v + 0.25) * 100) / 100) {
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
                ? '<span class="catch-all-label">Below all</span>'
                : `<select class="tc-sel" data-idx="${i}">${tcOptions(row.tc)}</select>`
            }</td>
            <td>
                <div class="btn-group hands-grp">
                    ${[1,2,3].map(n =>
                        `<button class="hands-btn${row.hands===n?' active':''}" data-idx="${i}" data-val="${n}">${n}</button>`
                    ).join('')}
                </div>
            </td>
            <td class="bet-cell">
                <div class="bet-cell-inner">
                    <input type="range"   class="bet-slider" data-idx="${i}"
                           min="0" max="2000" step="5" value="${Math.min(2000, row.bet)}">
                    <input type="number"  class="bet-inp"    data-idx="${i}"
                           value="${row.bet}" min="0" style="width:58px">
                </div>
            </td>
            <td>${isCatchAll ? '' : `<button class="del-row-btn" data-idx="${i}" title="Remove">×</button>`}</td>
        `;
        tbody.appendChild(tr);
    });

    tbody.querySelectorAll('.tc-sel').forEach(el =>
        el.addEventListener('change', e => {
            betRows[e.target.dataset.idx].tc = parseFloat(e.target.value);
            sortBetRows(); renderBetTable();
        })
    );
    tbody.querySelectorAll('.hands-btn').forEach(btn =>
        btn.addEventListener('click', e => {
            betRows[e.currentTarget.dataset.idx].hands = parseInt(e.currentTarget.dataset.val);
            renderBetTable(); scheduleRedraw();
        })
    );
    tbody.querySelectorAll('.bet-slider').forEach(el =>
        el.addEventListener('input', e => {
            const idx = e.target.dataset.idx;
            betRows[idx].bet = parseFloat(e.target.value) || 0;
            const inp = e.target.closest('td').querySelector('.bet-inp');
            if (inp) inp.value = betRows[idx].bet;
            scheduleRedraw();
        })
    );
    tbody.querySelectorAll('.bet-inp').forEach(el =>
        el.addEventListener('input', e => {
            const idx = e.target.dataset.idx;
            betRows[idx].bet = parseFloat(e.target.value) || 0;
            const slider = e.target.closest('td').querySelector('.bet-slider');
            if (slider) slider.value = Math.min(2000, betRows[idx].bet);
            scheduleRedraw();
        })
    );
    tbody.querySelectorAll('.del-row-btn').forEach(btn =>
        btn.addEventListener('click', e => {
            betRows.splice(parseInt(e.currentTarget.dataset.idx), 1);
            renderBetTable(); scheduleRedraw();
        })
    );
}

function sortBetRows() {
    const catchAll   = betRows.filter(r => r.tc === null);
    const thresholds = betRows.filter(r => r.tc !== null).sort((a, b) => a.tc - b.tc);
    betRows = [...catchAll, ...thresholds];
}

$('add-bet-row').addEventListener('click', () => {
    const lastTC = betRows.filter(r => r.tc !== null).pop()?.tc ?? 0;
    const newTC  = Math.min(8.0, Math.round((lastTC + 1) * 4) / 4);
    betRows.push({ tc: newTC, hands: 1, bet: 25 });
    sortBetRows(); renderBetTable();
});

renderBetTable();

// ── Config extraction ─────────────────────────────────────────────────────────
function getConfig() {
    return {
        H17:             $('h17').checked,
        DAS:             $('das').checked,
        RSA:             $('rsa').checked,
        Surrender:       parseInt(getSelected('surrender')),
        BJ65:            $('bj65').checked,
        maxSplit:        parseInt(getSelected('maxsplit')),
        numDecks:        parseInt(getSelected('decks')),
        deckPen:         Math.round(parseInt($('pen-slider').value) * 13),
        numOtherPlayers: parseInt($('other-players').value) || 0,
    };
}

// ── Bet lookup ────────────────────────────────────────────────────────────────
function getBetForTC(tcVal) {
    const thresholds = betRows.filter(r => r.tc !== null).sort((a, b) => b.tc - a.tc);
    for (const row of thresholds) {
        if (tcVal >= row.tc) return { bet: row.bet, hands: row.hands };
    }
    const base = betRows.find(r => r.tc === null) ?? { bet: 25, hands: 1 };
    return { bet: base.bet, hands: base.hands };
}

// ── EV / Variance calculation ─────────────────────────────────────────────────
function calcStats(merged, bj65) {
    const bjPayout = bj65 ? 1.2 : 1.5;
    const payoffs  = { ...PAYOFFS, blackjack: bjPayout };

    let totalRounds = 0;
    for (let i = 0; i < TC_BUCKETS; i++) totalRounds += merged.roudsplayed[i];
    if (!totalRounds) return null;

    const evPerBucket   = new Float64Array(TC_BUCKETS);
    const varPerBucket  = new Float64Array(TC_BUCKETS);
    const freqPerBucket = new Float64Array(TC_BUCKETS);

    for (let i = 0; i < TC_BUCKETS; i++) {
        const rounds = merged.roudsplayed[i];
        if (!rounds) continue;
        freqPerBucket[i] = rounds / totalRounds;

        let ev = 0, sq = 0;
        for (const [key, payoff] of Object.entries(payoffs)) {
            const count = merged[key]?.[i] ?? 0;
            ev += count * payoff;
            sq += count * payoff * payoff;
        }
        evPerBucket[i]  = ev / rounds;
        varPerBucket[i] = sq / rounds - evPerBucket[i] ** 2;
    }

    let weightedEV = 0, weightedVar = 0;
    for (let i = 0; i < TC_BUCKETS; i++) {
        const freq = freqPerBucket[i];
        if (!freq) continue;
        const tcVal       = tc(i);
        const { bet, hands } = getBetForTC(tcVal);
        weightedEV  += evPerBucket[i]  * bet * hands * freq;
        weightedVar += varPerBucket[i] * bet * bet   * hands * freq;
    }

    return { evPerBucket, varPerBucket, freqPerBucket, weightedEV, weightedVar, totalRounds, bjPayout };
}

function calcSessionStats(stats) {
    if (!stats) return null;
    const rph      = parseInt($('rph').value) || 80;
    const bankroll = parseFloat($('bankroll').value) || 0;
    const evPerHour = stats.weightedEV * rph;

    let ror = null;
    if (bankroll > 0 && stats.weightedVar > 0) {
        ror = stats.weightedEV <= 0
            ? 1.0
            : Math.min(1.0, Math.exp(-2 * bankroll * stats.weightedEV / stats.weightedVar));
    }
    return { evPerHour, ror, rph };
}

function calcFlatEdge(stats) {
    let ev = 0;
    for (let i = 0; i < TC_BUCKETS; i++) ev += stats.evPerBucket[i] * stats.freqPerBucket[i];
    return ev;
}

// ── Result merging ────────────────────────────────────────────────────────────
function mergeResults() {
    const merged = {};
    for (const key of RESULT_TYPES) merged[key] = new Array(TC_BUCKETS).fill(0);
    for (const wr of workerResults) {
        if (!wr) continue;
        for (const key of RESULT_TYPES) {
            if (!wr[key]) continue;
            for (let i = 0; i < TC_BUCKETS; i++) merged[key][i] += wr[key][i];
        }
    }
    return merged;
}

// ── Charts ────────────────────────────────────────────────────────────────────
function initCharts() {
    const base = {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        plugins: {
            legend: { display: false },
            tooltip: {
                backgroundColor: '#21262d', borderColor: '#30363d', borderWidth: 1,
                titleColor: '#e6edf3', bodyColor: '#7d8590',
            }
        },
        scales: {
            x: { grid: { color: 'rgba(255,255,255,.06)' }, ticks: { color: '#7d8590', font: { size: 11 } } },
            y: { grid: { color: 'rgba(255,255,255,.06)' }, ticks: { color: '#7d8590', font: { size: 11 } } },
        }
    };

    chartEV = new Chart($('chart-ev'), {
        type: 'bar',
        data: { labels: [], datasets: [{ data: [], backgroundColor: [], borderWidth: 0, barPercentage: 0.9 }] },
        options: {
            ...base,
            plugins: { ...base.plugins, tooltip: { ...base.plugins.tooltip,
                callbacks: { label: ctx => ` $${ctx.raw.toFixed(2)} / round` } } },
            scales: {
                x: { ...base.scales.x, title: { display: true, text: 'True Count', color: '#7d8590', font: { size: 11 } } },
                y: { ...base.scales.y, title: { display: true, text: 'EV ($)',      color: '#7d8590', font: { size: 11 } } },
            }
        }
    });

    chartFreq = new Chart($('chart-freq'), {
        type: 'bar',
        data: { labels: [], datasets: [{ data: [], backgroundColor: 'rgba(88,166,255,.4)',
                borderColor: 'rgba(88,166,255,.7)', borderWidth: 1, barPercentage: 0.9 }] },
        options: {
            ...base,
            plugins: { ...base.plugins, tooltip: { ...base.plugins.tooltip,
                callbacks: { label: ctx => ` ${(ctx.raw * 100).toFixed(2)}% of rounds` } } },
            scales: {
                x: { ...base.scales.x, title: { display: true, text: 'True Count',  color: '#7d8590', font: { size: 11 } } },
                y: { ...base.scales.y, title: { display: true, text: 'Frequency',   color: '#7d8590', font: { size: 11 } },
                    ticks: { ...base.scales.y.ticks, callback: v => (v * 100).toFixed(1) + '%' } },
            }
        }
    });
}

function tcLabel(i) {
    const v = tc(i);
    return (i % 4 === 0) ? (v >= 0 ? '+' : '') + v.toFixed(1) : '';
}

function updateCharts(stats) {
    if (!chartEV || !stats) return;
    const labels   = Array.from({ length: TC_BUCKETS }, (_, i) => tcLabel(i));
    const evData   = [], evColor = [];

    for (let i = 0; i < TC_BUCKETS; i++) {
        const { bet, hands } = getBetForTC(tc(i));
        const evDollar = stats.evPerBucket[i] * bet * hands;
        evData.push(parseFloat(evDollar.toFixed(4)));
        evColor.push(evDollar >= 0 ? 'rgba(63,185,80,.75)' : 'rgba(248,81,73,.75)');
    }

    chartEV.data.labels = chartFreq.data.labels = labels;
    chartEV.data.datasets[0].data = evData;
    chartEV.data.datasets[0].backgroundColor = evColor;
    chartEV.update('none');

    chartFreq.data.datasets[0].data = Array.from(stats.freqPerBucket);
    chartFreq.update('none');
}

// ── Outcome table ─────────────────────────────────────────────────────────────
function updateOutcomeTable(merged) {
    const thead = $('outcome-thead');
    const tbody = $('outcome-tbody');
    thead.innerHTML = tbody.innerHTML = '';

    const keys = ['win','lose','push','blackjack','doublewin','doublelose','surrender','insurancewin','insurancelose'];
    const hr = document.createElement('tr');
    ['TC', ...keys.map(k => k.replace('insurance','ins.').replace('double','dbl.')), 'rounds']
        .forEach(h => { const th = document.createElement('th'); th.textContent = h; hr.appendChild(th); });
    thead.appendChild(hr);

    for (let i = 0; i < TC_BUCKETS; i++) {
        const rounds = merged.roudsplayed[i];
        if (!rounds) continue;
        const tr = document.createElement('tr');
        const tcTd = document.createElement('td');
        tcTd.textContent = (tc(i) >= 0 ? '+' : '') + tc(i).toFixed(2);
        tr.appendChild(tcTd);
        keys.forEach(key => {
            const td = document.createElement('td');
            td.textContent = ((merged[key]?.[i] ?? 0) / rounds * 100).toFixed(1) + '%';
            tr.appendChild(td);
        });
        const rtd = document.createElement('td');
        rtd.textContent = rounds.toLocaleString();
        tr.appendChild(rtd);
        tbody.appendChild(tr);
    }
}

// ── Summary cards ─────────────────────────────────────────────────────────────
function updateSummary(stats, session) {
    if (!stats || !session) return;

    const evHr = session.evPerHour;
    const evEl = $('ev-per-hour');
    evEl.textContent = (evHr >= 0 ? '+' : '') + '$' + Math.abs(evHr).toFixed(2);
    evEl.className   = 'value ' + (evHr >= 0 ? 'val-pos' : 'val-neg');

    const edge    = calcFlatEdge(stats) * 100;
    const edgeEl  = $('house-edge');
    edgeEl.textContent = (edge >= 0 ? '+' : '') + edge.toFixed(3) + '%';
    edgeEl.className   = 'value ' + (edge >= 0 ? 'val-pos' : 'val-neg');

    const rorEl = $('ror');
    if (session.ror !== null) {
        rorEl.textContent = (session.ror * 100).toFixed(1) + '%';
        rorEl.className   = 'value ' + (session.ror > 0.5 ? 'val-neg' : session.ror > 0.2 ? 'val-warn' : 'val-pos');
    } else {
        rorEl.textContent = '—';
        rorEl.className   = 'value val-neutral';
    }

    const bankroll = parseFloat($('bankroll').value) || 0;
    const hblEl    = $('hours-to-broke');
    if (bankroll > 0 && evHr < 0) {
        hblEl.textContent = (bankroll / Math.abs(evHr)).toFixed(0) + ' hrs';
        hblEl.className   = 'value val-neutral';
    } else if (bankroll > 0 && evHr > 0) {
        hblEl.textContent = 'Positive EV';
        hblEl.className   = 'value val-pos';
    } else {
        hblEl.textContent = '—';
        hblEl.className   = 'value val-neutral';
    }
}

// ── Redraw (throttled via rAF) ────────────────────────────────────────────────
function scheduleRedraw() {
    if (pendingRedraw) return;
    pendingRedraw = true;
    requestAnimationFrame(() => {
        pendingRedraw = false;
        const merged  = mergeResults();
        const stats   = calcStats(merged, $('bj65').checked);
        const session = calcSessionStats(stats);
        updateSummary(stats, session);
        updateCharts(stats);
        updateOutcomeTable(merged);
    });
}

// ── Progress UI ───────────────────────────────────────────────────────────────
function updateProgress() {
    const totalShoes = workerTotal.reduce((a, b) => a + b, 0) || 1;
    const doneShoes  = workerProgress.reduce((a, b) => a + b, 0);
    const pct        = doneShoes / totalShoes;
    const elapsed    = (Date.now() - simStartTime) / 1000;
    const rate       = elapsed > 0 ? doneShoes / elapsed : 0;
    const remaining  = rate > 0 ? (totalShoes - doneShoes) / rate : 0;

    $('progress-bar-fill').style.width = (pct * 100).toFixed(1) + '%';
    $('progress-pct').textContent      = (pct * 100).toFixed(0) + '%';

    const fmt = n => n >= 1e9 ? (n/1e9).toFixed(1)+'B' : n >= 1e6 ? (n/1e6).toFixed(1)+'M' : n >= 1e3 ? (n/1e3).toFixed(0)+'K' : String(n);
    $('progress-stats').textContent =
        `${fmt(doneShoes)} / ${fmt(totalShoes)} shoes  ·  ${fmt(Math.round(rate))}/s  ·  ETA ${fmtTime(remaining)}`;

    document.querySelectorAll('.worker-bar-fill').forEach((bar, i) => {
        bar.style.width = workerTotal[i] ? (workerProgress[i] / workerTotal[i] * 100) + '%' : '0%';
    });
}

function fmtTime(s) {
    if (!isFinite(s) || s > 7200) return '>2h';
    if (s > 3600) return Math.floor(s/3600) + 'h ' + Math.floor((s%3600)/60) + 'm';
    const m = Math.floor(s / 60), sec = Math.floor(s % 60);
    return m > 0 ? `${m}m ${sec}s` : `${sec}s`;
}

// ── Worker message handling ───────────────────────────────────────────────────
function handleWorkerMessage(e, workerId) {
    const { type, shoesRun, results } = e.data;

    if (type === 'progress') {
        workerProgress[workerId] = shoesRun;
        if (results) workerResults[workerId] = results;
        updateProgress();
        scheduleRedraw();
    } else if (type === 'done') {
        workerProgress[workerId] = workerTotal[workerId];
        updateProgress();
        checkAllDone();
    } else if (type === 'error') {
        console.error(`Worker ${workerId}:`, e.data.message);
        finishSim();
    }
}

function checkAllDone() {
    if (workerProgress.every((p, i) => p >= workerTotal[i])) finishSim();
}

function finishSim() {
    simRunning = false;
    hasRanSim  = true;
    workers.forEach(w => w.terminate());
    workers = [];

    $('run-btn').disabled        = false;
    $('run-btn').textContent     = 'Run Simulation';
    $('cancel-btn').style.display = 'none';
    $('progress-bar-fill').style.width = '100%';
    $('progress-pct').textContent      = '100%';
    $('progress-stats').textContent    = `Complete — ${((Date.now()-simStartTime)/1000).toFixed(1)}s`;

    scheduleRedraw();
    refreshProfileList();  // update shoe count in profiles
}

// ── Run / Cancel ──────────────────────────────────────────────────────────────
$('run-btn').addEventListener('click', startSim);
$('cancel-btn').addEventListener('click', () => {
    workers.forEach(w => w.terminate());
    workers = [];
    finishSim();
});

function startSim() {
    // Cancel any currently running sim before starting a new one
    if (simRunning) {
        workers.forEach(w => w.terminate());
        workers    = [];
        simRunning = false;
    }
    clearTimeout(resimTimer);
    simRunning = true;

    const config     = getConfig();
    const numShoes   = parseInt(getSelected('shoes'));
    const numWorkers = parseInt($('thread-slider').value);
    const batchSize  = 2000;

    workerResults  = new Array(numWorkers).fill(null);
    workerProgress = new Array(numWorkers).fill(0);
    workerTotal    = [];

    const base  = Math.floor(numShoes / numWorkers);
    const extra = numShoes % numWorkers;
    for (let i = 0; i < numWorkers; i++) workerTotal.push(base + (i < extra ? 1 : 0));

    const wbarsEl = $('worker-bars');
    wbarsEl.innerHTML = '';
    for (let i = 0; i < numWorkers; i++)
        wbarsEl.innerHTML += `<div class="worker-bar"><div class="worker-bar-fill"></div></div>`;

    $('progress-bar-fill').style.width = '0%';
    $('progress-pct').textContent      = '0%';
    $('progress-stats').textContent    = 'Starting workers…';
    $('progress-panel').style.display  = 'flex';
    $('results-panel').style.display   = 'flex';
    $('placeholder').style.display     = 'none';

    $('run-btn').disabled        = true;
    $('run-btn').textContent     = 'Running…';
    $('cancel-btn').style.display = 'block';

    simStartTime = Date.now();
    workers = [];

    for (let i = 0; i < numWorkers; i++) {
        const w  = new Worker('worker.js');
        const id = i;
        w.onmessage = e => handleWorkerMessage(e, id);
        w.onerror   = err => { console.error('Worker error', err); finishSim(); };
        w.postMessage({ type: 'start', config, numShoes: workerTotal[i], batchSize, workerId: id });
        workers.push(w);
    }
}

// ── Profiles (localStorage) ───────────────────────────────────────────────────
function loadProfiles() {
    try { return JSON.parse(localStorage.getItem(LS_KEY)) || []; }
    catch { return []; }
}

function saveProfiles(profiles) {
    localStorage.setItem(LS_KEY, JSON.stringify(profiles));
}

function currentTotalShoes() {
    const doneShoes = workerProgress.reduce((a, b) => a + b, 0);
    return doneShoes > 0 ? doneShoes : null;
}

function captureSettings() {
    return {
        decks:        parseInt(getSelected('decks')),
        penQ:         parseInt($('pen-slider').value),
        h17:          $('h17').checked,
        bj65:         $('bj65').checked,
        das:          $('das').checked,
        rsa:          $('rsa').checked,
        surrender:    parseInt(getSelected('surrender')),
        maxsplit:     parseInt(getSelected('maxsplit')),
        otherPlayers: parseInt($('other-players').value) || 0,
        shoes:        parseInt(getSelected('shoes')),
        threads:      parseInt($('thread-slider').value),
        rph:          parseInt($('rph').value),
        bankroll:     parseFloat($('bankroll').value) || 0,
        betRows:      JSON.parse(JSON.stringify(betRows)),
    };
}

function applySettings(s) {
    setSelected('decks',    s.decks);
    setSelected('shoes',    s.shoes);
    setSelected('surrender',s.surrender);
    setSelected('maxsplit', s.maxsplit);

    $('pen-slider').max   = Math.round((s.decks / 2) * 4);
    $('pen-slider').value = s.penQ;
    updatePenLabel();

    $('h17').checked         = s.h17;
    $('bj65').checked        = s.bj65;
    $('das').checked         = s.das;
    $('rsa').checked         = s.rsa;
    $('other-players').value = s.otherPlayers;
    $('thread-slider').value = s.threads;
    $('thread-val').textContent = s.threads;
    $('rph').value           = s.rph;
    $('rph-val').textContent = s.rph;
    $('bankroll').value      = s.bankroll || '';

    betRows = JSON.parse(JSON.stringify(s.betRows));
    renderBetTable();
}

function refreshProfileList() {
    const profiles = loadProfiles();
    const sel      = $('profile-select');
    const current  = sel.value;

    sel.innerHTML = '<option value="">— select a profile —</option>';
    profiles.forEach(p => {
        const opt   = document.createElement('option');
        opt.value   = p.id;
        const shoes = p.totalShoes ? fmtShoes(p.totalShoes) : 'no results';
        opt.textContent = `${p.name}  (${shoes})`;
        sel.appendChild(opt);
    });
    if (current) sel.value = current;
}

$('profile-save-btn').addEventListener('click', () => {
    const name = $('profile-name').value.trim() || 'Untitled';
    const merged = mergeResults();
    const total  = currentTotalShoes();

    const profile = {
        id:         crypto.randomUUID(),
        name,
        savedAt:    new Date().toISOString(),
        settings:   captureSettings(),
        results:    total ? merged : null,
        totalShoes: total,
    };

    const profiles = loadProfiles();
    profiles.unshift(profile);
    saveProfiles(profiles);
    refreshProfileList();
    $('profile-select').value = profile.id;
    $('profile-name').value   = '';
});

$('profile-load-btn').addEventListener('click', () => {
    const id       = $('profile-select').value;
    if (!id) return;
    const profiles = loadProfiles();
    const profile  = profiles.find(p => p.id === id);
    if (!profile) return;

    applySettings(profile.settings);

    if (profile.results && profile.totalShoes) {
        workerResults  = [profile.results];
        workerTotal    = [profile.totalShoes];
        workerProgress = [profile.totalShoes];
        hasRanSim      = true;

        $('progress-bar-fill').style.width = '100%';
        $('progress-pct').textContent      = '100%';
        $('progress-stats').textContent    = `Loaded — ${fmtShoes(profile.totalShoes)}`;
        $('progress-panel').style.display  = 'flex';
        $('results-panel').style.display   = 'flex';
        $('placeholder').style.display     = 'none';
        $('worker-bars').innerHTML         = '';
        scheduleRedraw();
    }
});

$('profile-delete-btn').addEventListener('click', () => {
    const id = $('profile-select').value;
    if (!id) return;
    if (!confirm('Delete this profile?')) return;
    saveProfiles(loadProfiles().filter(p => p.id !== id));
    refreshProfileList();
});

// ── Export ────────────────────────────────────────────────────────────────────
$('export-btn').addEventListener('click', () => {
    const blob = {
        meta:    { config: getConfig(), betStrategy: betRows, generatedAt: new Date().toISOString(), version: 1 },
        results: mergeResults(),
    };
    const url = URL.createObjectURL(new Blob([JSON.stringify(blob, null, 2)], { type: 'application/json' }));
    Object.assign(document.createElement('a'), { href: url, download: 'bjsim-results.json' }).click();
    URL.revokeObjectURL(url);
});

// ── Init ──────────────────────────────────────────────────────────────────────
updatePenSlider();
initCharts();
refreshProfileList();
