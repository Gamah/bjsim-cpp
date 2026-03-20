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

// ── Illustrious 18 deviation definitions ──────────────────────────────────────
// bit: matches dev:: namespace in utilities.h (bit position 0-27)
const DEVIATIONS = [
    { group: 'Pair Splits' },
    { bit: 0,  label: 'Split 10s vs 4  (TC ≥ 6)' },
    { bit: 1,  label: 'Split 10s vs 5  (TC ≥ 5)' },
    { bit: 2,  label: 'Split 10s vs 6  (TC ≥ 4)' },
    { group: 'Soft Totals' },
    { bit: 3,  label: 'Double soft 19 vs 4  (TC ≥ 3)' },
    { bit: 4,  label: 'Double soft 19 vs 5  (TC ≥ 1)' },
    { bit: 5,  label: 'Stand soft 19 vs 6  (neg count)' },
    { bit: 6,  label: 'Double soft 17 vs 2  (TC ≥ 1)' },
    { group: 'Hard 16' },
    { bit: 7,  label: 'Stand 16 vs 9  (TC ≥ 4)' },
    { bit: 8,  label: 'Stand 16 vs 10  (pos count)' },
    { bit: 9,  label: 'Stand 16 vs A  (TC ≥ 3)' },
    { group: 'Hard 15' },
    { bit: 10, label: 'Stand 15 vs 10  (TC ≥ 4)' },
    { bit: 11, label: 'Stand 15 vs A  (TC ≥ 5)' },
    { group: 'Hard 12–13' },
    { bit: 12, label: 'Hit 13 vs 2  (TC ≤ −1)' },
    { bit: 13, label: 'Stand 12 vs 2  (TC ≥ 3)' },
    { bit: 14, label: 'Stand 12 vs 3  (TC ≥ 2)' },
    { bit: 15, label: 'Hit 12 vs 4  (neg count)' },
    { group: 'Hard Doubles' },
    { bit: 16, label: 'Double 10 vs 10  (TC ≥ 4)' },
    { bit: 17, label: 'Double 10 vs A  (TC ≥ 3)' },
    { bit: 18, label: 'Double 9 vs 2  (TC ≥ 1)' },
    { bit: 19, label: 'Double 9 vs 7  (TC ≥ 3)' },
    { bit: 20, label: 'Double 8 vs 6  (TC ≥ 2)' },
    { group: 'Surrender Deviations' },
    { bit: 21, label: 'Surrender 17 vs A  (H17 games)' },
    { bit: 22, label: 'Surrender 16 vs 8  (TC ≥ 4)' },
    { bit: 23, label: 'Hit 16 vs 9  (TC ≤ −1)' },
    { bit: 24, label: 'Surrender 16 vs 10/A  (H17 games)' },
    { bit: 25, label: 'Surrender 15 vs 9  (TC ≥ 2)' },
    { bit: 26, label: 'Hit 15 vs 10  (neg count)' },
    { bit: 27, label: 'Surrender 15 vs A  (TC ≤ −1)' },
];

// ── Simulation state ──────────────────────────────────────────────────────────
let workers        = [];
let workerResults  = [];
let workerProgress = [];
let workerTotal    = [];
let simStartTime   = 0;
let simRunning     = false;
let hasRanSim      = false;
let resimTimer     = null;
let progressInterval = null;

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

function fmtTime(s) {
    if (!isFinite(s) || s > 7200) return '>2h';
    if (s > 3600) return Math.floor(s/3600) + 'h ' + Math.floor((s%3600)/60) + 'm';
    const m = Math.floor(s / 60), sec = Math.floor(s % 60);
    return m > 0 ? `${m}m ${sec}s` : `${sec}s`;
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

document.querySelectorAll('[data-group="decks"] button').forEach(b =>
    b.addEventListener('click', () => { updatePenSlider(); triggerAutoResim(); })
);
document.querySelectorAll('[data-group="surrender"] button').forEach(b =>
    b.addEventListener('click', triggerAutoResim)
);
document.querySelectorAll('[data-group="maxsplit"] button').forEach(b =>
    b.addEventListener('click', triggerAutoResim)
);
document.querySelectorAll('[data-group="strategy"] button').forEach(b =>
    b.addEventListener('click', () => {
        const isDevs = getSelected('strategy') === '2';
        $('deviation-container').style.display = isDevs ? '' : 'none';
        triggerAutoResim();
    })
);

// ── Deviation section ─────────────────────────────────────────────────────────
function computeDeviationMask() {
    let mask = 0;
    document.querySelectorAll('.dev-check').forEach(cb => {
        if (cb.checked) mask |= (1 << parseInt(cb.dataset.bit));
    });
    return mask;
}

function setAllDeviations(checked) {
    document.querySelectorAll('.dev-check').forEach(cb => { cb.checked = checked; });
    triggerAutoResim();
}

function initDeviationSection() {
    const list = $('deviation-list');
    list.innerHTML = '';
    let currentGroup = null;

    for (const entry of DEVIATIONS) {
        if (entry.group !== undefined) {
            const hdr = document.createElement('div');
            hdr.className = 'dev-group-label';
            hdr.textContent = entry.group;
            list.appendChild(hdr);
            currentGroup = entry.group;
            continue;
        }
        const row = document.createElement('label');
        row.className = 'dev-item';
        row.innerHTML = `<input type="checkbox" class="dev-check" data-bit="${entry.bit}" checked> ${entry.label}`;
        list.appendChild(row);
    }

    list.querySelectorAll('.dev-check').forEach(cb =>
        cb.addEventListener('change', triggerAutoResim)
    );

    $('dev-all-btn').addEventListener('click',  () => setAllDeviations(true));
    $('dev-none-btn').addEventListener('click', () => setAllDeviations(false));
}

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
function getPlayerStrategy() {
    return parseInt(getSelected('strategy') ?? '2');
}

function getConfig() {
    const strategy = getPlayerStrategy();
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
        playerStrategy:  strategy,
        deviationMask:   strategy === 2 ? computeDeviationMask() : 0,
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
        const tcVal          = tc(i);
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
    $('progress-eta').textContent = `${fmt(doneShoes)} / ${fmt(totalShoes)} shoes  ·  ${fmt(Math.round(rate))}/s  ·  ETA ${fmtTime(remaining)}`;

    // Update worker tiles
    document.querySelectorAll('.worker-tile').forEach((tile, i) => {
        const p = workerTotal[i] ? workerProgress[i] / workerTotal[i] : 0;
        tile.querySelector('.worker-tile-fill').style.width = (p * 100).toFixed(1) + '%';
        tile.querySelector('.worker-tile-count').textContent = fmt(workerProgress[i]);
    });
}

// 250ms tick: update progress + trigger redraw
function tick() {
    updateProgress();
    scheduleRedraw();
}

// ── Worker message handling ───────────────────────────────────────────────────
function handleWorkerMessage(e, workerId) {
    const { type, shoesRun, results } = e.data;

    if (type === 'progress') {
        workerProgress[workerId] = shoesRun;
        if (results) workerResults[workerId] = results;
        // No immediate UI update — the 250ms interval handles it
    } else if (type === 'done') {
        workerProgress[workerId] = workerTotal[workerId];
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
    clearInterval(progressInterval);
    progressInterval = null;

    simRunning = false;
    hasRanSim  = true;
    workers.forEach(w => w.terminate());
    workers = [];

    $('run-btn').disabled        = false;
    $('run-btn').textContent     = 'Run Simulation';
    $('cancel-btn').style.display = 'none';
    $('progress-bar-fill').style.width = '100%';
    $('progress-pct').textContent      = '100%';
    $('progress-eta').textContent      = `Complete — ${((Date.now()-simStartTime)/1000).toFixed(1)}s`;

    // Final redraw with complete data
    document.querySelectorAll('.worker-tile').forEach((tile, i) => {
        tile.querySelector('.worker-tile-fill').style.width = '100%';
    });

    scheduleRedraw();
    refreshProfileList();
}

// ── Run / Cancel ──────────────────────────────────────────────────────────────
$('run-btn').addEventListener('click', startSim);
$('cancel-btn').addEventListener('click', () => {
    workers.forEach(w => w.terminate());
    workers = [];
    finishSim();
});

function buildWorkerGrid(numWorkers) {
    const tilesEl = $('worker-tiles');
    tilesEl.innerHTML = '';

    const cols = numWorkers === 1 ? 1 : Math.ceil(numWorkers / 2);
    tilesEl.style.gridTemplateColumns = `repeat(${cols}, 1fr)`;

    for (let i = 0; i < numWorkers; i++) {
        const tile = document.createElement('div');
        tile.className = 'worker-tile';
        tile.innerHTML = `
            <div class="worker-tile-header">
                <span class="worker-tile-label">W${i + 1}</span>
                <span class="worker-tile-count">0</span>
            </div>
            <div class="worker-tile-track">
                <div class="worker-tile-fill"></div>
            </div>
        `;
        tilesEl.appendChild(tile);
    }
}

function startSim() {
    if (simRunning) {
        clearInterval(progressInterval);
        progressInterval = null;
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

    buildWorkerGrid(numWorkers);

    $('progress-bar-fill').style.width = '0%';
    $('progress-pct').textContent      = '0%';
    $('progress-eta').textContent      = 'Starting workers…';
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

    // 250ms progress tick — saves CPU vs per-message DOM updates
    progressInterval = setInterval(tick, 250);
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
        decks:          parseInt(getSelected('decks')),
        penQ:           parseInt($('pen-slider').value),
        h17:            $('h17').checked,
        bj65:           $('bj65').checked,
        das:            $('das').checked,
        rsa:            $('rsa').checked,
        surrender:      parseInt(getSelected('surrender')),
        maxsplit:       parseInt(getSelected('maxsplit')),
        otherPlayers:   parseInt($('other-players').value) || 0,
        shoes:          parseInt(getSelected('shoes')),
        threads:        parseInt($('thread-slider').value),
        rph:            parseInt($('rph').value),
        bankroll:       parseFloat($('bankroll').value) || 0,
        betRows:        JSON.parse(JSON.stringify(betRows)),
        playerStrategy: getPlayerStrategy(),
        deviationMask:  computeDeviationMask(),
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

    if (s.playerStrategy !== undefined) {
        setSelected('strategy', s.playerStrategy);
        const isDevs = s.playerStrategy === 2;
        $('deviation-container').style.display = isDevs ? '' : 'none';
    }
    if (s.deviationMask !== undefined) {
        document.querySelectorAll('.dev-check').forEach(cb => {
            cb.checked = !!(s.deviationMask & (1 << parseInt(cb.dataset.bit)));
        });
    }
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
        $('progress-eta').textContent      = `Loaded — ${fmtShoes(profile.totalShoes)}`;
        $('progress-panel').style.display  = 'flex';
        $('results-panel').style.display   = 'flex';
        $('placeholder').style.display     = 'none';
        $('worker-tiles').innerHTML        = '';
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
initDeviationSection();
refreshProfileList();
