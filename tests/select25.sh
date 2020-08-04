:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmpXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmpXXXXXX` || exit 1

./hxselect -s '\n' 'abc[alt=""]' >$TMP1 <<-EOF
<doc>
  <nested>
    <abc alt="a1">def</abc>
    <abc alt="">ghi</abc>
  </nested>
</doc>
EOF
cat >$TMP2 <<EOF
<abc alt="">ghi</abc>
EOF

diff -u $TMP1 $TMP2
