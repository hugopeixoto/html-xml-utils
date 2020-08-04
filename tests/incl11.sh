:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

printf "Test\n" >$TMP2

printf '<!-- include "'$TMP2'" -->' | ./hxincl -f >$TMP1

cmp -s $TMP1 $TMP2
