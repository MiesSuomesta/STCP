#!/usr/bin/env bash
#
# decode-oops.sh
# Käyttö:
#   dmesg | tail -200 | ~/bin/decode-oops.sh
#   tai
#   ~/bin/decode-oops.sh debug.kernel.log
#

# Säädä nämä omalle puullesi sopiviksi
KDIR="/home/pomo/stcp/kernel/sources/theMasterLinux"      # kernel-lähteet, missä scripts/ ja vmlinux
VMLINUX="$KDIR/vmlinux"                       # debugattu vmlinux
#MODDIR="/lib/modules/$(uname -r)"             # moduulit (voi olla myös oma build-kansio)
MODDIR="/home/pomo/stcp/kernel/OOT/linux/kmod/"             # moduulit (voi olla myös oma build-kansio)

DECODE="$KDIR/scripts/decode_stacktrace.sh"

if [[ ! -x "$DECODE" ]]; then
  echo "ERROR: $DECODE ei löydy tai ei ole ajettava."
  echo "Tarkista KDIR-polku skriptistä."
  exit 1
fi

if [[ ! -f "$VMLINUX" ]]; then
  echo "ERROR: vmlinux ei löydy: $VMLINUX"
  echo "Käännä kerneli 'vmlinux' debug-symboleilla tai korjaa polku."
  exit 1
fi

# Luetaan oops joko tiedostosta tai stdin:stä
if [[ -n "$1" && -f "$1" ]]; then
  INPUT="$1"
else
  INPUT="/dev/stdin"
fi

# Varsinainen dekoodaus
# Parametrit:
#   vmlinux    – symbolit
#   MODDIR     – moduulikansiot
#   (lisäksi voi antaa lisää moduulikansioita, jos käytät omaa buildiasemaa)
"$DECODE" "$VMLINUX" "$MODDIR" /home/pomo/stcp/kernel/OOT/linux/kmod  < "$INPUT"
