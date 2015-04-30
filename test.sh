function dofolder() {
lastfile=""
for f in $1/*
do
  if [[ -n "$lastfile" ]]; then
    them=""
    us=""
    outus=$(/mnt/work/Documents/7Limi/build/buildr/timbuk "$lastfile" "$f" 2>&1)
    #outthem=$(./hkc -incl "$lastfile" "$f" 2>&1)
    if [[ "$outthem" == *"true"* ]]; then
      them=true
    fi
    if [[ "$outthem" == *"false"* ]]; then
      them=false
    fi
    #if [[ -z "$them" ]]; then
    #  echo "They did not answer"
    #  echo "$outthem"
    #  exit 1
    #fi
    if [[ "$outus" == *"Not Included"* ]]; then
      us=false
    elif [[ "$outus" == *"Included"* ]]; then
      us=true
    fi
    if [[ -z "$us" ]]; then
      echo "We did not answer"
      exit 1
    fi
    #if [[ "$them" != "$us" ]]; then
    #  echo "Answers differ"
    #fi
    timethem=$(echo "$outthem" | grep seconds | cut -f2 | cut -d" " -f1)
    timeus=$(echo "$outus" | grep TIME | cut -d" " -f2)
    if [[ "$(echo $timeus'>'1.0 | bc)" == "1" ]]; then
    echo -ne "$f\t"
    echo -ne "$us\t"
    echo -ne "$timethem\t"
    echo $timeus
    fi
  else
    echo "$f"
  fi
  lastfile=$f
done
}

for fo in ARMCautomata/*; do
  dofolder "$fo"
done
