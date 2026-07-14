#!/usr/bin/env bash
set -euo pipefail

DEST_IP="${DEST_IP:-192.168.1.100}"
DEST_PORT="${DEST_PORT:-6666}"
SRC_PORT="${SRC_PORT:-6666}"

if [[ $EUID -ne 0 ]]; then
    echo "Aja tämä sudo-komennolla:"
    echo "  sudo $0"
    exit 1
fi

echo "Netconsole-kohde: ${DEST_IP}:${DEST_PORT}"

# Selvitetään reitti kohdekoneelle.
ROUTE="$(ip -4 route get "$DEST_IP" 2>/dev/null | head -n 1 || true)"

if [[ -z "$ROUTE" ]]; then
    echo "Virhe: reittiä osoitteeseen $DEST_IP ei löytynyt."
    exit 1
fi

IFACE="$(awk '
{
    for (i = 1; i <= NF; i++) {
        if ($i == "dev") {
            print $(i + 1)
            exit
        }
    }
}' <<< "$ROUTE")"

SRC_IP="$(awk '
{
    for (i = 1; i <= NF; i++) {
        if ($i == "src") {
            print $(i + 1)
            exit
        }
    }
}' <<< "$ROUTE")"

if [[ -z "$IFACE" || -z "$SRC_IP" ]]; then
    echo "Virhe: lähde-IP:tä tai verkkoliitäntää ei saatu selvitettyä."
    echo "Reitti: $ROUTE"
    exit 1
fi

echo "Lähde-IP:       $SRC_IP"
echo "Verkkoliitäntä: $IFACE"

# Täytetään ARP/neighbor-taulu.
ping -c 1 -W 1 -I "$IFACE" "$DEST_IP" >/dev/null 2>&1 || true

DEST_MAC="$(
    ip neigh show to "$DEST_IP" dev "$IFACE" 2>/dev/null |
    awk '
        {
            for (i = 1; i <= NF; i++) {
                if ($i == "lladdr" && (i + 1) <= NF) {
                    print $(i + 1)
                    exit
                }
            }
        }
    '
)"

if [[ -z "$DEST_MAC" ]] && command -v arping >/dev/null 2>&1; then
    arping -c 1 -w 2 -I "$IFACE" "$DEST_IP" >/dev/null 2>&1 || true

    DEST_MAC="$(
        ip neigh show to "$DEST_IP" dev "$IFACE" 2>/dev/null |
        awk '
            {
                for (i = 1; i <= NF; i++) {
                    if ($i == "lladdr" && (i + 1) <= NF) {
                        print $(i + 1)
                        exit
                    }
                }
            }
        '
    )"
fi

# Jos tavallinen ping ei täyttänyt neighbor-taulua, kokeillaan arpingia.
if [[ -z "$DEST_MAC" ]] && command -v arping >/dev/null 2>&1; then
    arping -c 1 -w 2 -I "$IFACE" "$DEST_IP" >/dev/null 2>&1 || true

    DEST_MAC="$(ip neigh show "$DEST_IP" dev "$IFACE" |
        awk '$0 !~ /FAILED|INCOMPLETE/ { print $5; exit }')"
fi

if [[ -z "$DEST_MAC" ]]; then
    echo "Virhe: kohdekoneen MAC-osoitetta ei saatu selvitettyä."
    echo
    echo "Varmista, että $DEST_IP on päällä ja samassa lähiverkossa."
    echo "Voit tarkistaa tilanteen komennolla:"
    echo "  ip neigh show $DEST_IP"
    exit 1
fi

echo "Kohteen MAC:    $DEST_MAC"

NETCONSOLE_ARG="${SRC_PORT}@${SRC_IP}/${IFACE},${DEST_PORT}@${DEST_IP}/${DEST_MAC}"

echo
echo "Netconsole-argumentti:"
echo "  $NETCONSOLE_ARG"
echo

# Poistetaan vanha instanssi, jos sellainen on ladattuna.
if lsmod | grep -q '^netconsole '; then
    echo "Poistetaan vanha netconsole-moduuli..."
    modprobe -r netconsole
fi

echo "Ladataan netconsole..."
modprobe netconsole "netconsole=${NETCONSOLE_ARG}"

# Varmistetaan, että kaikki printk-tasot pääsevät konsoliin.
sysctl -w kernel.printk="8 8 1 7" >/dev/null

# Debug-käytössä ratelimit kannattaa poistaa käytöstä.
sysctl -w kernel.printk_ratelimit=0 >/dev/null
sysctl -w kernel.printk_ratelimit_burst=0 >/dev/null

if ! lsmod | grep -q '^netconsole '; then
    echo "Virhe: netconsole ei näytä olevan ladattuna."
    exit 1
fi

echo
echo "NETCONSOLE ARMED"
echo "  ${SRC_IP}:${SRC_PORT} -> ${DEST_IP}:${DEST_PORT}"
echo "  interface: $IFACE"
echo "  destination MAC: $DEST_MAC"

echo "<6>NETCONSOLE TEST OK from $(hostname) at $(date --iso-8601=seconds)" \
    > /dev/kmsg

echo
echo "Testiviesti lähetettiin."
echo "Vastaanota kohdekoneella esimerkiksi:"
echo "  sudo nc -klu $DEST_PORT"
echo
echo "Tai tarkkaile paketteja:"
echo "  sudo tcpdump -ni any udp port $DEST_PORT"
