:

trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmpXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmpXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmpXXXXXX` || exit 1

cat >$TMP1 <<EOF
<doc><a id=a1><a id=b1 /><a id=b2 /></a><a id=a2 /></doc>
EOF

./hxselect ':not(:first-child)' <$TMP1 >$TMP2
./hxselect ':nth-child(n+2)' <$TMP1 >$TMP3
diff -u $TMP2 $TMP3
