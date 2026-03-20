#!/usr/bin/env bash
# deploy.sh — one-shot build and deploy for bjsim on Ubuntu 24.04
# Usage: sudo bash deploy.sh
set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EMSDK_DIR="/opt/emsdk"
DEPLOY_DIR="/var/www/bjsim"
SITE_NAME="bjsim"

# ── Colours ───────────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; CYAN='\033[0;36m'; NC='\033[0m'
info()  { echo -e "${GREEN}[+]${NC} $1"; }
step()  { echo -e "\n${CYAN}━━ $1 ━━${NC}"; }
warn()  { echo -e "${YELLOW}[!]${NC} $1"; }
die()   { echo -e "${RED}[✗]${NC} $1" >&2; exit 1; }

[[ $EUID -ne 0 ]] && die "Run as root or with sudo: sudo bash deploy.sh"

# ── System packages ───────────────────────────────────────────────────────────
step "System packages"
apt-get update -qq
apt-get install -y -qq build-essential git python3 nginx
info "System packages ready"

# ── Emscripten SDK ────────────────────────────────────────────────────────────
step "Emscripten SDK"
if [[ ! -d "$EMSDK_DIR/.git" ]]; then
    info "Cloning emsdk to $EMSDK_DIR (this may take a minute)..."
    git clone --depth=1 https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR"
else
    info "Updating emsdk..."
    git -C "$EMSDK_DIR" pull --ff-only --quiet
fi

info "Installing latest Emscripten toolchain (first run takes ~10–15 min)..."
"$EMSDK_DIR/emsdk" install latest
"$EMSDK_DIR/emsdk" activate latest

# Source emsdk environment into this script session
# shellcheck source=/dev/null
source "$EMSDK_DIR/emsdk_env.sh" >/dev/null 2>&1
info "emcc $(emcc --version | head -1)"

# ── Tests ─────────────────────────────────────────────────────────────────────
step "Test suite"
cd "$REPO_DIR"
g++ -O2 -std=c++17 -o tests/tests \
    tests/tests.cpp hand.cpp shoe.cpp strategies.cpp utilities.cpp card.cpp player.cpp
./tests/tests | tail -1   # print only the summary line; full output on failure

# ── Native build ──────────────────────────────────────────────────────────────
step "Native binary"
g++ -O3 -std=c++17 -pthread -o bjsim \
    game.cpp player.cpp strategies.cpp shoe.cpp hand.cpp card.cpp utilities.cpp bjsim.cpp
info "bjsim built"

# ── WebAssembly build ─────────────────────────────────────────────────────────
step "WebAssembly"
mkdir -p "$REPO_DIR/public"
emcc -O3 -std=c++17 \
    -s EXPORTED_FUNCTIONS='["_bjsim_configure","_bjsim_run_batch","_bjsim_reset","_bjsim_get_results"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s MODULARIZE=1 \
    -s EXPORT_NAME=BJSim \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s ENVIRONMENT=worker \
    -o "$REPO_DIR/public/bjsim.js" \
    game.cpp player.cpp strategies.cpp shoe.cpp hand.cpp card.cpp utilities.cpp wasm_api.cpp
info "bjsim.js + bjsim.wasm built ($(du -sh "$REPO_DIR/public/bjsim.wasm" | cut -f1) wasm)"

# ── Deploy web files ──────────────────────────────────────────────────────────
step "Deploying to $DEPLOY_DIR"
mkdir -p "$DEPLOY_DIR"
cp -r "$REPO_DIR/public/." "$DEPLOY_DIR/"
chown -R www-data:www-data "$DEPLOY_DIR"
info "Web files copied"

# ── WebAssembly MIME type ─────────────────────────────────────────────────────
# Ubuntu 24.04 nginx ships application/wasm in mime.types; add it if absent
if ! grep -q "application/wasm" /etc/nginx/mime.types; then
    warn "application/wasm not found in mime.types — adding it"
    sed -i '/^types {/a\    application/wasm                      wasm;' /etc/nginx/mime.types
fi

# ── Nginx site config ─────────────────────────────────────────────────────────
step "Nginx configuration"
cat > "/etc/nginx/sites-available/$SITE_NAME" << EOF
server {
    listen 80 default_server;
    listen [::]:80 default_server;

    root $DEPLOY_DIR;
    index index.html;

    server_name _;

    # WebAssembly and large JS — cache for 1 hour, serve gzip for .js only
    location ~* \\.js\$ {
        expires 1h;
        add_header Cache-Control "public";
        gzip_static on;
    }

    # .wasm files must not be re-compressed (already binary-compressed by emcc)
    location ~* \\.wasm\$ {
        expires 1h;
        add_header Cache-Control "public";
        gzip off;
    }

    # HTML — never cache so redeployments are immediate
    location ~* \\.html\$ {
        expires -1;
        add_header Cache-Control "no-store, no-cache, must-revalidate";
    }

    location / {
        try_files \$uri \$uri/ =404;
    }
}
EOF

ln -sf "/etc/nginx/sites-available/$SITE_NAME" "/etc/nginx/sites-enabled/$SITE_NAME"
# Remove the default placeholder site so ours is the only one on port 80
rm -f /etc/nginx/sites-enabled/default

nginx -t
systemctl enable nginx --quiet
systemctl restart nginx

# ── Done ──────────────────────────────────────────────────────────────────────
IP=$(hostname -I | awk '{print $1}')
echo -e "\n${GREEN}✓ Deployed.${NC}  Open http://${IP}/ in your browser.\n"
