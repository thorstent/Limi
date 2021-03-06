
function dofolder() {
lastfile=""
for f in $1/*
do
  if [[ -n "$lastfile" && $(du -k "$f" | cut -f 1) -gt 1000 ]]; then
    them=""
    us=""
    outus=$(../build/buildr/timbuk "$lastfile" "$f" 2>&1)
    outthem=$(/mnt/work/Documents/hknt-1.0/libvata/build/cli/vata -t incl "$lastfile" "$f" 2>&1)
    if [[ "$(echo "$outthem" | tail -1)" == "1" ]]; then
      them=true
    fi
    if [[ "$(echo "$outthem" | tail -1)" == "0" ]]; then
      them=false
    fi
    if [[ -z "$them" ]]; then
      echo "They did not answer"
      echo "$outthem"
      exit 1
    fi
    if [[ "$outus" == *"Not Included"* ]]; then
      us=false
    elif [[ "$outus" == *"Included"* ]]; then
      us=true
    fi
    if [[ -z "$us" ]]; then
      echo "We did not answer"
      exit 1
    fi
    if [[ "$them" != "$us" ]]; then
      echo "Answers differ"
    fi
    timethem=$(echo "$outthem" | head -1)
    timeus=$(echo "$outus" | grep TIME | cut -d" " -f2)
    if [[ -z "$timethem" ]]; then
    exit 1
    fi
    #if [[ "$(echo $timethem'>'0.9 | bc)" == "1" ]]; then
    echo "$lastfile"
    echo -ne "$f\t"
    echo -ne "$us\t"
    echo -ne "$timethem\t"
    echo $timeus
    #fi
  #else
    #echo "$f"
  fi
  lastfile=$f
done
}

for fo in /mnt/work/Documents/hknt-1.0/ARMCautomata/*; do
  dofolder "$fo"
done
