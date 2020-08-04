:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

echo "<" | ./xml2asc >$TMP1
echo "<" >$TMP2
cmp -s $TMP1 $TMP2
